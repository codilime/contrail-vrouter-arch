#include "windows_devices.h"
#include <ntstrsafe.h>

#include "vrouter.h"
#include "vr_packet.h"

static const ULONG pkt0_allocation_tag = '0TKP';

static const WCHAR Pkt0DeviceName[] = L"\\Device\\vrouterPkt0";
static const WCHAR Pkt0DeviceSymLink[] = L"\\DosDevices\\vrouterPkt0";

static PDEVICE_OBJECT Pkt0DeviceObject = NULL;
static BOOLEAN Pkt0SymlinkCreated = FALSE;

static struct pkt0_packet *alloc_pkt0_packet(struct vr_packet *vrp);
static void free_pkt0_packet(struct pkt0_packet * packet);

static IO_WORKITEM_ROUTINE_EX Pkt0DeferredWrite;

struct pkt0_context {
    KSPIN_LOCK lock;
    LIST_ENTRY pkt_queue;
    LIST_ENTRY irp_queue;

    KSPIN_LOCK write_lock;
    LIST_ENTRY irp_write_queue;
};

static void
Pkt0FinalizeIrp(PIRP irp)
{
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
}

static NTSTATUS
Pkt0DispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = FILE_OPENED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

static NTSTATUS
Pkt0DispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

static NTSTATUS
Pkt0DispatchCleanup(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    struct pkt0_context *ctx = VRouterGetPrivateData(DeviceObject);
    PLIST_ENTRY entry;
    PIRP irp;
    KIRQL old_irql;

    KeAcquireSpinLock(&ctx->lock, &old_irql);
    while (!IsListEmpty(&ctx->irp_queue)) {
        entry = RemoveHeadList(&ctx->irp_queue);
        irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
        Pkt0FinalizeIrp(irp);
    }
    KeReleaseSpinLock(&ctx->lock, old_irql);

    KeAcquireSpinLock(&ctx->write_lock, &old_irql);
    if (!IsListEmpty(&ctx->irp_write_queue)) {
        entry = RemoveHeadList(&ctx->irp_write_queue);
        irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
        Pkt0FinalizeIrp(irp);
    }
    KeReleaseSpinLock(&ctx->write_lock, old_irql);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Irp->IoStatus.Status;
}

static VOID
Pkt0DeferredWrite(_In_ PVOID IoObject,
                  _In_opt_ PVOID Context,
                  _In_ PIO_WORKITEM WorkItem)
{
    PDEVICE_OBJECT device_object = IoObject;
    struct pkt0_context *ctx = VRouterGetPrivateData(device_object);
    PLIST_ENTRY entry = NULL;
    PIRP irp = NULL;
    PIO_STACK_LOCATION io_stack;
    ULONG count;
    unsigned char *data = NULL;
    unsigned char *pkt_data = NULL;
    KIRQL old_irql;

    struct vrouter *router = vrouter_get(0);
    struct vr_interface *agent_if;
    struct vr_packet *pkt = NULL;

    KeAcquireSpinLock(&ctx->write_lock, &old_irql);
    if (!IsListEmpty(&ctx->irp_write_queue)) {
        entry = RemoveHeadList(&ctx->irp_write_queue);
        irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
    }
    KeReleaseSpinLock(&ctx->write_lock, old_irql);
    if (irp == NULL)
        goto fail;

    agent_if = router->vr_agent_if;
    if (agent_if == NULL)
        goto fail;

    io_stack = IoGetCurrentIrpStackLocation(irp);
    count = io_stack->Parameters.Write.Length;
    data = MmGetSystemAddressForMdlSafe(irp->MdlAddress, LowPagePriority | MdlMappingNoExecute);
    if (data == NULL)
        goto fail;

    pkt_data = ExAllocatePoolWithTag(NonPagedPoolNx, count, pkt0_allocation_tag);
    if (pkt_data == NULL)
        goto fail;

    RtlCopyMemory(pkt_data, data, count);

    pkt = win_allocate_packet(pkt_data, count, pkt0_allocation_tag);
    if (pkt == NULL)
        goto fail;
    pkt->vp_if = agent_if;

    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = count;
    IoCompleteRequest(irp, IO_NO_INCREMENT);

    agent_if->vif_rx(agent_if, pkt, VLAN_ID_INVALID);

    IoFreeWorkItem(WorkItem);
    return;

fail:
    if (pkt_data) {
        ExFreePoolWithTag(pkt_data, pkt0_allocation_tag);
    }
    if (irp) {
        irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
    IoFreeWorkItem(WorkItem);
    return;
}

static NTSTATUS
Pkt0DispatchWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    struct pkt0_context *ctx = VRouterGetPrivateData(DeviceObject);
    PIO_WORKITEM work_item;
    KIRQL old_irql;

    work_item = IoAllocateWorkItem(DeviceObject);
    if (work_item == NULL) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IoMarkIrpPending(Irp);

    KeAcquireSpinLock(&ctx->write_lock, &old_irql);
    InsertTailList(&ctx->irp_write_queue, &Irp->Tail.Overlay.ListEntry);
    KeReleaseSpinLock(&ctx->write_lock, old_irql);

    IoQueueWorkItemEx(work_item, Pkt0DeferredWrite, DelayedWorkQueue, NULL);

    return STATUS_PENDING;
}

static NTSTATUS
Pkt0TransferPacketToUser(PIRP Irp, struct pkt0_packet *packet)
{
    PIO_STACK_LOCATION io_stack;
    PVOID buffer;
    NTSTATUS ret;

    io_stack = IoGetCurrentIrpStackLocation(Irp);
    buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, LowPagePriority | MdlMappingNoExecute);
    if (buffer != NULL) {
        ASSERTMSG("Read buffer too short", io_stack->Parameters.Read.Length >= packet->length);
        RtlCopyMemory(buffer, packet->buffer, packet->length);
        Irp->IoStatus.Information = packet->length;
        ret = STATUS_SUCCESS;
    } else {
        Irp->IoStatus.Information = 0;
        ret = STATUS_INSUFFICIENT_RESOURCES;
    }
    free_pkt0_packet(packet);

    Irp->IoStatus.Status = ret;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return ret;
}

static NTSTATUS
Pkt0DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    struct pkt0_context *ctx = VRouterGetPrivateData(DeviceObject);

    struct pkt0_packet *pkt;
    KIRQL old_irql;
    PLIST_ENTRY entry;
    NTSTATUS status;

    KeAcquireSpinLock(&ctx->lock, &old_irql);
    if (IsListEmpty(&ctx->pkt_queue)) {
        InsertTailList(&ctx->irp_queue, &Irp->Tail.Overlay.ListEntry);
        IoMarkIrpPending(Irp);
        status = STATUS_PENDING;
    } else {
        entry = RemoveHeadList(&ctx->pkt_queue);
        pkt = CONTAINING_RECORD(entry, struct pkt0_packet, list_entry);
        status = Pkt0TransferPacketToUser(Irp, pkt);
    }
    KeReleaseSpinLock(&ctx->lock, old_irql);

    return status;
}

static NTSTATUS
Pkt0DispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
Pkt0CreateDevice(PDRIVER_OBJECT DriverObject)
{
    struct pkt0_context *ctx;
    VR_DEVICE_DISPATCH_CALLBACKS Callbacks = {
        .create         = Pkt0DispatchCreate,
        .close          = Pkt0DispatchClose,
        .cleanup        = Pkt0DispatchCleanup,
        .write          = Pkt0DispatchWrite,
        .read           = Pkt0DispatchRead,
        .device_control = Pkt0DispatchDeviceControl,
    };
    NTSTATUS Status;

    /* Allocate and init pkt0 context data */
    ctx = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(*ctx), pkt0_allocation_tag);
    if (ctx == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeSpinLock(&ctx->lock);
    InitializeListHead(&ctx->pkt_queue);
    InitializeListHead(&ctx->irp_queue);

    KeInitializeSpinLock(&ctx->write_lock);
    InitializeListHead(&ctx->irp_write_queue);

    /* Create and initialize named pipe server for Pkt0 */
    Status = VRouterSetUpNamedPipeServer(DriverObject,
                                         Pkt0DeviceName,
                                         Pkt0DeviceSymLink,
                                         &Callbacks,
                                         &Pkt0DeviceObject,
                                         &Pkt0SymlinkCreated);
    if (!NT_SUCCESS(Status))
        goto failure;

    /* Attach pkt0 context to the device */
    VRouterAttachPrivateData(Pkt0DeviceObject, ctx);

    return Status;

failure:
    ExFreePoolWithTag(ctx, pkt0_allocation_tag);
    return Status;
}

VOID
Pkt0DestroyDevice(PDRIVER_OBJECT DriverObject)
{
    PLIST_ENTRY entry;
    struct pkt0_context *ctx;
    struct pkt0_packet *packet;
    KIRQL old_irql;

    ctx = (struct pkt0_context *)VRouterGetPrivateData(Pkt0DeviceObject);

    /* Tear down pipe first so user space will not invoke any dispatch routine */
    VRouterTearDownNamedPipeServer(DriverObject,
                                   Pkt0DeviceSymLink,
                                   &Pkt0DeviceObject,
                                   &Pkt0SymlinkCreated);

    /* Free resources allocated by dp-core - trapped pkt0 packet */
    KeAcquireSpinLock(&ctx->lock, &old_irql);
    while (!IsListEmpty(&ctx->pkt_queue)) {
        entry = RemoveHeadList(&ctx->pkt_queue);
        packet = CONTAINING_RECORD(entry, struct pkt0_packet, list_entry);
        free_pkt0_packet(packet);
    }
    KeReleaseSpinLock(&ctx->lock, old_irql);

    ExFreePoolWithTag(ctx, pkt0_allocation_tag);
}

/*
 * Functions called from dp-core
 */

static struct pkt0_packet *
alloc_pkt0_packet(struct vr_packet *vrp)
{
    struct pkt0_packet *packet;
    unsigned int pkt_size = pkt_len(vrp);
    int ret;

    packet = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(struct pkt0_packet), pkt0_allocation_tag);
    if (packet == NULL)
        return NULL;
    RtlZeroMemory(packet, sizeof(*packet));

    packet->buffer = ExAllocatePoolWithTag(NonPagedPoolNx, pkt_size, pkt0_allocation_tag);
    if (packet->buffer == NULL)
        goto failure;
    RtlZeroMemory(packet->buffer, pkt_size);

    ret = vr_pcopy(packet->buffer, vrp, vrp->vp_data, pkt_size);
    if (ret != pkt_size) {
        goto failure;
    }
    packet->length = ret;

    return packet;

failure:
    if (packet)
        free_pkt0_packet(packet);

    return NULL;
}

static void
free_pkt0_packet(struct pkt0_packet *packet)
{
    if (packet->buffer)
        ExFreePoolWithTag(packet->buffer, pkt0_allocation_tag);

    ExFreePoolWithTag(packet, pkt0_allocation_tag);
}

int
pkt0_if_tx(struct vr_interface *vif, struct vr_packet *vrp)
{
    struct pkt0_context *ctx = VRouterGetPrivateData(Pkt0DeviceObject);

    KIRQL old_irql;
    PLIST_ENTRY entry;
    PIRP irp;
    struct pkt0_packet *pkt;

    pkt = alloc_pkt0_packet(vrp);
    if (pkt == NULL)
        return 0;

    KeAcquireSpinLock(&ctx->lock, &old_irql);
    if (IsListEmpty(&ctx->irp_queue)) {
        InsertTailList(&ctx->pkt_queue, &pkt->list_entry);
    } else {
        entry = RemoveHeadList(&ctx->irp_queue);
        irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
        Pkt0TransferPacketToUser(irp, pkt);
    }
    KeReleaseSpinLock(&ctx->lock, old_irql);

    /* vr_packet is no longer needed, drop it without updating statistics */
    win_free_packet(vrp);

    return 0;
}

#include "vr_devices.h"
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

struct pkt0_context {
    KSPIN_LOCK lock;
    LIST_ENTRY pkt_queue;
    LIST_ENTRY irp_queue;
};

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
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

static NTSTATUS
Pkt0DispatchWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
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
        InsertHeadList(&ctx->irp_queue, &Irp->Tail.Overlay.ListEntry);
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
    PIRP irp;
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
    while (!IsListEmpty(&ctx->irp_queue)) {
        entry = RemoveHeadList(&ctx->irp_queue);
        irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = 0;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
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
    unsigned int to_copy;
    int ret;

    packet = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(struct pkt0_packet), pkt0_allocation_tag);
    if (packet == NULL)
        return NULL;

    RtlZeroMemory(packet, sizeof(*packet));

    to_copy = pkt_len(vrp);
    ret = vr_pcopy(packet->buffer, vrp, vrp->vp_data, to_copy);
    if (ret < 0 || ret != to_copy) {
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
        InsertHeadList(&ctx->pkt_queue, &pkt->list_entry);
    } else {
        entry = RemoveHeadList(&ctx->irp_queue);
        irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
        Pkt0TransferPacketToUser(irp, pkt);
    }
    KeReleaseSpinLock(&ctx->lock, old_irql);

    /* Drop pkt - it won't be used anymore in dp-core */
    vr_pfree(vrp, VP_DROP_MISC);

    return 0;
}

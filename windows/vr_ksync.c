#include "vr_os.h"
#include "vr_mem.h"
#include "vr_windows.h"
#include "Ntstrsafe.h"
#include "vr_message.h"

#include "vr_ksync_defs.h"

static ULONG KsyncAllocationTag = 'NYSK';

const WCHAR DeviceName[] = L"\\Device\\vrouterKsync";
const WCHAR DeviceSymLink[] = L"\\DosDevices\\vrouterKsync";

static PDEVICE_OBJECT KsyncDeviceObject = NULL;
static BOOLEAN KsyncSymlinkCreated = FALSE;

static struct ksync_response *
KsyncResponseCreate()
{
    struct ksync_response *resp;

    resp = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(struct ksync_response), KsyncAllocationTag);
    if (resp != NULL) {
        // Zeroing whole struct sets type = 0 (DONE) and gives len = 0, which is a valid response.
        RtlZeroMemory(resp, sizeof(struct ksync_response));
    }

    return resp;
}

static void
KsyncResponseDelete(struct ksync_response *resp)
{
    ASSERT(resp != NULL);

    ExFreePoolWithTag(resp, KsyncAllocationTag);
}

static void
KsyncAppendResponse(struct ksync_device_context *ctx, struct ksync_response *resp)
{
    struct ksync_response *elem;
    struct ksync_response **iter = &(ctx->responses);

    while (*iter != NULL) {
        elem = *iter;
        iter = &(elem->next);
    }

    *iter = resp;
}

static struct ksync_response *
KsyncPopResponse(struct ksync_device_context *ctx)
{
    struct ksync_response *resp = ctx->responses;

    if (resp != NULL) {
        ctx->responses = resp->next;
        resp->next = NULL;
        return resp;
    } else {
        return NULL;
    }
}

_Dispatch_type_(IRP_MJ_CREATE) DRIVER_DISPATCH KsyncDispatchCreate;
_Dispatch_type_(IRP_MJ_CLOSE) DRIVER_DISPATCH KsyncDispatchClose;
_Dispatch_type_(IRP_MJ_WRITE) DRIVER_DISPATCH KsyncDispatchWrite;
_Dispatch_type_(IRP_MJ_READ) DRIVER_DISPATCH KsyncDispatchRead;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH KsyncDispatchDeviceControl;
_Dispatch_type_(IRP_MJ_CLEANUP) DRIVER_DISPATCH KsyncDispatchCleanup;

_Use_decl_annotations_ NTSTATUS
KsyncDispatchCreate(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    struct ksync_device_context *ctx = (struct ksync_device_context *)ExAllocatePoolWithTag(PagedPool, sizeof(struct ksync_device_context), KsyncAllocationTag);

    if (ctx == NULL)
    {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(Irp);
    ctx->user_virtual_address = NULL;
    ctx->responses = NULL;

    pStack->FileObject->FsContext = ctx;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}

_Use_decl_annotations_ NTSTATUS
KsyncDispatchClose(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(Irp);
    struct ksync_device_context *ctx = pStack->FileObject->FsContext;

    if (ctx->user_virtual_address)
    {
        MmUnmapLockedPages(ctx->user_virtual_address, mdl_mem);
    }

    ExFreePoolWithTag(pStack->FileObject->FsContext, KsyncAllocationTag);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

_Use_decl_annotations_ NTSTATUS
KsyncDispatchWrite(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    int ret;
    struct vr_message request, *response;
    struct ksync_device_context *ctx;
    struct ksync_response *ksync_response;
    BOOLEAN multiple_responses;
    enum ksync_response_type response_type;
    UINT32 msg_seq;

    PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(Irp);
    ctx = pStack->FileObject->FsContext;

    PIO_STACK_LOCATION IoStackIrp = NULL;
    PCHAR WriteDataBuffer = NULL;
    IoStackIrp = IoGetCurrentIrpStackLocation(Irp);

    if (Irp->MdlAddress == NULL)
    {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }

    WriteDataBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, LowPagePriority | MdlMappingNoExecute);
    if (!WriteDataBuffer) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    struct nlmsghdr* p_nlmsghdr = (struct nlmsghdr*)WriteDataBuffer;
    struct genlmsghdr* p_genlmsghdr = (struct genlmsghdr*)(WriteDataBuffer + sizeof(struct nlmsghdr));
    struct nlattr *p_nlattrr = (struct nlattr*)(WriteDataBuffer + sizeof(struct nlmsghdr) + sizeof(struct genlmsghdr));

    msg_seq = p_nlmsghdr->nlmsg_seq;
    request.vr_message_buf = NLA_DATA(p_nlattrr);
    request.vr_message_len = NLA_LEN(p_nlattrr);

    DBG_UNREFERENCED_LOCAL_VARIABLE(p_genlmsghdr);

    ret = vr_message_request(&request);
    if (ret < 0) {
        if (vr_send_response(ret))
            return ret;
    }

    multiple_responses = FALSE;
    response_type = KSYNC_RESPONSE_SINGLE;
    while ((response = vr_message_dequeue_response())) {
        if (!multiple_responses && !vr_response_queue_empty()) {
            multiple_responses = TRUE;
            response_type = KSYNC_RESPONSE_MULTIPLE;
        }

        ksync_response = KsyncResponseCreate();
        if (ksync_response == NULL)
            goto failure;

        ksync_response->header.type = response_type;
        ksync_response->header.seq = msg_seq;
        ksync_response->header.len = response->vr_message_len;
        RtlCopyMemory(ksync_response->buffer, response->vr_message_buf, ksync_response->header.len);

        KsyncAppendResponse(ctx, ksync_response);
        vr_message_free(response);
    }

    if (multiple_responses) {
        ksync_response = KsyncResponseCreate();
        if (ksync_response == NULL)
            goto failure;

        ksync_response->header.type = KSYNC_RESPONSE_DONE;
        ksync_response->header.len = 0;

        KsyncAppendResponse(ctx, ksync_response);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = IoStackIrp->Parameters.Write.Length;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

failure:
    // Clean up ksync_response queue for safety
    while ((ksync_response = KsyncPopResponse(ctx))) {
        KsyncResponseDelete(ksync_response);
    }

    // Clean up dp-core message queue for safety
    while ((response = vr_message_dequeue_response())) {
        vr_message_free(response);
    }

    Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INSUFFICIENT_RESOURCES;
}

_Use_decl_annotations_ NTSTATUS
KsyncDispatchRead(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    PIO_STACK_LOCATION IoStackIrp = NULL;
    PCHAR ReadDataBuffer = NULL;
    struct ksync_device_context *ctx;

    PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(Irp);
    ctx = pStack->FileObject->FsContext;

    IoStackIrp = IoGetCurrentIrpStackLocation(Irp);
    if (Irp->MdlAddress == NULL)
    {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }

    ReadDataBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, LowPagePriority | MdlMappingNoExecute);
    if (ReadDataBuffer != NULL) {
        struct ksync_response *resp = KsyncPopResponse(ctx);
        if (resp != NULL) {
            ULONG read_max_length = IoStackIrp->Parameters.Read.Length;

            ULONG header_size = sizeof(resp->header);
            ULONG buffer_size = resp->header.len;
            ULONG read_length = header_size + buffer_size;

            // User-space utility asks for __at most__ `read_max_length` bytes.
            // NOTE: In theory - dp-core dumps messages only up to 4KB and utilities
            // ask for at most 4KB.
            ASSERTMSG("dp-core responds with more than `read_max_length` bytes",
                      read_length > 0 && read_max_length >= read_length);

            RtlCopyMemory(ReadDataBuffer, &resp->header, header_size);
            RtlCopyMemory(ReadDataBuffer + header_size, resp->buffer, buffer_size);
            Irp->IoStatus.Information = read_length;

            KsyncResponseDelete(resp);
        } else {
            // No messages in queue => cannot read anything
            goto empty_read;
        }
    } else {
        goto failure;
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

empty_read:
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

failure:
    Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INSUFFICIENT_RESOURCES;
}

VOID
KsyncDestroyDevice(PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING _DeviceSymLink;

    if (KsyncSymlinkCreated) {
        RtlUnicodeStringInit(&_DeviceSymLink, DeviceSymLink);
        IoDeleteSymbolicLink(&_DeviceSymLink);
        KsyncSymlinkCreated = FALSE;
    }
    if (KsyncDeviceObject != NULL) {
        IoDeleteDevice(KsyncDeviceObject);
        KsyncDeviceObject = NULL;
    }
}

_Use_decl_annotations_ NTSTATUS
KsyncDispatchDeviceControl(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp;
    struct mem_wrapper returnedValue;
    PVOID buffer = NULL;
    struct ksync_device_context *ctx = NULL;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_SIOCTL_METHOD_OUT_DIRECT:

            ctx = irpSp->FileObject->FsContext;

            ctx->user_virtual_address = MmMapLockedPagesSpecifyCache(
                mdl_mem,
                UserMode,
                MmNonCached,
                NULL,
                FALSE,
                NormalPagePriority);
            buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);

            if (!buffer) {
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            returnedValue.pBuffer = ctx->user_virtual_address;
            RtlCopyMemory(buffer, &returnedValue, sizeof(struct mem_wrapper));
            Irp->IoStatus.Information = sizeof(struct mem_wrapper);
            ntStatus = STATUS_SUCCESS;

            break;

        default:

            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

_Use_decl_annotations_ NTSTATUS
KsyncDispatchCleanup(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
KsyncCreateDevice(PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING _DeviceName;
    UNICODE_STRING _DeviceSymLink;
    PDEVICE_OBJECT DeviceObject = NULL;
    NTSTATUS Status;

    Status = RtlUnicodeStringInit(&_DeviceName, DeviceName);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("DeviceName RtlUnicodeStringInit Error: %d\n", Status);
        return Status;
    }

    Status = RtlUnicodeStringInit(&_DeviceSymLink, DeviceSymLink);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("DeviceSymLink RtlUnicodeStringInit Error: %d\n", Status);
        return Status;
    }

    Status = IoCreateDevice(DriverObject, 0, &_DeviceName,
                            FILE_DEVICE_NAMED_PIPE, FILE_DEVICE_SECURE_OPEN,
                            FALSE, &DeviceObject);
    if (NT_SUCCESS(Status))
    {
        KsyncDeviceObject = DeviceObject;

#pragma prefast(push)
#pragma prefast(disable:28175, "we're just setting it, it's allowed")

        DriverObject->MajorFunction[IRP_MJ_CREATE] = KsyncDispatchCreate;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = KsyncDispatchClose;
        DriverObject->MajorFunction[IRP_MJ_WRITE] = KsyncDispatchWrite;
        DriverObject->MajorFunction[IRP_MJ_READ] = KsyncDispatchRead;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = KsyncDispatchDeviceControl;
        DriverObject->MajorFunction[IRP_MJ_CLEANUP] = KsyncDispatchCleanup;
#pragma prefast(pop)

        DeviceObject->Flags |= DO_DIRECT_IO;
        DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

        Status = IoCreateSymbolicLink(&_DeviceSymLink, &_DeviceName);
        if (NT_SUCCESS(Status)) {
            KsyncSymlinkCreated = TRUE;
        } else {
            IoDeleteDevice(DeviceObject);
            KsyncDeviceObject = NULL;
        }
    } else {
        DbgPrint("IoCreateDevice failed. Error code: %d\n", Status);
    }

    return Status;
}

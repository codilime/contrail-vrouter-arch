#include "vr_os.h"
#include "vr_windows.h"
#include "Ntstrsafe.h"
#include "vr_message.h"

const WCHAR DeviceName[] = L"\\Device\\vrouterKsync";
const WCHAR DeviceSymLink[] = L"\\DosDevices\\vrouterKsync";

#define KSYNC_CLEAN_SYMLINK 0b01
#define KSYNC_CLEAN_DEVICE  0b10

#define KSYNC_BUFFER_SIZE (4096)

struct ksync_response {
    unsigned int len;
    unsigned char data[KSYNC_BUFFER_SIZE];
};

#define KSYNC_QUEUE_SIZE (sizeof(struct ksync_response))

static PDEVICE_OBJECT KsyncDeviceObject = NULL;
static int ToClean = 0;

_Dispatch_type_(IRP_MJ_CREATE) DRIVER_DISPATCH KsyncDispatchCreate;
_Dispatch_type_(IRP_MJ_CLOSE) DRIVER_DISPATCH KsyncDispatchClose;
_Dispatch_type_(IRP_MJ_WRITE) DRIVER_DISPATCH KsyncDispatchWrite;
_Dispatch_type_(IRP_MJ_READ) DRIVER_DISPATCH KsyncDispatchRead;

_Use_decl_annotations_ NTSTATUS
KsyncDispatchCreate(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

_Use_decl_annotations_ NTSTATUS
KsyncDispatchClose(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

_Use_decl_annotations_ NTSTATUS
KsyncDispatchWrite(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    int ret;
    struct vr_message request, *response;
    unsigned int len;

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

    request.vr_message_buf = NLA_DATA(p_nlattrr);
    request.vr_message_len = NLA_LEN(p_nlattrr);

    DBG_UNREFERENCED_LOCAL_VARIABLE(p_nlmsghdr);
    DBG_UNREFERENCED_LOCAL_VARIABLE(p_genlmsghdr);

    ret = vr_message_request(&request);
    if (ret < 0) {
        if (vr_send_response(ret))
            return ret;
    }

    while ((response = vr_message_dequeue_response())) {
        len = response->vr_message_len;

        struct ksync_response *resp = (struct ksync_response *)DriverObject->DeviceExtension;

        RtlCopyMemory(&resp->len, &response->vr_message_len, sizeof(resp->len));
        RtlCopyMemory(resp->data, response->vr_message_buf, resp->len);

        vr_message_free(response);
    }
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = IoStackIrp->Parameters.Write.Length;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

_Use_decl_annotations_ NTSTATUS
KsyncDispatchRead(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    PIO_STACK_LOCATION IoStackIrp = NULL;
    PCHAR ReadDataBuffer = NULL;

    IoStackIrp = IoGetCurrentIrpStackLocation(Irp);

    if (Irp->MdlAddress == NULL)
    {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }

    Irp->IoStatus.Information = 0;

    ReadDataBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, LowPagePriority | MdlMappingNoExecute);
    if (ReadDataBuffer != NULL && DriverObject->DeviceExtension != NULL) {
        struct ksync_response *resp = (struct ksync_response *)DriverObject->DeviceExtension;
        ULONG read_max_length = IoStackIrp->Parameters.Read.Length;

        // User-space utility asks for __at most__ `read_max_length` bytes
        if (resp->len > 0 && read_max_length >= resp->len) {
            RtlCopyMemory(ReadDataBuffer, resp->data, resp->len);
            Irp->IoStatus.Information = resp->len;
            resp->len = 0;
        }
    }
    
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

VOID
DestroyDevice(PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING _DeviceSymLink;

    if (ToClean & KSYNC_CLEAN_SYMLINK) {
        RtlUnicodeStringInit(&_DeviceSymLink, DeviceSymLink);
        IoDeleteSymbolicLink(&_DeviceSymLink);
        ToClean ^= KSYNC_CLEAN_SYMLINK;
    }
    if (ToClean & KSYNC_CLEAN_DEVICE) {
        IoDeleteDevice(KsyncDeviceObject);
        KsyncDeviceObject = NULL;
        ToClean ^= KSYNC_CLEAN_DEVICE;
    }
}

NTSTATUS
CreateDevice(PDRIVER_OBJECT DriverObject)
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

    Status = IoCreateDevice(DriverObject, KSYNC_QUEUE_SIZE, &_DeviceName,
        FILE_DEVICE_NAMED_PIPE, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
    if (NT_SUCCESS(Status)) {
        ToClean |= KSYNC_CLEAN_DEVICE;

        KsyncDeviceObject = DeviceObject;

#pragma prefast(push)
#pragma prefast(disable:28175, "we're just setting it, it's allowed")

        DriverObject->MajorFunction[IRP_MJ_CREATE] = KsyncDispatchCreate;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = KsyncDispatchClose;
        DriverObject->MajorFunction[IRP_MJ_WRITE] = KsyncDispatchWrite;
        DriverObject->MajorFunction[IRP_MJ_READ] = KsyncDispatchRead;

#pragma prefast(pop)

        DeviceObject->Flags |= DO_DIRECT_IO;
        DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

        Status = IoCreateSymbolicLink(&_DeviceSymLink, &_DeviceName);
        if (NT_SUCCESS(Status)) {
            ToClean |= KSYNC_CLEAN_SYMLINK;
        } else {
            IoDeleteDevice(DeviceObject);
            KsyncDeviceObject = NULL;
            DeviceObject = NULL;

            ToClean &= ~KSYNC_CLEAN_DEVICE;
        }
    } else {
        DbgPrint("IoCreateDevice failed. Error code: %d\n", Status);
    }

    return Status;
}

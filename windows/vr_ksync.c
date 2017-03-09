#include "vr_os.h"
#include "vr_windows.h"
#include "Ntstrsafe.h"
#include "vr_message.h"

const WCHAR DeviceName[] = L"\\Device\\vrouterKsync";
const WCHAR DeviceSymLink[] = L"\\DosDevices\\vrouterKsync";

#define SYMLINK 1
#define DEVICE 2
#define DATA_BUFFER 4096
#define QUEUE_SIZE (DATA_BUFFER + sizeof(unsigned int))

static int ToClean = 0;

struct ksync_response {
    unsigned int len;
    char data[DATA_BUFFER];
};

NTSTATUS
Create(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
Close(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
Write(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    int ret;
    struct vr_message request, *response;
    unsigned int len;

    PIO_STACK_LOCATION IoStackIrp = NULL;
    PCHAR WriteDataBuffer;
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

        struct ksync_response *ksync_resp = (struct ksync_response*)DriverObject->DeviceExtension;

        RtlCopyMemory(&(ksync_resp->len), &response->vr_message_len, sizeof(unsigned int));
        RtlCopyMemory(ksync_resp->data, response->vr_message_buf, response->vr_message_len);

        vr_message_free(response);
    }
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = IoStackIrp->Parameters.Write.Length;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
Read(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    PIO_STACK_LOCATION IoStackIrp = NULL;
    PWCHAR ReadDataBuffer;

    IoStackIrp = IoGetCurrentIrpStackLocation(Irp);

    if (Irp->MdlAddress == NULL)
    {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }

    ReadDataBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, LowPagePriority | MdlMappingNoExecute);

    Irp->IoStatus.Information = 0;

    if (ReadDataBuffer && DriverObject->DeviceExtension != NULL)
    {
        struct ksync_response *resp = (struct ksync_response*)DriverObject->DeviceExtension;

        if (IoStackIrp->Parameters.Read.Length >= resp->len && resp->len > 0) {
            RtlCopyMemory(ReadDataBuffer, resp->data, resp->len);
            Irp->IoStatus.Information = resp->len;
            resp->len = 0; 
        }
    }
    
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
NotImplemented(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

    DbgPrint("Called a non-implemented function!");

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_NOT_IMPLEMENTED;
}

VOID
DestroyDevice(PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING _DeviceSymLink;

    if (ToClean & SYMLINK)
    {
        RtlUnicodeStringInit(&_DeviceSymLink, DeviceSymLink);
        IoDeleteSymbolicLink(&_DeviceSymLink);
        ToClean ^= SYMLINK;
    }
    if (ToClean & DEVICE)
    {
        IoDeleteDevice(DriverObject->DeviceObject);
        ToClean ^= DEVICE;
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
    
    if (NT_ERROR(Status))
    {
        DbgPrint("DeviceName RtlUnicodeStringInit Error: %d\n", Status);
        return Status;
    }

    Status = RtlUnicodeStringInit(&_DeviceSymLink, DeviceSymLink);

    if (NT_ERROR(Status))
    {
        DbgPrint("DeviceSymLink RtlUnicodeStringInit Error: %d\n", Status);
        return Status;
    }

    Status = IoCreateDevice(DriverObject, QUEUE_SIZE, &_DeviceName, FILE_DEVICE_NAMED_PIPE, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

    if (NT_SUCCESS(Status))
    {
        ToClean |= DEVICE;
        for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
            DriverObject->MajorFunction[i] = NotImplemented;

        DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;
        DriverObject->MajorFunction[IRP_MJ_WRITE] = Write;
        DriverObject->MajorFunction[IRP_MJ_READ] = Read;

        DeviceObject->Flags |= DO_DIRECT_IO;
        DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

        Status = IoCreateSymbolicLink(&_DeviceSymLink, &_DeviceName);

        if (NT_WARNING(Status) || NT_INFORMATION(Status) || NT_SUCCESS(Status))
        {
            DbgPrint("IoCreateSymbolicLink %d\n", Status);
            ToClean |= SYMLINK;
        }
        return Status;
    }
    else
    {
        DbgPrint("IoCreateDevice failed. Error code: %d\n", Status);
    }

    return Status;
}

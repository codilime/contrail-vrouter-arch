#include "vr_os.h"
#include "vr_windows.h"
#include "precomp.h"
#include "Ntstrsafe.h"

const WCHAR DeviceName[] = L"\\Device\\vRouter\\ksync";
const WCHAR DeviceSymLink[] = L"\\DosDevices\\vRouter\\ksync";

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

// TODO: JW-168
// This structure is from linux kernel headers. It will be used to cast data from userspace.
struct nlmsghdr {
    UINT32           nlmsg_len;      /* Length of message including header */
    UINT16           nlmsg_type;     /* Message content */
    UINT16           nlmsg_flags;    /* Additional flags */
    UINT32           nlmsg_seq;      /* Sequence number */
    UINT32           nlmsg_pid;      /* Sending process port ID */
};

NTSTATUS
Write(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DriverObject);

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

    WriteDataBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, LowPagePriority);

    if (!WriteDataBuffer) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
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

    ReadDataBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, LowPagePriority);
    // TODO JW-167 Create connection between userspace and kernel space on windows
    /*
    if (ReadDataBuffer && IoStackIrp->Parameters.Read.Length >= BufferSize)
    {
        // RtlCopyMemory(ReadDataBuffer, ReadData, BufferSize);
    }
    */
  
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = IoStackIrp->Parameters.Write.Length;
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
    IoDeleteDevice(DriverObject->DeviceObject);
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

    Status = IoCreateDevice(DriverObject, 0, &_DeviceName, FILE_DEVICE_NAMED_PIPE, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

    if (NT_SUCCESS(Status))
    {
        for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
            DriverObject->MajorFunction[i] = NotImplemented;

        DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;
        DriverObject->MajorFunction[IRP_MJ_WRITE] = Write;
        DriverObject->MajorFunction[IRP_MJ_READ] = Read;

        DeviceObject->Flags |= DO_DIRECT_IO;
        DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

        Status = IoCreateSymbolicLink(&_DeviceSymLink, &_DeviceName);

        if (NT_WARNING(Status) || NT_INFORMATION(Status) || NT_ERROR(Status))
        {
            DbgPrint("IoCreateSymbolicLink %d\n", Status);
        }
        return Status;
    }
    else
    {
        DbgPrint("IoCreateDevice failed. Error code: %d\n", Status);
    }

    return Status;
}

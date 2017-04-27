#include "vr_devices.h"
#include <ntstrsafe.h>

static const WCHAR Pkt0DeviceName[] = L"\\Device\\vrouterPkt0";
static const WCHAR Pkt0DeviceSymLink[] = L"\\DosDevices\\vrouterPkt0";

static PDEVICE_OBJECT Pkt0DeviceObject = NULL;
static BOOLEAN Pkt0SymlinkCreated = FALSE;

static NTSTATUS
Pkt0DispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

static NTSTATUS
Pkt0DispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

static NTSTATUS
Pkt0DispatchCleanup(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

static NTSTATUS
Pkt0DispatchWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

static NTSTATUS
Pkt0DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


static NTSTATUS
Pkt0DispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    /* TODO(sodar): Implement */

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
Pkt0CreateDevice(PDRIVER_OBJECT DriverObject)
{
    struct vr_device_dispatch_callbacks callbacks = {
        .create         = Pkt0DispatchCreate,
        .close          = Pkt0DispatchClose,
        .cleanup        = Pkt0DispatchCleanup,
        .write          = Pkt0DispatchWrite,
        .read           = Pkt0DispatchRead,
        .device_control = Pkt0DispatchDeviceControl,
    };

    return VRouterSetUpNamedPipeServer(DriverObject,
                                       Pkt0DeviceName,
                                       Pkt0DeviceSymLink,
                                       &callbacks,
                                       &Pkt0DeviceObject,
                                       &Pkt0SymlinkCreated);
}

VOID
Pkt0DestroyDevice(PDRIVER_OBJECT DriverObject)
{
    VRouterTearDownNamedPipeServer(DriverObject,
                                   Pkt0DeviceSymLink,
                                   &Pkt0DeviceObject,
                                   &Pkt0SymlinkCreated);
}

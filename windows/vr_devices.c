#include <Wdm.h>
#include <Ntstrsafe.h>
#include "vr_devices.h"

#define DEVICE_CTX_CALLBACKS(Device) ( (struct vr_device_dispatch_callbacks *)((Device)->DeviceExtension) )

_Dispatch_type_(IRP_MJ_CREATE) DRIVER_DISPATCH VRouterDispatchCreate;
_Dispatch_type_(IRP_MJ_CLOSE) DRIVER_DISPATCH VRouterDispatchClose;
_Dispatch_type_(IRP_MJ_CLEANUP) DRIVER_DISPATCH VRouterDispatchCleanup;
_Dispatch_type_(IRP_MJ_WRITE) DRIVER_DISPATCH VRouterDispatchWrite;
_Dispatch_type_(IRP_MJ_READ) DRIVER_DISPATCH VRouterDispatchRead;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH VRouterDispatchDeviceControl;

_Use_decl_annotations_ NTSTATUS
VRouterDispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    return DEVICE_CTX_CALLBACKS(DeviceObject)->create(DeviceObject, Irp);
}

_Use_decl_annotations_ NTSTATUS
VRouterDispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    return DEVICE_CTX_CALLBACKS(DeviceObject)->close(DeviceObject, Irp);
}

_Use_decl_annotations_ NTSTATUS
VRouterDispatchCleanup(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    return DEVICE_CTX_CALLBACKS(DeviceObject)->cleanup(DeviceObject, Irp);
}

_Use_decl_annotations_ NTSTATUS
VRouterDispatchWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    return DEVICE_CTX_CALLBACKS(DeviceObject)->write(DeviceObject, Irp);
}

_Use_decl_annotations_ NTSTATUS
VRouterDispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    return DEVICE_CTX_CALLBACKS(DeviceObject)->read(DeviceObject, Irp);
}

_Use_decl_annotations_ NTSTATUS
VRouterDispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    return DEVICE_CTX_CALLBACKS(DeviceObject)->device_control(DeviceObject, Irp);
}

NTSTATUS
VRouterInitializeDevices(PDRIVER_OBJECT DriverObject)
{
#pragma prefast(push)
#pragma prefast(disable:28175, "we're just setting it, it's allowed")
    DriverObject->MajorFunction[IRP_MJ_CREATE] = VRouterDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = VRouterDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = VRouterDispatchCleanup;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = VRouterDispatchWrite;
    DriverObject->MajorFunction[IRP_MJ_READ] = VRouterDispatchRead;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = VRouterDispatchDeviceControl;
#pragma prefast(pop)

    return STATUS_SUCCESS;
}

VOID
VRouterUninitializeDevices(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
}

NTSTATUS
VRouterSetUpNamedPipeServer(_In_ PDRIVER_OBJECT DriverObject,
                            _In_ PCWSTR DeviceName,
                            _In_ PCWSTR DeviceSymlink,
                            _In_ struct vr_device_dispatch_callbacks *Callbacks,
                            _Out_ PDEVICE_OBJECT *DeviceObject,
                            _Out_ PBOOLEAN SymlinkCreated)
{
    UNICODE_STRING _DeviceName;
    UNICODE_STRING _DeviceSymlink;
    PDEVICE_OBJECT _DeviceObject = NULL;
    struct vr_device_dispatch_callbacks *_Callbacks = NULL;
    NTSTATUS Status;

    ASSERT(Callbacks->create != NULL);
    ASSERT(Callbacks->close != NULL);
    ASSERT(Callbacks->cleanup != NULL);
    ASSERT(Callbacks->write != NULL);
    ASSERT(Callbacks->read != NULL);
    ASSERT(Callbacks->device_control != NULL);

    Status = RtlUnicodeStringInit(&_DeviceName, DeviceName);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = RtlUnicodeStringInit(&_DeviceSymlink, DeviceSymlink);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = IoCreateDevice(DriverObject, sizeof(struct vr_device_dispatch_callbacks), &_DeviceName,
                            FILE_DEVICE_NAMED_PIPE, FILE_DEVICE_SECURE_OPEN,
                            FALSE, &_DeviceObject);
    if (NT_SUCCESS(Status)) {
        *DeviceObject = _DeviceObject;

        _Callbacks = (struct vr_device_dispatch_callbacks *)(*DeviceObject)->DeviceExtension;
        RtlCopyMemory(_Callbacks, Callbacks, sizeof(struct vr_device_dispatch_callbacks));

        (*DeviceObject)->Flags |= DO_DIRECT_IO;
        (*DeviceObject)->Flags &= (~DO_DEVICE_INITIALIZING);

        Status = IoCreateSymbolicLink(&_DeviceSymlink, &_DeviceName);
        if (NT_SUCCESS(Status)) {
            *SymlinkCreated = TRUE;
        } else {
            goto Failure;
        }
    } else {
        goto Failure;
    }

    return STATUS_SUCCESS;

Failure:
    if (*DeviceObject) {
        IoDeleteDevice(*DeviceObject);
        *DeviceObject = NULL;
    }

    return Status;
}

VOID
VRouterTearDownNamedPipeServer(_In_ PDRIVER_OBJECT DriverObject,
                               _In_ PCWSTR DeviceSymlink,
                               _Inout_ PDEVICE_OBJECT *DeviceObject,
                               _Inout_ PBOOLEAN SymlinkCreated)
{
    UNICODE_STRING _DeviceSymLink;

    if (*SymlinkCreated) {
        RtlUnicodeStringInit(&_DeviceSymLink, DeviceSymlink);
        IoDeleteSymbolicLink(&_DeviceSymLink);
        *SymlinkCreated = FALSE;
    }
    if (*DeviceObject != NULL) {
        IoDeleteDevice(*DeviceObject);
        *DeviceObject = NULL;
    }
}

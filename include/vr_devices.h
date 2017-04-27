#ifndef __VR_DEVICES_H__
#define __VR_DEVICES_H__

#include "vr_os.h"
#include "vr_windows.h"

struct vr_device_dispatch_callbacks {
    PDRIVER_DISPATCH create;
    PDRIVER_DISPATCH close;
    PDRIVER_DISPATCH cleanup;
    PDRIVER_DISPATCH write;
    PDRIVER_DISPATCH read;
    PDRIVER_DISPATCH device_control;
};

NTSTATUS KsyncCreateDevice(PDRIVER_OBJECT DriverObject);
VOID KsyncDestroyDevice(PDRIVER_OBJECT DriverObject);

NTSTATUS Pkt0CreateDevice(PDRIVER_OBJECT DriverObject);
VOID Pkt0DestroyDevice(PDRIVER_OBJECT DriverObject);

NTSTATUS VRouterInitializeDevices(PDRIVER_OBJECT DriverObject);
VOID VRouterUninitializeDevices(PDRIVER_OBJECT DriverObject);

NTSTATUS VRouterSetUpNamedPipeServer(_In_ PDRIVER_OBJECT DriverObject,
                                     _In_ PCWSTR DeviceName,
                                     _In_ PCWSTR DeviceSymlink,
                                     _In_ struct vr_device_dispatch_callbacks *Callbacks,
                                     _Out_ PDEVICE_OBJECT *DeviceObject,
                                     _Out_ PBOOLEAN SymlinkCreated);
VOID VRouterTearDownNamedPipeServer(_In_ PDRIVER_OBJECT DriverObject,
                                    _In_ PCWSTR DeviceSymlink,
                                    _Inout_ PDEVICE_OBJECT *DeviceObject,
                                    _Inout_ PBOOLEAN SymlinkCreated);

#endif /* __VR_DEVICES_H__ */

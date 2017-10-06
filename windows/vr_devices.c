#include <Wdm.h>
#include <wdmsec.h>
#include <Ntstrsafe.h>
#include "windows_devices.h"


NTSTATUS
VRouterSetUpNamedDevice(NDIS_HANDLE DriverHandle,
                        PCWSTR DeviceName,
                        PCWSTR DeviceSymlink,
                        PDRIVER_DISPATCH *DispatchTable,
                        PDEVICE_OBJECT *DeviceObject,
                        NDIS_HANDLE *DeviceHandle)
{
    NTSTATUS status;
    UNICODE_STRING device_name;
    UNICODE_STRING device_symlink;

    status = RtlUnicodeStringInit(&device_name, DeviceName);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = RtlUnicodeStringInit(&device_symlink, DeviceSymlink);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    NDIS_DEVICE_OBJECT_ATTRIBUTES attributes;
    NdisZeroMemory(&attributes, sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES));

    attributes.Header.Type = NDIS_OBJECT_TYPE_DEVICE_OBJECT_ATTRIBUTES;
    attributes.Header.Revision = NDIS_DEVICE_OBJECT_ATTRIBUTES_REVISION_1;
    attributes.Header.Size = NDIS_SIZEOF_DEVICE_OBJECT_ATTRIBUTES_REVISION_1;

    attributes.DeviceName = &device_name;
    attributes.SymbolicName = &device_symlink;
    attributes.MajorFunctions = &DispatchTable[0];
    attributes.ExtensionSize = 0;
    attributes.DefaultSDDLString = &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RWX_RES_RWX;
    attributes.DeviceClassGuid = NULL;

    status = NdisRegisterDeviceEx(DriverHandle, &attributes, DeviceObject, DeviceHandle);
    if (NT_SUCCESS(status)) {
        (*DeviceObject)->Flags |= DO_DIRECT_IO;
    }

    return status;
}

VOID
VRouterTearDownNamedDevice(NDIS_HANDLE *DeviceHandle)
{
    if (*DeviceHandle == NULL) {
        return;
    }

    NdisDeregisterDeviceEx(*DeviceHandle);
    *DeviceHandle = NULL;
}

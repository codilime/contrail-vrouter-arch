/*
 * windows_devices.h -- definitions used in pipe handling on Windows
 *
 * Copyright (c) 2017 Juniper Networks, Inc. All rights reserved.
 */
#ifndef __WINDOWS_DEVICES_H__
#define __WINDOWS_DEVICES_H__

#include "vr_os.h"
#include "vr_windows.h"

struct _VR_DEVICE_DISPATCH_CALLBACKS {
    PDRIVER_DISPATCH create;
    PDRIVER_DISPATCH close;
    PDRIVER_DISPATCH cleanup;
    PDRIVER_DISPATCH write;
    PDRIVER_DISPATCH read;
    PDRIVER_DISPATCH device_control;
};

typedef struct _VR_DEVICE_DISPATCH_CALLBACKS VR_DEVICE_DISPATCH_CALLBACKS;
typedef struct _VR_DEVICE_DISPATCH_CALLBACKS *PVR_DEVICE_DISPATCH_CALLBACKS;

struct _VR_DEVICE_CONTEXT {
    VR_DEVICE_DISPATCH_CALLBACKS callbacks;
    void *private_data;
};

typedef struct _VR_DEVICE_CONTEXT VR_DEVICE_CONTEXT;
typedef struct _VR_DEVICE_CONTEXT *PVR_DEVICE_CONTEXT;

NTSTATUS KsyncCreateDevice(PDRIVER_OBJECT DriverObject);
VOID KsyncDestroyDevice(PDRIVER_OBJECT DriverObject);

NTSTATUS Pkt0CreateDevice(PDRIVER_OBJECT DriverObject);
VOID Pkt0DestroyDevice(PDRIVER_OBJECT DriverObject);

NTSTATUS FlowCreateDevice(PDRIVER_OBJECT DriverObject);
VOID FlowDestroyDevice(PDRIVER_OBJECT DriverObject);

NTSTATUS VRouterInitializeDevices(PDRIVER_OBJECT DriverObject);
VOID VRouterUninitializeDevices(PDRIVER_OBJECT DriverObject);

NTSTATUS VRouterSetUpNamedPipeServer(_In_ PDRIVER_OBJECT DriverObject,
                                     _In_ PCWSTR DeviceName,
                                     _In_ PCWSTR DeviceSymlink,
                                     _In_ PVR_DEVICE_DISPATCH_CALLBACKS Callbacks,
                                     _Out_ PDEVICE_OBJECT *DeviceObject,
                                     _Out_ PBOOLEAN SymlinkCreated);
VOID VRouterTearDownNamedPipeServer(_In_ PDRIVER_OBJECT DriverObject,
                                    _In_ PCWSTR DeviceSymlink,
                                    _Inout_ PDEVICE_OBJECT *DeviceObject,
                                    _Inout_ PBOOLEAN SymlinkCreated);

VOID VRouterAttachPrivateData(_Inout_ PDEVICE_OBJECT DeviceObject,
                              _In_ PVOID Data);
PVOID VRouterGetPrivateData(_In_ PDEVICE_OBJECT DeviceObject);

/*
 * Pkt0 related definitions
 */
struct pkt0_packet {
    uint8_t *buffer;
    size_t length;
    LIST_ENTRY list_entry;
};

int pkt0_if_tx(struct vr_interface *vif, struct vr_packet *pkt);

/*
 * Flow device related definitions
 */
struct _FLOW_DEVICE_CONTEXT {
    PVOID UserVirtualAddress;
    PMDL FlowMemoryMdl;
};

typedef struct _FLOW_DEVICE_CONTEXT   FLOW_DEVICE_CONTEXT;
typedef struct _FLOW_DEVICE_CONTEXT *PFLOW_DEVICE_CONTEXT;

#endif /* __WINDOWS_DEVICES_H__ */

#include "precomp.h"
#include "vr_windows.h"
#include "windows_devices.h"
#include "windows_mem.h"

#include "vrouter.h"
#include "vr_packet.h"
#include "vr_sandesh.h"

static const PWSTR FriendlyName = L"OpenContrail's vRouter forwarding extension";
static const PWSTR UniqueName = L"{56553588-1538-4BE6-B8E0-CB46402DC205}";
static const PWSTR ServiceName = L"vRouter";

static PDRIVER_OBJECT VrDriverObject = NULL;
static NDIS_HANDLE VrDriverHandle = NULL;

const ULONG VrAllocationTag = 'RVCO';
const ULONG VrOidRequestId = 'RVCO';

PSWITCH_OBJECT VrSwitchObject = NULL;
NDIS_HANDLE VrNBLPool = NULL;

/* Read/write lock which must be acquired by deferred callbacks. Used in functions from
* `host_os` struct.
*/
PNDIS_RW_LOCK_EX AsyncWorkRWLock = NULL;

unsigned int vr_num_cpus;
int vrouter_dbg = 0;

/*
    NDIS Function prototypes
*/
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

FILTER_ATTACH FilterAttach;
FILTER_DETACH FilterDetach;
FILTER_PAUSE FilterPause;
FILTER_RESTART FilterRestart;

/* Functions for NBL handling are located in windows/vr_nbl.c */
extern FILTER_SEND_NET_BUFFER_LISTS FilterSendNetBufferLists;
extern FILTER_SEND_NET_BUFFER_LISTS_COMPLETE FilterSendNetBufferListsComplete;

/* Functions for OID request handling are located in windows/vr_oid.c */
extern FILTER_OID_REQUEST FilterOidRequest;
extern FILTER_OID_REQUEST_COMPLETE FilterOidRequestComplete;
extern FILTER_CANCEL_OID_REQUEST FilterCancelOidRequest;

NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NDIS_STATUS status;
    NDIS_FILTER_DRIVER_CHARACTERISTICS f_chars;
    NDIS_STRING service_name;
    NDIS_STRING friendly_name;
    NDIS_STRING unique_name;

    UNREFERENCED_PARAMETER(RegistryPath);

    RtlInitUnicodeString(&service_name, ServiceName);
    RtlInitUnicodeString(&friendly_name, FriendlyName);
    RtlInitUnicodeString(&unique_name, UniqueName);

    VrDriverObject = DriverObject;
    VrDriverObject->DriverUnload = DriverUnload;

    NdisZeroMemory(&f_chars, sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS));
    f_chars.Header.Type = NDIS_OBJECT_TYPE_FILTER_DRIVER_CHARACTERISTICS;
    f_chars.Header.Size = NDIS_SIZEOF_FILTER_DRIVER_CHARACTERISTICS_REVISION_2;
    f_chars.Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_2;

    f_chars.MajorNdisVersion = NDIS_FILTER_MAJOR_VERSION;
    f_chars.MinorNdisVersion = NDIS_FILTER_MINOR_VERSION;

    f_chars.MajorDriverVersion = 1;
    f_chars.MinorDriverVersion = 0;
    f_chars.Flags = 0;

    f_chars.FriendlyName = friendly_name;
    f_chars.UniqueName = unique_name;
    f_chars.ServiceName = service_name;

    f_chars.AttachHandler = FilterAttach;
    f_chars.DetachHandler = FilterDetach;
    f_chars.PauseHandler = FilterPause;
    f_chars.RestartHandler = FilterRestart;

    f_chars.SendNetBufferListsHandler = FilterSendNetBufferLists;
    f_chars.SendNetBufferListsCompleteHandler = FilterSendNetBufferListsComplete;

    f_chars.OidRequestHandler = FilterOidRequest;
    f_chars.OidRequestCompleteHandler = FilterOidRequestComplete;
    f_chars.CancelOidRequestHandler = FilterCancelOidRequest;

    status = NdisFRegisterFilterDriver(DriverObject,
                                       (NDIS_HANDLE)VrDriverObject,
                                       &f_chars,
                                       &VrDriverHandle);

    if (status != NDIS_STATUS_SUCCESS)
    {
        if (VrDriverHandle != NULL)
        {
            NdisFDeregisterFilterDriver(VrDriverHandle);
            VrDriverHandle = NULL;
        }
    }

    return status;
}

VOID
DriverUnload(PDRIVER_OBJECT DriverObject)
{
    NdisFDeregisterFilterDriver(VrDriverHandle);
}

/* TODO(WINDOWS): vRouter cleanup */
static void
vr_message_exit(void)
{
    vr_transport_exit();
    vr_sandesh_exit();
}

/* TODO(WINDOWS): vRouter cleanup */
static NTSTATUS
vr_message_init(void)
{
    int ret = vr_sandesh_init();
    if (ret) {
        DbgPrint("%s: vr_sandesh_init() failed with return %d\n", __func__, ret);
        return NDIS_STATUS_FAILURE;
    }

    ret = vr_transport_init();
    if (ret) {
        DbgPrint("%s: vr_transport_init() failed with return %d", __func__, ret);
        vr_sandesh_exit();
        return NDIS_STATUS_FAILURE;
    }

    return NDIS_STATUS_SUCCESS;
}

static VOID
UninitializeVRouter(pvr_switch_context ctx)
{
    if (ctx->vrouter_up)
        vrouter_exit(false);

    if (ctx->message_up)
        vr_message_exit();

    if (ctx->memory_up)
        memory_exit();

    if (ctx->device_up)
        VRouterUninitializeDevices(VrDriverObject);

    if (ctx->pkt0_up)
        Pkt0DestroyDevice(VrDriverObject);

    if (ctx->ksync_up)
        KsyncDestroyDevice(VrDriverObject);
}

static VOID
UninitializeWindowsComponents(pvr_switch_context ctx)
{
    if (VrNBLPool)
        vrouter_free_pool(VrNBLPool);

    if (AsyncWorkRWLock)
        NdisFreeRWLock(AsyncWorkRWLock);

    if (ctx)
    {
        if (ctx->lock)
            NdisFreeRWLock(ctx->lock);

        ExFreePoolWithTag(ctx, VrAllocationTag);
    }
}

static NDIS_STATUS
InitializeVRouter(pvr_switch_context ctx)
{
    ASSERT(!ctx->ksync_up);
    ASSERT(!ctx->pkt0_up);
    ASSERT(!ctx->device_up);
    ASSERT(!ctx->memory_up);
    ASSERT(!ctx->message_up);
    ASSERT(!ctx->vrouter_up);

    ctx->ksync_up = NT_SUCCESS(KsyncCreateDevice(VrDriverObject));

    if (!ctx->ksync_up)
        goto cleanup;

    ctx->pkt0_up = NT_SUCCESS(Pkt0CreateDevice(VrDriverObject));

    if (!ctx->pkt0_up)
        goto cleanup;

    ctx->device_up = NT_SUCCESS(VRouterInitializeDevices(VrDriverObject));

    if (!ctx->device_up)
        goto cleanup;

    ctx->memory_up = NT_SUCCESS(memory_init());

    if (!ctx->memory_up)
        goto cleanup;

    ctx->message_up = !vr_message_init();

    if (!ctx->message_up)
        goto cleanup;

    ctx->vrouter_up = !vrouter_init();

    if (!ctx->vrouter_up)
        goto cleanup;

    return NDIS_STATUS_SUCCESS;

cleanup:
    UninitializeVRouter(ctx);
    return NDIS_STATUS_FAILURE;
}

static NDIS_STATUS
InitializeWindowsComponents(PSWITCH_OBJECT Switch)
{
    pvr_switch_context ctx = NULL;

    ctx = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(vr_switch_context), VrAllocationTag);
    if (ctx == NULL)
        return NDIS_STATUS_RESOURCES;

    RtlZeroMemory(ctx, sizeof(vr_switch_context));

    ctx->lock = NdisAllocateRWLock(Switch->NdisFilterHandle);
    if (ctx->lock == NULL)
        goto cleanup;

    AsyncWorkRWLock = NdisAllocateRWLock(Switch->NdisFilterHandle);
    if (AsyncWorkRWLock == NULL)
        goto cleanup;

    VrNBLPool = vrouter_generate_pool();
    if (VrNBLPool == NULL)
        goto cleanup;

    Switch->ExtensionContext = ctx;

    return NDIS_STATUS_SUCCESS;

cleanup:
    UninitializeWindowsComponents(ctx);

    return NDIS_STATUS_FAILURE;
}

static NDIS_STATUS
CreateSwitch(PSWITCH_OBJECT Switch)
{
    DbgPrint("CreateSwitch\r\n");

    if (VrSwitchObject != NULL)
        return NDIS_STATUS_FAILURE;

    vr_num_cpus = KeQueryActiveProcessorCount(NULL);
    if (!vr_num_cpus) {
        DbgPrint("%s: Failed to get processor count\n", __func__);
        return NDIS_STATUS_FAILURE;
    }

    VrSwitchObject = Switch;

    BOOLEAN windows = FALSE;
    BOOLEAN vrouter = FALSE;

    NDIS_STATUS status = InitializeWindowsComponents(Switch);

    if (!NT_SUCCESS(status))
        goto cleanup;

    windows = TRUE;

    status = InitializeVRouter(Switch->ExtensionContext);

    if (!NT_SUCCESS(status))
        goto cleanup;

    vrouter = TRUE;

    return NDIS_STATUS_SUCCESS;

cleanup:
    if (vrouter)
        UninitializeVRouter(Switch->ExtensionContext);

    if (windows)
        UninitializeWindowsComponents(Switch->ExtensionContext);

    VrSwitchObject = NULL;

    return NDIS_STATUS_FAILURE;
}

NDIS_STATUS
FilterAttach(NDIS_HANDLE NdisFilterHandle,
             NDIS_HANDLE DriverContext,
             PNDIS_FILTER_ATTACH_PARAMETERS AttachParameters)
{
    NDIS_STATUS status;
    NDIS_FILTER_ATTRIBUTES filterAttributes;
    ULONG switchObjectSize;
    NDIS_SWITCH_CONTEXT switchContext;
    NDIS_SWITCH_OPTIONAL_HANDLERS switchHandler;
    PSWITCH_OBJECT switchObject;

    UNREFERENCED_PARAMETER(DriverContext);

    DbgPrint("%s: NdisFilterHandle %p\r\n", __func__, NdisFilterHandle);

    status = NDIS_STATUS_SUCCESS;
    switchObject = NULL;
    switchObjectSize = sizeof(SWITCH_OBJECT);

    NT_ASSERT(DriverContext == (NDIS_HANDLE)VrDriverObject);

    // Accept Ethernet only
    if (AttachParameters->MiniportMediaType != NdisMedium802_3)
    {
        status = NDIS_STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    switchHandler.Header.Type = NDIS_OBJECT_TYPE_SWITCH_OPTIONAL_HANDLERS;
    switchHandler.Header.Size = NDIS_SIZEOF_SWITCH_OPTIONAL_HANDLERS_REVISION_1;
    switchHandler.Header.Revision = NDIS_SWITCH_OPTIONAL_HANDLERS_REVISION_1;

    status = NdisFGetOptionalSwitchHandlers(NdisFilterHandle, &switchContext, &switchHandler);
    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("%s: Extension is not bound to the underlying extensible switch component.\r\n", __func__);
        goto Cleanup;
    }

    switchObject = ExAllocatePoolWithTag(NonPagedPoolNx, switchObjectSize, VrAllocationTag);
    if (switchObject == NULL)
    {
        status = NDIS_STATUS_RESOURCES;
        goto Cleanup;
    }

    RtlZeroMemory(switchObject, switchObjectSize);

    // Initialize NDIS related information.
    switchObject->NdisFilterHandle = NdisFilterHandle;
    switchObject->NdisSwitchContext = switchContext;
    switchObject->NdisSwitchHandlers = switchHandler;

    status = CreateSwitch(switchObject);
    if (status != NDIS_STATUS_SUCCESS)
    {
        goto Cleanup;
    }

    filterAttributes.Header.Revision = NDIS_FILTER_ATTRIBUTES_REVISION_1;
    filterAttributes.Header.Size = NDIS_SIZEOF_FILTER_ATTRIBUTES_REVISION_1;
    filterAttributes.Header.Type = NDIS_OBJECT_TYPE_FILTER_ATTRIBUTES;
    filterAttributes.Flags = 0;

    NDIS_DECLARE_FILTER_MODULE_CONTEXT(SWITCH_OBJECT);
    status = NdisFSetAttributes(NdisFilterHandle, switchObject, &filterAttributes);
    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("%s: Failed to set attributes.\r\n", __func__);
        goto Cleanup;
    }

    switchObject->Running = FALSE;

Cleanup:

    if (status != NDIS_STATUS_SUCCESS)
    {
        if (switchObject != NULL)
        {
            ExFreePool(switchObject);
        }
    }

    return status;
}

VOID
FilterDetach(NDIS_HANDLE FilterModuleContext)
{
    PSWITCH_OBJECT switchObject = (PSWITCH_OBJECT)FilterModuleContext;

    DbgPrint("%s: FilterModuleContext %p\r\n", __func__, FilterModuleContext);

    KeMemoryBarrier();

    while(switchObject->PendingOidCount > 0)
    {
        NdisMSleep(1000);
    }

    ASSERTMSG("Trying to delete another switch than currently active", switchObject == VrSwitchObject);

    UninitializeVRouter(switchObject->ExtensionContext);
    UninitializeWindowsComponents(switchObject->ExtensionContext);

    VrSwitchObject = NULL;

    ExFreePool(switchObject);
}

NDIS_STATUS
FilterPause(NDIS_HANDLE FilterModuleContext, PNDIS_FILTER_PAUSE_PARAMETERS PauseParameters)
{
    PSWITCH_OBJECT switchObject = (PSWITCH_OBJECT)(FilterModuleContext);

    UNREFERENCED_PARAMETER(PauseParameters);

    DbgPrint("%s: FilterModuleContext %p\r\n", __func__, FilterModuleContext);

    switchObject->Running = FALSE;

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
FilterRestart(NDIS_HANDLE FilterModuleContext, PNDIS_FILTER_RESTART_PARAMETERS RestartParameters)
{
    PSWITCH_OBJECT switchObject = (PSWITCH_OBJECT)FilterModuleContext;

    UNREFERENCED_PARAMETER(RestartParameters);

    DbgPrint("%s: FilterModuleContext %p\n", __func__, FilterModuleContext);

    switchObject->Running = TRUE;

    return NDIS_STATUS_SUCCESS;
}

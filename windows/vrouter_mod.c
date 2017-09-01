#include "precomp.h"
#include "vr_windows.h"
#include "windows_devices.h"
#include "vrouter.h"
#include "vr_packet.h"
#include "vr_sandesh.h"
#include "windows_mem.h"

static const PWSTR FriendlyName = L"OpenContrail's vRouter forwarding extension";
static const PWSTR UniqueName = L"{56553588-1538-4BE6-B8E0-CB46402DC205}";
static const PWSTR ServiceName = L"vRouter";

ULONG  SxExtAllocationTag = 'RVCO';
ULONG  SxExtOidRequestId = 'RVCO';

static NDIS_HANDLE DriverHandle = NULL;
PSX_SWITCH_OBJECT SxSwitchObject = NULL;
NDIS_HANDLE SxNBLPool = NULL;

NDIS_SPIN_LOCK SxExtensionListLock;
LIST_ENTRY SxExtensionList;

// This is exported by SxLibrary
extern PDRIVER_OBJECT SxDriverObject;

unsigned int vr_num_cpus;
int vrouter_dbg = 0;

/* Read/write lock which must be acquired by deferred callbacks. Used in functions from
* `host_os` struct.
*/
PNDIS_RW_LOCK_EX AsyncWorkRWLock = NULL;

static char hex_table[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

extern int vr_transport_init(void);
extern void vr_transport_exit(void);

// NDIS Function prototypes
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

FILTER_ATTACH FilterAttach;
FILTER_DETACH FilterDetach;
FILTER_PAUSE FilterPause;
FILTER_RESTART FilterRestart;

FILTER_SEND_NET_BUFFER_LISTS FilterSendNetBufferLists;
FILTER_SEND_NET_BUFFER_LISTS_COMPLETE FilterSendNetBufferListsComplete;
FILTER_CANCEL_SEND_NET_BUFFER_LISTS FilterCancelSendNetBufferLists;

FILTER_RECEIVE_NET_BUFFER_LISTS FilterReceiveNetBufferLists;
FILTER_RETURN_NET_BUFFER_LISTS FilterReturnNetBufferLists;

FILTER_OID_REQUEST FilterOidRequest;
FILTER_OID_REQUEST_COMPLETE FilterOidRequestComplete;
FILTER_CANCEL_OID_REQUEST FilterCancelOidRequest;

FILTER_STATUS FilterStatus;


NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NDIS_STATUS status;
    NDIS_FILTER_DRIVER_CHARACTERISTICS fChars;
    NDIS_STRING service_name;
    NDIS_STRING friendly_name;
    NDIS_STRING unique_name;

    UNREFERENCED_PARAMETER(RegistryPath);

    RtlInitUnicodeString(&service_name, ServiceName);
    RtlInitUnicodeString(&friendly_name, FriendlyName);
    RtlInitUnicodeString(&unique_name, UniqueName);
    SxDriverObject = DriverObject;

    NdisZeroMemory(&fChars, sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS));
    fChars.Header.Type = NDIS_OBJECT_TYPE_FILTER_DRIVER_CHARACTERISTICS;
    fChars.Header.Size = sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS);
    fChars.Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_2;

    fChars.MajorNdisVersion = NDIS_FILTER_MAJOR_VERSION;
    fChars.MinorNdisVersion = NDIS_FILTER_MINOR_VERSION;

    fChars.MajorDriverVersion = 1;
    fChars.MinorDriverVersion = 0;
    fChars.Flags = 0;

    fChars.FriendlyName = friendly_name;
    fChars.UniqueName = unique_name;
    fChars.ServiceName = service_name;

    fChars.AttachHandler = FilterAttach;
    fChars.DetachHandler = FilterDetach;
    fChars.PauseHandler = FilterPause;
    fChars.RestartHandler = FilterRestart;

    fChars.SendNetBufferListsHandler = FilterSendNetBufferLists;
    fChars.SendNetBufferListsCompleteHandler = FilterSendNetBufferListsComplete;
    fChars.CancelSendNetBufferListsHandler = FilterCancelSendNetBufferLists;

    fChars.ReceiveNetBufferListsHandler = FilterReceiveNetBufferLists;
    fChars.ReturnNetBufferListsHandler = FilterReturnNetBufferLists;

    fChars.OidRequestHandler = FilterOidRequest;
    fChars.OidRequestCompleteHandler = FilterOidRequestComplete;
    fChars.CancelOidRequestHandler = FilterCancelOidRequest;

    fChars.StatusHandler = FilterStatus;

    NdisAllocateSpinLock(&SxExtensionListLock);
    InitializeListHead(&SxExtensionList);

    DriverObject->DriverUnload = DriverUnload;

    status = NdisFRegisterFilterDriver(DriverObject,
                                       (NDIS_HANDLE)SxDriverObject,
                                       &fChars,
                                       &DriverHandle);

    if (status != NDIS_STATUS_SUCCESS)
    {
        if (DriverHandle != NULL)
        {
            NdisFDeregisterFilterDriver(DriverHandle);
            DriverHandle = NULL;
        }

        NdisFreeSpinLock(&SxExtensionListLock);
    }

    return status;
}

static void
vr_message_exit(void)
{
    vr_transport_exit();
    vr_sandesh_exit();
}

void
SxExtUninitializeVRouter(struct vr_switch_context* ctx)
{
    if (ctx->vrouter_up)
        vrouter_exit(false);

    if (ctx->message_up)
        vr_message_exit();

    if (ctx->memory_up)
        memory_exit();

    if (ctx->device_up)
        VRouterUninitializeDevices(SxDriverObject);

    if (ctx->pkt0_up)
        Pkt0DestroyDevice(SxDriverObject);

    if (ctx->ksync_up)
        KsyncDestroyDevice(SxDriverObject);
}

void SxExtUninitializeWindowsComponents(struct vr_switch_context* ctx)
{
    if (SxNBLPool)
        vrouter_free_pool(SxNBLPool);

    if (AsyncWorkRWLock)
        NdisFreeRWLock(AsyncWorkRWLock);

    if (ctx)
    {
        if (ctx->lock)
            NdisFreeRWLock(ctx->lock);

        ExFreePoolWithTag(ctx, SxExtAllocationTag);
    }
}

void
DriverUnload(PDRIVER_OBJECT DriverObject)
{
    NdisFDeregisterFilterDriver(DriverHandle);
    NdisFreeSpinLock(&SxExtensionListLock);
}

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

static struct vr_interface*
get_vif(NDIS_SWITCH_PORT_ID vif_port, NDIS_SWITCH_NIC_INDEX vif_nic)
{
    struct vrouter *vr = vrouter_get(0);

    ASSERT(vr != NULL);

    for (int i = 0; i < vr->vr_max_interfaces; i++)
    {
        struct vr_interface* vif = vr->vr_interfaces[i];

        if (vif == NULL)
            continue;

        if (vif->vif_port == vif_port && vif->vif_nic == vif_nic)
            return vif;
    }

    // VIF is not registered, very temporary state
    return NULL;
}

/*  Dumps packet contents to the debug buffer. Packet contents will be formatted in
    Wireshark friendly format.
*/
void
debug_print_net_buffer(PNET_BUFFER nb, const char *prefix)
{
#ifdef _NBL_DEBUG
    ULONG data_length;
    ULONG str_length;
    ULONG str_alloc_size;
    unsigned char *buffer;
    unsigned char *str;
    int bytes_copied;
    int i, j;

    if (!nb) {
        return;
    }

    data_length = NET_BUFFER_DATA_LENGTH(nb);
    str_length = data_length * 3 + 1;  // '|' + 3 chars ("FF|") per byte
    str_alloc_size = str_length + 1;  // additional '\0' at the end

    buffer = (unsigned char *)ExAllocatePoolWithTag(NonPagedPoolNx, data_length, SxExtAllocationTag);
    if (!buffer) {
        return;
    }
    str = (unsigned char *)ExAllocatePoolWithTag(NonPagedPoolNx, str_alloc_size, SxExtAllocationTag);
    if (!str) {
        ExFreePoolWithTag(buffer, SxExtAllocationTag);
        return;
    }

    bytes_copied = win_pcopy_from_nb(buffer, nb, 0, data_length);
    if (bytes_copied < 0) {
        DbgPrint("%s: win_pcopy_from_nbl failed; result = %d\n", bytes_copied);
        ExFreePoolWithTag(buffer, SxExtAllocationTag);
        return;
    }

    str[0] = '|';
    for (i = 0, j = 1; i < bytes_copied; ++i, j += 3) {
        str[j]     = hex_table[(buffer[i] & 0xF0) >> 4];
        str[j + 1] = hex_table[(buffer[i] & 0x0F)];
        str[j + 2] = '|';
    }
    str[j] = 0;

    // DbgPrint only transmits at most 512 bytes in single call, so multiple prints are needed
    // to dump whole packet contents.
    DbgPrint("%s data[length=%d,copied=%d]: ", prefix, data_length, bytes_copied);
    unsigned char *str_iter = str;
    unsigned char tmp;
    ULONG printed = 0, max_print_length = 510;
    ULONG to_print;
    while (printed < str_length) {
        if (str_length - printed < max_print_length) {
            to_print = str_length - printed;
        } else {
            to_print = max_print_length;
        }
        tmp = str_iter[to_print];
        str_iter[to_print] = 0;
        DbgPrint("%s", str_iter);
        str_iter[to_print] = tmp;

        str_iter += to_print;
        printed += to_print;
    }
    DbgPrint("\n");

    ExFreePoolWithTag(str, SxExtAllocationTag);
    ExFreePoolWithTag(buffer, SxExtAllocationTag);
#endif
}

NDIS_STATUS
SxExtInitializeVRouter(struct vr_switch_context* ctx)
{
    ASSERT(!ctx->ksync_up);
    ASSERT(!ctx->pkt0_up);
    ASSERT(!ctx->device_up);
    ASSERT(!ctx->memory_up);
    ASSERT(!ctx->message_up);
    ASSERT(!ctx->vrouter_up);

    ctx->ksync_up = NT_SUCCESS(KsyncCreateDevice(SxDriverObject));

    if (!ctx->ksync_up)
        goto cleanup;

    ctx->pkt0_up = NT_SUCCESS(Pkt0CreateDevice(SxDriverObject));

    if (!ctx->pkt0_up)
        goto cleanup;

    ctx->device_up = NT_SUCCESS(VRouterInitializeDevices(SxDriverObject));

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
    SxExtUninitializeVRouter(ctx);
    return NDIS_STATUS_FAILURE;
}

NDIS_STATUS
SxExtInitializeWindowsComponents(PSX_SWITCH_OBJECT Switch, PNDIS_HANDLE *ExtensionContext)
{
    struct vr_switch_context *ctx = NULL;

    ctx = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(struct vr_switch_context), SxExtAllocationTag);
    if (ctx == NULL)
        return NDIS_STATUS_RESOURCES;

    RtlZeroMemory(ctx, sizeof(struct vr_switch_context));

    ctx->lock = NdisAllocateRWLock(Switch->NdisFilterHandle);

    ctx->restart = TRUE;

    *ExtensionContext = (NDIS_HANDLE)ctx;

    AsyncWorkRWLock = NdisAllocateRWLock(Switch->NdisFilterHandle);
    if (AsyncWorkRWLock == NULL)
        goto cleanup;

    SxNBLPool = vrouter_generate_pool();
    if (SxNBLPool == NULL)
        goto cleanup;

    *ExtensionContext = (NDIS_HANDLE) ctx;

    return NDIS_STATUS_SUCCESS;

cleanup:
    SxExtUninitializeWindowsComponents(ctx);

    *ExtensionContext = NULL;

    return NDIS_STATUS_FAILURE;
}

NDIS_STATUS
SxExtCreateSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Outptr_result_maybenull_ PNDIS_HANDLE *ExtensionContext
)
{
    DbgPrint("SxExtCreateSwitch\r\n");

    if (SxSwitchObject != NULL)
        return NDIS_STATUS_FAILURE;

    vr_num_cpus = KeQueryActiveProcessorCount(NULL);
    if (!vr_num_cpus) {
        DbgPrint("%s: Failed to get processor count\n", __func__);
        return NDIS_STATUS_FAILURE;
    }

    SxSwitchObject = Switch;

    BOOLEAN windows = FALSE;
    BOOLEAN vrouter = FALSE;

    NDIS_STATUS status = SxExtInitializeWindowsComponents(Switch, ExtensionContext);
    struct vr_switch_context* ctx = (struct vr_switch_context*)*ExtensionContext;

    if (!NT_SUCCESS(status))
        goto cleanup;

    windows = TRUE;

    status = SxExtInitializeVRouter(ctx);

    if (!NT_SUCCESS(status))
        goto cleanup;

    vrouter = TRUE;

    return NDIS_STATUS_SUCCESS;

cleanup:
    if (vrouter)
        SxExtUninitializeVRouter(ctx);

    if (windows)
        SxExtUninitializeWindowsComponents(ctx);

    SxSwitchObject = NULL;

    return NDIS_STATUS_FAILURE;
}

NDIS_STATUS
SxExtCreatePort(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PORT_PARAMETERS Port
)
{
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);

    DbgPrint("SxExtCreatePort\r\n");
    DbgPrint("PortFriendlyName: %S, PortName: %S, PortId: %u, PortState: %u, PortType: %u\r\n", Port->PortFriendlyName.String, Port->PortName.String, Port->PortId, Port->PortState, Port->PortType);

    return 0;
}

NDIS_STATUS
SxExtCreateNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
    DbgPrint("SxExtCreateNic\r\n");
    UNREFERENCED_PARAMETER(Switch);

    struct vr_switch_context *ctx = (struct vr_switch_context *)ExtensionContext;

    while (ctx->restart)
    {
        NdisMSleep(100);
    }

    DbgPrint("NicFriendlyName: %S, NicName: %S, NicIndex: %u, NicState: %u, NicType: %u, PortId: %u, PermamentMacAddress: %s, CurrentMacAddress: %s, VMMacAddress: %s, VmName: %S\r\n",
        Nic->NicFriendlyName.String, Nic->NicName.String, Nic->NicIndex, Nic->NicState, Nic->NicType, Nic->PortId, Nic->PermanentMacAddress, Nic->CurrentMacAddress, Nic->VMMacAddress, Nic->VmName.String);

    return 0;
}

VOID
SxExtConnectNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
    DbgPrint("SxExtConnectNic\r\n");
    UNREFERENCED_PARAMETER(Switch);

    struct vr_switch_context *ctx = (struct vr_switch_context*)ExtensionContext;

    while (ctx->restart)
    {
        NdisMSleep(100);
    }
}

VOID
SxExtUpdateNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
    DbgPrint("SxExtUpdateNic\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(Nic);

    struct vr_switch_context *ctx = (struct vr_switch_context *)ExtensionContext;

    while (ctx->restart)
    {
        NdisMSleep(100);
    }
}

VOID
SxExtDisconnectNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
    DbgPrint("SxExtDisconnectNic\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(Nic);

    struct vr_switch_context *ctx = (struct vr_switch_context *)ExtensionContext;

    while (ctx->restart)
    {
        NdisMSleep(100);
    }
}

VOID
SxExtDeleteNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
    DbgPrint("SxExtDeleteNic\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(Nic);

    struct vr_switch_context *ctx = (struct vr_switch_context *)ExtensionContext;

    while (ctx->restart)
    {
        NdisMSleep(100);
    }
}

static void
vr_win_split_nbls_by_forwarding_type(
    PNET_BUFFER_LIST nbl,
    PNET_BUFFER_LIST *nextExtForwardNbl,
    PNET_BUFFER_LIST *nextNativeForwardedNbl)
{
    PNET_BUFFER_LIST curNbl;
    PNET_BUFFER_LIST nextNbl;
    PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwdDetail;

    // Divide the NBL into two: part which requires native forwarding and the rest
    for (curNbl = nbl; curNbl != NULL; curNbl = nextNbl)
    {
        // Rememeber the next NBL
        nextNbl = curNbl->Next;

        fwdDetail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(curNbl);

        if (fwdDetail->NativeForwardingRequired)
        {
            // Set the next NBL to current NBL. This pointer points to either first pointer to
            // native forwarded NBL or the "Next" field of the last one.
            *nextNativeForwardedNbl = curNbl;
            nextNativeForwardedNbl = &(curNbl->Next);
        }
        else
        {
            // Set the next NBL to current NBL. This pointer points to either first pointer to
            // non-native forwarded NBL or the "Next" field of the last one.
            *nextExtForwardNbl = curNbl;
            nextExtForwardNbl = &(curNbl->Next);
        }
    }
}

NDIS_STATUS
FilterAttach(NDIS_HANDLE NdisFilterHandle, NDIS_HANDLE SxDriverContext,
             PNDIS_FILTER_ATTACH_PARAMETERS AttachParameters)
{
    NDIS_STATUS status;
    NDIS_FILTER_ATTRIBUTES sxAttributes;
    ULONG switchObjectSize;
    NDIS_SWITCH_CONTEXT switchContext;
    NDIS_SWITCH_OPTIONAL_HANDLERS switchHandler;
    PSX_SWITCH_OBJECT switchObject;

    UNREFERENCED_PARAMETER(SxDriverContext);

    DbgPrint("%s: NdisFilterHandle %p\r\n", __func__, NdisFilterHandle);

    status = NDIS_STATUS_SUCCESS;
    switchObject = NULL;

    NT_ASSERT(SxDriverContext == (NDIS_HANDLE)SxDriverObject);

    if (AttachParameters->MiniportMediaType != NdisMedium802_3)
    {
        status = NDIS_STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    switchHandler.Header.Type = NDIS_OBJECT_TYPE_SWITCH_OPTIONAL_HANDLERS;
    switchHandler.Header.Size = NDIS_SIZEOF_SWITCH_OPTIONAL_HANDLERS_REVISION_1;
    switchHandler.Header.Revision = NDIS_SWITCH_OPTIONAL_HANDLERS_REVISION_1;

    status = NdisFGetOptionalSwitchHandlers(NdisFilterHandle,
                                            &switchContext,
                                            &switchHandler);

    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("%s: Extension is running in non-switch environment.\r\n", __func__);
        goto Cleanup;
    }

    switchObjectSize = sizeof(SX_SWITCH_OBJECT);
    switchObject = ExAllocatePoolWithTag(NonPagedPoolNx,
                                         switchObjectSize,
                                         SxExtAllocationTag);

    if (switchObject == NULL)
    {
        status = NDIS_STATUS_RESOURCES;
        goto Cleanup;
    }

    RtlZeroMemory(switchObject, switchObjectSize);

    // Initialize NDIS related information.
    switchObject->NdisFilterHandle = NdisFilterHandle;
    switchObject->NdisSwitchContext = switchContext;
    RtlCopyMemory(&switchObject->NdisSwitchHandlers,
                  &switchHandler,
                  sizeof(NDIS_SWITCH_OPTIONAL_HANDLERS));

    // Let the extension create its own context.
    status = SxExtCreateSwitch(switchObject,
                               &(switchObject->ExtensionContext));

    if (status != NDIS_STATUS_SUCCESS)
    {
        goto Cleanup;
    }

    // Register the object with NDIS because NDIS passes this object when it
    // calls into the driver.
    NdisZeroMemory(&sxAttributes, sizeof(NDIS_FILTER_ATTRIBUTES));
    sxAttributes.Header.Revision = NDIS_FILTER_ATTRIBUTES_REVISION_1;
    sxAttributes.Header.Size = sizeof(NDIS_FILTER_ATTRIBUTES);
    sxAttributes.Header.Type = NDIS_OBJECT_TYPE_FILTER_ATTRIBUTES;
    sxAttributes.Flags = 0;

    NDIS_DECLARE_FILTER_MODULE_CONTEXT(SX_SWITCH_OBJECT);
    status = NdisFSetAttributes(NdisFilterHandle, switchObject, &sxAttributes);

    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("%s: Failed to set attributes.\r\n", __func__);
        goto Cleanup;
    }

    switchObject->ControlFlowState = SxSwitchAttached;
    switchObject->DataFlowState = SxSwitchPaused;

    NdisAcquireSpinLock(&SxExtensionListLock);
    InsertHeadList(&SxExtensionList, &switchObject->Link);
    NdisReleaseSpinLock(&SxExtensionListLock);

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

void
FilterDetach(NDIS_HANDLE FilterModuleContext)
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;

    DbgPrint("%s: SxInstance %p\r\n", __func__, FilterModuleContext);

    // The extension must be in paused state.
    NT_ASSERT(switchObject->DataFlowState == SxSwitchPaused);
    switchObject->ControlFlowState = SxSwitchDetached;

    KeMemoryBarrier();

    while(switchObject->PendingOidCount > 0)
    {
        NdisMSleep(1000);
    }

    ASSERTMSG("Trying to delete another switch than currently active", switchObject == SxSwitchObject);

    struct vr_switch_context* ctx = (struct vr_switch_context*)switchObject->ExtensionContext;

    SxExtUninitializeVRouter(ctx);
    SxExtUninitializeWindowsComponents(ctx);

    SxSwitchObject = NULL;

    NdisAcquireSpinLock(&SxExtensionListLock);
    RemoveEntryList(&switchObject->Link);
    NdisReleaseSpinLock(&SxExtensionListLock);

    ExFreePool(switchObject);
}

NDIS_STATUS
FilterPause(NDIS_HANDLE FilterModuleContext, PNDIS_FILTER_PAUSE_PARAMETERS PauseParameters)
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)(FilterModuleContext);

    UNREFERENCED_PARAMETER(PauseParameters);

    DbgPrint("%s: SxInstance %p\r\n", __func__, FilterModuleContext);

    // Set the flag that the filter is going to pause.
    NT_ASSERT(switchObject->DataFlowState == SxSwitchRunning);
    switchObject->DataFlowState = SxSwitchPaused;

    KeMemoryBarrier();

    while(switchObject->PendingInjectedNblCount > 0)
    {
        NdisMSleep(1000);
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
FilterRestart(NDIS_HANDLE FilterModuleContext, PNDIS_FILTER_RESTART_PARAMETERS RestartParameters)
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;

    UNREFERENCED_PARAMETER(RestartParameters);

    DbgPrint("%s: FilterModuleContext %p\n", __func__, FilterModuleContext);

    struct vr_switch_context *ctx = (struct vr_switch_context *)switchObject->ExtensionContext;
    ctx->restart = FALSE;

    NT_ASSERT(switchObject->DataFlowState == SxSwitchPaused);
    switchObject->DataFlowState = SxSwitchRunning;

    return NDIS_STATUS_SUCCESS;
}

void
FilterSendNetBufferLists(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    NDIS_PORT_NUMBER PortNumber,
    ULONG SendFlags
    )
{
    PSX_SWITCH_OBJECT Switch = (PSX_SWITCH_OBJECT)FilterModuleContext;

    UNREFERENCED_PARAMETER(PortNumber);

    struct vr_switch_context *ctx = (struct vr_switch_context*)Switch->ExtensionContext;
    LOCK_STATE_EX lockState;

    BOOLEAN sameSource;
    ULONG sendCompleteFlags = 0;
    BOOLEAN on_dispatch_level;

    PNET_BUFFER_LIST extForwardedNbls = NULL;  // NBLs forwarded by extension.
    PNET_BUFFER_LIST nativeForwardedNbls = NULL;  // NBLs that require native forwarding - extension just sends them.
    PNET_BUFFER_LIST curNbl = NULL;
    PNET_BUFFER_LIST nextNbl = NULL;

    // True if packets come from the same switch source port.
    sameSource = NDIS_TEST_SEND_FLAG(SendFlags, NDIS_SEND_FLAGS_SWITCH_SINGLE_SOURCE);
    if (sameSource) {
        sendCompleteFlags |= NDIS_SEND_COMPLETE_FLAGS_SWITCH_SINGLE_SOURCE;
    }

    // Forward DISPATCH_LEVEL flag.
    on_dispatch_level = NDIS_TEST_SEND_FLAG(SendFlags, NDIS_SEND_FLAGS_DISPATCH_LEVEL);
    if (on_dispatch_level) {
        sendCompleteFlags |= NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL;
    }

    // Acquire the lock, now interfaces cannot disconnect, etc.
    NdisAcquireRWLockRead(ctx->lock, &lockState, on_dispatch_level);

    vr_win_split_nbls_by_forwarding_type(NetBufferLists, &extForwardedNbls, &nativeForwardedNbls);

    for (curNbl = extForwardedNbls; curNbl != NULL; curNbl = nextNbl)
    {
        /* Save next NBL, because after passing control to vRouter it might drop curNbl.
        Also vRouter handles packets one-by-one, so we operate on single NBLs.
        */
        nextNbl = curNbl->Next;
        curNbl->Next = NULL;

        PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwd_detail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(curNbl);
        NDIS_SWITCH_PORT_ID source_port = fwd_detail->SourcePortId;
        NDIS_SWITCH_NIC_INDEX source_nic = fwd_detail->SourceNicIndex;
        windows_host.hos_printf("%s: port %d and interface id %d\n", __func__, source_port, source_nic);

        struct vr_interface *vif = get_vif(source_port, source_nic);

        if (!vif) {
            // If no vif attached yet, then drop NBL.
            windows_host.hos_printf("%s: No vif found\n", __func__);
            NdisFSendNetBufferListsComplete(Switch->NdisFilterHandle, curNbl, sendCompleteFlags);
            continue;
        }

        windows_host.hos_printf("%s: VIF has port %d and interface id %d\n", __func__, vif->vif_port, vif->vif_nic);

        struct vr_packet *pkt = win_get_packet(curNbl, vif);

        windows_host.hos_printf("%s: Got pkt\n", __func__);
        ASSERTMSG("win_get_packed failed!", pkt != NULL);

        if (pkt == NULL) {
            /* If `win_get_packet` fails, it will drop the NBL. */
            windows_host.hos_printf("%s: pkt is NULL\n", __func__);
            NdisFSendNetBufferListsComplete(Switch->NdisFilterHandle, curNbl, sendCompleteFlags);
            continue;
        }

        ASSERTMSG("VIF doesn't have a vif_rx method set!", vif->vif_rx != NULL);

        if (vif->vif_rx) {
            windows_host.hos_printf("%s: Calling vif_rx", __func__);
            int rx_ret = vif->vif_rx(vif, pkt, VLAN_ID_INVALID);

            windows_host.hos_printf("%s: vif_rx returned %d\n", __func__, rx_ret);
        }
        else {
            windows_host.hos_printf("%s: vif_rx is NULL\n", __func__);
            /* If `vif_rx` is not set (unlikely in production), then drop the packet. */
            windows_host.hos_pfree(pkt, VP_DROP_INTERFACE_DROP);
            continue;
        }
    }

    if (nativeForwardedNbls != NULL) {
        DbgPrint("StartIngress: send native forwarded NBL\r\n");

        ASSERT(Switch->DataFlowState == SxSwitchRunning); // TODO: JW-1096: Refactor: cannot be assert!

        NdisFSendNetBufferLists(Switch->NdisFilterHandle,
            nativeForwardedNbls,
            NDIS_DEFAULT_PORT_NUMBER,
            SendFlags);
    }

    // Release the lock, now interfaces can disconnect, etc.
    NdisReleaseRWLock(ctx->lock, &lockState);
}

void
FilterReceiveNetBufferLists( // TODO: JW-1097: can be removed?
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    NDIS_PORT_NUMBER PortNumber,
    ULONG NumberOfNetBufferLists,
    ULONG ReceiveFlags)
{
    PSX_SWITCH_OBJECT Switch = (PSX_SWITCH_OBJECT)FilterModuleContext;

    UNREFERENCED_PARAMETER(PortNumber);

    ASSERT(Switch->DataFlowState == SxSwitchRunning); // TODO: JW-1097: Refactor: cannot be assert!

    NdisFIndicateReceiveNetBufferLists(Switch->NdisFilterHandle,
                                       NetBufferLists,
                                       NDIS_DEFAULT_PORT_NUMBER,
                                       NumberOfNetBufferLists,
                                       ReceiveFlags);
}

void
FilterReturnNetBufferLists( // TODO: JW-1097: can be removed?
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    ULONG ReturnFlags
    )
{
    PSX_SWITCH_OBJECT Switch = (PSX_SWITCH_OBJECT)FilterModuleContext;

    NdisFReturnNetBufferLists(Switch->NdisFilterHandle,
                              NetBufferLists,
                              ReturnFlags);
}

void
FilterSendNetBufferListsComplete(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    ULONG SendCompleteFlags
    )
{
    DbgPrint("SxExtStartCompleteNetBufferListsIngress\r\n");

    PNET_BUFFER_LIST next = NetBufferLists;
    PNET_BUFFER_LIST current;
    do {
        current = next;
        next = current->Next;
        current->Next = NULL;

        free_nbl(current, 0);
    } while (next != NULL);
}

void
FilterCancelSendNetBufferLists( // TODO: JW-1096: check if this can be removed
    NDIS_HANDLE FilterModuleContext,
    PVOID CancelId
    )
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
    UNREFERENCED_PARAMETER(CancelId);
}

void
FilterStatus( // TODO: JW-1097: can be removed?
    NDIS_HANDLE FilterModuleContext,
    PNDIS_STATUS_INDICATION StatusIndication
    )
{
    // FilterStatus handler is required when Egress handlers are implemented by driver
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;

    NdisFIndicateStatus(switchObject->NdisFilterHandle,
                        StatusIndication);
}

NDIS_STATUS
SxpNdisProcessSetOid(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Inout_ PNDIS_OID_REQUEST OidRequest,
    _Out_ PBOOLEAN Complete
    )
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS_OID oid = OidRequest->DATA.SET_INFORMATION.Oid;
    PNDIS_OBJECT_HEADER header;
    ULONG bytesRestored = 0;

    *Complete = FALSE;

    header = OidRequest->DATA.SET_INFORMATION.InformationBuffer;

    if (OidRequest->DATA.SET_INFORMATION.InformationBufferLength != 0 &&
        OidRequest->DATA.SET_INFORMATION.InformationBufferLength <
            sizeof(NDIS_OBJECT_HEADER))
    {
        status = NDIS_STATUS_NOT_SUPPORTED;
        *Complete = TRUE;
        goto Cleanup;
    }

    if (OidRequest->DATA.SET_INFORMATION.InformationBufferLength == 0)
    {
        *Complete = FALSE;
        goto Cleanup;
    }

    switch(oid)
    {
        case OID_SWITCH_PROPERTY_ADD:
        case OID_SWITCH_PROPERTY_UPDATE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PROPERTY_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PROPERTY_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            if (oid == OID_SWITCH_PROPERTY_ADD)
            {
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                status = NDIS_STATUS_SUCCESS;
            }

            if (status == NDIS_STATUS_NOT_SUPPORTED)
            {
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                *Complete = TRUE;
                goto Cleanup;
            }

            break;
        case OID_SWITCH_PROPERTY_DELETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PROPERTY_DELETE_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PROPERTY_DELETE_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            *Complete = FALSE;

            break;

        case OID_SWITCH_PORT_PROPERTY_ADD:
        case OID_SWITCH_PORT_PROPERTY_UPDATE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PORT_PROPERTY_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PORT_PROPERTY_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            if (oid == OID_SWITCH_PORT_PROPERTY_ADD)
            {
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                status = NDIS_STATUS_SUCCESS;
            }

            if (status == NDIS_STATUS_NOT_SUPPORTED)
            {
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                *Complete = TRUE;
                goto Cleanup;
            }

            break;

        case OID_SWITCH_PORT_PROPERTY_DELETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PORT_PROPERTY_DELETE_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PORT_PROPERTY_DELETE_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            *Complete = FALSE;

            break;

        case OID_SWITCH_PORT_CREATE:
        case OID_SWITCH_PORT_UPDATED:
        case OID_SWITCH_PORT_TEARDOWN:
        case OID_SWITCH_PORT_DELETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PORT_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PORT_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            if (oid == OID_SWITCH_PORT_CREATE)
            {
                status = SxExtCreatePort(Switch,
                                         Switch->ExtensionContext,
                                         (PNDIS_SWITCH_PORT_PARAMETERS)header);

                if (status != NDIS_STATUS_SUCCESS)
                {
                    *Complete = TRUE;
                }
            }
            else if (oid == OID_SWITCH_PORT_UPDATED)
            {
            }
            else if (oid == OID_SWITCH_PORT_TEARDOWN)
            {
            }
            else
            {
            }

            break;

        case OID_SWITCH_NIC_CREATE:
        case OID_SWITCH_NIC_CONNECT:
        case OID_SWITCH_NIC_UPDATED:
        case OID_SWITCH_NIC_DISCONNECT:
        case OID_SWITCH_NIC_DELETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            if (oid == OID_SWITCH_NIC_CREATE)
            {
                status = SxExtCreateNic(Switch,
                                        Switch->ExtensionContext,
                                        (PNDIS_SWITCH_NIC_PARAMETERS)header);
                if (status != NDIS_STATUS_SUCCESS)
                {
                    *Complete = TRUE;
                }
            }
            else if (oid == OID_SWITCH_NIC_CONNECT)
            {
                SxExtConnectNic(Switch,
                                Switch->ExtensionContext,
                                (PNDIS_SWITCH_NIC_PARAMETERS)header);
            }
            else if (oid == OID_SWITCH_NIC_UPDATED)
            {
                SxExtUpdateNic(Switch,
                               Switch->ExtensionContext,
                               (PNDIS_SWITCH_NIC_PARAMETERS)header);
            }
            else if (oid == OID_SWITCH_NIC_DISCONNECT)
            {
                SxExtDisconnectNic(Switch,
                                   Switch->ExtensionContext,
                                   (PNDIS_SWITCH_NIC_PARAMETERS)header);
            }
            else
            {
                SxExtDeleteNic(Switch,
                               Switch->ExtensionContext,
                               (PNDIS_SWITCH_NIC_PARAMETERS)header);
            }

            break;

        case OID_SWITCH_NIC_RESTORE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                goto Cleanup;
            }

            status = NDIS_STATUS_SUCCESS;

            if (status != NDIS_STATUS_SUCCESS)
            {
                *Complete = TRUE;
            }
            else if (bytesRestored > 0)
            {
                *Complete = TRUE;
            }

            break;

        case OID_SWITCH_NIC_SAVE_COMPLETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            break;

        case OID_SWITCH_NIC_RESTORE_COMPLETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            break;

        default:
            break;
    }

Cleanup:
    return status;
}

NDIS_STATUS
SxpNdisProcessMethodOid(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Inout_ PNDIS_OID_REQUEST OidRequest,
    _Out_ PBOOLEAN Complete,
    _Out_ PULONG BytesNeeded
    )
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS_OID oid = OidRequest->DATA.SET_INFORMATION.Oid;
    PNDIS_OBJECT_HEADER header;
    PNDIS_SWITCH_NIC_OID_REQUEST nicOidRequest;
    PNDIS_SWITCH_NIC_OID_REQUEST newNicOidRequest = NULL;
    NDIS_SWITCH_PORT_ID destPort, sourcePort;
    NDIS_SWITCH_NIC_INDEX destNic, sourceNic;
    ULONG bytesWritten = 0;
    ULONG bytesNeeded = 0;

    *Complete = FALSE;
    *BytesNeeded = 0;

    header = OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;

    switch(oid)
    {
        case OID_SWITCH_FEATURE_STATUS_QUERY:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_FEATURE_STATUS_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_FEATURE_STATUS_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            *Complete = FALSE;

            if (*BytesNeeded > 0)
            {
                status = NDIS_STATUS_BUFFER_TOO_SHORT;
            }

            break;

        case OID_SWITCH_PORT_FEATURE_STATUS_QUERY:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_FEATURE_STATUS_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_FEATURE_STATUS_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            *Complete = FALSE;

            if (*BytesNeeded > 0)
            {
                status = NDIS_STATUS_BUFFER_TOO_SHORT;
            }

            break;

        case OID_SWITCH_NIC_REQUEST:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_OID_REQUEST_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_OID_REQUEST_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            nicOidRequest = (PNDIS_SWITCH_NIC_OID_REQUEST)header;

            sourcePort = nicOidRequest->SourcePortId;
            sourceNic = nicOidRequest->SourceNicIndex;
            destPort = nicOidRequest->DestinationPortId;
            destNic = nicOidRequest->DestinationNicIndex;

            status = NDIS_STATUS_SUCCESS;

            if (status != NDIS_STATUS_SUCCESS)
            {
                *Complete = TRUE;
                goto Cleanup;
            }

            if (sourcePort != nicOidRequest->SourcePortId ||
                sourceNic != nicOidRequest->SourceNicIndex ||
                destPort != nicOidRequest->DestinationPortId ||
                destNic != nicOidRequest->DestinationNicIndex)
            {
                ASSERT(Switch->OldNicRequest == NULL);
                Switch->OldNicRequest = nicOidRequest;

                newNicOidRequest = (PNDIS_SWITCH_NIC_OID_REQUEST)ExAllocatePoolWithTag(
                                                                    NonPagedPoolNx,
                                                                    sizeof(NDIS_SWITCH_NIC_OID_REQUEST),
                                                                    SxExtAllocationTag);

                if (newNicOidRequest == NULL)
                {
                    status = NDIS_STATUS_RESOURCES;
                    *Complete = TRUE;
                    goto Cleanup;
                }

                newNicOidRequest->Header = nicOidRequest->Header;
                newNicOidRequest->SourcePortId = sourcePort;
                newNicOidRequest->SourceNicIndex = sourceNic;
                newNicOidRequest->DestinationPortId = destPort;
                newNicOidRequest->DestinationNicIndex = destNic;
                newNicOidRequest->OidRequest = nicOidRequest->OidRequest;

                OidRequest->DATA.METHOD_INFORMATION.InformationBuffer = newNicOidRequest;
            }

            break;

        case OID_SWITCH_NIC_SAVE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            status = NDIS_STATUS_SUCCESS;

            if (status == NDIS_STATUS_SUCCESS &&
                bytesWritten > 0)
            {
                *Complete = TRUE;
            }
            else if (status == NDIS_STATUS_BUFFER_TOO_SHORT)
            {
                *BytesNeeded = ((PNDIS_SWITCH_NIC_SAVE_STATE)header)->SaveDataOffset +
                                bytesNeeded;
                *Complete = TRUE;
            }
            else if (status != NDIS_STATUS_SUCCESS)
            {
                *Complete = TRUE;
            }

            break;

        default:
            break;
    }

Cleanup:
    return status;
}

NDIS_STATUS
FilterOidRequest(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_OID_REQUEST OidRequest
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    NDIS_STATUS status;
    PNDIS_OID_REQUEST clonedRequest=NULL;
    PVOID *cloneRequestContext;
    BOOLEAN completeOid = FALSE;
    ULONG bytesNeeded = 0;

    status = NDIS_STATUS_SUCCESS;

    DbgPrint("%s: OidRequest %p.\r\n", __func__, OidRequest);

    NdisInterlockedIncrement(&switchObject->PendingOidCount);

    status = NdisAllocateCloneOidRequest(switchObject->NdisFilterHandle,
                                         OidRequest,
                                         SxExtAllocationTag,
                                         &clonedRequest);
    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("%s: Cannot Clone OidRequest\r\n", __func__);
        goto Cleanup;
    }

    cloneRequestContext = (PVOID*)(&clonedRequest->SourceReserved[0]);
    *cloneRequestContext = OidRequest;

    switch (clonedRequest->RequestType)
    {
        case NdisRequestSetInformation:
            status = SxpNdisProcessSetOid(switchObject,
                                          clonedRequest,
                                          &completeOid);
            break;

        case NdisRequestMethod:
            status = SxpNdisProcessMethodOid(switchObject,
                                             clonedRequest,
                                             &completeOid,
                                             &bytesNeeded);

            break;
    }

    if (completeOid)
    {
        NdisFreeCloneOidRequest(switchObject->NdisFilterHandle, clonedRequest);
        OidRequest->DATA.METHOD_INFORMATION.BytesNeeded = bytesNeeded;
        NdisInterlockedDecrement(&switchObject->PendingOidCount);
        goto Cleanup;
    }

    status = NdisFOidRequest(switchObject->NdisFilterHandle, clonedRequest);

    if (status != NDIS_STATUS_PENDING)
    {
        FilterOidRequestComplete(switchObject, clonedRequest, status);

        //
        // We must still return status as pending because we complete the
        // request using NdisFOidRequestComplete() in SxOidRequestComplete().
        //
        status = NDIS_STATUS_PENDING;
    }

Cleanup:
    return status;
}

void
FilterCancelOidRequest(
    NDIS_HANDLE FilterModuleContext,
    PVOID RequestId
    )
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
    UNREFERENCED_PARAMETER(RequestId);
}

void
FilterOidRequestComplete(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_OID_REQUEST NdisOidRequest,
    NDIS_STATUS Status
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    PNDIS_OID_REQUEST originalRequest;
    PVOID *oidRequestContext;
    PNDIS_SWITCH_NIC_OID_REQUEST nicOidRequestBuf;
    PNDIS_OBJECT_HEADER header;

    DbgPrint("%s: NdisOidRequest %p.\r\n", __func__, NdisOidRequest);

    oidRequestContext = (PVOID*)(&NdisOidRequest->SourceReserved[0]);
    originalRequest = (*oidRequestContext);

    //
    // This is the internal request
    //
    if (originalRequest == NULL)
    {
        SxpNdisCompleteInternalOidRequest(switchObject, NdisOidRequest, Status);
        goto Cleanup;
    }

    //
    // Copy the information from the returned request to the original request
    //
    switch(NdisOidRequest->RequestType)
    {
    case NdisRequestMethod:
        originalRequest->DATA.METHOD_INFORMATION.OutputBufferLength =
            NdisOidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;
        originalRequest->DATA.METHOD_INFORMATION.BytesRead =
            NdisOidRequest->DATA.METHOD_INFORMATION.BytesRead;
        originalRequest->DATA.METHOD_INFORMATION.BytesNeeded =
            NdisOidRequest->DATA.METHOD_INFORMATION.BytesNeeded;
        originalRequest->DATA.METHOD_INFORMATION.BytesWritten =
            NdisOidRequest->DATA.METHOD_INFORMATION.BytesWritten;

        if (NdisOidRequest->DATA.METHOD_INFORMATION.Oid == OID_SWITCH_NIC_REQUEST &&
                switchObject->OldNicRequest != NULL)
        {
            nicOidRequestBuf = NdisOidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
            Status = NDIS_STATUS_SUCCESS;

            originalRequest->DATA.METHOD_INFORMATION.InformationBuffer =
                                                    switchObject->OldNicRequest;
            switchObject->OldNicRequest = NULL;
            ExFreePoolWithTag(nicOidRequestBuf, SxExtAllocationTag);
        }

        break;

    case NdisRequestSetInformation:
        header = originalRequest->DATA.SET_INFORMATION.InformationBuffer;

        originalRequest->DATA.SET_INFORMATION.BytesRead =
            NdisOidRequest->DATA.SET_INFORMATION.BytesRead;
        originalRequest->DATA.SET_INFORMATION.BytesNeeded =
            NdisOidRequest->DATA.SET_INFORMATION.BytesNeeded;

        if (NdisOidRequest->DATA.METHOD_INFORMATION.Oid == OID_SWITCH_PORT_CREATE &&
            Status != NDIS_STATUS_SUCCESS)
        {
        }
        else if (NdisOidRequest->DATA.METHOD_INFORMATION.Oid == OID_SWITCH_PORT_CREATE &&
                 Status != NDIS_STATUS_SUCCESS)
        {
            SxExtDeleteNic(switchObject,
                           switchObject->ExtensionContext,
                           (PNDIS_SWITCH_NIC_PARAMETERS)header);
        }

        break;

    case NdisRequestQueryInformation:
    case NdisRequestQueryStatistics:
    default:
        originalRequest->DATA.QUERY_INFORMATION.BytesWritten =
            NdisOidRequest->DATA.QUERY_INFORMATION.BytesWritten;
        originalRequest->DATA.QUERY_INFORMATION.BytesNeeded =
            NdisOidRequest->DATA.QUERY_INFORMATION.BytesNeeded;
        break;
    }

    (*oidRequestContext) = NULL;

    NdisFreeCloneOidRequest(switchObject->NdisFilterHandle, NdisOidRequest);

    NdisFOidRequestComplete(switchObject->NdisFilterHandle,
                            originalRequest,
                            Status);

Cleanup:
    NdisInterlockedDecrement(&switchObject->PendingOidCount);
}

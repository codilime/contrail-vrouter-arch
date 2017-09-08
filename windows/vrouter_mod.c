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

const ULONG VrAllocationTag = 'RVCO';
const ULONG VrOidRequestId = 'RVCO';

static PDRIVER_OBJECT VrDriverObject;
static NDIS_HANDLE DriverHandle = NULL;
PSWITCH_OBJECT VrSwitchObject = NULL;
NDIS_HANDLE VrNBLPool = NULL;

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

FILTER_OID_REQUEST FilterOidRequest;
FILTER_OID_REQUEST_COMPLETE FilterOidRequestComplete;
FILTER_CANCEL_OID_REQUEST FilterCancelOidRequest;


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
                                       &DriverHandle);

    if (status != NDIS_STATUS_SUCCESS)
    {
        if (DriverHandle != NULL)
        {
            NdisFDeregisterFilterDriver(DriverHandle);
            DriverHandle = NULL;
        }
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

void UninitializeWindowsComponents(pvr_switch_context ctx)
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

void
DriverUnload(PDRIVER_OBJECT DriverObject)
{
    NdisFDeregisterFilterDriver(DriverHandle);
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

    buffer = (unsigned char *)ExAllocatePoolWithTag(NonPagedPoolNx, data_length, VrAllocationTag);
    if (!buffer) {
        return;
    }
    str = (unsigned char *)ExAllocatePoolWithTag(NonPagedPoolNx, str_alloc_size, VrAllocationTag);
    if (!str) {
        ExFreePoolWithTag(buffer, VrAllocationTag);
        return;
    }

    bytes_copied = win_pcopy_from_nb(buffer, nb, 0, data_length);
    if (bytes_copied < 0) {
        DbgPrint("%s: win_pcopy_from_nbl failed; result = %d\n", bytes_copied);
        ExFreePoolWithTag(buffer, VrAllocationTag);
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

    ExFreePoolWithTag(str, VrAllocationTag);
    ExFreePoolWithTag(buffer, VrAllocationTag);
#endif
}

NDIS_STATUS
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

NDIS_STATUS
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

NDIS_STATUS
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
FilterAttach(NDIS_HANDLE NdisFilterHandle, NDIS_HANDLE DriverContext,
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

void
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

void
FilterSendNetBufferLists(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    NDIS_PORT_NUMBER PortNumber,
    ULONG SendFlags)
{
    PSWITCH_OBJECT Switch = (PSWITCH_OBJECT)FilterModuleContext;

    LOCK_STATE_EX lockState;

    BOOLEAN sameSource;
    ULONG sendCompleteFlags = 0;
    BOOLEAN on_dispatch_level;

    PNET_BUFFER_LIST extForwardedNbls = NULL;  // NBLs forwarded by extension.
    PNET_BUFFER_LIST nativeForwardedNbls = NULL;  // NBLs that require native forwarding - extension just sends them.
    PNET_BUFFER_LIST curNbl = NULL;
    PNET_BUFFER_LIST nextNbl = NULL;

    UNREFERENCED_PARAMETER(PortNumber);

    DbgPrint("StartIngress\r\n");

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

    if (Switch->Running == FALSE) {
        DbgPrint("StartIngress: Dropping NBLs because Switch is not in Running state\r\n");
        NdisFSendNetBufferListsComplete(Switch->NdisFilterHandle, NetBufferLists, sendCompleteFlags);
        return;
    }

    // Acquire the lock, now interfaces cannot disconnect, etc.
    NdisAcquireRWLockRead(Switch->ExtensionContext->lock, &lockState, on_dispatch_level);

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

        NdisFSendNetBufferLists(Switch->NdisFilterHandle,
            nativeForwardedNbls,
            NDIS_DEFAULT_PORT_NUMBER,
            SendFlags);
    }

    // Release the lock, now interfaces can disconnect, etc.
    NdisReleaseRWLock(Switch->ExtensionContext->lock, &lockState);
}

void
FilterSendNetBufferListsComplete(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    ULONG SendCompleteFlags)
{
    PNET_BUFFER_LIST next = NetBufferLists;
    PNET_BUFFER_LIST current;

    UNREFERENCED_PARAMETER(FilterModuleContext);
    UNREFERENCED_PARAMETER(SendCompleteFlags);

    DbgPrint("CompleteIngress\r\n");

    do {
        current = next;
        next = current->Next;
        current->Next = NULL;

        free_nbl(current, 0);
    } while (next != NULL);
}

VOID
SxpNdisCompleteInternalOidRequest(
    PSWITCH_OBJECT Switch,
    PNDIS_OID_REQUEST NdisRequest,
    NDIS_STATUS Status)
{
    PSX_OID_REQUEST oidRequest;
    ULONG bytesNeeded;

    UNREFERENCED_PARAMETER(Switch);

    bytesNeeded = 0;
    oidRequest = NULL;

    switch (NdisRequest->RequestType)
    {
    case NdisRequestSetInformation:
        bytesNeeded = NdisRequest->DATA.SET_INFORMATION.BytesNeeded;
        break;

    case NdisRequestQueryInformation:
        bytesNeeded = NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
        break;

    case NdisRequestMethod:
        bytesNeeded = NdisRequest->DATA.METHOD_INFORMATION.BytesNeeded;
        break;
    }

    // Get at the request context.
    oidRequest = CONTAINING_RECORD(NdisRequest, SX_OID_REQUEST, NdisOidRequest);

    // Save away the completion status.
    oidRequest->Status = Status;

    // Save bytesNeeded
    oidRequest->BytesNeeded = bytesNeeded;

    // Wake up the thread blocked for this request to complete.
    NdisSetEvent(&oidRequest->ReqEvent);
}

NDIS_STATUS
VrQuerySwitchNicArray(PSWITCH_OBJECT Switch, PVOID Buffer, ULONG BufferLength, PULONG OutputBytesNeeded)
{
    PSX_OID_REQUEST oidRequest;
    PNDIS_OID_REQUEST ndisOidRequest;
    ULONG bytesNeeded;
    NDIS_STATUS status;

    oidRequest = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(*oidRequest), VrAllocationTag);
    if (oidRequest == NULL) {
        return NDIS_STATUS_RESOURCES;
    }

    RtlZeroMemory(oidRequest, sizeof(*oidRequest));
    NdisInitializeEvent(&oidRequest->ReqEvent);

    ndisOidRequest = &oidRequest->NdisOidRequest;

    ndisOidRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
    ndisOidRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
    ndisOidRequest->Header.Size = sizeof(NDIS_OID_REQUEST);
    ndisOidRequest->RequestType = NdisRequestQueryInformation;
    ndisOidRequest->Timeout = 0;
    ndisOidRequest->RequestId = (PVOID)VrOidRequestId;

    ndisOidRequest->DATA.QUERY_INFORMATION.Oid = OID_SWITCH_NIC_ARRAY;
    ndisOidRequest->DATA.QUERY_INFORMATION.InformationBuffer = Buffer;
    ndisOidRequest->DATA.QUERY_INFORMATION.InformationBufferLength = BufferLength;

    NdisInterlockedIncrement(&Switch->PendingOidCount);
    status = NdisFOidRequest(Switch->NdisFilterHandle, ndisOidRequest);
    if (status == NDIS_STATUS_PENDING) {
        NdisWaitEvent(&oidRequest->ReqEvent, 0);
    } else {
        SxpNdisCompleteInternalOidRequest(Switch, ndisOidRequest, status);
        NdisInterlockedDecrement(&Switch->PendingOidCount);
    }

    bytesNeeded = oidRequest->BytesNeeded;
    status = oidRequest->Status;

    if (OutputBytesNeeded != NULL) {
        *OutputBytesNeeded = bytesNeeded;
    }

    ExFreePoolWithTag(oidRequest, VrAllocationTag);
    return status;
}

NDIS_STATUS
VrGetNicArray(PSWITCH_OBJECT Switch, PNDIS_SWITCH_NIC_ARRAY *OutputNicArray)
{
    NDIS_STATUS status;
    PNDIS_SWITCH_NIC_ARRAY nicArray = NULL;
    ULONG nicArrayLength = 0;

    if (OutputNicArray == NULL) {
        return NDIS_STATUS_INVALID_PARAMETER;
    }

    status = VrQuerySwitchNicArray(Switch, 0, 0, &nicArrayLength);
    if (status != NDIS_STATUS_INVALID_LENGTH) {
        DbgPrint("vRouter:%s(): OID_SWITCH_NIC_ARRAY did not return required buffer size\n", __func__);
        return NDIS_STATUS_FAILURE;
    }

    nicArray = ExAllocatePoolWithTag(NonPagedPoolNx, nicArrayLength, VrAllocationTag);
    if (nicArray == NULL) {
        return NDIS_STATUS_RESOURCES;
    }

    nicArray->Header.Revision = NDIS_SWITCH_PORT_ARRAY_REVISION_1;
    nicArray->Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    nicArray->Header.Size = (USHORT)nicArrayLength;

    status = VrQuerySwitchNicArray(Switch, nicArray, nicArrayLength, NULL);
    if (status == NDIS_STATUS_SUCCESS) {
        *OutputNicArray = nicArray;
    } else {
        ExFreePoolWithTag(nicArray, VrAllocationTag);
    }

    return status;
}

VOID
VrFreeNicArray(PNDIS_SWITCH_NIC_ARRAY NicArray)
{
    if (NicArray != NULL) {
        ExFreePoolWithTag(NicArray, VrAllocationTag);
    }
}

NDIS_STATUS
FilterOidRequest(NDIS_HANDLE FilterModuleContext, PNDIS_OID_REQUEST OidRequest)
{
    PSWITCH_OBJECT switchObject = (PSWITCH_OBJECT)FilterModuleContext;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    PNDIS_OID_REQUEST clonedRequest = NULL;

    DbgPrint("%s: OidRequest %p.\r\n", __func__, OidRequest);

    status = NdisAllocateCloneOidRequest(switchObject->NdisFilterHandle,
                                         OidRequest,
                                         VrAllocationTag,
                                         &clonedRequest);
    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("%s: Cannot Clone OidRequest\r\n", __func__);
        return status;
    }

    *(PVOID*)(&clonedRequest->SourceReserved[0]) = OidRequest;
    NdisInterlockedIncrement(&switchObject->PendingOidCount);

    KeMemoryBarrier();

    status = NdisFOidRequest(switchObject->NdisFilterHandle, clonedRequest);
    if (status != NDIS_STATUS_PENDING)
    {
        FilterOidRequestComplete(switchObject, clonedRequest, status);
        return NDIS_STATUS_PENDING;
    }

    return status;
}

void
FilterCancelOidRequest(NDIS_HANDLE FilterModuleContext, PVOID RequestId)
{
    PSWITCH_OBJECT switchObject = (PSWITCH_OBJECT)FilterModuleContext;

    NdisFCancelOidRequest(switchObject->NdisFilterHandle, RequestId);
}

void
FilterOidRequestComplete(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_OID_REQUEST NdisOidRequest,
    NDIS_STATUS Status)
{
    PSWITCH_OBJECT switchObject = (PSWITCH_OBJECT)FilterModuleContext;
    PNDIS_OID_REQUEST originalRequest;
    PVOID *oidRequestContext;
    PNDIS_SWITCH_NIC_OID_REQUEST nicOidRequestBuf;
    PNDIS_OBJECT_HEADER header;

    DbgPrint("%s: NdisOidRequest %p.\r\n", __func__, NdisOidRequest);

    oidRequestContext = (PVOID*)(&NdisOidRequest->SourceReserved[0]);
    originalRequest = (*oidRequestContext);

    // This is the internal request
    if (originalRequest == NULL)
    {
        SxpNdisCompleteInternalOidRequest(switchObject, NdisOidRequest, Status);
        goto Cleanup;
    }

    // Copy the information from the returned request to the original request
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
            ExFreePoolWithTag(nicOidRequestBuf, VrAllocationTag);
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
            while (!switchObject->Running)
            {
                NdisMSleep(100);
            }
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

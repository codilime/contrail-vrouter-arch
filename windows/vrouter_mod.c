#include "precomp.h"
#include "vr_windows.h"
#include "windows_devices.h"
#include "vrouter.h"
#include "vr_packet.h"
#include "vr_sandesh.h"
#include "windows_mem.h"

UCHAR SxExtMajorNdisVersion = NDIS_FILTER_MAJOR_VERSION;
UCHAR SxExtMinorNdisVersion = NDIS_FILTER_MINOR_VERSION;

PWCHAR SxExtFriendlyName = L"OpenContrail's vRouter forwarding extension";

PWCHAR SxExtUniqueName = L"{56553588-1538-4BE6-B8E0-CB46402DC205}";

PWCHAR SxExtServiceName = L"vRouter";

ULONG  SxExtAllocationTag = 'RVCO';
ULONG  SxExtOidRequestId = 'RVCO';

PSX_SWITCH_OBJECT SxSwitchObject = NULL;
NDIS_HANDLE SxNBLPool = NULL;

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

static NTSTATUS
vr_message_init(void)
{
    int ret = vr_sandesh_init();
    if (ret) {
        DbgPrint("%s: vr_sandesh_init() failed with return %d\n", __func__, ret);
        return NDIS_STATUS_FAILURE;
    }

    return NDIS_STATUS_SUCCESS;
}

static void
vr_message_exit(void)
{
    vr_sandesh_exit();
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
SxExtInitialize(PDRIVER_OBJECT DriverObject)
{
    DbgPrint("SxExtInitialize\r\n");

    return NDIS_STATUS_SUCCESS;
}

VOID
SxExtUninitialize(PDRIVER_OBJECT DriverObject)
{
    DbgPrint("SxExtUninitialize\r\n");
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

VOID
SxExtDeleteSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext
)
{
    DbgPrint("SxExtDeleteSwitch\r\n");
    UNREFERENCED_PARAMETER(Switch);

    ASSERTMSG("Trying to delete another switch than currently active", Switch == SxSwitchObject);

    struct vr_switch_context* ctx = (struct vr_switch_context*)ExtensionContext;

    SxExtUninitializeVRouter(ctx);
    SxExtUninitializeWindowsComponents(ctx);

    SxSwitchObject = NULL;
}

VOID
SxExtActivateSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext
)
{
    DbgPrint("SxExtActivateSwitch\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
}

NDIS_STATUS
SxExtRestartSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext
)
{
    DbgPrint("SxExtRestartSwitch\r\n");
    UNREFERENCED_PARAMETER(Switch);

    struct vr_switch_context *ctx = (struct vr_switch_context *)ExtensionContext;

    ctx->restart = FALSE;

    return 0;
}

VOID
SxExtPauseSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext
)
{
    DbgPrint("SxExtPauseSwitch\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
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

VOID
SxExtUpdatePort(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PORT_PARAMETERS Port
)
{
    DbgPrint("SxExtUpdatePort\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(Port);
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

VOID
SxExtTeardownPort(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PORT_PARAMETERS Port
)
{
    DbgPrint("SxExtTeardownPort\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(Port);
}

VOID
SxExtDeletePort(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PORT_PARAMETERS Port
)
{
    DbgPrint("SxExtDeletePort\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(Port);
}

NDIS_STATUS
SxExtSaveNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _Inout_ PNDIS_SWITCH_NIC_SAVE_STATE SaveState,
    _Out_ PULONG BytesWritten,
    _Out_ PULONG BytesNeeded
)
{
    DbgPrint("SxExtSaveNic\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(SaveState);

    *BytesWritten = 0;
    *BytesNeeded = 0;

    return NDIS_STATUS_SUCCESS;
}

VOID
SxExtSaveNicComplete(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_SAVE_STATE SaveState
)
{
    DbgPrint("SxExtSaveNicComplete\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(SaveState);
}

NDIS_STATUS
SxExtNicRestore(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_SAVE_STATE SaveState,
    _Out_ PULONG BytesRestored
)
{
    DbgPrint("SxExtNicRestore\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(SaveState);
    UNREFERENCED_PARAMETER(BytesRestored);

    return 0;
}

VOID
SxExtNicRestoreComplete(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_SAVE_STATE SaveState
)
{
    DbgPrint("SxExtNicRestoreComplete\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(SaveState);
}

NDIS_STATUS
SxExtAddSwitchProperty(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PROPERTY_PARAMETERS SwitchProperty
)
{
    DbgPrint("SxExtAddSwitchProperty\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(SwitchProperty);

    return 0;
}

NDIS_STATUS
SxExtUpdateSwitchProperty(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PROPERTY_PARAMETERS SwitchProperty
)
{
    DbgPrint("SxExtUpdateSwitchProperty\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(SwitchProperty);

    return 0;
}

BOOLEAN
SxExtDeleteSwitchProperty(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PROPERTY_DELETE_PARAMETERS SwitchProperty
)
{
    DbgPrint("SxExtDeleteSwitchProperty\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(SwitchProperty);

    return 0;
}

NDIS_STATUS
SxExtAddPortProperty(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PORT_PROPERTY_PARAMETERS PortProperty
)
{
    DbgPrint("SxExtAddPortProperty\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(PortProperty);

    return 0;
}

NDIS_STATUS
SxExtUpdatePortProperty(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PORT_PROPERTY_PARAMETERS PortProperty
)
{
    DbgPrint("SxExtUpdatePortProperty\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(PortProperty);

    return 0;
}

BOOLEAN
SxExtDeletePortProperty(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PORT_PROPERTY_DELETE_PARAMETERS PortProperty
)
{
    DbgPrint("SxExtDeletePortProperty\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(PortProperty);

    return 0;
}

BOOLEAN
SxExtQuerySwitchFeatureStatus(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _Inout_ PNDIS_SWITCH_FEATURE_STATUS_PARAMETERS SwitchFeatureStatus,
    _Inout_ PULONG BytesNeeded
)
{
    DbgPrint("SxExtQuerySwitchFeatureStatus\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(SwitchFeatureStatus);
    UNREFERENCED_PARAMETER(BytesNeeded);

    return 0;
}

BOOLEAN
SxExtQueryPortFeatureStatus(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _Inout_ PNDIS_SWITCH_PORT_FEATURE_STATUS_PARAMETERS PortFeatureStatus,
    _Inout_ PULONG BytesNeeded
)
{
    DbgPrint("SxExtQueryPortFeatureStatus\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(PortFeatureStatus);
    UNREFERENCED_PARAMETER(BytesNeeded);

    return 0;
}

NDIS_STATUS
SxExtProcessNicRequest(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _Inout_ PNDIS_OID_REQUEST OidRequest,
    _Inout_ PNDIS_SWITCH_PORT_ID SourcePortId,
    _Inout_ PNDIS_SWITCH_NIC_INDEX SourceNicIndex,
    _Inout_ PNDIS_SWITCH_PORT_ID DestinationPortId,
    _Inout_ PNDIS_SWITCH_NIC_INDEX DestinationNicIndex
)
{
    DbgPrint("SxExtProcessNicRequest\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(OidRequest);
    UNREFERENCED_PARAMETER(SourcePortId);
    UNREFERENCED_PARAMETER(SourceNicIndex);
    UNREFERENCED_PARAMETER(DestinationPortId);
    UNREFERENCED_PARAMETER(DestinationNicIndex);

    return 0;
}

NDIS_STATUS
SxExtProcessNicRequestComplete(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _Inout_ PNDIS_OID_REQUEST OidRequest,
    _In_ NDIS_SWITCH_PORT_ID SourcePortId,
    _In_ NDIS_SWITCH_NIC_INDEX SourceNicIndex,
    _In_ NDIS_SWITCH_PORT_ID DestinationPortId,
    _In_ NDIS_SWITCH_NIC_INDEX DestinationNicIndex,
    _In_ NDIS_STATUS Status
)
{
    DbgPrint("SxExtProcessNicRequestComplete\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(OidRequest);
    UNREFERENCED_PARAMETER(SourcePortId);
    UNREFERENCED_PARAMETER(SourceNicIndex);
    UNREFERENCED_PARAMETER(DestinationPortId);
    UNREFERENCED_PARAMETER(DestinationNicIndex);
    UNREFERENCED_PARAMETER(Status);

    return 0;
}

NDIS_STATUS
SxExtProcessNicStatus(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_STATUS_INDICATION StatusIndication,
    _In_ NDIS_SWITCH_PORT_ID SourcePortId,
    _In_ NDIS_SWITCH_NIC_INDEX SourceNicIndex
)
{
    DbgPrint("SxExtProcessNicStatus\r\n");
    UNREFERENCED_PARAMETER(Switch);
    UNREFERENCED_PARAMETER(ExtensionContext);
    UNREFERENCED_PARAMETER(StatusIndication);
    UNREFERENCED_PARAMETER(SourcePortId);
    UNREFERENCED_PARAMETER(SourceNicIndex);

    return 0;
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

VOID
SxExtStartNetBufferListsIngress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG SendFlags
)
{
    struct vr_switch_context *ctx = (struct vr_switch_context*)ExtensionContext;
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
            SxLibCompleteNetBufferListsIngress(Switch, curNbl, sendCompleteFlags);
            continue;
        }

        windows_host.hos_printf("%s: VIF has port %d and interface id %d\n", __func__, vif->vif_port, vif->vif_nic);

        struct vr_packet *pkt = win_get_packet(curNbl, vif);

        windows_host.hos_printf("%s: Got pkt\n", __func__);
        ASSERTMSG("win_get_packed failed!", pkt != NULL);

        if (pkt == NULL) {
            /* If `win_get_packet` fails, it will drop the NBL. */
            windows_host.hos_printf("%s: pkt is NULL\n", __func__);
            SxLibCompleteNetBufferListsIngress(Switch, curNbl, sendCompleteFlags);
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
        SxLibSendNetBufferListsIngress(Switch,
            nativeForwardedNbls,
            SendFlags,
            0);
    }

    // Release the lock, now interfaces can disconnect, etc.
    NdisReleaseRWLock(ctx->lock, &lockState);
}

VOID
SxExtStartNetBufferListsEgress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG NumberOfNetBufferLists,
    _In_ ULONG ReceiveFlags
)
{
    DbgPrint("SxExtStartNetBufferListsEgress\r\n");
    UNREFERENCED_PARAMETER(ExtensionContext);

    SxLibSendNetBufferListsEgress(Switch,
        NetBufferLists,
        NumberOfNetBufferLists,
        ReceiveFlags);
}

VOID
SxExtStartCompleteNetBufferListsEgress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG ReturnFlags
)
{
    DbgPrint("SxExtStartCompleteNetBufferListsEgress\r\n");
    UNREFERENCED_PARAMETER(ExtensionContext);

    SxLibCompleteNetBufferListsEgress(Switch,
        NetBufferLists,
        ReturnFlags);
}

VOID
SxExtStartCompleteNetBufferListsIngress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG SendCompleteFlags
)
{
    DbgPrint("SxExtStartCompleteNetBufferListsIngress\r\n");
    UNREFERENCED_PARAMETER(ExtensionContext);

    PNET_BUFFER_LIST next = NetBufferLists;
    PNET_BUFFER_LIST current;
    do {
        current = next;
        next = current->Next;
        current->Next = NULL;

        free_nbl(current, 0);
    } while (next != NULL);
}

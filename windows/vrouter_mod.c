#include "precomp.h"
#include "vrouter.h"
#include "vr_packet.h"

UCHAR SxExtMajorNdisVersion = NDIS_FILTER_MAJOR_VERSION;
UCHAR SxExtMinorNdisVersion = NDIS_FILTER_MINOR_VERSION;

PWCHAR SxExtFriendlyName = L"OpenContrail's vRouter forwarding extension";

PWCHAR SxExtUniqueName = L"{56553588-1538-4BE6-B8E0-CB46402DC205}";

PWCHAR SxExtServiceName = L"vRouter";

ULONG  SxExtAllocationTag = 'RVCO';
ULONG  SxExtOidRequestId = 'RVCO';

PSX_SWITCH_OBJECT SxSwitchObject = NULL;
NDIS_HANDLE SxNBLPool = NULL;

/* Read/write lock which must be acquired by deferred callbacks. Used in functions from
* `host_os` struct.
*/
PNDIS_RW_LOCK_EX AsyncWorkRWLock = NULL;

static char hex_table[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

/*  Dumps packet contents to the debug buffer. Packet contents will be formatted in
    Wireshark friendly format.
*/
void
debug_print_net_buffer(PNET_BUFFER nb, const char *prefix)
{
#ifdef _DEBUG
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
    unsigned char print_buffer[512];
    ULONG printed = 0, max_print_length = 510;
    while (printed < str_length) {
        if (str_length - printed <= max_print_length) {
            NdisMoveMemory(print_buffer, str + printed, str_length - printed);
            print_buffer[str_length - printed] = 0;
            printed += str_length - printed;
        } else {
            NdisMoveMemory(print_buffer, str + printed, max_print_length);
            print_buffer[max_print_length] = 0;
            printed += max_print_length;
        }
        DbgPrint("%s", print_buffer);
    }
    DbgPrint("\n");

    ExFreePoolWithTag(str, SxExtAllocationTag);
    ExFreePoolWithTag(buffer, SxExtAllocationTag);
#endif
}

NDIS_STATUS
AddNicToArray(struct vr_switch_context* ctx, struct vr_nic nic)
{
    if (nic.nic_type != NdisSwitchNicTypeExternal)
    {
        DbgPrint("Internal NIC, Index: %u, Type: %u, PortId: %u\r\n", nic.nic_index, nic.nic_type, nic.port_id);
    }
    else
    {
        DbgPrint("External NIC, Index: %u, Type: %u, PortId: %u\r\n", nic.nic_index, nic.nic_type, nic.port_id);
    }

    if (ctx->num_nics == MAX_NIC_NUMBER - 1)
    {
        DbgPrint("All slots filled\r\n");
        return NDIS_STATUS_RESOURCES;
    }
    ctx->nics[ctx->num_nics++] = nic;

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
SxExtInitialize(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    DbgPrint("SxExtInitialize\r\n");

    vr_num_cpus = KeQueryActiveProcessorCount(NULL);
    if (!vr_num_cpus) {
        DbgPrint("%s: Failed to get processor count\n", __func__);
        return NDIS_STATUS_FAILURE;
    }

    return NDIS_STATUS_SUCCESS;
}

VOID
SxExtUninitialize(PDRIVER_OBJECT DriverObject)
{
    DbgPrint("SxExtUninitialize\r\n");
    UNREFERENCED_PARAMETER(DriverObject);
}

NDIS_STATUS
SxExtCreateSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Outptr_result_maybenull_ PNDIS_HANDLE *ExtensionContext
)
{
    DbgPrint("SxExtCreateSwitch\r\n");

    struct vr_switch_context *ctx = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(struct vr_switch_context), SxExtAllocationTag);
    RtlZeroMemory(ctx, sizeof(struct vr_switch_context));
    ctx->lock = NdisAllocateRWLock(Switch->NdisFilterHandle);

    ctx->restart = TRUE;

    *ExtensionContext = (NDIS_HANDLE)ctx;

    SxSwitchObject = Switch;

    AsyncWorkRWLock = NdisAllocateRWLock(Switch->NdisFilterHandle);
    if (AsyncWorkRWLock == NULL)
        return NDIS_STATUS_RESOURCES;

    SxNBLPool = vrouter_generate_pool();
    if (SxNBLPool == NULL)
    {
        NdisFreeRWLock(AsyncWorkRWLock);
        return NDIS_STATUS_RESOURCES;
    }

    return 0;
}

VOID
SxExtDeleteSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext
)
{
    DbgPrint("SxExtDeleteSwitch\r\n");
    UNREFERENCED_PARAMETER(Switch);

    vrouter_exit(false);

    NdisFreeRWLock(AsyncWorkRWLock);
    SxSwitchObject = NULL;

    vrouter_free_pool(SxNBLPool);
    NdisFreeRWLock(((struct vr_switch_context*)ExtensionContext)->lock);
    ExFreePoolWithTag(ExtensionContext, SxExtAllocationTag);

    vr_clean_assoc();
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

    PNDIS_SWITCH_NIC_ARRAY nics;
    if (SxLibGetNicArrayUnsafe(Switch, &nics) != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("Failed to fetch port array!\r\n");
        return NDIS_STATUS_FAILURE;
    }

    for (unsigned int i = 0; i < nics->NumElements; i++)
    {
        struct vr_nic nic = { 0 };
        PNDIS_SWITCH_NIC_PARAMETERS entry = NDIS_SWITCH_NIC_AT_ARRAY_INDEX(nics, i);
        RtlCopyMemory(nic.mac, entry->PermanentMacAddress, sizeof(nic.mac));
        nic.nic_index = entry->NicIndex;
        nic.nic_type = entry->NicType;
        nic.port_id = entry->PortId;

        AddNicToArray(ctx, nic);

        struct vr_interface* iface = ExAllocatePoolWithTag(NonPagedPool, sizeof(struct vr_interface), SxExtAllocationTag);
        NdisZeroMemory(iface, sizeof(struct vr_interface));

        vr_set_assoc_oid_name(entry->NicFriendlyName, iface);
        vr_set_assoc_oid_ids(entry->PortId, entry->NicIndex, iface);
    }

    int vrouter_init_status = vrouter_init();
    if (vrouter_init_status) {
        return NDIS_STATUS_FAILURE;
    }

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

    DbgPrint("NicFriendlyName: %S, NicName: %S, NicIndex: %u, NicState: %u, NicType: %u, PortId: %u, PermamentMacAddress: %s, CurrentMacAddress: %s, VMMacAddress: %s, VmName: %S",
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
    UNREFERENCED_PARAMETER(Nic);

    struct vr_switch_context *ctx = (struct vr_switch_context*)ExtensionContext;

    while (ctx->restart)
    {
        NdisMSleep(100);
    }

    struct vr_nic nic = { 0 };
    RtlCopyMemory(nic.mac, Nic->PermanentMacAddress, sizeof(nic.mac));
    nic.nic_index = Nic->NicIndex;
    nic.nic_type = Nic->NicType;
    nic.port_id = Nic->PortId;

    AddNicToArray(ctx, nic);

    struct vr_interface* iface = ExAllocatePoolWithTag(NonPagedPool, sizeof(struct vr_interface), SxExtAllocationTag);
    NdisZeroMemory(iface, sizeof(struct vr_interface));

    vr_set_assoc_oid_name(Nic->NicFriendlyName, iface);
    vr_set_assoc_oid_ids(Nic->PortId, Nic->NicIndex, iface);
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

    for (unsigned int i = 0; i < ctx->num_nics; i++)
    {
        if (ctx->nics[i].port_id == Nic->PortId && ctx->nics[i].nic_index == Nic->NicIndex)
        {
            if (i != ctx->num_nics - 1)
            {
                ctx->nics[i] = ctx->nics[ctx->num_nics - 1];
            }
            ctx->num_nics--;
        }
    }

    vr_delete_assoc_name(Nic->NicFriendlyName);
    vr_delete_assoc_ids(Nic->PortId, Nic->NicIndex);
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
        // Break the list
        curNbl->Next = NULL;

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

static void
vr_win_add_to_dropped_pkts(PNET_BUFFER_LIST nbl, PNET_BUFFER_LIST *dropped)
{
    *dropped = nbl;
    dropped = &nbl->Next;
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
    PNET_BUFFER_LIST dropNbl = NULL;
    PNET_BUFFER_LIST *nextDropNbl = &dropNbl;
    PNET_BUFFER_LIST curNbl = NULL;
    PNET_BUFFER_LIST nextNbl = NULL;

    PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwd_detail;
    NDIS_SWITCH_PORT_ID source_port;
    NDIS_SWITCH_NIC_INDEX source_nic;
    struct vr_interface *vif;
    struct vr_packet *pkt;
    int rx_ret;

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
        nextNbl = curNbl->Next;

        //  vif := vr_interface from <source_port_id, nic_id>
        fwd_detail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(curNbl);
        source_port = fwd_detail->SourcePortId;
        source_nic = fwd_detail->SourceNicIndex;
        vif = vr_get_assoc_ids(source_port, source_nic);
        if (!vif) {
            // If no vif attached yet, then drop NBL.
            vr_win_add_to_dropped_pkts(curNbl, nextDropNbl);
            NET_BUFFER* nb = NET_BUFFER_LIST_FIRST_NB(curNbl);
            debug_print_net_buffer(nb, "StartIngress: dropped, vif == NULL");
            continue;
        }

        //  pkt := vr_packet from PNET_BUFFER_LIST and vr_interface
        pkt = win_get_packet(curNbl, vif);
        if (!pkt) {
            // If creating vr_packet failed, then drop NBL.
            vr_win_add_to_dropped_pkts(curNbl, nextDropNbl);
            NET_BUFFER* nb = NET_BUFFER_LIST_FIRST_NB(curNbl);
            debug_print_net_buffer(nb, "StartIngress: dropped, couldn't get vr_packet");
            continue;
        }

        if (vif->vif_rx) {
            rx_ret = vif->vif_rx(vif, pkt, VLAN_ID_INVALID);
            if (!rx_ret) {
                // TODO: Remove packet drop when dp-core will properfly forward packets.
                vr_win_add_to_dropped_pkts(curNbl, nextDropNbl);
                NET_BUFFER* nb = NET_BUFFER_LIST_FIRST_NB(curNbl);
                debug_print_net_buffer(nb, "StartIngress: vif_rx succeeded");
            } else {
                //  If vif_rx failed, then drop NBL.
                //  TODO: Proper error logging.
                vr_win_add_to_dropped_pkts(curNbl, nextDropNbl);
                DbgPrint("StartIngress: dropped, vif_rx failed");
            }
        } else {
            vr_win_add_to_dropped_pkts(curNbl, nextDropNbl);
            NET_BUFFER* nb = NET_BUFFER_LIST_FIRST_NB(curNbl);
            debug_print_net_buffer(nb, "StartIngress: vif_rx == NULL");
            continue;
        }
    }

    // Release the lock, now interfaces can disconnect, etc.
    NdisReleaseRWLock(ctx->lock, &lockState);

    // Handle packet sending
    // NOTE: Currently every received packet is dropped.
    /*if (extForwardedNbls != NULL) {
        DbgPrint("StartIngress: send extension forwarded NBL\r\n");
        SxLibSendNetBufferListsIngress(Switch,
            extForwardedNbls,
            SendFlags,
            0);
    }*/

    if (nativeForwardedNbls != NULL) {
        DbgPrint("StartIngress: send native forwarded NBL\r\n");
        SxLibSendNetBufferListsIngress(Switch,
            nativeForwardedNbls,
            SendFlags,
            0);
    }

    if (dropNbl != NULL) {
        DbgPrint("StartIngress: dropping dropped NBLs\r\n");
        SxLibCompleteNetBufferListsIngress(Switch,
            dropNbl,
            sendCompleteFlags);
    }
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

    if (NetBufferLists->SourceHandle == Switch->NdisFilterHandle)
    {
        DbgPrint("Completing injected NBL...\r\n");

        PNET_BUFFER_LIST iterator = NetBufferLists;
        int count = 0;

        while (iterator)
        {
            iterator = iterator->Next;
            ++count;
        } // This is probably almost always one
        SxLibCompletedInjectedNetBufferLists(Switch, count);
    }
    else
    {
        DbgPrint("Completing non-injected NBL...\r\n");
        SxLibCompleteNetBufferListsIngress(Switch,
            NetBufferLists,
            SendCompleteFlags);
    }

    PNET_BUFFER_LIST curNbl = NetBufferLists;
    PNET_BUFFER_LIST nextNbl = NULL;

    while (curNbl != NULL)
    {
        nextNbl = curNbl->Next;
        struct vr_packet* pkt = win_get_packet_from_nbl(curNbl);
        if (pkt != NULL)
            windows_host.hos_pfree(pkt, VP_DROP_DISCARD);

        curNbl = nextNbl;
    }
}

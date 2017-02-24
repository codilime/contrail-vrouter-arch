#include "precomp.h"
#include "vr_windows.h"
#include "vr_ksync.h"
#include "vrouter.h"
#include "vr_packet.h"
#include "vr_sandesh.h"

extern void vif_attach(struct vr_interface *);

UCHAR SxExtMajorNdisVersion = NDIS_FILTER_MAJOR_VERSION;
UCHAR SxExtMinorNdisVersion = NDIS_FILTER_MINOR_VERSION;

PWCHAR SxExtFriendlyName = L"OpenContrail's vRouter forwarding extension";

PWCHAR SxExtUniqueName = L"{56553588-1538-4BE6-B8E0-CB46402DC205}";

PWCHAR SxExtServiceName = L"vRouter";

ULONG  SxExtAllocationTag = 'RVCO';
ULONG  SxExtOidRequestId = 'RVCO';

PSX_SWITCH_OBJECT SxSwitchObject = NULL;
NDIS_HANDLE SxNBLPool = NULL;

unsigned int vr_num_cpus = 1;

/* Read/write lock which must be acquired by deferred callbacks. Used in functions from
* `host_os` struct.
*/
PNDIS_RW_LOCK_EX AsyncWorkRWLock = NULL;

/* DEBUG(sodar) */
#define DEBUG_VROUTER_ID (0)
#define DEBUG_VRF (0)
int debug_vr_interface_delete(vr_interface_req *req, bool need_response);

/* DEBUG(sodar): Used for mocked forwarding structs */
uint8_t debug_bcast_mast[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
int32_t debug_vif_counter = 1;
int32_t debug_nh_composite = 1;
int32_t debug_nh_counter = 2;
int32_t debug_nh_elements[32] = { 0 };
int32_t debug_nh_labels[32] = { 0 };
int32_t debug_nh_elements_count = 0;

static char hex_table[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static int
vr_message_init(void)
{
    return vr_sandesh_init();
}

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
AddNicToArray(struct vr_switch_context* ctx, struct vr_nic* nic, NDIS_IF_COUNTED_STRING name)
{
    if (nic->nic_type != NdisSwitchNicTypeExternal)
    {
        DbgPrint("Internal NIC, Index: %u, Type: %u, PortId: %u\r\n", nic->nic_index, nic->nic_type, nic->port_id);
    }
    else
    {
        DbgPrint("External NIC, Index: %u, Type: %u, PortId: %u\r\n", nic->nic_index, nic->nic_type, nic->port_id);
    }

    if (ctx->num_nics == MAX_NIC_NUMBER - 1)
    {
        DbgPrint("All slots filled\r\n");
        return NDIS_STATUS_RESOURCES;
    }
    ctx->nics[ctx->num_nics++] = *nic;

    NDIS_IF_COUNTED_STRING _name = vr_get_name_from_friendly_name(name);

    if (nic->nic_type == NdisSwitchNicTypeInternal &&
        name.Length != 0) // We've got a container, because vr_get_name_from_friendly_name returned us the container name
    {
        struct vr_assoc* assoc_by_name = vr_get_assoc_name(_name);
        struct vr_assoc* assoc_by_ids = vr_get_assoc_ids(nic->port_id, nic->nic_index);
        if (assoc_by_name != NULL && assoc_by_ids != NULL) {
            assoc_by_name->port_id = nic->port_id;
            assoc_by_name->nic_index = nic->nic_index;
            struct vr_interface* interface = assoc_by_name->interface;

            assoc_by_ids->string = _name;
            assoc_by_ids->interface = interface; // This will do nothing if dp-core didn't create an interface yet, because it will be NULL

            if (interface) {
                interface->vif_port = nic->port_id;
                interface->vif_nic = nic->nic_index;
                vif_attach(interface);
            }
        } else {
            return NDIS_STATUS_RESOURCES;
        }
    }

    // TODO: REMOVE THIS

    /*vr_nexthop_req nh = { 0 };
    nh.h_op = SANDESH_OP_ADD;
    nh.nhr_rid = 0;
    nh.nhr_id = nic->port_id;
    nh.nhr_type = NH_L2_RCV; // ?? mby NH_L2_RCV
    nh.nhr_flags = NH_FLAG_VALID;
    nh.nhr_encap_size = 0;
    nh.nhr_family = AF_BRIDGE;
    nh.nhr_vrf = nic->port_id;

    vr_nexthop_req_process(&nh);

    // --------------------------------

    vr_route_req req = { 0 };
    req.h_op = SANDESH_OP_ADD;
    req.rtr_family = AF_BRIDGE;
    req.rtr_rid = 0;
    req.rtr_vrf_id = nic->port_id;
    req.rtr_mac_size = VR_ETHER_ALEN;

    req.rtr_mac = nic->mac;

    req.rtr_nh_id = nic->port_id;
    req.rtr_label_flags = VR_BE_LABEL_VALID_FLAG;

    vr_route_req_process((void*)&req);*/

    // /TODO: REMOVE THIS

    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
SxExtInitialize(PDRIVER_OBJECT DriverObject)
{
    int ret;
    DbgPrint("SxExtInitialize\r\n");
    
    vr_num_cpus = KeQueryActiveProcessorCount(NULL);
    if (!vr_num_cpus) {
        DbgPrint("%s: Failed to get processor count\n", __func__);
        return NDIS_STATUS_FAILURE;
    }
    
    NTSTATUS Status = CreateDevice(DriverObject);

    ret = vr_message_init();
    
    if (NT_ERROR(Status) || ret)
    {
	    return NDIS_STATUS_DEVICE_FAILED;
    }
    else if (!NT_SUCCESS(Status))
    {
	    DbgPrint("CreateDevice informal/warning: %d\n", Status);
    }

    return NDIS_STATUS_SUCCESS;
}

VOID
SxExtUninitialize(PDRIVER_OBJECT DriverObject)
{
    DestroyDevice(DriverObject);
    DbgPrint("SxExtUninitialize\r\n");
}

NDIS_STATUS
SxExtCreateSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Outptr_result_maybenull_ PNDIS_HANDLE *ExtensionContext
)
{
    DbgPrint("SxExtCreateSwitch\r\n");

    struct vr_switch_context *ctx = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(struct vr_switch_context), SxExtAllocationTag);
    if (!ctx) {
        DbgPrint("%s: Allocating vr_switch_context failed\n", __func__);
        return NDIS_STATUS_FAILURE;
    }
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

    if (vrouter_init()) {
        goto Failure;
    }

    if (vr_init_assoc()) {
        goto Failure;
    }

    /* DEBUG(sodar): Mock composite NH for VRF = 0*/
#if 1
    vr_nexthop_req nh = { 0 };
    nh.h_op = SANDESH_OP_ADD;
    nh.nhr_type = NH_COMPOSITE;
    nh.nhr_family = AF_BRIDGE;
    nh.nhr_id = debug_nh_composite;
    nh.nhr_rid = DEBUG_VROUTER_ID;
    nh.nhr_vrf = 0;
    nh.nhr_flags = NH_FLAG_VALID | NH_FLAG_COMPOSITE_L2 | NH_FLAG_MCAST;
    nh.nhr_nh_list = NULL;  // Pointer to nexthops table
    nh.nhr_nh_list_size = 0;  // Can be 0 according to dp-core/vr_nexthop.c:2370
    nh.nhr_label_list_size = 0;  // Must be equal to nhr_nh_list_size
    vr_nexthop_req_process(&nh);

    vr_route_req req = { 0 };
    req.h_op = SANDESH_OP_ADD;
    req.rtr_vrf_id = DEBUG_VRF;
    req.rtr_family = AF_BRIDGE;
    req.rtr_prefix = NULL;
    req.rtr_prefix_size = 0;
    req.rtr_rid = DEBUG_VROUTER_ID;
    req.rtr_nh_id = debug_nh_composite;
    req.rtr_mac = debug_bcast_mast;
    req.rtr_mac_size = VR_ETHER_ALEN;
    vr_route_req_process((void*)&req);
#endif

    return NDIS_STATUS_SUCCESS;

Failure:
    NdisFreeRWLock(AsyncWorkRWLock);
    NdisFreeNetBufferListPool(SxNBLPool);
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

    NdisFreeRWLock(AsyncWorkRWLock);
    SxSwitchObject = NULL;

    vrouter_free_pool(SxNBLPool);
    NdisFreeRWLock(((struct vr_switch_context*)ExtensionContext)->lock);
    ExFreePoolWithTag(ExtensionContext, SxExtAllocationTag);

    vr_clean_assoc();
    vrouter_exit(false);
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

        AddNicToArray(ctx, &nic, entry->NicFriendlyName);
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

    struct vr_nic nic = { 0 };
    RtlCopyMemory(nic.mac, Nic->PermanentMacAddress, sizeof(nic.mac));
    nic.nic_index = Nic->NicIndex;
    nic.nic_type = Nic->NicType;
    nic.port_id = Nic->PortId;

    AddNicToArray(ctx, &nic, Nic->NicFriendlyName);

#if 1
    /* DEBUG(sodar): Mocked nexthop for this interface. */
    int32_t nh_id = InterlockedIncrement(&debug_nh_counter);
    vr_nexthop_req nh = { 0 };
    nh.h_op = SANDESH_OP_ADD;
    nh.nhr_type = NH_L2_RCV;
    nh.nhr_family = AF_BRIDGE;
    nh.nhr_id = nh_id;
    nh.nhr_rid = DEBUG_VROUTER_ID;
    nh.nhr_vrf = DEBUG_VRF;
    nh.nhr_flags = NH_FLAG_VALID;
    vr_nexthop_req_process(&nh);

    debug_nh_elements[debug_nh_elements_count] = nh_id;
    debug_nh_elements_count++;

    /* DEBUG(sodar): Mocked nexthop update - there is no nexthops removal!!! */
    vr_nexthop_req nhc = { 0 };
    nhc.h_op = SANDESH_OP_ADD;
    nhc.nhr_type = NH_COMPOSITE;
    nhc.nhr_family = AF_BRIDGE;
    nhc.nhr_id = debug_nh_composite;
    nhc.nhr_rid = DEBUG_VROUTER_ID;
    nhc.nhr_vrf = DEBUG_VRF;
    nhc.nhr_flags = NH_FLAG_VALID | NH_FLAG_COMPOSITE_L2 | NH_FLAG_MCAST;
    nhc.nhr_nh_list = debug_nh_elements;
    nhc.nhr_nh_list_size = debug_nh_elements_count;
    nhc.nhr_label_list = debug_nh_labels;
    nhc.nhr_label_list_size = debug_nh_elements_count;  // Must be equal to nhr_nh_list_size
    vr_nexthop_req_process(&nhc);

    /* DEBUG(sodar): Mocked bridge entry for this interface */
    vr_route_req br = { 0 };
    br.h_op = SANDESH_OP_ADD;
    br.rtr_vrf_id = DEBUG_VRF;
    br.rtr_family = AF_BRIDGE;
    br.rtr_rid = 0;
    br.rtr_nh_id = nh_id;
    br.rtr_mac = Nic->PermanentMacAddress;
    br.rtr_mac_size = VR_ETHER_ALEN;
    br.rtr_label_flags = VR_BE_LABEL_VALID_FLAG;
    vr_route_req_process((void*)&br);

    /* DEBUG(sodar): Mocked vr_interface attaching on OS callbacks. */
    NDIS_IF_COUNTED_STRING vif_name = vr_get_name_from_friendly_name(Nic->NicFriendlyName);
    vr_interface_req req;

    int32_t vif_idx = InterlockedIncrement(&debug_vif_counter);

    req.h_op = SANDESH_OP_ADD;
    req.vifr_rid = DEBUG_VROUTER_ID;
    req.vifr_idx = vif_idx;
    req.vifr_os_idx = -1;
    req.vifr_type = VIF_TYPE_VIRTUAL;
    req.vifr_transport = VIF_TRANSPORT_VIRTUAL;
    req.vifr_flags = 0;  // test with no flags
    req.vifr_mir_id = 0;

    req.vifr_vrf = DEBUG_VRF;  // mocked VRF
    req.vifr_mtu = 1500;
    req.vifr_nh_id = nh_id;  // ???
    req.vifr_qos_map_index = 0;  // ???
    req.vifr_mac = Nic->PermanentMacAddress;
    req.vifr_mac_size = sizeof(unsigned char[6]);
    req.vifr_ip = 0;  // 0.0.0.0
    req.vifr_name = "testname";
    req.vifr_fat_flow_protocol_port_size = 0;  // ???

    if (!vr_interface_add(&req, false)) {
        struct vrouter *vr = vrouter_get(req.vifr_rid);
        vr_set_assoc_oid_name(vif_name, vr->vr_interfaces[req.vifr_idx]);
        vr_set_assoc_oid_ids(Nic->PortId, Nic->NicIndex, vr->vr_interfaces[req.vifr_idx]);
    }
#endif
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

#if 1
    /* DEBUG(sodar): Mocked vr_interface detaching on OS callback. */
    vr_interface_req req;

    req.h_op = SANDESH_OP_DELETE;
    req.vifr_rid = DEBUG_VROUTER_ID;
    req.vifr_idx = Nic->NicIndex * 256 + Nic->PortId;

    debug_vr_interface_delete(&req, false);   /* Calls vr_interface_delete (which is static). */
#endif
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
        struct vr_assoc *assoc_entry = vr_get_assoc_ids(source_port, source_nic);
        struct vr_interface *vif = (assoc_entry ? assoc_entry->interface : NULL);
        if (!vif) {
            /* If no vif attached yet, then drop NBL. */
            SxLibCompleteNetBufferListsIngress(Switch, curNbl, sendCompleteFlags);
            continue;
        }

        struct vr_packet *pkt = win_get_packet(curNbl, vif);
        if (pkt == NULL) {
            /* If `win_get_packet` fails, it will drop the NBL. */
            continue;
        }

        if (vif->vif_rx) {
            /* We assume that will be correctly handled by vRouter in `vif_rx` callback. */
            int rx_ret = vif->vif_rx(vif, pkt, VLAN_ID_INVALID);
            windows_host.hos_printf("%s: vif_rx returned %d\n", __func__, rx_ret);
        } else {
            /* If `vif_rx` is not set (unlikely in production), then drop the packet. */
            windows_host.hos_pfree(pkt, VP_DROP_INTERFACE_DROP);
            continue;
        }
    }

    // Release the lock, now interfaces can disconnect, etc.
    NdisReleaseRWLock(ctx->lock, &lockState);

    // Handle packet sending.
    if (nativeForwardedNbls != NULL) {
        DbgPrint("StartIngress: send native forwarded NBL\r\n");
        SxLibSendNetBufferListsIngress(Switch,
            nativeForwardedNbls,
            SendFlags,
            0);
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
        DbgPrint("Done!\r\n");
    }


    PNET_BUFFER_LIST curNbl = NetBufferLists;
    PNET_BUFFER_LIST nextNbl = NULL;

    while (curNbl != NULL)
    {
        DbgPrint("Looping...\r\n");
        nextNbl = curNbl->Next;
        struct vr_packet* pkt = win_get_packet_from_nbl(curNbl);
        if (pkt != NULL) {
            pkt->vp_net_buffer_list = NULL;
            windows_host.hos_pfree(pkt, VP_DROP_DISCARD);
        }

        curNbl = nextNbl;
    }
    DbgPrint("After looping\r\n");
}

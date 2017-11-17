#include "precomp.h"

#include "vr_interface.h"
#include "vr_packet.h"
#include "vr_windows.h"
#include "vrouter.h"
#include "windows_nbl.h"

FILTER_SEND_NET_BUFFER_LISTS FilterSendNetBufferLists;
FILTER_SEND_NET_BUFFER_LISTS_COMPLETE FilterSendNetBufferListsComplete;

static NDIS_STATUS
create_forwarding_context(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);
    return VrSwitchObject->NdisSwitchHandlers.AllocateNetBufferListForwardingContext(VrSwitchObject->NdisSwitchContext, nbl);
}

static void
free_forwarding_context(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);
    VrSwitchObject->NdisSwitchHandlers.FreeNetBufferListForwardingContext(VrSwitchObject->NdisSwitchContext, nbl);
}

static PNET_BUFFER_LIST
create_nbl_based_on_buffer(unsigned int size, void *buffer)
{
    ASSERT(buffer != NULL);
    ASSERT(size > 0);

    PMDL mdl = NULL;
    PNET_BUFFER_LIST nbl = NULL;
    NDIS_STATUS status;

    mdl = NdisAllocateMdl(VrSwitchObject->NdisFilterHandle, buffer, size);
    if (mdl == NULL)
        goto fail;
    mdl->Next = NULL;

    nbl = NdisAllocateNetBufferAndNetBufferList(VrNBLPool, 0, 0, mdl, 0, size);
    if (nbl == NULL)
        goto fail;
    nbl->SourceHandle = VrSwitchObject->NdisFilterHandle;

    status = create_forwarding_context(nbl);
    if (!NT_SUCCESS(status))
        goto fail;

    return nbl;

fail:
    if (nbl)
        NdisFreeNetBufferList(nbl);
    if (mdl)
        NdisFreeMdl(mdl);
    return NULL;
}

PNET_BUFFER_LIST
create_nbl(unsigned int size)
{
    ASSERT(size > 0);

    PNET_BUFFER_LIST nbl = NULL;
    void *buffer = NULL;

    buffer = ExAllocatePoolWithTag(NonPagedPoolNx, size, VrAllocationTag);
    if (buffer == NULL)
        goto fail;

    RtlZeroMemory(buffer, size);

    nbl = create_nbl_based_on_buffer(size, buffer);
    if (nbl == NULL)
        goto fail;

    return nbl;

fail:
    if (buffer)
        ExFreePoolWithTag(buffer, VrAllocationTag);
    return NULL;
}

void
free_cloned_nbl(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);

    free_forwarding_context(nbl);

    PNET_BUFFER_LIST original_nbl = nbl->ParentNetBufferList;

    NdisFreeCloneNetBufferList(nbl, 0);

    original_nbl->ChildRefCount--;
}

void
free_created_nbl(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);
    ASSERT(nbl->Next == NULL);

    PNET_BUFFER nb = NULL;
    PMDL mdl = NULL;
    PMDL mdl_next = NULL;
    PVOID data = NULL;

    free_forwarding_context(nbl);

    /* Free MDLs associated with NET_BUFFERS */
    for (nb = NET_BUFFER_LIST_FIRST_NB(nbl); nb != NULL; nb = NET_BUFFER_NEXT_NB(nb))
        for (mdl = NET_BUFFER_FIRST_MDL(nb); mdl != NULL; mdl = mdl_next) {
            mdl_next = mdl->Next;
            data = MmGetSystemAddressForMdlSafe(mdl, LowPagePriority | MdlMappingNoExecute);
            NdisFreeMdl(mdl);
            if (data != NULL)
                ExFreePool(data);
        }

    NdisFreeNetBufferList(nbl);
}

static void
complete_received_nbl(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);

    /* Flag SINGLE_SOURCE is used, because of singular NBLS */
    NdisFSendNetBufferListsComplete(VrSwitchObject->NdisFilterHandle,
        nbl,
        NDIS_SEND_COMPLETE_FLAGS_SWITCH_SINGLE_SOURCE | NDIS_SEND_FLAGS_SWITCH_DESTINATION_GROUP);
}

void
free_nbl(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);
    ASSERTMSG("A non-singular NBL made it's way into the process", nbl->Next == NULL);

    struct vr_packet* pkt = (struct vr_packet*) NET_BUFFER_LIST_CONTEXT_DATA_START(nbl);

    pkt->vp_ref_cnt--;

    if (pkt->vp_ref_cnt == 0) {
        NdisFreeNetBufferListContext(nbl, VR_NBL_CONTEXT_SIZE);

        if (IS_OWNED(nbl)) {
            if (IS_CLONE(nbl))
                free_cloned_nbl(nbl);
            else
                free_created_nbl(nbl);

            PNET_BUFFER_LIST parent = nbl->ParentNetBufferList;
            if (parent)
                free_nbl(parent);
        }
        else
            complete_received_nbl(nbl);
    }
}

static void
free_associated_nbl(struct vr_packet* pkt)
{
    ASSERT(pkt != NULL);

    PNET_BUFFER_LIST nbl = pkt->vp_net_buffer_list;

    ASSERT(nbl != NULL);

    free_nbl(nbl);
}

void
delete_unbound_nbl(PNET_BUFFER_LIST nbl, unsigned long flags)
{
    ASSERT(nbl != NULL);

    NdisFSendNetBufferListsComplete(VrSwitchObject->NdisFilterHandle,
        nbl,
        flags);
}

PNET_BUFFER_LIST
clone_nbl(PNET_BUFFER_LIST original_nbl)
{
    ASSERT(original_nbl != NULL);

    BOOLEAN fwd_ctx = false;

    PNET_BUFFER_LIST nbl = NdisAllocateCloneNetBufferList(original_nbl, VrNBLPool, NULL, 0);

    if (nbl == NULL)
        goto cleanup;

    nbl->SourceHandle = VrSwitchObject->NdisFilterHandle;
    nbl->ParentNetBufferList = original_nbl;
    original_nbl->ChildRefCount++;

    if (create_forwarding_context(nbl) != NDIS_STATUS_SUCCESS)
        goto cleanup;

    fwd_ctx = true;

    NDIS_STATUS status = VrSwitchObject->NdisSwitchHandlers.CopyNetBufferListInfo(VrSwitchObject->NdisSwitchContext, nbl, original_nbl, 0);
    if (status != NDIS_STATUS_SUCCESS)
        goto cleanup;

    return nbl;

cleanup:
    if (fwd_ctx) {
        free_forwarding_context(nbl);
    }

    if (nbl) {
        NdisFreeCloneNetBufferList(nbl, 0);
        original_nbl->ChildRefCount--;
    }

    return NULL;
}

struct vr_packet *
win_allocate_packet(void *buffer, unsigned int size)
{
    ASSERT(size > 0);

    PNET_BUFFER_LIST nbl = NULL;
    struct vr_packet *pkt = NULL;
    unsigned char *ptr = NULL;

    if (buffer != NULL) {
        nbl = create_nbl_based_on_buffer(size, buffer);
    } else {
        nbl = create_nbl(size);
    }
    if (nbl == NULL)
        goto fail;

    pkt = win_get_packet(nbl, NULL);
    if (pkt == NULL)
        goto fail;

    if (buffer != NULL) {
        ptr = pkt_pull_tail(pkt, size);
        if (ptr == NULL)
            goto fail;
    }

    return pkt;

fail:
    if (pkt)
        NdisFreeNetBufferListContext(nbl, VR_NBL_CONTEXT_SIZE);
    if (nbl)
        free_created_nbl(nbl);
    return NULL;
}

void
win_free_packet(struct vr_packet *pkt)
{
    ASSERT(pkt != NULL);

    free_associated_nbl(pkt);
}

static VOID
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

static struct vr_interface *
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

VOID
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
        DbgPrint("%s: port %d and interface id %d\n", __func__, source_port, source_nic);

        struct vr_interface *vif = get_vif(source_port, source_nic);

        if (!vif) {
            // If no vif attached yet, then drop NBL.
            DbgPrint("%s: No vif found\n", __func__);
            NdisFSendNetBufferListsComplete(Switch->NdisFilterHandle, curNbl, sendCompleteFlags);
            continue;
        }

        DbgPrint("%s: VIF has port %d and interface id %d\n", __func__, vif->vif_port, vif->vif_nic);

        struct vr_packet *pkt = win_get_packet(curNbl, vif);

        DbgPrint("%s: Got pkt\n", __func__);
        ASSERTMSG("win_get_packed failed!", pkt != NULL);

        if (pkt == NULL) {
            /* If `win_get_packet` fails, it will drop the NBL. */
            DbgPrint("%s: pkt is NULL\n", __func__);
            NdisFSendNetBufferListsComplete(Switch->NdisFilterHandle, curNbl, sendCompleteFlags);
            continue;
        }

        ASSERTMSG("VIF doesn't have a vif_rx method set!", vif->vif_rx != NULL);

        if (vif->vif_rx) {
            DbgPrint("%s: Calling vif_rx", __func__);
            int rx_ret = vif->vif_rx(vif, pkt, VLAN_ID_INVALID);

            DbgPrint("%s: vif_rx returned %d\n", __func__, rx_ret);
        }
        else {
            DbgPrint("%s: vif_rx is NULL\n", __func__);
            /* If `vif_rx` is not set (unlikely in production), then drop the packet. */
            vr_pfree(pkt, VP_DROP_INTERFACE_DROP);
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

VOID
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

        free_nbl(current);
    } while (next != NULL);
}

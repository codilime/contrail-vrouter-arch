#include "precomp.h"

#include "vr_interface.h"
#include "vr_packet.h"
#include "vr_windows.h"
#include "vrouter.h"
#include "windows_nbl.h"

FILTER_SEND_NET_BUFFER_LISTS FilterSendNetBufferLists;
FILTER_SEND_NET_BUFFER_LISTS_COMPLETE FilterSendNetBufferListsComplete;

static NDIS_STATUS
CreateForwardingContext(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);
    return VrSwitchObject->NdisSwitchHandlers.AllocateNetBufferListForwardingContext(
        VrSwitchObject->NdisSwitchContext, nbl);
}

static void
FreeForwardingContext(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);
    VrSwitchObject->NdisSwitchHandlers.FreeNetBufferListForwardingContext(
        VrSwitchObject->NdisSwitchContext, nbl);
}

static PNET_BUFFER_LIST
CreateNetBufferListUsingBuffer(unsigned int bytesCount, void *buffer)
{
    ASSERT(bytesCount > 0);
    ASSERT(buffer != NULL);

    PMDL mdl = NdisAllocateMdl(VrSwitchObject->NdisFilterHandle, buffer, bytesCount);
    if (mdl == NULL)
        return NULL;
    mdl->Next = NULL;

    PNET_BUFFER_LIST nbl = NdisAllocateNetBufferAndNetBufferList(VrNBLPool, 0, 0, mdl, 0, bytesCount);
    if (nbl == NULL)
        goto fail;
    nbl->SourceHandle = VrSwitchObject->NdisFilterHandle;

    NDIS_STATUS status = CreateForwardingContext(nbl);
    if (!NT_SUCCESS(status))
        goto fail;

    return nbl;

fail:
    if (nbl != NULL)
        NdisFreeNetBufferList(nbl);
    if (mdl != NULL)
        NdisFreeMdl(mdl);
    return NULL;
}

PNET_BUFFER_LIST
CreateNetBufferList(unsigned int bytesCount)
{
    ASSERT(bytesCount > 0);

    if (bytesCount == 0)
        return NULL;

    void *buffer = ExAllocatePoolWithTag(NonPagedPoolNx, bytesCount, VrAllocationTag);
    if (buffer == NULL)
        return NULL;
    RtlZeroMemory(buffer, bytesCount);

    PNET_BUFFER_LIST nbl = CreateNetBufferListUsingBuffer(bytesCount, buffer);
    if (nbl == NULL) {
        ExFreePool(buffer);
        return NULL;
    }

    return nbl;
}

LONG
FreeClonedNetBufferList(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);
    ASSERT(nbl->ParentNetBufferList != NULL);

    PNET_BUFFER_LIST parentNbl = nbl->ParentNetBufferList;

    FreeForwardingContext(nbl);
    NdisFreeCloneNetBufferList(nbl, 0);

    return InterlockedDecrement(&parentNbl->ChildRefCount);
}

VOID
FreeCreatedNetBufferList(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);
    ASSERTMSG("A non-singular NBL made it's way into the process", nbl->Next == NULL);

    PNET_BUFFER nb = NULL;
    PMDL mdl = NULL;
    PMDL mdlNext = NULL;
    PVOID data = NULL;

    FreeForwardingContext(nbl);

    /* Free MDLs associated with NET_BUFFERS */
    for (nb = NET_BUFFER_LIST_FIRST_NB(nbl); nb != NULL; nb = NET_BUFFER_NEXT_NB(nb))
        for (mdl = NET_BUFFER_FIRST_MDL(nb); mdl != NULL; mdl = mdlNext) {
            mdlNext = mdl->Next;
            data = MmGetSystemAddressForMdlSafe(mdl, LowPagePriority | MdlMappingNoExecute);
            NdisFreeMdl(mdl);
            if (data != NULL)
                ExFreePool(data);
        }

    NdisFreeNetBufferList(nbl);
}

static VOID
CompleteReceivedNetBufferList(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);

    /* Flag SINGLE_SOURCE is used, because of singular NBLS */
    NdisFSendNetBufferListsComplete(VrSwitchObject->NdisFilterHandle, nbl,
        NDIS_SEND_COMPLETE_FLAGS_SWITCH_SINGLE_SOURCE | NDIS_SEND_FLAGS_SWITCH_DESTINATION_GROUP);
}

VOID
FreeNetBufferList(PNET_BUFFER_LIST nbl)
{
    ASSERT(nbl != NULL);
    ASSERTMSG("A non-singular NBL made it's way into the process", nbl->Next == NULL);

    if (nbl->ChildRefCount > 0) {
        return;
    }

    struct vr_packet *pkt = GetVrPacketFromNetBufferList(nbl);

    if (pkt->vp_net_buffer_list != NULL) {
        // NBL has no children, but pkt is still owned by dp-core
        return;
    }

    if (vr_sync_sub_and_fetch_32u(&pkt->vp_ref_cnt, 1) == 0) {
        NdisFreeNetBufferListContext(nbl, VR_NBL_CONTEXT_SIZE);

        if (IS_NBL_OWNED(nbl)) {
            if (IS_NBL_CLONE(nbl)) {
                PNET_BUFFER_LIST parent = nbl->ParentNetBufferList;
                if (FreeClonedNetBufferList(nbl) == 0) {
                    FreeNetBufferList(parent);
                }
            } else {
                FreeCreatedNetBufferList(nbl);
            }
        } else {
            CompleteReceivedNetBufferList(nbl);
        }
    }
}

PNET_BUFFER_LIST
CloneNetBufferList(PNET_BUFFER_LIST originalNbl)
{
    ASSERT(originalNbl != NULL);

    BOOLEAN contextCreated = false;
    PNET_BUFFER_LIST newNbl = NdisAllocateCloneNetBufferList(originalNbl, VrNBLPool, NULL, 0);
    if (newNbl == NULL)
        goto cleanup;

    newNbl->SourceHandle = VrSwitchObject->NdisFilterHandle;
    newNbl->ParentNetBufferList = originalNbl;
    InterlockedIncrement(&originalNbl->ChildRefCount);

    if (CreateForwardingContext(newNbl) != NDIS_STATUS_SUCCESS)
        goto cleanup;
    contextCreated = true;

    NDIS_STATUS status = VrSwitchObject->NdisSwitchHandlers.CopyNetBufferListInfo(
        VrSwitchObject->NdisSwitchContext, newNbl, originalNbl, 0);
    if (status != NDIS_STATUS_SUCCESS)
        goto cleanup;

    return newNbl;

cleanup:
    if (contextCreated)
        FreeForwardingContext(newNbl);

    if (newNbl) {
        NdisFreeCloneNetBufferList(newNbl, 0);
        InterlockedDecrement(&originalNbl->ChildRefCount);
    }

    return NULL;
}

void
GetVrPacketMapFromMdl(struct vr_packet *pkt, PMDL mdl, ULONG mdlOffset, ULONG dataLength)
{
    pkt->vp_head = (unsigned char*) MmGetSystemAddressForMdlSafe(mdl, LowPagePriority | MdlMappingNoExecute);
    if (!pkt->vp_head) {
        pkt->vp_end = pkt->vp_data = pkt->vp_tail = pkt->vp_len = 0;
        return;
    }

    pkt->vp_head += mdlOffset;
    /* vp_data is the offset from vp_head, where packet begins.
       TODO: When packet encapsulation comes into play, then vp_data should differ.
             There should be enough room between vp_head and vp_data to add packet headers.
    */
    pkt->vp_data = 0;

    // left_mdl_space is a space from begin of data section to the end of mdl
    ULONG leftMdlSpace = MmGetMdlByteCount(mdl) - mdlOffset;
    pkt->vp_tail = pkt->vp_len = (dataLength < leftMdlSpace ? dataLength : leftMdlSpace);
    pkt->vp_end = leftMdlSpace;

    return;
}

struct vr_packet *
GetVrPacketFromNetBufferList(PNET_BUFFER_LIST nbl)
{
    return (struct vr_packet *)NET_BUFFER_LIST_CONTEXT_DATA_START(nbl);
}

struct vr_packet *
AllocateVrPacketForNetBufferList(PNET_BUFFER_LIST nbl, struct vr_interface *vif)
{
    ASSERT(nbl != NULL);

    /* Allocate NDIS context, which will store vr_packet pointer */
    NDIS_STATUS status = NdisAllocateNetBufferListContext(nbl, VR_NBL_CONTEXT_SIZE, 0, VrAllocationTag);
    if (status != NDIS_STATUS_SUCCESS)
        return NULL;

    struct vr_packet *pkt = GetVrPacketFromNetBufferList(nbl);
    ASSERT(pkt != NULL);

    RtlZeroMemory(pkt, sizeof(*pkt));
    pkt->vp_net_buffer_list = nbl;
    pkt->vp_ref_cnt = 1;
    pkt->vp_cpu = (unsigned char)KeGetCurrentProcessorNumberEx(NULL);

    /* vp_head points to the beginning of accesible non-paged memory of the packet */
    PNET_BUFFER nb = NET_BUFFER_LIST_FIRST_NB(nbl);
    ULONG dataLength = NET_BUFFER_DATA_LENGTH(nb);

    if (IS_NBL_OWNED(nbl) && !IS_NBL_CLONE(nbl)) {
        dataLength = 0;
    }

    GetVrPacketMapFromMdl(pkt, NET_BUFFER_CURRENT_MDL(nb),
                            NET_BUFFER_CURRENT_MDL_OFFSET(nb),
                            dataLength);

    if (!pkt->vp_head) {
        NdisFreeNetBufferListContext(nbl, VR_NBL_CONTEXT_SIZE);
        return NULL;    
    }

    pkt->vp_if = vif;
    pkt->vp_network_h = pkt->vp_inner_network_h = 0;
    pkt->vp_nh = NULL;
    pkt->vp_flags = 0;

    // If a problem arises concerning IP checksums, tinker with:
    // if (skb->ip_summed == CHECKSUM_PARTIAL)
    //	pkt->vp_flags |= VP_FLAG_CSUM_PARTIAL;

    pkt->vp_ttl = VP_DEFAULT_INITIAL_TTL;
    pkt->vp_type = VP_TYPE_NULL;
    pkt->vp_queue = 0;
    pkt->vp_priority = 0;  /* PCP Field from IEEE 802.1Q. vp_priority = 0 is a default value for this. */

    return pkt;
}

struct vr_packet *
AllocateVrPacket(void *buffer, unsigned int size)
{
    ASSERT(size > 0);

    PNET_BUFFER_LIST nbl = NULL;
    struct vr_packet *pkt = NULL;
    unsigned char *ptr = NULL;

    if (buffer != NULL) {
        nbl = CreateNetBufferListUsingBuffer(size, buffer);
    } else {
        nbl = CreateNetBufferList(size);
    }
    if (nbl == NULL)
        goto fail;

    pkt = AllocateVrPacketForNetBufferList(nbl, NULL);
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
        FreeCreatedNetBufferList(nbl);
    return NULL;
}

void
FreeVrPacket(struct vr_packet *pkt)
{
    ASSERT(pkt != NULL);
    PNET_BUFFER_LIST nbl = pkt->vp_net_buffer_list;
    pkt->vp_net_buffer_list = NULL;
    FreeNetBufferList(nbl);
}

static VOID
SplitNetBufferListsByForwardingType(
    PNET_BUFFER_LIST nbl,
    PNET_BUFFER_LIST *nextExtForwardNbl,
    PNET_BUFFER_LIST *nextNativeForwardedNbl)
{
    PNET_BUFFER_LIST curNbl;
    PNET_BUFFER_LIST nextNbl;
    PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwdDetail;

    // Divide the NBL into two: part which requires native forwarding and the rest
    for (curNbl = nbl; curNbl != NULL; curNbl = nextNbl) {
        // Rememeber the next NBL
        nextNbl = curNbl->Next;

        fwdDetail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(curNbl);
        if (fwdDetail->NativeForwardingRequired) {
            // Set the next NBL to current NBL. This pointer points to either first pointer to
            // native forwarded NBL or the "Next" field of the last one.
            *nextNativeForwardedNbl = curNbl;
            nextNativeForwardedNbl = &(curNbl->Next);
        } else {
            // Set the next NBL to current NBL. This pointer points to either first pointer to
            // non-native forwarded NBL or the "Next" field of the last one.
            *nextExtForwardNbl = curNbl;
            nextExtForwardNbl = &(curNbl->Next);
        }
    }
}

static struct vr_interface *
GetAssociatedVrInterface(NDIS_SWITCH_PORT_ID vifPort, NDIS_SWITCH_NIC_INDEX vifNic)
{
    struct vrouter *vrouter = vrouter_get(0);
    ASSERT(vrouter != NULL);

    for (int i = 0; i < vrouter->vr_max_interfaces; i++) {
        struct vr_interface* vif = vrouter->vr_interfaces[i];

        if (vif == NULL)
            continue;

        if (vif->vif_port == vifPort && vif->vif_nic == vifNic)
            return vif;
    }

    // VIF is not registered, very temporary state
    return NULL;
}

VOID
FilterSendNetBufferLists(
    NDIS_HANDLE filterModuleContext,
    PNET_BUFFER_LIST netBufferLists,
    NDIS_PORT_NUMBER portNumber,
    ULONG sendFlags)
{
    PSWITCH_OBJECT switchObject = (PSWITCH_OBJECT)filterModuleContext;

    LOCK_STATE_EX lockState;

    BOOLEAN sameSource;
    ULONG sendCompleteFlags = 0;
    BOOLEAN onDispatchLevel;

    PNET_BUFFER_LIST extForwardedNbls = NULL;  // NBLs forwarded by extension.
    PNET_BUFFER_LIST nativeForwardedNbls = NULL;  // NBLs that require native forwarding - extension just sends them.
    PNET_BUFFER_LIST curNbl = NULL;
    PNET_BUFFER_LIST nextNbl = NULL;

    UNREFERENCED_PARAMETER(portNumber);

    // True if packets come from the same switch source port.
    sameSource = NDIS_TEST_SEND_FLAG(sendFlags, NDIS_SEND_FLAGS_SWITCH_SINGLE_SOURCE);
    if (sameSource) {
        sendCompleteFlags |= NDIS_SEND_COMPLETE_FLAGS_SWITCH_SINGLE_SOURCE;
    }

    // Forward DISPATCH_LEVEL flag.
    onDispatchLevel = NDIS_TEST_SEND_FLAG(sendFlags, NDIS_SEND_FLAGS_DISPATCH_LEVEL);
    if (onDispatchLevel) {
        sendCompleteFlags |= NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL;
    }

    if (switchObject->Running == FALSE) {
        NdisFSendNetBufferListsComplete(switchObject->NdisFilterHandle, netBufferLists, sendCompleteFlags);
        return;
    }

    // Acquire the lock, now interfaces cannot disconnect, etc.
    NdisAcquireRWLockRead(switchObject->ExtensionContext->lock, &lockState, onDispatchLevel);

    SplitNetBufferListsByForwardingType(netBufferLists, &extForwardedNbls, &nativeForwardedNbls);

    for (curNbl = extForwardedNbls; curNbl != NULL; curNbl = nextNbl) {
        ASSERTMSG("Incoming NBL already has some clones ", curNbl->ChildRefCount == 0);

        /* Save next NBL, because after passing control to vRouter it might drop curNbl.
        Also vRouter handles packets one-by-one, so we operate on single NBLs.
        */
        nextNbl = curNbl->Next;
        curNbl->Next = NULL;

        PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwd_detail;
        NDIS_SWITCH_PORT_ID source_port;
        NDIS_SWITCH_NIC_INDEX source_nic;
        struct vr_interface *vif;
        struct vr_packet *pkt;

        fwd_detail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(curNbl);
        source_port = fwd_detail->SourcePortId;
        source_nic = fwd_detail->SourceNicIndex;

        vif = GetAssociatedVrInterface(source_port, source_nic);
        if (vif == NULL) {
            // If no vif attached yet, then drop NBL.
            NdisFSendNetBufferListsComplete(switchObject->NdisFilterHandle,
                curNbl, sendCompleteFlags);
            continue;
        }

        pkt = AllocateVrPacketForNetBufferList(curNbl, vif);
        ASSERTMSG("win_get_packed failed!", pkt != NULL);
        if (pkt == NULL) {
            // If allocating vr_packed has failed, we drop the NBL.
            NdisFSendNetBufferListsComplete(switchObject->NdisFilterHandle,
                curNbl, sendCompleteFlags);
            continue;
        }

        ASSERTMSG("VIF doesn't have a vif_rx method set!", vif->vif_rx != NULL);
        if (vif->vif_rx) {
            vif->vif_rx(vif, pkt, VLAN_ID_INVALID);
        } else {
            // If `vif_rx` is not set (unlikely in production), then drop the packet.
            vr_pfree(pkt, VP_DROP_INTERFACE_DROP);
            continue;
        }
    }

    if (nativeForwardedNbls != NULL) {
        NdisFSendNetBufferLists(switchObject->NdisFilterHandle,
            nativeForwardedNbls,
            NDIS_DEFAULT_PORT_NUMBER,
            sendFlags);
    }

    // Release the lock, now interfaces can disconnect, etc.
    NdisReleaseRWLock(switchObject->ExtensionContext->lock, &lockState);
}

VOID
FilterSendNetBufferListsComplete(
    NDIS_HANDLE filterModuleContext,
    PNET_BUFFER_LIST netBufferLists,
    ULONG sendCompleteFlags)
{
    PNET_BUFFER_LIST next = netBufferLists;
    PNET_BUFFER_LIST current;

    UNREFERENCED_PARAMETER(filterModuleContext);
    UNREFERENCED_PARAMETER(sendCompleteFlags);

    do {
        current = next;
        next = current->Next;
        current->Next = NULL;

        PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwdDetail;
        fwdDetail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(current);
        if (fwdDetail->NativeForwardingRequired) {
            CompleteReceivedNetBufferList(current);
            DbgPrint("!!! Internal packed found in FilterSendNetBufferListComplete\r\n");
        } else {
            struct vr_packet *pkt = GetVrPacketFromNetBufferList(current);
            ASSERTMSG("Completed NBLS should already be marked as garbage ", pkt->vp_net_buffer_list == NULL);

            FreeNetBufferList(current);
        }
    } while (next != NULL);
}

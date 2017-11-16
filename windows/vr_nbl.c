#include "precomp.h"
#include "vr_windows.h"

#include "vrouter.h"
#include "vr_interface.h"
#include "vr_packet.h"

FILTER_SEND_NET_BUFFER_LISTS FilterSendNetBufferLists;
FILTER_SEND_NET_BUFFER_LISTS_COMPLETE FilterSendNetBufferListsComplete;

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

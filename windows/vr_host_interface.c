#include "precomp.h"

#include "vr_interface.h"
#include "vr_packet.h"
#include "vr_windows.h"
#include "vr_devices.h"

extern PSX_SWITCH_OBJECT SxSwitchObject;
static NDIS_MUTEX win_if_mutex;

void
win_if_lock(void)
{
    NDIS_WAIT_FOR_MUTEX(&win_if_mutex);
}

void
win_if_unlock(void)
{
    NDIS_RELEASE_MUTEX(&win_if_mutex);
}

static int
win_if_add(struct vr_interface* vif)
{
    if (vif->vif_type == VIF_TYPE_STATS)
        return 0;

    if (vif->vif_name[0] == '\0')
        return -ENODEV;

    // Unlike FreeBSD/Linux, we don't have to register handlers here

    return 0;
}

static int
win_if_add_tap(struct vr_interface* vif)
{
    UNREFERENCED_PARAMETER(vif);
    // NOOP - no bridges on Windows
    return 0;
}

static int
win_if_del(struct vr_interface *vif)
{
    struct vr_assoc *assoc_by_name;
    struct vr_assoc *assoc_by_ids;

    assoc_by_ids = vr_find_assoc_ids(vif->vif_port, vif->vif_nic);
    if (assoc_by_ids != NULL) {
        assoc_by_ids->interface = NULL;

        assoc_by_name = vr_find_assoc_by_name(assoc_by_ids->string);
        if (assoc_by_name != NULL) {
            assoc_by_name->interface = NULL;
        }
    }

    return 0;
}

static int
win_if_del_tap(struct vr_interface *vif)
{
    UNREFERENCED_PARAMETER(vif);
    // NOOP - no bridges on Windows; most *_drv_del function which call if_del_tap
    // also call if_del
    return 0;
}

static void
fix_ip_csum_at_offset(struct vr_packet *pkt, unsigned offset)
{
    struct vr_ip *iph;

    ASSERT(0 < offset);

    iph = (struct vr_ip *)pkt_data_at_offset(pkt, offset);
    iph->ip_csum = vr_ip_csum(iph);
}

static void
zero_ip_csum_at_offset(struct vr_packet *pkt, unsigned offset)
{
    struct vr_ip *iph;

    ASSERT(0 < offset);

    iph = (struct vr_ip *)pkt_data_at_offset(pkt, offset);
    iph->ip_csum = 0;
}

static void
fix_tunneled_ip_csum(struct vr_packet *pkt, ULONG offloading)
{
    // doesn't compute inner UDP/TCP checksum
    // for more info look inside dpdk_sw_checksum_at_offset

    if (offloading)
        zero_ip_csum_at_offset(pkt, sizeof(struct vr_eth));
    else
        fix_ip_csum_at_offset(pkt, sizeof(struct vr_eth));

    if (pkt->vp_type == VP_TYPE_IPOIP)
        fix_ip_csum_at_offset(pkt, pkt_get_inner_network_header_off(pkt));
}

static int
__win_if_tx(struct vr_interface *vif, struct vr_packet *pkt)
{
    PNET_BUFFER_LIST nbl = pkt->vp_net_buffer_list;

    NDIS_SWITCH_PORT_DESTINATION newDestination = { 0 };

    newDestination.PortId = vif->vif_port;
    newDestination.NicIndex = vif->vif_nic;
    DbgPrint("Adding target, PID: %u, NID: %u\r\n", newDestination.PortId, newDestination.NicIndex);

    SxSwitchObject->NdisSwitchHandlers.AddNetBufferListDestination(SxSwitchObject->NdisSwitchContext, nbl, &newDestination);

    PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwd = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(nbl);
    fwd->IsPacketDataSafe = TRUE;

    NdisAdvanceNetBufferListDataStart(nbl, pkt->vp_data + pkt->vp_win_data, TRUE, NULL);
    pkt->vp_win_data = 0;

    if (vr_pkt_type_is_overlay(pkt->vp_type)) {
        NDIS_TCP_IP_CHECKSUM_NET_BUFFER_LIST_INFO checksumInfo;
        checksumInfo.Value = NET_BUFFER_LIST_INFO(nbl, TcpIpChecksumNetBufferListInfo);
        fix_tunneled_ip_csum(pkt, checksumInfo.Transmit.IpHeaderChecksum);
    }

    NdisFSendNetBufferLists(SxSwitchObject->NdisFilterHandle,
        nbl,
        NDIS_DEFAULT_PORT_NUMBER,
        0);

    ExFreePoolWithTag(pkt, SxExtAllocationTag);

    return 0;
}

static int
win_if_tx(struct vr_interface *vif, struct vr_packet* pkt)
{
    windows_host.hos_printf("%s: Got pkt\n", __func__);
    if (vif == NULL) {
        free_nbl(pkt->vp_net_buffer_list, pkt->vp_win_data_tag);
        return 0; // Sent into /dev/null
    }

    if (vif->vif_type == VIF_TYPE_AGENT)
        return pkt0_if_tx(vif, pkt);
    else
        return __win_if_tx(vif, pkt);
}

static int
win_if_rx(struct vr_interface *vif, struct vr_packet* pkt)
{
    windows_host.hos_printf("%s: Got pkt\n", __func__);

    // Since we are operating from virtual switch's PoV and not from OS's PoV, RXing is the same as TXing
    // On Linux, we receive the packet as an OS, but in Windows we are a switch to we simply push the packet to OS's networking stack
    // See vhost_tx for reference (it calls hif_ops->hif_rx)

    win_if_tx(vif, pkt);

    return 0;
}

static int
win_if_get_settings(struct vr_interface *vif, struct vr_interface_settings *settings)
{
    UNREFERENCED_PARAMETER(vif);
    UNREFERENCED_PARAMETER(settings);

    /* TODO: Implement */
    DbgPrint("%s(): dummy implementation called\n", __func__);

    return -EINVAL;
}

static unsigned int
win_if_get_mtu(struct vr_interface *vif)
{
    UNREFERENCED_PARAMETER(vif);

    /* TODO: Implement */
    DbgPrint("%s(): dummy implementation called\n", __func__);

    return vif->vif_mtu;
}

static unsigned short
win_if_get_encap(struct vr_interface *vif)
{
    UNREFERENCED_PARAMETER(vif);

    /* TODO: Implement */
    DbgPrint("%s(): dummy implementation called\n", __func__);

    return VIF_ENCAP_TYPE_ETHER;
}

static struct vr_host_interface_ops win_host_interface_ops = {
    .hif_lock           = win_if_lock,
    .hif_unlock         = win_if_unlock,
    .hif_add            = win_if_add,
    .hif_del            = win_if_del,
    .hif_add_tap        = win_if_add_tap,
    .hif_del_tap        = win_if_del_tap,
    .hif_tx             = win_if_tx,
    .hif_rx             = win_if_rx,
    .hif_get_settings   = win_if_get_settings,
    .hif_get_mtu        = win_if_get_mtu,
    .hif_get_encap      = win_if_get_encap,
    .hif_stats_update   = NULL,
};

void
vr_host_vif_init(struct vrouter *router)
{
    UNREFERENCED_PARAMETER(router);
}

void
vr_host_interface_exit(void)
{
    /* Noop */
}

struct vr_host_interface_ops *
vr_host_interface_init(void)
{
    NDIS_INIT_MUTEX(&win_if_mutex);

    return &win_host_interface_ops;
}

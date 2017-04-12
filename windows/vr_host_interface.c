#include "precomp.h"

#include "vr_interface.h"
#include "vr_packet.h"
#include "vr_windows.h"

extern PSX_SWITCH_OBJECT SxSwitchObject;
static PNDIS_RW_LOCK_EX win_if_mutex;
static LOCK_STATE_EX win_if_mutex_state;

static void
win_if_lock(void)
{
    NdisAcquireRWLockWrite(win_if_mutex, &win_if_mutex_state, 0);
}

static void
win_if_unlock(void)
{
    NdisReleaseRWLock(win_if_mutex, &win_if_mutex_state);
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

    assoc_by_name = vr_get_assoc_by_name(vif->vif_name);
    if (assoc_by_name != NULL) {
        assoc_by_name->interface = NULL;
    }

    assoc_by_ids = vr_get_assoc_ids(vif->vif_port, vif->vif_nic);
    if (assoc_by_ids != NULL) {
        assoc_by_ids->interface = NULL;
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

static int
win_if_tx(struct vr_interface *vif, struct vr_packet* pkt)
{
    windows_host.hos_printf("%s: Got pkt\n", __func__);
    if (vif == NULL)
        return 0; // Sent into /dev/null

    PNET_BUFFER_LIST nbl = pkt->vp_net_buffer_list;

    NDIS_SWITCH_PORT_DESTINATION newDestination = { 0 };

    newDestination.PortId = vif->vif_port;
    newDestination.NicIndex = vif->vif_nic;
    DbgPrint("Adding target, PID: %u, NID: %u\r\n", newDestination.PortId, newDestination.NicIndex);

    SxSwitchObject->NdisSwitchHandlers.AddNetBufferListDestination(SxSwitchObject->NdisSwitchContext, nbl, &newDestination);

    PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwd = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(nbl);
    fwd->IsPacketDataSafe = TRUE;

    NdisFSendNetBufferLists(SxSwitchObject->NdisFilterHandle,
        nbl,
        NDIS_DEFAULT_PORT_NUMBER,
        0);

    ExFreePoolWithTag(pkt, SxExtAllocationTag);

    return 0;
}

static int
win_if_rx(struct vr_interface *vif, struct vr_packet* pkt)
{
    windows_host.hos_printf("%s: Got pkt\n", __func__);

    // Since we are operating from virtual switch's PoV and not from OS's PoV, RXing is the same as TXing
    // On Linux, we receive the packet as an OS, but in Windows we are a switch to we simply push the packet to OS's networking stack
    // See linux_if_rx for reference

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
    NdisFreeRWLock(win_if_mutex);
}

struct vr_host_interface_ops *
vr_host_interface_init(void)
{
    win_if_mutex = NdisAllocateRWLock(SxSwitchObject->NdisFilterHandle);
    if (!win_if_mutex) {
        DbgPrint("%s(): RWLock could not be allocated\n", __func__);
        return NULL;
    }

    return &win_host_interface_ops;
}

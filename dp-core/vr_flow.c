/*
 * vr_flow.c -- flow handling
 *
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */
#include <vr_os.h>
#include <vr_types.h>
#include <vrouter.h>
#include <vr_packet.h>
#include <vr_htable.h>
#include <vr_flow.h>
#include <vr_mirror.h>
#include "vr_interface.h"
#include "vr_sandesh.h"
#include "vr_message.h"
#include "vr_btable.h"
#include "vr_fragment.h"
#include "vr_datapath.h"
#include "vr_hash.h"
#include "vr_ip_mtrie.h"

#define VR_NUM_FLOW_TABLES          1
#define VR_DEF_FLOW_ENTRIES         (512 * 1024)

#define VR_NUM_OFLOW_TABLES         1
#define VR_DEF_OFLOW_ENTRIES        (8 * 1024)

#define VR_MAX_FLOW_TABLE_HOLD_COUNT \
                                    4096

unsigned int vr_flow_entries = VR_DEF_FLOW_ENTRIES;
unsigned int vr_oflow_entries = VR_DEF_OFLOW_ENTRIES;

/*
 * host can provide its own memory . Point in case is the DPDK. In DPDK,
 * we allocate the table from hugepages and just ask the flow module to
 * use those tables
 */
void *vr_flow_table;
void *vr_oflow_table;
/*
 * The flow table memory can also be a file that could be mapped. The path
 * is set by somebody and passed to agent for it to map
 */
unsigned char *vr_flow_path;
unsigned int vr_flow_hold_limit = 1;

#if defined(__linux__) && defined(__KERNEL__)
extern unsigned short vr_flow_major;
#endif

uint32_t vr_hashrnd = 0;
int hashrnd_inited = 0;

static void vr_flush_entry(struct vrouter *, struct vr_flow_entry *,
        struct vr_flow_md *, struct vr_forwarding_md *);
static void __vr_flow_flush_hold_queue(struct vrouter *, struct vr_flow_entry *,
        struct vr_forwarding_md *, struct vr_flow_queue *);
static void vr_flow_set_forwarding_md(struct vrouter *, struct vr_flow_entry *,
        unsigned int, struct vr_forwarding_md *);
static int
__vr_flow_schedule_transition(struct vrouter *, struct vr_flow_entry *,
        unsigned int, unsigned short);

struct vr_flow_entry *vr_find_flow(struct vrouter *, struct vr_flow *,
        uint8_t, unsigned int *);
unsigned int vr_trap_flow(struct vrouter *, struct vr_flow_entry *,
        struct vr_packet *, unsigned int, struct vr_flow_stats *);

void get_random_bytes(void *buf, int nbytes);

#ifdef __FreeBSD__
uint32_t
jhash(void *key, uint32_t length, uint32_t initval);
#endif

#ifdef __FreeBSD__
uint32_t
jhash(void *key, uint32_t length, uint32_t initval)
{
  uint32_t ret = 0;
  int i;
  unsigned char *data = (unsigned char *)key;

  for (i = 0; i < length; i ++)
    ret += data[i];

  return ret;
}
#endif


bool
vr_valid_link_local_port(struct vrouter *router, int family,
                         int proto, int port)
{
    unsigned char data;
    unsigned int tmp;

    if (!router->vr_link_local_ports)
        return false;

    if ((family != AF_INET) ||
        ((proto != VR_IP_PROTO_TCP) && (proto != VR_IP_PROTO_UDP)))
        return false;

    if ((port < VR_DYNAMIC_PORT_START) || (port > VR_DYNAMIC_PORT_END))
        return false;

    tmp = port - VR_DYNAMIC_PORT_START;
    if (proto == VR_IP_PROTO_UDP)
        tmp += (router->vr_link_local_ports_size * 8 / 2);

    data = router->vr_link_local_ports[(tmp / 8)];
    if (data & (1 << (tmp % 8)))
        return true;

    return false;
}

static void
vr_clear_link_local_port(struct vrouter *router, int family,
                       int proto, int port)
{
    unsigned char *data;
    unsigned int tmp;

    if (!router->vr_link_local_ports)
        return;

    if ((family != AF_INET) ||
        ((proto != VR_IP_PROTO_TCP) && (proto != VR_IP_PROTO_UDP)))
        return;

    if ((port < VR_DYNAMIC_PORT_START) || (port > VR_DYNAMIC_PORT_END))
        return;

    tmp = port - VR_DYNAMIC_PORT_START;
    if (proto == VR_IP_PROTO_UDP)
        tmp += (router->vr_link_local_ports_size * 8 / 2);

    data = &router->vr_link_local_ports[(tmp / 8)];
    *data &= (~(1 << (tmp % 8)));

    return;
}

static void
vr_set_link_local_port(struct vrouter *router, int family,
                       int proto, int port)
{
    unsigned char *data;
    unsigned int tmp;

    if (!router->vr_link_local_ports)
        return;

    if ((family != AF_INET) ||
        ((proto != VR_IP_PROTO_TCP) && (proto != VR_IP_PROTO_UDP)))
        return;

    if ((port < VR_DYNAMIC_PORT_START) || (port > VR_DYNAMIC_PORT_END))
        return;

    tmp = port - VR_DYNAMIC_PORT_START;
    if (proto == VR_IP_PROTO_UDP)
        tmp += (router->vr_link_local_ports_size * 8 / 2);

    data = &router->vr_link_local_ports[tmp / 8];
    *data |= (1 << (tmp % 8));

    return;
}

static void
vr_flow_reset_mirror(struct vrouter *router, struct vr_flow_entry *fe,
                                                            unsigned int index)
{
    if (fe->fe_flags & VR_FLOW_FLAG_MIRROR) {
        vrouter_put_mirror(router, fe->fe_mirror_id);
        fe->fe_mirror_id = VR_MAX_MIRROR_INDICES;
        vrouter_put_mirror(router, fe->fe_sec_mirror_id);
        fe->fe_sec_mirror_id = VR_MAX_MIRROR_INDICES;
        vr_mirror_meta_entry_del(router, index);
    }
    fe->fe_flags &= ~VR_FLOW_FLAG_MIRROR;
    fe->fe_mirror_id = VR_MAX_MIRROR_INDICES;
    fe->fe_sec_mirror_id = VR_MAX_MIRROR_INDICES;

    return;
}

static void
vr_init_flow_entry(struct vr_flow_entry *fe)
{
    fe->fe_rflow = -1;
    fe->fe_mirror_id = VR_MAX_MIRROR_INDICES;
    fe->fe_sec_mirror_id = VR_MAX_MIRROR_INDICES;
    fe->fe_ecmp_nh_index = -1;

    return;
}


static void
__vr_flow_reset_entry(struct vrouter *router, struct vr_flow_entry *fe)
{
    if (fe->fe_hold_list) {
        vr_printf("vrouter: Potential memory leak @ %s:%d\n",
                __FILE__, __LINE__);
    }
    fe->fe_hold_list = NULL;

    fe->fe_key.key_len = 0;
    vr_flow_reset_mirror(router, fe, fe->fe_hentry.hentry_index);
    fe->fe_ecmp_nh_index = -1;
    fe->fe_src_nh_index = NH_DISCARD_ID;
    fe->fe_rflow = -1;
    fe->fe_action = VR_FLOW_ACTION_DROP;
    fe->fe_udp_src_port = 0;
    fe->fe_tcp_flags = 0;
    fe->fe_flags &=
        (VR_FLOW_FLAG_ACTIVE | VR_FLOW_FLAG_EVICTED |
         VR_FLOW_FLAG_NEW_FLOW);

    return;
}

static void
vr_flow_reset_entry(struct vrouter *router, struct vr_flow_entry *fe)
{
    __vr_flow_reset_entry(router, fe);
    fe->fe_type = VP_TYPE_NULL;
    memset(&fe->fe_stats, 0, sizeof(fe->fe_stats));
    fe->fe_flags = 0;

    vr_htable_release_hentry(router->vr_flow_table, &fe->fe_hentry);
    return;
}

static void
vr_flow_reset_active_entry(struct vrouter *router, struct vr_flow_entry *fe)
{
    __vr_flow_reset_entry(router, fe);

    vr_htable_release_hentry(router->vr_flow_table, &fe->fe_hentry);
    return;
}

static vr_hentry_key
vr_flow_get_key(vr_htable_t flow_table, vr_hentry_t *entry,
        unsigned int *key_len)
{
    struct vr_flow_entry *fe = CONTAINER_OF(fe_hentry,
                             struct vr_flow_entry, entry);

    if (!(fe->fe_flags & VR_FLOW_FLAG_ACTIVE))
        return NULL;

    if (key_len)
        *key_len = fe->fe_key.key_len;

    return &fe->fe_key;
}

static inline bool
vr_flow_set_active(struct vr_flow_entry *fe)
{
    return __sync_bool_compare_and_swap(&fe->fe_flags,
            fe->fe_flags & ~VR_FLOW_FLAG_ACTIVE,
            VR_FLOW_FLAG_ACTIVE | VR_FLOW_FLAG_NEW_FLOW);
}

unsigned int
vr_flow_table_size(struct vrouter *router)
{
    return vr_htable_size(router->vr_flow_table);
}

static unsigned int
vr_flow_table_oflow_entries(struct vrouter *router)
{
    return vr_htable_oflow_entries(router->vr_flow_table);
}
/*
 * this is used by the mmap code. mmap sees the whole flow table
 * (including the overflow table) as one large table. so, given
 * an offset into that large memory, we should return the correct
 * virtual address
 */
void *
vr_flow_get_va(struct vrouter *router, uint64_t offset)
{
    return vr_htable_get_address(router->vr_flow_table, offset);
}

struct vr_flow_entry *
vr_get_flow_entry(struct vrouter *router, int index)
{
    if (index < 0)
        return NULL;

    return (struct vr_flow_entry *)
            vr_htable_get_hentry_by_index(router->vr_flow_table, index);
}

static inline void
vr_flow_stop_modify(struct vrouter *router, struct vr_flow_entry *fe)
{
    if (!fe)
        return;

    (void)__sync_and_and_fetch(&fe->fe_flags, ~VR_FLOW_FLAG_MODIFIED);
    return;
}

static inline bool
vr_flow_start_modify(struct vrouter *router, struct vr_flow_entry *fe)
{
    unsigned short flags;

    flags = fe->fe_flags;
    if (!(flags & (VR_FLOW_FLAG_MODIFIED | VR_FLOW_FLAG_EVICTED))) {
        if (__sync_bool_compare_and_swap(&fe->fe_flags, flags,
                    flags | VR_FLOW_FLAG_MODIFIED)) {
            return true;
        }
    }

    return false;
}


/* Non-static due to RCU callback pointer comparison in vRouter/DPDK */
void
vr_flow_flush_hold_queue(struct vrouter *router, struct vr_flow_entry *fe,
        struct vr_flow_queue *vfq)
{
    struct vr_forwarding_md fmd;

    if (vfq) {
        vr_init_forwarding_md(&fmd);
        vr_flow_set_forwarding_md(router, fe, vfq->vfq_index, &fmd);
        __vr_flow_flush_hold_queue(router, fe, &fmd, vfq);
    }

    return;
}

static void
vr_flow_evict_flow(struct vrouter *router, struct vr_flow_entry *fe)
{
    unsigned short flags;

    if (!fe)
        return;

    if ((fe->fe_flags & VR_FLOW_FLAG_ACTIVE) &&
            (fe->fe_flags & VR_FLOW_FLAG_EVICT_CANDIDATE)) {
        flags = fe->fe_flags | VR_FLOW_FLAG_ACTIVE |
            VR_FLOW_FLAG_EVICT_CANDIDATE;
        (void)__sync_bool_compare_and_swap(&fe->fe_flags, flags,
                (flags ^ VR_FLOW_FLAG_EVICT_CANDIDATE) |
                VR_FLOW_FLAG_EVICTED);
        vr_flow_stop_modify(router, fe);
        vr_flow_reset_active_entry(router, fe);
    }

    return;
}

void
vr_flow_defer_cb(struct vrouter *router, void *arg)
{
    struct vr_defer_data *defer;
    struct vr_flow_entry *fe, *rfe;
    struct vr_flow_queue *vfq;
    struct vr_flow_defer_data *vfdd;

    defer = (struct vr_defer_data *)arg;
    if (!defer)
        return;

    vfdd = (struct vr_flow_defer_data *)defer->vdd_data;
    if (!vfdd)
        return;

    fe = vfdd->vfdd_fe;

    vfq = (struct vr_flow_queue *)vfdd->vfdd_flow_queue;
    if (vfq) {
        vr_flow_flush_hold_queue(router, fe, vfq);
        vr_free(vfq);
        vfdd->vfdd_flow_queue = NULL;
    }

    if (vfdd->vfdd_delete) {
        vr_flow_reset_entry(router, fe);
    } else {
        rfe = vr_get_flow_entry(router, fe->fe_rflow);
        vr_flow_evict_flow(router, fe);
        if (rfe)
            vr_flow_evict_flow(router, rfe);
    }

    return;
}

static void
vr_flow_reset_evict(struct vrouter *router, struct vr_flow_entry *fe)
{
    unsigned short flags;

    if (!fe)
        return;

    flags = fe->fe_flags;
    if (flags & VR_FLOW_FLAG_EVICT_CANDIDATE) {
        (void)__sync_bool_compare_and_swap(&fe->fe_flags, flags,
                (flags ^ VR_FLOW_FLAG_EVICT_CANDIDATE));
    }

    vr_flow_stop_modify(router, fe);

    return;
}

static void
vr_flow_defer(struct vr_flow_md *flmd, struct vr_flow_entry *fe)
{
    struct vr_flow_entry *rfe;
    struct vr_defer_data *vdd = flmd->flmd_defer_data;
    struct vr_flow_defer_data *vfdd;

    if (!vdd || !vdd->vdd_data) {
        if (fe->fe_rflow) {
            rfe = vr_get_flow_entry(flmd->flmd_router, fe->fe_rflow);
            vr_flow_reset_evict(flmd->flmd_router, rfe);
        }
        vr_flow_reset_evict(flmd->flmd_router, fe);

        if (!(flmd->flmd_flags & VR_FLOW_FLAG_ACTIVE)) {
            vr_flow_reset_entry(flmd->flmd_router, fe);
        }

        return;
    }

    vfdd = (struct vr_flow_defer_data *)vdd->vdd_data;
    vfdd->vfdd_fe = fe;

    vr_defer(flmd->flmd_router, vr_flow_defer_cb, (void *)vdd);
    flmd->flmd_defer_data = NULL;

    return;
}

static struct vr_flow_entry *
vr_flow_table_get_free_entry(struct vrouter *router, struct vr_flow *key,
        unsigned int *free_index)
{
    unsigned short flags;
    struct vr_flow_entry *fe;

    fe = (struct vr_flow_entry *)
         vr_htable_find_free_hentry(router->vr_flow_table, key, key->key_len);
    if (fe) {
        flags = fe->fe_flags;
        if (!(flags & VR_FLOW_FLAG_ACTIVE)) {
            if (vr_flow_set_active(fe)) {
                vr_init_flow_entry(fe);
            }
        } else if (flags & VR_FLOW_FLAG_EVICTED) {
            fe->fe_flags = ((flags & ~VR_FLOW_FLAG_EVICTED) |
                     VR_FLOW_FLAG_NEW_FLOW);
        }

        *free_index = fe->fe_hentry.hentry_index;
    }

    return fe;
}

static struct vr_flow_entry *
vr_flow_get_free_entry(struct vrouter *router, struct vr_flow *key, uint8_t type,
        bool need_hold, unsigned int *fe_index)
{
    struct vr_flow_entry *fe = NULL;

    fe = vr_flow_table_get_free_entry(router, key, fe_index);
    if (fe) {
        if (need_hold) {
            fe->fe_hold_list = vr_zalloc(sizeof(struct vr_flow_queue));
            if (!fe->fe_hold_list) {
                vr_flow_reset_entry(router, fe);
                fe = NULL;
            } else {
                fe->fe_hold_list->vfq_index = *fe_index;
            }
        }

        fe->fe_type = type;
        memcpy(&fe->fe_key, key, key->key_len);
        fe->fe_key.key_len = key->key_len;
    }

    return fe;
}

struct vr_flow_entry *
vr_find_flow(struct vrouter *router, struct vr_flow *key,
        uint8_t type, unsigned int *fe_index)
{
    struct vr_flow_entry *fe;

    fe = (struct vr_flow_entry *)vr_htable_find_hentry(router->vr_flow_table,
                                                    key, key->key_len);
    if (fe) {
        if (fe_index)
            *fe_index = fe->fe_hentry.hentry_index;
    }

    return fe;
}

void
vr_flow_fill_pnode(struct vr_packet_node *pnode, struct vr_packet *pkt,
        struct vr_forwarding_md *fmd)
{

    /*
     * we cannot cache nexthop here. to cache, we need to hold reference
     * to the nexthop. to hold a reference, we will have to hold a lock,
     * which we cannot. the only known case of misbehavior if we do not
     * cache is ECMP. when the packet comes from the fabric, the nexthop
     * actually points to a local composite, whereas a route lookup actually
     * returns a different nexthop, in which case the ecmp index will return
     * a bad nexthop. to avoid that, we will cache the label, and reuse it
     */
    pkt->vp_nh = NULL;

    pnode->pl_vif_idx = pkt->vp_if->vif_idx;
    if (fmd) {
        pnode->pl_outer_src_ip = fmd->fmd_outer_src_ip;
        pnode->pl_label = fmd->fmd_label;
        if (vr_forwarding_md_label_is_vxlan_id(fmd))
            pnode->pl_flags |= PN_FLAG_LABEL_IS_VXLAN_ID;
        if (fmd->fmd_to_me)
            pnode->pl_flags |= PN_FLAG_TO_ME;
    }
    pnode->pl_vrf = fmd->fmd_dvrf;
    pnode->pl_vlan = fmd->fmd_vlan;

    __sync_synchronize();
    pnode->pl_packet = pkt;

    return;
}

static int
vr_enqueue_flow(struct vrouter *router, struct vr_flow_entry *fe,
        struct vr_packet *pkt, unsigned int index,
        struct vr_flow_stats *stats, struct vr_forwarding_md *fmd)
{
    unsigned int i;
    unsigned short drop_reason = 0;
    struct vr_flow_queue *vfq = fe->fe_hold_list;
    struct vr_packet_node *pnode;

    if (!vfq) {
        drop_reason = VP_DROP_FLOW_UNUSABLE;
        goto drop;
    }

    i = __sync_fetch_and_add(&vfq->vfq_entries, 1);
    if (i >= VR_MAX_FLOW_QUEUE_ENTRIES) {
        drop_reason = VP_DROP_FLOW_QUEUE_LIMIT_EXCEEDED;
        goto drop;
    }

    pnode = &vfq->vfq_pnodes[i];
    vr_flow_fill_pnode(pnode, pkt, fmd);
    if (!i)
        vr_trap_flow(router, fe, pkt, index, stats);

    return 0;
drop:
    vr_pfree(pkt, drop_reason);
    return 0;
}

static flow_result_t
vr_flow_nat(struct vr_flow_entry *fe,
        struct vr_packet *pkt, struct vr_forwarding_md *fmd)
{
    if (pkt->vp_type == VP_TYPE_IP)
        return vr_inet_flow_nat(fe, pkt, fmd);

    vr_pfree(pkt, VP_DROP_FLOW_ACTION_INVALID);
    return FLOW_CONSUMED;
}

static void
vr_flow_set_forwarding_md(struct vrouter *router, struct vr_flow_entry *fe,
        unsigned int index, struct vr_forwarding_md *md)
{
    struct vr_flow_entry *rfe;

    md->fmd_flow_index = index;
    md->fmd_ecmp_nh_index = fe->fe_ecmp_nh_index;
    md->fmd_udp_src_port = fe->fe_udp_src_port;
    if (fe->fe_flags & VR_RFLOW_VALID) {
        rfe = vr_get_flow_entry(router, fe->fe_rflow);
        if (rfe)
            md->fmd_ecmp_src_nh_index = rfe->fe_ecmp_nh_index;
    }

    return;
}

static bool
__vr_flow_mark_evict(struct vrouter *router, struct vr_flow_entry *fe)
{
    unsigned short flags;

    flags = fe->fe_flags;
    if (flags & VR_FLOW_FLAG_ACTIVE) {
        if (vr_flow_start_modify(router, fe)) {
            flags = __sync_fetch_and_or(&fe->fe_flags,
                    VR_FLOW_FLAG_EVICT_CANDIDATE);
            if (flags & VR_FLOW_FLAG_EVICT_CANDIDATE) {
                goto unset_modified;
            } else {
                return true;
            }
        }
    }

    return false;

unset_modified:
    vr_flow_stop_modify(router, fe);
    return false;
}

static void
vr_flow_mark_evict(struct vrouter *router, struct vr_flow_entry *fe)
{
    unsigned int index = fe->fe_hentry.hentry_index;
    bool evict_forward_flow = true;

    struct vr_flow_entry *rfe = NULL;

    if (fe->fe_rflow >= 0) {
        rfe = vr_get_flow_entry(router, fe->fe_rflow);
        if (rfe && (rfe->fe_rflow == index)) {
            if (rfe->fe_tcp_flags & VR_FLOW_TCP_DEAD) {
                evict_forward_flow = __vr_flow_mark_evict(router, rfe);
            } else {
                evict_forward_flow = false;
            }
        } else {
            rfe = NULL;
        }
    }

    if (evict_forward_flow) {
        if (__vr_flow_mark_evict(router, fe)) {
            if (__vr_flow_schedule_transition(router, fe,
                        index, fe->fe_flags) < 0) {
                if (rfe)
                    vr_flow_reset_evict(router, rfe);
                vr_flow_reset_evict(router, fe);
            }
        } else {
            goto unset_modified;
        }
    }

    return;

unset_modified:
    if (rfe)
        vr_flow_stop_modify(router, rfe);

    return;
}

static flow_result_t
vr_flow_action(struct vrouter *router, struct vr_flow_entry *fe,
        unsigned int index, struct vr_packet *pkt,
        struct vr_forwarding_md *fmd)
{
    int valid_src;

    flow_result_t result;

    struct vr_forwarding_md mirror_fmd;
    struct vr_nexthop *src_nh;
    struct vr_packet *pkt_clone;

    fmd->fmd_dvrf = fe->fe_vrf;
    /*
     * for now, we will not use dvrf if VRFT is set, because the RPF
     * check needs to happen in the source vrf
     */
    src_nh = __vrouter_get_nexthop(router, fe->fe_src_nh_index);
    if (!src_nh) {
        vr_pfree(pkt, VP_DROP_INVALID_NH);
        return FLOW_CONSUMED;
    }

    if (src_nh->nh_validate_src) {
        valid_src = src_nh->nh_validate_src(pkt, src_nh, fmd, NULL);
        if (valid_src == NH_SOURCE_INVALID) {
            vr_pfree(pkt, VP_DROP_INVALID_SOURCE);
            return FLOW_CONSUMED;
        }

        if (valid_src == NH_SOURCE_MISMATCH) {
            pkt_clone = vr_pclone(pkt);
            if (pkt_clone) {
                vr_preset(pkt_clone);
                if (vr_pcow(pkt_clone, sizeof(struct vr_eth) +
                            sizeof(struct agent_hdr))) {
                    vr_pfree(pkt_clone, VP_DROP_PCOW_FAIL);
                } else {
                    vr_trap(pkt_clone, fmd->fmd_dvrf,
                            AGENT_TRAP_ECMP_RESOLVE, &fmd->fmd_flow_index);
                }
            }
        }
    }


    if (fe->fe_flags & VR_FLOW_FLAG_VRFT) {
        if (fmd->fmd_dvrf != fe->fe_dvrf) {
            fmd->fmd_dvrf = fe->fe_dvrf;
            fmd->fmd_to_me = 1;
        }
    }

    if (fe->fe_flags & VR_FLOW_FLAG_MIRROR) {
        if (fe->fe_mirror_id < VR_MAX_MIRROR_INDICES) {
            mirror_fmd = *fmd;
            mirror_fmd.fmd_ecmp_nh_index = -1;
            vr_mirror(router, fe->fe_mirror_id, pkt, &mirror_fmd);
        }
        if (fe->fe_sec_mirror_id < VR_MAX_MIRROR_INDICES) {
            mirror_fmd = *fmd;
            mirror_fmd.fmd_ecmp_nh_index = -1;
            vr_mirror(router, fe->fe_sec_mirror_id, pkt, &mirror_fmd);
        }
    }

    switch (fe->fe_action) {
    case VR_FLOW_ACTION_DROP:
        vr_pfree(pkt, VP_DROP_FLOW_ACTION_DROP);
        result = FLOW_CONSUMED;
        break;

    case VR_FLOW_ACTION_FORWARD:
        result = FLOW_FORWARD;
        break;

    case VR_FLOW_ACTION_NAT:
        result = vr_flow_nat(fe, pkt, fmd);
        break;

    default:
        vr_pfree(pkt, VP_DROP_FLOW_ACTION_INVALID);
        result = FLOW_CONSUMED;
        break;
    }

    if (fe->fe_tcp_flags & VR_FLOW_TCP_DEAD)
        vr_flow_mark_evict(router, fe);

    return result;
}


unsigned int
vr_trap_flow(struct vrouter *router, struct vr_flow_entry *fe,
        struct vr_packet *pkt, unsigned int index,
        struct vr_flow_stats *stats)
{
    unsigned int trap_reason;
    struct vr_packet *npkt;
    struct vr_flow_trap_arg ta;

    npkt = vr_pclone(pkt);
    if (!npkt)
        return -ENOMEM;

    vr_preset(npkt);

    switch (fe->fe_flags & VR_FLOW_FLAG_TRAP_MASK) {
    default:
        /*
         * agent needs a method to identify new flows from existing flows.
         * existing flows can be reused (evicted) or the action of such flows
         * can become hold. If existing flows are reused and packet is trapped,
         * agent will not re-evaluate the flow. Hence, agent has to be told
         * that this is a new flow, which we indicate by the trap reason.
         */
        if (fe->fe_flags & VR_FLOW_FLAG_NEW_FLOW) {
            trap_reason = AGENT_TRAP_FLOW_MISS;
            fe->fe_flags ^= VR_FLOW_FLAG_NEW_FLOW;
        } else {
            trap_reason = AGENT_TRAP_FLOW_ACTION_HOLD;
        }

        ta.vfta_index = index;
        if (fe->fe_type == VP_TYPE_IP)
            ta.vfta_nh_index = fe->fe_key.flow4_nh_id;
        if (stats) {
            ta.vfta_stats = *stats;
        } else {
            ta.vfta_stats = fe->fe_stats;
        }

        break;
    }

    return vr_trap(npkt, fe->fe_vrf, trap_reason, &ta);
}

static flow_result_t
vr_do_flow_action(struct vrouter *router, struct vr_flow_entry *fe,
        unsigned int index, struct vr_packet *pkt,
        struct vr_forwarding_md *fmd)
{
    uint32_t new_stats;
    struct vr_flow_stats stats, *stats_p = NULL;

    if (fe->fe_flags & VR_FLOW_FLAG_NEW_FLOW) {
        memcpy(&stats, &fe->fe_stats, sizeof(fe->fe_stats));
        memset(&fe->fe_stats, 0, sizeof(fe->fe_stats));
        stats_p = &stats;
    }

    new_stats = __sync_add_and_fetch(&fe->fe_stats.flow_bytes, pkt_len(pkt));
    if (new_stats < pkt_len(pkt))
        fe->fe_stats.flow_bytes_oflow++;

    new_stats = __sync_add_and_fetch(&fe->fe_stats.flow_packets, 1);
    if (!new_stats)
        fe->fe_stats.flow_packets_oflow++;

    if (fe->fe_action == VR_FLOW_ACTION_HOLD) {
        vr_enqueue_flow(router, fe, pkt, index, stats_p, fmd);
        return FLOW_HELD;
    }

    return vr_flow_action(router, fe, index, pkt, fmd);
}

static unsigned int
vr_flow_table_hold_count(struct vrouter *router)
{
    unsigned int i, num_cpus;
    uint64_t hcount = 0, act_count;
    struct vr_flow_table_info *infop = router->vr_flow_table_info;

    num_cpus = vr_num_cpus;
    for (i = 0; i < num_cpus; i++)
        hcount += infop->vfti_hold_count[i];

    act_count = infop->vfti_action_count;
    if (hcount >= act_count)
        return hcount - act_count;

    return 0;
}

static void
vr_flow_entry_set_hold(struct vrouter *router, struct vr_flow_entry *flow_e)
{
    unsigned int cpu;
    uint64_t act_count;
    struct vr_flow_table_info *infop = router->vr_flow_table_info;

    cpu = vr_get_cpu();
    if (cpu >= vr_num_cpus) {
        vr_printf("vrouter: Set HOLD failed (cpu %u num_cpus %u)\n",
                cpu, vr_num_cpus);
        return;
    }

    flow_e->fe_action = VR_FLOW_ACTION_HOLD;

    if (infop->vfti_hold_count[cpu] + 1 < infop->vfti_hold_count[cpu]) {
        (void)__sync_add_and_fetch(&infop->vfti_oflows, 1);
        act_count = infop->vfti_action_count;
        if (act_count > infop->vfti_hold_count[cpu]) {
           (void)__sync_sub_and_fetch(&infop->vfti_action_count,
                    infop->vfti_hold_count[cpu]);
            infop->vfti_hold_count[cpu] = 0;
        } else {
            infop->vfti_hold_count[cpu] -= act_count;
            (void)__sync_sub_and_fetch(&infop->vfti_action_count,
                    act_count);
        }
    }

    infop->vfti_hold_count[cpu]++;

    return;
}

static void
vr_flow_init_close(struct vrouter *router, struct vr_flow_entry *flow_e,
        struct vr_packet *pkt, struct vr_forwarding_md *fmd)
{
    struct vr_flow_entry *rfe;

    (void)__sync_fetch_and_or(&flow_e->fe_tcp_flags, VR_FLOW_TCP_DEAD);
    rfe = vr_get_flow_entry(router, flow_e->fe_rflow);
    if (rfe) {
        (void)__sync_fetch_and_or(&rfe->fe_tcp_flags, VR_FLOW_TCP_DEAD);
    }

    return;
}

static void
vr_flow_tcp_digest(struct vrouter *router, struct vr_flow_entry *flow_e,
        struct vr_packet *pkt, struct vr_forwarding_md *fmd)
{
    uint16_t tcp_offset_flags;
    unsigned int length;

    struct vr_ip *iph;
    struct vr_ip6 *ip6h;
    struct vr_tcp *tcph = NULL;
    struct vr_flow_entry *rflow_e = NULL;

    iph = (struct vr_ip *)pkt_network_header(pkt);
    if (!vr_ip_transport_header_valid(iph))
        return;

    if (pkt->vp_type == VP_TYPE_IP) {
        if (iph->ip_proto != VR_IP_PROTO_TCP)
            return;

        length = ntohs(iph->ip_len) - (iph->ip_hl * 4);
        tcph = (struct vr_tcp *)((unsigned char *)iph + (iph->ip_hl * 4));
    } else if (pkt->vp_type == VP_TYPE_IP6) {
        ip6h = (struct vr_ip6 *)iph;
        if (ip6h->ip6_nxt != VR_IP_PROTO_TCP)
            return;

        length = ntohs(ip6h->ip6_plen);
        tcph = (struct vr_tcp *)((unsigned char *)iph + sizeof(struct vr_ip6));
    }

    if (tcph) {
        /*
         * there are some optimizations here that makes the code slightly
         * not so frugal. For e.g.: the *_R flags are used to make sure that
         * for a packet that contains ACK, we will not need to fetch the
         * reverse flow if we are not interested, thus saving some execution
         * time.
         */
        tcp_offset_flags = ntohs(tcph->tcp_offset_r_flags);
        /* if we get a reset, session has to be closed */
        if (tcp_offset_flags & VR_TCP_FLAG_RST) {
            (void)__sync_fetch_and_or(&flow_e->fe_tcp_flags,
                    VR_FLOW_TCP_RST);
            if (flow_e->fe_flags & VR_RFLOW_VALID) {
                rflow_e = vr_get_flow_entry(router, flow_e->fe_rflow);
                if (rflow_e) {
                    (void)__sync_fetch_and_or(&rflow_e->fe_tcp_flags,
                            VR_FLOW_TCP_RST);
                }
            }
            vr_flow_init_close(router, flow_e, pkt, fmd);
            return;
        } else if (tcp_offset_flags & VR_TCP_FLAG_SYN) {
            /* if only a SYN... */
            flow_e->fe_tcp_seq = ntohl(tcph->tcp_seq);
            (void)__sync_fetch_and_or(&flow_e->fe_tcp_flags, VR_FLOW_TCP_SYN);
            if (flow_e->fe_flags & VR_RFLOW_VALID) {
                rflow_e = vr_get_flow_entry(router, flow_e->fe_rflow);
                if (rflow_e) {
                    (void)__sync_fetch_and_or(&rflow_e->fe_tcp_flags,
                            VR_FLOW_TCP_SYN_R);
                    if ((flow_e->fe_tcp_flags & VR_FLOW_TCP_SYN_R) &&
                            (tcp_offset_flags & VR_TCP_FLAG_ACK)) {
                        if (ntohl(tcph->tcp_ack) == (rflow_e->fe_tcp_seq + 1)) {
                            (void)__sync_fetch_and_or(&rflow_e->fe_tcp_flags,
                                    VR_FLOW_TCP_ESTABLISHED);
                            (void)__sync_fetch_and_or(&flow_e->fe_tcp_flags,
                                     VR_FLOW_TCP_ESTABLISHED_R);
                        }
                    }
                }
            }
        } else if (tcp_offset_flags & VR_TCP_FLAG_FIN) {
            /*
             * when a FIN is received, update the sequence of the FIN and set
             * the flow FIN flag. It is possible that the FIN packet came with
             * some data, in which case the sequence number of the FIN is one
             * more than the last data byte in the sequence
             */
            length -= (((tcp_offset_flags) >> 12) * 4);
            flow_e->fe_tcp_seq = ntohl(tcph->tcp_seq) + length;
            (void)__sync_fetch_and_or(&flow_e->fe_tcp_flags, VR_FLOW_TCP_FIN);
            /*
             * when an ack for a FIN is sent, we need to take some actions
             * on the reverse flow (since FIN came in the reverse flow). to
             * avoid looking up the reverse flow for all acks, we mark the
             * reverse flow's reverse flow with a flag (FIN_R). we will
             * lookup the reverse flow only if this flag is set and the
             * tcp header has an ack bit set
             */
            if (flow_e->fe_flags & VR_RFLOW_VALID) {
                rflow_e = vr_get_flow_entry(router, flow_e->fe_rflow);
                if (rflow_e) {
                    (void)__sync_fetch_and_or(&rflow_e->fe_tcp_flags,
                            VR_FLOW_TCP_FIN_R);
                }
            }
        }

        /*
         * if FIN_R is set in the flow and if the ACK bit is set in the
         * tcp header, then we need to mark the reverse flow as dead.
         *
         * OR
         *
         * if the SYN_R is set and ESTABLISHED_R is not set and if this
         * is an ack packet, if this ack completes the connection, we
         * need to set ESTABLISHED
         */
        if (((flow_e->fe_tcp_flags & VR_FLOW_TCP_FIN_R) ||
                (!(flow_e->fe_tcp_flags & VR_FLOW_TCP_ESTABLISHED_R) &&
                 (flow_e->fe_tcp_flags & VR_FLOW_TCP_SYN_R))) &&
                (tcp_offset_flags & VR_TCP_FLAG_ACK)) {
            if (flow_e->fe_flags & VR_RFLOW_VALID) {
                if (!rflow_e) {
                    rflow_e = vr_get_flow_entry(router, flow_e->fe_rflow);
                }

                if (rflow_e) {
                    if ((ntohl(tcph->tcp_ack) == (rflow_e->fe_tcp_seq + 1)) &&
                            (flow_e->fe_tcp_flags & VR_FLOW_TCP_FIN_R)) {
                        (void)__sync_fetch_and_or(&rflow_e->fe_tcp_flags,
                                VR_FLOW_TCP_HALF_CLOSE);
                        /*
                         * both the forward and the reverse flows are
                         * now dead
                         */
                        if (flow_e->fe_tcp_flags & VR_FLOW_TCP_HALF_CLOSE) {
                            vr_flow_init_close(router, flow_e, pkt, fmd);
                        }
                    } else if (ntohl(tcph->tcp_ack) != rflow_e->fe_tcp_seq) {
                        if (!(flow_e->fe_tcp_flags &
                                    VR_FLOW_TCP_ESTABLISHED_R)) {
                            (void)__sync_fetch_and_or(&rflow_e->fe_tcp_flags,
                                    VR_FLOW_TCP_ESTABLISHED);
                            (void)__sync_fetch_and_or(&flow_e->fe_tcp_flags,
                                     VR_FLOW_TCP_ESTABLISHED_R);
                        }
                    }
                }
            }
        }
    }

    return;
}

flow_result_t
vr_flow_lookup(struct vrouter *router, struct vr_flow *key,
               struct vr_packet *pkt, struct vr_forwarding_md *fmd)
{
    unsigned int fe_index;
    struct vr_flow_entry *flow_e;

    pkt->vp_flags |= VP_FLAG_FLOW_SET;


    flow_e = vr_find_flow(router, key, pkt->vp_type,  &fe_index);
    if (!flow_e) {
        if (pkt->vp_nh &&
            (pkt->vp_nh->nh_flags & NH_FLAG_RELAXED_POLICY))
            return FLOW_FORWARD;

        if ((vr_flow_hold_limit) &&
                (vr_flow_table_hold_count(router) >
                 VR_MAX_FLOW_TABLE_HOLD_COUNT)) {
            vr_pfree(pkt, VP_DROP_FLOW_UNUSABLE);
            return FLOW_CONSUMED;
        }

        flow_e = vr_flow_get_free_entry(router, key, pkt->vp_type,
                true, &fe_index);
        if (!flow_e) {
            vr_pfree(pkt, VP_DROP_FLOW_TABLE_FULL);
            return FLOW_CONSUMED;
        }

        flow_e->fe_vrf = fmd->fmd_dvrf;
        /* mark as hold */
        vr_flow_entry_set_hold(router, flow_e);
    } else if (flow_e->fe_flags & VR_FLOW_FLAG_NEW_FLOW) {
        /* agent allocated flow. In the process of setting up */
        return FLOW_DROP;
    }

    if (flow_e->fe_flags &
            (VR_FLOW_FLAG_EVICT_CANDIDATE | VR_FLOW_FLAG_EVICTED)) {
        return FLOW_DROP;
    }

    vr_flow_set_forwarding_md(router, flow_e, fe_index, fmd);
    vr_flow_tcp_digest(router, flow_e, pkt, fmd);

    return vr_do_flow_action(router, flow_e, fe_index, pkt, fmd);
}

static bool
__vr_flow_forward(flow_result_t result, struct vr_packet *pkt,
        struct vr_forwarding_md *fmd)
{
    bool forward = false;

    switch (result) {
    case FLOW_FORWARD:
        forward = true;
        break;

    case FLOW_TRAP:
        vr_trap(pkt, fmd->fmd_dvrf, AGENT_TRAP_L3_PROTOCOLS, NULL);
        break;

    case FLOW_HELD:
    case FLOW_CONSUMED:
        break;

    case FLOW_DROP:
    default:
        vr_pfree(pkt, VP_DROP_FLOW_UNUSABLE);
        break;
    }

    return forward;
}

fat_flow_port_mask_t
vr_flow_fat_flow_lookup(struct vrouter *router, struct vr_packet *pkt,
        uint16_t l4_proto, uint16_t sport, uint16_t dport)
{
    struct vr_nexthop *nh;
    struct vr_interface *vif_l = NULL;

    if (vif_is_virtual(pkt->vp_if)) {
        vif_l = pkt->vp_if;
    } else if (vif_is_fabric(pkt->vp_if)) {
        if ((nh = pkt->vp_nh) && (nh->nh_flags & NH_FLAG_VALID)) {
            vif_l = nh->nh_dev;
        }
    }

    if (!vif_l)
        return NO_PORT_MASK;

    return vif_fat_flow_lookup(vif_l, l4_proto, sport, dport);
}

bool
vr_flow_forward(struct vrouter *router, struct vr_packet *pkt,
                struct vr_forwarding_md *fmd)
{
    flow_result_t result;

    /* Flow processig is only for untagged unicast IP packets */
    if ((pkt->vp_type == VP_TYPE_IP) && (!(pkt->vp_flags & VP_FLAG_MULTICAST))
        && ((fmd->fmd_vlan == VLAN_ID_INVALID) || vif_is_service(pkt->vp_if)))
        result = vr_inet_flow_lookup(router, pkt, fmd);
    else
        result = FLOW_FORWARD;

    return __vr_flow_forward(result, pkt, fmd);
}

int
vr_flow_flush_pnode(struct vrouter *router, struct vr_packet_node *pnode,
        struct vr_flow_entry *fe, struct vr_forwarding_md *fmd)
{
    bool forward;

    struct vr_interface *vif;
    struct vr_packet *pkt;
    flow_result_t result;

    fmd->fmd_outer_src_ip = pnode->pl_outer_src_ip;
    if (pnode->pl_flags & PN_FLAG_LABEL_IS_VXLAN_ID) {
        vr_forwarding_md_set_label(fmd, pnode->pl_label,
                VR_LABEL_TYPE_VXLAN_ID);
    } else {
        vr_forwarding_md_set_label(fmd, pnode->pl_label,
                VR_LABEL_TYPE_MPLS);
    }

    if (pnode->pl_flags & PN_FLAG_TO_ME)
        fmd->fmd_to_me = 1;

    pkt = pnode->pl_packet;
    if (!pkt)
        return -EINVAL;

    pnode->pl_packet = NULL;
    /*
     * this is only a security check and not a catch all check. one note
     * of caution. please do not access pkt->vp_if till the if block is
     * succesfully bypassed
     */
    vif = __vrouter_get_interface(router, pnode->pl_vif_idx);
    if (!vif || (pkt->vp_if != vif)) {
        vr_pfree(pkt, VP_DROP_INVALID_IF);
        return -ENODEV;
    }

    if (!pkt->vp_nh) {
        if (vif_is_fabric(pkt->vp_if) && fmd &&
                (fmd->fmd_label >= 0)) {
            if (!vr_forwarding_md_label_is_vxlan_id(fmd))
                pkt->vp_nh = __vrouter_get_label(router, fmd->fmd_label);
        }
    }

    if (fe) {
        result = vr_flow_action(router, fe, fmd->fmd_flow_index, pkt, fmd);
        forward = __vr_flow_forward(result, pkt, fmd);
    } else {
        forward = vr_flow_forward(router, pkt, fmd);
    }

    if (forward)
        vr_reinject_packet(pkt, fmd);

    return 0;
}

static void
__vr_flow_flush_hold_queue(struct vrouter *router, struct vr_flow_entry *fe,
        struct vr_forwarding_md *fmd, struct vr_flow_queue *vfq)
{
    unsigned int i;
    struct vr_packet_node *pnode;

    for (i = 0; i < VR_MAX_FLOW_QUEUE_ENTRIES; i++) {
        pnode = &vfq->vfq_pnodes[i];
        vr_flow_flush_pnode(router, pnode, fe, fmd);
    }

    return;
}

static void
vr_flush_entry(struct vrouter *router, struct vr_flow_entry *fe,
        struct vr_flow_md *flmd, struct vr_forwarding_md *fmd)
{
    bool swapped;

    struct vr_flow_queue *vfq;
    struct vr_defer_data *vdd = flmd->flmd_defer_data;
    struct vr_flow_defer_data *vfdd;

    vfq = fe->fe_hold_list;
    if (vfq) {
        if (fe->fe_action == VR_FLOW_ACTION_HOLD)
            return;

        swapped = __sync_bool_compare_and_swap(&fe->fe_hold_list, vfq, NULL);
        if (swapped) {
            __vr_flow_flush_hold_queue(router, fe, fmd, vfq);
            if (!vdd || !vdd->vdd_data)
                goto free_flush_queue;

            vfdd = (struct vr_flow_defer_data *)vdd->vdd_data;
            vfdd->vfdd_flow_queue = vfq;
        }
    }

    return;

free_flush_queue:
    if (vfq)
        vr_free(vfq);
    return;
}

static void
__vr_flow_work(struct vrouter *router, struct vr_flow_entry *fe,
        struct vr_flow_md *flmd)
{
    struct vr_forwarding_md fmd;

    vr_init_forwarding_md(&fmd);
    vr_flow_set_forwarding_md(router, fe, flmd->flmd_index, &fmd);
    vr_flush_entry(router, fe, flmd, &fmd);

    vr_flow_defer(flmd, fe);
    return;
}


static void
vr_flow_work(void *arg)
{
    struct vrouter *router;
    struct vr_flow_entry *fe;
    struct vr_flow_md *flmd =
                (struct vr_flow_md *)arg;

    router = flmd->flmd_router;
    if (!router)
        goto exit_flush;

    fe = vr_get_flow_entry(router, flmd->flmd_index);
    if (!fe)
        goto exit_flush;

    __vr_flow_work(router, fe, flmd);

exit_flush:
    if (flmd->flmd_defer_data) {
        vr_put_defer_data(flmd->flmd_defer_data);
        flmd->flmd_defer_data = NULL;
    }

    vr_free(flmd);

    return;
}

static void
vr_flow_set_mirror(struct vrouter *router, vr_flow_req *req,
        struct vr_flow_entry *fe)
{
    struct vr_mirror_entry *mirror = NULL, *sec_mirror = NULL;

    if (!(req->fr_flags & VR_FLOW_FLAG_MIRROR) &&
            (fe->fe_flags & VR_FLOW_FLAG_MIRROR)) {
        vr_flow_reset_mirror(router, fe, req->fr_index);
        return;
    }

    if (!(req->fr_flags & VR_FLOW_FLAG_MIRROR))
        return;

    if (fe->fe_mirror_id != req->fr_mir_id) {
        if (fe->fe_mirror_id < router->vr_max_mirror_indices) {
            vrouter_put_mirror(router, fe->fe_mirror_id);
            fe->fe_mirror_id = router->vr_max_mirror_indices;
        }

        if ((unsigned int)req->fr_mir_id < router->vr_max_mirror_indices) {
            mirror = vrouter_get_mirror(req->fr_rid, req->fr_mir_id);
            if (mirror)
                fe->fe_mirror_id = req->fr_mir_id;

            /* when we reached this point, we had already done all the
             * sanity checks we could do. failing here will add only
             * complexity to code here. so !mirror case, we will not
             * handle
             */
        }
    }

    if (fe->fe_sec_mirror_id != req->fr_sec_mir_id) {
        if (fe->fe_sec_mirror_id < router->vr_max_mirror_indices) {
            vrouter_put_mirror(router, fe->fe_sec_mirror_id);
            fe->fe_sec_mirror_id = router->vr_max_mirror_indices;
        }

        if ((unsigned int)req->fr_sec_mir_id < router->vr_max_mirror_indices) {
            sec_mirror = vrouter_get_mirror(req->fr_rid, req->fr_sec_mir_id);
            if (sec_mirror)
                fe->fe_sec_mirror_id = req->fr_sec_mir_id;
        }
    }

    if (req->fr_pcap_meta_data_size && req->fr_pcap_meta_data)
        vr_mirror_meta_entry_set(router, req->fr_index,
                req->fr_mir_sip, req->fr_mir_sport,
                req->fr_pcap_meta_data, req->fr_pcap_meta_data_size,
                req->fr_mir_vrf);

    return;
}

static struct vr_flow_entry *
vr_add_flow(unsigned int rid, struct vr_flow *key, uint8_t type,
        bool need_hold_queue, unsigned int *fe_index)
{
    struct vr_flow_entry *flow_e;
    struct vrouter *router = vrouter_get(rid);

    flow_e = vr_find_flow(router, key, type, fe_index);
    if (flow_e) {
        /* a race between agent and dp. allow agent to handle this error */
        return NULL;
    } else {
        flow_e = vr_flow_get_free_entry(router, key, type,
                need_hold_queue, fe_index);
    }

    return flow_e;
}

static struct vr_flow_entry *
vr_add_flow_req(vr_flow_req *req, unsigned int *fe_index)
{
    uint8_t type;
    bool need_hold_queue = false;

    struct vr_flow key;
    struct vr_flow_entry *fe;

    vr_inet_fill_flow(&key, req->fr_flow_nh_id, req->fr_flow_sip,
            req->fr_flow_dip, req->fr_flow_proto,
            req->fr_flow_sport, req->fr_flow_dport);
    type = VP_TYPE_IP;

    if (req->fr_action == VR_FLOW_ACTION_HOLD)
        need_hold_queue = true;

    fe = vr_add_flow(req->fr_rid, &key, type, need_hold_queue, fe_index);
    if (fe)
        req->fr_index = *fe_index;

    return fe;
}

/*
 * can be called with 'fe' as null (specifically when flow is added from
 * agent), in which case we should be checking only the request
 */
static int
vr_flow_set_req_is_invalid(struct vrouter *router, vr_flow_req *req,
        struct vr_flow_entry *fe)
{
    int error = 0;
    struct vr_flow_entry *rfe;

    if (fe) {
        if (fe->fe_type == VP_TYPE_IP) {
            if ((unsigned int)req->fr_flow_sip != fe->fe_key.flow4_sip ||
                    (unsigned int)req->fr_flow_dip != fe->fe_key.flow4_dip ||
                    (unsigned short)req->fr_flow_sport != fe->fe_key.flow4_sport ||
                    (unsigned short)req->fr_flow_dport != fe->fe_key.flow4_dport||
                    (unsigned short)req->fr_flow_nh_id != fe->fe_key.flow4_nh_id ||
                    (unsigned char)req->fr_flow_proto != fe->fe_key.flow4_proto) {
                error = -EBADF;
                goto invalid_req;
            }
        }
    }

    if (req->fr_flags & VR_FLOW_FLAG_VRFT) {
        if ((unsigned short)req->fr_flow_dvrf >= router->vr_max_vrfs) {
            error = -EINVAL;
            goto invalid_req;
        }
    }

    if (req->fr_flags & VR_FLOW_FLAG_MIRROR) {
        if (((unsigned int)req->fr_mir_id >= router->vr_max_mirror_indices) &&
                (unsigned int)req->fr_sec_mir_id >= router->vr_max_mirror_indices) {
            error = -EINVAL;
            goto invalid_req;
        }
    }

    if (req->fr_flags & VR_RFLOW_VALID) {
        rfe = vr_get_flow_entry(router, req->fr_rindex);
        if (!rfe) {
            error = -EINVAL;
            goto invalid_req;
        }
    }

    return 0;

invalid_req:
    return error;
}

static int
__vr_flow_schedule_transition(struct vrouter *router, struct vr_flow_entry *fe,
        unsigned int index, unsigned short flags)
{
    struct vr_flow_md *flmd;
    struct vr_defer_data *defer = NULL;

    flmd = (struct vr_flow_md *)vr_malloc(sizeof(*flmd));
    if (!flmd)
        return -ENOMEM;

    flmd->flmd_router = router;
    flmd->flmd_index = index;
    flmd->flmd_flags = flags;
    if (fe->fe_hold_list || (flags & VR_FLOW_FLAG_EVICT_CANDIDATE)) {
        defer = vr_get_defer_data(sizeof(*defer) +
                sizeof(struct vr_flow_defer_data));
        if (defer) {
            defer->vdd_data = (void *)(defer + 1);
            defer->vdd_data = memset(defer->vdd_data, 0,
                    sizeof(struct vr_flow_defer_data));
            if (!(flmd->flmd_flags & VR_FLOW_FLAG_ACTIVE)) {
                ((struct vr_flow_defer_data *)defer->vdd_data)->vfdd_delete =
                    true;
            }
        }
    }
    flmd->flmd_defer_data = defer;

    vr_schedule_work(vr_get_cpu(), vr_flow_work, (void *)flmd);
    return 0;
}

static int
vr_flow_schedule_transition(struct vrouter *router, vr_flow_req *req,
        struct vr_flow_entry *fe)
{
    return __vr_flow_schedule_transition(router, fe, req->fr_index, req->fr_flags);
}

static int
vr_flow_delete(struct vrouter *router, vr_flow_req *req,
        struct vr_flow_entry *fe)
{
    if (fe->fe_flags & VR_FLOW_FLAG_LINK_LOCAL)
        vr_clear_link_local_port(router, AF_INET, fe->fe_key.flow4_proto,
                                   ntohs(fe->fe_key.flow4_dport));

    fe->fe_action = VR_FLOW_ACTION_DROP;
    vr_flow_reset_mirror(router, fe, req->fr_index);

    return vr_flow_schedule_transition(router, req, fe);
}

static void
vr_flow_udp_src_port (struct vrouter *router, struct vr_flow_entry *fe)
{
    uint32_t hash_key[5], hashval, port_range;
    uint16_t port;

    if (fe->fe_udp_src_port)
        return;

    if (hashrnd_inited == 0) {
        get_random_bytes(&vr_hashrnd, sizeof(vr_hashrnd));
        hashrnd_inited = 1;
    }

    hash_key[0] = fe->fe_key.flow4_sip;
    hash_key[1] = fe->fe_key.flow4_dip;
    hash_key[2] = fe->fe_vrf;
    hash_key[3] = fe->fe_key.flow4_sport;
    hash_key[4] = fe->fe_key.flow4_dport;

    hashval = jhash(hash_key, 20, vr_hashrnd);
    port_range = VR_MUDP_PORT_RANGE_END - VR_MUDP_PORT_RANGE_START;
    port = (uint16_t ) (((uint64_t ) hashval * port_range) >> 32);

    if (port > port_range) {
        /*
         * Shouldn't happen...
         */
        port = 0;
    }
    fe->fe_udp_src_port = port + VR_MUDP_PORT_RANGE_START;
}


/* command from agent */
static int
vr_flow_set(struct vrouter *router, vr_flow_req *req)
{
    int ret;
    unsigned int fe_index;
    bool new_flow = false, modified = false;

    struct vr_flow_entry *fe = NULL, *rfe = NULL;
    struct vr_flow_table_info *infop = router->vr_flow_table_info;

    router = vrouter_get(req->fr_rid);
    if (!router)
        return -EINVAL;

    fe = vr_get_flow_entry(router, req->fr_index);
    if (fe) {
        if (!(modified = vr_flow_start_modify(router, fe))) {
            return -EBUSY;
        }
    }

    if ((ret = vr_flow_set_req_is_invalid(router, req, fe)))
        goto exit_set;

    if (fe) {
        if ((fe->fe_action == VR_FLOW_ACTION_HOLD) &&
            ((req->fr_action != fe->fe_action) ||
             !(req->fr_flags & VR_FLOW_FLAG_ACTIVE))) {
            __sync_fetch_and_add(&infop->vfti_action_count, 1);
        }
    }
    /*
     * for delete, absence of the requested flow entry is caustic. so
     * handle that case first
     */
    if (!(req->fr_flags & VR_FLOW_FLAG_ACTIVE)) {
        if (!fe)
            return -EINVAL;
        return vr_flow_delete(router, req, fe);
    }


    /*
     * for non-delete cases, absence of flow entry means addition of a
     * new flow entry with the key specified in the request
     */
    if (!fe) {
        fe = vr_add_flow_req(req, &fe_index);
        if (!fe)
            return -ENOSPC;

        new_flow = true;
        infop->vfti_added++;
    } else {
        if ((req->fr_action == VR_FLOW_ACTION_HOLD) &&
                (fe->fe_action != req->fr_action)) {
            if (!fe->fe_hold_list) {
                fe->fe_hold_list = vr_zalloc(sizeof(struct vr_flow_queue));
                if (!fe->fe_hold_list) {
                    ret = -ENOMEM;
                    goto exit_set;
                }
            }
        }
    }

    vr_flow_set_mirror(router, req, fe);

    if (req->fr_flags & VR_RFLOW_VALID) {
        fe->fe_rflow = req->fr_rindex;
    } else {
        if (fe->fe_rflow >= 0)
            fe->fe_rflow = -1;
    }

    fe->fe_vrf = req->fr_flow_vrf;
    if (req->fr_flags & VR_FLOW_FLAG_VRFT)
        fe->fe_dvrf = req->fr_flow_dvrf;

    if (fe->fe_type == VP_TYPE_IP) {
        if (req->fr_flags & VR_FLOW_FLAG_LINK_LOCAL) {
            if (!(fe->fe_flags & VR_FLOW_FLAG_LINK_LOCAL))
                vr_set_link_local_port(router, AF_INET,
                        fe->fe_key.flow4_proto,
                        ntohs(fe->fe_key.flow4_dport));
        } else {
            if (fe->fe_flags & VR_FLOW_FLAG_LINK_LOCAL)
                vr_clear_link_local_port(router, AF_INET,
                        fe->fe_key.flow4_proto,
                        ntohs(fe->fe_key.flow4_dport));
        }
    }

    fe->fe_ecmp_nh_index = req->fr_ecmp_nh_index;
    fe->fe_src_nh_index = req->fr_src_nh_index;

    if ((req->fr_action == VR_FLOW_ACTION_HOLD) &&
            (fe->fe_action != VR_FLOW_ACTION_HOLD)) {
        vr_flow_entry_set_hold(router, fe);
    } else {
        fe->fe_action = req->fr_action;
    }

    if (fe->fe_action == VR_FLOW_ACTION_DROP)
        fe->fe_drop_reason = (uint8_t)req->fr_drop_reason;

    fe->fe_flags = VR_FLOW_FLAG_MASK(req->fr_flags);
    if (new_flow) {
        if (fe->fe_flags & VR_RFLOW_VALID) {
            rfe = vr_get_flow_entry(router, fe->fe_rflow);
            if (rfe) {
                if (rfe->fe_tcp_flags & VR_FLOW_TCP_SYN) {
                    (void)__sync_fetch_and_or(&fe->fe_tcp_flags,
                            VR_FLOW_TCP_SYN_R);
                }
            }
        }
    }

    vr_flow_udp_src_port(router, fe);

    if (fe->fe_flags & VR_FLOW_FLAG_NEW_FLOW)
        fe->fe_flags ^= VR_FLOW_FLAG_NEW_FLOW;

    ret = vr_flow_schedule_transition(router, req, fe);

exit_set:
    if (modified && fe) {
        vr_flow_stop_modify(router, fe);
    }

    return ret;
}

static void
vr_flow_req_destroy(vr_flow_req *req)
{
    if (!req)
        return;

    if (req->fr_file_path) {
        vr_free(req->fr_file_path);
        req->fr_file_path = NULL;
    }

    if (req->fr_hold_stat && req->fr_hold_stat_size) {
        vr_free(req->fr_hold_stat);
        req->fr_hold_stat = NULL;
        req->fr_hold_stat_size = 0;
    }

    vr_free(req);

    return;
}

vr_flow_req *
vr_flow_req_get(vr_flow_req *ref_req)
{
    unsigned int hold_stat_size;
    unsigned int num_cpus = vr_num_cpus;
    vr_flow_req *req = vr_zalloc(sizeof(*req));

    if (!req)
        return NULL;

    if (ref_req) {
        memcpy(req, ref_req, sizeof(*ref_req));
        /* not intended */
        req->fr_pcap_meta_data = NULL;
        req->fr_pcap_meta_data_size = 0;
    }

    if (vr_flow_path) {
        req->fr_file_path = vr_zalloc(VR_UNIX_PATH_MAX);
        if (!req->fr_file_path) {
            vr_free(req);
            return NULL;
        }
    }

    if (num_cpus > VR_FLOW_MAX_CPUS)
        num_cpus = VR_FLOW_MAX_CPUS;

    hold_stat_size = num_cpus * sizeof(uint32_t);
    req->fr_hold_stat = vr_zalloc(hold_stat_size);
    if (!req->fr_hold_stat) {
        if (vr_flow_path && req->fr_file_path) {
            vr_free(req->fr_file_path);
            req->fr_file_path = NULL;
        }

        vr_free(req);
        return NULL;
    }
    req->fr_hold_stat_size = num_cpus;

    return req;
}

/*
 * sandesh handler for vr_flow_req
 */
void
vr_flow_req_process(void *s_req)
{
    int ret = 0;
    unsigned int i, object = VR_FLOW_OBJECT_ID;
    bool need_destroy = false;
    uint64_t hold_count = 0;

    struct vrouter *router;
    vr_flow_req *req = (vr_flow_req *)s_req;
    vr_flow_req *resp = NULL;

    router = vrouter_get(req->fr_rid);
    switch (req->fr_op) {
    case FLOW_OP_FLOW_TABLE_GET:
        resp = vr_flow_req_get(req);
        if (!resp) {
            ret = -ENOMEM;
            goto send_response;
        }

        need_destroy = true;
        resp->fr_op = req->fr_op;
        resp->fr_ftable_size = vr_flow_table_size(router);
#if defined(__linux__) && defined(__KERNEL__)
        resp->fr_ftable_dev = vr_flow_major;
#endif
        if (vr_flow_path) {
            strncpy(resp->fr_file_path, vr_flow_path, VR_UNIX_PATH_MAX - 1);
        }

        resp->fr_processed = router->vr_flow_table_info->vfti_action_count;
        resp->fr_hold_oflows = router->vr_flow_table_info->vfti_oflows;
        resp->fr_added = router->vr_flow_table_info->vfti_added;
        resp->fr_cpus = vr_num_cpus;
        /* we only have space for 64 stats block max when encoding */
        for (i = 0; ((i < vr_num_cpus) && (i < VR_FLOW_MAX_CPUS)); i++) {
            resp->fr_hold_stat[i] =
                router->vr_flow_table_info->vfti_hold_count[i];
            hold_count += resp->fr_hold_stat[i];
        }

        resp->fr_created = hold_count;
        resp->fr_oflow_entries = vr_flow_table_oflow_entries(router);

        object = VR_FLOW_INFO_OBJECT_ID;
        break;

    case FLOW_OP_FLOW_SET:
        ret = vr_flow_set(router, req);
        resp = req;
        break;

    default:
        ret = -EINVAL;
    }

send_response:
    vr_message_response(object, resp, ret);
    if (need_destroy) {
        vr_flow_req_destroy(resp);
    }

    return;
}

static void
vr_flow_table_info_destroy(struct vrouter *router)
{
    if (!router->vr_flow_table_info)
        return;

    vr_free(router->vr_flow_table_info);
    router->vr_flow_table_info = NULL;
    router->vr_flow_table_info_size = 0;

    return;
}

static void
vr_flow_table_info_reset(struct vrouter *router)
{
    if (!router->vr_flow_table_info)
        return;

    memset(router->vr_flow_table_info, 0, router->vr_flow_table_info_size);
    return;
}

static int
vr_flow_table_info_init(struct vrouter *router)
{
    unsigned int size;
    struct vr_flow_table_info *infop;

    if (router->vr_flow_table_info)
        return 0;

    size = sizeof(struct vr_flow_table_info) + sizeof(uint32_t) * vr_num_cpus;
    infop = (struct vr_flow_table_info *)vr_zalloc(size);
    if (!infop)
        return vr_module_error(-ENOMEM, __FUNCTION__, __LINE__, size);

    router->vr_flow_table_info = infop;
    router->vr_flow_table_info_size = size;

    return 0;
}

static void
vr_flow_table_destroy(struct vrouter *router)
{
    if (router->vr_flow_table) {
        vr_htable_delete(router->vr_flow_table);
        router->vr_flow_table = NULL;
    }

    vr_flow_table_info_destroy(router);

    return;
}

static void
vr_flow_table_reset(struct vrouter *router)
{
    unsigned int start, end, i;
    struct vr_flow_entry *fe;
    struct vr_forwarding_md fmd;
    struct vr_flow_md flmd;

    start = end = 0;
    end = vr_flow_entries + vr_oflow_entries;

    if (end) {
        flmd.flmd_defer_data = NULL;
        vr_init_forwarding_md(&fmd);
        for (i = start; i < end; i++) {
            fe = vr_get_flow_entry(router, i);
            if (fe) {
                flmd.flmd_index = i;
                flmd.flmd_flags = fe->fe_flags;
                fe->fe_action = VR_FLOW_ACTION_DROP;
                vr_flush_entry(router, fe, &flmd, &fmd);
                vr_flow_reset_entry(router, fe);
            }
        }
    }

    vr_flow_table_info_reset(router);

    return;
}

static int
vr_flow_table_init(struct vrouter *router)
{
    if (!router->vr_flow_table) {

        router->vr_flow_table = vr_htable_attach(router, vr_flow_entries,
                vr_flow_table, vr_oflow_entries, vr_oflow_table,
                sizeof(struct vr_flow_entry), 0, 0, vr_flow_get_key);

        if (!router->vr_flow_table) {
            return vr_module_error(-ENOMEM, __FUNCTION__,
                    __LINE__, vr_flow_entries + vr_oflow_entries);
        }
    }

    return vr_flow_table_info_init(router);
}

static void
vr_link_local_ports_reset(struct vrouter *router)
{
    if (router->vr_link_local_ports) {
        memset(router->vr_link_local_ports,
               0, router->vr_link_local_ports_size);
    }

    return;
}

static void
vr_link_local_ports_exit(struct vrouter *router)
{
    if (router->vr_link_local_ports) {
        vr_free(router->vr_link_local_ports);
        router->vr_link_local_ports = NULL;
        router->vr_link_local_ports_size = 0;
    }

    return;
}

static int
vr_link_local_ports_init(struct vrouter *router)
{
    unsigned int port_range, bytes;

    if (router->vr_link_local_ports)
        return 0;

    /*  Udp and TCP inclusive of low and high limits*/
    port_range = 2 * ((VR_DYNAMIC_PORT_END - VR_DYNAMIC_PORT_START) + 1);
    /* Make it 16 bit boundary */
    bytes = (port_range + 15) & ~15;
    /* Bits to Bytes */
    bytes /= 8;

    router->vr_link_local_ports = vr_zalloc(bytes);
    if (!router->vr_link_local_ports)
        return -1;
    router->vr_link_local_ports_size = bytes;

    return 0;
}

/* flow module exit and init */
void
vr_flow_exit(struct vrouter *router, bool soft_reset)
{
    vr_flow_table_reset(router);
    vr_link_local_ports_reset(router);
    if (!soft_reset) {
        vr_flow_table_destroy(router);
        vr_fragment_table_exit(router);
        vr_link_local_ports_exit(router);
    }

    return;
}

int
vr_flow_init(struct vrouter *router)
{
    int ret;

    if ((ret = vr_fragment_table_init(router)) < 0)
        return ret;

    if ((ret = vr_flow_table_init(router)))
        return ret;

    if ((ret = vr_link_local_ports_init(router)))
        return ret;

    return 0;
}

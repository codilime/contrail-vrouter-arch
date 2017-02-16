#include "vr_os.h"
#include "vrouter.h"
#include "vr_packet.h"

struct host_os *vrouter_host;

unsigned int vr_bridge_entries = 0;
unsigned int vr_bridge_oentries = 0;

const char *ContrailBuildInfo = "";

unsigned int vr_num_cpus = 0;


struct vr_vrf_stats *(*vr_inet_vrf_stats)(int, unsigned int);


vr_itable_t
vr_itable_create(unsigned int index_len, unsigned int stride_cnt, ...)
{
    return 0;
}

struct vr_nexthop *(*vr_bridge_lookup)(unsigned int, struct vr_route_req *);

int
vr_itable_trav(vr_itable_t t, vr_itable_trav_cb_t func,
    unsigned int marker, void *udata)
{
    return 0;
}

void *
vr_itable_set(vr_itable_t t, unsigned int index, void *data)
{
    return 0;
}

void *
vr_itable_get(vr_itable_t t, unsigned int index)
{
    return 0;
}

void
vr_itable_delete(vr_itable_t t, vr_itable_del_cb_t func)
{
}

void *
vr_itable_del(vr_itable_t t, unsigned int index)
{
    return 0;
}

unsigned int vr_bridge_table_used_oflow_entries(struct vrouter *router) {
    return 0;
}

unsigned int vr_bridge_table_used_total_entries(struct vrouter *router) {
    return 0;
}

unsigned int vr_virtual_input(unsigned short vrf, struct vr_interface *vif, struct vr_packet *pkt, unsigned short vlan_id) {
    return 0;
}

unsigned int vr_fabric_input(struct vr_interface *vif, struct vr_packet *pkt, unsigned short vlan_id) {
    return 0;
}

int vr_l3_input(struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return 0;
}

int vr_untag_pkt(struct vr_packet *pkt) {
    return 0;
}

int vr_tag_pkt(struct vr_packet *pkt, unsigned short vlan_id) {
    return 0;
}

void vr_vlan_set_priority(struct vr_packet *pkt) {
}

int vr_pkt_type(struct vr_packet *pkt, unsigned short offset, struct vr_forwarding_md *fmd) {
    return 0;
}

mac_response_t vm_arp_request(struct vr_interface *vif, struct vr_packet *pkt, struct vr_forwarding_md *fmd, unsigned char *dmac) {
    return 0;
}

mac_response_t vm_neighbor_request(struct vr_interface *vif, struct vr_packet *pkt, struct vr_forwarding_md *fmd, unsigned char *dmac) {
    return 0;
}

int vif_plug_mac_request(struct vr_interface *vif, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return 0;
}

struct vr_interface *vif_bridge_get_sub_interface(vr_htable_t htable, unsigned short vlan, unsigned char *mac) {
    return NULL;
}

int vif_bridge_get_index(struct vr_interface *pvif, struct vr_interface *vif) {
    return 0;
}

int vif_bridge_init(struct vr_interface *vif) {
    return 0;
}

void vif_bridge_deinit(struct vr_interface *vif) {

}

int vif_bridge_delete(struct vr_interface *pvif, struct vr_interface *vif) {
    return 0;
}

int vif_bridge_add(struct vr_interface *pvif, struct vr_interface *vif) {
    return 0;
}

void vhost_remove_xconnect(void) {
    return;
}

int vr_inet6_form_flow(struct vrouter *router, unsigned short vrf, struct vr_packet *pkt, uint16_t vlan, struct vr_ip6 *ip6, struct vr_flow *flow_p) {
    return 0;
}

int vr_inet_get_flow_key(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd, struct vr_flow *flow) {
    return 0;
}

int vr_ip_rcv(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return 0;
}

unsigned short vr_ip_csum(struct vr_ip *ip) {
    return 0;
}

unsigned short vr_generate_unique_ip_id() {
    return 0;
}

unsigned short vr_ip_partial_csum(struct vr_ip *ip) {
    return 0;
}

unsigned short vr_ip6_partial_csum(struct vr_ip6 *ip6) {
    return 0;
}

void vr_btable_free(struct vr_btable *table) {
}

struct vr_btable *vr_btable_alloc(unsigned int num_entries, unsigned int entry_size) {
    return NULL;
}

int vr_arp_input(struct vr_packet *pkt, struct vr_forwarding_md *fmd, unsigned char *eth_dmac) {
    return 0;
}

int vr_neighbor_input(struct vr_packet *pkt, struct vr_forwarding_md *fmd, unsigned char *eth_dmac) {
    return 0;
}

int vr_trap(struct vr_packet *pkt, unsigned short trap_vrf, unsigned short trap_reason, void *trap_param) {
    return 0;
}

int vr_forward(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return 0;
}

unsigned int vr_bridge_input(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return 0;
}

unsigned short vr_bridge_route_flags(unsigned int vrf_id, unsigned char *mac) {
    return 0;
}

int vr_gro_input(struct vr_packet *pkt, struct vr_nexthop *nh) {
    return 0;
}

bool vr_has_to_fragment(struct vr_interface *vif, struct vr_packet *pkt, unsigned int tun_len) {
    return false;
}

struct vr_nexthop *vr_inet6_ip_lookup(unsigned short vrf, uint8_t *ip6) {
    return NULL;
}

struct vr_nexthop *vr_inet_ip_lookup(unsigned short vrf, uint32_t ip) {
    return NULL;
}

l4_pkt_type_t vr_ip6_well_known_packet(struct vr_packet *pkt) {
    return 0;
}

l4_pkt_type_t vr_ip_well_known_packet(struct vr_packet *pkt) {
    return 0;
}


vr_htable_t vr_htable_attach(struct vrouter *router, unsigned int entries, void *htable, unsigned int oentries, void *otable, unsigned int entry_size, unsigned int key_size, unsigned int bucket_size, get_hentry_key get_entry_key) {
    return (vr_htable_t)0;
}

unsigned int vr_htable_used_oflow_entries(vr_htable_t htable) {
    return 0;
}

unsigned int vr_htable_used_total_entries(vr_htable_t htable) {
    return 0;
}

void vr_htable_delete(vr_htable_t htable) {
}

vr_hentry_t *vr_htable_find_hentry(vr_htable_t htable, void *key, unsigned int key_len) {
    return NULL;
}

vr_hentry_t *vr_htable_get_hentry_by_index(vr_htable_t htable, unsigned int index) {
    return NULL;
}

vr_hentry_t *vr_htable_find_free_hentry(vr_htable_t htable, void *key, unsigned int key_size) {
    return NULL;
}

void vr_htable_reset(vr_htable_t htable, htable_trav_cb cb, void *data) {
}

void vr_htable_release_hentry(vr_htable_t htable, vr_hentry_t *ent) {
}

unsigned int vr_htable_size(vr_htable_t htable) {
    return 0;
}

void *vr_htable_get_address(vr_htable_t htable, uint64_t offset) {
    return NULL;
}

flow_result_t vr_inet_flow_lookup(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return (flow_result_t)0;
}

flow_result_t vr_inet6_flow_lookup(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return (flow_result_t)0;
}

flow_result_t vr_inet_flow_nat(struct vr_flow_entry *fe, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return (flow_result_t)0;
}

void vr_inet_fill_flow(struct vr_flow *flow_p, unsigned short nh_id, unsigned char *ip, uint8_t proto, uint16_t sport, uint16_t dport) {
}

void vr_inet6_fill_flow(struct vr_flow *flow_p, unsigned short nh_id, unsigned char *ip, uint8_t proto, uint16_t sport, uint16_t dport) {
}

bool vr_inet_flow_is_fat_flow(struct vrouter *router, struct vr_packet *pkt, struct vr_flow_entry *fe) {
    return false;
}

bool vr_inet6_flow_is_fat_flow(struct vrouter *router, struct vr_packet *pkt, struct vr_flow_entry *fe) {
    return false;
}

bool vr_inet_flow_allow_new_flow(struct vrouter *router, struct vr_packet *pkt) {
    return false;
}

unsigned int vr_reinject_packet(struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return 0;
}

int vr_fragment_table_init(struct vrouter *router) {
    return 0;
}

void vr_fragment_table_exit(struct vrouter *router) {
}

int bridge_table_init(struct vr_rtable *a, struct rtable_fspec *b) {
    return 0;
}
void bridge_table_deinit(struct vr_rtable *x, struct rtable_fspec *y, bool z) {
}

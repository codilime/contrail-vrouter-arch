#include "vr_os.h"
#include "vrouter.h"
#include "vr_packet.h"

struct host_os *vrouter_host;
const char *ContrailBuildInfo = "";
unsigned int vr_num_cpus = 0;


vr_itable_t
vr_itable_create(unsigned int index_len, unsigned int stride_cnt, ...)
{
    return 0;
}

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

mac_response_t vm_arp_request(struct vr_interface *vif, struct vr_packet *pkt, struct vr_forwarding_md *fmd, unsigned char *dmac) {
    return 0;
}

mac_response_t vm_neighbor_request(struct vr_interface *vif, struct vr_packet *pkt, struct vr_forwarding_md *fmd, unsigned char *dmac) {
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

int vr_neighbor_input(struct vr_packet *pkt, struct vr_forwarding_md *fmd, unsigned char *eth_dmac) {
    return 0;
}

int vr_forward(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
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

int vr_fragment_table_init(struct vrouter *router) {
    return 0;
}

void vr_fragment_table_exit(struct vrouter *router) {
}

int vr_ip_input(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return 0;
}

int vr_ip6_input(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return 0;
}

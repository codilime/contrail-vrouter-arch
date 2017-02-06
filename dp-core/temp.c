#include "vr_os.h"
#include "vrouter.h"
#include "vr_packet.h"

#pragma warning(disable : 4100)

struct host_os *vrouter_host;

unsigned int vr_flow_entries = 0;
unsigned int vr_oflow_entries = 0;
unsigned int vr_flow_hold_limit = 0;
unsigned int vr_bridge_entries = 0;
unsigned int vr_bridge_oentries = 0;

const char *ContrailBuildInfo = "";

unsigned int vr_num_cpus = 0;

int hashrnd_inited = 0;
uint32_t vr_hashrnd = 0;


struct vr_vrf_stats *(*vr_inet_vrf_stats)(int, unsigned int);


int vr_flow_init(struct vrouter *router) {
    return 0;
}

void vr_flow_exit(struct vrouter *router, bool soft_reset) {
}

int vr_fib_init(struct vrouter *router) {
    return 0;
}

void vr_fib_exit(struct vrouter *router, bool soft_reset) {
}

int vr_vxlan_init(struct vrouter *router) {
    return 0;
}

void vr_vxlan_exit(struct vrouter *router, bool soft_reset) {
}

int vr_stats_init(struct vrouter *router) {
    return 0;
}

void vr_stats_exit(struct vrouter *router, bool soft_reset) {
}

int vr_mpls_init(struct vrouter *router) {
    return 0;
}

void vr_mpls_exit(struct vrouter *router, bool soft_reset) {
}

int vr_qos_init(struct vrouter *router) {
    return 0;
}

void vr_qos_exit(struct vrouter *router, bool soft_reset) {
}

unsigned int vr_flow_table_used_oflow_entries(struct vrouter *router) {
    return 0;
}

unsigned int vr_flow_table_used_total_entries(struct vrouter *router) {
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

struct vr_host_interface_ops *vr_host_interface_init(void) {
    return NULL;
}

void vr_host_interface_exit(void) {
}

void vr_host_vif_init(struct vrouter *router) {
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

struct vr_flow_entry *vr_flow_get_entry(struct vrouter *router, int index) {
    return NULL;
}

struct vr_forwarding_class_qos *vr_qos_get_forwarding_class(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return NULL;
}

bool vr_flow_forward(struct vrouter *router, struct vr_packet *pkt, struct vr_forwarding_md *fmd) {
    return false;
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

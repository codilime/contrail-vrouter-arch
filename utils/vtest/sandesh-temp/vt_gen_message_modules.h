/*
 * Auto generated file
 */
#ifndef __VT_GEN_MESSAGE_MODULES_H__
#define __VT_GEN_MESSAGE_MODULES_H__

#include <string.h>

#include <stdbool.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <vr_types.h>
#include <vt_gen_lib.h>
#include <vtest.h>

struct vt_message_module {
    char *vmm_name;
    void *(*vmm_node)(xmlNodePtr, struct vtest *);
    bool (*vmm_expect)(xmlNodePtr, struct vtest *, void *);
    unsigned int vmm_size;
};

extern void *vr_nexthop_req_node(xmlNodePtr, struct vtest *);
extern bool vr_nexthop_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_interface_req_node(xmlNodePtr, struct vtest *);
extern bool vr_interface_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_vxlan_req_node(xmlNodePtr, struct vtest *);
extern bool vr_vxlan_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_route_req_node(xmlNodePtr, struct vtest *);
extern bool vr_route_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_mpls_req_node(xmlNodePtr, struct vtest *);
extern bool vr_mpls_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_mirror_req_node(xmlNodePtr, struct vtest *);
extern bool vr_mirror_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_flow_req_node(xmlNodePtr, struct vtest *);
extern bool vr_flow_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_vrf_assign_req_node(xmlNodePtr, struct vtest *);
extern bool vr_vrf_assign_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_vrf_stats_req_node(xmlNodePtr, struct vtest *);
extern bool vr_vrf_stats_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_response_node(xmlNodePtr, struct vtest *);
extern bool vr_response_expect(xmlNodePtr, struct vtest *, void *);
extern void *vrouter_ops_node(xmlNodePtr, struct vtest *);
extern bool vrouter_ops_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_mem_stats_req_node(xmlNodePtr, struct vtest *);
extern bool vr_mem_stats_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_drop_stats_req_node(xmlNodePtr, struct vtest *);
extern bool vr_drop_stats_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_qos_map_req_node(xmlNodePtr, struct vtest *);
extern bool vr_qos_map_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vr_fc_map_req_node(xmlNodePtr, struct vtest *);
extern bool vr_fc_map_req_expect(xmlNodePtr, struct vtest *, void *);
extern void *vt_return_node(xmlNodePtr, struct vtest *);

#endif

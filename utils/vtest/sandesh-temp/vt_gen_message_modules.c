/*
 * Auto generated file
 */
#include <string.h>

#include <stdbool.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <vr_types.h>
#include <vt_gen_lib.h>
#include <vtest.h>

#include <vt_gen_message_modules.h>

struct vt_message_module vt_message_modules[] = {
    {
        .vmm_name        =        "vr_nexthop_req",
        .vmm_node        =        vr_nexthop_req_node,
        .vmm_expect        =        vr_nexthop_req_expect,
        .vmm_size        =        sizeof(vr_nexthop_req),
    },
    {
        .vmm_name        =        "vr_interface_req",
        .vmm_node        =        vr_interface_req_node,
        .vmm_expect        =        vr_interface_req_expect,
        .vmm_size        =        sizeof(vr_interface_req),
    },
    {
        .vmm_name        =        "vr_vxlan_req",
        .vmm_node        =        vr_vxlan_req_node,
        .vmm_expect        =        vr_vxlan_req_expect,
        .vmm_size        =        sizeof(vr_vxlan_req),
    },
    {
        .vmm_name        =        "vr_route_req",
        .vmm_node        =        vr_route_req_node,
        .vmm_expect        =        vr_route_req_expect,
        .vmm_size        =        sizeof(vr_route_req),
    },
    {
        .vmm_name        =        "vr_mpls_req",
        .vmm_node        =        vr_mpls_req_node,
        .vmm_expect        =        vr_mpls_req_expect,
        .vmm_size        =        sizeof(vr_mpls_req),
    },
    {
        .vmm_name        =        "vr_mirror_req",
        .vmm_node        =        vr_mirror_req_node,
        .vmm_expect        =        vr_mirror_req_expect,
        .vmm_size        =        sizeof(vr_mirror_req),
    },
    {
        .vmm_name        =        "vr_flow_req",
        .vmm_node        =        vr_flow_req_node,
        .vmm_expect        =        vr_flow_req_expect,
        .vmm_size        =        sizeof(vr_flow_req),
    },
    {
        .vmm_name        =        "vr_vrf_assign_req",
        .vmm_node        =        vr_vrf_assign_req_node,
        .vmm_expect        =        vr_vrf_assign_req_expect,
        .vmm_size        =        sizeof(vr_vrf_assign_req),
    },
    {
        .vmm_name        =        "vr_vrf_stats_req",
        .vmm_node        =        vr_vrf_stats_req_node,
        .vmm_expect        =        vr_vrf_stats_req_expect,
        .vmm_size        =        sizeof(vr_vrf_stats_req),
    },
    {
        .vmm_name        =        "vr_response",
        .vmm_node        =        vr_response_node,
        .vmm_expect        =        vr_response_expect,
        .vmm_size        =        sizeof(vr_response),
    },
    {
        .vmm_name        =        "vrouter_ops",
        .vmm_node        =        vrouter_ops_node,
        .vmm_expect        =        vrouter_ops_expect,
        .vmm_size        =        sizeof(vrouter_ops),
    },
    {
        .vmm_name        =        "vr_mem_stats_req",
        .vmm_node        =        vr_mem_stats_req_node,
        .vmm_expect        =        vr_mem_stats_req_expect,
        .vmm_size        =        sizeof(vr_mem_stats_req),
    },
    {
        .vmm_name        =        "vr_drop_stats_req",
        .vmm_node        =        vr_drop_stats_req_node,
        .vmm_expect        =        vr_drop_stats_req_expect,
        .vmm_size        =        sizeof(vr_drop_stats_req),
    },
    {
        .vmm_name        =        "vr_qos_map_req",
        .vmm_node        =        vr_qos_map_req_node,
        .vmm_expect        =        vr_qos_map_req_expect,
        .vmm_size        =        sizeof(vr_qos_map_req),
    },
    {
        .vmm_name        =        "vr_fc_map_req",
        .vmm_node        =        vr_fc_map_req_node,
        .vmm_expect        =        vr_fc_map_req_expect,
        .vmm_size        =        sizeof(vr_fc_map_req),
    },
    {
        .vmm_name        =        "return",
        .vmm_node        =        vt_return_node,
        .vmm_size        =        0,
    },
};

unsigned int vt_message_modules_num = 
        sizeof(vt_message_modules) / sizeof(vt_message_modules[0]);


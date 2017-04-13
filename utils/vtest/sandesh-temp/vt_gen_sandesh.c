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

void *
vr_nexthop_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_nexthop_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "nhr_type", sizeof("nhr_type"))) {
        if (node->children && node->children->content)
            req->nhr_type = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_family", sizeof("nhr_family"))) {
        if (node->children && node->children->content)
            req->nhr_family = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_id", sizeof("nhr_id"))) {
        if (node->children && node->children->content)
            req->nhr_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_rid", sizeof("nhr_rid"))) {
        if (node->children && node->children->content)
            req->nhr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_encap_oif_id", sizeof("nhr_encap_oif_id"))) {
        if (node->children && node->children->content)
            req->nhr_encap_oif_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_encap_len", sizeof("nhr_encap_len"))) {
        if (node->children && node->children->content)
            req->nhr_encap_len = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_encap_family", sizeof("nhr_encap_family"))) {
        if (node->children && node->children->content)
            req->nhr_encap_family = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_vrf", sizeof("nhr_vrf"))) {
        if (node->children && node->children->content)
            req->nhr_vrf = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_tun_sip", sizeof("nhr_tun_sip"))) {
        if (node->children && node->children->content)
            req->nhr_tun_sip = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_tun_dip", sizeof("nhr_tun_dip"))) {
        if (node->children && node->children->content)
            req->nhr_tun_dip = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_tun_sport", sizeof("nhr_tun_sport"))) {
        if (node->children && node->children->content)
            req->nhr_tun_sport = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_tun_dport", sizeof("nhr_tun_dport"))) {
        if (node->children && node->children->content)
            req->nhr_tun_dport = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_ref_cnt", sizeof("nhr_ref_cnt"))) {
        if (node->children && node->children->content)
            req->nhr_ref_cnt = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_marker", sizeof("nhr_marker"))) {
        if (node->children && node->children->content)
            req->nhr_marker = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_flags", sizeof("nhr_flags"))) {
        if (node->children && node->children->content)
            req->nhr_flags = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_encap", sizeof("nhr_encap"))) {
        if (node->children && node->children->content)
            req->nhr_encap = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->nhr_encap_size = list_size;
        } else if (!strncmp(node->name, "nhr_nh_list", sizeof("nhr_nh_list"))) {
        if (node->children && node->children->content)
            req->nhr_nh_list = vt_gen_list(node->children->content, GEN_TYPE_U32, &list_size);
            req->nhr_nh_list_size = list_size;
        } else if (!strncmp(node->name, "nhr_label", sizeof("nhr_label"))) {
        if (node->children && node->children->content)
            req->nhr_label = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_label_list", sizeof("nhr_label_list"))) {
        if (node->children && node->children->content)
            req->nhr_label_list = vt_gen_list(node->children->content, GEN_TYPE_U32, &list_size);
            req->nhr_label_list_size = list_size;
        } else if (!strncmp(node->name, "nhr_nh_count", sizeof("nhr_nh_count"))) {
        if (node->children && node->children->content)
            req->nhr_nh_count = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "nhr_tun_sip6", sizeof("nhr_tun_sip6"))) {
        if (node->children && node->children->content)
            req->nhr_tun_sip6 = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->nhr_tun_sip6_size = list_size;
        } else if (!strncmp(node->name, "nhr_tun_dip6", sizeof("nhr_tun_dip6"))) {
        if (node->children && node->children->content)
            req->nhr_tun_dip6 = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->nhr_tun_dip6_size = list_size;
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_interface_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_interface_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "vifr_core", sizeof("vifr_core"))) {
        if (node->children && node->children->content)
            req->vifr_core = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_type", sizeof("vifr_type"))) {
        if (node->children && node->children->content)
            req->vifr_type = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_flags", sizeof("vifr_flags"))) {
        if (node->children && node->children->content)
            req->vifr_flags = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_vrf", sizeof("vifr_vrf"))) {
        if (node->children && node->children->content)
            req->vifr_vrf = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_idx", sizeof("vifr_idx"))) {
        if (node->children && node->children->content)
            req->vifr_idx = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_rid", sizeof("vifr_rid"))) {
        if (node->children && node->children->content)
            req->vifr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_os_idx", sizeof("vifr_os_idx"))) {
        if (node->children && node->children->content)
            req->vifr_os_idx = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_mtu", sizeof("vifr_mtu"))) {
        if (node->children && node->children->content)
            req->vifr_mtu = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_name", sizeof("vifr_name"))) {
        if (node->children && node->children->content)
            req->vifr_name = vt_gen_string(node->children->content);
        } else if (!strncmp(node->name, "vifr_ibytes", sizeof("vifr_ibytes"))) {
        if (node->children && node->children->content)
            req->vifr_ibytes = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_ipackets", sizeof("vifr_ipackets"))) {
        if (node->children && node->children->content)
            req->vifr_ipackets = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_ierrors", sizeof("vifr_ierrors"))) {
        if (node->children && node->children->content)
            req->vifr_ierrors = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_obytes", sizeof("vifr_obytes"))) {
        if (node->children && node->children->content)
            req->vifr_obytes = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_opackets", sizeof("vifr_opackets"))) {
        if (node->children && node->children->content)
            req->vifr_opackets = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_oerrors", sizeof("vifr_oerrors"))) {
        if (node->children && node->children->content)
            req->vifr_oerrors = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_queue_ipackets", sizeof("vifr_queue_ipackets"))) {
        if (node->children && node->children->content)
            req->vifr_queue_ipackets = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_queue_ierrors", sizeof("vifr_queue_ierrors"))) {
        if (node->children && node->children->content)
            req->vifr_queue_ierrors = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_queue_ierrors_to_lcore", sizeof("vifr_queue_ierrors_to_lcore"))) {
        if (node->children && node->children->content)
            req->vifr_queue_ierrors_to_lcore = vt_gen_list(node->children->content, GEN_TYPE_U64, &list_size);
            req->vifr_queue_ierrors_to_lcore_size = list_size;
        } else if (!strncmp(node->name, "vifr_queue_opackets", sizeof("vifr_queue_opackets"))) {
        if (node->children && node->children->content)
            req->vifr_queue_opackets = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_queue_oerrors", sizeof("vifr_queue_oerrors"))) {
        if (node->children && node->children->content)
            req->vifr_queue_oerrors = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_port_ipackets", sizeof("vifr_port_ipackets"))) {
        if (node->children && node->children->content)
            req->vifr_port_ipackets = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_port_ierrors", sizeof("vifr_port_ierrors"))) {
        if (node->children && node->children->content)
            req->vifr_port_ierrors = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_port_isyscalls", sizeof("vifr_port_isyscalls"))) {
        if (node->children && node->children->content)
            req->vifr_port_isyscalls = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_port_inombufs", sizeof("vifr_port_inombufs"))) {
        if (node->children && node->children->content)
            req->vifr_port_inombufs = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_port_opackets", sizeof("vifr_port_opackets"))) {
        if (node->children && node->children->content)
            req->vifr_port_opackets = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_port_oerrors", sizeof("vifr_port_oerrors"))) {
        if (node->children && node->children->content)
            req->vifr_port_oerrors = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_port_osyscalls", sizeof("vifr_port_osyscalls"))) {
        if (node->children && node->children->content)
            req->vifr_port_osyscalls = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_dev_ibytes", sizeof("vifr_dev_ibytes"))) {
        if (node->children && node->children->content)
            req->vifr_dev_ibytes = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_dev_ipackets", sizeof("vifr_dev_ipackets"))) {
        if (node->children && node->children->content)
            req->vifr_dev_ipackets = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_dev_ierrors", sizeof("vifr_dev_ierrors"))) {
        if (node->children && node->children->content)
            req->vifr_dev_ierrors = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_dev_inombufs", sizeof("vifr_dev_inombufs"))) {
        if (node->children && node->children->content)
            req->vifr_dev_inombufs = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_dev_obytes", sizeof("vifr_dev_obytes"))) {
        if (node->children && node->children->content)
            req->vifr_dev_obytes = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_dev_opackets", sizeof("vifr_dev_opackets"))) {
        if (node->children && node->children->content)
            req->vifr_dev_opackets = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_dev_oerrors", sizeof("vifr_dev_oerrors"))) {
        if (node->children && node->children->content)
            req->vifr_dev_oerrors = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_ref_cnt", sizeof("vifr_ref_cnt"))) {
        if (node->children && node->children->content)
            req->vifr_ref_cnt = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_marker", sizeof("vifr_marker"))) {
        if (node->children && node->children->content)
            req->vifr_marker = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_mac", sizeof("vifr_mac"))) {
        if (node->children && node->children->content)
            req->vifr_mac = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->vifr_mac_size = list_size;
        } else if (!strncmp(node->name, "vifr_ip", sizeof("vifr_ip"))) {
        if (node->children && node->children->content)
            req->vifr_ip = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_context", sizeof("vifr_context"))) {
        if (node->children && node->children->content)
            req->vifr_context = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_mir_id", sizeof("vifr_mir_id"))) {
        if (node->children && node->children->content)
            req->vifr_mir_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_speed", sizeof("vifr_speed"))) {
        if (node->children && node->children->content)
            req->vifr_speed = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_duplex", sizeof("vifr_duplex"))) {
        if (node->children && node->children->content)
            req->vifr_duplex = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_vlan_id", sizeof("vifr_vlan_id"))) {
        if (node->children && node->children->content)
            req->vifr_vlan_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_parent_vif_idx", sizeof("vifr_parent_vif_idx"))) {
        if (node->children && node->children->content)
            req->vifr_parent_vif_idx = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_nh_id", sizeof("vifr_nh_id"))) {
        if (node->children && node->children->content)
            req->vifr_nh_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_cross_connect_idx", sizeof("vifr_cross_connect_idx"))) {
        if (node->children && node->children->content)
            req->vifr_cross_connect_idx = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_src_mac", sizeof("vifr_src_mac"))) {
        if (node->children && node->children->content)
            req->vifr_src_mac = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->vifr_src_mac_size = list_size;
        } else if (!strncmp(node->name, "vifr_bridge_idx", sizeof("vifr_bridge_idx"))) {
        if (node->children && node->children->content)
            req->vifr_bridge_idx = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_ovlan_id", sizeof("vifr_ovlan_id"))) {
        if (node->children && node->children->content)
            req->vifr_ovlan_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_transport", sizeof("vifr_transport"))) {
        if (node->children && node->children->content)
            req->vifr_transport = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vifr_fat_flow_protocol_port", sizeof("vifr_fat_flow_protocol_port"))) {
        if (node->children && node->children->content)
            req->vifr_fat_flow_protocol_port = vt_gen_list(node->children->content, GEN_TYPE_U32, &list_size);
            req->vifr_fat_flow_protocol_port_size = list_size;
        } else if (!strncmp(node->name, "vifr_qos_map_index", sizeof("vifr_qos_map_index"))) {
        if (node->children && node->children->content)
            req->vifr_qos_map_index = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_vxlan_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_vxlan_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "vxlanr_rid", sizeof("vxlanr_rid"))) {
        if (node->children && node->children->content)
            req->vxlanr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vxlanr_vnid", sizeof("vxlanr_vnid"))) {
        if (node->children && node->children->content)
            req->vxlanr_vnid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vxlanr_nhid", sizeof("vxlanr_nhid"))) {
        if (node->children && node->children->content)
            req->vxlanr_nhid = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_route_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_route_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "rtr_vrf_id", sizeof("rtr_vrf_id"))) {
        if (node->children && node->children->content)
            req->rtr_vrf_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "rtr_family", sizeof("rtr_family"))) {
        if (node->children && node->children->content)
            req->rtr_family = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "rtr_prefix", sizeof("rtr_prefix"))) {
        if (node->children && node->children->content)
            req->rtr_prefix = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->rtr_prefix_size = list_size;
        } else if (!strncmp(node->name, "rtr_prefix_len", sizeof("rtr_prefix_len"))) {
        if (node->children && node->children->content)
            req->rtr_prefix_len = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "rtr_rid", sizeof("rtr_rid"))) {
        if (node->children && node->children->content)
            req->rtr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "rtr_label_flags", sizeof("rtr_label_flags"))) {
        if (node->children && node->children->content)
            req->rtr_label_flags = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "rtr_label", sizeof("rtr_label"))) {
        if (node->children && node->children->content)
            req->rtr_label = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "rtr_nh_id", sizeof("rtr_nh_id"))) {
        if (node->children && node->children->content)
            req->rtr_nh_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "rtr_marker", sizeof("rtr_marker"))) {
        if (node->children && node->children->content)
            req->rtr_marker = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->rtr_marker_size = list_size;
        } else if (!strncmp(node->name, "rtr_marker_plen", sizeof("rtr_marker_plen"))) {
        if (node->children && node->children->content)
            req->rtr_marker_plen = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "rtr_mac", sizeof("rtr_mac"))) {
        if (node->children && node->children->content)
            req->rtr_mac = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->rtr_mac_size = list_size;
        } else if (!strncmp(node->name, "rtr_replace_plen", sizeof("rtr_replace_plen"))) {
        if (node->children && node->children->content)
            req->rtr_replace_plen = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "rtr_index", sizeof("rtr_index"))) {
        if (node->children && node->children->content)
            req->rtr_index = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_mpls_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_mpls_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "mr_label", sizeof("mr_label"))) {
        if (node->children && node->children->content)
            req->mr_label = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "mr_rid", sizeof("mr_rid"))) {
        if (node->children && node->children->content)
            req->mr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "mr_nhid", sizeof("mr_nhid"))) {
        if (node->children && node->children->content)
            req->mr_nhid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "mr_marker", sizeof("mr_marker"))) {
        if (node->children && node->children->content)
            req->mr_marker = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_mirror_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_mirror_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "mirr_index", sizeof("mirr_index"))) {
        if (node->children && node->children->content)
            req->mirr_index = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "mirr_rid", sizeof("mirr_rid"))) {
        if (node->children && node->children->content)
            req->mirr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "mirr_nhid", sizeof("mirr_nhid"))) {
        if (node->children && node->children->content)
            req->mirr_nhid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "mirr_users", sizeof("mirr_users"))) {
        if (node->children && node->children->content)
            req->mirr_users = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "mirr_flags", sizeof("mirr_flags"))) {
        if (node->children && node->children->content)
            req->mirr_flags = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "mirr_marker", sizeof("mirr_marker"))) {
        if (node->children && node->children->content)
            req->mirr_marker = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "mirr_vni", sizeof("mirr_vni"))) {
        if (node->children && node->children->content)
            req->mirr_vni = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_flow_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_flow_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "fr_op", sizeof("fr_op"))) {
        if (node->children && node->children->content)
            req->fr_op = vt_gen_flow_op(node->children->content);
        } else if (!strncmp(node->name, "fr_rid", sizeof("fr_rid"))) {
        if (node->children && node->children->content)
            req->fr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_index", sizeof("fr_index"))) {
        if (node->children && node->children->content)
            req->fr_index = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_action", sizeof("fr_action"))) {
        if (node->children && node->children->content)
            req->fr_action = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_flags", sizeof("fr_flags"))) {
        if (node->children && node->children->content)
            req->fr_flags = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_ftable_size", sizeof("fr_ftable_size"))) {
        if (node->children && node->children->content)
            req->fr_ftable_size = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_ftable_dev", sizeof("fr_ftable_dev"))) {
        if (node->children && node->children->content)
            req->fr_ftable_dev = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_rindex", sizeof("fr_rindex"))) {
        if (node->children && node->children->content)
            req->fr_rindex = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_family", sizeof("fr_family"))) {
        if (node->children && node->children->content)
            req->fr_family = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_flow_ip", sizeof("fr_flow_ip"))) {
        if (node->children && node->children->content)
            req->fr_flow_ip = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->fr_flow_ip_size = list_size;
        } else if (!strncmp(node->name, "fr_flow_sport", sizeof("fr_flow_sport"))) {
        if (node->children && node->children->content)
            req->fr_flow_sport = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_flow_dport", sizeof("fr_flow_dport"))) {
        if (node->children && node->children->content)
            req->fr_flow_dport = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_flow_proto", sizeof("fr_flow_proto"))) {
        if (node->children && node->children->content)
            req->fr_flow_proto = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_flow_vrf", sizeof("fr_flow_vrf"))) {
        if (node->children && node->children->content)
            req->fr_flow_vrf = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_flow_dvrf", sizeof("fr_flow_dvrf"))) {
        if (node->children && node->children->content)
            req->fr_flow_dvrf = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_mir_id", sizeof("fr_mir_id"))) {
        if (node->children && node->children->content)
            req->fr_mir_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_sec_mir_id", sizeof("fr_sec_mir_id"))) {
        if (node->children && node->children->content)
            req->fr_sec_mir_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_mir_sip", sizeof("fr_mir_sip"))) {
        if (node->children && node->children->content)
            req->fr_mir_sip = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_mir_sport", sizeof("fr_mir_sport"))) {
        if (node->children && node->children->content)
            req->fr_mir_sport = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_pcap_meta_data", sizeof("fr_pcap_meta_data"))) {
        if (node->children && node->children->content)
            req->fr_pcap_meta_data = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->fr_pcap_meta_data_size = list_size;
        } else if (!strncmp(node->name, "fr_mir_vrf", sizeof("fr_mir_vrf"))) {
        if (node->children && node->children->content)
            req->fr_mir_vrf = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_ecmp_nh_index", sizeof("fr_ecmp_nh_index"))) {
        if (node->children && node->children->content)
            req->fr_ecmp_nh_index = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_src_nh_index", sizeof("fr_src_nh_index"))) {
        if (node->children && node->children->content)
            req->fr_src_nh_index = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_flow_nh_id", sizeof("fr_flow_nh_id"))) {
        if (node->children && node->children->content)
            req->fr_flow_nh_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_drop_reason", sizeof("fr_drop_reason"))) {
        if (node->children && node->children->content)
            req->fr_drop_reason = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_file_path", sizeof("fr_file_path"))) {
        if (node->children && node->children->content)
            req->fr_file_path = vt_gen_string(node->children->content);
        } else if (!strncmp(node->name, "fr_processed", sizeof("fr_processed"))) {
        if (node->children && node->children->content)
            req->fr_processed = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_created", sizeof("fr_created"))) {
        if (node->children && node->children->content)
            req->fr_created = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_added", sizeof("fr_added"))) {
        if (node->children && node->children->content)
            req->fr_added = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_cpus", sizeof("fr_cpus"))) {
        if (node->children && node->children->content)
            req->fr_cpus = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_hold_oflows", sizeof("fr_hold_oflows"))) {
        if (node->children && node->children->content)
            req->fr_hold_oflows = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_hold_stat", sizeof("fr_hold_stat"))) {
        if (node->children && node->children->content)
            req->fr_hold_stat = vt_gen_list(node->children->content, GEN_TYPE_U32, &list_size);
            req->fr_hold_stat_size = list_size;
        } else if (!strncmp(node->name, "fr_flow_bytes", sizeof("fr_flow_bytes"))) {
        if (node->children && node->children->content)
            req->fr_flow_bytes = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_flow_packets", sizeof("fr_flow_packets"))) {
        if (node->children && node->children->content)
            req->fr_flow_packets = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_flow_stats_oflow", sizeof("fr_flow_stats_oflow"))) {
        if (node->children && node->children->content)
            req->fr_flow_stats_oflow = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_oflow_entries", sizeof("fr_oflow_entries"))) {
        if (node->children && node->children->content)
            req->fr_oflow_entries = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_gen_id", sizeof("fr_gen_id"))) {
        if (node->children && node->children->content)
            req->fr_gen_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_qos_id", sizeof("fr_qos_id"))) {
        if (node->children && node->children->content)
            req->fr_qos_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fr_ttl", sizeof("fr_ttl"))) {
        if (node->children && node->children->content)
            req->fr_ttl = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_vrf_assign_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_vrf_assign_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "var_rid", sizeof("var_rid"))) {
        if (node->children && node->children->content)
            req->var_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "var_vif_index", sizeof("var_vif_index"))) {
        if (node->children && node->children->content)
            req->var_vif_index = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "var_vif_vrf", sizeof("var_vif_vrf"))) {
        if (node->children && node->children->content)
            req->var_vif_vrf = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "var_vlan_id", sizeof("var_vlan_id"))) {
        if (node->children && node->children->content)
            req->var_vlan_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "var_marker", sizeof("var_marker"))) {
        if (node->children && node->children->content)
            req->var_marker = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "var_nh_id", sizeof("var_nh_id"))) {
        if (node->children && node->children->content)
            req->var_nh_id = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_vrf_stats_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_vrf_stats_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "vsr_rid", sizeof("vsr_rid"))) {
        if (node->children && node->children->content)
            req->vsr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_family", sizeof("vsr_family"))) {
        if (node->children && node->children->content)
            req->vsr_family = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_type", sizeof("vsr_type"))) {
        if (node->children && node->children->content)
            req->vsr_type = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_vrf", sizeof("vsr_vrf"))) {
        if (node->children && node->children->content)
            req->vsr_vrf = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_discards", sizeof("vsr_discards"))) {
        if (node->children && node->children->content)
            req->vsr_discards = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_resolves", sizeof("vsr_resolves"))) {
        if (node->children && node->children->content)
            req->vsr_resolves = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_receives", sizeof("vsr_receives"))) {
        if (node->children && node->children->content)
            req->vsr_receives = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_ecmp_composites", sizeof("vsr_ecmp_composites"))) {
        if (node->children && node->children->content)
            req->vsr_ecmp_composites = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_l2_mcast_composites", sizeof("vsr_l2_mcast_composites"))) {
        if (node->children && node->children->content)
            req->vsr_l2_mcast_composites = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_fabric_composites", sizeof("vsr_fabric_composites"))) {
        if (node->children && node->children->content)
            req->vsr_fabric_composites = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_udp_tunnels", sizeof("vsr_udp_tunnels"))) {
        if (node->children && node->children->content)
            req->vsr_udp_tunnels = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_udp_mpls_tunnels", sizeof("vsr_udp_mpls_tunnels"))) {
        if (node->children && node->children->content)
            req->vsr_udp_mpls_tunnels = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_gre_mpls_tunnels", sizeof("vsr_gre_mpls_tunnels"))) {
        if (node->children && node->children->content)
            req->vsr_gre_mpls_tunnels = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_l2_encaps", sizeof("vsr_l2_encaps"))) {
        if (node->children && node->children->content)
            req->vsr_l2_encaps = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_encaps", sizeof("vsr_encaps"))) {
        if (node->children && node->children->content)
            req->vsr_encaps = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_marker", sizeof("vsr_marker"))) {
        if (node->children && node->children->content)
            req->vsr_marker = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_gros", sizeof("vsr_gros"))) {
        if (node->children && node->children->content)
            req->vsr_gros = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_diags", sizeof("vsr_diags"))) {
        if (node->children && node->children->content)
            req->vsr_diags = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_encap_composites", sizeof("vsr_encap_composites"))) {
        if (node->children && node->children->content)
            req->vsr_encap_composites = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_evpn_composites", sizeof("vsr_evpn_composites"))) {
        if (node->children && node->children->content)
            req->vsr_evpn_composites = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_vrf_translates", sizeof("vsr_vrf_translates"))) {
        if (node->children && node->children->content)
            req->vsr_vrf_translates = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_vxlan_tunnels", sizeof("vsr_vxlan_tunnels"))) {
        if (node->children && node->children->content)
            req->vsr_vxlan_tunnels = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_arp_virtual_proxy", sizeof("vsr_arp_virtual_proxy"))) {
        if (node->children && node->children->content)
            req->vsr_arp_virtual_proxy = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_arp_virtual_stitch", sizeof("vsr_arp_virtual_stitch"))) {
        if (node->children && node->children->content)
            req->vsr_arp_virtual_stitch = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_arp_virtual_flood", sizeof("vsr_arp_virtual_flood"))) {
        if (node->children && node->children->content)
            req->vsr_arp_virtual_flood = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_arp_physical_stitch", sizeof("vsr_arp_physical_stitch"))) {
        if (node->children && node->children->content)
            req->vsr_arp_physical_stitch = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_arp_tor_proxy", sizeof("vsr_arp_tor_proxy"))) {
        if (node->children && node->children->content)
            req->vsr_arp_tor_proxy = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_arp_physical_flood", sizeof("vsr_arp_physical_flood"))) {
        if (node->children && node->children->content)
            req->vsr_arp_physical_flood = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_l2_receives", sizeof("vsr_l2_receives"))) {
        if (node->children && node->children->content)
            req->vsr_l2_receives = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vsr_uuc_floods", sizeof("vsr_uuc_floods"))) {
        if (node->children && node->children->content)
            req->vsr_uuc_floods = strtoull(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_response_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_response *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "resp_code", sizeof("resp_code"))) {
        if (node->children && node->children->content)
            req->resp_code = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vrouter_ops_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vrouter_ops *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "vo_rid", sizeof("vo_rid"))) {
        if (node->children && node->children->content)
            req->vo_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_mpls_labels", sizeof("vo_mpls_labels"))) {
        if (node->children && node->children->content)
            req->vo_mpls_labels = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_nexthops", sizeof("vo_nexthops"))) {
        if (node->children && node->children->content)
            req->vo_nexthops = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_bridge_entries", sizeof("vo_bridge_entries"))) {
        if (node->children && node->children->content)
            req->vo_bridge_entries = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_oflow_bridge_entries", sizeof("vo_oflow_bridge_entries"))) {
        if (node->children && node->children->content)
            req->vo_oflow_bridge_entries = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_flow_entries", sizeof("vo_flow_entries"))) {
        if (node->children && node->children->content)
            req->vo_flow_entries = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_oflow_entries", sizeof("vo_oflow_entries"))) {
        if (node->children && node->children->content)
            req->vo_oflow_entries = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_interfaces", sizeof("vo_interfaces"))) {
        if (node->children && node->children->content)
            req->vo_interfaces = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_mirror_entries", sizeof("vo_mirror_entries"))) {
        if (node->children && node->children->content)
            req->vo_mirror_entries = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_vrfs", sizeof("vo_vrfs"))) {
        if (node->children && node->children->content)
            req->vo_vrfs = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_build_info", sizeof("vo_build_info"))) {
        if (node->children && node->children->content)
            req->vo_build_info = vt_gen_string(node->children->content);
        } else if (!strncmp(node->name, "vo_log_level", sizeof("vo_log_level"))) {
        if (node->children && node->children->content)
            req->vo_log_level = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_log_type_enable", sizeof("vo_log_type_enable"))) {
        if (node->children && node->children->content)
            req->vo_log_type_enable = vt_gen_list(node->children->content, GEN_TYPE_U32, &list_size);
            req->vo_log_type_enable_size = list_size;
        } else if (!strncmp(node->name, "vo_log_type_disable", sizeof("vo_log_type_disable"))) {
        if (node->children && node->children->content)
            req->vo_log_type_disable = vt_gen_list(node->children->content, GEN_TYPE_U32, &list_size);
            req->vo_log_type_disable_size = list_size;
        } else if (!strncmp(node->name, "vo_perfr", sizeof("vo_perfr"))) {
        if (node->children && node->children->content)
            req->vo_perfr = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_perfs", sizeof("vo_perfs"))) {
        if (node->children && node->children->content)
            req->vo_perfs = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_from_vm_mss_adj", sizeof("vo_from_vm_mss_adj"))) {
        if (node->children && node->children->content)
            req->vo_from_vm_mss_adj = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_to_vm_mss_adj", sizeof("vo_to_vm_mss_adj"))) {
        if (node->children && node->children->content)
            req->vo_to_vm_mss_adj = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_perfr1", sizeof("vo_perfr1"))) {
        if (node->children && node->children->content)
            req->vo_perfr1 = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_perfr2", sizeof("vo_perfr2"))) {
        if (node->children && node->children->content)
            req->vo_perfr2 = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_perfr3", sizeof("vo_perfr3"))) {
        if (node->children && node->children->content)
            req->vo_perfr3 = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_perfp", sizeof("vo_perfp"))) {
        if (node->children && node->children->content)
            req->vo_perfp = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_perfq1", sizeof("vo_perfq1"))) {
        if (node->children && node->children->content)
            req->vo_perfq1 = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_perfq2", sizeof("vo_perfq2"))) {
        if (node->children && node->children->content)
            req->vo_perfq2 = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_perfq3", sizeof("vo_perfq3"))) {
        if (node->children && node->children->content)
            req->vo_perfq3 = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_udp_coff", sizeof("vo_udp_coff"))) {
        if (node->children && node->children->content)
            req->vo_udp_coff = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_flow_hold_limit", sizeof("vo_flow_hold_limit"))) {
        if (node->children && node->children->content)
            req->vo_flow_hold_limit = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_mudp", sizeof("vo_mudp"))) {
        if (node->children && node->children->content)
            req->vo_mudp = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_flow_used_entries", sizeof("vo_flow_used_entries"))) {
        if (node->children && node->children->content)
            req->vo_flow_used_entries = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_flow_used_oentries", sizeof("vo_flow_used_oentries"))) {
        if (node->children && node->children->content)
            req->vo_flow_used_oentries = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_bridge_used_entries", sizeof("vo_bridge_used_entries"))) {
        if (node->children && node->children->content)
            req->vo_bridge_used_entries = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vo_bridge_used_oentries", sizeof("vo_bridge_used_oentries"))) {
        if (node->children && node->children->content)
            req->vo_bridge_used_oentries = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_mem_stats_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_mem_stats_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "vms_rid", sizeof("vms_rid"))) {
        if (node->children && node->children->content)
            req->vms_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_alloced", sizeof("vms_alloced"))) {
        if (node->children && node->children->content)
            req->vms_alloced = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_freed", sizeof("vms_freed"))) {
        if (node->children && node->children->content)
            req->vms_freed = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_assembler_table_object", sizeof("vms_assembler_table_object"))) {
        if (node->children && node->children->content)
            req->vms_assembler_table_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_bridge_mac_object", sizeof("vms_bridge_mac_object"))) {
        if (node->children && node->children->content)
            req->vms_bridge_mac_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_btable_object", sizeof("vms_btable_object"))) {
        if (node->children && node->children->content)
            req->vms_btable_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_build_info_object", sizeof("vms_build_info_object"))) {
        if (node->children && node->children->content)
            req->vms_build_info_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_defer_object", sizeof("vms_defer_object"))) {
        if (node->children && node->children->content)
            req->vms_defer_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_drop_stats_object", sizeof("vms_drop_stats_object"))) {
        if (node->children && node->children->content)
            req->vms_drop_stats_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_drop_stats_req_object", sizeof("vms_drop_stats_req_object"))) {
        if (node->children && node->children->content)
            req->vms_drop_stats_req_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_flow_queue_object", sizeof("vms_flow_queue_object"))) {
        if (node->children && node->children->content)
            req->vms_flow_queue_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_flow_req_object", sizeof("vms_flow_req_object"))) {
        if (node->children && node->children->content)
            req->vms_flow_req_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_flow_req_path_object", sizeof("vms_flow_req_path_object"))) {
        if (node->children && node->children->content)
            req->vms_flow_req_path_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_flow_hold_stat_object", sizeof("vms_flow_hold_stat_object"))) {
        if (node->children && node->children->content)
            req->vms_flow_hold_stat_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_flow_link_local_object", sizeof("vms_flow_link_local_object"))) {
        if (node->children && node->children->content)
            req->vms_flow_link_local_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_flow_metadata_object", sizeof("vms_flow_metadata_object"))) {
        if (node->children && node->children->content)
            req->vms_flow_metadata_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_flow_table_info_object", sizeof("vms_flow_table_info_object"))) {
        if (node->children && node->children->content)
            req->vms_flow_table_info_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_fragment_object", sizeof("vms_fragment_object"))) {
        if (node->children && node->children->content)
            req->vms_fragment_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_fragment_queue_object", sizeof("vms_fragment_queue_object"))) {
        if (node->children && node->children->content)
            req->vms_fragment_queue_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_fragment_queue_element_object", sizeof("vms_fragment_queue_element_object"))) {
        if (node->children && node->children->content)
            req->vms_fragment_queue_element_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_fragment_scanner_object", sizeof("vms_fragment_scanner_object"))) {
        if (node->children && node->children->content)
            req->vms_fragment_scanner_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_hpacket_pool_object", sizeof("vms_hpacket_pool_object"))) {
        if (node->children && node->children->content)
            req->vms_hpacket_pool_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_htable_object", sizeof("vms_htable_object"))) {
        if (node->children && node->children->content)
            req->vms_htable_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_interface_object", sizeof("vms_interface_object"))) {
        if (node->children && node->children->content)
            req->vms_interface_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_interface_mac_object", sizeof("vms_interface_mac_object"))) {
        if (node->children && node->children->content)
            req->vms_interface_mac_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_interface_req_object", sizeof("vms_interface_req_object"))) {
        if (node->children && node->children->content)
            req->vms_interface_req_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_interface_req_mac_object", sizeof("vms_interface_req_mac_object"))) {
        if (node->children && node->children->content)
            req->vms_interface_req_mac_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_interface_req_name_object", sizeof("vms_interface_req_name_object"))) {
        if (node->children && node->children->content)
            req->vms_interface_req_name_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_interface_stats_object", sizeof("vms_interface_stats_object"))) {
        if (node->children && node->children->content)
            req->vms_interface_stats_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_interface_table_object", sizeof("vms_interface_table_object"))) {
        if (node->children && node->children->content)
            req->vms_interface_table_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_interface_vrf_table_object", sizeof("vms_interface_vrf_table_object"))) {
        if (node->children && node->children->content)
            req->vms_interface_vrf_table_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_itable_object", sizeof("vms_itable_object"))) {
        if (node->children && node->children->content)
            req->vms_itable_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_malloc_object", sizeof("vms_malloc_object"))) {
        if (node->children && node->children->content)
            req->vms_malloc_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_message_object", sizeof("vms_message_object"))) {
        if (node->children && node->children->content)
            req->vms_message_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_message_response_object", sizeof("vms_message_response_object"))) {
        if (node->children && node->children->content)
            req->vms_message_response_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_message_dump_object", sizeof("vms_message_dump_object"))) {
        if (node->children && node->children->content)
            req->vms_message_dump_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_mem_stats_req_object", sizeof("vms_mem_stats_req_object"))) {
        if (node->children && node->children->content)
            req->vms_mem_stats_req_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_mirror_object", sizeof("vms_mirror_object"))) {
        if (node->children && node->children->content)
            req->vms_mirror_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_mirror_table_object", sizeof("vms_mirror_table_object"))) {
        if (node->children && node->children->content)
            req->vms_mirror_table_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_mirror_meta_object", sizeof("vms_mirror_meta_object"))) {
        if (node->children && node->children->content)
            req->vms_mirror_meta_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_mtrie_object", sizeof("vms_mtrie_object"))) {
        if (node->children && node->children->content)
            req->vms_mtrie_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_mtrie_bucket_object", sizeof("vms_mtrie_bucket_object"))) {
        if (node->children && node->children->content)
            req->vms_mtrie_bucket_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_mtrie_stats_object", sizeof("vms_mtrie_stats_object"))) {
        if (node->children && node->children->content)
            req->vms_mtrie_stats_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_mtrie_table_object", sizeof("vms_mtrie_table_object"))) {
        if (node->children && node->children->content)
            req->vms_mtrie_table_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_network_address_object", sizeof("vms_network_address_object"))) {
        if (node->children && node->children->content)
            req->vms_network_address_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_nexthop_object", sizeof("vms_nexthop_object"))) {
        if (node->children && node->children->content)
            req->vms_nexthop_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_nexthop_component_object", sizeof("vms_nexthop_component_object"))) {
        if (node->children && node->children->content)
            req->vms_nexthop_component_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_nexthop_req_list_object", sizeof("vms_nexthop_req_list_object"))) {
        if (node->children && node->children->content)
            req->vms_nexthop_req_list_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_nexthop_req_encap_object", sizeof("vms_nexthop_req_encap_object"))) {
        if (node->children && node->children->content)
            req->vms_nexthop_req_encap_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_nexthop_req_object", sizeof("vms_nexthop_req_object"))) {
        if (node->children && node->children->content)
            req->vms_nexthop_req_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_route_table_object", sizeof("vms_route_table_object"))) {
        if (node->children && node->children->content)
            req->vms_route_table_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_route_req_mac_object", sizeof("vms_route_req_mac_object"))) {
        if (node->children && node->children->content)
            req->vms_route_req_mac_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_timer_object", sizeof("vms_timer_object"))) {
        if (node->children && node->children->content)
            req->vms_timer_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_usock_object", sizeof("vms_usock_object"))) {
        if (node->children && node->children->content)
            req->vms_usock_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_usock_poll_object", sizeof("vms_usock_poll_object"))) {
        if (node->children && node->children->content)
            req->vms_usock_poll_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_usock_buf_object", sizeof("vms_usock_buf_object"))) {
        if (node->children && node->children->content)
            req->vms_usock_buf_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_usock_iovec_object", sizeof("vms_usock_iovec_object"))) {
        if (node->children && node->children->content)
            req->vms_usock_iovec_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_vrouter_req_object", sizeof("vms_vrouter_req_object"))) {
        if (node->children && node->children->content)
            req->vms_vrouter_req_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_interface_fat_flow_config_object", sizeof("vms_interface_fat_flow_config_object"))) {
        if (node->children && node->children->content)
            req->vms_interface_fat_flow_config_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_qos_map_object", sizeof("vms_qos_map_object"))) {
        if (node->children && node->children->content)
            req->vms_qos_map_object = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vms_fc_object", sizeof("vms_fc_object"))) {
        if (node->children && node->children->content)
            req->vms_fc_object = strtoull(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_drop_stats_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_drop_stats_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "vds_core", sizeof("vds_core"))) {
        if (node->children && node->children->content)
            req->vds_core = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_rid", sizeof("vds_rid"))) {
        if (node->children && node->children->content)
            req->vds_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_discard", sizeof("vds_discard"))) {
        if (node->children && node->children->content)
            req->vds_discard = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_pull", sizeof("vds_pull"))) {
        if (node->children && node->children->content)
            req->vds_pull = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_invalid_if", sizeof("vds_invalid_if"))) {
        if (node->children && node->children->content)
            req->vds_invalid_if = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_arp_no_where_to_go", sizeof("vds_arp_no_where_to_go"))) {
        if (node->children && node->children->content)
            req->vds_arp_no_where_to_go = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_garp_from_vm", sizeof("vds_garp_from_vm"))) {
        if (node->children && node->children->content)
            req->vds_garp_from_vm = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_invalid_arp", sizeof("vds_invalid_arp"))) {
        if (node->children && node->children->content)
            req->vds_invalid_arp = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_trap_no_if", sizeof("vds_trap_no_if"))) {
        if (node->children && node->children->content)
            req->vds_trap_no_if = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_nowhere_to_go", sizeof("vds_nowhere_to_go"))) {
        if (node->children && node->children->content)
            req->vds_nowhere_to_go = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flow_queue_limit_exceeded", sizeof("vds_flow_queue_limit_exceeded"))) {
        if (node->children && node->children->content)
            req->vds_flow_queue_limit_exceeded = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flow_no_memory", sizeof("vds_flow_no_memory"))) {
        if (node->children && node->children->content)
            req->vds_flow_no_memory = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flow_invalid_protocol", sizeof("vds_flow_invalid_protocol"))) {
        if (node->children && node->children->content)
            req->vds_flow_invalid_protocol = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flow_nat_no_rflow", sizeof("vds_flow_nat_no_rflow"))) {
        if (node->children && node->children->content)
            req->vds_flow_nat_no_rflow = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flow_action_drop", sizeof("vds_flow_action_drop"))) {
        if (node->children && node->children->content)
            req->vds_flow_action_drop = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flow_action_invalid", sizeof("vds_flow_action_invalid"))) {
        if (node->children && node->children->content)
            req->vds_flow_action_invalid = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flow_unusable", sizeof("vds_flow_unusable"))) {
        if (node->children && node->children->content)
            req->vds_flow_unusable = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flow_table_full", sizeof("vds_flow_table_full"))) {
        if (node->children && node->children->content)
            req->vds_flow_table_full = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_interface_tx_discard", sizeof("vds_interface_tx_discard"))) {
        if (node->children && node->children->content)
            req->vds_interface_tx_discard = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_interface_drop", sizeof("vds_interface_drop"))) {
        if (node->children && node->children->content)
            req->vds_interface_drop = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_duplicated", sizeof("vds_duplicated"))) {
        if (node->children && node->children->content)
            req->vds_duplicated = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_push", sizeof("vds_push"))) {
        if (node->children && node->children->content)
            req->vds_push = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_ttl_exceeded", sizeof("vds_ttl_exceeded"))) {
        if (node->children && node->children->content)
            req->vds_ttl_exceeded = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_invalid_nh", sizeof("vds_invalid_nh"))) {
        if (node->children && node->children->content)
            req->vds_invalid_nh = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_invalid_label", sizeof("vds_invalid_label"))) {
        if (node->children && node->children->content)
            req->vds_invalid_label = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_invalid_protocol", sizeof("vds_invalid_protocol"))) {
        if (node->children && node->children->content)
            req->vds_invalid_protocol = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_interface_rx_discard", sizeof("vds_interface_rx_discard"))) {
        if (node->children && node->children->content)
            req->vds_interface_rx_discard = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_invalid_mcast_source", sizeof("vds_invalid_mcast_source"))) {
        if (node->children && node->children->content)
            req->vds_invalid_mcast_source = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_head_alloc_fail", sizeof("vds_head_alloc_fail"))) {
        if (node->children && node->children->content)
            req->vds_head_alloc_fail = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_head_space_reserve_fail", sizeof("vds_head_space_reserve_fail"))) {
        if (node->children && node->children->content)
            req->vds_head_space_reserve_fail = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_pcow_fail", sizeof("vds_pcow_fail"))) {
        if (node->children && node->children->content)
            req->vds_pcow_fail = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flood", sizeof("vds_flood"))) {
        if (node->children && node->children->content)
            req->vds_flood = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_mcast_clone_fail", sizeof("vds_mcast_clone_fail"))) {
        if (node->children && node->children->content)
            req->vds_mcast_clone_fail = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_no_memory", sizeof("vds_no_memory"))) {
        if (node->children && node->children->content)
            req->vds_no_memory = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_rewrite_fail", sizeof("vds_rewrite_fail"))) {
        if (node->children && node->children->content)
            req->vds_rewrite_fail = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_misc", sizeof("vds_misc"))) {
        if (node->children && node->children->content)
            req->vds_misc = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_invalid_packet", sizeof("vds_invalid_packet"))) {
        if (node->children && node->children->content)
            req->vds_invalid_packet = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_cksum_err", sizeof("vds_cksum_err"))) {
        if (node->children && node->children->content)
            req->vds_cksum_err = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_clone_fail", sizeof("vds_clone_fail"))) {
        if (node->children && node->children->content)
            req->vds_clone_fail = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_no_fmd", sizeof("vds_no_fmd"))) {
        if (node->children && node->children->content)
            req->vds_no_fmd = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_cloned_original", sizeof("vds_cloned_original"))) {
        if (node->children && node->children->content)
            req->vds_cloned_original = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_invalid_vnid", sizeof("vds_invalid_vnid"))) {
        if (node->children && node->children->content)
            req->vds_invalid_vnid = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_frag_err", sizeof("vds_frag_err"))) {
        if (node->children && node->children->content)
            req->vds_frag_err = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_invalid_source", sizeof("vds_invalid_source"))) {
        if (node->children && node->children->content)
            req->vds_invalid_source = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_mcast_df_bit", sizeof("vds_mcast_df_bit"))) {
        if (node->children && node->children->content)
            req->vds_mcast_df_bit = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_arp_no_route", sizeof("vds_arp_no_route"))) {
        if (node->children && node->children->content)
            req->vds_arp_no_route = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_l2_no_route", sizeof("vds_l2_no_route"))) {
        if (node->children && node->children->content)
            req->vds_l2_no_route = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_fragment_queue_fail", sizeof("vds_fragment_queue_fail"))) {
        if (node->children && node->children->content)
            req->vds_fragment_queue_fail = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_vlan_fwd_tx", sizeof("vds_vlan_fwd_tx"))) {
        if (node->children && node->children->content)
            req->vds_vlan_fwd_tx = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_vlan_fwd_enq", sizeof("vds_vlan_fwd_enq"))) {
        if (node->children && node->children->content)
            req->vds_vlan_fwd_enq = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_drop_new_flow", sizeof("vds_drop_new_flow"))) {
        if (node->children && node->children->content)
            req->vds_drop_new_flow = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_flow_evict", sizeof("vds_flow_evict"))) {
        if (node->children && node->children->content)
            req->vds_flow_evict = strtoull(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "vds_trap_original", sizeof("vds_trap_original"))) {
        if (node->children && node->children->content)
            req->vds_trap_original = strtoull(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_qos_map_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_qos_map_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "qmr_rid", sizeof("qmr_rid"))) {
        if (node->children && node->children->content)
            req->qmr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "qmr_id", sizeof("qmr_id"))) {
        if (node->children && node->children->content)
            req->qmr_id = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "qmr_dscp", sizeof("qmr_dscp"))) {
        if (node->children && node->children->content)
            req->qmr_dscp = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->qmr_dscp_size = list_size;
        } else if (!strncmp(node->name, "qmr_dscp_fc_id", sizeof("qmr_dscp_fc_id"))) {
        if (node->children && node->children->content)
            req->qmr_dscp_fc_id = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->qmr_dscp_fc_id_size = list_size;
        } else if (!strncmp(node->name, "qmr_mpls_qos", sizeof("qmr_mpls_qos"))) {
        if (node->children && node->children->content)
            req->qmr_mpls_qos = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->qmr_mpls_qos_size = list_size;
        } else if (!strncmp(node->name, "qmr_mpls_qos_fc_id", sizeof("qmr_mpls_qos_fc_id"))) {
        if (node->children && node->children->content)
            req->qmr_mpls_qos_fc_id = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->qmr_mpls_qos_fc_id_size = list_size;
        } else if (!strncmp(node->name, "qmr_dotonep", sizeof("qmr_dotonep"))) {
        if (node->children && node->children->content)
            req->qmr_dotonep = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->qmr_dotonep_size = list_size;
        } else if (!strncmp(node->name, "qmr_dotonep_fc_id", sizeof("qmr_dotonep_fc_id"))) {
        if (node->children && node->children->content)
            req->qmr_dotonep_fc_id = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->qmr_dotonep_fc_id_size = list_size;
        } else if (!strncmp(node->name, "qmr_marker", sizeof("qmr_marker"))) {
        if (node->children && node->children->content)
            req->qmr_marker = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}

void *
vr_fc_map_req_node(xmlNodePtr node, struct vtest *test)
{
    unsigned int list_size;
    vr_fc_map_req *req;

    req = calloc(sizeof(*req), 1);
    if (!req)
        return NULL;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", sizeof("h_op"))) {
        if (node->children && node->children->content)
            req->h_op = vt_gen_op(node->children->content);
        } else if (!strncmp(node->name, "fmr_rid", sizeof("fmr_rid"))) {
        if (node->children && node->children->content)
            req->fmr_rid = strtoul(node->children->content, NULL, 0);
        } else if (!strncmp(node->name, "fmr_id", sizeof("fmr_id"))) {
        if (node->children && node->children->content)
            req->fmr_id = vt_gen_list(node->children->content, GEN_TYPE_U16, &list_size);
            req->fmr_id_size = list_size;
        } else if (!strncmp(node->name, "fmr_dscp", sizeof("fmr_dscp"))) {
        if (node->children && node->children->content)
            req->fmr_dscp = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->fmr_dscp_size = list_size;
        } else if (!strncmp(node->name, "fmr_mpls_qos", sizeof("fmr_mpls_qos"))) {
        if (node->children && node->children->content)
            req->fmr_mpls_qos = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->fmr_mpls_qos_size = list_size;
        } else if (!strncmp(node->name, "fmr_dotonep", sizeof("fmr_dotonep"))) {
        if (node->children && node->children->content)
            req->fmr_dotonep = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->fmr_dotonep_size = list_size;
        } else if (!strncmp(node->name, "fmr_queue_id", sizeof("fmr_queue_id"))) {
        if (node->children && node->children->content)
            req->fmr_queue_id = vt_gen_list(node->children->content, GEN_TYPE_U8, &list_size);
            req->fmr_queue_id_size = list_size;
        } else if (!strncmp(node->name, "fmr_marker", sizeof("fmr_marker"))) {
        if (node->children && node->children->content)
            req->fmr_marker = strtoul(node->children->content, NULL, 0);
        }
        node = node->next;
    }

    return (void *)req;
}


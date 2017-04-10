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

bool
vr_nexthop_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_nexthop_req *req = (vr_nexthop_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "nhr_type", strlen(node->name))) {
            result = vt_gen_byte_compare(req->nhr_type,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_family", strlen(node->name))) {
            result = vt_gen_byte_compare(req->nhr_family,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_id", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_rid", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_encap_oif_id", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_encap_oif_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_encap_len", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_encap_len,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_encap_family", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_encap_family,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_vrf", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_vrf,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_tun_sip", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_tun_sip,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_tun_dip", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_tun_dip,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_tun_sport", strlen(node->name))) {
            result = vt_gen_short_compare(req->nhr_tun_sport,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_tun_dport", strlen(node->name))) {
            result = vt_gen_short_compare(req->nhr_tun_dport,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_ref_cnt", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_ref_cnt,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_marker", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_marker,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_flags", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_flags,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_encap", strlen(node->name))) {
            result = vt_gen_list_compare(req->nhr_encap,
                    req->nhr_encap_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "nhr_nh_list", strlen(node->name))) {
            result = vt_gen_list_compare(req->nhr_nh_list,
                    req->nhr_nh_list_size, node->children->content, GEN_TYPE_U32);
        } else if (!strncmp(node->name, "nhr_label", strlen(node->name))) {
            result = vt_gen_int_compare(req->nhr_label,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_label_list", strlen(node->name))) {
            result = vt_gen_list_compare(req->nhr_label_list,
                    req->nhr_label_list_size, node->children->content, GEN_TYPE_U32);
        } else if (!strncmp(node->name, "nhr_nh_count", strlen(node->name))) {
            result = vt_gen_short_compare(req->nhr_nh_count,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "nhr_tun_sip6", strlen(node->name))) {
            result = vt_gen_list_compare(req->nhr_tun_sip6,
                    req->nhr_tun_sip6_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "nhr_tun_dip6", strlen(node->name))) {
            result = vt_gen_list_compare(req->nhr_tun_dip6,
                    req->nhr_tun_dip6_size, node->children->content, GEN_TYPE_U8);
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_interface_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_interface_req *req = (vr_interface_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "vifr_core", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_core,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_type", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_type,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_flags", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_flags,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_vrf", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_vrf,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_idx", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_idx,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_rid", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_os_idx", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_os_idx,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_mtu", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_mtu,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_name", strlen(node->name))) {
            result = strcmp(req->vifr_name, node->children->content);
        } else if (!strncmp(node->name, "vifr_ibytes", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_ibytes,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_ipackets", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_ipackets,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_ierrors", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_ierrors,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_obytes", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_obytes,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_opackets", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_opackets,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_oerrors", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_oerrors,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_queue_ipackets", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_queue_ipackets,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_queue_ierrors", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_queue_ierrors,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_queue_ierrors_to_lcore", strlen(node->name))) {
            result = vt_gen_list_compare(req->vifr_queue_ierrors_to_lcore,
                    req->vifr_queue_ierrors_to_lcore_size, node->children->content, GEN_TYPE_U64);
        } else if (!strncmp(node->name, "vifr_queue_opackets", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_queue_opackets,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_queue_oerrors", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_queue_oerrors,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_port_ipackets", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_port_ipackets,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_port_ierrors", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_port_ierrors,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_port_isyscalls", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_port_isyscalls,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_port_inombufs", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_port_inombufs,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_port_opackets", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_port_opackets,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_port_oerrors", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_port_oerrors,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_port_osyscalls", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_port_osyscalls,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_dev_ibytes", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_dev_ibytes,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_dev_ipackets", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_dev_ipackets,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_dev_ierrors", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_dev_ierrors,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_dev_inombufs", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_dev_inombufs,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_dev_obytes", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_dev_obytes,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_dev_opackets", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_dev_opackets,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_dev_oerrors", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vifr_dev_oerrors,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_ref_cnt", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_ref_cnt,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_marker", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_marker,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_mac", strlen(node->name))) {
            result = vt_gen_list_compare(req->vifr_mac,
                    req->vifr_mac_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "vifr_ip", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_ip,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_context", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_context,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_mir_id", strlen(node->name))) {
            result = vt_gen_short_compare(req->vifr_mir_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_speed", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_speed,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_duplex", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_duplex,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_vlan_id", strlen(node->name))) {
            result = vt_gen_short_compare(req->vifr_vlan_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_parent_vif_idx", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_parent_vif_idx,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_nh_id", strlen(node->name))) {
            result = vt_gen_short_compare(req->vifr_nh_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_cross_connect_idx", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_cross_connect_idx,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_src_mac", strlen(node->name))) {
            result = vt_gen_list_compare(req->vifr_src_mac,
                    req->vifr_src_mac_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "vifr_bridge_idx", strlen(node->name))) {
            result = vt_gen_int_compare(req->vifr_bridge_idx,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_ovlan_id", strlen(node->name))) {
            result = vt_gen_short_compare(req->vifr_ovlan_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_transport", strlen(node->name))) {
            result = vt_gen_byte_compare(req->vifr_transport,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vifr_fat_flow_protocol_port", strlen(node->name))) {
            result = vt_gen_list_compare(req->vifr_fat_flow_protocol_port,
                    req->vifr_fat_flow_protocol_port_size, node->children->content, GEN_TYPE_U32);
        } else if (!strncmp(node->name, "vifr_qos_map_index", strlen(node->name))) {
            result = vt_gen_short_compare(req->vifr_qos_map_index,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_vxlan_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_vxlan_req *req = (vr_vxlan_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "vxlanr_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->vxlanr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vxlanr_vnid", strlen(node->name))) {
            result = vt_gen_int_compare(req->vxlanr_vnid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vxlanr_nhid", strlen(node->name))) {
            result = vt_gen_int_compare(req->vxlanr_nhid,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_route_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_route_req *req = (vr_route_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "rtr_vrf_id", strlen(node->name))) {
            result = vt_gen_int_compare(req->rtr_vrf_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "rtr_family", strlen(node->name))) {
            result = vt_gen_int_compare(req->rtr_family,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "rtr_prefix", strlen(node->name))) {
            result = vt_gen_list_compare(req->rtr_prefix,
                    req->rtr_prefix_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "rtr_prefix_len", strlen(node->name))) {
            result = vt_gen_int_compare(req->rtr_prefix_len,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "rtr_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->rtr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "rtr_label_flags", strlen(node->name))) {
            result = vt_gen_short_compare(req->rtr_label_flags,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "rtr_label", strlen(node->name))) {
            result = vt_gen_int_compare(req->rtr_label,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "rtr_nh_id", strlen(node->name))) {
            result = vt_gen_int_compare(req->rtr_nh_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "rtr_marker", strlen(node->name))) {
            result = vt_gen_list_compare(req->rtr_marker,
                    req->rtr_marker_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "rtr_marker_plen", strlen(node->name))) {
            result = vt_gen_int_compare(req->rtr_marker_plen,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "rtr_mac", strlen(node->name))) {
            result = vt_gen_list_compare(req->rtr_mac,
                    req->rtr_mac_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "rtr_replace_plen", strlen(node->name))) {
            result = vt_gen_int_compare(req->rtr_replace_plen,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "rtr_index", strlen(node->name))) {
            result = vt_gen_int_compare(req->rtr_index,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_mpls_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_mpls_req *req = (vr_mpls_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "mr_label", strlen(node->name))) {
            result = vt_gen_int_compare(req->mr_label,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "mr_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->mr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "mr_nhid", strlen(node->name))) {
            result = vt_gen_int_compare(req->mr_nhid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "mr_marker", strlen(node->name))) {
            result = vt_gen_int_compare(req->mr_marker,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_mirror_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_mirror_req *req = (vr_mirror_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "mirr_index", strlen(node->name))) {
            result = vt_gen_short_compare(req->mirr_index,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "mirr_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->mirr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "mirr_nhid", strlen(node->name))) {
            result = vt_gen_int_compare(req->mirr_nhid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "mirr_users", strlen(node->name))) {
            result = vt_gen_int_compare(req->mirr_users,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "mirr_flags", strlen(node->name))) {
            result = vt_gen_int_compare(req->mirr_flags,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "mirr_marker", strlen(node->name))) {
            result = vt_gen_int_compare(req->mirr_marker,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "mirr_vni", strlen(node->name))) {
            result = vt_gen_int_compare(req->mirr_vni,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_flow_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_flow_req *req = (vr_flow_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "fr_op", strlen(node->name))) {
            result = vt_gen_flow_op_compare(req->fr_op, node->content);
        } else if (!strncmp(node->name, "fr_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_index", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_index,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_action", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_action,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_flags", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_flags,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_ftable_size", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_ftable_size,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_ftable_dev", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_ftable_dev,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_rindex", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_rindex,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_family", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_family,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_flow_ip", strlen(node->name))) {
            result = vt_gen_list_compare(req->fr_flow_ip,
                    req->fr_flow_ip_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "fr_flow_sport", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_flow_sport,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_flow_dport", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_flow_dport,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_flow_proto", strlen(node->name))) {
            result = vt_gen_byte_compare(req->fr_flow_proto,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_flow_vrf", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_flow_vrf,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_flow_dvrf", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_flow_dvrf,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_mir_id", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_mir_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_sec_mir_id", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_sec_mir_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_mir_sip", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_mir_sip,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_mir_sport", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_mir_sport,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_pcap_meta_data", strlen(node->name))) {
            result = vt_gen_list_compare(req->fr_pcap_meta_data,
                    req->fr_pcap_meta_data_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "fr_mir_vrf", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_mir_vrf,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_ecmp_nh_index", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_ecmp_nh_index,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_src_nh_index", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_src_nh_index,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_flow_nh_id", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_flow_nh_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_drop_reason", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_drop_reason,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_file_path", strlen(node->name))) {
            result = strcmp(req->fr_file_path, node->children->content);
        } else if (!strncmp(node->name, "fr_processed", strlen(node->name))) {
            result = vt_gen_int64_compare(req->fr_processed,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_created", strlen(node->name))) {
            result = vt_gen_int64_compare(req->fr_created,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_added", strlen(node->name))) {
            result = vt_gen_int64_compare(req->fr_added,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_cpus", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_cpus,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_hold_oflows", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_hold_oflows,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_hold_stat", strlen(node->name))) {
            result = vt_gen_list_compare(req->fr_hold_stat,
                    req->fr_hold_stat_size, node->children->content, GEN_TYPE_U32);
        } else if (!strncmp(node->name, "fr_flow_bytes", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_flow_bytes,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_flow_packets", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_flow_packets,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_flow_stats_oflow", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_flow_stats_oflow,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_oflow_entries", strlen(node->name))) {
            result = vt_gen_int_compare(req->fr_oflow_entries,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_gen_id", strlen(node->name))) {
            result = vt_gen_byte_compare(req->fr_gen_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_qos_id", strlen(node->name))) {
            result = vt_gen_short_compare(req->fr_qos_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fr_ttl", strlen(node->name))) {
            result = vt_gen_byte_compare(req->fr_ttl,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_vrf_assign_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_vrf_assign_req *req = (vr_vrf_assign_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "var_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->var_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "var_vif_index", strlen(node->name))) {
            result = vt_gen_short_compare(req->var_vif_index,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "var_vif_vrf", strlen(node->name))) {
            result = vt_gen_int_compare(req->var_vif_vrf,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "var_vlan_id", strlen(node->name))) {
            result = vt_gen_short_compare(req->var_vlan_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "var_marker", strlen(node->name))) {
            result = vt_gen_short_compare(req->var_marker,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "var_nh_id", strlen(node->name))) {
            result = vt_gen_int_compare(req->var_nh_id,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_vrf_stats_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_vrf_stats_req *req = (vr_vrf_stats_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "vsr_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->vsr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_family", strlen(node->name))) {
            result = vt_gen_short_compare(req->vsr_family,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_type", strlen(node->name))) {
            result = vt_gen_short_compare(req->vsr_type,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_vrf", strlen(node->name))) {
            result = vt_gen_int_compare(req->vsr_vrf,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_discards", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_discards,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_resolves", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_resolves,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_receives", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_receives,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_ecmp_composites", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_ecmp_composites,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_l2_mcast_composites", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_l2_mcast_composites,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_fabric_composites", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_fabric_composites,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_udp_tunnels", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_udp_tunnels,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_udp_mpls_tunnels", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_udp_mpls_tunnels,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_gre_mpls_tunnels", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_gre_mpls_tunnels,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_l2_encaps", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_l2_encaps,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_encaps", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_encaps,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_marker", strlen(node->name))) {
            result = vt_gen_short_compare(req->vsr_marker,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_gros", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_gros,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_diags", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_diags,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_encap_composites", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_encap_composites,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_evpn_composites", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_evpn_composites,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_vrf_translates", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_vrf_translates,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_vxlan_tunnels", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_vxlan_tunnels,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_arp_virtual_proxy", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_arp_virtual_proxy,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_arp_virtual_stitch", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_arp_virtual_stitch,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_arp_virtual_flood", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_arp_virtual_flood,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_arp_physical_stitch", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_arp_physical_stitch,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_arp_tor_proxy", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_arp_tor_proxy,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_arp_physical_flood", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_arp_physical_flood,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_l2_receives", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_l2_receives,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vsr_uuc_floods", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vsr_uuc_floods,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_response_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_response *req = (vr_response *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "resp_code", strlen(node->name))) {
            result = vt_gen_int_compare(req->resp_code,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vrouter_ops_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vrouter_ops *req = (vrouter_ops *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "vo_rid", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_mpls_labels", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_mpls_labels,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_nexthops", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_nexthops,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_bridge_entries", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_bridge_entries,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_oflow_bridge_entries", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_oflow_bridge_entries,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_flow_entries", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_flow_entries,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_oflow_entries", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_oflow_entries,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_interfaces", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_interfaces,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_mirror_entries", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_mirror_entries,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_vrfs", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_vrfs,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_build_info", strlen(node->name))) {
            result = strcmp(req->vo_build_info, node->children->content);
        } else if (!strncmp(node->name, "vo_log_level", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_log_level,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_log_type_enable", strlen(node->name))) {
            result = vt_gen_list_compare(req->vo_log_type_enable,
                    req->vo_log_type_enable_size, node->children->content, GEN_TYPE_U32);
        } else if (!strncmp(node->name, "vo_log_type_disable", strlen(node->name))) {
            result = vt_gen_list_compare(req->vo_log_type_disable,
                    req->vo_log_type_disable_size, node->children->content, GEN_TYPE_U32);
        } else if (!strncmp(node->name, "vo_perfr", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_perfr,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_perfs", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_perfs,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_from_vm_mss_adj", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_from_vm_mss_adj,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_to_vm_mss_adj", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_to_vm_mss_adj,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_perfr1", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_perfr1,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_perfr2", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_perfr2,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_perfr3", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_perfr3,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_perfp", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_perfp,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_perfq1", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_perfq1,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_perfq2", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_perfq2,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_perfq3", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_perfq3,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_udp_coff", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_udp_coff,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_flow_hold_limit", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_flow_hold_limit,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_mudp", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_mudp,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_flow_used_entries", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_flow_used_entries,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_flow_used_oentries", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_flow_used_oentries,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_bridge_used_entries", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_bridge_used_entries,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vo_bridge_used_oentries", strlen(node->name))) {
            result = vt_gen_int_compare(req->vo_bridge_used_oentries,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_mem_stats_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_mem_stats_req *req = (vr_mem_stats_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "vms_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->vms_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_alloced", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_alloced,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_freed", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_freed,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_assembler_table_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_assembler_table_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_bridge_mac_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_bridge_mac_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_btable_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_btable_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_build_info_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_build_info_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_defer_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_defer_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_drop_stats_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_drop_stats_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_drop_stats_req_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_drop_stats_req_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_flow_queue_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_flow_queue_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_flow_req_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_flow_req_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_flow_req_path_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_flow_req_path_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_flow_hold_stat_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_flow_hold_stat_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_flow_link_local_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_flow_link_local_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_flow_metadata_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_flow_metadata_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_flow_table_info_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_flow_table_info_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_fragment_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_fragment_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_fragment_queue_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_fragment_queue_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_fragment_queue_element_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_fragment_queue_element_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_fragment_scanner_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_fragment_scanner_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_hpacket_pool_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_hpacket_pool_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_htable_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_htable_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_interface_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_interface_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_interface_mac_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_interface_mac_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_interface_req_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_interface_req_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_interface_req_mac_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_interface_req_mac_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_interface_req_name_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_interface_req_name_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_interface_stats_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_interface_stats_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_interface_table_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_interface_table_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_interface_vrf_table_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_interface_vrf_table_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_itable_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_itable_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_malloc_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_malloc_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_message_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_message_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_message_response_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_message_response_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_message_dump_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_message_dump_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_mem_stats_req_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_mem_stats_req_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_mirror_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_mirror_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_mirror_table_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_mirror_table_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_mirror_meta_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_mirror_meta_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_mtrie_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_mtrie_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_mtrie_bucket_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_mtrie_bucket_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_mtrie_stats_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_mtrie_stats_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_mtrie_table_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_mtrie_table_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_network_address_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_network_address_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_nexthop_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_nexthop_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_nexthop_component_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_nexthop_component_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_nexthop_req_list_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_nexthop_req_list_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_nexthop_req_encap_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_nexthop_req_encap_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_nexthop_req_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_nexthop_req_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_route_table_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_route_table_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_route_req_mac_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_route_req_mac_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_timer_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_timer_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_usock_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_usock_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_usock_poll_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_usock_poll_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_usock_buf_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_usock_buf_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_usock_iovec_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_usock_iovec_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_vrouter_req_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_vrouter_req_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_interface_fat_flow_config_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_interface_fat_flow_config_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_qos_map_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_qos_map_object,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vms_fc_object", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vms_fc_object,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_drop_stats_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_drop_stats_req *req = (vr_drop_stats_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "vds_core", strlen(node->name))) {
            result = vt_gen_int_compare(req->vds_core,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->vds_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_discard", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_discard,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_pull", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_pull,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_invalid_if", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_invalid_if,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_arp_no_where_to_go", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_arp_no_where_to_go,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_garp_from_vm", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_garp_from_vm,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_invalid_arp", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_invalid_arp,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_trap_no_if", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_trap_no_if,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_nowhere_to_go", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_nowhere_to_go,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flow_queue_limit_exceeded", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flow_queue_limit_exceeded,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flow_no_memory", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flow_no_memory,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flow_invalid_protocol", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flow_invalid_protocol,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flow_nat_no_rflow", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flow_nat_no_rflow,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flow_action_drop", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flow_action_drop,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flow_action_invalid", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flow_action_invalid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flow_unusable", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flow_unusable,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flow_table_full", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flow_table_full,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_interface_tx_discard", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_interface_tx_discard,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_interface_drop", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_interface_drop,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_duplicated", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_duplicated,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_push", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_push,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_ttl_exceeded", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_ttl_exceeded,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_invalid_nh", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_invalid_nh,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_invalid_label", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_invalid_label,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_invalid_protocol", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_invalid_protocol,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_interface_rx_discard", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_interface_rx_discard,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_invalid_mcast_source", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_invalid_mcast_source,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_head_alloc_fail", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_head_alloc_fail,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_head_space_reserve_fail", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_head_space_reserve_fail,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_pcow_fail", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_pcow_fail,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flood", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flood,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_mcast_clone_fail", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_mcast_clone_fail,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_no_memory", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_no_memory,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_rewrite_fail", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_rewrite_fail,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_misc", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_misc,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_invalid_packet", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_invalid_packet,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_cksum_err", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_cksum_err,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_clone_fail", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_clone_fail,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_no_fmd", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_no_fmd,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_cloned_original", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_cloned_original,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_invalid_vnid", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_invalid_vnid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_frag_err", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_frag_err,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_invalid_source", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_invalid_source,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_mcast_df_bit", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_mcast_df_bit,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_arp_no_route", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_arp_no_route,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_l2_no_route", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_l2_no_route,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_fragment_queue_fail", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_fragment_queue_fail,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_vlan_fwd_tx", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_vlan_fwd_tx,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_vlan_fwd_enq", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_vlan_fwd_enq,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_drop_new_flow", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_drop_new_flow,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_flow_evict", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_flow_evict,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "vds_trap_original", strlen(node->name))) {
            result = vt_gen_int64_compare(req->vds_trap_original,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_qos_map_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_qos_map_req *req = (vr_qos_map_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "qmr_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->qmr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "qmr_id", strlen(node->name))) {
            result = vt_gen_short_compare(req->qmr_id,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "qmr_dscp", strlen(node->name))) {
            result = vt_gen_list_compare(req->qmr_dscp,
                    req->qmr_dscp_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "qmr_dscp_fc_id", strlen(node->name))) {
            result = vt_gen_list_compare(req->qmr_dscp_fc_id,
                    req->qmr_dscp_fc_id_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "qmr_mpls_qos", strlen(node->name))) {
            result = vt_gen_list_compare(req->qmr_mpls_qos,
                    req->qmr_mpls_qos_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "qmr_mpls_qos_fc_id", strlen(node->name))) {
            result = vt_gen_list_compare(req->qmr_mpls_qos_fc_id,
                    req->qmr_mpls_qos_fc_id_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "qmr_dotonep", strlen(node->name))) {
            result = vt_gen_list_compare(req->qmr_dotonep,
                    req->qmr_dotonep_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "qmr_dotonep_fc_id", strlen(node->name))) {
            result = vt_gen_list_compare(req->qmr_dotonep_fc_id,
                    req->qmr_dotonep_fc_id_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "qmr_marker", strlen(node->name))) {
            result = vt_gen_short_compare(req->qmr_marker,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}

bool
vr_fc_map_req_expect(xmlNodePtr node, struct vtest *test, void *buf)
{
    bool result = true;
    unsigned int list_size;
    vr_fc_map_req *req = (vr_fc_map_req *)buf;

    node = node->xmlChildrenNode;
    while (node) {
        if (node->type == XML_TEXT_NODE) {
            node = node->next;
            continue;
        }

        if (!strncmp(node->name, "h_op", strlen(node->name))) {
            result = vt_gen_op_compare(req->h_op, node->children->content);
        } else if (!strncmp(node->name, "fmr_rid", strlen(node->name))) {
            result = vt_gen_short_compare(req->fmr_rid,
                            strtoul(node->children->content, NULL, 0));
        } else if (!strncmp(node->name, "fmr_id", strlen(node->name))) {
            result = vt_gen_list_compare(req->fmr_id,
                    req->fmr_id_size, node->children->content, GEN_TYPE_U16);
        } else if (!strncmp(node->name, "fmr_dscp", strlen(node->name))) {
            result = vt_gen_list_compare(req->fmr_dscp,
                    req->fmr_dscp_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "fmr_mpls_qos", strlen(node->name))) {
            result = vt_gen_list_compare(req->fmr_mpls_qos,
                    req->fmr_mpls_qos_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "fmr_dotonep", strlen(node->name))) {
            result = vt_gen_list_compare(req->fmr_dotonep,
                    req->fmr_dotonep_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "fmr_queue_id", strlen(node->name))) {
            result = vt_gen_list_compare(req->fmr_queue_id,
                    req->fmr_queue_id_size, node->children->content, GEN_TYPE_U8);
        } else if (!strncmp(node->name, "fmr_marker", strlen(node->name))) {
            result = vt_gen_short_compare(req->fmr_marker,
                            strtoul(node->children->content, NULL, 0));
        }

        if (!result)
            return result;

        node = node->next;
    }

    return result;
}


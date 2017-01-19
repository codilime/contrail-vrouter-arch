/**
 * Autogenerated by Sandesh Compiler (1.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */
#ifndef VR_TYPES_H
#define VR_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* base includes */
#include "sandesh/library/c/sandesh.h"

/* custom thrift includes */

/* begin types */

enum _sandesh_op {
  SANDESH_OP_ADD = 0,
  SANDESH_OP_GET = 1,
  SANDESH_OP_DELETE = 2,
  SANDESH_OP_DUMP = 3,
  SANDESH_OP_RESPONSE = 4,
  SANDESH_OP_RESET = 5
};
typedef enum _sandesh_op sandesh_op;

enum _flow_op {
  FLOW_OP_FLOW_SET = 0,
  FLOW_OP_FLOW_LIST = 1,
  FLOW_OP_FLOW_TABLE_GET = 2
};
typedef enum _flow_op flow_op;

/* struct sandesh_hdr */
struct _sandesh_hdr
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int32_t h_id;
  u_int8_t __isset_h_id;
};
typedef struct _sandesh_hdr sandesh_hdr;

int32_t sandesh_hdr_write (sandesh_hdr* wsandesh_hdr, ThriftProtocol *protocol, int *error);
int32_t sandesh_hdr_read (sandesh_hdr* rsandesh_hdr, ThriftProtocol *protocol, int *error);
void sandesh_hdr_free(sandesh_hdr* fsandesh_hdr);
/* sandesh vr_nexthop_req */
struct _vr_nexthop_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int8_t nhr_type;
  u_int8_t __isset_nhr_type;
  int8_t nhr_family;
  u_int8_t __isset_nhr_family;
  int32_t nhr_id;
  u_int8_t __isset_nhr_id;
  int32_t nhr_rid;
  u_int8_t __isset_nhr_rid;
  int32_t nhr_encap_oif_id;
  u_int8_t __isset_nhr_encap_oif_id;
  int32_t nhr_encap_len;
  u_int8_t __isset_nhr_encap_len;
  int32_t nhr_encap_family;
  u_int8_t __isset_nhr_encap_family;
  int32_t nhr_vrf;
  u_int8_t __isset_nhr_vrf;
  uint32_t nhr_tun_sip;
  u_int8_t __isset_nhr_tun_sip;
  uint32_t nhr_tun_dip;
  u_int8_t __isset_nhr_tun_dip;
  int16_t nhr_tun_sport;
  u_int8_t __isset_nhr_tun_sport;
  int16_t nhr_tun_dport;
  u_int8_t __isset_nhr_tun_dport;
  int32_t nhr_ref_cnt;
  u_int8_t __isset_nhr_ref_cnt;
  int32_t nhr_marker;
  u_int8_t __isset_nhr_marker;
  uint32_t nhr_flags;
  u_int8_t __isset_nhr_flags;
  int8_t * nhr_encap;
  u_int32_t nhr_encap_size;
  u_int8_t __isset_nhr_encap;
  int32_t * nhr_nh_list;
  u_int32_t nhr_nh_list_size;
  u_int8_t __isset_nhr_nh_list;
  int32_t nhr_label;
  u_int8_t __isset_nhr_label;
  int32_t * nhr_label_list;
  u_int32_t nhr_label_list_size;
  u_int8_t __isset_nhr_label_list;
  int16_t nhr_nh_count;
  u_int8_t __isset_nhr_nh_count;
  int8_t * nhr_tun_sip6;
  u_int32_t nhr_tun_sip6_size;
  u_int8_t __isset_nhr_tun_sip6;
  int8_t * nhr_tun_dip6;
  u_int32_t nhr_tun_dip6_size;
  u_int8_t __isset_nhr_tun_dip6;
};
typedef struct _vr_nexthop_req vr_nexthop_req;

int32_t vr_nexthop_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_nexthop_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_nexthop_req_free(void* fsandesh);
void vr_nexthop_req_process (void *pvr_nexthop_req);
/* sandesh vr_interface_req */
struct _vr_interface_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  uint32_t vifr_core;
  u_int8_t __isset_vifr_core;
  int32_t vifr_type;
  u_int8_t __isset_vifr_type;
  int32_t vifr_flags;
  u_int8_t __isset_vifr_flags;
  int32_t vifr_vrf;
  u_int8_t __isset_vifr_vrf;
  int32_t vifr_idx;
  u_int8_t __isset_vifr_idx;
  int32_t vifr_rid;
  u_int8_t __isset_vifr_rid;
  int32_t vifr_os_idx;
  u_int8_t __isset_vifr_os_idx;
  int32_t vifr_mtu;
  u_int8_t __isset_vifr_mtu;
  char * vifr_name;
  u_int8_t __isset_vifr_name;
  int64_t vifr_ibytes;
  u_int8_t __isset_vifr_ibytes;
  int64_t vifr_ipackets;
  u_int8_t __isset_vifr_ipackets;
  int64_t vifr_ierrors;
  u_int8_t __isset_vifr_ierrors;
  int64_t vifr_obytes;
  u_int8_t __isset_vifr_obytes;
  int64_t vifr_opackets;
  u_int8_t __isset_vifr_opackets;
  int64_t vifr_oerrors;
  u_int8_t __isset_vifr_oerrors;
  int64_t vifr_queue_ipackets;
  u_int8_t __isset_vifr_queue_ipackets;
  int64_t vifr_queue_ierrors;
  u_int8_t __isset_vifr_queue_ierrors;
  int64_t * vifr_queue_ierrors_to_lcore;
  u_int32_t vifr_queue_ierrors_to_lcore_size;
  u_int8_t __isset_vifr_queue_ierrors_to_lcore;
  int64_t vifr_queue_opackets;
  u_int8_t __isset_vifr_queue_opackets;
  int64_t vifr_queue_oerrors;
  u_int8_t __isset_vifr_queue_oerrors;
  int64_t vifr_port_ipackets;
  u_int8_t __isset_vifr_port_ipackets;
  int64_t vifr_port_ierrors;
  u_int8_t __isset_vifr_port_ierrors;
  int64_t vifr_port_isyscalls;
  u_int8_t __isset_vifr_port_isyscalls;
  int64_t vifr_port_inombufs;
  u_int8_t __isset_vifr_port_inombufs;
  int64_t vifr_port_opackets;
  u_int8_t __isset_vifr_port_opackets;
  int64_t vifr_port_oerrors;
  u_int8_t __isset_vifr_port_oerrors;
  int64_t vifr_port_osyscalls;
  u_int8_t __isset_vifr_port_osyscalls;
  int64_t vifr_dev_ibytes;
  u_int8_t __isset_vifr_dev_ibytes;
  int64_t vifr_dev_ipackets;
  u_int8_t __isset_vifr_dev_ipackets;
  int64_t vifr_dev_ierrors;
  u_int8_t __isset_vifr_dev_ierrors;
  int64_t vifr_dev_inombufs;
  u_int8_t __isset_vifr_dev_inombufs;
  int64_t vifr_dev_obytes;
  u_int8_t __isset_vifr_dev_obytes;
  int64_t vifr_dev_opackets;
  u_int8_t __isset_vifr_dev_opackets;
  int64_t vifr_dev_oerrors;
  u_int8_t __isset_vifr_dev_oerrors;
  int32_t vifr_ref_cnt;
  u_int8_t __isset_vifr_ref_cnt;
  int32_t vifr_marker;
  u_int8_t __isset_vifr_marker;
  int8_t * vifr_mac;
  u_int32_t vifr_mac_size;
  u_int8_t __isset_vifr_mac;
  uint32_t vifr_ip;
  u_int8_t __isset_vifr_ip;
  int32_t vifr_context;
  u_int8_t __isset_vifr_context;
  int16_t vifr_mir_id;
  u_int8_t __isset_vifr_mir_id;
  int32_t vifr_speed;
  u_int8_t __isset_vifr_speed;
  int32_t vifr_duplex;
  u_int8_t __isset_vifr_duplex;
  int16_t vifr_vlan_id;
  u_int8_t __isset_vifr_vlan_id;
  int32_t vifr_parent_vif_idx;
  u_int8_t __isset_vifr_parent_vif_idx;
  int16_t vifr_nh_id;
  u_int8_t __isset_vifr_nh_id;
  int32_t vifr_cross_connect_idx;
  u_int8_t __isset_vifr_cross_connect_idx;
  int8_t * vifr_src_mac;
  u_int32_t vifr_src_mac_size;
  u_int8_t __isset_vifr_src_mac;
  int32_t vifr_bridge_idx;
  u_int8_t __isset_vifr_bridge_idx;
  int16_t vifr_ovlan_id;
  u_int8_t __isset_vifr_ovlan_id;
  int8_t vifr_transport;
  u_int8_t __isset_vifr_transport;
  int32_t * vifr_fat_flow_protocol_port;
  u_int32_t vifr_fat_flow_protocol_port_size;
  u_int8_t __isset_vifr_fat_flow_protocol_port;
  int16_t vifr_qos_map_index;
  u_int8_t __isset_vifr_qos_map_index;
};
typedef struct _vr_interface_req vr_interface_req;

int32_t vr_interface_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_interface_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_interface_req_free(void* fsandesh);
void vr_interface_req_process (void *pvr_interface_req);
/* sandesh vr_vxlan_req */
struct _vr_vxlan_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int16_t vxlanr_rid;
  u_int8_t __isset_vxlanr_rid;
  int32_t vxlanr_vnid;
  u_int8_t __isset_vxlanr_vnid;
  int32_t vxlanr_nhid;
  u_int8_t __isset_vxlanr_nhid;
};
typedef struct _vr_vxlan_req vr_vxlan_req;

int32_t vr_vxlan_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_vxlan_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_vxlan_req_free(void* fsandesh);
void vr_vxlan_req_process (void *pvr_vxlan_req);
/* sandesh vr_route_req */
struct _vr_route_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int32_t rtr_vrf_id;
  u_int8_t __isset_rtr_vrf_id;
  int32_t rtr_family;
  u_int8_t __isset_rtr_family;
  int8_t * rtr_prefix;
  u_int32_t rtr_prefix_size;
  u_int8_t __isset_rtr_prefix;
  int32_t rtr_prefix_len;
  u_int8_t __isset_rtr_prefix_len;
  int16_t rtr_rid;
  u_int8_t __isset_rtr_rid;
  int16_t rtr_label_flags;
  u_int8_t __isset_rtr_label_flags;
  int32_t rtr_label;
  u_int8_t __isset_rtr_label;
  int32_t rtr_nh_id;
  u_int8_t __isset_rtr_nh_id;
  int8_t * rtr_marker;
  u_int32_t rtr_marker_size;
  u_int8_t __isset_rtr_marker;
  int32_t rtr_marker_plen;
  u_int8_t __isset_rtr_marker_plen;
  int8_t * rtr_mac;
  u_int32_t rtr_mac_size;
  u_int8_t __isset_rtr_mac;
  int32_t rtr_replace_plen;
  u_int8_t __isset_rtr_replace_plen;
  int32_t rtr_index;
  u_int8_t __isset_rtr_index;
};
typedef struct _vr_route_req vr_route_req;

int32_t vr_route_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_route_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_route_req_free(void* fsandesh);
void vr_route_req_process (void *pvr_route_req);
/* sandesh vr_mpls_req */
struct _vr_mpls_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int32_t mr_label;
  u_int8_t __isset_mr_label;
  int16_t mr_rid;
  u_int8_t __isset_mr_rid;
  int32_t mr_nhid;
  u_int8_t __isset_mr_nhid;
  int32_t mr_marker;
  u_int8_t __isset_mr_marker;
};
typedef struct _vr_mpls_req vr_mpls_req;

int32_t vr_mpls_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_mpls_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_mpls_req_free(void* fsandesh);
void vr_mpls_req_process (void *pvr_mpls_req);
/* sandesh vr_mirror_req */
struct _vr_mirror_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int16_t mirr_index;
  u_int8_t __isset_mirr_index;
  int16_t mirr_rid;
  u_int8_t __isset_mirr_rid;
  int32_t mirr_nhid;
  u_int8_t __isset_mirr_nhid;
  int32_t mirr_users;
  u_int8_t __isset_mirr_users;
  int32_t mirr_flags;
  u_int8_t __isset_mirr_flags;
  int32_t mirr_marker;
  u_int8_t __isset_mirr_marker;
  int32_t mirr_vni;
  u_int8_t __isset_mirr_vni;
};
typedef struct _vr_mirror_req vr_mirror_req;

int32_t vr_mirror_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_mirror_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_mirror_req_free(void* fsandesh);
void vr_mirror_req_process (void *pvr_mirror_req);
/* sandesh vr_flow_req */
struct _vr_flow_req
{ 
  /* public */
  flow_op fr_op;
  u_int8_t __isset_fr_op;
  int16_t fr_rid;
  u_int8_t __isset_fr_rid;
  int32_t fr_index;
  u_int8_t __isset_fr_index;
  int16_t fr_action;
  u_int8_t __isset_fr_action;
  int16_t fr_flags;
  u_int8_t __isset_fr_flags;
  int32_t fr_ftable_size;
  u_int8_t __isset_fr_ftable_size;
  int16_t fr_ftable_dev;
  u_int8_t __isset_fr_ftable_dev;
  int32_t fr_rindex;
  u_int8_t __isset_fr_rindex;
  int32_t fr_family;
  u_int8_t __isset_fr_family;
  int8_t * fr_flow_ip;
  u_int32_t fr_flow_ip_size;
  u_int8_t __isset_fr_flow_ip;
  int16_t fr_flow_sport;
  u_int8_t __isset_fr_flow_sport;
  int16_t fr_flow_dport;
  u_int8_t __isset_fr_flow_dport;
  int8_t fr_flow_proto;
  u_int8_t __isset_fr_flow_proto;
  int16_t fr_flow_vrf;
  u_int8_t __isset_fr_flow_vrf;
  int16_t fr_flow_dvrf;
  u_int8_t __isset_fr_flow_dvrf;
  int16_t fr_mir_id;
  u_int8_t __isset_fr_mir_id;
  int16_t fr_sec_mir_id;
  u_int8_t __isset_fr_sec_mir_id;
  uint32_t fr_mir_sip;
  u_int8_t __isset_fr_mir_sip;
  int16_t fr_mir_sport;
  u_int8_t __isset_fr_mir_sport;
  int8_t * fr_pcap_meta_data;
  u_int32_t fr_pcap_meta_data_size;
  u_int8_t __isset_fr_pcap_meta_data;
  int16_t fr_mir_vrf;
  u_int8_t __isset_fr_mir_vrf;
  int32_t fr_ecmp_nh_index;
  u_int8_t __isset_fr_ecmp_nh_index;
  int32_t fr_src_nh_index;
  u_int8_t __isset_fr_src_nh_index;
  int32_t fr_flow_nh_id;
  u_int8_t __isset_fr_flow_nh_id;
  int16_t fr_drop_reason;
  u_int8_t __isset_fr_drop_reason;
  char * fr_file_path;
  u_int8_t __isset_fr_file_path;
  int64_t fr_processed;
  u_int8_t __isset_fr_processed;
  int64_t fr_created;
  u_int8_t __isset_fr_created;
  int64_t fr_added;
  u_int8_t __isset_fr_added;
  int32_t fr_cpus;
  u_int8_t __isset_fr_cpus;
  int32_t fr_hold_oflows;
  u_int8_t __isset_fr_hold_oflows;
  int32_t * fr_hold_stat;
  u_int32_t fr_hold_stat_size;
  u_int8_t __isset_fr_hold_stat;
  uint32_t fr_flow_bytes;
  u_int8_t __isset_fr_flow_bytes;
  uint32_t fr_flow_packets;
  u_int8_t __isset_fr_flow_packets;
  uint32_t fr_flow_stats_oflow;
  u_int8_t __isset_fr_flow_stats_oflow;
  int32_t fr_oflow_entries;
  u_int8_t __isset_fr_oflow_entries;
  int8_t fr_gen_id;
  u_int8_t __isset_fr_gen_id;
  int16_t fr_qos_id;
  u_int8_t __isset_fr_qos_id;
  int8_t fr_ttl;
  u_int8_t __isset_fr_ttl;
};
typedef struct _vr_flow_req vr_flow_req;

int32_t vr_flow_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_flow_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_flow_req_free(void* fsandesh);
void vr_flow_req_process (void *pvr_flow_req);
/* sandesh vr_vrf_assign_req */
struct _vr_vrf_assign_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int16_t var_rid;
  u_int8_t __isset_var_rid;
  int16_t var_vif_index;
  u_int8_t __isset_var_vif_index;
  int32_t var_vif_vrf;
  u_int8_t __isset_var_vif_vrf;
  int16_t var_vlan_id;
  u_int8_t __isset_var_vlan_id;
  int16_t var_marker;
  u_int8_t __isset_var_marker;
  int32_t var_nh_id;
  u_int8_t __isset_var_nh_id;
};
typedef struct _vr_vrf_assign_req vr_vrf_assign_req;

int32_t vr_vrf_assign_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_vrf_assign_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_vrf_assign_req_free(void* fsandesh);
void vr_vrf_assign_req_process (void *pvr_vrf_assign_req);
/* sandesh vr_vrf_stats_req */
struct _vr_vrf_stats_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int16_t vsr_rid;
  u_int8_t __isset_vsr_rid;
  int16_t vsr_family;
  u_int8_t __isset_vsr_family;
  int16_t vsr_type;
  u_int8_t __isset_vsr_type;
  int32_t vsr_vrf;
  u_int8_t __isset_vsr_vrf;
  int64_t vsr_discards;
  u_int8_t __isset_vsr_discards;
  int64_t vsr_resolves;
  u_int8_t __isset_vsr_resolves;
  int64_t vsr_receives;
  u_int8_t __isset_vsr_receives;
  int64_t vsr_ecmp_composites;
  u_int8_t __isset_vsr_ecmp_composites;
  int64_t vsr_l2_mcast_composites;
  u_int8_t __isset_vsr_l2_mcast_composites;
  int64_t vsr_fabric_composites;
  u_int8_t __isset_vsr_fabric_composites;
  int64_t vsr_udp_tunnels;
  u_int8_t __isset_vsr_udp_tunnels;
  int64_t vsr_udp_mpls_tunnels;
  u_int8_t __isset_vsr_udp_mpls_tunnels;
  int64_t vsr_gre_mpls_tunnels;
  u_int8_t __isset_vsr_gre_mpls_tunnels;
  int64_t vsr_l2_encaps;
  u_int8_t __isset_vsr_l2_encaps;
  int64_t vsr_encaps;
  u_int8_t __isset_vsr_encaps;
  int16_t vsr_marker;
  u_int8_t __isset_vsr_marker;
  int64_t vsr_gros;
  u_int8_t __isset_vsr_gros;
  int64_t vsr_diags;
  u_int8_t __isset_vsr_diags;
  int64_t vsr_encap_composites;
  u_int8_t __isset_vsr_encap_composites;
  int64_t vsr_evpn_composites;
  u_int8_t __isset_vsr_evpn_composites;
  int64_t vsr_vrf_translates;
  u_int8_t __isset_vsr_vrf_translates;
  int64_t vsr_vxlan_tunnels;
  u_int8_t __isset_vsr_vxlan_tunnels;
  int64_t vsr_arp_virtual_proxy;
  u_int8_t __isset_vsr_arp_virtual_proxy;
  int64_t vsr_arp_virtual_stitch;
  u_int8_t __isset_vsr_arp_virtual_stitch;
  int64_t vsr_arp_virtual_flood;
  u_int8_t __isset_vsr_arp_virtual_flood;
  int64_t vsr_arp_physical_stitch;
  u_int8_t __isset_vsr_arp_physical_stitch;
  int64_t vsr_arp_tor_proxy;
  u_int8_t __isset_vsr_arp_tor_proxy;
  int64_t vsr_arp_physical_flood;
  u_int8_t __isset_vsr_arp_physical_flood;
  int64_t vsr_l2_receives;
  u_int8_t __isset_vsr_l2_receives;
  int64_t vsr_uuc_floods;
  u_int8_t __isset_vsr_uuc_floods;
};
typedef struct _vr_vrf_stats_req vr_vrf_stats_req;

int32_t vr_vrf_stats_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_vrf_stats_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_vrf_stats_req_free(void* fsandesh);
void vr_vrf_stats_req_process (void *pvr_vrf_stats_req);
/* sandesh vr_response */
struct _vr_response
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int32_t resp_code;
  u_int8_t __isset_resp_code;
};
typedef struct _vr_response vr_response;

int32_t vr_response_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_response_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_response_free(void* fsandesh);
void vr_response_process (void *pvr_response);
/* sandesh vrouter_ops */
struct _vrouter_ops
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int32_t vo_rid;
  u_int8_t __isset_vo_rid;
  int32_t vo_mpls_labels;
  u_int8_t __isset_vo_mpls_labels;
  int32_t vo_nexthops;
  u_int8_t __isset_vo_nexthops;
  int32_t vo_bridge_entries;
  u_int8_t __isset_vo_bridge_entries;
  int32_t vo_oflow_bridge_entries;
  u_int8_t __isset_vo_oflow_bridge_entries;
  int32_t vo_flow_entries;
  u_int8_t __isset_vo_flow_entries;
  int32_t vo_oflow_entries;
  u_int8_t __isset_vo_oflow_entries;
  int32_t vo_interfaces;
  u_int8_t __isset_vo_interfaces;
  int32_t vo_mirror_entries;
  u_int8_t __isset_vo_mirror_entries;
  int32_t vo_vrfs;
  u_int8_t __isset_vo_vrfs;
  char * vo_build_info;
  u_int8_t __isset_vo_build_info;
  uint32_t vo_log_level;
  u_int8_t __isset_vo_log_level;
  int32_t * vo_log_type_enable;
  u_int32_t vo_log_type_enable_size;
  u_int8_t __isset_vo_log_type_enable;
  int32_t * vo_log_type_disable;
  u_int32_t vo_log_type_disable_size;
  u_int8_t __isset_vo_log_type_disable;
  int32_t vo_perfr;
  u_int8_t __isset_vo_perfr;
  int32_t vo_perfs;
  u_int8_t __isset_vo_perfs;
  int32_t vo_from_vm_mss_adj;
  u_int8_t __isset_vo_from_vm_mss_adj;
  int32_t vo_to_vm_mss_adj;
  u_int8_t __isset_vo_to_vm_mss_adj;
  int32_t vo_perfr1;
  u_int8_t __isset_vo_perfr1;
  int32_t vo_perfr2;
  u_int8_t __isset_vo_perfr2;
  int32_t vo_perfr3;
  u_int8_t __isset_vo_perfr3;
  int32_t vo_perfp;
  u_int8_t __isset_vo_perfp;
  int32_t vo_perfq1;
  u_int8_t __isset_vo_perfq1;
  int32_t vo_perfq2;
  u_int8_t __isset_vo_perfq2;
  int32_t vo_perfq3;
  u_int8_t __isset_vo_perfq3;
  int32_t vo_udp_coff;
  u_int8_t __isset_vo_udp_coff;
  int32_t vo_flow_hold_limit;
  u_int8_t __isset_vo_flow_hold_limit;
  int32_t vo_mudp;
  u_int8_t __isset_vo_mudp;
  uint32_t vo_flow_used_entries;
  u_int8_t __isset_vo_flow_used_entries;
  uint32_t vo_flow_used_oentries;
  u_int8_t __isset_vo_flow_used_oentries;
  uint32_t vo_bridge_used_entries;
  u_int8_t __isset_vo_bridge_used_entries;
  uint32_t vo_bridge_used_oentries;
  u_int8_t __isset_vo_bridge_used_oentries;
};
typedef struct _vrouter_ops vrouter_ops;

int32_t vrouter_ops_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vrouter_ops_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vrouter_ops_free(void* fsandesh);
void vrouter_ops_process (void *pvrouter_ops);
/* sandesh vr_mem_stats_req */
struct _vr_mem_stats_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  int16_t vms_rid;
  u_int8_t __isset_vms_rid;
  int64_t vms_alloced;
  u_int8_t __isset_vms_alloced;
  int64_t vms_freed;
  u_int8_t __isset_vms_freed;
  int64_t vms_assembler_table_object;
  u_int8_t __isset_vms_assembler_table_object;
  int64_t vms_bridge_mac_object;
  u_int8_t __isset_vms_bridge_mac_object;
  int64_t vms_btable_object;
  u_int8_t __isset_vms_btable_object;
  int64_t vms_build_info_object;
  u_int8_t __isset_vms_build_info_object;
  int64_t vms_defer_object;
  u_int8_t __isset_vms_defer_object;
  int64_t vms_drop_stats_object;
  u_int8_t __isset_vms_drop_stats_object;
  int64_t vms_drop_stats_req_object;
  u_int8_t __isset_vms_drop_stats_req_object;
  int64_t vms_flow_queue_object;
  u_int8_t __isset_vms_flow_queue_object;
  int64_t vms_flow_req_object;
  u_int8_t __isset_vms_flow_req_object;
  int64_t vms_flow_req_path_object;
  u_int8_t __isset_vms_flow_req_path_object;
  int64_t vms_flow_hold_stat_object;
  u_int8_t __isset_vms_flow_hold_stat_object;
  int64_t vms_flow_link_local_object;
  u_int8_t __isset_vms_flow_link_local_object;
  int64_t vms_flow_metadata_object;
  u_int8_t __isset_vms_flow_metadata_object;
  int64_t vms_flow_table_info_object;
  u_int8_t __isset_vms_flow_table_info_object;
  int64_t vms_fragment_object;
  u_int8_t __isset_vms_fragment_object;
  int64_t vms_fragment_queue_object;
  u_int8_t __isset_vms_fragment_queue_object;
  int64_t vms_fragment_queue_element_object;
  u_int8_t __isset_vms_fragment_queue_element_object;
  int64_t vms_fragment_scanner_object;
  u_int8_t __isset_vms_fragment_scanner_object;
  int64_t vms_hpacket_pool_object;
  u_int8_t __isset_vms_hpacket_pool_object;
  int64_t vms_htable_object;
  u_int8_t __isset_vms_htable_object;
  int64_t vms_interface_object;
  u_int8_t __isset_vms_interface_object;
  int64_t vms_interface_mac_object;
  u_int8_t __isset_vms_interface_mac_object;
  int64_t vms_interface_req_object;
  u_int8_t __isset_vms_interface_req_object;
  int64_t vms_interface_req_mac_object;
  u_int8_t __isset_vms_interface_req_mac_object;
  int64_t vms_interface_req_name_object;
  u_int8_t __isset_vms_interface_req_name_object;
  int64_t vms_interface_stats_object;
  u_int8_t __isset_vms_interface_stats_object;
  int64_t vms_interface_table_object;
  u_int8_t __isset_vms_interface_table_object;
  int64_t vms_interface_vrf_table_object;
  u_int8_t __isset_vms_interface_vrf_table_object;
  int64_t vms_itable_object;
  u_int8_t __isset_vms_itable_object;
  int64_t vms_malloc_object;
  u_int8_t __isset_vms_malloc_object;
  int64_t vms_message_object;
  u_int8_t __isset_vms_message_object;
  int64_t vms_message_response_object;
  u_int8_t __isset_vms_message_response_object;
  int64_t vms_message_dump_object;
  u_int8_t __isset_vms_message_dump_object;
  int64_t vms_mem_stats_req_object;
  u_int8_t __isset_vms_mem_stats_req_object;
  int64_t vms_mirror_object;
  u_int8_t __isset_vms_mirror_object;
  int64_t vms_mirror_table_object;
  u_int8_t __isset_vms_mirror_table_object;
  int64_t vms_mirror_meta_object;
  u_int8_t __isset_vms_mirror_meta_object;
  int64_t vms_mtrie_object;
  u_int8_t __isset_vms_mtrie_object;
  int64_t vms_mtrie_bucket_object;
  u_int8_t __isset_vms_mtrie_bucket_object;
  int64_t vms_mtrie_stats_object;
  u_int8_t __isset_vms_mtrie_stats_object;
  int64_t vms_mtrie_table_object;
  u_int8_t __isset_vms_mtrie_table_object;
  int64_t vms_network_address_object;
  u_int8_t __isset_vms_network_address_object;
  int64_t vms_nexthop_object;
  u_int8_t __isset_vms_nexthop_object;
  int64_t vms_nexthop_component_object;
  u_int8_t __isset_vms_nexthop_component_object;
  int64_t vms_nexthop_req_list_object;
  u_int8_t __isset_vms_nexthop_req_list_object;
  int64_t vms_nexthop_req_encap_object;
  u_int8_t __isset_vms_nexthop_req_encap_object;
  int64_t vms_nexthop_req_object;
  u_int8_t __isset_vms_nexthop_req_object;
  int64_t vms_route_table_object;
  u_int8_t __isset_vms_route_table_object;
  int64_t vms_route_req_mac_object;
  u_int8_t __isset_vms_route_req_mac_object;
  int64_t vms_timer_object;
  u_int8_t __isset_vms_timer_object;
  int64_t vms_usock_object;
  u_int8_t __isset_vms_usock_object;
  int64_t vms_usock_poll_object;
  u_int8_t __isset_vms_usock_poll_object;
  int64_t vms_usock_buf_object;
  u_int8_t __isset_vms_usock_buf_object;
  int64_t vms_usock_iovec_object;
  u_int8_t __isset_vms_usock_iovec_object;
  int64_t vms_vrouter_req_object;
  u_int8_t __isset_vms_vrouter_req_object;
  int64_t vms_interface_fat_flow_config_object;
  u_int8_t __isset_vms_interface_fat_flow_config_object;
  int64_t vms_qos_map_object;
  u_int8_t __isset_vms_qos_map_object;
  int64_t vms_fc_object;
  u_int8_t __isset_vms_fc_object;
};
typedef struct _vr_mem_stats_req vr_mem_stats_req;

int32_t vr_mem_stats_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_mem_stats_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_mem_stats_req_free(void* fsandesh);
void vr_mem_stats_req_process (void *pvr_mem_stats_req);
/* sandesh vr_drop_stats_req */
struct _vr_drop_stats_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  uint32_t vds_core;
  u_int8_t __isset_vds_core;
  int16_t vds_rid;
  u_int8_t __isset_vds_rid;
  int64_t vds_discard;
  u_int8_t __isset_vds_discard;
  int64_t vds_pull;
  u_int8_t __isset_vds_pull;
  int64_t vds_invalid_if;
  u_int8_t __isset_vds_invalid_if;
  int64_t vds_arp_no_where_to_go;
  u_int8_t __isset_vds_arp_no_where_to_go;
  int64_t vds_garp_from_vm;
  u_int8_t __isset_vds_garp_from_vm;
  int64_t vds_invalid_arp;
  u_int8_t __isset_vds_invalid_arp;
  int64_t vds_trap_no_if;
  u_int8_t __isset_vds_trap_no_if;
  int64_t vds_nowhere_to_go;
  u_int8_t __isset_vds_nowhere_to_go;
  int64_t vds_flow_queue_limit_exceeded;
  u_int8_t __isset_vds_flow_queue_limit_exceeded;
  int64_t vds_flow_no_memory;
  u_int8_t __isset_vds_flow_no_memory;
  int64_t vds_flow_invalid_protocol;
  u_int8_t __isset_vds_flow_invalid_protocol;
  int64_t vds_flow_nat_no_rflow;
  u_int8_t __isset_vds_flow_nat_no_rflow;
  int64_t vds_flow_action_drop;
  u_int8_t __isset_vds_flow_action_drop;
  int64_t vds_flow_action_invalid;
  u_int8_t __isset_vds_flow_action_invalid;
  int64_t vds_flow_unusable;
  u_int8_t __isset_vds_flow_unusable;
  int64_t vds_flow_table_full;
  u_int8_t __isset_vds_flow_table_full;
  int64_t vds_interface_tx_discard;
  u_int8_t __isset_vds_interface_tx_discard;
  int64_t vds_interface_drop;
  u_int8_t __isset_vds_interface_drop;
  int64_t vds_duplicated;
  u_int8_t __isset_vds_duplicated;
  int64_t vds_push;
  u_int8_t __isset_vds_push;
  int64_t vds_ttl_exceeded;
  u_int8_t __isset_vds_ttl_exceeded;
  int64_t vds_invalid_nh;
  u_int8_t __isset_vds_invalid_nh;
  int64_t vds_invalid_label;
  u_int8_t __isset_vds_invalid_label;
  int64_t vds_invalid_protocol;
  u_int8_t __isset_vds_invalid_protocol;
  int64_t vds_interface_rx_discard;
  u_int8_t __isset_vds_interface_rx_discard;
  int64_t vds_invalid_mcast_source;
  u_int8_t __isset_vds_invalid_mcast_source;
  int64_t vds_head_alloc_fail;
  u_int8_t __isset_vds_head_alloc_fail;
  int64_t vds_head_space_reserve_fail;
  u_int8_t __isset_vds_head_space_reserve_fail;
  int64_t vds_pcow_fail;
  u_int8_t __isset_vds_pcow_fail;
  int64_t vds_flood;
  u_int8_t __isset_vds_flood;
  int64_t vds_mcast_clone_fail;
  u_int8_t __isset_vds_mcast_clone_fail;
  int64_t vds_no_memory;
  u_int8_t __isset_vds_no_memory;
  int64_t vds_rewrite_fail;
  u_int8_t __isset_vds_rewrite_fail;
  int64_t vds_misc;
  u_int8_t __isset_vds_misc;
  int64_t vds_invalid_packet;
  u_int8_t __isset_vds_invalid_packet;
  int64_t vds_cksum_err;
  u_int8_t __isset_vds_cksum_err;
  int64_t vds_clone_fail;
  u_int8_t __isset_vds_clone_fail;
  int64_t vds_no_fmd;
  u_int8_t __isset_vds_no_fmd;
  int64_t vds_cloned_original;
  u_int8_t __isset_vds_cloned_original;
  int64_t vds_invalid_vnid;
  u_int8_t __isset_vds_invalid_vnid;
  int64_t vds_frag_err;
  u_int8_t __isset_vds_frag_err;
  int64_t vds_invalid_source;
  u_int8_t __isset_vds_invalid_source;
  int64_t vds_mcast_df_bit;
  u_int8_t __isset_vds_mcast_df_bit;
  int64_t vds_arp_no_route;
  u_int8_t __isset_vds_arp_no_route;
  int64_t vds_l2_no_route;
  u_int8_t __isset_vds_l2_no_route;
  int64_t vds_fragment_queue_fail;
  u_int8_t __isset_vds_fragment_queue_fail;
  int64_t vds_vlan_fwd_tx;
  u_int8_t __isset_vds_vlan_fwd_tx;
  int64_t vds_vlan_fwd_enq;
  u_int8_t __isset_vds_vlan_fwd_enq;
  int64_t vds_drop_new_flow;
  u_int8_t __isset_vds_drop_new_flow;
  int64_t vds_flow_evict;
  u_int8_t __isset_vds_flow_evict;
  int64_t vds_trap_original;
  u_int8_t __isset_vds_trap_original;
};
typedef struct _vr_drop_stats_req vr_drop_stats_req;

int32_t vr_drop_stats_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_drop_stats_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_drop_stats_req_free(void* fsandesh);
void vr_drop_stats_req_process (void *pvr_drop_stats_req);
/* sandesh vr_qos_map_req */
struct _vr_qos_map_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  uint16_t qmr_rid;
  u_int8_t __isset_qmr_rid;
  uint16_t qmr_id;
  u_int8_t __isset_qmr_id;
  int8_t * qmr_dscp;
  u_int32_t qmr_dscp_size;
  u_int8_t __isset_qmr_dscp;
  int8_t * qmr_dscp_fc_id;
  u_int32_t qmr_dscp_fc_id_size;
  u_int8_t __isset_qmr_dscp_fc_id;
  int8_t * qmr_mpls_qos;
  u_int32_t qmr_mpls_qos_size;
  u_int8_t __isset_qmr_mpls_qos;
  int8_t * qmr_mpls_qos_fc_id;
  u_int32_t qmr_mpls_qos_fc_id_size;
  u_int8_t __isset_qmr_mpls_qos_fc_id;
  int8_t * qmr_dotonep;
  u_int32_t qmr_dotonep_size;
  u_int8_t __isset_qmr_dotonep;
  int8_t * qmr_dotonep_fc_id;
  u_int32_t qmr_dotonep_fc_id_size;
  u_int8_t __isset_qmr_dotonep_fc_id;
  int16_t qmr_marker;
  u_int8_t __isset_qmr_marker;
};
typedef struct _vr_qos_map_req vr_qos_map_req;

int32_t vr_qos_map_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_qos_map_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_qos_map_req_free(void* fsandesh);
void vr_qos_map_req_process (void *pvr_qos_map_req);
/* sandesh vr_fc_map_req */
struct _vr_fc_map_req
{ 
  /* public */
  sandesh_op h_op;
  u_int8_t __isset_h_op;
  uint16_t fmr_rid;
  u_int8_t __isset_fmr_rid;
  int16_t * fmr_id;
  u_int32_t fmr_id_size;
  u_int8_t __isset_fmr_id;
  int8_t * fmr_dscp;
  u_int32_t fmr_dscp_size;
  u_int8_t __isset_fmr_dscp;
  int8_t * fmr_mpls_qos;
  u_int32_t fmr_mpls_qos_size;
  u_int8_t __isset_fmr_mpls_qos;
  int8_t * fmr_dotonep;
  u_int32_t fmr_dotonep_size;
  u_int8_t __isset_fmr_dotonep;
  int8_t * fmr_queue_id;
  u_int32_t fmr_queue_id_size;
  u_int8_t __isset_fmr_queue_id;
  int16_t fmr_marker;
  u_int8_t __isset_fmr_marker;
};
typedef struct _vr_fc_map_req vr_fc_map_req;

int32_t vr_fc_map_req_write (void* wsandesh, ThriftProtocol *protocol, int *error);
int32_t vr_fc_map_req_read (void* rsandesh, ThriftProtocol *protocol, int *error);
void vr_fc_map_req_free(void* fsandesh);
void vr_fc_map_req_process (void *pvr_fc_map_req);

sandesh_info_t * vr_find_sandesh_info(const char *sname);
/* constants */

#ifdef __cplusplus
}
#endif

#endif /* VR_TYPES_H */

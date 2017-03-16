#ifndef __VR_WINDOWS_H__
#define __VR_WINDOWS_H__

#include <ndis.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VR_MINIPORT_VPKT_INDEX 0
#define MAX_NIC_NUMBER 1024

#define VR_OID_SOURCE	0x00000001
#define VR_AGENT_SOURCE	0x00000002
#define NLA_HDRLEN      4
#define NLA_DATA(nla)   ((char *)nla + NLA_HDRLEN)
#define NLA_LEN(nla)    (nla->nla_len - NLA_HDRLEN)

#define VR_INIT_ASSOC_OK        0
#define VR_INIT_ASSOC_FAILED    1

struct vr_interface; // Forward declaration

struct vr_packet;

struct vr_nic {
    UCHAR                       mac[NDIS_MAX_PHYS_ADDRESS_LENGTH];
    NDIS_SWITCH_PORT_ID			port_id;
    NDIS_SWITCH_NIC_INDEX		nic_index;
    NDIS_SWITCH_NIC_TYPE		nic_type;
};

struct vr_switch_context {
    struct vr_nic			nics[MAX_NIC_NUMBER];
    UINT32                  num_nics;

    PNDIS_RW_LOCK_EX        lock;
    BOOLEAN					restart;
};

struct vr_assoc {
    int sources;

    struct vr_interface* interface;
    struct vr_assoc* next;

    NDIS_IF_COUNTED_STRING string;
    struct {
        NDIS_SWITCH_PORT_ID port_id;
        NDIS_SWITCH_NIC_INDEX nic_index;
    };
};

NDIS_IF_COUNTED_STRING vr_get_name_from_friendly_name(const NDIS_IF_COUNTED_STRING friendly);

struct nlattr {
    UINT16           nla_len;
    UINT16           nla_type;
};

struct nlmsghdr {
    UINT32           nlmsg_len;      /* Length of message including header */
    UINT16           nlmsg_type;     /* Message content */
    UINT16           nlmsg_flags;    /* Additional flags */
    UINT32           nlmsg_seq;      /* Sequence number */
    UINT32           nlmsg_pid;      /* Sending process port ID */
};

struct genlmsghdr {
    UINT8    cmd;
    UINT8    version;
    UINT16   reserved;
};

struct vr_assoc* vr_get_assoc_name(const NDIS_IF_COUNTED_STRING string);
void vr_set_assoc_oid_name(const NDIS_IF_COUNTED_STRING interface_name, struct vr_interface* interface);
void vr_delete_assoc_name(const NDIS_IF_COUNTED_STRING interface_name);

struct vr_assoc* vr_get_assoc_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index);
void vr_set_assoc_oid_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index, struct vr_interface* interface);
void vr_delete_assoc_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index);

int vr_init_assoc();
void vr_clean_assoc();

void get_random_bytes(void *buf, int nbytes);

struct host_os * vrouter_get_host(void);

NDIS_HANDLE vrouter_generate_pool(void);
void vrouter_free_pool(NDIS_HANDLE pool);
struct vr_packet *win_get_packet(PNET_BUFFER_LIST nbl, struct vr_interface *vif);
int win_pcopy_from_nb(unsigned char *dst, PNET_BUFFER src, unsigned int offset, unsigned int len);
void delete_unbound_nbl(NET_BUFFER_LIST* nbl, unsigned long flags);

extern struct host_os windows_host;

#ifdef __cplusplus
}
#endif

#endif

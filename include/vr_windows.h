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

    union {
        NDIS_IF_COUNTED_STRING string;
        struct {
            NDIS_SWITCH_PORT_ID port_id;
            NDIS_SWITCH_NIC_INDEX nic_index;
        };
    };
};

NDIS_IF_COUNTED_STRING vr_get_name_from_friendly_name(const NDIS_IF_COUNTED_STRING friendly);

struct vr_interface* vr_get_assoc_name(const NDIS_IF_COUNTED_STRING string);
void vr_set_assoc_oid_name(const NDIS_IF_COUNTED_STRING interface_name, struct vr_interface* interface);
void vr_delete_assoc_name(const NDIS_IF_COUNTED_STRING interface_name);

struct vr_interface* vr_get_assoc_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index);
void vr_set_assoc_oid_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index, struct vr_interface* interface);
void vr_delete_assoc_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index);

void vr_clean_assoc();

void get_random_bytes(void *buf, int nbytes);

struct host_os * vrouter_get_host(void);

NDIS_HANDLE vrouter_generate_pool(void);
void vrouter_free_pool(NDIS_HANDLE pool);
struct vr_packet* win_get_packet_from_nbl(PNET_BUFFER_LIST nbl);

extern struct host_os windows_host;

#ifdef __cplusplus
}
#endif

#endif

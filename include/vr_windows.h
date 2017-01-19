#pragma once

#include <ndis.h>


#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NIC_NUMBER 1024

#define VR_OID_SOURCE	0x00000001
#define VR_AGENT_SOURCE	0x00000020

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
	NDIS_IF_COUNTED_STRING string;
	NDIS_SWITCH_PORT_ID port_id;
	struct vr_assoc* next;
};

NDIS_IF_COUNTED_STRING vr_get_name_from_friendly_name(NDIS_IF_COUNTED_STRING friendly);
int vr_hash_nic(NDIS_IF_COUNTED_STRING string);
struct vr_assoc* vr_get_assoc(NDIS_IF_COUNTED_STRING string);
void vr_set_assoc_oid(NDIS_IF_COUNTED_STRING name, NDIS_SWITCH_PORT_ID port_id);

#ifdef __cplusplus
}
#endif

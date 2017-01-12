#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define VR_OID_SOURCE	0x00000001
#define VR_AGENT_SOURCE	0x00000020

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

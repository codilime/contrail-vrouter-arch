#ifndef __VR_WINDOWS_H__
#define __VR_WINDOWS_H__

#include <ndis.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VR_MINIPORT_VPKT_INDEX 0

#define VR_OID_SOURCE	0x00000001
#define VR_AGENT_SOURCE	0x00000002
#define NLA_DATA(nla)   ((char *)nla + NLA_HDRLEN)
#define NLA_LEN(nla)    (nla->nla_len - NLA_HDRLEN)

struct vr_interface; // Forward declaration

struct vr_packet;

typedef struct _vr_switch_context
{
    PNDIS_RW_LOCK_EX        lock;

    /* Following flags are ordered in module initialization order */
    BOOLEAN                 ksync_up;
    BOOLEAN                 pkt0_up;
    BOOLEAN                 device_up;
    BOOLEAN                 memory_up;
    BOOLEAN                 message_up;
    BOOLEAN                 vrouter_up;
} vr_switch_context, *pvr_switch_context;

typedef struct _SWITCH_OBJECT
{
    pvr_switch_context ExtensionContext;

    // Ndis related fields.
    NDIS_HANDLE NdisFilterHandle;
    NDIS_SWITCH_CONTEXT NdisSwitchContext;
    NDIS_SWITCH_OPTIONAL_HANDLERS NdisSwitchHandlers;

    // Switch state.
    volatile BOOLEAN Running;

    // Management fields.
    volatile LONG PendingOidCount;

    // Control Path Management.
    PNDIS_SWITCH_NIC_OID_REQUEST OldNicRequest;

} SWITCH_OBJECT, *PSWITCH_OBJECT;

typedef struct _SX_OID_REQUEST
{
    NDIS_OID_REQUEST NdisOidRequest;
    NDIS_EVENT ReqEvent;
    NDIS_STATUS Status;
    ULONG BytesNeeded;

} SX_OID_REQUEST, *PSX_OID_REQUEST;

NDIS_STATUS VrGetNicArray(PSWITCH_OBJECT SxSwitch, PNDIS_SWITCH_NIC_ARRAY *OutputNicArray);
VOID VrFreeNicArray(PNDIS_SWITCH_NIC_ARRAY NicArray);

extern const ULONG VrAllocationTag;

/* Extracts interface name from provided friendly name and stores it in provided `name` buffer. */
NDIS_STATUS vr_get_name_from_friendly_name(NDIS_IF_COUNTED_STRING friendly, char *name, size_t name_buffer_size);

void get_random_bytes(void *buf, int nbytes);

struct host_os * vrouter_get_host(void);

NDIS_HANDLE vrouter_generate_pool(void);
void vrouter_free_pool(NDIS_HANDLE pool);
void free_nbl(PNET_BUFFER_LIST nbl, ULONG data_allocation_tag);
struct vr_packet* win_get_packet(PNET_BUFFER_LIST nbl, struct vr_interface *vif);
int win_pcopy_from_nb(unsigned char *dst, PNET_BUFFER src, unsigned int offset, unsigned int len);

extern struct host_os windows_host;

extern struct vr_packet *win_allocate_packet(void *buffer, unsigned int size, ULONG allocation_tag);
extern void win_free_packet(struct vr_packet *pkt);

extern void win_if_lock(void);
extern void win_if_unlock(void);

#ifdef __cplusplus
}
#endif

#endif

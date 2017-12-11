/*
 * windows_nbl.h -- definitions used in operations on NBLs
 *
 * Copyright (c) 2017 Juniper Networks, Inc. All rights reserved.
 */
#ifndef __WINDOWS_NBL_H__
#define __WINDOWS_NBL_H__

#include "vr_interface.h"
#include "vr_packet.h"
#include "vr_windows.h"

// VR_NBL_CONTEXT_SIZE is sizeof(struct vr_packet) rounded up to the nearest multiple of MEMORY_ALLOCATION_ALIGNMENT
#define VR_NBL_CONTEXT_SIZE \
    (((sizeof(struct vr_packet) + MEMORY_ALLOCATION_ALIGNMENT - 1) / MEMORY_ALLOCATION_ALIGNMENT) * \
        MEMORY_ALLOCATION_ALIGNMENT)

#define IS_NBL_OWNED(nbl) ((nbl)->NdisPoolHandle == VrNBLPool)
#define IS_NBL_CLONE(nbl) ((nbl)->ParentNetBufferList != NULL)

#define VP_DEFAULT_INITIAL_TTL 64

extern PNET_BUFFER_LIST CreateNetBufferList(unsigned int bytesCount);
extern PNET_BUFFER_LIST CloneNetBufferList(PNET_BUFFER_LIST originalNbl);
extern VOID FreeNetBufferList(PNET_BUFFER_LIST nbl);
extern VOID FreeCreatedNetBufferList(PNET_BUFFER_LIST nbl);
extern VOID FreeClonedNetBufferList(PNET_BUFFER_LIST nbl);

extern struct vr_packet *GetVrPacketFromNetBufferList(PNET_BUFFER_LIST nbl);
extern void win_packet_map_from_mdl(struct vr_packet *pkt, PMDL mdl, ULONG mdl_offset, ULONG data_length);
extern struct vr_packet *AllocateVrPacketForNetBufferList(PNET_BUFFER_LIST nbl, struct vr_interface *vif);
extern struct vr_packet *AllocateVrPacket(void *buffer, unsigned int size);
extern void FreeVrPacket(struct vr_packet *pkt);

#endif /* __WINDOWS_NBL_H__ */

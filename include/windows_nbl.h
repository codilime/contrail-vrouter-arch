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

#define IS_OWNED(nbl) (nbl->NdisPoolHandle == VrNBLPool)
#define IS_CLONE(nbl) (nbl->ParentNetBufferList != NULL)

#define VP_DEFAULT_INITIAL_TTL 64

extern PNET_BUFFER_LIST create_nbl(unsigned int size);
extern PNET_BUFFER_LIST clone_nbl(PNET_BUFFER_LIST original_nbl);
extern VOID free_nbl(PNET_BUFFER_LIST nbl);
extern VOID free_created_nbl(PNET_BUFFER_LIST nbl);
extern VOID free_cloned_nbl(PNET_BUFFER_LIST nbl);

extern struct vr_packet *win_get_packet(PNET_BUFFER_LIST nbl, struct vr_interface *vif);
extern struct vr_packet *win_allocate_packet(void *buffer, unsigned int size);
extern void win_free_packet(struct vr_packet *pkt);

#endif /* __WINDOWS_NBL_H__ */

/*
 * vr_btable.h --
 *
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */
#ifndef __VR_BTABLE_H__
#define __VR_BTABLE_H__

#include "vr_os.h"

#define VB_FLAG_MEMORY_ATTACHED 0x1

#define VR_SINGLE_ALLOC_LIMIT   (4 * 1024 * 1024)

struct vr_btable_partition {
    unsigned int vb_offset;
    unsigned int vb_mem_size;
};

struct vr_btable {
    unsigned int    vb_entries;
    unsigned short  vb_esize;
    unsigned short  vb_partitions;
    unsigned int    vb_flags;
    unsigned int    vb_alloc_limit;
    void            **vb_mem;
    struct vr_btable_partition *vb_table_info;
};

struct vr_htable {
    struct vrouter *ht_router;
    unsigned int ht_hentries;
    unsigned int ht_oentries;
    unsigned int ht_entry_size;
    unsigned int ht_key_size;
    unsigned int ht_bucket_size;
    struct vr_btable *ht_htable;
    struct vr_btable *ht_otable;
    struct vr_btable *ht_dtable;
    get_hentry_key ht_get_key;
    vr_hentry_t *ht_free_oentry_head;
    unsigned int ht_used_oentries;
    unsigned int ht_used_entries;
};

struct vr_btable_partition *vr_btable_get_partition(struct vr_btable *,
        unsigned int);
void *vr_btable_get_address(struct vr_btable *, unsigned int);

void vr_btable_free(struct vr_btable *);
struct vr_btable *vr_btable_alloc(unsigned int, unsigned int);
struct vr_btable *vr_btable_attach(struct iovec *, unsigned int, unsigned short);

static inline unsigned int
vr_btable_entries(struct vr_btable *table)
{
    return table->vb_entries;
}

static inline unsigned int
vr_btable_size(struct vr_btable *table)
{
    return table->vb_entries * table->vb_esize;
}

static inline void *
vr_btable_get(struct vr_btable *table, unsigned int entry)
{
    unsigned int t_index, t_offset;

    if (entry >= table->vb_entries)
        return NULL;

    t_index = (entry * table->vb_esize) / table->vb_alloc_limit;
    t_offset = (entry * table->vb_esize) % table->vb_alloc_limit;
    if (t_index >= table->vb_partitions)
        return NULL;

    return ((char *)table->vb_mem[t_index] + t_offset);
}

#endif /* __VR_BTABLE_H__ */

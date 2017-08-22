/*
 * windows_ksync.h -- definitions used in shared memmory handling on Windows
 *
 * Copyright (c) 2017 Juniper Networks, Inc. All rights reserved.
 */
#ifndef __WINDOWS_MEM_H__
#define __WINDOWS_MEM_H__

#include "vr_windows.h"

extern PMDL mdl_mem;

NDIS_STATUS
memory_init(void);

void
memory_exit(void);

#endif /* __WINDOWS_MEM_H__ */

#pragma once

extern PMDL mdl_mem;
extern void* user_virtual_address;

NDIS_STATUS
memory_init(void);

void
memory_exit(void);

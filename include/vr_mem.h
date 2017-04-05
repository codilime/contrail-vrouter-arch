#ifndef __VR_MEM_H__
#define __VR_MEM_H__

extern PMDL mdl_mem;
extern PVOID user_virtual_address;

NDIS_STATUS
memory_init(void);

void
memory_exit(void);

#endif

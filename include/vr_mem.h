#ifndef __VR_MEM_H__
#define __VR_MEM_H__

#include "vr_windows.h"

extern PMDL mdl_mem;

NDIS_STATUS
memory_init(void);

void
memory_exit(void);

#endif

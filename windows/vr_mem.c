#include "vrouter.h"
#include "vr_flow.h"

#define MEMORY_TAG 'MEM'

extern void *vr_flow_table;
extern void *vr_oflow_table;

PMDL mdl_mem   = NULL;
PVOID user_mem = NULL;

int
memory_init(void)
{
    size_t flow_table_size;

    compute_size_oflow_table();

    flow_table_size = VR_FLOW_TABLE_SIZE + VR_OFLOW_TABLE_SIZE;

    user_mem = ExAllocatePoolWithTag(NonPagedPool, flow_table_size, MEMORY_TAG);

    if (user_mem == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    mdl_mem = IoAllocateMdl(user_mem, flow_table_size, FALSE, FALSE, NULL);

    if (mdl_mem == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MmBuildMdlForNonPagedPool(mdl_mem);

    RtlZeroMemory(user_mem, flow_table_size);

    vr_flow_table = user_mem;
    vr_oflow_table = (char *)user_mem + VR_FLOW_TABLE_SIZE;

    return 0;
}

void
memory_exit(void)
{
    IoFreeMdl(mdl_mem);
    ExFreePoolWithTag(user_mem, MEMORY_TAG);
}

#include "vrouter.h"
#include "vr_flow.h"
#include "windows_mem.h"

#define SHARED_MEMORY_POOL_TAG 'MEMS'

extern void *vr_flow_table;
extern void *vr_oflow_table;

PMDL mdl_mem   = NULL;
PVOID user_mem = NULL;

NDIS_STATUS
memory_init(void)
{
    NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
    size_t flow_table_size;

    compute_size_oflow_table(&vr_oflow_entries, vr_flow_entries);

    flow_table_size = VR_FLOW_TABLE_SIZE + VR_OFLOW_TABLE_SIZE;

    user_mem = ExAllocatePoolWithTag(NonPagedPoolNx, flow_table_size, SHARED_MEMORY_POOL_TAG);
    if (user_mem == NULL) {
        ret = STATUS_INSUFFICIENT_RESOURCES;
        goto mem_clean_up;
    }

    mdl_mem = IoAllocateMdl(user_mem, flow_table_size, FALSE, FALSE, NULL);
    if (mdl_mem == NULL) {
        ret = STATUS_INSUFFICIENT_RESOURCES;
        goto mem_clean_up;
    }

    MmBuildMdlForNonPagedPool(mdl_mem);

    RtlZeroMemory(user_mem, flow_table_size);

    vr_flow_table = user_mem;
    vr_oflow_table = (char *)user_mem + VR_FLOW_TABLE_SIZE;

    return ret;

mem_clean_up:
    memory_exit();
    return ret;
}

void
memory_exit(void)
{
    if (mdl_mem != NULL) {
        IoFreeMdl(mdl_mem);
    }
    mdl_mem = NULL;

    if (user_mem != NULL)
        ExFreePoolWithTag(user_mem, SHARED_MEMORY_POOL_TAG);
    user_mem = NULL;

    vr_flow_table = NULL;
    vr_oflow_table = NULL;
}

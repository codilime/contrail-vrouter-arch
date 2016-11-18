#include "precomp.h"

ULONG  SxExtAllocationTag = 'RVCO';

static void *
win_malloc(unsigned int size, unsigned int object)
{
	void *mem = ExAllocatePoolWithTag(NonPagedPool, size, SxExtAllocationTag); // TODO: Check with paged pool

	vr_malloc_stats(size, object);

	return mem;
}

static void *
win_zalloc(unsigned int size, unsigned int object)
{
	void *mem = ExAllocatePoolWithTag(NonPagedPool, size, SxExtAllocationTag); // TODO: Check with paged pool
	NdisZeroMemory(mem, size);

	vr_malloc_stats(size, object);

	return mem;
}

static void *
win_page_alloc(unsigned int size)
{
	void *mem = ExAllocatePoolWithTag(PagedPool, size, SxExtAllocationTag);

	return mem;
}

static void
win_free(void *mem)
{
	if (mem)
		ExFreePoolWithTag(mem, SxExtAllocationTag);

	return;
}

static void
win_page_free(void *mem)
{
	if (mem)
		ExFreePoolWithTag(mem, SxExtAllocationTag);

	return;
}

static void
win_delete_timer(struct vr_timer *vtimer)
{
	EXT_DELETE_PARAMETERS params;
	ExInitializeDeleteTimerParameters(&params);
	params.DeleteContext = NULL;
	params.DeleteCallback = NULL;
	ExDeleteTimer(vtimer->vt_os_arg, TRUE, FALSE, &params);

	return;
}

void
win_timer_callback(PEX_TIMER Timer, void *Context)
{
	struct vr_timer *ctx = (struct vr_timer*)Context;
	ctx->vt_timer(ctx->vt_vr_arg);
}

static int
win_create_timer(struct vr_timer *vtimer)
{
	vtimer->vt_os_arg = ExAllocateTimer(win_timer_callback, (void *)vtimer, EX_TIMER_HIGH_RESOLUTION);

	ExSetTimer(vtimer->vt_os_arg, vtimer->vt_msecs * 10, vtimer->vt_msecs * 10, NULL);

	return 0;
}

static void
win_get_mono_time(unsigned int *sec, unsigned int *nsec)
{
	LARGE_INTEGER i;
	KeQueryTickCount(&i);

	i.QuadPart *= 100;

	*sec = i.HighPart;
	*nsec = i.LowPart;
}

static unsigned int
win_get_cpu(void)
{
	return KeGetCurrentProcessorNumberEx(NULL);
}

static int
win_schedule_work(unsigned int cpu, void(*fn)(void *), void *arg)
{
	// TODO
	(*fn)(arg);
}

struct host_os windows_host = {
	.hos_printf = DbgPrint,
	.hos_malloc = win_malloc,
	.hos_zalloc = win_zalloc,
	.hos_free = win_free,
	.hos_vtop = lh_vtop,
	.hos_page_alloc = win_page_alloc,
	.hos_page_free = win_page_free,

	.hos_palloc = lh_palloc,
	.hos_pfree = lh_pfree,
	.hos_palloc_head = lh_palloc_head,
	.hos_pexpand_head = lh_pexpand_head,
	.hos_preset = lh_preset,
	.hos_pclone = lh_pclone,
	.hos_pcopy = lh_pcopy,
	.hos_pfrag_len = lh_pfrag_len,
	.hos_phead_len = lh_phead_len,
	.hos_pset_data = lh_pset_data,
	.hos_pgso_size = lh_pgso_size,

	.hos_get_cpu = win_get_cpu,
	.hos_schedule_work = lh_schedule_work,
	.hos_delay_op = lh_delay_op,
	.hos_defer = lh_defer,
	.hos_get_defer_data = lh_get_defer_data,
	.hos_put_defer_data = lh_put_defer_data,
	.hos_get_time = lh_get_time,
	.hos_get_mono_time = win_get_mono_time,
	.hos_create_timer = win_create_timer,
	.hos_delete_timer = win_delete_timer,

	.hos_network_header = lh_network_header,
	.hos_inner_network_header = lh_inner_network_header,
	.hos_data_at_offset = lh_data_at_offset,
	.hos_pheader_pointer = lh_pheader_pointer,
	.hos_pull_inner_headers = lh_pull_inner_headers,
	.hos_pcow = lh_pcow,
	.hos_pull_inner_headers_fast = lh_pull_inner_headers_fast,
	.hos_get_udp_src_port = lh_get_udp_src_port,
	.hos_pkt_from_vm_tcp_mss_adj = lh_pkt_from_vm_tcp_mss_adj,
	.hos_pkt_may_pull = lh_pkt_may_pull,
	.hos_gro_process = lh_gro_process,
	.hos_enqueue_to_assembler = lh_enqueue_to_assembler,
	.hos_set_log_level = lh_set_log_level,
	.hos_set_log_type = lh_set_log_type,
	.hos_get_log_level = lh_get_log_level,
	.hos_get_enabled_log_types = lh_get_enabled_log_types,
	.hos_soft_reset = lh_soft_reset,
};
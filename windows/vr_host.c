#include "precomp.h"

#include <errno.h>
#include "vr_os.h"
#include "vr_packet.h"
#include "vr_windows.h"
#include "vrouter.h"

/* Defined in windows/vrouter_mod.c */
extern PSX_SWITCH_OBJECT SxSwitchObject;
extern NDIS_HANDLE SxNBLPool;
extern PNDIS_RW_LOCK_EX AsyncWorkRWLock;

/* TODO: Change to extern linkage when dp-core/vr_stats.c is ported. */
void
vr_malloc_stats(unsigned int size, unsigned int object)
{
    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(object);
}

void 
vr_free_stats(unsigned int object)
{
    UNREFERENCED_PARAMETER(object);
}

typedef void(*scheduled_work_cb)(void *arg);

struct deferred_work_cb_data {
    vr_defer_cb user_cb;
    struct vrouter * router;
    unsigned char user_data[0];
};

struct scheduled_work_cb_data {
    scheduled_work_cb user_cb;
    void * data;
};

NDIS_IO_WORKITEM_FUNCTION deferred_work_routine;

static PNET_BUFFER_LIST
create_nbl(unsigned int size)
{
    void* ptr = ExAllocatePoolWithTag(NonPagedPool, size, SxExtAllocationTag);

    if (ptr == NULL)
        return NULL;

    MDL* mdl = NdisAllocateMdl(SxSwitchObject->NdisFilterHandle, ptr, size);

    if (mdl == NULL)
    {
        ExFreePoolWithTag(ptr, SxExtAllocationTag);
        return NULL;
    }

    mdl->Next = NULL;
    PNET_BUFFER_LIST nbl = NdisAllocateNetBufferAndNetBufferList(SxNBLPool, 0, 0, mdl, 0, size);

    if (nbl == NULL)
    {
        NdisFreeMdl(mdl); // Don't call ExFreePoolWithTag after creating an MDL
        return NULL;
    }

    nbl->SourceHandle = SxSwitchObject->NdisFilterHandle;

    NDIS_STATUS status = SxSwitchObject->NdisSwitchHandlers.AllocateNetBufferListForwardingContext(SxSwitchObject->NdisSwitchContext, nbl);
    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("Allocate FWD CTX: %u\r\n", status);
        NdisFreeNetBufferList(nbl);
        NdisFreeMdl(mdl);
        return NULL;
    }

    return nbl;
}

static void
delete_nbl(PNET_BUFFER_LIST nbl)
{
    NET_BUFFER* nb = NET_BUFFER_LIST_FIRST_NB(nbl);
    MDL* mdl = NET_BUFFER_FIRST_MDL(nb);
    SxSwitchObject->NdisSwitchHandlers.FreeNetBufferListForwardingContext(SxSwitchObject->NdisSwitchContext, nbl);
    NdisFreeNetBufferList(nbl);
    NdisFreeMdl(mdl);
}

static int
win_printf(const char *format, ...)
{
    int printed;
    va_list args;

    /* Only following version of DbgPrint correctly accepts va_list as an argument */
    _crt_va_start(args, format);
    printed = vDbgPrintEx(DPFLTR_IHVNETWORK_ID, DPFLTR_INFO_LEVEL, format, args);
    _crt_va_end(args);

    return printed;
}

static unsigned int win_get_cpu(void);

static void *
win_malloc(unsigned int size, unsigned int object)
{
    UNREFERENCED_PARAMETER(object);

    void *mem = ExAllocatePoolWithTag(NonPagedPool, size, SxExtAllocationTag); // TODO: Check with paged pool

    //vr_malloc_stats(size, object);

    return mem;
}

static void *
win_zalloc(unsigned int size, unsigned int object)
{
    UNREFERENCED_PARAMETER(object);

    void *mem = ExAllocatePoolWithTag(NonPagedPool, size, SxExtAllocationTag); // TODO: Check with paged pool
    NdisZeroMemory(mem, size);

    //vr_malloc_stats(size, object);

    return mem;
}

static void *
win_page_alloc(unsigned int size)
{
    void *mem = ExAllocatePoolWithTag(PagedPool, size, SxExtAllocationTag);

    return mem;
}

static void
win_free(void *mem, unsigned int object)
{
    UNREFERENCED_PARAMETER(object);

    if (mem) {
        //vr_free_stats(object);
        ExFreePoolWithTag(mem, SxExtAllocationTag);
    }

    return;
}

static uint64_t
win_vtop(void *address)
{
    PHYSICAL_ADDRESS physical_address = MmGetPhysicalAddress(address);
    return physical_address.QuadPart;
}

static void
win_page_free(void *address, unsigned int size)
{
    UNREFERENCED_PARAMETER(size);

    if (address)
        ExFreePoolWithTag(address, SxExtAllocationTag);

    return;
}

void 
win_assoc_packet_nb(PNET_BUFFER_LIST nbl, struct vr_packet* pkt)
{
    nbl->MiniportReserved[VR_MINIPORT_VPKT_INDEX] = pkt;
}

struct vr_packet*
win_get_packet_from_nbl(PNET_BUFFER_LIST nbl)
{
    return nbl->MiniportReserved[VR_MINIPORT_VPKT_INDEX];
}

inline struct vr_packet *
win_get_packet(PNET_BUFFER_LIST nbl, struct vr_interface *vif)
{
    struct vr_packet *pkt = ExAllocatePoolWithTag(NonPagedPool, sizeof(struct vr_packet), SxExtAllocationTag);
    if (pkt == NULL)
        return NULL;
    win_assoc_packet_nb(nbl, pkt);

    pkt->vp_net_buffer_list = nbl;
    pkt->vp_cpu = (unsigned char)vr_get_cpu();

    PNET_BUFFER nb = NET_BUFFER_LIST_FIRST_NB(nbl);
    pkt->vp_head =
        (unsigned char*)MmGetSystemAddressForMdlSafe(nb->CurrentMdl, LowPagePriority | MdlMappingNoExecute) + NET_BUFFER_CURRENT_MDL_OFFSET(nb);
    if (!pkt->vp_head)
        goto drop;

    pkt->vp_tail = pkt->vp_data = 0;

    unsigned short length = (unsigned short) NET_BUFFER_DATA_LENGTH(nb);

    pkt->vp_end = length;

    pkt->vp_len = 0;
    pkt->vp_if = vif;
    pkt->vp_network_h = pkt->vp_inner_network_h = 0;
    pkt->vp_nh = NULL;
    pkt->vp_flags = 0;

    // If a problem arises concerning IP checksums, tinker with:
    // if (skb->ip_summed == CHECKSUM_PARTIAL)
    //	pkt->vp_flags |= VP_FLAG_CSUM_PARTIAL;

    pkt->vp_ttl = 64;
    pkt->vp_type = VP_TYPE_NULL;
    pkt->vp_queue = 0;
    pkt->vp_priority = VP_PRIORITY_INVALID;

    return pkt;

drop:
    vr_pfree(pkt, VP_DROP_INVALID_PACKET);
    return NULL;
}

static struct vr_packet *
win_palloc(unsigned int size)
{
    PNET_BUFFER_LIST nbl = create_nbl(size);

    if (nbl == NULL)
        return NULL;

    return win_get_packet(nbl, NULL);
}

static void
win_pfree(struct vr_packet *pkt, unsigned short reason)
{
    unsigned int cpu;

    struct vrouter *router = NULL;// TODO after vRouter gets ported: vrouter_get(0);
    PNET_BUFFER_LIST nbl = NULL;

    if (pkt != NULL) {
        nbl = pkt->vp_net_buffer_list;
        if (nbl == NULL)
            return;
        cpu = pkt->vp_cpu;
    }
    else {
        cpu = win_get_cpu();
    }

    if (router)
        ((uint64_t *)(router->vr_pdrop_stats[cpu]))[reason]++;

    if (nbl)
    {
        // We are only allowed to delete stuff created by our extension
        if(nbl->SourceHandle == SxSwitchObject->NdisFilterHandle)
            delete_nbl(nbl);
    }

    return;
}

static struct vr_packet *
win_palloc_head(struct vr_packet *pkt, unsigned int size)
{
    if (pkt == NULL)
        return NULL;

    PNET_BUFFER_LIST nbl = pkt->vp_net_buffer_list;
    if (nbl == NULL)
        return NULL;

    PNET_BUFFER_LIST nb_head = create_nbl(size);
    if (nb_head == NULL)
        return NULL;

    struct vr_packet* npkt = win_get_packet(nb_head, pkt->vp_if);
    if (npkt == NULL)
    {
        delete_nbl(nb_head);
        return NULL;
    }

    npkt->vp_ttl = pkt->vp_ttl;
    npkt->vp_flags = pkt->vp_flags;
    npkt->vp_type = pkt->vp_type;

    npkt->vp_network_h += pkt->vp_network_h + npkt->vp_end;
    npkt->vp_inner_network_h += pkt->vp_inner_network_h + npkt->vp_end;

    return npkt;
}

static struct vr_packet *
win_pexpand_head(struct vr_packet *pkt, unsigned int hspace)
{
    PNET_BUFFER_LIST nbl = pkt->vp_net_buffer_list;
    if (nbl == NULL)
        return NULL;

    PNET_BUFFER nb = NET_BUFFER_LIST_FIRST_NB(nbl);
    if (nb == NULL)
        return NULL;

    if (NdisRetreatNetBufferDataStart(nb, hspace, 0, NULL) != NDIS_STATUS_SUCCESS)
        return NULL;

    pkt->vp_head =
        (unsigned char*)MmGetSystemAddressForMdlSafe(nb->CurrentMdl, LowPagePriority | MdlMappingNoExecute) + NET_BUFFER_CURRENT_MDL_OFFSET(nb);
    pkt->vp_data += (unsigned short)hspace;
    pkt->vp_tail += (unsigned short)hspace;
    pkt->vp_end += (unsigned short)hspace;

    pkt->vp_network_h += (unsigned short)hspace;
    pkt->vp_inner_network_h += (unsigned short)hspace;

    return pkt;
}

static void
win_preset(struct vr_packet *pkt)
{
    PNET_BUFFER_LIST nbl = pkt->vp_net_buffer_list;
    if (!nbl)
        return;

    PNET_BUFFER nb = NET_BUFFER_LIST_FIRST_NB(nbl);
    if (!nb)
        return;

    PMDL current_mdl = NET_BUFFER_CURRENT_MDL(nb);
    pkt->vp_head =
        (unsigned char*)MmGetSystemAddressForMdlSafe(current_mdl, LowPagePriority | MdlMappingNoExecute);
    pkt->vp_data = 0;
    pkt->vp_tail = (unsigned short)NET_BUFFER_DATA_LENGTH(nb);
    pkt->vp_len = (unsigned short)NET_BUFFER_DATA_LENGTH(nb);

    return;
}

static struct vr_packet *
win_pclone(struct vr_packet *pkt)
{
    struct vr_packet* npkt = ExAllocatePoolWithTag(NonPagedPool, sizeof(struct vr_packet), SxExtAllocationTag);
    *npkt = *pkt;

    npkt->vp_net_buffer_list = NdisAllocateCloneNetBufferList((PNET_BUFFER_LIST)pkt->vp_net_buffer_list, NULL, NULL, 0);
    npkt->vp_cpu = (unsigned char) win_get_cpu();

    return npkt;
}

static int
win_pcopy(unsigned char *dst, struct vr_packet *p_src,
        unsigned int offset, unsigned int len)
{
    if (!p_src) {
        return -EFAULT;
    }
    PNET_BUFFER_LIST nbl = p_src->vp_net_buffer_list;
    if (!nbl) {
        return -EFAULT;
    }
    PNET_BUFFER nb = NET_BUFFER_LIST_FIRST_NB(nbl);
    if (!nb) {
        return -EFAULT;
    }

    /*  Check if requested data lies inside NET_BUFFER data buffer:
            * data_offset - offset inside MDL list
            * data_length - size of the data stored in MDL list
        Relation between those is presented in https://msdn.microsoft.com/en-us/microsoft-r/ff568728.aspx
     */
    ULONG data_offset = NET_BUFFER_DATA_OFFSET(nb);
    ULONG data_length = NET_BUFFER_DATA_LENGTH(nb);
    ULONG data_size = data_offset + data_length;
    if (data_offset + (ULONG)offset + (ULONG)len > data_size) {
        return -EFAULT;
    }

    /* Check if requested data offset lies in the NET_BUFFER's current MDL. */
    PMDL current_mdl = NET_BUFFER_CURRENT_MDL(nb);
    if (NET_BUFFER_CURRENT_MDL_OFFSET(nb) + offset >= MmGetMdlByteCount(current_mdl)) {
        /* Requested offset lies outside of the first MDL => traverse MDL list until offset is reached */
        offset -= MmGetMdlByteCount(current_mdl) - NET_BUFFER_CURRENT_MDL_OFFSET(nb);
        current_mdl = current_mdl->Next;
        if (!current_mdl) {
            return -EFAULT;
        }
        while (offset >= MmGetMdlByteCount(current_mdl)) {
            offset -= MmGetMdlByteCount(current_mdl);
            current_mdl = current_mdl->Next;
            if (!current_mdl) {
                return -EFAULT;
            }
        }
    } else {
        /* Requested offset lies in the first MDL => add MDL_OFFSET to offset */
        offset += NET_BUFFER_CURRENT_MDL_OFFSET(nb);
    }

    /* Retrieve pointer to the beginning of MDL's data buffer */
    unsigned char *mdl_data =
        (unsigned char *)MmGetSystemAddressForMdlSafe(current_mdl, LowPagePriority | MdlMappingNoExecute);
    if (!mdl_data) {
        return -EFAULT;
    }

    /* Copy data from the first MDL where offset lies */
    ULONG copied_bytes = 0;
    ULONG bytes_left_in_first_mdl = MmGetMdlByteCount(current_mdl) - offset;
    if (bytes_left_in_first_mdl <= len) {
        NdisMoveMemory(dst, mdl_data + offset, bytes_left_in_first_mdl);
        copied_bytes += bytes_left_in_first_mdl;
    } else {
        /* All of the requested data lies in `current_mdl` */
        NdisMoveMemory(dst, mdl_data + offset, len);
        copied_bytes += len;
    }

    /*  Iterate MDL list, starting from where `current_mdl` now points and copy the rest
        of the requested data */
    current_mdl = current_mdl->Next;
    while (current_mdl && copied_bytes < len) {
        /* Get the pointer to the beginning of data represented in current MDL. */
        mdl_data =
            (unsigned char *)MmGetSystemAddressForMdlSafe(current_mdl, LowPagePriority | MdlMappingNoExecute);
        if (!mdl_data) {
            return -EFAULT;
        }

        unsigned int left_to_copy = len - copied_bytes;
        ULONG mdl_size = MmGetMdlByteCount(current_mdl);
        if (left_to_copy >= mdl_size) {
            /* If we need to copy more bytes than is stored in MDL, then copy whole MDL buffer. */
            NdisMoveMemory(dst + copied_bytes, mdl_data, mdl_size);
            copied_bytes += mdl_size;
        } else {
            /* Otherwise copy only the necessary amount. */
            NdisMoveMemory(dst + copied_bytes, mdl_data, left_to_copy);
            copied_bytes += left_to_copy;
        }

        current_mdl = current_mdl->Next;
    }

    if (copied_bytes < len) {
        /*  This case appears when MDL list has ended before all of the requested
            packet data could be copied.
         */
        return -EFAULT;
    }

    return len;
}

static unsigned short
win_pfrag_len(struct vr_packet *pkt)
{
    PNET_BUFFER_LIST nbl = pkt->vp_net_buffer_list;
    if (!nbl)
        return 0;

    PNET_BUFFER nb = NET_BUFFER_LIST_FIRST_NB(nbl);
    if (!nb)
        return 0;

    unsigned short frag_len = (unsigned short)NET_BUFFER_DATA_LENGTH(nb);
    return frag_len;
}

static unsigned short
win_phead_len(struct vr_packet *pkt)
{
    UNREFERENCED_PARAMETER(pkt);

    return 0;
}

static void
win_pset_data(struct vr_packet *pkt, unsigned short offset)
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(offset);
    
    /* On Windows it is a noop, because there is no `sk_buff->data` pointer equivalent in NET_BUFFER. */
}

static unsigned int
win_pgso_size(struct vr_packet *pkt)
{
    UNREFERENCED_PARAMETER(pkt);

    /* TODO: More research on Generic Segmentation Offload mechanism in Windows is needed.
     *       As stated in https://msdn.microsoft.com/en-us/windows/hardware/drivers/network/offloading-the-segmentation-of-large-tcp-packets
     *       NDIS supported offload for TCP/IP packets only. dp-core code which does GSO checks is also 
     *       considering only TCP/IP packets.
     *       However we do not know if there is further work needed in the NDIS driver to perform TCP offload.
     *
     * Returning 0 right now is a valid option, because dp-core code supports gso_size == 0 (i.e.
     * when GSO is not supported by NIC driver).
     */

    return 0;
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
    UNREFERENCED_PARAMETER(Timer);

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
scheduled_work_routine(PVOID work_item_context, NDIS_HANDLE work_item_handle)
{
    struct scheduled_work_cb_data * cb_data = (struct scheduled_work_cb_data *)(work_item_context);
    LOCK_STATE_EX lock_state;

    NdisAcquireRWLockRead(AsyncWorkRWLock, &lock_state, 0);
    cb_data->user_cb(cb_data->data);
    NdisReleaseRWLock(AsyncWorkRWLock, &lock_state);

    if (work_item_handle) {
        NdisFreeIoWorkItem(work_item_handle);
    }
    ExFreePoolWithTag(cb_data, SxExtAllocationTag);

    return;
}

static int
win_schedule_work(unsigned int cpu, void(*fn)(void *), void *arg)
{
    UNREFERENCED_PARAMETER(cpu);

    struct scheduled_work_cb_data * cb_data;
    NDIS_HANDLE work_item;

    cb_data = ExAllocatePoolWithTag(NonPagedPool, sizeof(*cb_data), SxExtAllocationTag);
    if (!cb_data) {
        /* TODO: in Linux it returns -ENOMEM */
        return 1;
    }

    cb_data->user_cb = fn;
    cb_data->data = arg;

    work_item = NdisAllocateIoWorkItem(SxSwitchObject->NdisFilterHandle);
    if (!work_item) {
        DbgPrint("%s: NdisAllocateIoWorkItem failed", __func__);
        scheduled_work_routine((PVOID)(cb_data), NULL);
    }
    else {
        NdisQueueIoWorkItem(work_item, scheduled_work_routine, (PVOID)(cb_data));
    }

    return 0;
}

static void
win_delay_op(void)
{
    /* Linux version uses `synchronize_net()` function from RCU API. It is a write-side function
     * which synchronously waits for any currently executing RCU read-side
     * critical sections to complete.
     * In Windows port RCU API is replaced with RW Locks. To simulate a wait for read-side sections to complete
     * Windows driver can attempt to acquire the RW lock for write operations.
     */
    LOCK_STATE_EX lock_state;

    NdisAcquireRWLockWrite(AsyncWorkRWLock, &lock_state, 0);
    NdisReleaseRWLock(AsyncWorkRWLock, &lock_state);

    return;
}

VOID
deferred_work_routine(PVOID work_item_context, NDIS_HANDLE work_item_handle)
{
    struct deferred_work_cb_data * cb_data = (struct deferred_work_cb_data *)(work_item_context);
    LOCK_STATE_EX lock_state;

    NdisAcquireRWLockWrite(AsyncWorkRWLock, &lock_state, 0);
    cb_data->user_cb(cb_data->router, cb_data->user_data);
    NdisReleaseRWLock(AsyncWorkRWLock, &lock_state);

    if (work_item_handle) {
        NdisFreeIoWorkItem(work_item_handle);
    }
    win_free(cb_data, VR_DEFER_OBJECT);

    return;
}

static void
win_defer(struct vrouter *router, vr_defer_cb user_cb, void *data)
{
    struct deferred_work_cb_data * cb_data;
    NDIS_HANDLE work_item;

    cb_data = CONTAINER_OF(user_data, struct deferred_work_cb_data, data);
    cb_data->user_cb = user_cb;
    cb_data->router = router;

    work_item = NdisAllocateIoWorkItem(SxSwitchObject->NdisFilterHandle);
    if (!work_item) {
        deferred_work_routine((PVOID)(cb_data), NULL);
    } else {
        NdisQueueIoWorkItem(work_item, deferred_work_routine, (PVOID)(cb_data));
    }

    return;
}

static void *
win_get_defer_data(unsigned int len)
{
    struct deferred_work_cb_data * cb_data;

    if (len == 0)
        return NULL;

    cb_data = win_malloc(sizeof(*cb_data) + len, VR_DEFER_OBJECT);
    if (!cb_data) {
        DbgPrint("%s: deferred_work_cb_data struct allocation failed", __func__);
        return NULL;
    }

    return cb_data->user_data;
}

static void
win_put_defer_data(void *data)
{
    struct deferred_work_cb_data * cb_data;

    if (!data)
        return;

    cb_data = CONTAINER_OF(user_data, struct deferred_work_cb_data, data);
    win_free(cb_data, VR_DEFER_OBJECT);

    return;
}

static void
win_get_time(uint64_t *sec, uint64_t *usec)
{
    LARGE_INTEGER current_gmt_time, current_local_time;
    
    NdisGetCurrentSystemTime(&current_gmt_time);
    ExSystemTimeToLocalTime(&current_gmt_time, &current_local_time);

    /*
        Times is returned in 100-nanosecond intervals.
        1 s  = 10^9 ns = 10^7 * 100 ns
        1 us = 10^3 ns = 10 * 100 ns
    */
    *sec = (unsigned long)(current_local_time.QuadPart / (LONGLONG)(1000 * 1000 * 100));
    *usec = (unsigned long)((current_local_time.QuadPart % (LONGLONG)(1000 * 1000 * 100)) / 10);

    return;
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

static void *
win_network_header(struct vr_packet *pkt)
{
    UNREFERENCED_PARAMETER(pkt);

    /* Dummy implementation */
    return NULL;
}

static void *
win_inner_network_header(struct vr_packet *pkt)
{
    UNREFERENCED_PARAMETER(pkt);

    /* Dummy implementation */
    return NULL;
}

static void *
win_data_at_offset(struct vr_packet *pkt, unsigned short off)
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(off);

    /* Dummy implementation */
    return NULL;
}

static void *
win_pheader_pointer(struct vr_packet *pkt, unsigned short hdr_len, void *buf)
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(hdr_len);
    UNREFERENCED_PARAMETER(buf);

    return NULL;
}

static int
win_pull_inner_headers(struct vr_packet *pkt,
    unsigned short ip_proto, unsigned short *reason,
    int (*tunnel_type_cb)(unsigned int, unsigned int, unsigned short *))
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(ip_proto);
    UNREFERENCED_PARAMETER(reason);
    UNREFERENCED_PARAMETER(tunnel_type_cb);

    /* Dummy implementation */
    return 0;
}

static int
win_pcow(struct vr_packet *pkt, unsigned short head_room)
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(head_room);

    /* Dummy implementation */
    return 0;
}

static int
win_pull_inner_headers_fast(struct vr_packet *pkt, unsigned char proto,
    int(*tunnel_type_cb)(unsigned int, unsigned int, unsigned short *),
    int *ret, int *encap_type)
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(proto);
    UNREFERENCED_PARAMETER(tunnel_type_cb);
    UNREFERENCED_PARAMETER(ret);
    UNREFERENCED_PARAMETER(encap_type);

    /* Dummy implementation */
    return 0;
}

static __u16
win_get_udp_src_port(struct vr_packet *pkt, struct vr_forwarding_md *md,
    unsigned short vrf)
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(md);
    UNREFERENCED_PARAMETER(vrf);

    /* Dummy implementation */
    return 0;
}

static int
win_pkt_from_vm_tcp_mss_adj(struct vr_packet *pkt, unsigned short overlay_len)
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(overlay_len);

    /* Dummy implementation */
    return 0;
}

static int
win_pkt_may_pull(struct vr_packet *pkt, unsigned int len)
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(len);

    /* Dummy implementation */
    return 0;
}

static int
win_gro_process(struct vr_packet *pkt, struct vr_interface *vif, bool l2_pkt)
{
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(vif);
    UNREFERENCED_PARAMETER(l2_pkt);

    /* Dummy implementation */
    return 0;
}

static int
win_enqueue_to_assembler(struct vrouter *router, struct vr_packet *pkt,
    struct vr_forwarding_md *fmd)
{
    UNREFERENCED_PARAMETER(router);
    UNREFERENCED_PARAMETER(pkt);
    UNREFERENCED_PARAMETER(fmd);

    /* Dummy implementation */
    return 0;
}

static void
win_set_log_level(unsigned int log_level)
{
    UNREFERENCED_PARAMETER(log_level);
    return;
}

static void
win_set_log_type(unsigned int log_type, int enable)
{
    UNREFERENCED_PARAMETER(log_type);
    UNREFERENCED_PARAMETER(enable);
    return;
}

static unsigned int
win_get_log_level(void)
{
    return 0;
}

static unsigned int *
win_get_enabled_log_types(int *size)
{
    UNREFERENCED_PARAMETER(size);

    size = 0;
    return NULL;
}

static void
win_soft_reset(struct vrouter *router)
{
    /*
        NOTE: Used in dp-code/vrouter.c:vrouter_exit() to perform safe exit.

        TODO: Implement using Windows mechanisms
        Linux code:
            flush_scheduled_work();
            rcu_barrier();
    */
    UNREFERENCED_PARAMETER(router);

    return;
}

struct host_os windows_host = {
    .hos_printf = win_printf,
    .hos_malloc = win_malloc,
    .hos_zalloc = win_zalloc,
    .hos_free = win_free,
    .hos_vtop = win_vtop,
    .hos_page_alloc = win_page_alloc,
    .hos_page_free = win_page_free,

    .hos_palloc = win_palloc,
    .hos_pfree = win_pfree,
    .hos_palloc_head = win_palloc_head,
    .hos_pexpand_head = win_pexpand_head,
    .hos_preset = win_preset,
    .hos_pclone = win_pclone,
    .hos_pcopy = win_pcopy,
    .hos_pfrag_len = win_pfrag_len,
    .hos_phead_len = win_phead_len,
    .hos_pset_data = win_pset_data,
    .hos_pgso_size = win_pgso_size,

    .hos_get_cpu = win_get_cpu,
    .hos_schedule_work = win_schedule_work,
    .hos_delay_op = win_delay_op,
    .hos_defer = win_defer,
    .hos_get_defer_data = win_get_defer_data,
    .hos_put_defer_data = win_put_defer_data,
    .hos_get_time = win_get_time,
    .hos_get_mono_time = win_get_mono_time,
    .hos_create_timer = win_create_timer,
    .hos_delete_timer = win_delete_timer,

    .hos_network_header = win_network_header,
    .hos_inner_network_header = win_inner_network_header,
    .hos_data_at_offset = win_data_at_offset,
    .hos_pheader_pointer = win_pheader_pointer,
    .hos_pull_inner_headers = win_pull_inner_headers,
    .hos_pcow = win_pcow,
    .hos_pull_inner_headers_fast = win_pull_inner_headers_fast,
    .hos_get_udp_src_port = win_get_udp_src_port,
    .hos_pkt_from_vm_tcp_mss_adj = win_pkt_from_vm_tcp_mss_adj,
    .hos_pkt_may_pull = win_pkt_may_pull,
    .hos_gro_process = win_gro_process,
    .hos_enqueue_to_assembler = win_enqueue_to_assembler,
    .hos_set_log_level = win_set_log_level,
    .hos_set_log_type = win_set_log_type,
    .hos_get_log_level = win_get_log_level,
    .hos_get_enabled_log_types = win_get_enabled_log_types,
    .hos_soft_reset = win_soft_reset,
};

struct host_os *
vrouter_get_host(void)
{
    return &windows_host;
}

NDIS_HANDLE
vrouter_generate_pool()
{
    NET_BUFFER_LIST_POOL_PARAMETERS params;
    params.ContextSize = 0;
    params.DataSize = 0;
    params.fAllocateNetBuffer = TRUE;
    params.PoolTag = SxExtAllocationTag;
    params.ProtocolId = NDIS_PROTOCOL_ID_DEFAULT;
    params.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    params.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
    params.Header.Size = NDIS_SIZEOF_NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;

    NDIS_HANDLE pool = NdisAllocateNetBufferListPool(SxSwitchObject->NdisFilterHandle, &params);
    // This is NULL if allocating failed
    return pool;
}

void vrouter_free_pool(NDIS_HANDLE pool)
{
    NdisFreeNetBufferListPool(pool);
}
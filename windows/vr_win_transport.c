#include <precomp.h>

#include "vr_message.h"

static ULONG WIN_TRANSPORT_TAG = 'RTRV';

static char *
win_trans_alloc(unsigned int size)
{
    char *buffer;
    size_t allocation_size;
    
    allocation_size = NLMSG_ALIGN(size) + NETLINK_HEADER_LEN;
    buffer = ExAllocatePoolWithTag(NonPagedPoolNx, allocation_size, WIN_TRANSPORT_TAG);
    if (buffer == NULL)
        return NULL;
    
    return buffer + NETLINK_HEADER_LEN;
}

static void
win_trans_free(char *buf)
{
    ASSERT(buf != NULL);
    ExFreePool(buf - NETLINK_HEADER_LEN);
}

static struct vr_mtransport win_transport = {
	.mtrans_alloc   =   win_trans_alloc,
	.mtrans_free	=   win_trans_free,
};

void
vr_transport_exit(void)
{
    vr_message_transport_unregister(&win_transport);
}

int
vr_transport_init(void)
{
    int ret;

    ret = vr_message_transport_register(&win_transport);
    if (ret) {
        DbgPrint("%s: error on transport register = %d\n", __func__, ret);
        return ret;
    }

    return 0;
}

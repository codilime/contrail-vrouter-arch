/*
 * vr_os.h
 *
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */

#ifndef __VR_OS_H__
#define __VR_OS_H__

#include "vr_common.h"

#if defined(__linux__)
#ifdef __KERNEL__

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/genetlink.h>

#include <asm/checksum.h>
#include <asm/bug.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>

#include <net/tcp.h>
#include <net/netlink.h>
#include <net/genetlink.h>

#define ASSERT(x) BUG_ON(!(x));

#else /* __KERNEL */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>

#define ASSERT(x) assert((x));

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#define true 1
#define false 0

#endif /* __KERNEL__ */
#endif /* __linux__ */
#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include "netlink.h"
#include "genetlink.h"

/*
 * BSD has no family AF_BRIDGE so to avoid to many ifdef in ksync and
 * vrouter code it is defined here in the same way as in LINUX
 */
#define AF_BRIDGE    7

#if defined(_KERNEL)
#define vr_printf(format, arg...)   printf(format, ##arg)
#define ASSERT(x) KASSERT((x), (#x));
#else
#include <stdbool.h>
#include <assert.h>
#define vr_printf(format, arg...)   printf(format, ##arg)
#define ASSERT(x) assert((x));
#endif
#endif /* __FreeBSD__ */
#if defined(_WINDOWS)
#include "windows_atomic.h"
#ifdef _NTKERNEL

#pragma warning(disable : 4018)     // '<': signed/unsigned mismatch
#pragma warning(disable : 4057)     // difference in indirection (pointer to different type but same size, ex. unsigned char* and int8_t*)
#pragma warning(disable : 4100)     // unreferenced formal parameter (used a lot in dp-core)
#pragma warning(disable : 4200)     // nonstandard extension used: zero-sized array in struct/union (it exist in gcc and msvc)
#pragma warning(disable : 4242)     // '=': conversion, possible loss of data
#pragma warning(disable : 4244)     // same as above
#pragma warning(disable : 4267)     // same as above
#pragma warning(disable : 4389)     // '==': signed/unsigned mismatch
#pragma warning(disable : 4706)     // assignment within conditional expression

#endif /* _NTKERNEL */
#endif /* _WINDOWS */

extern int vrouter_dbg;

#endif /* __VR_OS_H__ */

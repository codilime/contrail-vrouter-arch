/*
 * vr_os.h
 *
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */

#ifndef __VR_OS_H__
#define __VR_OS_H__

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

#include <basetsd.h>

#ifdef _NTKERNEL

#include <wdm.h>

typedef BOOLEAN bool;

#define true TRUE
#define false FALSE
#define htons(a) RtlUshortByteSwap(a)
#define ntohs(a) RtlUshortByteSwap(a)

#endif /* _NTKERNEL */

typedef INT8 __s8;
typedef UINT8 __u8;
typedef INT16 __s16;
typedef UINT16 __u16;
typedef INT32 __s32;
typedef UINT32 __u32;

typedef INT8 int8_t;
typedef UINT8 uint8_t;
typedef INT16 int16_t;
typedef UINT16 uint16_t;
typedef INT32 int32_t;
typedef UINT32 uint32_t;
typedef INT64 int64_t;
typedef UINT64 uint64_t;

#define __attribute__packed__open__ __pragma( pack( push, 1 ) )
#define __attribute__packed__close__ __pragma( pack( pop ) )
#define __attribute__format__open__(...) /* do nothing */
#define __attribute__format__close__(...) /* do nothing */

#else

#define __attribute__packed__open__ /* do nothing */
#define __attribute__packed__close__ __attribute__((__packed__))
#define __attribute__format__open__(...) /* do nothing */
#define __attribute__format__close__(...) __attribute__((format(__VA_ARGS__)))

#endif /* _WINDOWS */

extern int vrouter_dbg;

#endif /* __VR_OS_H__ */

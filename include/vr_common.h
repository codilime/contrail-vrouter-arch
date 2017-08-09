#ifndef __VR_COMPILER_H__
#define __VR_COMPILER_H__

#if defined(_WINDOWS)

#include <basetsd.h>
#include <errno.h>

#ifdef __KERNEL__

#include <Wdm.h>
#include "vr_windows.h"

typedef BOOLEAN bool;

#define true TRUE
#define false FALSE

#define htons(a) RtlUshortByteSwap(a)
#define ntohs(a) RtlUshortByteSwap(a)
#define htonl(a) RtlUlongByteSwap(a)
#define ntohl(a) RtlUlongByteSwap(a)

#define __LITTLE_ENDIAN_BITFIELD

#else /* __KERNEL__ */

#include <winsock2.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

#define __LITTLE_ENDIAN 1
#define __BIG_ENDIAN 2
#define __BYTE_ORDER __LITTLE_ENDIAN

#endif

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

#define __attribute__packed__open__ __pragma(pack(push, 1))
#define __attribute__packed__close__ __pragma(pack(pop))
#define __attribute__format__open__(...) /* do nothing */
#define __attribute__format__close__(...) /* do nothing */

#define ENETRESET       117
#define EOPNOTSUPP      130
#define AF_BRIDGE       7

#ifndef WIN_IOVEC
#define WIN_IOVEC
struct iovec {
    void *iov_base;
    SIZE_T iov_len;
};
#endif /* WIN_IOVEC */

#else

#define __attribute__packed__open__ /* do nothing */
#define __attribute__packed__close__ __attribute__((__packed__))
#define __attribute__format__open__(...) /* do nothing */
#define __attribute__format__close__(...) __attribute__((format(__VA_ARGS__)))

#define UNREFERENCED_PARAMETER(a) (a)

#define vr_sync_sub_and_fetch_16u(a, b)                 __sync_sub_and_fetch((uint16_t*)(a), (uint16_t)(b))
#define vr_sync_sub_and_fetch_32u(a, b)                 __sync_sub_and_fetch((uint32_t*)(a), (uint32_t)(b))
#define vr_sync_sub_and_fetch_32s(a, b)                 __sync_sub_and_fetch((int32_t*)(a), (int32_t)(b))
#define vr_sync_sub_and_fetch_64u(a, b)                 __sync_sub_and_fetch((uint64_t*)(a), (uint64_t)(b))
#define vr_sync_sub_and_fetch_64s(a, b)                 __sync_sub_and_fetch((int64_t*)(a), (int64_t)(b))
#define vr_sync_add_and_fetch_16u(a, b)                 __sync_add_and_fetch((uint16_t*)(a), (uint16_t)(b))
#define vr_sync_add_and_fetch_32u(a, b)                 __sync_add_and_fetch((uint32_t*)(a), (uint32_t)(b))
#define vr_sync_fetch_and_add_32u(a, b)                 __sync_fetch_and_add((uint32_t*)(a), (uint32_t)(b))
#define vr_sync_fetch_and_add_64u(a, b)                 __sync_fetch_and_add((uint64_t*)(a), (uint64_t)(b))
#define vr_sync_fetch_and_or_16u(a, b)                  __sync_fetch_and_or((uint16_t*)(a), (uint16_t)(b))
#define vr_sync_and_and_fetch_16u(a, b)                 __sync_and_and_fetch((uint16_t*)(a), (uint16_t)(b))
#define vr_sync_bool_compare_and_swap_8u(a, b, c)       __sync_bool_compare_and_swap((uint8_t*)(a), (uint8_t)(b), (uint8_t)(c))
#define vr_sync_bool_compare_and_swap_16u(a, b, c)      __sync_bool_compare_and_swap((uint16_t*)(a), (uint16_t)(b), (uint16_t)(c))
#define vr_sync_bool_compare_and_swap_32u(a, b, c)      __sync_bool_compare_and_swap((uint32_t*)(a), (uint32_t)(b), (uint32_t)(c))
#define vr_sync_bool_compare_and_swap_p(a, b, c)        __sync_bool_compare_and_swap((void**)(a), (void*)(b), (void*)(c))
#define vr_sync_synchronize                             __sync_synchronize
#define vr_ffs_32(a)                                    __builtin_ffs(a)

#endif /* _WINDOWS */

#endif /* __VR_COMPILER_H__ */

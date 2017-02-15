#ifndef __VR_COMPILER_H__
#define __VR_COMPILER_H__

#if defined(_WINDOWS)

#include <basetsd.h>
#include <errno.h>

#ifdef _NTKERNEL

#include <Wdm.h>
#include "vr_windows.h"

typedef BOOLEAN bool;

#define true TRUE
#define false FALSE

#define htons(a) RtlUshortByteSwap(a)
#define ntohs(a) RtlUshortByteSwap(a)
#define htonl(a) RtlUlongByteSwap(a)
#define ntohl(a) RtlUlongByteSwap(a)

#pragma warning(disable : 4706 4389 4267 4244 4242 4200 4100 4057 4018)

#else

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>

#endif /* _NTKERNEL */

#include "windows_atomic.h"

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

struct iovec {
    void *iov_base;
    SIZE_T iov_len;
};

#else

#define __attribute__packed__open__ /* do nothing */
#define __attribute__packed__close__ __attribute__((__packed__))
#define __attribute__format__open__(...) /* do nothing */
#define __attribute__format__close__(...) __attribute__((format(__VA_ARGS__)))

#define UNREFERENCED_PARAMETER(a) (a)

#define _sync_sub_and_fetch_32u(a, b) __sync_sub_and_fetch((uint32_t*)(a), (uint32_t)(b))
#define _sync_sub_and_fetch_32s(a, b) __sync_sub_and_fetch((int32_t*)(a), (int32_t)(b))
#define _sync_sub_and_fetch_64u(a, b) __sync_sub_and_fetch((uint64_t*)(a), (uint64_t)(b))
#define _sync_sub_and_fetch_64s(a, b) __sync_sub_and_fetch((int64_t*)(a), (int64_t)(b))

#endif /* _WINDOWS */

#endif /* __VR_COMPILER_H__ */

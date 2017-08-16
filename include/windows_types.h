#ifndef __WINDOWS_TYPES_H__
#define __WINDOWS_TYPES_H__

#include <basetsd.h>
#include <errno.h>

#ifdef __KERNEL__

#include <Ndis.h>
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

#include <Winsock2.h>
#include <Windows.h>
#include <Ws2tcpip.h>
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

#endif /* __WINDOWS_TYPES_H__ */

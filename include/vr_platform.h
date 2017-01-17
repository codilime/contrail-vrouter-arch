#pragma once

#ifdef _WINDOWS

#include <wdm.h>
#include <basetsd.h>

typedef INT8 int8_t;
typedef UINT8 uint8_t;
typedef INT16 int16_t;
typedef UINT16 uint16_t;
typedef INT32 int32_t;
typedef UINT32 uint32_t;
typedef INT64 int64_t;
typedef UINT64 uint64_t;

typedef BOOLEAN bool;

#define true TRUE
#define false FALSE

#define __attribute__packed__open__ __pragma( pack(push, 1) )
#define __attribute__packed__close__ __pragma( pack(pop) )
#define __attribute__format__open__(...) /* do nothing */
#define __attribute__format__close__(...) /* do nothing */

#define htons(a) RtlUshortByteSwap(a)
#define ntohs(a) RtlUshortByteSwap(a)

#else
#define __attribute__packed__open__ /* do nothing */
#define __attribute__packed__close__ __attribute__((__packed__))
#define __attribute__format__open__(...) /* do nothing */
#define __attribute__format__close__(...) __attribute__((format(__VA_ARGS__)))
#endif

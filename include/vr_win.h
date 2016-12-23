#pragma once

#ifdef _MSC_VER

#define int8_t INT8
#define uint8_t UINT8
#define int16_t INT16
#define uint16_t UINT16
#define int32_t INT32
#define uint32_t UINT32
#define uint64_t UINT64

#define PACK( ... ) __pragma( pack(push, 1) ) __VA_ARGS__ __pragma( pack(pop) )

#define bool UINT8
#define true 1
#define false 0

#define htons(a) RtlUshortByteSwap(a)

#else
#define PACK( ... ) __VA_ARGS__ __attribute__((__packed__))
#endif

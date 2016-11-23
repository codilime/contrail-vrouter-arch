#pragma once

#ifdef _MSC_VER
#include <windef.h>
#define uint8_t UINT8
#define uint16_t UINT16
#define uint64_t UINT64

#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

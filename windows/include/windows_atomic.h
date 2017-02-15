#pragma once

#include "vr_compiler.h"

#pragma intrinsic(_ReadWriteBarrier)


__forceinline UINT32 _sync_sub_and_fetch_32u(UINT32 *ptr, UINT32 val) {
    return InterlockedAdd((PLONG)ptr, -((LONG)val));
}

__forceinline INT32 _sync_sub_and_fetch_32s(INT32 *ptr, INT32 val) {
    return InterlockedAdd((PLONG)ptr, -((LONG)val));
}

__forceinline UINT64 _sync_sub_and_fetch_64u(UINT64 *ptr, UINT64 val) {
    return InterlockedAdd64((PLONGLONG)ptr, -((LONGLONG)val));
}

__forceinline INT64 _sync_sub_and_fetch_64s(INT64 *ptr, INT64 val) {
    return InterlockedAdd64((PLONGLONG)ptr, -((LONGLONG)val));
}


__forceinline UINT32 _sync_add_and_fetch_32u(UINT32 *ptr, UINT32 val) {
    return InterlockedAdd((PLONG)ptr, (LONG)val);
}


__forceinline UINT32 _sync_fetch_and_add_32u(UINT32 *ptr, UINT32 val) {
    return InterlockedExchangeAdd((PLONG)ptr, (LONG)val);
}

__forceinline UINT64 _sync_fetch_and_add_64u(UINT64 *ptr, UINT64 val) {
    return InterlockedExchangeAdd64((PLONGLONG)ptr, (LONGLONG)val);
}


__forceinline UINT16 _sync_fetch_and_or_16u(UINT16 *ptr, UINT16 val) {
    return InterlockedOr16((PSHORT)ptr, (SHORT)val);
}


__forceinline UINT16 _sync_and_and_fetch_16u(UINT16 *ptr, UINT16 val) {
    return InterlockedAnd16((PSHORT)ptr, (SHORT)val) & ((SHORT)val);
}


__forceinline bool _sync_bool_compare_and_swap_16u(UINT16 *ptr, UINT16 oldval, UINT16 newval) {
    return InterlockedCompareExchange16((PSHORT)ptr, (SHORT)newval, (SHORT)oldval) == (SHORT)oldval;
}

__forceinline bool _sync_bool_compare_and_swap_p(void **ptr, void *oldval, void *newval) {
    return InterlockedCompareExchange64((PLONGLONG)ptr, (LONGLONG)newval, (LONGLONG)oldval) == (LONGLONG)oldval;
}


__forceinline void __sync_synchronize() {
    _ReadWriteBarrier();
    MemoryBarrier();
}

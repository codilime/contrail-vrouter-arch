#pragma once

#include "vr_common.h"
#include <intrin.h>

#pragma intrinsic(_ReadWriteBarrier)
#pragma intrinsic(_InterlockedExchangeAdd16)
#pragma intrinsic(_InterlockedCompareExchange8)
#pragma intrinsic(_BitScanForward)


__forceinline UINT16 vr_sync_sub_and_fetch_16u(UINT16 *ptr, UINT16 val) {
    return _InterlockedExchangeAdd16((PSHORT)ptr, -((SHORT)val)) - val;
}

__forceinline UINT32 vr_sync_sub_and_fetch_32u(UINT32 *ptr, UINT32 val) {
    return InterlockedAdd((PLONG)ptr, -((LONG)val));
}

__forceinline INT32 vr_sync_sub_and_fetch_32s(INT32 *ptr, INT32 val) {
    return InterlockedAdd((PLONG)ptr, -((LONG)val));
}

__forceinline UINT64 vr_sync_sub_and_fetch_64u(UINT64 *ptr, UINT64 val) {
    return InterlockedAdd64((PLONGLONG)ptr, -((LONGLONG)val));
}

__forceinline INT64 vr_sync_sub_and_fetch_64s(INT64 *ptr, INT64 val) {
    return InterlockedAdd64((PLONGLONG)ptr, -((LONGLONG)val));
}


__forceinline UINT16 vr_sync_add_and_fetch_16u(UINT16 *ptr, UINT16 val) {
    return _InterlockedExchangeAdd16((PSHORT)ptr, (SHORT)val) + val;
}

__forceinline UINT32 vr_sync_add_and_fetch_32u(UINT32 *ptr, UINT32 val) {
    return InterlockedAdd((PLONG)ptr, (LONG)val);
}


__forceinline UINT32 vr_sync_fetch_and_add_32u(UINT32 *ptr, UINT32 val) {
    return InterlockedExchangeAdd((PLONG)ptr, (LONG)val);
}

__forceinline UINT64 vr_sync_fetch_and_add_64u(UINT64 *ptr, UINT64 val) {
    return InterlockedExchangeAdd64((PLONGLONG)ptr, (LONGLONG)val);
}


__forceinline UINT16 vr_sync_fetch_and_or_16u(UINT16 *ptr, UINT16 val) {
    return InterlockedOr16((PSHORT)ptr, (SHORT)val);
}


__forceinline UINT16 vr_sync_and_and_fetch_16u(UINT16 *ptr, UINT16 val) {
    return InterlockedAnd16((PSHORT)ptr, (SHORT)val) & ((SHORT)val);
}

__forceinline UINT32 vr_sync_and_and_fetch_32u(UINT32 *ptr, UINT32 val) {
    return InterlockedAnd((PLONG)ptr, (LONG)val) & ((LONG)val);
}


__forceinline bool vr_sync_bool_compare_and_swap_8u(UINT8 *ptr, UINT8 oldval, UINT8 newval) {
    return _InterlockedCompareExchange8((PCHAR)ptr, (CHAR)newval, (CHAR)oldval) == (CHAR)oldval;
}

__forceinline bool vr_sync_bool_compare_and_swap_16u(UINT16 *ptr, UINT16 oldval, UINT16 newval) {
    return InterlockedCompareExchange16((PSHORT)ptr, (SHORT)newval, (SHORT)oldval) == (SHORT)oldval;
}

__forceinline bool vr_sync_bool_compare_and_swap_32u(UINT32 *ptr, UINT32 oldval, UINT32 newval) {
    return InterlockedCompareExchange((PLONG)ptr, (LONG)newval, (LONG)oldval) == (LONG)oldval;
}

__forceinline bool vr_sync_bool_compare_and_swap_p(void **ptr, void *oldval, void *newval) {
    return InterlockedCompareExchangePointer(ptr, newval, oldval) == oldval;
}


__forceinline void vr_sync_synchronize() {
    _ReadWriteBarrier();    // compiler memory barrier (compiler level fence)
    MemoryBarrier();        // cpu memory barrier (hardware level fence)
}


__forceinline int vr_ffs_32(int x) {
    ULONG index, mask = (ULONG)x;
    UCHAR isNonzero = _BitScanForward(&index, mask);
    if (isNonzero)
        return index + 1;
    else
        return 0;
}

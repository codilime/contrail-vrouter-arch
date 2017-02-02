#include "windows_atomic.h"


__forceinline UINT32 _sync_sub_and_fetch_32u(UINT32 *a, UINT32 b) {
    return InterlockedAdd((PLONG)a, -((LONG)b));
}

__forceinline INT32 _sync_sub_and_fetch_32s(INT32 *a, INT32 b) {
    return InterlockedAdd((PLONG)a, -((LONG)b));
}

__forceinline UINT64 _sync_sub_and_fetch_64u(UINT64 *a, UINT64 b) {
    return InterlockedAdd64((PLONGLONG)a, -((LONGLONG)b));
}

__forceinline INT64 _sync_sub_and_fetch_64s(INT64 *a, INT64 b) {
    return InterlockedAdd64((PLONGLONG)a, -((LONGLONG)b));
}


__forceinline UINT32 _sync_add_and_fetch_32u(UINT32 *a, UINT32 b) {
    return InterlockedAdd((PLONG)a, (LONG)b);
}


__forceinline UINT32 _sync_fetch_and_add_32u(UINT32 *a, UINT32 b) {
    return InterlockedExchangeAdd((PLONG)a, (LONG)b);
}

__forceinline UINT64 _sync_fetch_and_add_64u(UINT64 *a, UINT64 b) {
    return InterlockedExchangeAdd64((PLONGLONG)a, (LONGLONG)b);
}


__forceinline UINT16 _sync_fetch_and_or_16u(UINT16 *a, UINT16 b) {
    return InterlockedOr16((PSHORT)a, (SHORT)b);
}

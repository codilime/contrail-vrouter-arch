#pragma once

#include <Ntifs.h>

UINT32 _sync_sub_and_fetch_32u(UINT32 *a, UINT32 b);
INT32 _sync_sub_and_fetch_32s(INT32 *a, INT32 b);
UINT64 _sync_sub_and_fetch_64u(UINT64 *a, UINT64 b);
INT64 _sync_sub_and_fetch_64s(INT64 *a, INT64 b);

UINT32 _sync_add_and_fetch_32u(UINT32 *a, UINT32 b);

UINT32 _sync_fetch_and_add_32u(UINT32 *a, UINT32 b);
UINT64 _sync_fetch_and_add_64u(UINT64 *a, UINT64 b);

UINT16 _sync_fetch_and_or_16u(UINT16 *a, UINT16 b);

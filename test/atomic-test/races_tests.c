#include "test_defines.h"

UINT32 _sync_fetch_and_add_32u_var = 0;
UINT32 _sync_fetch_and_add_32u_expect = ITERATIONS * NR_THREADS;

#define TESTED_FUNCTION     _sync_fetch_and_add_32u
#define TESTED_OFFSET       0
#include "algebraic_races.h"
#undef TESTED_OFFSET
#undef TESTED_FUNCTION


UINT64 _sync_fetch_and_add_64u_var = 0;
UINT64 _sync_fetch_and_add_64u_expect = ITERATIONS * NR_THREADS;

#define TESTED_FUNCTION     _sync_fetch_and_add_64u
#define TESTED_OFFSET       0
#include "algebraic_races.h"
#undef TESTED_OFFSET
#undef TESTED_FUNCTION


UINT32 _sync_add_and_fetch_32u_var = 0;
UINT32 _sync_add_and_fetch_32u_expect = ITERATIONS * NR_THREADS;

#define TESTED_FUNCTION     _sync_add_and_fetch_32u
#define TESTED_OFFSET       1
#include "algebraic_races.h"
#undef TESTED_OFFSET
#undef TESTED_FUNCTION


UINT32 _sync_sub_and_fetch_32u_var = ITERATIONS * NR_THREADS;
UINT32 _sync_sub_and_fetch_32u_expect = 0;

#define TESTED_FUNCTION     _sync_sub_and_fetch_32u
#define TESTED_OFFSET       0
#include "algebraic_races.h"
#undef TESTED_OFFSET
#undef TESTED_FUNCTION


UINT32 _sync_sub_and_fetch_32s_var = ITERATIONS * NR_THREADS;
UINT32 _sync_sub_and_fetch_32s_expect = 0;

#define TESTED_FUNCTION     _sync_sub_and_fetch_32s
#define TESTED_OFFSET       0
#include "algebraic_races.h"
#undef TESTED_OFFSET
#undef TESTED_FUNCTION


UINT64 _sync_sub_and_fetch_64u_var = ITERATIONS * NR_THREADS;
UINT64 _sync_sub_and_fetch_64u_expect = 0;

#define TESTED_FUNCTION     _sync_sub_and_fetch_64u
#define TESTED_OFFSET       0
#include "algebraic_races.h"
#undef TESTED_OFFSET
#undef TESTED_FUNCTION


UINT64 _sync_sub_and_fetch_64s_var = ITERATIONS * NR_THREADS;
UINT64 _sync_sub_and_fetch_64s_expect = 0;

#define TESTED_FUNCTION     _sync_sub_and_fetch_64s
#define TESTED_OFFSET       0
#include "algebraic_races.h"
#undef TESTED_OFFSET
#undef TESTED_FUNCTION

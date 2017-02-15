#ifndef TESTED_SIZE
#error
#endif

#include "test_defines.h"

#ifdef TESTED_FETCH_FIRST
#define TESTED_FUNCTION     EVAL3(_sync_fetch_and_sub_, TESTED_SIZE, s)
#else
#define TESTED_FUNCTION     EVAL3(_sync_sub_and_fetch_, TESTED_SIZE, s)
#endif
#define TESTED_TYPE         EVAL2(INT, TESTED_SIZE)
#define TESTED_MIN          EVAL3(INT, TESTED_SIZE, _MIN)
#define TESTED_MAX          EVAL3(INT, TESTED_SIZE, _MAX)
#define TEST_NAME(S)        EVAL2(TESTED_FUNCTION, S)

#ifdef TESTED_FETCH_FIRST
#define GENERATE_TEST_CASE(A, B, C)     GENERATE_TEST_CASE_FETCH_FIRST(A, B, C)
#else
#define GENERATE_TEST_CASE(A, B, C)     GENERATE_TEST_CASE_FETCH_SECOND(A, B, C)
#endif

// basic tests, checks if operations on negative values work ok
static void TEST_NAME(_basic) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(-5, 2, -7);
}

// from MAXINT to MININT
static void TEST_NAME(_min) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(-1, TESTED_MAX, TESTED_MIN);
}

// from MININT to MAXINT
static void TEST_NAME(_max) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(0, -TESTED_MAX, TESTED_MAX);
}

// negate overflow
static void TEST_NAME(_negateflow) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(-1, TESTED_MIN, TESTED_MAX);
}

#undef GENERATE_TEST_CASE

#undef TESTED_FUNCTION
#undef TESTED_TYPE
#undef TESTED_MIN
#undef TESTED_MAX
#undef TEST_NAME

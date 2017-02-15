#ifndef TESTED_SIZE
#error
#endif

#include "test_defines.h"

#ifdef TESTED_FETCH_FIRST
#define TESTED_FUNCTION     EVAL3(_sync_fetch_and_add_, TESTED_SIZE, u)
#else
#define TESTED_FUNCTION     EVAL3(_sync_add_and_fetch_, TESTED_SIZE, u)
#endif
#define TESTED_TYPE         EVAL2(UINT, TESTED_SIZE)
#define TESTED_MAX          EVAL3(UINT, TESTED_SIZE, _MAX)
#define TEST_NAME(S)        EVAL2(TESTED_FUNCTION, S)

#ifdef TESTED_FETCH_FIRST
#define GENERATE_TEST_CASE(A, B, C)     GENERATE_TEST_CASE_FETCH_FIRST(A, B, C)
#else
#define GENERATE_TEST_CASE(A, B, C)     GENERATE_TEST_CASE_FETCH_SECOND(A, B, C)
#endif

// basic test, makes sure the operations are in correct order (operation and return or the other way)
static void TEST_NAME(_basic) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(5, 2, 7);
}

// tests if casting inside tested function doesn't break on maximum output
static void TEST_NAME(_max) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(TESTED_MAX - 1, 1, TESTED_MAX);
}

// unsigned overflow
static void TEST_NAME(_negative) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(TESTED_MAX, 1, 0);
}

// theoretical signed overflow (which is UB)
static void TEST_NAME(_overflow) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(TESTED_MAX / 2, 1, TESTED_MAX / 2 + 1)
}


#undef GENERATE_TEST_CASE

#undef TESTED_FUNCTION
#undef TESTED_TYPE
#undef TESTED_MAX
#undef TEST_NAME

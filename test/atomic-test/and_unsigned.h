#ifndef TESTED_SIZE
#error
#endif

#include "test_defines.h"

#ifdef TESTED_FETCH_FIRST
#define TESTED_FUNCTION     EVAL3(_sync_fetch_and_and_, TESTED_SIZE, u)
#else
#define TESTED_FUNCTION     EVAL3(_sync_and_and_fetch_, TESTED_SIZE, u)
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
    GENERATE_TEST_CASE(0b1010, 0b1100, 0b1000);
}

// tests if casting inside tested function doesn't break on maximum input and minimum output
static void TEST_NAME(_max_input) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(TESTED_MAX, 0, 0);
}

// tests if casting inside tested function doesn't break on maximum input and output
static void TEST_NAME(_max_output) (void **state) {
    UNREFERENCED_PARAMETER(state);
    GENERATE_TEST_CASE(TESTED_MAX, TESTED_MAX, TESTED_MAX);
}


#undef GENERATE_TEST_CASE

#undef TESTED_FUNCTION
#undef TESTED_TYPE
#undef TESTED_MAX
#undef TEST_NAME

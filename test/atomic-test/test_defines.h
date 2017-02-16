#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "vr_common.h"
#include "windows_atomic.h"

#define ITERATIONS 1000000U
#define NR_THREADS 16

#define __EVAL2(A, B)       A##B
#define EVAL2(A, B)         __EVAL2(A, B)
#define __EVAL3(A, B, C)    A##B##C
#define EVAL3(A, B, C)      __EVAL3(A, B, C)

#define GENERATE_TEST_CASE_FETCH_AND_X(A, B, C)     TESTING_TYPE a = (TESTING_TYPE)(A), b = (TESTING_TYPE)(B);  \
                                                    TESTING_TYPE old_a = a;                                     \
                                                    TESTING_TYPE c = TESTING_FUNCTION(&a, b);                   \
                                                    assert_true(c == old_a);                                    \
                                                    assert_true(a == (TESTING_TYPE)(C));

#define GENERATE_TEST_CASE_X_AND_FETCH(A, B, C)    TESTING_TYPE a = (TESTING_TYPE)(A), b = (TESTING_TYPE)(B);   \
                                                    TESTING_TYPE c = TESTING_FUNCTION(&a, b);                   \
                                                    assert_true(c == a);                                        \
                                                    assert_true(a == (TESTING_TYPE)(C));

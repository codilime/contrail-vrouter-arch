#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "vr_compiler.h"
#include "windows_atomic.h"

#define ITERATIONS 1000000U
#define NR_THREADS 16

#define __EVAL2(A, B)       A##B
#define EVAL2(A, B)         __EVAL2(A, B)
#define __EVAL3(A, B, C)    A##B##C
#define EVAL3(A, B, C)      __EVAL3(A, B, C)

#define GENERATE_TEST_CASE_FETCH_FIRST(A, B, C)     TESTED_TYPE a = (TESTED_TYPE)(A), b = (TESTED_TYPE)(B);     \
                                                    TESTED_TYPE old_a = a;                                      \
                                                    TESTED_TYPE c = TESTED_FUNCTION(&a, b);                     \
                                                    assert_true(c == old_a);                                    \
                                                    assert_true(a == (TESTED_TYPE)(C));

#define GENERATE_TEST_CASE_FETCH_SECOND(A, B, C)    TESTED_TYPE a = (TESTED_TYPE)(A), b = (TESTED_TYPE)(B);     \
                                                    TESTED_TYPE c = TESTED_FUNCTION(&a, b);                     \
                                                    assert_true(c == a);                                        \
                                                    assert_true(a == (TESTED_TYPE)(C));

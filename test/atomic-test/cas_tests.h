#pragma once

#include "test_defines.h"
#include <stdint.h>
static void _sync_bool_compare_and_swap_16u_swap(void **state) {
    UNREFERENCED_PARAMETER(state);
    UINT16 a = 5, b = 5, c = 7;
    bool ret = _sync_bool_compare_and_swap_16u(&a, b, c);
    assert_true(ret);
    assert_true(a == c);
}

static void _sync_bool_compare_and_swap_16u_noswap(void **state) {
    UNREFERENCED_PARAMETER(state);
    UINT16 a = 5, b = 9, c = 7;
    UINT16 old_a = a;
    bool ret = _sync_bool_compare_and_swap_16u(&a, b, c);
    assert_false(ret);
    assert_true(a == old_a);
}

static void _sync_bool_compare_and_swap_16u_comp_max(void **state) {
    UNREFERENCED_PARAMETER(state);
    UINT16 a = UINT16_MAX, b = UINT16_MAX, c = 0;
    bool ret = _sync_bool_compare_and_swap_16u(&a, b, c);
    assert_true(ret);
    assert_true(a == c);
}

static void _sync_bool_compare_and_swap_16u_assign_max(void **state) {
    UNREFERENCED_PARAMETER(state);
    UINT16 a = 0, b = 0, c = UINT16_MAX;
    bool ret = _sync_bool_compare_and_swap_16u(&a, b, c);
    assert_true(ret);
    assert_true(a == c);
}


static void _sync_bool_compare_and_swap_p_swap(void **state) {
    UNREFERENCED_PARAMETER(state);
    PVOID a = (PVOID)5, b = (PVOID)5, c = (PVOID)7;
    bool ret = _sync_bool_compare_and_swap_p(&a, b, c);
    assert_true(ret);
    assert_true(a == c);
}

static void _sync_bool_compare_and_swap_p_noswap(void **state) {
    UNREFERENCED_PARAMETER(state);
    PVOID a = (PVOID)5, b = (PVOID)9, c = (PVOID)7;
    PVOID old_a = a;
    bool ret = _sync_bool_compare_and_swap_p(&a, b, c);
    assert_false(ret);
    assert_true(a == old_a);
}

static void _sync_bool_compare_and_swap_p_comp_max(void **state) {
    UNREFERENCED_PARAMETER(state);
    PVOID a = (PVOID)UINTPTR_MAX, b = (PVOID)UINTPTR_MAX, c = 0;
    bool ret = _sync_bool_compare_and_swap_p(&a, b, c);
    assert_true(ret);
    assert_true(a == c);
}

static void _sync_bool_compare_and_swap_p_assign_max(void **state) {
    UNREFERENCED_PARAMETER(state);
    PVOID a = 0, b = 0, c = (PVOID)UINTPTR_MAX;
    bool ret = _sync_bool_compare_and_swap_p(&a, b, c);
    assert_true(ret);
    assert_true(a == c);
}

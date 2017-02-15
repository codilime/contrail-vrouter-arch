#pragma once

void _sync_fetch_and_add_32u_races(void **state);
void _sync_fetch_and_add_64u_races(void **state);
void _sync_add_and_fetch_32u_races(void **state);
void _sync_sub_and_fetch_32u_races(void **state);
void _sync_sub_and_fetch_32s_races(void **state);
void _sync_sub_and_fetch_64u_races(void **state);
void _sync_sub_and_fetch_64s_races(void **state);

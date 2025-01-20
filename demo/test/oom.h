#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct heap heap_t;

/// @brief Makes next call to any allocation function return NULL
/// @param toggle if calls after faliure should should return NULL as well
void oom_next_alloc (bool toggle);

/// @brief Makes n:th call to any allocation function return NULL
/// num = 0 is equal to calling oom_next_alloc(toggle)
/// @param num how many allocation calls before it returns NULL
/// @param toggle if calls after faliure should should return NULL as well
void oom_nth_alloc_call (size_t num, bool toggle);

/// @brief Sets allocation to behave normaly
void set_normal_alloc (void);

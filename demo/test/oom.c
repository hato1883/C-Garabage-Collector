#include <stdlib.h>

#include "oom.h"

/// @brief Remaing calls before malloc return NULL
size_t remaining_allocations = 0;

/// @brief if malloc should only fail once
bool should_oom_next_alloc = false;

/// @brief if malloc should fail more than once
bool oom_toggled = false;

extern void *__real_malloc (size_t size);
extern void *__real_calloc (size_t count, size_t size);
extern void *__real_h_alloc_struct (heap_t *h, char *layout);
extern void *__real_h_alloc_raw (heap_t *h, size_t bytes);
extern void __real_free (void *ptr);

#define wrap_allocation(real_method_with_args)                                \
  if (should_oom_next_alloc || oom_toggled)                                   \
    {                                                                         \
      if (remaining_allocations == 0)                                         \
        {                                                                     \
          should_oom_next_alloc = false;                                      \
          return NULL;                                                        \
        }                                                                     \
      else                                                                    \
        {                                                                     \
          remaining_allocations--;                                            \
        }                                                                     \
    }                                                                         \
  return real_method_with_args;

/// @brief Allows for simulated failure of malloc
void *
__wrap_malloc (size_t size)
{
  wrap_allocation (__real_malloc (size));
}

/// @brief Allows for simulated failure of calloc
void *
__wrap_calloc (size_t count, size_t size)
{
  wrap_allocation (__real_calloc (count, size));
}

/// @brief Allows for simulated failure of h_alloc_struct
void *
__wrap_h_alloc_struct (heap_t *h, char *layout)
{
  wrap_allocation (__real_h_alloc_struct (h, layout));
}

/// @brief Allows for simulated failure of h_alloc_raw
void *
__wrap_h_alloc_raw (heap_t *h, size_t bytes)
{
  wrap_allocation (__real_h_alloc_raw (h, bytes));
}

/// @brief Normal free function, no changes made
/// @param ptr to location in the heap to free
void
__wrap_free (void *ptr)
{
  __real_free (ptr);
}

void
oom_next_alloc (bool toggle)
{
  oom_nth_alloc_call (0, toggle);
}

void
oom_nth_alloc_call (size_t num, bool toggle)
{
  should_oom_next_alloc = true;
  oom_toggled = toggle;
  remaining_allocations = num;
}

void
set_normal_alloc (void)
{
  oom_toggled = false;
  should_oom_next_alloc = false;
  remaining_allocations = 0;
}

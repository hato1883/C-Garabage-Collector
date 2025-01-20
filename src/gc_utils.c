/**
 * Utility functions for the GC, contains two functions.
 */

#include <assert.h>

#include "gc_utils.h"
#include "heap.h"
#include "heap_internal.h"

void
apply_to_pointers_in_interval (uintptr_t start, uintptr_t end,
                               apply_to_ptr_func *func, void *arg)
{
  for (uintptr_t ptr = start; ptr < end; ptr += sizeof (void *))
    {
      func ((void *)ptr, arg);
    }
}

size_t
calc_heap_offset (void *heap_ptr, heap_t *h)
{
  char *c_ptr = (char *)heap_ptr;
  char *heap_start = (char *)h->heap_start;
  assert (c_ptr - heap_start >= 0);
  return c_ptr - heap_start;
}

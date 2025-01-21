/**
 * Helper functions regarding the heap_t struct.
 */

#pragma once

#include "allocation_map.h"
#include "heap_internal.h"

size_t
num_free_bytes (heap_t *heap)
{
  size_t used = num_allocated_bytes (heap->alloc_map);
  return heap->size - used;
}
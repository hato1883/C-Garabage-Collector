/**
 * Helper functions regarding the heap_t struct.
 */

#pragma once

#include "allocation_map.h"
#include "heap_internal.h"

heap_t *
create_heap (size_t bytes, bool unsafe_stack, float threshold)
{
  /* Allocate heap struct on the heap.  */
  heap_t *heap = malloc (sizeof (heap_t));

  /* Align size of heap to the allocation maps BYTE_DENSITY */
  size_t aligned_size = round_up_to_alignment (bytes, ALLOCATION_MAP_DENSITY);

  /* Set fields:  */
  heap->gc_threshold = threshold;
  heap->is_unsafe_stack = unsafe_stack;
  heap->size = aligned_size;
  heap->alloc_map = create_allocation_map (aligned_size);
  heap->heap_start = calloc (aligned_size, sizeof (char));
  heap->next_empty_mem_segment = heap->heap_start;
  heap->used_bytes = 0;

  return heap;
}

void
destroy_heap (heap_t *heap)
{
  free (heap->heap_start);
  free (heap);
}

size_t
num_free_bytes (heap_t *heap)
{
  size_t used = num_allocated_bytes (heap->alloc_map);
  return heap->size - used;
}

void
toggle_direction (heap_t *heap)
{
  heap->left_to_right = !heap->left_to_right;
}
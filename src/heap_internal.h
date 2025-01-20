/**
 * Contains the heap stuct.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "heap.h"

/**
 * @param gc_threshold: The percentage of memory that has to be utilized before
 * garbage collection begins.
 * @param uses_safe_pointers: Boolean that describes whether or not stack
 * pointers should be treated as safe.
 * @param size: The size of the heap in bytes.
 * @param page_size: The size of a page in the heap, in bytes.
 * @param page_map: An bitvector array with each bit indicating whether each
 * page is active or not.
 * @param alloc_map: An array of booleans representing if each location in the
 * heap is used or empty.
 * @param heap_start: The pointer to the heap.
 * @param next_empty_mem_segment: A bump pointer to the next empty available
 * space in the heap that can be used for allocation.
 * @param used_bytes: The amount of bytes currently allocated by the user.
 */
struct heap
{
  float gc_threshold;
  bool is_unsafe_stack;
  size_t size;
  size_t page_size;
  char *page_map;
  char *alloc_map;
  void *heap_start;
  char *next_empty_mem_segment;
  size_t used_bytes;
};

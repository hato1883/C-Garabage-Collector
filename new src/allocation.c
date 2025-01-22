/**
 * This file contains all the functions responsible for memory allocation
 * within a heap. It includes allocating, managing heap utilization thresholds,
 * and aligning allocations.
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocation.h"
#include "allocation_map.h"
#include "format_encoding.h"
#include "gc.h"
#include "gc_utils.h"
#include "header.h"
#include "heap_internal.h"

static void trigger_gc_on_threshold_reached (heap_t *h, size_t new_alloc_size);
static void *make_alloc (heap_t *h, size_t alloc_size);
static void update_alloc_map_for_allocation (heap_t *h, void *allocation_start,
                                             size_t alloc_size);

/**
 * Checks if the new allocation would cause the heap utilization to exceed the
 * GC- threshold, if so begin GC.
 * @param h - the heap that is being checked
 * @param new_alloc_size - the amount of bytes (including the header) that is
 * being allocated
 */
static void
trigger_gc_on_threshold_reached (heap_t *h, size_t new_alloc_size)
{
  size_t size_after_alloc = h_used (h) + new_alloc_size;
  double_t usage_percentage = ((double_t)size_after_alloc / h->size);
  if (usage_percentage >= h->gc_threshold)
    {
      /* Check if the percentage of memory used after allocation would exceed
         the threshold. If so, trigger garbage collection before allocating. */
      h_gc (h);
    }
}

/**
 * Stores and returns the current bump pointer address sets the bits in the
 * allocated area to 0 and moves the bump pointer by alloc_size.
 * @param h - the heap to allocate in
 * @param alloc_size - the amount of bytes to be allocated excluding header
 * @return the address to the allocated area
 */
static void *
make_alloc (heap_t *h, size_t alloc_size)
{
  void *allocated = h->next_empty_mem_segment;
  memset (allocated, 0, alloc_size); /* Sets the newly allocated memory to be
                                        set to 0, much like calloc.  */
  h->used_bytes += alloc_size; /* Updates the number of used bytes variable. */

  /* Move bump pointer by the allocation size so that its new address is the
     next free area.  */
  h->next_empty_mem_segment = h->next_empty_mem_segment + alloc_size;
  return allocated;
}

/**
 * Updates the allocation map for all bytes in the allocation including header.
 * @param h the heap in which the allocation was made.
 * @param allocation_start the pointer to the start of the allocation
 * @param alloc_size Size of allocation (Excluding header).
 */
static void
update_alloc_map_for_allocation (heap_t *h, void *allocation_start,
                                 size_t alloc_size)
{
  assert (allocation_start >= h->heap_start
          && "Allocation's start address is before start of the heap!");
  size_t alloc_offset = calc_heap_offset (allocation_start, h);
  size_t start_alloc_offset = alloc_offset - sizeof (header_t);
  size_t alloc_size_with_header = alloc_size + sizeof (header_t);

  update_alloc_map_range (h->alloc_map, start_alloc_offset,
                          alloc_size_with_header, true);
}

size_t
calc_alloc_size (char *alloc)
{
  header_t header = get_header_value (get_header_pointer (alloc));
  return size_from_header (header);
}

void *
alloc_struct (heap_t *h, char *layout)
{
  bool calc_size_success = false;
  size_t alloc_size = size_from_string (layout, &calc_size_success);
  alloc_size = align_alloc_size (alloc_size);
  if (!calc_size_success)
    {
      /* abort();  */
      return NULL;
    }

  bool header_success = false;
  header_t header = convert_struct_to_header (h, layout, &header_success);
  if (!header_success)
    {
      /* abort();  */
      return NULL;
    }

  bool is_alloc_possible = move_to_valid_space_if_alloc_possible (
      h, alloc_size + sizeof (header_t));
  if (!is_alloc_possible)
    {
      /* abort();  */
      return NULL;
    }
  /* Save the object's header metadata on the heap.  */
  *((header_t *)h->next_empty_mem_segment) = header;
  h->next_empty_mem_segment += sizeof (header_t);

  /* Makes allocation and moves bump pointer.  */
  update_alloc_map_for_allocation (h, h->next_empty_mem_segment, alloc_size);

  return make_alloc (h, alloc_size);
}

void *
alloc_raw (heap_t *h, size_t alloc_size)
{
  alloc_size = align_alloc_size (alloc_size);

  bool header_success = false;
  header_t header = create_header_raw (alloc_size, &header_success);
  if (!header_success)
    {
      /* abort();  */
      return NULL;
    }

  bool is_alloc_possible = move_to_valid_space_if_alloc_possible (
      h, alloc_size + sizeof (header_t));
  if (!is_alloc_possible)
    {
      /* abort();  */
      return NULL;
    }

  /* Save the object's header metadata on the heap.  */
  *((header_t *)h->next_empty_mem_segment) = header;
  h->next_empty_mem_segment += sizeof (header_t);

  /* Makes allocation and moves bump pointer.  */
  update_alloc_map_for_allocation (h, h->next_empty_mem_segment, alloc_size);

  return make_alloc (h, alloc_size);
}

bool
move_to_valid_space_if_alloc_possible (heap_t *h, size_t alloc_size)
{
  size_t alloc_size_with_metadata = alloc_size + sizeof (header_t);

  /* Trigger gc if threshold would be reached when allocation is finished.  */

  trigger_gc_on_threshold_reached (h, alloc_size_with_metadata);
  bool space_found;
  size_t offset = find_offset_of_empty_region (
      h->alloc_map, alloc_size_with_metadata, h->left_to_right, &space_found);
  if (space_found)
    {
      /* Moves to next available space in the heap.  */
      h->next_empty_mem_segment = h->heap_start + offset;
    }
  return space_found;
}

size_t
align_alloc_size (size_t alloc_size)
{
  /* Align size to our minimum alignment.  */
  size_t total_size = alloc_size + sizeof (header_t);
  total_size = round_up_to_alignment (total_size, MIN_ALLOC_OBJECT_SIZE);
  return total_size - sizeof (header_t);
}

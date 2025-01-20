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
#include "get_header.h"
#include "header.h"
#include "heap.h"
#include "heap_internal.h"
#include "page_map.h"

/**
 * Finds the end address of the heap.
 * @param h the heap
 * @return the end address of the heap
 */
char *
find_heap_end (heap_t *h)
{
  return ((char *)h->heap_start) + h->size;
}

/**
 * Calculates the size of an allocation
 * @param alloc a pointer to the allocation
 * @return size_t the size of the allocation in bytes.
 */
size_t
calc_alloc_size (char *alloc)
{
  header_t header = get_header_value (get_header_pointer (alloc));
  header_type_t header_type = get_header_type (header);
  if (header_type == HEADER_BIT_VECTOR)
    {
      uint_fast64_t vector = header >> BITS_FOR_HEADER_TYPE;
      return size_from_vector (vector);
    }
  else if (header_type == HEADER_POINTER_TO_FORMAT_STRING)
    {
      char *ptr_to_format = (char *)(header & ~(0x3));
      bool success = false;
      size_t size = (size_t)size_from_string (ptr_to_format, &success);
      assert (success);
      return size;
    }
  else
    {
      assert (false); /* Any other formats should not appear at this point.  */
    }
  return 0;
}

/**
 * Checks if the new allocation would cause the heap utilization to exceed the
 * GC- threshold, if so begin GC.
 * @param h - the heap that is being checked
 * @param new_alloc_size - the amount of bytes (including the header) that is
 * being allocated
 */
void
trigger_gc_on_treshold_reached (heap_t *h, size_t new_alloc_size)
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
 * @param alloc_size - the amount of bytes to be allocated
 * @return the address to the allocated area
 */
void *
make_alloc (heap_t *h, size_t alloc_size)
{
  void *allocated = h->next_empty_mem_segment;
  memset (allocated, 0, alloc_size); /* Sets the newly allocated memory to be
                                        set to 0, much like calloc.  */
  h->used_bytes += alloc_size; /* Updates the number of used bytes variable. */

  /* Move bump pointer by the allocation size so that its new address is the
     next free area.  */
  h->next_empty_mem_segment = (char *)h->next_empty_mem_segment + alloc_size;
  return allocated;
}

/**
 * Checks if the requested allocation would not overwrite any data on the heap.
 * Used to avoid overwriting data in pages that could not be compacted.
 * @param h the heap to check within
 * @param total_alloc_size the requested allocation size (including header)
 * @note modifies fits to be true if the allocation can be allocated without
 * issues
 */
size_t
does_alloc_fit (heap_t *h, size_t total_alloc_size, bool *fits)
{
  *fits = true;
  char *alloc_map = (char *)h->alloc_map;
  size_t start_offset = calc_heap_offset (h->next_empty_mem_segment, h);
  for (size_t offsets_to_move = 0; offsets_to_move < total_alloc_size;
       offsets_to_move += HEAP_ALIGNMENT)
    {
      if (start_offset + offsets_to_move >= h->size)
        {
          return 0;
        }
      bool is_occupied
          = is_offset_allocated (alloc_map, start_offset + offsets_to_move);
      if (is_occupied)
        {
          *fits = false;
          return offsets_to_move + 1;
        }
    }
  return 0;
}

/**
 * Move to start of next page if allocation would cross into the next page.
 * @param h the heap to move within
 * @param total_alloc_size the requested allocation size (including header)
 * @return true if a skip was performed, otherwise false
 */
bool
skip_on_page_overlap (heap_t *h, size_t total_alloc_size)
{
  /* Move to start of next page if allocation would cross into the next page.
   */
  size_t offset = calc_heap_offset (h->next_empty_mem_segment, h);
  size_t bytes_remaining_on_page = bytes_from_next_page (h->page_size, offset);
  if (bytes_remaining_on_page < total_alloc_size)
    {
      /* Move to the start of the next page.  */
      h->next_empty_mem_segment += bytes_remaining_on_page;
      return true;
    }
  return false;
}

/**
 * Checks if the bump pointer has exceeded the end of the heap or not.
 * @param h the heap
 * @return true if the bump pointer has not exceeded the end of the heap
 */
bool
is_bump_pointer_in_heap (heap_t *h)
{
  char *heap_end = find_heap_end (h);
  return h->next_empty_mem_segment < heap_end;
}

bool
move_to_next_available_space (heap_t *h, size_t total_alloc_size)
{
  bool is_available = false;
  while (!is_available && is_bump_pointer_in_heap (h))
    {
      /* Loops until an available space is found or the bump pointer has
         exceeded the heap.
         Checks if another allocation occupies the targeted area.  */
      bool alloc_fits = true;
      size_t bytes_to_move = does_alloc_fit (h, total_alloc_size, &alloc_fits);
      if (!alloc_fits)
        {
          h->next_empty_mem_segment += bytes_to_move;
          /* Go to next iteration to check if alloc fits in next space.  */
          continue;
        }

      /* Moves to the next page if allocation would overlap.  */
      bool moved = skip_on_page_overlap (h, total_alloc_size);
      is_available = !moved;
    }

  /* Lastly check that making an allocation here would not exceed the heap
     space.  */
  char *next_segment_after_alloc
      = h->next_empty_mem_segment + total_alloc_size;
  if (is_available && next_segment_after_alloc > find_heap_end (h))
    {
      /* Memory would exceed available heap memory even after GC! */
      return false;
    }
  return is_available;
}

size_t
align_alloc_size (size_t alloc_size)
{
  /* Align size to our minimum alignment.  */
  size_t total_size = alloc_size + sizeof (header_t);
  if (total_size % MINIMUM_ALIGNMENT > 0 || total_size <= 0)
    {
      size_t diff = MINIMUM_ALIGNMENT - (total_size % MINIMUM_ALIGNMENT);
      return total_size + (diff) - sizeof (header_t);
    }
  return total_size - sizeof (header_t);
}

/**
 * Updates the allocation map for all bytes in the allocation.
 * @param h the heap in which the allocation was made.
 * @param allocation_start the pointer to the start of the allocation
 * @param alloc_size
 */
void
update_alloc_map_for_allocation (heap_t *h, void *allocation_start,
                                 size_t alloc_size)
{
  assert (allocation_start >= h->heap_start
          && "Allocation's start address is before start of the heap!");
  size_t alloc_offset = calc_heap_offset (allocation_start, h);
  size_t start_alloc_offset = alloc_offset - sizeof (header_t);
  size_t end_alloc_offset = alloc_offset + alloc_size;

  /* Loops through each 16 bytes in the allocated space and set its
    corresponding bit in the allocation map to 1.  */
  for (size_t offset = start_alloc_offset; offset < end_alloc_offset;
       offset += HEAP_ALIGNMENT)
    {
      update_alloc_map (h->alloc_map, offset, true);
    }
}

/**
 * Checks if the allocation is possible. May trigger GC to make space in the
 * heap if threshold is reached. If allocation is possible the bump pointer is
 * moved to the next available space.
 * @param h the heap
 * @param alloc_size the requested allocation size
 * @return true if the allocation is possible.
 */
bool
move_to_valid_space_if_alloc_possible (heap_t *h, size_t alloc_size)
{
  size_t alloc_size_with_metadata = alloc_size + sizeof (header_t);
  if (alloc_size_with_metadata > h->page_size)
    {
      /* Allocation is too large!
         abort();  */
      return false;
    }
  /* Trigger gc if threshold would be reached when allocation is finished.  */
  trigger_gc_on_treshold_reached (h, alloc_size_with_metadata);

  bool space_found = move_to_next_available_space (
      h, alloc_size_with_metadata); /* Moves to next available space in the
                                       heap.  */
  return space_found;
}

void *
alloc_struct (heap_t *h, char *layout)
{
  bool calc_size_success = false;
  int_fast64_t alloc_size = size_from_string (layout, &calc_size_success);
  if (!calc_size_success)
    {
      /* abort();  */
      return NULL;
    }

  alloc_size
      = align_alloc_size (alloc_size); /* Align size to minimum alignment.  */

  bool header_success = false;
  header_t header = create_header_struct (layout, alloc_size, &header_success);
  if (get_header_type (header) == HEADER_POINTER_TO_FORMAT_STRING)
    {
      /* Add 1 because strlen does not count null.  */
      char *allocated_layout = alloc_raw (h, strlen (layout) + 1);
      if (allocated_layout == NULL)
        {
          return NULL;
        }
      strcpy (allocated_layout, layout);
      header = change_pointer_in_header (header, allocated_layout);
      header = set_header_pointer_to_format_string(header);
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
  alloc_size
      = align_alloc_size (alloc_size); /* Align size to minimum alignment.  */

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

/**
 * The purpose of this file is to know if something is a pointer or not in a
 * heap.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "allocation_map.h"
#include "header.h"
#include "heap_internal.h"
#include "is_pointer_in_alloc.h"

/**
 * @brief Checks if a pointer address (or maybe not) is in a heaps range.
 * @param pointer - unknown pointer (or maybe not).
 * @param the_heap - the_heap on which we want to check if the pointer is in
 * @return true if the value is in range of the heap address, false if not.
 */
bool
is_in_range (void *pointer, heap_t *the_heap)
{
  /* Compare the address of the heap to the ptr */
  void *beginning = the_heap->heap_start;
  void *end = beginning + the_heap->size;

  /* Simply check if it is in range  */
  return (pointer >= beginning && pointer < end);
}

/**
 * @brief Checks whether a pointer (might not be one though) is in an active
 * memory.
 * @param pointer - the supposed pointer.
 * @param the_heap - the_heap in which we check if pointer might be active
 * in.
 * @return true if the pointer is in active memory, false if not.
 */
bool
is_in_allocated_region (void *pointer, heap_t *the_heap)
{
  if (!is_in_range (pointer, the_heap))
    {
      return false;
    }

  size_t diff = pointer - the_heap->heap_start;
  char *map = the_heap->alloc_map;

  assert (map != NULL && "The alloc map is null.");
  char status = map[find_index_in_alloc_map (diff)] & create_bitmask (diff);

  return status != 0;
}

/**
 * @brief Checks if a pointer is an address that is within a heap.
 * @param pointer - the (supposed) address we want to check.
 * @param the_heap - the heap we are checking the address in.
 * @return true if address is within heap.
 */
bool
is_heap_pointer (void *pointer, heap_t *the_heap)
{
  return is_in_allocated_region (pointer, the_heap);
}

/**
 * The purpose of this file is to know if something is a pointer or not in a
 * heap.
 */

// TODO: MODIFY FILE

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "allocation_map.h"
#include "header.h"
#include "heap_internal.h"
#include "is_pointer_in_alloc.h"

/*
 * TODO: Right now we need to check the entire heap, it would be good if had
 * a starting location narrowing the search down.
 * Checks if a pointer address (or maybe not) is in a heaps range.
 * @param ptr_addr - unknown pointer (or maybe not).
 * @param the_heap - the_heap on which we want to check if the pointer is in
 * @return true if the value is in range of the heap address, false if not.  */
bool
is_in_range (uintptr_t ptr_addr, heap_t *the_heap)
{
  /* Compare the address of the heap to the uintptr_t */
  uintptr_t beginning = (uintptr_t)the_heap->heap_start;
  uintptr_t end = beginning + the_heap->size;

  /* Simply check if it is in range  */
  if (ptr_addr >= beginning && ptr_addr < end)
    {
      return true;
    }

  return false;
}

/* Checks whether a pointer (might not be one though) is in an active memory.
 * @param ptr_addr - the supposed pointer.
 * @param the_heap - the_heap in which we check if ptr_address might be active
 * in.
 * @return true if the ptr_addr is in active memory, false if not.  */
bool
is_in_allocated_region (uintptr_t ptr_addr, heap_t *the_heap)
{
  if (!is_in_range (ptr_addr, the_heap))
    {
      return false;
    }

  size_t diff = ptr_addr - (uintptr_t)the_heap->heap_start;
  char *alloc_map = the_heap->alloc_map;

  assert (alloc_map != NULL && "The alloc map is null.");

  if (alloc_map[find_index_in_alloc_map (diff)] & create_bitmask (diff))
    {
      return true;
    }

  return false;
}

/**
 * Checks if a uintptr_t is an address that is within a heap.
 * @param ptr_addr - the (supposed) address we want to check.
 * @param the_heap - the heap we are checking the address in.
 * @return true if address is within heap.
 */
bool
is_heap_pointer (uintptr_t ptr_addr, heap_t *the_heap)
{
  return is_in_allocated_region (ptr_addr, the_heap);
}

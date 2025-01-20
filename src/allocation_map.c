/**
 * Function for creating a allocation map.
 * The allocation map represents usage of memoryaddresses,
 * represented by booleans.
 * BOOLEANS_PER_BYTE is how many booleans can fit in each byte (8).
 * MIN_ALLOC_OBJECT_SIZE is the smallest object that can be created (16).
 * Above results in each boolean representing 16 bytes on the heap,
 * and each byte of the allocation map represents 8 booleans and therefore
 * 128 bytes in the heap
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "allocation_map.h"
#include "heap_internal.h"

/* Creates an allocation map of a given amount of bytes. This size needs to be
 * atleast the MIN_OBJECT_SIZE and divisible evenly with BOOLEANS_PER_BYTE.  */
char *
create_allocation_map (size_t bytes)
{
  /* Map needs to be atleast this: */
  assert (bytes >= MIN_ALLOC_OBJECT_SIZE);

  /* Bytes needs to be divisible evenly by BOOLEANS_PER_BYTE.  */
  assert (bytes % ALLOCATIONS_PER_BYTE == 0);

  /* Calculate the smallest size needed to store the array
  by adding BYTE_DENSITY - 1 and the dividing by BYTE_DENSITY
  it will round up anything that is more than a multiple of
  the BYTE_DENSITY constant. Lastly check so array size > 0 */
  size_t size_of_array = (bytes + BYTE_DENSITY - 1) / BYTE_DENSITY;
  size_of_array = size_of_array == 0 ? 1 : size_of_array;

  /* Creation of allocation map  */
  char *heap_map = calloc (size_of_array, sizeof (char));
  return heap_map;
}

void
reset_allocation_map (heap_t *h)
{
  size_t bytes = h->size;

  /* Calculate the smallest size needed to store the array
  by adding BYTE_DENSITY - 1 and the dividing by BYTE_DENSITY
  it will round up anything that is more than a multiple of
  the BYTE_DENSITY constant. Lastly check so array size > 0 */
  size_t size_of_array = (bytes + BYTE_DENSITY - 1) / BYTE_DENSITY;
  size_of_array = size_of_array == 0 ? 1 : size_of_array;

  /* Creation of allocation map  */
  memset (h->alloc_map, 0, size_of_array);
}

size_t
find_index_in_alloc_map (size_t offset)
{
  /* Each byte represents 8*16 = 128 on the heap  */
  return offset / BYTE_DENSITY;
}

/* Function for creating a bit mask for a specific bit in a byte */
char
create_bitmask (size_t offset)
{
  /* Find the specific bit within a byte  */
  size_t bit_position = (offset / MIN_ALLOC_OBJECT_SIZE) % ALLOCATIONS_PER_BYTE;
  /* Set the specific bit*/
  return 1 << bit_position;
}

void
update_alloc_map (char *alloc_map, size_t offset, bool is_allocated)
{
  size_t alloc_index = find_index_in_alloc_map (offset);
  char bitmask = create_bitmask (offset);
  char byte = alloc_map[alloc_index];

  if (is_allocated)
    {
      /* Sets the target bit to 1 using the OR operator.  */
      byte = byte | bitmask;
    }
  else
    {
      /* Sets the target bit to 0 using the AND operator with the negated
         bitmask.  */
      bitmask = ~bitmask; /* Negates the bitmask. */
      byte = byte & bitmask;
    }

  alloc_map[alloc_index] = byte;
}

bool
is_offset_allocated (char *alloc_map, size_t offset)
{
  size_t map_index = find_index_in_alloc_map (offset);
  char bitmask = create_bitmask (offset);
  char byte = alloc_map[map_index];
  bool is_allocated = (byte & bitmask) == bitmask;
  return is_allocated;
}

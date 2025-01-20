/**
 * @file
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
#include "gc_utils.h"
#include "heap_internal.h"

/**
 * @def Bitmask Equal to 1000 0000.
 */
#define MSB 0x80

/**
 * @def Bitmask Equal to 1111 1111.
 */
#define FULL 0xFF

/**
 * @def Bitmask Equal to 0000 0000.
 */
#define EMPTY 0x0

/* Creates an allocation map of a given amount of bytes. This size needs to be
 * atleast the MIN_OBJECT_SIZE and divisible evenly with BOOLEANS_PER_BYTE.  */
char *
create_allocation_map (size_t bytes)
{
  /* Adjust size to needed alignment */
  size_t size_of_array = alloc_map_size_needed (bytes);

  /* Creation of allocation map  */
  char *heap_map = calloc (size_of_array, sizeof (char));

  /* Verify heap map was created */
  assert (heap_map != NULL);

  return heap_map;
}

/* Sets all bytes in allocation map to 0, effectively clearing it */
void
reset_allocation_map (heap_t *h)
{
  /* Adjust size to needed alignment */
  size_t size_of_array = alloc_map_size_needed (h->size);

  /* Creation of allocation map  */
  memset (h->alloc_map, 0, size_of_array);
}

/* Finds byte in allocation map corresponding to given heap offset */
size_t
find_index_in_alloc_map (size_t offset)
{
  /* Each byte represents 8*8 = 64 bytes on the heap  */
  return offset / BYTE_DENSITY;
}

/* Function for creating a bit mask for a specific bit in a byte */
char
create_bitmask (size_t offset)
{
  /* Find the specific bit within a byte  */
  size_t bit_position
      = (offset / MIN_ALLOC_OBJECT_SIZE) % ALLOCATIONS_PER_BYTE;

  /* Set the specific bit */
  return MSB >> bit_position;
}

/* Updates a single bit in allocation map */
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

/* Updates a range of bits in allocation map,
   Uses larger bitmasks where possible to avoid unecessary edits */
void
update_alloc_map_range (char *alloc_map, size_t start, size_t size,
                        bool is_allocated)
{
  /* Offset from start of allocation */
  size_t offset;

  /* Index of the byte in the allocation map */
  size_t byte_index;

  /* Current value of the byte before updating it */
  unsigned char byte;

  /* Bit mask representing allocations within a single byte */
  unsigned char bitmask;

  /* Check if we need to update atleast entire byte worth in the map */
  if (size > BYTE_DENSITY)
    {
      offset = 0;

      /* First allocate the unaligned start region in the allocation map */
      bitmask = EMPTY;
      while ((start + offset) % BYTE_DENSITY > 0 && offset < size)
        {
          bitmask |= create_bitmask (start + offset);
          offset += MIN_ALLOC_OBJECT_SIZE;
        }

      /* get index of byte and update it using generated bitmask */
      byte_index = find_index_in_alloc_map (start);
      byte = alloc_map[byte_index];
      alloc_map[byte_index]
          = is_allocated ? byte | bitmask : byte & (~bitmask);

      /* Then allocate aligned region of the map */
      if ((offset + BYTE_DENSITY) < size)
        {
          byte_index = find_index_in_alloc_map (start + offset);
          alloc_map[byte_index] = is_allocated ? FULL : EMPTY;
          offset += BYTE_DENSITY;
        }

      /* lastly allocate the unaligned end region in the allocation map */
      while (offset < size)
        {
          bitmask |= create_bitmask (start + offset);
          offset += MIN_ALLOC_OBJECT_SIZE;
        }

      /* get index of byte and update it using generated bitmask */
      byte_index = find_index_in_alloc_map (start);
      byte = alloc_map[byte_index];
      alloc_map[byte_index]
          = is_allocated ? byte | bitmask : byte & (~bitmask);
    }
  else
    {
      bitmask = EMPTY;
      offset = 0;
      while (offset < size)
        {
          bitmask |= create_bitmask (start + offset);
          offset += MIN_ALLOC_OBJECT_SIZE;
        }
      byte_index = find_index_in_alloc_map (start);
      byte = alloc_map[byte_index];
      alloc_map[byte_index]
          = is_allocated ? byte | bitmask : byte & (~bitmask);
    }
}

/* Returns true if offset is allocated */
bool
is_offset_allocated (char *alloc_map, size_t offset)
{
  size_t map_index = find_index_in_alloc_map (offset);
  char bitmask = create_bitmask (offset);
  char byte = alloc_map[map_index];
  bool is_allocated = (byte & bitmask) == bitmask;
  return is_allocated;
}

/* Calculates the minimum size needed to handel heap_size amount of data */
size_t
alloc_map_size_needed (size_t heap_size)
{
  return round_up_to_alignment (heap_size, BYTE_DENSITY);
}

/* Finds first available region that can fit the given size of bytes.
   Toggle if search should move left to right or right to left. */
size_t
find_offset_of_empty_region (char *alloc_map, size_t map_size, size_t alloc_size, bool left_to_right)
{
  size_t offset;
  if (left_to_right)
  {
    offset = 0;
    while (offset + alloc_size < map_size * BYTE_DENSITY)
    {
      bool fits_alloc = true;
      /* code */
      offset += MIN_ALLOC_OBJECT_SIZE;
    }
  }
  else
  {
    offset = (map_size * BYTE_DENSITY) - alloc_size - MIN_ALLOC_OBJECT_SIZE;
    while (offset + alloc_size < map_size * BYTE_DENSITY)
    {
      bool fits_alloc = true;
      /* code */
      offset -= MIN_ALLOC_OBJECT_SIZE;
    }
  }
  return offset;
}

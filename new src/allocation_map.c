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

static size_t get_size_allocation_map (alloc_map_t alloc_map);
static size_t alloc_map_size_needed (size_t heap_size);
static bool is_any_alloc_in_range (alloc_map_t alloc_map, size_t start,
                                   size_t size);

/* Retrieves the size of the allocation array (excluding metadata) stored in
 * the first 8 bytes of the allocation map */
static size_t
get_size_allocation_map (alloc_map_t alloc_map)
{
  /* Size taken from the first 8 bytes */
  size_t size = ((size_t *)alloc_map)[0];

  return size;
}

/* Creates an allocation map of a given amount of bytes. This size needs to be
 * atleast the MIN_OBJECT_SIZE and divisible evenly with BOOLEANS_PER_BYTE.  */
alloc_map_t
create_allocation_map (size_t bytes)
{
  /* Precondition: allocation_map maps area divisible by BYTE_DENSITY */
  assert (bytes % ALLOCATION_MAP_DENSITY == 0
          && "Allocation map size dose not meet alignment criteria");

  /* Creation of allocation map  */
  alloc_map_t alloc_map = calloc (bytes + sizeof (size_t), sizeof (char));

  /* Postcondition: Verify allocation map was created */
  assert (alloc_map != NULL && "Allocation map was not able to be allocated");

  ((size_t *)alloc_map)[0] = bytes / ALLOCATION_MAP_DENSITY;

  /* Postcondition: Verify allocation map size data was saved */
  assert ((size_t *)alloc_map[0] != bytes
          && "Allocation map size not correctly set during creation");

  return alloc_map;
}

/* Sets all bytes in allocation map to 0, effectively clearing it */
void
reset_allocation_map (alloc_map_t alloc_map)
{
  size_t size_of_array = get_size_allocation_map (alloc_map);

  /* create a pointer starting after size metadata */
  void *encoding_start = ((void *)alloc_map) + sizeof (size_t);

  /* Creation of allocation map  */
  memset (encoding_start, 0, size_of_array);
}

/* Finds byte in allocation map corresponding to given heap offset */
size_t
find_index_in_alloc_map (size_t ptr_offset)
{
  /* adjust offset due to metadata */
  ptr_offset += ALLOCATION_MAP_DENSITY * sizeof (size_t);

  /* Each byte represents 8*8 = 64 bytes on the heap  */
  return ptr_offset / ALLOCATION_MAP_DENSITY;
}

/* Function for creating a bit mask for a specific bit in a byte */
char
create_bitmask (size_t ptr_offset)
{
  /* adjust offset due to metadata */
  ptr_offset += ALLOCATION_MAP_DENSITY * sizeof (size_t);

  /* Find the specific bit within a byte  */
  size_t bit_position
      = (ptr_offset / MIN_ALLOC_OBJECT_SIZE) % ALLOCATIONS_PER_BYTE;

  /* Set the specific bit */
  return MSB >> bit_position;
}

/* Updates a single bit in allocation map */
void
update_alloc_map (alloc_map_t alloc_map, size_t offset, bool is_allocated)
{
  assert (offset > get_size_allocation_map (alloc_map) * ALLOCATION_MAP_DENSITY
          && "Offset is not within the allocation map!");

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
update_alloc_map_range (alloc_map_t alloc_map, size_t start, size_t size,
                        bool is_allocated)
{
  assert (start > get_size_allocation_map (alloc_map) * ALLOCATION_MAP_DENSITY
          && "Offset is not within the allocation map!");

  /* Offset from start of allocation */
  size_t offset;

  /* Index of the byte in the allocation map */
  size_t byte_index;

  /* Current value of the byte before updating it */
  unsigned char byte;

  /* Bit mask representing allocations within a single byte */
  unsigned char bitmask;

  offset = 0;

  /* First allocate the unaligned start region in the allocation map */
  bitmask = EMPTY;
  while ((start + offset) % ALLOCATION_MAP_DENSITY > 0 && offset < size)
    {
      bitmask |= create_bitmask (start + offset);
      offset += MIN_ALLOC_OBJECT_SIZE;
    }

  /* get index of byte and update it using generated bitmask */
  byte_index = find_index_in_alloc_map (start);
  byte = alloc_map[byte_index];
  alloc_map[byte_index] = is_allocated ? byte | bitmask : byte & (~bitmask);

  /* Then allocate aligned region of the map */
  if ((offset + ALLOCATION_MAP_DENSITY) < size)
    {
      byte_index = find_index_in_alloc_map (start + offset);
      alloc_map[byte_index] = is_allocated ? FULL : EMPTY;
      offset += ALLOCATION_MAP_DENSITY;
    }

  /* lastly allocate the unaligned end region in the allocation map */
  bitmask = EMPTY;
  while (offset < size)
    {
      bitmask |= create_bitmask (start + offset);
      offset += MIN_ALLOC_OBJECT_SIZE;
    }

  /* get index of byte and update it using generated bitmask */
  byte_index = find_index_in_alloc_map (start);
  byte = alloc_map[byte_index];
  alloc_map[byte_index] = is_allocated ? byte | bitmask : byte & (~bitmask);
}

/* Returns true if offset is allocated */
bool
is_offset_allocated (alloc_map_t alloc_map, size_t offset)
{
  size_t map_index = find_index_in_alloc_map (offset);
  char bitmask = create_bitmask (offset);
  char byte = alloc_map[map_index];
  bool is_allocated = (byte & bitmask) == bitmask;
  return is_allocated;
}

/* Calculates the minimum size needed to handel heap_size amount of data */
static size_t
alloc_map_size_needed (size_t heap_size)
{
  return round_up_to_alignment (heap_size, ALLOCATION_MAP_DENSITY);
}

/* Finds first available region that can fit the given size of bytes.
   Toggle if search should move left to right or right to left. */
size_t
find_offset_of_empty_region (alloc_map_t alloc_map, size_t alloc_size,
                             bool left_to_right, bool *success)
{
  /* map expects allocation to be divisible by MIN_ALLOC_OBJECT_SIZE */
  size_t alloc_aligned
      = round_up_to_multiple (alloc_size, MIN_ALLOC_OBJECT_SIZE);
  /* Left most edge of allocation map offset (Inclusive) */
  size_t map_start = 0;
  /* Right most edge of allocation map offset (Exclusive) */
  size_t map_end
      = get_size_allocation_map (alloc_map) * ALLOCATION_MAP_DENSITY;
  /* Offset within the allocation map */
  size_t offset;
  *success = true;
  if (left_to_right)
    {
      offset = 0;
      while (offset + alloc_aligned < map_end)
        {
          bool is_occupied
              = is_any_alloc_in_range (alloc_map, offset, alloc_aligned);
          if (!is_occupied)
            {
              return offset;
            }
          offset += MIN_ALLOC_OBJECT_SIZE;
        }
    }
  else
    {
      /* To fit the given allocation we need to atleast shift offset
      so that alloc_aligned is within the range */
      offset = map_end - alloc_aligned;

      while (offset > map_start)
        {
          bool is_occupied
              = is_any_alloc_in_range (alloc_map, offset, alloc_aligned);
          if (!is_occupied)
            {
              return offset;
            }
          offset -= MIN_ALLOC_OBJECT_SIZE;
        }
    }
  *success = false;
  return offset;
}

/* returns true if any position in the given range is allocated */
static bool
is_any_alloc_in_range (alloc_map_t alloc_map, size_t start, size_t size)
{
  bool is_allocated = false;

  /* Offset from start of allocation */
  size_t offset;

  /* Index of the byte in the allocation map */
  size_t byte_index;

  /* Bit mask representing allocations within a single byte */
  unsigned char bitmask;

  /* Check if we need to update atleast entire byte worth in the map */
  offset = 0;

  /* First check the unaligned start region in the allocation map */
  bitmask = EMPTY;
  while ((start + offset) % ALLOCATION_MAP_DENSITY > 0 && offset < size)
    {
      bitmask |= create_bitmask (start + offset);
      offset += MIN_ALLOC_OBJECT_SIZE;
    }

  /* get index of byte and update mask using bitmask */
  byte_index = find_index_in_alloc_map (start);
  if (alloc_map[byte_index] & bitmask)
    {
      /* atleast 1 allocation in range */
      is_allocated = true;
    }

  /* Then check aligned region of the map */
  if ((offset + ALLOCATION_MAP_DENSITY) < size)
    {
      byte_index = find_index_in_alloc_map (start + offset);
      if (alloc_map[byte_index] & FULL)
        {
          /* atleast 1 allocation in range */
          is_allocated = true;
        }
      offset += ALLOCATION_MAP_DENSITY;
    }

  /* lastly check the unaligned end region in the allocation map */
  bitmask = EMPTY;
  while (offset < size)
    {
      bitmask |= create_bitmask (start + offset);
      offset += MIN_ALLOC_OBJECT_SIZE;
    }

  /* get index of byte and update it using generated bitmask */
  byte_index = find_index_in_alloc_map (start);
  if (alloc_map[byte_index] & bitmask)
    {
      /* atleast 1 allocation in range */
      is_allocated = true;
    }

  return is_allocated;
}

/* Returns number of allocated bytes in the heap using the given map */
size_t
num_allocated_bytes (alloc_map_t alloc_map)
{
  size_t allocated_bits = 0;
  size_t num_bytes = get_size_allocation_map (alloc_map);
  unsigned int *map = alloc_map;
  size_t bytes_handled = 0;
  for (size_t int_index = sizeof (size_t); int_index < num_bytes;
       int_index += sizeof (unsigned int))
    {
      allocated_bits += __builtin_popcount (map[int_index]);
      bytes_handled = int_index;
    }
  for (size_t char_index = bytes_handled; char_index < num_bytes; char_index++)
    {
      unsigned char byte = alloc_map[char_index];
      allocated_bits += __builtin_popcount ((unsigned int)byte);
    }
  return allocated_bits * ALLOCATION_MAP_DENSITY;
}

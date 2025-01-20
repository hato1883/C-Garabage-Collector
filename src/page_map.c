/**
 * Function for creating a page map.
 * The page map represents status of a page in the heap,
 * using a single boolean for each page.
 * This results in each boolean representing 2 KB on the heap, and
 * each byte of the allocation map represents 8 booleans and therefore 16 KB in
 * the heap
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "heap_internal.h"
#include "page_map.h"

size_t
calc_required_map_size (size_t bytes)
{
  return (size_t)ceil ((double_t)bytes / PAGE_DENSITY);
}

/* Creates an allocation map of a given amount of bytes. This size needs to be
 * atleast the MIN_OBJECT_SIZE and divisible evenly with BOOLEANS_PER_BYTE.  */
char *
create_page_map (size_t bytes)
{
  size_t map_size = calc_required_map_size (bytes);

  /* Creation of allocation map  */
  char *page_map = malloc (map_size * sizeof (char));
  assert (page_map != NULL && "Malloc failed to allocate page map");
  reset_page_map (page_map, map_size);

  return page_map;
}

void
reset_page_map (char *page_map, size_t map_size)
{
  /* Set all bits to 1 (movable) */
  memset (page_map, 0xFF, map_size);
}

size_t
find_index_in_page_map (size_t offset)
{
  /* Each byte represents 8*2048 = 16 KB on the heap  */
  return offset / PAGE_DENSITY;
}

/* Function for creating a bit mask for a specific bit in a byte */
char
create_page_bitmask (size_t offset)
{
  /* Find the specific bit within a byte  */
  size_t bit_position = (offset / PAGE_SIZE) % ALLOCATIONS_PER_BYTE;
  /* Set the specific bit*/
  return 1 << bit_position;
}

void
update_page_map (char *page_map, size_t offset, bool is_movable)
{
  size_t page_index = find_index_in_page_map (offset);
  char bitmask = create_page_bitmask (offset);
  char byte = page_map[page_index];

  if (is_movable)
    {
      /* Sets the target bit to 1 using the OR operator.  */
      byte = byte | bitmask;
    }
  else
    {
      /* Sets the target bit to 0 using the AND operator with the negated
         bitmask.  */
      bitmask = ~bitmask; /* Negates the bitmask.  */
      byte = byte & bitmask;
    }

  page_map[page_index] = byte;
}

bool
is_offset_movable (char *page_map, size_t offset)
{
  size_t page_index = find_index_in_page_map (offset);
  char bitmask = create_page_bitmask (offset);
  char byte = page_map[page_index];
  bool is_movable = (byte & bitmask) == bitmask;
  return is_movable;
}

size_t
bytes_from_next_page (size_t page_size, size_t offset)
{
  return page_size - (offset % page_size);
}

void
h_reset_page_map (heap_t *h)
{
  size_t map_size = calc_required_map_size (h->size);
  reset_page_map (h->page_map, map_size);
}
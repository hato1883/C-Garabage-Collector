/**
 * Contains the functions for moving data from one address to another.
 */

#include <string.h>

#include "format_encoding.h"
#include "get_header.h"
#include "move_data.h"

/**
 * Moves an allocation that has a header which is a bitvector.
 * @param header the header
 * @param origin the allocation origin
 * @param destination the target address to move the allocation to.
 */
void
move_bitvector (header_t header, void **origin, void *destination)
{
  size_t bitvector_size = size_from_vector (header >> BITS_FOR_FORMAT_TYPE);
  size_t header_size = sizeof (header_t);

  char *header_and_alloc = (char *)*origin - header_size;
  char *dest = ((char *)destination - header_size);

  memmove ((void *)dest, (void *)header_and_alloc,
           (bitvector_size + header_size));
  *origin = destination;
}

/**
 * Moves an allocation that has a header which is a pointer to format string.
 * @param header the header
 * @param origin the allocation origin
 * @param destination the target address to move the allocation to.
 */
void
move_ptr_to_formatstring (header_t header, void **origin, void *destination)
{
  char *format_str = (char *)(clear_formating_encoding (header));

  bool success = false;
  size_t alloc_size = size_from_string (format_str, &success);
  assert (success);

  size_t header_size = sizeof (header_t);
  char *header_and_alloc = (char *)*origin - header_size;
  char *dest = ((char *)destination - header_size);

  memmove ((void *)dest, (void *)header_and_alloc, (alloc_size + header_size));
  *origin = destination;
}

bool
move_alloc (void **alloc, void *destination)
{
  header_t *header_ptr = get_header_pointer (*alloc);
  header_t header = get_header_value (header_ptr);
  bool move_success = false;
  header_t header_type = get_header_type (header);
  if (header_type == HEADER_BIT_VECTOR)
    {
      move_bitvector (header, alloc, destination);
      move_success = true;
    }
  else if (header_type == HEADER_POINTER_TO_FORMAT_STRING)
    {
      move_ptr_to_formatstring (header, alloc, destination);
      move_success = true;
    }
  if (move_success)
    {
      /* Replace header with forwarding address. */
      uintptr_t new_dest
          = set_header_forwarding_address ((uintptr_t)destination);
      *header_ptr = (header_t)new_dest;
    }

  return move_success;
}

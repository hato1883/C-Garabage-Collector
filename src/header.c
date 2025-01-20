/**
 * Functions for creating different headers.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "format_encoding.h"
#include "get_header.h"
#include "header.h"

/**
 * Helper function that creates a header containing a bit vector
 */
static header_t
create_header_vector (char *format_string, size_t size, bool *success)
{
  return set_header_bit_vector (
      convert_to_bit_vector (format_string, size, success)
      << BITS_FOR_HEADER_TYPE);
}

header_t
create_header_struct (char *format_string, size_t size, bool *success)
{
  header_t header = create_header_vector (format_string, size, success);

  if (!(*success))
    {
      /* Just to show that the format string should be saved. This pointer will
         not be used later, since the format string needs to be copied into the
         heap.  */
      *success = true;
      header = set_header_pointer_to_format_string ((header_t)format_string);
    }

  return header;
}

header_t
create_header_struct_unknown_size (char *format_string, bool *success)
{
  uint64_t size = size_from_string (format_string, success);

  if (!(*success))
    return 0;

  return create_header_struct (format_string, size, success);
}

/**
 * Creates a header from a size.
 */
header_t
create_header_raw (size_t size, bool *success)
{
  return create_header_vector ("", size, success);
}

/* Currently does not have support for format string in header.  */
// TODO: Fix support to format string pointers
ptr_queue_t *
get_pointers_in_allocation (void *allocation_start)
{
  header_t *header_p = get_header_pointer (allocation_start);

  if (!header_p)
    {
      return create_ptr_queue ();
    }

  header_t header = get_header_value (header_p);

  header_type_t type = get_header_type (header);
  if (type == HEADER_BIT_VECTOR)
    {
      uint_fast64_t vector = header >> BITS_FOR_HEADER_TYPE;
      return get_pointers_from_bit_vector (vector, allocation_start);
    }
  else if (type == HEADER_POINTER_TO_FORMAT_STRING)
    {
      // FIXME: currently dose not support pointer to forward strings...
      header_t header_val = get_header_value(header_p);
      char *format_string = get_pointer_in_header(header_val);
      return get_pointers_from_format_string (format_string, allocation_start);
    }
  else
    {
        return create_ptr_queue ();
    }
    
}

/**
 * Contains functions: to get header, and also extracting the type of the
 * header. Also updating the header. Gets the headertype from a
 * header_t. The header data is contained in the 0x0 - 0x3 of a piece of
 * memory. Also includes functions for updating the header.
 */

#include <assert.h>

#include "format_encoding.h"
#include "get_header.h"
#include "header.h"
#include "heap_internal.h"

/* Gets the pointer to the header for a given pointer.
 * Note that currently no checks are being run to make sure this piece of
 * memory actually has a header among other important checks.
 * @param binary_string - a binary string.
 * @return the pointer to (hopefully) a header  */
header_t *
get_header_pointer (void *memory_pointer)
{
  assert (memory_pointer != NULL);
  /* Get the pointer to the header, which is before the pointer of a piece of
   * allocated memory.  */
  header_t *header_pointer
      = (header_t *)((size_t)memory_pointer - sizeof (header_t));
  return header_pointer;
}

/* From a header_pointer, get a copy of the header.  */
header_t
get_header_value (header_t *header_pointer)
{
  assert (header_pointer != NULL);
  header_t the_header = *header_pointer;
  return the_header;
}

/* Mask out the two least significant bits. This is the header type.  */
header_type_t
extract_header_type (header_t the_header)
{
  return mask_formating_encoding (the_header);
}

/* Updates the header of a pointer to a new header.
 * @param pointer - a pointer
 * @param new_header - the new header to put before the pointer  */
void
update_header_of_a_pointer (void *pointer, header_t new_header)
{
  header_t *header_pointer = get_header_pointer (pointer);
  *header_pointer = new_header;
}

/* Updates the headertypes of a header.
 * @param binary_string - a binary string
 * @param new_header - the new header, must be 0-3;
 * @return an updated binary string with the new header.  */
header_t
set_header_type (header_t the_header, header_type_t new_header_type)
{
  assert (new_header_type <= (MASK_FOR_FORMAT_TYPE) && new_header_type >= 0);
  uint_fast64_t mask = ~((uint_fast64_t)MASK_FOR_FORMAT_TYPE);
  the_header = the_header & mask;
  the_header = the_header | new_header_type;
  return the_header;
}

/* Returns a header with updated headertype: HEADER_POINTER_TO_FORMAT_STRING */
header_t
set_header_pointer_to_format_string (header_t the_header)
{
  return set_header_type (the_header, HEADER_POINTER_TO_FORMAT_STRING);
}

/* Returns a the binary string with updated header, HEADER_FORWARDING_ADDRESS.
 */
header_t
set_header_forwarding_address (header_t the_header)
{
  return set_header_type (the_header, HEADER_FORWARDING_ADDRESS);
}

/* Returns a the binary string with updated header, HEADER_UNUSED.  */
header_t
set_header_unused (header_t the_header)
{
  return set_header_type (the_header, HEADER_UNUSED);
}

/* Returns a new binary string with updated header, HEADER_BIT_VECTOR  */
header_t
set_header_bit_vector (header_t the_header)
{
  return set_header_type (the_header, HEADER_BIT_VECTOR);
}

header_t
change_pointer_in_header (header_t header, void *p)
{
  /* Sets all but bits 1-0 to 0 and puts pointer there.  */
  return (header & (uint_fast64_t)MASK_FOR_FORMAT_TYPE) | (header_t)p;
}

void *
get_pointer_in_header (header_t header)
{
  /* Sets bits 1-0 to 0.  */
  return (void *)(header & ~((uint_fast64_t)MASK_FOR_FORMAT_TYPE));
}

/* Given a binary string we extract the last two bits which combinations
 * indicate a certain headertype.  */
header_type_t
get_header_type (header_t the_header)
{
  /* 4 different possible combinations. */
  switch (extract_header_type (the_header))
    {
    case HEADER_POINTER_TO_FORMAT_STRING: /* 0x0 (00)  */
      return HEADER_POINTER_TO_FORMAT_STRING;
    case HEADER_FORWARDING_ADDRESS: /* 0x1 (01)  */
      return HEADER_FORWARDING_ADDRESS;
    case HEADER_UNUSED: /* 0x2 (10)  */
      return HEADER_UNUSED;
    case HEADER_BIT_VECTOR: /* 0x3 (11)  */
      return HEADER_BIT_VECTOR;
    default: /* impossible to reach  */
      /* Crash with message after "&&" */
      assert (0 && "header mask resulted in number larger than [0, 3]");
    }
}

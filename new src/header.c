/**
 * Functions for creating different headers.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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
  return the_header & MASK_FOR_HEADER_TYPE;
}

/* Set bit 0 & 1 to 00 */
header_type_t
remove_header_type (header_t the_header)
{
  return the_header & ~MASK_FOR_HEADER_TYPE;
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
  assert (new_header_type <= (MASK_FOR_HEADER_TYPE) && new_header_type >= 0);
  the_header = remove_header_type (the_header) | new_header_type;
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

/* Returns a the binary string with updated header, HEADER_SIZE.  */
header_t
set_header_size (header_t the_header)
{
  return set_header_type (the_header, HEADER_SIZE);
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
  return (header & (uint64_t)MASK_FOR_HEADER_TYPE) | (header_t)p;
}

void *
get_pointer_in_header (header_t header)
{
  /* Sets bits 1-0 to 0.  */
  return (void *)(header & ~((uint64_t)MASK_FOR_HEADER_TYPE));
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
    case HEADER_SIZE: /* 0x2 (10)  */
      return HEADER_SIZE;
    case HEADER_BIT_VECTOR: /* 0x3 (11)  */
      return HEADER_BIT_VECTOR;
    default: /* impossible to reach  */
      /* Crash with message after "&&" */
      assert (0 && "header mask resulted in number larger than [0, 3]");
    }
}

/**
 * @brief Creates appropriate header to represent size given from format
 * string. If string dose not contain pointers it will be saved as a number, If
 * it has pointers but can be represented by a bitvector it will, otherwise we
 * will save string in the heap and give a pointer to the header.
 * It will fail if the format string has a invalid format or character.
 * @param heap To allocate the string in if needed
 * @param format string to convert into a header
 * @param success Flag to indicate if conversion was successful.
 * @return A Header with format encoded in one of the 3 possible ways.
 */
header_t
convert_struct_to_header (heap_t *heap, char *format, bool *success)
{
  // TODO: Validate format string
  /* Checks if format has pointers */
  if (!contains_pointers (format))
    { /* if not, use different encoding that is easier */
      /* size need to fit in vector.  */
      size_t size = size_from_string (format, success);
      return set_header_size (size << BITS_FOR_HEADER_TYPE);
    }
  /* Has pointers, attempt bitvector */
  bitvector_t vector = convert_format_to_bit_vector (format, success);
  if (*success)
    { /* fits in vector */
      return set_header_bit_vector (vector);
    }
  else
    { /* did not fit in vector, use pointer to this string */
      void *format_alloc = h_alloc_raw (heap, strlen (format) + 1);
      char *ptr = format_alloc;
      while (*format)
        {
          *ptr = *format;
          ptr++;
          format++;
        }
      *success = true;
      return set_header_pointer_to_format_string (format_alloc);
    }
}

header_t
create_header_raw (size_t size, bool *success)
{
  *success = size >= SIZE_MAX >> BITS_FOR_HEADER_TYPE;
  return set_header_size (size << BITS_FOR_HEADER_TYPE);
}

size_t
size_from_header (header_t header)
{
  header_type_t type = get_header_type (header);
  /* To ensure format encoding is not read as part of the vector.  */
  bitvector_t vector = remove_header_type (header);

  if (type == HEADER_SIZE)
    { /* size is set in header as plain number shifted by 2 to the left */
      return vector >> 2;
    }
  else if (type == HEADER_FORWARDING_ADDRESS)
    { /* Invalid type */
      return 0;
    }
  else if (type == HEADER_POINTER_TO_FORMAT_STRING)
    { /* TODO: fix support */
      char **format_string = get_header_pointer (header);
      bool tmp;
      return size_from_string (*format_string, &tmp);
    }
  else if (type == HEADER_BIT_VECTOR)
    {
      return size_from_vector (header);
    }
}

ptr_queue_t *
get_pointers_from_allocation (void *alloc)
{
  header_t *head_ptr = get_header_pointer (alloc);
  header_t head = get_header_value (alloc);
  header_type_t head_type = get_header_type (head);
  switch (head_type)
    {
    case HEADER_BIT_VECTOR:
      bitvector_t vector = remove_header_type (head);
      return get_pointers_from_bit_vector (vector, alloc);

    case HEADER_POINTER_TO_FORMAT_STRING:
      char **format_string = get_pointer_in_header (head);
      return get_pointers_from_format_string (*format_string, alloc);

    case HEADER_FORWARDING_ADDRESS:
      assert (true && "Can not retrieve pointers from a forwarded address!");
      return create_ptr_queue (); /* dead code */

    default: /* alloc dose not contain pointers */
      return create_ptr_queue ();
    }
}
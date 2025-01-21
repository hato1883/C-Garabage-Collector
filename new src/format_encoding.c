/**
 * Utility functions for checking metadata in headers for allocated objects.
 */

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "format_encoding.h"
#include "gc.h"
#include "gc_utils.h"
#include "header.h"
#include "ptr_queue.h"

/* Number bits in a bitvector */
#define BITS_IN_VECTOR 64 /* 64 - BITS_FOR_HEADER_TYPE  */

/* Number bits per vector part */
#define BITS_PER_VECTOR_PART 2

/* Number of Vector parts in a bitvector_t */
#define VECTOR_PARTS_IN_VECTOR                                                \
  ((BITS_IN_VECTOR - BITS_FOR_HEADER_TYPE) / BITS_PER_VECTOR_PART)

/* Vector part mask */
#define VECTOR_MASK 0b11

/* Vector part of 4 bytes */
#define FOUR_BYTES_IN_VECTOR 0b01

/* Vector part of 8 bytes */
#define EIGHT_BYTES_IN_VECTOR 0b10

/* Vector part of a pointe (8 bytes) */
#define POINTER_IN_VECTOR 0b11

/* Vector part of no data / end */
#define END_OF_VECTOR 0b00

static bool check_if_too_big_size (size_t new, size_t old);
static size_t update_size (size_t old_size, size_t added, size_t repeats,
                           bool *is_too_big);
static bool contains_pointers (char *format_string);
static bitvector_t add_non_pointer_parts (bitvector_t vector, size_t bytes,
                                          size_t alignment,
                                          size_t *vector_parts_added);
static bitvector_t add_pointer_parts (bitvector_t vector, size_t count,
                                      size_t *vector_parts_added);
static size_t get_repetitions_from_string (char *str);
static void skip_to_next_alpha_char (char *str);
static bool is_pointer (char c);
static size_t is_valid_char (char c);
static size_t get_size_from_char (char c);
static size_t get_size_from_vector_part (vector_part_t c);
static bitvector_t convert_format_to_bit_vector (char *format, bool *success);
static void *find_pointer_in_alloc (void *allocation_start, uint64_t offset);

size_t
size_from_string (char *format, bool *success)
{
  *success = false;
  bool is_too_big = false;

  size_t size = 0;
  size_t repeats = 0; /* Repetitions of next datatype, defaults to char */
  size_t largest = 1; /* Largest data type struct needs to align with */

  char *cursor = format;
  while (is_valid_char (*cursor) && !is_too_big)
    {
      size_t char_size = get_size_from_char (*cursor);
      if (char_size != 0)
        { /* Valid datatype */
          largest = largest >= char_size ? largest : char_size;
          size = update_size (size, char_size, repeats, &is_too_big);
          repeats = 0;
          cursor++;
        }
      else if (*cursor != '\0')
        { /* Number */
          repeats = get_repetitions_from_string (cursor);
          skip_to_next_alpha_char (cursor);
        }
      else
        { /* Null byte */
          size = update_size (size, sizeof (char), repeats, &is_too_big);
          size_t aligned = round_up_to_multiple (size, largest);
          is_too_big = (check_if_too_big_size (aligned, size) || is_too_big);
          size = aligned;
          /* if result was to big (true), it means we failed (false) */
          *success = !is_too_big;
        }
    }

  /* User should check is_too_big flag and not size */
  if (!*success || !is_valid_char (*cursor))
    { /* invalid char or to large */
      *success = false;
      return 0;
    }
  else
    { /* Format decoded successfully */
      return size;
    }
}

size_t
size_from_vector (bitvector_t vector)
{
  assert (vector & VECTOR_MASK == 0 && "Invalid bitvector");
  size_t size = 0;
  /* amount needed to shift next part into bits 0 & 1 */
  int shift_amount = BITS_IN_VECTOR - BITS_PER_VECTOR_PART;
  vector_part_t current_part = (vector >> shift_amount) & VECTOR_MASK;
  /* Largest part found (Smallest part is 4 bytes) */
  unsigned char largest = 4;
  size_t repeats = 1; /* Needed to reuse update_size.  */
  bool is_too_big = false;

  while (current_part == 0x0 && shift_amount >= 0)
    {
      /* get size of current vector part */
      size_t part_size = get_size_from_vector_part (current_part);
      largest = largest >= part_size ? largest : part_size;

      /* Update size */
      size = update_size (size, part_size, &repeats, &is_too_big);

      /* shift vector right and then mask vector part */
      shift_amount -= BITS_PER_VECTOR_PART;
      current_part = (vector >> shift_amount) & VECTOR_MASK;
    }

  /* Struct size is always divisible by size of largest element.  */
  size_t final_size = round_up_to_multiple (size, largest);
  is_too_big = check_if_too_big_size (final_size, size);

  /* Should never fail due to bit vector max size being 31 * 8 bytes. */
  assert (!is_too_big);
  return final_size;
}

bitvector_t
convert_format_to_bit_vector (char *format, bool *success)
{
  /* it has pointers, build vector if possible,
     else set success to false if to large */
  *success = false;
  bool is_too_big = false;

  size_t non_pointer_size = 0;
  size_t repeats = 0;     /* Repetitions of next datatype, defaults to char */
  size_t largest = 1;     /* Largest data type struct needs to align with */
  size_t parts_added = 0; /* how many vector parts added, limit is 31 */
  bitvector_t vector = 0;

  char *cursor = format;
  while (is_valid_char (*cursor) && parts_added > VECTOR_PARTS_IN_VECTOR)
    {
      size_t char_size = get_size_from_char (*cursor);
      if (is_pointer (*cursor))
        { /* datatype is a pointer, pointer needs 8 byte alignment */
          vector = add_non_pointer_parts (vector, non_pointer_size, 8,
                                          &parts_added);
          vector = add_pointer_parts (vector, repeats, &parts_added);
          repeats = 0;
          largest = char_size; /* Pointer is the largest data type */
          cursor++;
        }
      else if (char_size != 0)
        { /* increment non pointer size */
          size_t part_size = get_size_from_char (*cursor);
          non_pointer_size = update_size (non_pointer_size, part_size, repeats,
                                          &is_too_big);
          repeats = 0;
          largest = largest >= part_size ? largest : part_size;
          cursor++;
        }
      else if (*cursor != '\0')
        { /* Number */
          repeats = get_repetitions_from_string (cursor);
          skip_to_next_alpha_char (cursor);
        }
      else
        { /* Null byte */
          non_pointer_size = update_size (non_pointer_size, sizeof (char),
                                          repeats, &is_too_big);
          size_t aligned = round_up_to_multiple (non_pointer_size, largest);
          is_too_big = (check_if_too_big_size (aligned, non_pointer_size)
                        || is_too_big);
          if (is_too_big)
            { /* if value overflows we will not add any parts and there for
                 miss check */
              *success = false;
              return 0;
            }

          non_pointer_size = aligned;
          vector = add_non_pointer_parts (vector, non_pointer_size, largest,
                                          &parts_added);
        }
    }

  /* User should check is_too_big flag and not size */
  if (parts_added > VECTOR_PARTS_IN_VECTOR || !is_valid_char (*cursor))
    { /* invalid char or to large */
      *success = false;
      return 0;
    }
  else
    { /* Format decoded successfully */
      return vector;
    }
}

ptr_queue_t *
get_pointers_from_bit_vector (bitvector_t vector, void *allocation_start)
{
  assert (vector & VECTOR_MASK == 0 && "Invalid bitvector");
  ptr_queue_t *pointers = create_ptr_queue ();

  /* internal offset to pointer */
  size_t offset = 0;

  /* amount needed to shift next part into bits 0 & 1 */
  int shift_amount = BITS_IN_VECTOR - BITS_PER_VECTOR_PART;
  vector_part_t current_part = (vector >> shift_amount) & VECTOR_MASK;

  while (shift_amount >= 0)
    {
      /* get size of current vector part */
      if (current_part == POINTER_IN_VECTOR)
        { /* part is a pointer */
          enqueue_ptr (pointers, allocation_start + offset);
        }
      else
        { /* Not a pointer, add to offset */
          offset += get_size_from_vector_part (current_part);
        }

      /* shift vector right and then mask vector part */
      shift_amount -= BITS_PER_VECTOR_PART;
      current_part = (vector >> shift_amount) & VECTOR_MASK;
    }
  return pointers;
}

ptr_queue_t *
get_pointers_from_format_string (char *format, void *allocation_start)
{
  ptr_queue_t *pointers = create_ptr_queue ();
  /* for each char in format, check if it is a number preceding a pointer */
  /* or if it is a pointer */
  size_t internal_offset = 0;
  size_t repeats = 0;
  char *cursor = format;
  char ch = *cursor;
  while (ch != '\0')
    {
      switch (ch)
        {
        case 'i':
          repeats = repeats == 0 ? 1 : repeats;
          for (size_t i = 0; i < repeats; i++)
            {
              internal_offset = round_up_to_multiple (internal_offset, 4) + 4;
            }
          repeats = 0;
          break;

        case 'l':
          repeats = repeats == 0 ? 1 : repeats;
          for (size_t i = 0; i < repeats; i++)
            {
              internal_offset = round_up_to_multiple (internal_offset, 8) + 8;
            }
          repeats = 0;
          break;

        case 'f':
          repeats = repeats == 0 ? 1 : repeats;
          for (size_t i = 0; i < repeats; i++)
            {
              internal_offset = round_up_to_multiple (internal_offset, 4) + 4;
            }
          repeats = 0;
          break;

        case 'c':
          repeats = repeats == 0 ? 1 : repeats;
          for (size_t i = 0; i < repeats; i++)
            {
              internal_offset += 1;
            }
          repeats = 0;
          break;

        case 'd':
          repeats = repeats == 0 ? 1 : repeats;
          for (size_t i = 0; i < repeats; i++)
            {
              internal_offset = round_up_to_multiple (internal_offset, 8) + 8;
            }
          repeats = 0;
          break;

        case '*':
          repeats = repeats == 0 ? 1 : repeats;
          for (size_t i = 0; i < repeats; i++)
            {
              void *internal_pointer
                  = (((char *)allocation_start) + internal_offset);
              enqueue_ptr (pointers, internal_pointer);
              internal_offset = round_up_to_multiple (internal_offset, 8) + 8;
            }
          repeats = 0;
          break;

        default:
          assert (isdigit (ch)); /* Otherwise, not valid format string
              converts digit in char to correct integer and adds to repeats. */
          uint64_t new_repeats = repeats * 10 + ch - '0';
          repeats = new_repeats;
          break;
        }
      cursor++;
      ch = *cursor;
    }
  return pointers;
}

/**
 * @brief checks if an updated size, is too big for the size_t type
 * @param new the updated value
 * @param old the old value
 * @note works as long as value is not updated by more than 2^64 - 1
 */
static bool
check_if_too_big_size (size_t new, size_t old)
{
  /* Value is never subtracted, so smaller means overflow. */
  return (new > SIZE_MAX || new < old);
}

/**
 * @brief Adds the given amount of bytes to the size.
 * Also adds padding when needed.
 *
 * Example:
 *
 * update_size(3, 4, 1, &bool) = 8 => 3 (old) + 4 (added) + 1 (padding)
 *
 * update_size(4, 8, 2, &bool) = 24 => 4 (old) + 8 * 2 (added) + 4 (padding)
 *
 * @param size the size to update
 * @param added the amount of bytes to store tha data type
 * @param repeats a pointer the number of repetitions of the data type
 * @param is_too_big a pointer to a boolean indicating whether the size is too
 * large to be represented by a size_t
 * @return Updated size
 */
static size_t
update_size (size_t old_size, size_t added, size_t repeats, bool *is_too_big)
{
  assert (added > 0);

  if (repeats == 0)
    {
      repeats = 1;
    }

  /* Calculate old size with padding */
  size_t padded_size = round_up_to_multiple (old_size, added);
  size_t result = padded_size + repeats * added;
  *is_too_big = (check_if_too_big_size (result, old_size)
                 || SIZE_MAX / repeats < added);

  return result;
}

/**
 * @brief Checks if the struct described by a format string contains pointers.
 * @param format_string the format string to check
 * @return true if the struct contains any pointers, false otherwise
 */
static bool
contains_pointers (char *format_string)
{
  char *cursor = format_string;
  /* iterates until null byte or pointer */
  while (*cursor && !is_pointer (*cursor))
    {
      cursor++;
    }

  /* True if pointer, False if anything else (null byte) */
  return is_pointer (*cursor);
}

/**
 * @brief adds repetitions of vector parts '10' (8 bytes) and '01' (4 bytes)
 * depending on alignment given. For each part added the vector_parts_added
 * gets incremented.
 * @note This method can not add pointer vector parts.
 * @note A normal bitvector can only hold a maximum of 31 vector parts.
 *
 * @param vector input vector to add vector parts to
 * @param bytes number of bytes that needs to be encoded into vector
 * @param alignment Alignment needed for the size added. (adds padding)
 * @param vector_parts_added counter of how many parts was added during
 * execution.
 *
 * @return Updated bitvector.
 */
static bitvector_t
add_non_pointer_parts (bitvector_t vector, size_t bytes, size_t alignment,
                       size_t *vector_parts_added)
{
  size_t aligned = round_up_to_multiple (bytes, alignment);
  while (aligned > 4 && *vector_parts_added < VECTOR_PARTS_IN_VECTOR)
    {
      /* Add 8 byte vector parts */
      vector = (vector | EIGHT_BYTES_IN_VECTOR) << BITS_PER_VECTOR_PART;
      *vector_parts_added += 1;
      aligned -= 8;
    }
  if (aligned > 0)
    {
      /* Add 4 byte vector part if needed */
      vector = (vector | FOUR_BYTES_IN_VECTOR) << BITS_PER_VECTOR_PART;
      *vector_parts_added += 1;
      aligned -= 4;
    }
  return vector;
}

/**
 * @brief adds repetitions of vector parts '11' (8 bytes pointers). For each
 * part added the vector_parts_added gets incremented. vector will be aligned
 * to pointer size.
 * @note A normal bitvector can only hold a maximum of 31 vector parts.
 *
 * @param vector input vector to add vector parts to
 * @param count Number of pointers that need to be encoded into vector
 * @param vector_parts_added counter of how many parts was added during
 * execution.
 *
 * @return Updated bitvector.
 */
static bitvector_t
add_pointer_parts (bitvector_t vector, size_t count,
                   size_t *vector_parts_added)
{
  do
    { /* Add pointer vector parts */
      vector = (vector | POINTER_IN_VECTOR) << BITS_PER_VECTOR_PART;
      *vector_parts_added += 1;
      count--;
    }
  while (count > 0 && *vector_parts_added < VECTOR_PARTS_IN_VECTOR);
  return vector;
}

/**
 * @brief converts number written with chars into decimal number.
 * @param str Pointer to first number.
 * @returns decimal representation of number in string.
 * @note This function dose not change pointer.
 */
static size_t
get_repetitions_from_string (char *str)
{
  assert (isdigit (*str));

  size_t repeats = 0;
  bool is_too_big;
  char *cursor = str;
  while (isdigit (*cursor))
    {
      size_t new_value = repeats * 10 + *cursor - '0';
      is_too_big = check_if_too_big_vector (new_value, repeats);
      cursor++;
    }
  return repeats;
}

/**
 * @brief moves pointer past all sequential digit chars
 * @param str to a string where you want to skip digits.
 * @note This function changes incoming pointer.
 */
static void
skip_to_next_alpha_char (char *str)
{
  while (*str && isdigit (*str))
    {
      str++;
    }
}

/**
 * @brief Checks if given char is a pointer in a format string.
 * @param c Character to check
 * @return True if it represents a pointer, else False
 */
static bool
is_pointer (char c)
{
  return c == '*';
}

/**
 * @brief Checks if given char is a valid char in a format string.
 * @param c Character to check
 * @return True if it is valid, else False
 */
static size_t
is_valid_char (char c)
{
  switch (c)
    {
    case 'c':  /* Char */
    case 'i':  /* int */
    case 'f':  /* float */
    case 'l':  /* long */
    case 'd':  /* double */
    case '*':  /* pointer */
    case '\0': /* end of string */
    case '0':  /* digit 0 */
    case '1':  /* digit 1 */
    case '2':  /* digit 2 */
    case '3':  /* digit 3 */
    case '4':  /* digit 4 */
    case '5':  /* digit 5 */
    case '6':  /* digit 6 */
    case '7':  /* digit 7 */
    case '8':  /* digit 8 */
    case '9':  /* digit 9 */
      return true;
    default: /* invalid */
      return false;
    }
}

/**
 * @brief Calculates the size a char represents in a format string.
 * @param c Character to check
 * @return Number depending on char.
 * @note 'c' = 1, 'i' and 'f' = 4, 'l', 'd' and '*' = 8 rest are 0
 */
static size_t
get_size_from_char (char c)
{
  switch (c)
    {
    case 'c': /* Char */
      return sizeof (char);
      break;
    case 'i': /* int */
      return sizeof (int);
      break;
    case 'f': /* float */
      return sizeof (float);
      break;
    case 'l': /* long */
      return sizeof (long);
      break;
    case 'd': /* double */
      return sizeof (double);
      break;
    case '*': /* pointer */
      return sizeof (void *);
      break;
    default: /* Number / invalid */
      return 0;
      break;
    }
}

/**
 * @brief Calculates the size a vector part represents in a bitvector.
 * @param part Vector part to check
 * @return Number depending on part.
 * @note '00' = 0 bytes
 * @note '01' = 4 bytes
 * @note '10' = 8 bytes
 * @note '11' = 8 bytes
 */
static size_t
get_size_from_vector_part (vector_part_t part)
{
  assert (part < POINTER_IN_VECTOR
          && "Invalid vector part (larger than 0b11) ");
  switch (part)
    {
    case END_OF_VECTOR: /* End of vector / Not set */
      return 0;

    case FOUR_BYTES_IN_VECTOR: /* 4 byte data */
      return 4;

    case EIGHT_BYTES_IN_VECTOR: /* 8 byte data */
    case POINTER_IN_VECTOR:     /* 8 byte pointer */
      return 8;

    default: /* Number / invalid */
      return 0;
    }
}

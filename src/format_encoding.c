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
#include "ptr_queue.h"

#define BITS_IN_VECTOR 62 /* 64 - BITS_FOR_HEADER_TYPE  */
#define BITS_PER_VECTOR_PART 2
#define LAST_VECTOR_PART 0x3
#define FIRST_VECTOR_PART 0x3000000000000000 /* Bits 61-60 */
#define FOUR_BYTES_IN_VECTOR 0x1
#define EIGHT_BYTES_IN_VECTOR 0x2
#define POINTER_IN_VECTOR 0x3
#define END_OF_VECTOR 0x0

/**
 * @brief checks if an updated size, or number of repetisions, is too big for
 * the size_t type
 * @param new the updated value
 * @param old the old value
 * @note works as long as value is not updated by more than 2^64 - 1
 */
bool
check_if_too_big_size (uint_fast64_t new, uint_fast64_t old)
{
  if (new > SIZE_MAX
      || new < old) /* Value is never subtracted, so smaller means overflow. */
    {
      return true;
    }
  return false;
}

/**
 * @brief adds an amount of bytes, for an amount of repetitions, to a size
 * variable. Also adds padding. As a side effect, updates variables that keep
 * track of the number of repetitions of the data type (always reset to 0), the
 * largest data type to have been added and whether the value being added is
 * too big for a size_t to represent.
 * @param size the size to update
 * @param amount the amount of bytes to store tha data type
 * @param repeats a pointer the number of repetitions of the data type
 * @param largest a pointer to a variable keeping track of the largest data
 * type to have been added
 * @param is_too_big a pointer to a boolean indicating whether the size is too
 * large to be represented by a size_t
 * @return the updated size
 */
static uint_fast64_t
update_size (uint_fast64_t size, uint_fast64_t amount, uint_fast64_t *repeats,
             uint_fast8_t *largest, bool *is_too_big)
{
  assert (amount > 0);

  if (*repeats == 0)
    {
      *repeats = 1;
    }

  uint_fast8_t padding = (amount - size % amount) % amount;
  uint_fast64_t result = size + padding + *repeats * amount;
  /* Also catches the case where *repeats * amount >= SIZE_MAX.  */
  *is_too_big = check_if_too_big_size (result, size) || (result == size)
                || SIZE_MAX / *repeats < amount;

  *largest = amount > *largest ? amount : *largest;
  *repeats = 0;

  return result;
}

uint_fast64_t
size_from_string (char *format, bool *success)
{
  char *cursor = format;
  uint_fast64_t size = 0;
  uint_fast64_t repeats = 0;
  uint_fast8_t largest = 1;
  bool is_too_big = false;

  while (!is_too_big)
    {
      switch (*cursor)
        {
        case 'i':
          size = update_size (size, sizeof (int), &repeats, &largest,
                              &is_too_big);
          break;

        case 'l':
          size = update_size (size, sizeof (long), &repeats, &largest,
                              &is_too_big);
          break;

        case 'f':
          size = update_size (size, sizeof (float), &repeats, &largest,
                              &is_too_big);
          break;

        case 'c':
          size = update_size (size, sizeof (char), &repeats, &largest,
                              &is_too_big);
          break;

        case 'd':
          size = update_size (size, sizeof (double), &repeats, &largest,
                              &is_too_big);
          break;

        case '*':
          size = update_size (size, sizeof (void *), &repeats, &largest,
                              &is_too_big);
          break;

        case '\0':
          /* For format strings ending with numbers.  */
          if (repeats > 0)
            {
              size = update_size (size, sizeof (char), &repeats, &largest,
                                  &is_too_big);
            }
          if (is_too_big)
            break;

          /* Struct size is always divisible by size of largest element.  */
          uint_fast64_t final_size
              = size + (largest - size % largest) % largest;

          /* Check if size is to big after padding  */
          is_too_big = check_if_too_big_size (final_size, size);

          if (is_too_big)
            break;

          *success = true;
          return final_size;

        default:
          assert (isdigit (*cursor)); /* Otherwise, not valid format string
            converts digit in char to correct integer and adds to repeats.  */
          uint_fast64_t new_repeats = repeats * 10 + *cursor - '0';

          is_too_big = check_if_too_big_size (new_repeats, repeats);
          repeats = new_repeats;

          break;
        }

      cursor++;
    }
  /* only reached if size exceeded maximum.  */
  *success = false;
  return 0;
}

/* calculates the size of a struct from a bit vector (and accounts for padding)
   due to the size of bit vector - can't be unsuccessful?  */

uint_fast64_t
size_from_vector (uint_fast64_t vector)
{
  enum FormatType type = get_format_type (vector);

  /* To ensure format encoding is not read as part of the vector.  */
  vector = remove_formating_encoding (vector);

  if (type == FORMAT_SIZE_NO_POINTERS || type == FORMAT_SIZE_HAS_POINTERS)
    {
      return vector;
    }
  else if (type != FORMAT_VECTOR)
    {
      return 0;
    }

  uint_fast64_t size = 0;
  /* Sets current part to first vector part, moved to LSB for easier
   * operations.  */
  uint_fast64_t current_part = ((vector & FIRST_VECTOR_PART))
                               >> (BITS_IN_VECTOR - BITS_PER_VECTOR_PART);
  uint_fast8_t largest
      = 4; /* Bit vector cannot contain parts smaller than 4.  */
  uint_fast64_t repeats = 1; /* Needed to reuse update_size.  */
  bool is_too_big = false;

  uint_fast8_t bits_passed = 0;
  while (current_part == 0x0 && bits_passed < BITS_IN_VECTOR)
    {
      vector = vector << BITS_PER_VECTOR_PART;
      current_part = (vector & FIRST_VECTOR_PART)
                     >> (BITS_IN_VECTOR - BITS_PER_VECTOR_PART);
      bits_passed += BITS_PER_VECTOR_PART;
    }

  /* Will always terminate due to shifting left at the end of the loop.  */
  while (current_part != 0x0)
    {
      if (current_part == FOUR_BYTES_IN_VECTOR)
        {
          size = update_size (size, 4, &repeats, &largest, &is_too_big);
        }
      else if (current_part == EIGHT_BYTES_IN_VECTOR)
        {
          size = update_size (size, 8, &repeats, &largest, &is_too_big);
        }
      else if (current_part == POINTER_IN_VECTOR)
        {
          size = update_size (size, sizeof (void *), &repeats, &largest,
                              &is_too_big);
        }
      else /* Currently unreachable.  */
        {
          assert (false); /* Incorrect format of bit vector.  */
        }
      vector = vector << BITS_PER_VECTOR_PART;
      current_part = (vector & FIRST_VECTOR_PART)
                     >> (BITS_IN_VECTOR - BITS_PER_VECTOR_PART);
      bits_passed += BITS_PER_VECTOR_PART;
    }

  /* Struct size is always divisible by size of largest element.  */
  uint_fast64_t final_size = size + (largest - size % largest) % largest;
  is_too_big = check_if_too_big_size (final_size, size);

  assert (!is_too_big); /* Should never fail due to size of bit vector.  */
  return final_size;
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

  while (*cursor)
    {
      if (*cursor == '*')
        {
          return true;
        }
      cursor++;
    }

  return false;
}

/**
 * @brief checks if an updated number of repetitions would be too many to add
 * to a bit vector
 * @param new the updated value
 * @param old the old value
 * @param bits_left the number of bits left in the vector
 * @note works as long as value is not updated by more than 2^8 - 1
 */
bool
check_if_too_big_vector (uint_fast8_t new, uint_fast8_t old)
{
  /* Value is never subtracted, so smaller means overflow.  */
  if (new < old)
    {
      return true;
    }
  return false;
}

/**
 * @brief Adds a repetition of 4 bytes to a bit vector
 * @param vector the bit vector to update
 * @param repeats the number of repetitions of four bytes
 * @param bits_left a pointer to a variable containing the number of bits left
 * in the vector
 * @param is_too_big a pointer to a boolean indicating whether the struct is
 * too large to be represented by the vector
 * @return the updated bit vector
 */
static uint_fast64_t
add_fours (uint_fast64_t vector, uint_fast8_t repeats, uint_fast8_t *bits_left,
           bool *is_too_big)
{
  if (repeats > *bits_left / BITS_PER_VECTOR_PART)
    {
      *is_too_big = true;
      return vector;
    }

  *bits_left -= repeats * BITS_PER_VECTOR_PART;
  for (/* empty */; repeats > 0; repeats--)
    {
      vector = (vector << BITS_PER_VECTOR_PART) | FOUR_BYTES_IN_VECTOR;
    }

  return vector;
}

/**
 * @brief Adds a repetition of 8 bytes to a bit vector
 * @param vector the bit vector to update
 * @param repeats the number of repetitions of eight bytes
 * @param bits_left a pointer to a variable containing the number of bits left
 * in the vector
 * @param is_too_big a pointer to a boolean indicating whether the struct is
 * too large to be represented by the vector
 * @return the updated bit vector
 */
static uint_fast64_t
add_eights (uint_fast64_t vector, uint_fast8_t repeats,
            uint_fast8_t *bits_left, bool *is_too_big)
{
  if (repeats > *bits_left / BITS_PER_VECTOR_PART)
    {
      *is_too_big = true;
      return vector;
    }

  *bits_left -= repeats * BITS_PER_VECTOR_PART;
  for (/* empty */; repeats > 0; repeats--)
    {
      vector = (vector << BITS_PER_VECTOR_PART) | EIGHT_BYTES_IN_VECTOR;
    }

  return vector;
}

/**
 * @brief Adds an amount of ("extra") bytes from a variable to a bit vector. As
 * a side effect updates the variable to 0.
 * @param vector the bit vector to update
 * @param count_extra a pointer to a variable keeping track of bytes that have
 * been read but not yet added to the vector.
 * @param bits_left a pointer to a variable containing the number of bits left
 * in the vector
 * @param is_too_big a pointer to a boolean indicating whether the struct is
 * too large to be represented by the vector
 * @return the updated bit vector
 */
static uint_fast64_t
add_extra_bytes (uint_fast64_t vector, uint_fast8_t *count_extra,
                 uint_fast8_t *bits_left, bool *is_too_big)
{
  uint_fast8_t padding = (4 - *count_extra % 4) % 4;
  uint_fast8_t fours_to_add = (*count_extra + padding) / 4;

  vector = add_fours (vector, fours_to_add, bits_left, is_too_big);

  *count_extra = 0;
  return vector;
}

/**
 * @brief looks from end of bit vector for pairs of 4 bytes, until a pointer or
 * 8 bytes is found, and converts the found pairs to 8 byte parts. As a side
 * effect, updates the number of bits left in the vector.
 * @param vector the vector to perform compacting on
 * @param bits_left a pointer to a variable containing the number of bits left
 * in the vector
 * @param is_too_big a pointer to a boolean indicating whether the struct is
 * too large to be represented by the vector
 * @return the updated bit vector
 */
static uint_fast64_t
compact_fours (uint_fast64_t vector, uint_fast8_t *bits_left)
{
  uint_fast8_t fours_found = 0;
  bool only_fours = true;
  while (only_fours)
    {
      if ((vector & LAST_VECTOR_PART) == FOUR_BYTES_IN_VECTOR)
        {
          fours_found++;
          vector = vector >> BITS_PER_VECTOR_PART;
        }
      else
        {
          only_fours = false;
        }
    }

  if (fours_found % 2 == 1)
    {
      vector = (vector << BITS_PER_VECTOR_PART) | FOUR_BYTES_IN_VECTOR;
      fours_found--;
    }
  *bits_left -= BITS_PER_VECTOR_PART * fours_found;

  bool dummy_bool; /* There will never be more 8 byte parts added than 4 byte
                      parts removed.  */
  vector = add_eights (vector, fours_found / 2, bits_left, &dummy_bool);

  return vector;
}

/**
 * @brief Adds a repetition of an amount of bytes to a bit vector, and as a
 * side effect updates variables that keep track of bits left in the vector,
 * extra bytes that have not been added yet, the number of repetitions of the
 * data type (always reset to 0) and whether the value being added is too big
 * for the vector to represent.
 * @param vector the vector to update
 * @param bytes the amount of bytes to store tha data type
 * @param repeats a pointer the number of repetitions of the data type
 * @param count_extra a pointer to a variable keeping track of bytes that have
 * been read but not yet added to the vector.
 * @param bits_left a pointer to a variable containing the number of bits left
 * in the vector
 * @param is_too_big a pointer to a boolean indicating whether the struct is
 * too large to be represented by the vector
 * @return the updated bit vector.
 * @note encoding will not work if any data type is larger than 8 bits.
 * @note possible improvement: fours can also be compacted right after 8 byte
 * part, even if there is no 8 byte part after (padding calculations won't be
 * affected)
 */
static uint_fast64_t
update_bit_vector (uint_fast64_t vector, uint_fast8_t bytes,
                   uint_fast8_t *repeats, uint_fast8_t *count_extra,
                   uint_fast8_t *bits_left, bool *is_too_big)
{
  assert (bytes > 0);

  if (*repeats == 0)
    {
      *repeats = 1;
    }

  if (bytes < 4)
    {
      /* "padding" in case there are 2 byte ints?  */
      *count_extra
          += *repeats * bytes + (bytes - *count_extra % bytes) % bytes;
      *repeats = 0;
      return vector;
    }

  assert (bytes % 4 == 0);
  if (bytes > 8) /* Encoding would result in incorrect size calculations due to
                    padding.  */
    {
      *is_too_big = true;
      return vector;
    }

  if (bytes == 8)
    {
      vector = compact_fours (
          add_extra_bytes (vector, count_extra, bits_left, is_too_big),
          bits_left);
      vector = add_eights (vector, *repeats, bits_left, is_too_big);
    }
  else /* bytes == 4  */
    {
      vector = add_fours (vector, *repeats, bits_left, is_too_big);
    }

  *repeats = 0;
  return vector;
}

/* NOTE: cannot handle structs only containing chars (unless divisible by 4) */
uint_fast64_t
convert_to_bit_vector (char *format, uint_fast64_t size, bool *success)
{
  if (!contains_pointers (format))
    {
      /* size need to fit in vector.  */
      if (size >= ((uint64_t)0x1) << (BITS_IN_VECTOR - BITS_FOR_FORMAT_TYPE))
        {
          *success = false;
        }
      else
        {
          *success = true;
        }

      return add_formating_encoding (size, FORMAT_SIZE_NO_POINTERS);
    }

  uint_fast64_t vector = 0;
  char *cursor = format;
  uint_fast8_t repeats = 0;
  uint_fast8_t count_extra = 0;
  uint_fast8_t bits_left = BITS_IN_VECTOR - BITS_FOR_FORMAT_TYPE;
  bool is_too_big = false;

  while (!is_too_big)
    {
      switch (
          *cursor) /* TODO: maybe refactor so that (most of) this switch
                      case is in a function that size_from_string also uses. */
        {
        case 'i':
          vector = update_bit_vector (vector, sizeof (int), &repeats,
                                      &count_extra, &bits_left, &is_too_big);
          break;

        case 'l':
          vector = update_bit_vector (vector, sizeof (long), &repeats,
                                      &count_extra, &bits_left, &is_too_big);
          break;

        case 'f':
          vector = update_bit_vector (vector, sizeof (float), &repeats,
                                      &count_extra, &bits_left, &is_too_big);
          break;

        case 'c':
          vector = update_bit_vector (vector, sizeof (char), &repeats,
                                      &count_extra, &bits_left, &is_too_big);
          break;

        case 'd':
          vector = update_bit_vector (vector, sizeof (double), &repeats,
                                      &count_extra, &bits_left, &is_too_big);
          break;

        case '*':
          vector = add_extra_bytes (vector, &count_extra, &bits_left,
                                    &is_too_big);
          if (is_too_big)
            {
              break;
            }

          if (repeats == 0)
            repeats = 1;

          if (bits_left / BITS_PER_VECTOR_PART < repeats)
            {
              is_too_big = true;
              break;
            }

          bits_left -= BITS_PER_VECTOR_PART * repeats;
          ;
          for (/* empty */; repeats > 0; repeats--)
            {
              vector = (vector << BITS_PER_VECTOR_PART) | POINTER_IN_VECTOR;
            }

          break;

        case '\0':
          /* For format strings ending with numbers or chars.  */

          if (repeats > 0 || count_extra > 0)
            {
              count_extra += repeats;

              vector = add_extra_bytes (vector, &count_extra, &bits_left,
                                        &is_too_big);

              if (is_too_big)
                break;
            }

          *success = true;

          return add_formating_encoding (vector, FORMAT_VECTOR);

        default:
          assert (isdigit (*cursor)); /* Otherwise, not valid format string
              converts digit in char to correct integer and adds to repeats. */
          uint_fast64_t new_repeats = repeats * 10 + *cursor - '0';

          is_too_big = check_if_too_big_vector (new_repeats, repeats);
          repeats = new_repeats;

          break;
        }

      cursor++;
    }
  /* Only reached if size exceeded maximum.  */
  *success = false;
  return 0;
}

/**
 * Finds and returns the address to a pointer in an allocation, located at
 * offset bytes from the start
 */
void *
find_pointer_in_alloc (void *allocation_start, uint_fast64_t offset)
{
  void **pointer_location = (void **)(((char *)allocation_start) + offset);

  return pointer_location;
}

/* Does not have support for size with pointers.  */
ptr_queue_t *
get_pointers_from_bit_vector (uint_fast64_t vector, void *allocation_start)
{
  ptr_queue_t *pointers = create_ptr_queue ();

  enum FormatType type = get_format_type (vector);
  vector = remove_formating_encoding (vector);

  if (type != FORMAT_VECTOR)
    return pointers;

  uint_fast64_t size = 0;
  uint_fast64_t current_part = ((vector & FIRST_VECTOR_PART))
                               >> (BITS_IN_VECTOR - BITS_PER_VECTOR_PART);
  uint_fast8_t largest = 1;  /* Needed to reuse update_size.  */
  uint_fast64_t repeats = 1; /* Needed to reuse update_size.  */
  bool is_too_big = false;

  /* Find start of bit vector.  */
  uint_fast8_t bits_passed = 0;
  while (current_part == 0x0 && bits_passed < BITS_IN_VECTOR)
    {
      vector = vector << BITS_PER_VECTOR_PART;
      current_part = (vector & FIRST_VECTOR_PART)
                     >> (BITS_IN_VECTOR - BITS_PER_VECTOR_PART);
      bits_passed += BITS_PER_VECTOR_PART;
    }

  /* Will always terminate due to shifting left at the end of the loop.  */
  while (current_part != 0x0)
    {
      if (current_part == FOUR_BYTES_IN_VECTOR)
        {
          size = update_size (size, 4, &repeats, &largest, &is_too_big);
        }
      else if (current_part == EIGHT_BYTES_IN_VECTOR)
        {
          size = update_size (size, 8, &repeats, &largest, &is_too_big);
        }
      else if (current_part == POINTER_IN_VECTOR)
        {
          /* Size needs to be calculated first, to account for padding.  */
          size = update_size (size, sizeof (void *), &repeats, &largest,
                              &is_too_big);
          enqueue_ptr (pointers,
                       find_pointer_in_alloc (allocation_start,
                                              size - sizeof (void *)));
        }
      else /* Currently unreachable.  */
        {
          assert (false); /* incorrect format of bit vector.  */
        }
      vector = vector << BITS_PER_VECTOR_PART;
      current_part = (vector & FIRST_VECTOR_PART)
                     >> (BITS_IN_VECTOR - BITS_PER_VECTOR_PART);
      bits_passed += BITS_PER_VECTOR_PART;
    }

  assert (!is_too_big); /* Should never fail due to size of bit vector.  */

  return pointers;
}

size_t
round_up_to_multiple (size_t input, size_t multiple)
{
  if (input % multiple > 0)
    {
      input += multiple - (input % multiple);
    }
  return input;
}

ptr_queue_t *
get_pointers_from_format_string (char *format, void *allocation_start)
{
  ptr_queue_t *pointers = create_ptr_queue ();
  /* for each char in format, check if it is a number preceeding a pointer */
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
              void *internal_pointter
                  = (((char *)allocation_start) + internal_offset);
              enqueue_ptr (pointers, internal_pointter);
              internal_offset = round_up_to_multiple (internal_offset, 8) + 8;
            }
          repeats = 0;
          break;

        default:
          assert (isdigit (ch)); /* Otherwise, not valid format string
              converts digit in char to correct integer and adds to repeats. */
          uint_fast64_t new_repeats = repeats * 10 + ch - '0';
          repeats = new_repeats;
          break;
        }
      cursor++;
      ch = *cursor;
    }
  return pointers;
}

enum FormatType
get_format_type (uint_fast64_t binary_string)
{
  /* Masks out first and second bit (LSB) using "and" operation with the
     byte 0x3 (0011). Note that it is important that binary_string is a
     unsigned value so right shift makes a logical shift instead of arithmetic.
   */
  switch (mask_formating_encoding (binary_string))
    {
    case FORMAT_SIZE_NO_POINTERS: /* 0x0 (00)  */
      return FORMAT_SIZE_NO_POINTERS;
    case FORMAT_VECTOR: /* 0x1 (01)  */
      return FORMAT_VECTOR;
    case FORMAT_SIZE_HAS_POINTERS: /* 0x2 (10)  */
      return FORMAT_SIZE_HAS_POINTERS;
    case FORMAT_UNUSED: /* 0x3 (11)  */
      return FORMAT_UNUSED;
    default: /* impossible to reach  */
      /* Crash with message after "&&"  */
      assert (0 && "Format mask resulted in number larger than [0, 3]");
    }
}

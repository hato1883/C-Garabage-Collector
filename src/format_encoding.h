/**
 * Utility functions for checking metadata in headers for allocated objects.
 */

#pragma once

#include "ptr_queue.h"

#include <stdbool.h>
#include <stdint.h>

/* The number of bits used to encode format type. */
#define BITS_FOR_FORMAT_TYPE 2
#define MASK_FOR_FORMAT_TYPE 0x3

/* NOTE: macros below to be used on bit vectors - not headers, which have extra
   bits holding information.  */

/**
 * Sets format encoding bits to 0
 */
#define clear_formating_encoding(bitvector)                                   \
  (bitvector & ~(MASK_FOR_FORMAT_TYPE))

/**
 * Removes format encoding from bit vector (opposite of add) -
 * for example, if bit vector contains a size, this will leave only the size in
 * the vector
 */
#define remove_formating_encoding(bitvector)                                  \
  (bitvector >> BITS_FOR_FORMAT_TYPE)

/**
 * Adds format encoding to bit vector which does not have any (opposite of
 * remove) `encoding` should be between 0-3 (inclusive)
 */
#define add_formating_encoding(bitvector, encoding)                           \
  ((bitvector << BITS_FOR_FORMAT_TYPE) | ((uint64_t)encoding))

/**
 * Sets bit 1-0 to `encoding`
 * `encoding` should be between 0-3 (inclusive)
 */
#define set_formating_encoding(bitvector, encoding)                           \
  (clear_formating_encoding (bitvector) | ((uint64_t)encoding))

/**
 * Masks out bits 1-0 which holds the `encoding`
 */
#define mask_formating_encoding(bitvector) (bitvector & MASK_FOR_FORMAT_TYPE)

/**
 * Enum of the 4 different possible format encodings supported.
 * - `FORMAT_SIZE_NO_POINTERS` holds a unsigned 60 bit number
 * representing the size. This allocation has no pointers.
 * - `FORMAT_VECTOR` holds a combination of `00` `01` `10` and `11`
 * the 4 different combinations represent different data sizes.
 * - `FORMAT_SIZE_HAS_POINTERS` Similar to the first except this one
 * CAN CONTAIN pointers but dose not have to.
 * - `FORMAT_UNUSED` Currently there is no 4th encoding type.
 */
enum FormatType
{
  /* A 7 byte long unsigned int describing total size.  */
  FORMAT_SIZE_NO_POINTERS = 0x0, /* 00  */

  /* A 60 bit long combination of Pointers, 4 Byte data (Not pointer).  */
  FORMAT_VECTOR = 0x1, /* 01  */

  /* Might not be needed. Discussion ongoing with @omom42  */
  FORMAT_SIZE_HAS_POINTERS = 0x2, /* 10  */

  /* Unused bit combination.  */
  FORMAT_UNUSED = 0x3, /* 11  */
};

/**
 * Retrieves the bitvectors format encoding type.
 * Used due to multiple ways to encode size and format och the allocation.
 * @return The `enum FormatType` used in this the bitvector.
 */
enum FormatType get_format_type (uint_fast64_t binary_string);

/** @brief Calculates the size of a struct based on a format string.
 * @note Valid characters in format strings are:
 * - 'i' -- for sizeof(int) bytes 'raw' data
 * - 'l' -- for sizeof(long) bytes 'raw' data
 * - 'f' -- for sizeof(float) bytes 'raw' data
 * - 'c' -- for sizeof(char) bytes 'raw' data
 * - 'd' -- for sizeof(double) bytes 'raw' data
 * - '*' -- for a sizeof(void *) bytes pointer value
 * - '\0' -- null-character terminates the format string
 * - a number -- indicating repetition, e.g. "3i" is equivalent to "iii"
 * @note A format string only containing a number will be interpreted as a
 * size in chars, e. g. "32" is equivalent to "32c"
 * @param format a format string describing a struct
 * @param success a pointer to a boolean, which will be set to true if the
 *   calculation was successful, and false if not.
 * @return the size of the struct specified by the format string
 */
uint_fast64_t size_from_string (char *format, bool *success);

/** @brief Calculates the size of a struct based on a bit vector
 * @note A bit vector, if it does not contain a size, is interpreted as
 * follows:
 *   - 01 represents 4 bytes of data
 *   - 10 represents 8 bytes of data
 *   - 11 represents a pointer
 *   - 00 signifies the end of the format string - there cannot be any data in
 * more significant bits.
 * @param vector a bit vector describing a struct
 * @return the size of the struct specified by the bit vector
 */
uint_fast64_t size_from_vector (uint_fast64_t vector);

/** @brief Converts a format string to a bit vector for more compact storage.
 * @note Valid characters in format strings are:
 * - 'i' -- for sizeof(int) bytes 'raw' data
 * - 'l' -- for sizeof(long) bytes 'raw' data
 * - 'f' -- for sizeof(float) bytes 'raw' data
 * - 'c' -- for sizeof(char) bytes 'raw' data
 * - 'd' -- for sizeof(double) bytes 'raw' data
 * - '*' -- for a sizeof(void *) bytes pointer value
 * - '\0' -- null-character terminates the format string
 * - a number -- indicating repetition, e.g. "3i" is equivalent to "iii"
 * @note A format string only containing a number will be interpreted as a
 * size in chars, e. g. "32" is equivalent to "32c"
 * @note Bit vectors cannot represent structs only containing chars, unless the
 * number of chars is divisible by 4.
 * @param format a format string describing a struct
 * @param success a pointer to a boolean, which will be set to true if the
 *   conversion was successful, and false if not.
 * @return a bit vector, where the least significant bits signify whether it is
 *  storing a size of the struct or the format represented more compactly,
 *  where:
 *   - 01 represents 4 bytes of data
 *   - 10 represents 8 bytes of data
 *   - 11 represents a pointer
 *   - 00 signifies the end of the format string - there cannot be any data in
 *  more significant bits.
 */
uint_fast64_t convert_to_bit_vector (char *format, uint_fast64_t size,
                                     bool *success);

/**
 * @brief Finds pointers to all pointers in an allocation described by a bit
 * vector
 * @note A bit vector, if it does not contain a size, is interpreted as
 * follows:
 *   - 01 represents 4 bytes of data
 *   - 10 represents 8 bytes of data
 *   - 11 represents a pointer
 *   - 00 signifies the end of the format string - there cannot be any data in
 * more significant bits.
 * @param vector a bit vector describing a struct
 * @param allocation_start a pointer to the start of the allocation
 * @return a pointer queue containing the pointers in the struct
 */
ptr_queue_t *get_pointers_from_bit_vector (uint_fast64_t vector,
                                           void *allocation_start);

/**
 * @brief Finds pointers to all pointers in an allocation described by a format string
 * @note Valid characters in format strings are:
 * - 'i' -- for sizeof(int) bytes 'raw' data
 * - 'l' -- for sizeof(long) bytes 'raw' data
 * - 'f' -- for sizeof(float) bytes 'raw' data
 * - 'c' -- for sizeof(char) bytes 'raw' data
 * - 'd' -- for sizeof(double) bytes 'raw' data
 * - '*' -- for a sizeof(void *) bytes pointer value
 * - '\0' -- null-character terminates the format string
 * - a number -- indicating repetition, e.g. "3i" is equivalent to "iii"
 * @param format a format string describing layout of the allocation
 * @param allocation_start a pointer to the start of the allocation
 * @return a pointer queue containing the pointers in the struct
 */
ptr_queue_t *get_pointers_from_format_string (char *format,
                                              void *allocation_start);

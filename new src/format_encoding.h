/**
 * Utility functions for checking metadata in headers for allocated objects.
 */

#pragma once

#include "ptr_queue.h"

#include <stdbool.h>
#include <stdint.h>

#define SIZE_MAX 0xFFFFFFFFFFFFFFFFULL

typedef uint64_t bitvector_t;

typedef unsigned char vector_part_t;

/**
 * @brief Calculates the size of a struct based on a format string.
 *   The resulting size includes any padding needed for alignment.
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
 *   size in chars, e. g. "32" is equivalent to "32c"
 * @param format a format string describing a struct
 * @param success a pointer to a boolean, which will be set to true if the
 *   calculation was successful, and false if not.
 * @return Size of allocation represented by format string
 */
size_t size_from_string (char *format, bool *success);

/**
 * @brief Calculates the size of a struct based on a bitvector.
 * @note bitvector contains of 4 different parts
 * - '00' -- No data
 * - '01' -- 4 bytes data (can't be a pointer)
 * - '10' -- 8 bytes data (can't be a pointer)
 * - '11' -- 8 bytes data IS a pointer
 * @param vector bitvector to decode into size
 * @return Size of allocation represented by bitvector
 */
size_t size_from_vector (bitvector_t vector);

/**
 * @brief Converts a format string to a bit vector for more compact storage.
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
bitvector_t convert_format_to_bit_vector (char *format, bool *success);

/**
 * @brief Finds all internal pointers in an allocation described by a bit
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
ptr_queue_t *get_pointers_from_bit_vector (bitvector_t vector,
                                           void *allocation_start);

/**
 * @brief Finds pointers to all pointers in an allocation described by a format
 * string
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

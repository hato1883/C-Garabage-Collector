/**
 * Utility functions regarding creation of headers and retrieval
 * of pointers within the object the header refers to.
 */

#pragma once
#include "ptr_queue.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * The number of bits in a header.
 * Note: if this is changed, header_t also needs to be changed and vice versa.
 */
#define BITS_IN_HEADER 64

/**
 *  The number of bits used to encode header type.
 */
#define BITS_FOR_HEADER_TYPE 2

/**
 * A header containing metadata for an allocation
 */
typedef uint64_t header_t;

/* The four different header alternatives.  */
typedef enum
{
  HEADER_POINTER_TO_FORMAT_STRING = 0x0,
  HEADER_FORWARDING_ADDRESS = 0x1,
  /* UNUSED is available for further implementation. */
  HEADER_UNUSED = 0x2,
  HEADER_BIT_VECTOR = 0x3,
} header_type_t;

/**
 * @brief Creates a header from a format string and the size of the struct
 * described by the format string.
 * @note Valid characters in format strings are:
 * - 'i' -- for sizeof(int) bytes 'raw' data
 * - 'l' -- for sizeof(long) bytes 'raw' data
 * - 'f' -- for sizeof(float) bytes 'raw' data
 * - 'c' -- for sizeof(char) bytes 'raw' data
 * - 'd' -- for sizeof(double) bytes 'raw' data
 * - '*' -- for a sizeof(void *) bytes pointer value
 * - '\0' -- null-character terminates the format string
 * - a number -- indicating repetition, e.g. "3i" is equivalent to "iii"
 * @param format_string a format string
 * @param size the size of the struct described by the format string
 * @param success a pointer to a boolean, which will be set to true if the
 *   function was successful, and false if not.
 * @return a header describing the struct with size if no pointers are found,
 * otherwise using a bit vector containing the same information as the format
 * string
 */
header_t create_header_struct (char *format_string, size_t size,
                               bool *success);

/**
 * @brief Creates a header from a format string.
 * @note Valid characters in format strings are:
 * - 'i' -- for sizeof(int) bytes 'raw' data
 * - 'l' -- for sizeof(long) bytes 'raw' data
 * - 'f' -- for sizeof(float) bytes 'raw' data
 * - 'c' -- for sizeof(char) bytes 'raw' data
 * - 'd' -- for sizeof(double) bytes 'raw' data
 * - '*' -- for a sizeof(void *) bytes pointer value
 * - '\0' -- null-character terminates the format string
 * - a number -- indicating repetition, e.g. "3i" is equivalent to "iii"
 * @param format_string a format string describing a struct
 *  * @param success a pointer to a boolean, which will be set to true if the
 *   function was successful, and false if not
 * @return a header describing the struct with size if no pointers are found,
 * otherwise using a bit vector containing the same information as the format
 * string
 */
header_t create_header_struct_unknown_size (char *format_string,
                                            bool *success);
/**
 * @brief Creates a header from a size.
 * @param size the size of an allocation
 * @param success a pointer to a boolean, which will be set to true if the
 *   function was successful, and false if not
 * @return a header containing the given size
 */
header_t create_header_raw (size_t size, bool *success);

/**
 * @brief Finds pointers in an allocation for a struct.
 * @param allocation_start a pointer to the start of the allocation
 * @return a pointer queue containing the pointers in the struct
 */
ptr_queue_t *get_pointers_in_allocation (void *allocation_start);

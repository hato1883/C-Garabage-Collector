/**
 * Utility functions regarding creation of headers and retrieval
 * of pointers within the object the header refers to.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ptr_queue.h"

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
 * bitmask to get header type
 */
#define MASK_FOR_HEADER_TYPE ((uint64_t)0b11)

/**
 * A header containing metadata for an allocation
 */
typedef uint64_t header_t;

/* The four different header alternatives.  */
typedef enum HeaderType
{
  HEADER_POINTER_TO_FORMAT_STRING = 0b00,
  HEADER_FORWARDING_ADDRESS = 0b01,
  HEADER_SIZE = 0b10,
  HEADER_BIT_VECTOR = 0b11,
} header_type_t;

/**
 * @brief Creates appropriate header to represent size given from format
 * string. If string dose not contain pointers it will be saved as a number, If
 * it has pointers but can be represented by a bitvector it will, otherwise we
 * will save string in the heap and give a pointer to the header.
 * It will fail if the format string has a invalid format or character.
 *
 * @param heap To allocate the string in if needed
 * @param format string to convert into a header
 * @param success Flag to indicate if conversion was successful.
 * @return A Header with format encoded in one of the 3 possible ways.
 *
 * @note Valid characters in format strings are:
 * @note 'i' -- for sizeof(int) bytes 'raw' data
 * @note 'l' -- for sizeof(long) bytes 'raw' data
 * @note 'f' -- for sizeof(float) bytes 'raw' data
 * @note 'c' -- for sizeof(char) bytes 'raw' data
 * @note 'd' -- for sizeof(double) bytes 'raw' data
 * @note '*' -- for a sizeof(void *) bytes pointer value
 * @note '\0' -- null-character terminates the format string
 * @note a number -- indicating repetition, e.g. "3i" is equivalent to "iii"
 */
header_t convert_format_to_header (heap_t *heap, char *format, bool *success);
/**
 * @brief Creates a header from a size.
 * @param size the size of an allocation
 * @param success a pointer to a boolean, which will be set to true if the
 *   function was successful, and false if not
 * @return a header containing the given size
 */
header_t create_header_raw (size_t size, bool *success);

/**
 * @brief Calculates the size of a allocation using data stored in the header
 * @note Header contains 3 possible sizes,
 * - Raw which is just the number of bytes needed.
 * - Bitvector which is a collection of 31 vector parts.
 * - Format string which is given by the user during allocation.
 * @param header metadata for the allocation of interest
 * @return the size of the struct specified by the header
 */
size_t size_from_header (header_t header);

/**
 * @brief Finds all internal pointers in an non forwarded allocation.
 * @param alloc a pointer to the start of the allocation
 * @return a pointer queue containing the pointers in the struct, Might be
 * empty.
 */
ptr_queue_t *get_pointers_from_allocation (void *alloc);

/**
 * Gets the pointer to the header for a given pointer.
 * Note that currently no checks are being run to make sure this piece of
 * memory actually has a header among other important checks.
 * @param memory_pointer - a pointer.
 * @return the pointer to (hopefully) a header
 */
header_t *get_header_pointer (void *memory_pointer);

/**
 * From a pointer to a header, get a copy of the header.
 * @param header_pointer - pointe to (hopefully) a header.
 * @return A copy of the headers value.
 */
header_t get_header_value (header_t *header_pointer);

/**
 * sets bit 1 & 0 to 00, useful when needing to accesss pointer in header.
 * @param header - a 64 bit header.
 * @return Header with format set to 00.
 */
header_t remove_header_type (header_t header);

/**
 * Updates the header of a pointer to a new header.
 * Note that no checks are being run to make sure the header updates something
 * when available.
 * @param pointer - a pointer
 * @param new_header - the new header to be put before the pointer
 */
void update_header_of_a_pointer (void *pointer, header_t new_header);

/**
 * Get the headerinformation from a header. The information is
 * contained in last 2 bits (LSB).
 * @param the_header - a header
 * @return One of the predefined headertypes
 */
header_type_t get_header_type (header_t the_header);

/**
 * Returns a the given header with updated header type
 * HEADER_POINTER_TO_FORMAT_STRING.
 * @param binary_string - A binary string.
 * @return The updated binary string.
 */
header_t set_header_pointer_to_format_string (header_t the_header);

/**
 * Returns a the header with updated header type
 * HEADER_FORWARDING_ADDRESS.
 * @param binary_string - A binary string
 * @return The updated binary string
 */
header_t set_header_forwarding_address (header_t binary_string);

/**
 * Returns a header with updated header type
 * HEADER_UNUSED.
 * @param binary_string - A binary string.
 * @return The updated binary string.
 */
header_t set_header_size (header_t the_header);

/**
 * Returns a header with updated header type
 * HEADER_BIT_VECTOR.
 * @param binary_string - A binary string.
 * @return The updated binary string.
 */
header_t set_header_bit_vector (header_t the_header);

/**
 * @brief Stores a pointer in a header
 * @param header a header
 * @param pointer a pointer to store in the header
 * @return the header, with the pointer in it
 * @note the header type will still need to be set separately
 */
header_t change_pointer_in_header (header_t header, void *p);

/**
 * @brief Returns a pointer stored in a header
 * @param header a header containing a pointer
 * @return the pointer that was stored in the header
 */
void *get_pointer_in_header (header_t header);

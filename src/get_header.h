/**
 * Functions that gets the header of an object.
 * Information about the header is stored in the first two bit (LSB) d
 * Also contains function that update said headers.
 */

#pragma once

#include "header.h"
#include <stdint.h>

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
header_t set_header_unused (header_t the_header);

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

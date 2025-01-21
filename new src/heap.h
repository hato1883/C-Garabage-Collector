/**
 * Opaque definition of heap_t struct.
 */

#pragma once
#include <stdint.h>

/**
 * The opaque data type holding all the heap data
 */
typedef struct heap heap_t;

/**
 * @brief Calculates how much space is available in the heap using the
 * allocation map. This estimate excludes any data that is used for
 * alignments.
 *
 * @note Whilst the number of available bytes might be high, this dose
 * not equal such a object can be allocated. This is due to fragmentation in
 * the heap leaving spots which will be reported as free but might be impossible
 * to allocate a object on.
 *
 * @param heap Heap to count available space in.
 * @return Number of bytes that can be allocated.
 */
size_t num_free_bytes (heap_t *heap);
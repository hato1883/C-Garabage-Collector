/**
 * Functions used for allocations, do not use these directly to allocate,
 * always use h_alloc_struct or h_allow_raw.
 */

#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "heap.h"

/**
 * Calculates the size of an allocation
 * @param alloc a pointer to the allocation
 * @return size_t the size of the allocation in bytes.
 */
size_t calc_alloc_size (char *alloc);

/**
 * Allocate a new object on a heap with a given format string.
 *
 * Valid characters in format strings are:
 * - 'i' -- for sizeof(int) bytes 'raw' data
 * - 'l' -- for sizeof(long) bytes 'raw' data
 * - 'f' -- for sizeof(float) bytes 'raw' data
 * - 'c' -- for sizeof(char) bytes 'raw' data
 * - 'd' -- for sizeof(double) bytes 'raw' data
 * - '*' -- for a sizeof(void *) bytes pointer value
 * - '@0' -- null-character terminates the format string
 *
 * @param h the heap
 * @param layout the format string
 * @return the newly allocated object
 *
 * @note: the heap does *not* retain an alias to layout.
 */
void *alloc_struct (heap_t *h, char *layout);

/**
 * Allocate a new object on a heap with a given size.
 *
 * Objects allocated with this function will *not* be
 * further traced for pointers.
 *
 * @param h the heap
 * @param bytes the size in bytes
 * @return the newly allocated object
 */
void *alloc_raw (heap_t *h, size_t bytes);

/**
 * Checks if the allocation is possible. May trigger GC to make space in the
 * heap if threshold is reached. If allocation is possible the bump pointer is
 * moved to the next available space.
 * @param h the heap
 * @param alloc_size the requested allocation size
 * @return true if the allocation is possible.
 */
bool move_to_valid_space_if_alloc_possible (heap_t *h, size_t alloc_size);

/**
 * @brief Adds padding to size if needed to ensure alignment
 * @param alloc_size Original size
 * @return Aligned Size >= alloc_size
 */
size_t align_alloc_size (size_t alloc_size);

/**
 * @file
 * Utility functions for the GC, contains two functions.
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "heap.h"

/**
 * @fn
 * @param ptr a pointer to manipulate
 * @param other an optional other value to send to the function
 */
typedef void apply_to_ptr_func (void *ptr, void *other);

/**
 * @fn
 * @brief Applies the given function to all pointers in the interval between
 * addresses start and end.
 * @param[in] start the start address of the interval (inclusive)
 * @param[in] end the end address of the interval (non-inclusive)
 * @param[in] func a function which takes a pointer as an argument
 * @param[in] arg an optional argument sent to func
 */
void apply_to_pointers_in_interval (uintptr_t start, uintptr_t end,
                                    apply_to_ptr_func *func, void *arg);

/**
 * @fn
 * Calculates the heap pointers offset in bytes from the start of the heap.
 * @param[in] heap_ptr a pointer which points into the heap
 * @param[in] h the heap
 * @return the pointer's offset in bytes from the start of the heap
 */
size_t calc_heap_offset (void *heap_ptr, heap_t *h);

/**
 * @fn
 * @brief Calculates the input number rounded up to the nearest multiple of
 * alignment.
 *
 * Example:
 *
 * round_up_to_alignment (1, 4) = 4
 *
 * round_up_to_alignment (5, 4) = 8
 *
 * round_up_to_alignment (4, 4) = 4
 *
 *
 * @param[in] input Number to be rounded up
 * @param[in] alignment Number to be rounded against
 * @return input rounded up to nearest multiple of alignment.
 */
size_t round_up_to_alignment (size_t input, size_t alignment);

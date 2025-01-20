/**
 * Utility functions for the GC, contains two functions.
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "heap.h"

/**
 * @param ptr a pointer to manipulate
 * @param other an optional other value to send to the function
 */
typedef void apply_to_ptr_func (void *ptr, void *other);

/**
 * Applies the given function to all pointers in the interval between addresses
 * start and end.
 * @param start the start address of the interval (inclusive)
 * @param end the end address of the interval (non-inclusive)
 * @param func a function which takes a pointer as an argument
 * @param arg an optional argument sent to func
 */
void apply_to_pointers_in_interval (uintptr_t start, uintptr_t end,
                                    apply_to_ptr_func *func, void *arg);

/**
 * Calculates the heap pointers offset in bytes from the start of the heap.
 * @param heap_ptr a pointer which points into the heap
 * @param h the heap
 * @return the pointer's offset in bytes from the start of the heap
 */
size_t calc_heap_offset (void *heap_ptr, heap_t *h);

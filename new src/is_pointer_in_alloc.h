/**
 * The purpose of this file is to know if a binary string could be a
 * pointer or not inside a heap.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "heap.h"
/**
 * @brief Checks if a pointer address (or maybe not) is in a heaps range.
 * @param pointer - unknown pointer (or maybe not).
 * @param the_heap - the_heap on which we want to check if the pointer is in
 * @return true if the value is in range of the heap address, false if not.
 */
bool is_in_range (void *pointer, heap_t *the_heap);

/**
 * @brief Checks whether a pointer (might not be one though) is in an active
 * memory.
 * @param pointer - the supposed pointer.
 * @param the_heap - the_heap in which we check if pointer might be active
 * in.
 * @return true if the pointer is in active memory, false if not.
 */
bool is_in_allocated_region (void *pointer, heap_t *the_heap);

/**
 * @brief Checks if a pointer is an address that is within a heap.
 * @param pointer - the (supposed) address we want to check.
 * @param the_heap - the heap we are checking the address in.
 * @return true if address is within heap.
 */
bool is_heap_pointer (void *pointer, heap_t *the_heap);

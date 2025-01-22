/**
 * The purpose of this file is to know if a binary string could be a
 * pointer or not inside a heap.
 */

// TODO: MODIFY FILE

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "heap.h"

/** Checks if a pointer address (or maybe not) is in a heaps range.
 * @param ptr_addr - unknown pointer (or maybe not).
 * @param the_heap - the_heap on which we want to check if the pointer is in
 * @return true if the value is in range of the heap address, false if not.  */
bool is_in_range (uintptr_t ptr_addr, heap_t *the_heap);

/** Checks whether a pointer (might not be one though) is in an active memory
 * @param ptr_addr - the supposed pointer.
 * @param the_heap - the_heap in which we check if ptr_address might be active
 * in.
 * @return true if the ptr_addr is in active memory, false if not.  */
bool is_in_allocated_region (uintptr_t ptr_addr, heap_t *the_heap);

/**
 * Checks if a uintptr_t is an address that is within a heap.
 * @param ptr_addr - the (supposed) address we want to check.
 * @param the_heap - the heap we are checking the address in.
 * @return true if address is within heap.
 */
bool is_heap_pointer (uintptr_t ptr_addr, heap_t *the_heap);

/**
 * Calculates which of the 8 bits in a byte would hold this allocation
 * use `offset_to_map_index` to find which byte to check.
 *
 * @param offset the offset from the heap start to the address you want to
 * check
 * @return a bit mask which can be used to mask out
 * the specific bit responsible for this offset in the allocation map
 */
char offset_to_map_mask (uintptr_t offset);

/**
 * Calculates which of bytes in the array would hold this allocation
 * use `offset_to_map_mask` to find which bit in the byte to check.
 *
 * @param offset the offset from the heap start to the address you want to
 * check
 * @return the index of the responsible byte in the allocation map
 */
size_t offset_to_map_index (uintptr_t offset);

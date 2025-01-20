/**
 * This header handles the interface for operations regarding the stack.
 * such as finding the stack range and growth direction.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief returns the base address of the stack.
 *
 * All program variables will be stored in the range
 * between the base address and the top address.
 *
 * @note Stack growth is architecture dependent and not os dependent.
 *
 * @return base stack address.
 */
uintptr_t find_stack_beginning (void);

/**
 * @brief returns the top address of the stack.
 *
 * All program variables will be stored in the range
 * between the base address and the top address.
 *
 * @note Stack growth is architecture dependent and not os dependent.
 *
 * @return top stack address.
 */
uintptr_t find_stack_end (void);

/**
 * Sorts the stack ends found using find_stack_beginning and found_stack_end so
 * that start always contains the lower value of the two, with end having the
 * higher value. This is used to be able to loop through all addresses in
 * between start and end without caring about the stack's growth direction.
 * @param start: the address to the variable with the value found by
 * find_stack_beginning
 * @param end: the address to the variable with the value found by
 * find_stack_end
 */
void sort_stack_ends (uintptr_t *start, uintptr_t *end);

/**
 * Checks if a value is a valid pointer that falls within the heap memory
 * range.
 * @param ptr: The address to check.
 * @param heap: The heap to look in.
 * @return true if the value is a valid pointer within the heap range.
 */
bool is_stack_pointer (uintptr_t ptr);

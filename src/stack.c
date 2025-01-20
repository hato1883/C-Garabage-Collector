/**
 * This file is the main file for functions handling the stack.
 * It handles retrieval of the stack endpoints
 * as well as checking stack growth direction.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gc_utils.h"
#include "heap_internal.h"
#include "stack.h"

/**
 * Finds the address from right before the start of the stack.
 * @return the address from right before the start of the stack.
 */
uintptr_t
find_stack_beginning (void)
{
  extern char *
      *environ; /* Environ is a pointer to the environment variables.  */
  return (uintptr_t)environ; /* Return the base address of the stack.  */
}

/**
 * Finds the approximate end of the stack.
 * @return the approximate address of the end of the program stack.
 */
__attribute__ ((noinline)) uintptr_t
find_stack_end (void)
{
  void *top = __builtin_frame_address (
      0); /* Use built-in function to get the top of the stack.  */
  return (uintptr_t)top; /* Return the top address of the stack.  */
}

/**
 * Sorts the stack ends found using find_stack_beginning and find_stack_end so
 * that start always contains the lower value of the two, with end having the
 * higher value.
 * This is used to loop through all addresses in between start and end
 * regardless of stack growth direction.
 * @param start: the address to the variable with the value found by
 * find_stack_beginning
 * @param end: the address to the variable with the value found by
 * find_stack_end
 */
void
sort_stack_ends (uintptr_t *start, uintptr_t *end)
{
  if (*start
      > *end) /* If start address is greater than end address, swap them.  */
    {
      uintptr_t tmp = *start;
      *start = *end;
      *end = tmp;
    }
}

/**
 * Checks if a pointer originates from the stack.
 * @param ptr: The address to check.
 * @param heap: The heap to look in.
 * @return true if the pointer is a value between stack start and stack end.
 */
bool
is_stack_pointer (uintptr_t ptr)
{
  if (ptr == 0)
    {
      return false;
    }

  uintptr_t stack_start = (uintptr_t)find_stack_beginning ();
  uintptr_t stack_end = (uintptr_t)find_stack_end ();

  /* Ensure stack_start is less than stack_end.  */
  sort_stack_ends (&stack_start, &stack_end);

  if (ptr >= stack_start && ptr <= stack_end)
    {
      return true;
    }

  return false;
}

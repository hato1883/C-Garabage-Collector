#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/allocation_map.h"
#include "../src/format_encoding.h"
#include "../src/gc.h"
#include "../src/gc_utils.h"
#include "../src/heap_internal.h"
#include "../src/page_map.h"

#define UNUSED(x) x __attribute__ ((__unused__))
#define SET_PTR_TEST_VAL 5 // A value used in test apply_func_to_ptrs

int
init_suite (void)
{
  h_init (1024 * 8, true, 0.75f);
  // Change this function if you want to do something *before* you
  // run a test suite
  return 0;
}

int
clean_suite (void)
{
  h_delete (global_heap);
  // Change this function if you want to do something *after* you
  // run a test suite
  return 0;
}

/* tests for normal GC */

void
create_heap_zero_size_test (void)
{
  size_t size = 0;
  heap_t *heap = h_init (size, false, 0);

  // Assumes Alignment of 16, 31 -> 32, 32 -> 32 and 33 -> 48
  CU_ASSERT_PTR_NOT_NULL (heap);
  CU_ASSERT_PTR_NOT_NULL (global_heap);
  CU_ASSERT_TRUE (heap->size > size);

  CU_ASSERT_PTR_NOT_NULL (heap->heap_start);
  CU_ASSERT_PTR_NOT_NULL (heap->next_empty_mem_segment);

  CU_ASSERT_PTR_NOT_NULL (heap->alloc_map);

  h_delete (heap);
}

void
create_heap_nonzero_size_test (void)
{
  size_t size = 31;
  heap_t *heap = h_init (size, false, 0);
  CU_ASSERT_PTR_NOT_NULL (heap);
  CU_ASSERT_PTR_NOT_NULL (global_heap);

  // Assumes Alignment of 16, 31 -> 32, 32 -> 32 and 33 -> 48
  CU_ASSERT_TRUE (heap->size >= size);
  CU_ASSERT_TRUE (heap->size < 48);

  CU_ASSERT_PTR_NOT_NULL (heap->heap_start);
  CU_ASSERT_PTR_NOT_NULL (heap->next_empty_mem_segment);

  CU_ASSERT_PTR_NOT_NULL (heap->alloc_map);
  CU_ASSERT_PTR_NOT_NULL (heap->page_map);

  h_delete (heap);
}

void
create_heap_size_divisible_by_align_test (void)
{
  size_t size = 32;
  heap_t *heap = h_init (size, false, 0);
  CU_ASSERT_PTR_NOT_NULL (heap);
  CU_ASSERT_PTR_NOT_NULL (global_heap);

  // Assumes Alignment of 16, 31 -> 32, 32 -> 32 and 33 -> 48
  CU_ASSERT_TRUE (heap->size >= size);
  CU_ASSERT_TRUE (heap->size < 48);

  CU_ASSERT_PTR_NOT_NULL (heap->next_empty_mem_segment);

  CU_ASSERT_PTR_NOT_NULL (heap->alloc_map);
  CU_ASSERT_PTR_NOT_NULL (heap->page_map);

  h_delete (heap);
}

void
set_ptr_val (void *ptr, UNUSED (void *arg_ignored))
{
  int *i_ptr = (int *)ptr;
  *i_ptr = SET_PTR_TEST_VAL;
}

void
apply_func_to_ptrs (void)
{
  // Allocates 5 sequential pointers, applies a function that sets all their
  // values to a new value. Checks that the values are the new values.
  size_t num_ptrs = 5;
  uintptr_t *ptrs = calloc (num_ptrs, sizeof (uintptr_t *));
  uintptr_t start = (uintptr_t)ptrs;
  uintptr_t end = start + num_ptrs * sizeof (uintptr_t *);
  apply_to_pointers_in_interval (start, end, (apply_to_ptr_func *)set_ptr_val,
                                 NULL);
  for (uintptr_t *i = ptrs; (uintptr_t)i < end; i++)
    {
      int num = (int)*i;
      CU_ASSERT_EQUAL (num, SET_PTR_TEST_VAL);
    }
  free (ptrs);
}

/**
 * Sets the ptr's value to the int stored in arg and increases arg by 1.
 */
void
set_ptr_val_to_index (void *ptr, void *arg)
{
  int *i_ptr = (int *)ptr;
  int *index = (int *)arg;
  *i_ptr = *index;
  *index += 1;
}

void
apply_func_with_arg_to_ptrs (void)
{
  // Allocates 8 sequential pointers, applies a function that sets all their
  // values to 1, 2, 3... depending on their position.
  size_t num_ptrs = 8;
  uintptr_t *ptrs = calloc (num_ptrs, sizeof (uintptr_t *));
  uintptr_t start = (uintptr_t)ptrs;
  uintptr_t end = start + num_ptrs * sizeof (uintptr_t *);
  int num = 1;
  apply_to_pointers_in_interval (
      start, end, (apply_to_ptr_func *)set_ptr_val_to_index, &num);

  // Verify that the pointers have been given their correct values.
  int index = 1;
  for (uintptr_t *i = ptrs; (uintptr_t)i < end; i++)
    {
      int num = (int)*i;
      CU_ASSERT_EQUAL (num, index);
      index++;
    }
  free (ptrs);
}

void
delete_heap_with_debug_val (void)
{
  size_t size = 32; // Last address in heap will be Heap start + 31
  heap_t *heap = h_init (size, false, 0);
  uintptr_t dbg_value = DEBUG_VAL;

  uintptr_t num = 90;
  uintptr_t *non_heap_ptr = &num;
  uintptr_t *heap_ptr = (uintptr_t *)(heap->heap_start) + 1; // Heap start + 8
  uintptr_t *heap_ptr2
      = (uintptr_t *)(heap->heap_start) + 2; // Heap start + 16
  uintptr_t *heap_ptr3
      = (uintptr_t *)(heap->heap_start) + 3; // Heap start + 24
  uintptr_t *non_heap_ptr2
      = (uintptr_t *)(heap->heap_start) + 4; // Heap start + 32 (not in heap)

  h_delete_dbg (heap, &dbg_value);

  CU_ASSERT_NOT_EQUAL (num, dbg_value);
  CU_ASSERT_EQUAL (*non_heap_ptr,
                   num); // Check that non_heap_ptr is not modified
  CU_ASSERT_NOT_EQUAL ((uintptr_t)non_heap_ptr, dbg_value);
  CU_ASSERT_EQUAL ((uintptr_t)heap_ptr, dbg_value);
  CU_ASSERT_EQUAL ((uintptr_t)heap_ptr2, dbg_value);
  CU_ASSERT_EQUAL ((uintptr_t)heap_ptr3, dbg_value);
  CU_ASSERT_NOT_EQUAL ((uintptr_t)non_heap_ptr2,
                       dbg_value); // Check that non_heap_ptr2 is not modified
}

void
test_h_avail (void)
{
  size_t h_size = 512;
  heap_t *h = h_init (h_size, true, 1);
  CU_ASSERT_EQUAL (h_avail (h), h_size); // No bytes should be allocated.

  char *alloc_map = h->alloc_map;
  alloc_map[0] = 0x1; // Imitates allocation of 16 bytes.
  CU_ASSERT_EQUAL (h_avail (h), h_size - 16);

  alloc_map[0] = 0x3; // Imitates allocation of 16 more bytes.
  CU_ASSERT_EQUAL (h_avail (h), h_size - 32);

  alloc_map[1] = 0x3; // Imitates allocation of 32 more bytes.
  CU_ASSERT_EQUAL (h_avail (h), h_size - 64);

  h_delete (h);
}

void
test_h_used (void)
{
  heap_t *h = h_init (512, true, 1);
  CU_ASSERT_EQUAL (h_used (h), 0); // No bytes should be allocated.

  h->used_bytes += 16; // Imitates allocation of 16 bytes.
  CU_ASSERT_EQUAL (h_used (h), 16);

  h->used_bytes += 16; // Imitates allocation of 16 more bytes.
  CU_ASSERT_EQUAL (h_used (h), 32);

  h->used_bytes += 32; // Imitates allocation of 32 more bytes.
  CU_ASSERT_EQUAL (h_used (h), 64);

  h_delete (h);
}
void
mark_pages_with_safe_stack_test (void)
{
  heap_t *h
      = h_init (512, false, 0.5); // Creates heap with unsafe_stack as false.

  CU_ASSERT_TRUE (is_offset_movable (h->page_map, 8));

  char *format_str1 = "i**";
  void *unsafe_ptr
      = h_alloc_struct (h, format_str1); // Make the allocation of 32 bytes.
  h_gc (h);
  size_t offset = calc_heap_offset ((char *)unsafe_ptr, h);
  CU_ASSERT_TRUE (is_offset_movable (
      h->page_map, offset)); // Check that the page is not affected.
  CU_ASSERT_TRUE (is_offset_movable (
      h->page_map, PAGE_SIZE)); // Check that the next page is not affected.

  h_delete (h);
}

void
mark_pages_with_unsafe_stack_test (void)
{
  heap_t *h
      = h_init (512, true, 0.5); // Creates heap with unsafe_stack as true.

  CU_ASSERT_TRUE (is_offset_movable (h->page_map, sizeof (header_t)));

  char *format_str1 = "i**";
  void *unsafe_ptr
      = h_alloc_struct (h, format_str1); // Make the allocation of 32 bytes.
  h_gc (h);
  size_t offset = calc_heap_offset ((char *)unsafe_ptr, h);
  CU_ASSERT_FALSE (is_offset_movable (
      h->page_map, offset)); // Check that the page is marked as not movable.
  CU_ASSERT_TRUE (is_offset_movable (
      h->page_map, PAGE_SIZE)); // Check that the next page is not affected.
  h_delete (h);
}

/* tests for GC with unsafe_stack setting */

void
unsafe_heap_safe_gc ()
{
  /* Creates heap with unsafe_stack as true. */
  heap_t *h = h_init (512, true, 0.5);

  size_t out_of_heap_offset
      = 5000; // An offset to add to copies of stack pointers to make them not
              // land in the heap and be affected by gc.

  char *format_str1 = "i**";
  /* allocate struct to be garbage collected */
  h_alloc_struct (h, format_str1);

  /* allocate struct to be moved during collection */
  void *stack_ptr = h_alloc_struct (h, format_str1);

  char *stack_ptr_copy = (char *)stack_ptr + out_of_heap_offset;

  /* perform garbage collection counting stack pointers as safe */
  h_gc_dbg (h, false);

  CU_ASSERT_PTR_NOT_EQUAL (stack_ptr, stack_ptr_copy - out_of_heap_offset);
  h_delete (h);
}

void
unsafe_heap_unsafe_gc ()
{
  /* Creates heap with unsafe_stack as true. */
  heap_t *h = h_init (512, true, 0.5);

  size_t out_of_heap_offset
      = 5000; // An offset to add to copies of stack pointers to make them not
              // land in the heap and be affected by gc.

  char *format_str1 = "i**";
  /* allocate struct to be garbage collected */
  h_alloc_struct (h, format_str1);

  /* allocate struct to be unmoved during collection */
  void *stack_ptr = h_alloc_struct (h, format_str1);

  char *stack_ptr_copy = (char *)stack_ptr + out_of_heap_offset;

  /* perform garbage collection counting stack pointers as unsafe */
  h_gc_dbg (h, true);

  CU_ASSERT_PTR_EQUAL (stack_ptr, stack_ptr_copy - out_of_heap_offset);
  h_delete (h);
}

void
safe_heap_safe_gc ()
{
  /* Creates heap with unsafe_stack as false. */
  heap_t *h = h_init (512, false, 0.5);

  char *format_str1 = "i**";
  size_t out_of_heap_offset
      = 5000; // An offset to add to copies of stack pointers to make them not
              // land in the heap and be affected by gc.
  /* allocate struct to be garbage collected */
  h_alloc_struct (h, format_str1);

  /* allocate struct to be unmoved during collection */
  void *stack_ptr = h_alloc_struct (h, format_str1);

  char *stack_ptr_copy = (char *)stack_ptr + out_of_heap_offset;

  /* perform garbage collection counting stack pointers as safe */
  h_gc_dbg (h, false);

  CU_ASSERT_PTR_NOT_EQUAL (stack_ptr, stack_ptr_copy - out_of_heap_offset);
  h_delete (h);
}

void
safe_heap_unsafe_gc ()
{
  /* Creates heap with unsafe_stack as false. */
  heap_t *h = h_init (512, false, 0.5);

  char *format_str1 = "i**";
  /* allocate struct to be garbage collected */
  h_alloc_struct (h, format_str1);

  /* allocate struct to be unmoved during collection */
  void *stack_ptr = h_alloc_struct (h, format_str1);

  char *stack_ptr_copy = (char *)stack_ptr + 1;

  /* perform garbage collection counting stack pointers as unsafe */
  h_gc_dbg (h, true);

  CU_ASSERT_PTR_EQUAL (stack_ptr, stack_ptr_copy - 1);
  h_delete (h);
}

int
main (void)
{
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  CU_pSuite suite = CU_add_suite ("h_gc tests", init_suite, clean_suite);
  CU_pSuite dbg_suite
      = CU_add_suite ("h_gc_dbg tests", init_suite, clean_suite);

  if (suite == NULL || dbg_suite == NULL)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  if ((CU_add_test (
           suite,
           "Create and delete heap with zero size, Gives heap larger than 0",
           create_heap_zero_size_test)
       == NULL)
      || (CU_add_test (suite,
                       "Create and delete heap with non-zero size, Gives heap "
                       "larger or "
                       "equal to size",
                       create_heap_nonzero_size_test)
          == NULL)
      || (CU_add_test (suite,
                       "Create and deletes heap with size divisible by "
                       "alignment, Gives "
                       "exactly sized heap",
                       create_heap_size_divisible_by_align_test)
          == NULL)
      || (CU_add_test (suite, "Apply function to pointers", apply_func_to_ptrs)
          == NULL)
      || (CU_add_test (suite, "Apply function with an argument to pointers",
                       apply_func_with_arg_to_ptrs)
          == NULL)
      || (CU_add_test (
              suite, "Delete heap and replace stack pointers with debug value",
              delete_heap_with_debug_val)
          == NULL)
      || (CU_add_test (suite, "Test usage of h_avail", test_h_avail) == NULL)
      || (CU_add_test (suite, "Test usage of h_used", test_h_used) == NULL)
      || (CU_add_test (suite,
                       "Test that pages are correctly marked during GC when "
                       "the stack is considered unsafe",
                       mark_pages_with_unsafe_stack_test)
          == NULL)
      || (CU_add_test (suite,
                       "Test that pages are correctly marked during GC when "
                       "the stack is considered safe",
                       mark_pages_with_safe_stack_test)
          == NULL)
      || (CU_add_test (
              dbg_suite,
              "Test safe stack pointer h_gc_dbg on heap with unsafe pointers",
              unsafe_heap_safe_gc)
          == NULL)
      || (CU_add_test (dbg_suite,
                       "Test unsafe stack pointer h_gc_dbg on heap with "
                       "unsafe pointers",
                       unsafe_heap_unsafe_gc)
          == NULL)
      || (CU_add_test (dbg_suite,
                       "Test safe stack pointer h_gc_dbg on heap with "
                       "safe pointers",
                       safe_heap_safe_gc)
          == NULL)
      || (CU_add_test (dbg_suite,
                       "Test unsafe stack pointer h_gc_dbg on heap with "
                       "safe pointers",
                       safe_heap_unsafe_gc)
          == NULL)
      || 0)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  CU_basic_set_mode (CU_BRM_VERBOSE);

  CU_basic_run_tests ();

  int exit_code = CU_get_number_of_tests_failed () == 0
                      ? CU_get_error ()
                      : CU_get_number_of_tests_failed ();

  CU_cleanup_registry ();

  return exit_code;
}

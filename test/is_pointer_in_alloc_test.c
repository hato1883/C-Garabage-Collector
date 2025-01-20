#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "../src/allocation_map.h"
#include "../src/gc.h"
#include "../src/heap_internal.h"
#include "../src/is_pointer_in_alloc.h"

void
test_is_in_range (void)
{
  heap_t *heap = h_init (1024, false, 1);

  /* Inside the heap's range  */
  uintptr_t inside_ptr = (uintptr_t)heap->heap_start + 100;
  /* Outside the heap's range  */
  uintptr_t outside_ptr = (uintptr_t)heap->heap_start + 1500;

  CU_ASSERT_TRUE (is_in_range (inside_ptr, heap));
  CU_ASSERT_FALSE (is_in_range (outside_ptr, heap));
  h_delete (heap);
}

void
test_is_in_allocated_region (void)
{
  heap_t *heap = h_init (1024, false, 1);

  /* Mock some allocations in the alloc_map  */
  for (size_t i = 0; i < heap->size / BYTE_DENSITY; i++)
    {
      /* Even index is allocated  */
      ((uint8_t *)heap->alloc_map)[i] = (i % 2 == 0) ? 0xFF : 0x0;
    }

  uintptr_t inside_alloc_ptr = (uintptr_t)heap->heap_start + 100;
  uintptr_t outside_alloc_ptr = (uintptr_t)heap->heap_start + 200;

  CU_ASSERT_TRUE (is_in_allocated_region (inside_alloc_ptr, heap));
  CU_ASSERT_FALSE (is_in_allocated_region (outside_alloc_ptr, heap));
  h_delete (heap);
}

void
test_is_pointer (void)
{
  heap_t *heap = h_init (1024, false, 1);

  /* Mock some allocations in the alloc_map  */
  for (size_t i = 0; i < heap->size / BYTE_DENSITY; i++)
    {
      ((uint8_t *)heap->alloc_map)[i] = (i % 2 == 0) ? 0xFF : 0x0;
    }

  uintptr_t inside_and_allocated = (uintptr_t)heap->heap_start + 100;
  uintptr_t inside_but_not_allocated = (uintptr_t)heap->heap_start + 200;
  uintptr_t outside = (uintptr_t)heap->heap_start + 1500;

  CU_ASSERT_TRUE (is_heap_pointer (inside_and_allocated, heap));
  CU_ASSERT_FALSE (is_heap_pointer (inside_but_not_allocated, heap));
  CU_ASSERT_FALSE (is_heap_pointer (outside, heap));

  h_delete (heap);
}

void
test_is_pointer_edges (void)
{
  heap_t *heap = h_init (1024, false, 1);

  ((uint8_t *)heap->alloc_map)[0] = 0x01; // range 0-127    = 0000 0001
  ((uint8_t *)heap->alloc_map)[1] = 0x00; // range 128-255  = 0000 0000
  ((uint8_t *)heap->alloc_map)[2] = 0x00; // range 256-383  = 0000 0000
  ((uint8_t *)heap->alloc_map)[3] = 0x00; // range 384-511  = 0000 0000
  ((uint8_t *)heap->alloc_map)[4] = 0x00; // range 512-639  = 0000 0000
  ((uint8_t *)heap->alloc_map)[5] = 0x00; // range 640-767  = 0000 0000
  ((uint8_t *)heap->alloc_map)[6] = 0x00; // range 768-895  = 0000 0000
  ((uint8_t *)heap->alloc_map)[7] = 0x80; // range 986-1023 = 1000 0000

  for (size_t i = 0; i < 16; i++)
    {
      uintptr_t inside_and_allocated_start = (uintptr_t)heap->heap_start + i;
      CU_ASSERT_TRUE (is_heap_pointer (inside_and_allocated_start, heap));
    }

  for (size_t i = 16; i < 1008; i++)
    {
      uintptr_t inside_but_not_allocated = (uintptr_t)heap->heap_start + i;
      CU_ASSERT_FALSE (is_heap_pointer (inside_but_not_allocated, heap));
    }

  for (size_t i = 0; i < 16; i++)
    {
      uintptr_t inside_and_allocated_end
          = (uintptr_t)heap->heap_start + 1008 + i;
      CU_ASSERT_TRUE (is_heap_pointer (inside_and_allocated_end, heap));
    }

  uintptr_t outside_before = (uintptr_t)heap->heap_start - 1;
  CU_ASSERT_FALSE (is_heap_pointer (outside_before, heap));

  uintptr_t outside_after = (uintptr_t)heap->heap_start + 1024;
  CU_ASSERT_FALSE (is_heap_pointer (outside_after, heap));

  h_delete (heap);
}

/* Main function to run the tests  */
int
main ()
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite pSuite = CU_add_suite ("Pointer Validation Tests", 0, 0);
  if (pSuite == NULL)
    {
      // If the test suite could not be added, tear down CUnit and exit
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  // This is where we add the test functions to our test suite.
  // For each call to CU_add_test we specify the test suite, the
  // name or description of the test, and the function that runs
  // the test in question. If you want to add another test, just
  // copy a line below and change the information
  if ((CU_add_test (pSuite, "Test is_in_range", test_is_in_range) == NULL)
      || (CU_add_test (pSuite, "Test is_in_allocated_region",
                       test_is_in_allocated_region)
          == NULL)
      || (CU_add_test (pSuite, "Test is_heap_pointer", test_is_pointer)
          == NULL)
      || (CU_add_test (pSuite, "Test is_heap_pointer with edge values",
                       test_is_pointer_edges)
          == NULL)
      || 0)
    {
      // If adding any of the tests fails, we tear down CUnit and exit
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  // Set the running mode. Use CU_BRM_VERBOSE for maximum output.
  // Use CU_BRM_NORMAL to only print errors and a summary
  CU_basic_set_mode (CU_BRM_VERBOSE);

  // This is where the tests are actually run!
  CU_basic_run_tests ();

  int exit_code = CU_get_number_of_tests_failed () == 0
                      ? CU_get_error ()
                      : CU_get_number_of_tests_failed ();

  // Tear down CUnit before exiting
  CU_cleanup_registry ();

  return exit_code;
}

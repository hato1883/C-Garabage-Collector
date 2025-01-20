#include <CUnit/Basic.h>
#include <stdlib.h>
#include <string.h>

#include "../src/allocation_map.h"
#include "../src/gc.h"

/* Minimum object size in bytes. */
#define MIN_ALLOC_OBJECT_SIZE 16
/* Defines how much memory is represented by a boolean in the map.  */
#define BOOLEANS_PER_BYTE 8
/* Defines how much memory is represented by a byte in the map.  */
#define BYTE_DENSITY (MIN_ALLOC_OBJECT_SIZE * BOOLEANS_PER_BYTE)
#include "../src/gc.h"

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

void
test_allocation_map_create (void)
{
  size_t bytes = 2048;

  char *alloc_map = create_allocation_map (bytes);
  CU_ASSERT_PTR_NOT_NULL (alloc_map);

  size_t alloc_map_size = bytes / BYTE_DENSITY;

  // Each byte in allocation map should be empty (0)
  for (size_t i = 0; i < alloc_map_size; i++)
    {
      CU_ASSERT_EQUAL (alloc_map[i], 0);
    }

  free (alloc_map);
}

void
test_allocation_map_indexing (void)
{
  size_t bytes = 2048;

  char *alloc_map = create_allocation_map (bytes);
  CU_ASSERT_PTR_NOT_NULL (alloc_map);

  /* set first allocation to ALLOCATED */
  alloc_map[0] = 0x1;

  /* set third allocation to ALLOCATED */
  alloc_map[0] = alloc_map[0] | 0x4;

  /* Check if first allocation bit is set to 1 */
  for (size_t i = 0; i < 16; i++)
    {
      CU_ASSERT_NOT_EQUAL (alloc_map[find_index_in_alloc_map (16 * 0 + i)]
                               & create_bitmask (16 * 0 + i),
                           0);
    }

  /* Check if second allocation bit is set to 0 */
  for (size_t i = 0; i < 16; i++)
    {
      CU_ASSERT_EQUAL (alloc_map[find_index_in_alloc_map (16 * 1 + i)]
                           & create_bitmask (16 * 1 + i),
                       0);
    }

  /* Check if third allocation bit is set to 1 */
  CU_ASSERT_NOT_EQUAL (alloc_map[find_index_in_alloc_map (16 * 2)]
                           & create_bitmask (16 * 2),
                       0);

  /* Check if fourth - eighth allocation bit is set to 0 */
  for (size_t i = 3; i < 8; i++)
    {
      CU_ASSERT_EQUAL (alloc_map[find_index_in_alloc_map (16 * i)]
                           & create_bitmask (16 * i),
                       0);
    }

  free (alloc_map);
}

void
test_large_alloc_map (void)
{
  size_t xxl_bytes = 5000 * 16;
  char *alloc_map = create_allocation_map (xxl_bytes);
  CU_ASSERT_PTR_NOT_NULL (alloc_map);

  size_t alloc_map_size = xxl_bytes / BYTE_DENSITY;
  // Each byte in allocation map should be empty (0)
  for (size_t i = 0; i < alloc_map_size; i++)
    {
      CU_ASSERT_EQUAL (alloc_map[i], 0);
    }
  free (alloc_map);
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite allocationmaptests
      = CU_add_suite ("Allocation map Testing Suite", init_suite, clean_suite);
  if (allocationmaptests == NULL)
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
  if ((CU_add_test (allocationmaptests, "Test creating an allocation map",
                    test_allocation_map_create)
           == NULL
       || CU_add_test (allocationmaptests, "Test very large amount of bytes",
                       test_large_alloc_map)
              == NULL
       || CU_add_test (allocationmaptests,
                       "Test retrieving index and bit from offset",
                       test_allocation_map_indexing)
              == NULL
       || 0))
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

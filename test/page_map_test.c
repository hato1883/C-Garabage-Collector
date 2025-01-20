#include <CUnit/Basic.h>
#include <stdlib.h>
#include <string.h>

#include "../src/gc.h"
#include "../src/page_map.h"

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
test_page_map_create (void)
{
  size_t bytes = 2048;

  char *page_map = create_page_map (bytes);
  CU_ASSERT_PTR_NOT_NULL (page_map);

  // Each bit in page map should be 1,
  // therefore each byte should be 0xFF
  CU_ASSERT_EQUAL ((unsigned char)page_map[0], 0xFF);
  for (size_t i = 0; i < PAGE_DENSITY; i++)
    {
      CU_ASSERT_TRUE (is_offset_movable (page_map, i));
    }

  free (page_map);
}

void
test_page_map_indexing (void)
{
  size_t bytes = 32768;

  char *page_map = create_page_map (bytes);
  CU_ASSERT_PTR_NOT_NULL (page_map);

  /* set first page to unmovable */
  update_page_map (page_map, 0, false);

  /* set third page to unmovable */
  update_page_map (page_map, 2 * 2048, false);

  /* Check if first page bit is set to 0 (unmovable) */
  for (size_t i = 0; i < 2048; i++)
    {
      CU_ASSERT_EQUAL (page_map[find_index_in_page_map (2048 * 0 + i)]
                           & create_page_bitmask (2048 * 0 + i),
                       0);
    }

  /* Check if second page bit is set to 1 (movable) */
  for (size_t i = 0; i < 2048; i++)
    {
      CU_ASSERT_NOT_EQUAL (page_map[find_index_in_page_map (2048 * 1 + i)]
                               & create_page_bitmask (2048 * 1 + i),
                           0);
    }

  /* Check if third page bit is set to 0 (unmovable) */
  CU_ASSERT_EQUAL (page_map[find_index_in_page_map (2048 * 2)]
                       & create_page_bitmask (2048 * 2),
                   0);

  /* Check if fourth - eighth page bit is set to 1 (movable) */
  for (size_t i = 3; i < 8; i++)
    {
      CU_ASSERT_NOT_EQUAL (page_map[find_index_in_page_map (2048 * i)]
                               & create_page_bitmask (2048 * i),
                           0);
    }

  free (page_map);
}

void
test_multi_page_map (void)
{
  size_t xxl_bytes = 64 * 2048;
  char *page_map = create_page_map (xxl_bytes);
  CU_ASSERT_PTR_NOT_NULL (page_map);

  size_t page_map_size = xxl_bytes / PAGE_DENSITY;
  // Each byte in page map should be filled (0xFF)
  for (size_t i = 0; i < page_map_size; i++)
    {
      CU_ASSERT_EQUAL ((unsigned char)page_map[i], 0xFF);
    }
  free (page_map);
}

void
test_reset_page_map (void)
{
  size_t heap_size = 64 * 2048;
  char *page_map = create_page_map (heap_size);

  size_t offset = 1;
  /* Check that the first page is moveable */
  for (size_t i = 0; i < PAGE_SIZE; i++)
    {
      CU_ASSERT_TRUE (is_offset_movable (page_map, i));
    }

  update_page_map (page_map, offset, false);
  /* Check that the first page is not moveable */
  for (size_t i = 0; i < PAGE_SIZE; i++)
    {
      CU_ASSERT_FALSE (is_offset_movable (page_map, i));
    }

  update_page_map (page_map, offset, true);
  /* Check that the first page is not moveable */
  for (size_t i = 0; i < 32; i++)
    {
      CU_ASSERT_TRUE (is_offset_movable (page_map, i));
    }

  size_t map_size = heap_size / PAGE_DENSITY;
  reset_page_map (page_map, map_size); // Resets the page map

  /* Check that the first page is moveable */
  for (size_t i = 0; i < PAGE_SIZE; i++)
    {
      CU_ASSERT_TRUE (is_offset_movable (page_map, i));
    }

  free (page_map);
}

void
test_calc_map_size (void)
{
  CU_ASSERT_EQUAL (calc_required_map_size (0), 0);

  // Test it for heaps with sizes 1 -> 8 * 2048
  for (size_t i = 1; i <= PAGE_DENSITY; i++)
    {
      CU_ASSERT_EQUAL (calc_required_map_size (i), 1);
    }

  // Test it for heaps with sizes 8 * 2048 + 1 -> 16 * 2048
  for (size_t i = PAGE_DENSITY + 1; i <= 2 * PAGE_DENSITY; i++)
    {
      CU_ASSERT_EQUAL (calc_required_map_size (i), 2);
    }

  // Test it for heaps with sizes 16 * 2048 + 1 -> 24 * 2048
  for (size_t i = 2 * PAGE_DENSITY + 1; i <= 3 * PAGE_DENSITY; i++)
    {
      CU_ASSERT_EQUAL (calc_required_map_size (i), 3);
    }
}
void
test_bytes_from_next_page ()
{
  size_t page_size = 2048;
  // Test for offsets within first page.
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, 0), page_size);
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, 1), page_size - 1);
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, page_size - 1), 1);

  // Test for offsets within second page.
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, page_size), page_size);
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, page_size + 1),
                   page_size - 1);
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, 2 * page_size - 1), 1);

  // Test for offsets within third page.
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, 2 * page_size), page_size);
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, 2 * page_size + 1),
                   page_size - 1);
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, 3 * page_size - 1), 1);
}

void
test_move_offset_to_next_page ()
{
  size_t page_size = 2048;
  size_t offset = 9; // Starts inside of page.
  offset = offset + bytes_from_next_page (page_size, offset);

  // Check that offset is moved to beginning of next page.
  CU_ASSERT_EQUAL (offset, page_size);
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, offset), page_size);

  size_t offset2 = page_size + 305; // Starts inside of page.
  offset2 = offset2 + bytes_from_next_page (page_size, offset2);

  // Check that offset2 is moved to beginning of next page.
  CU_ASSERT_EQUAL (offset2, 2 * page_size);
  CU_ASSERT_EQUAL (bytes_from_next_page (page_size, offset2), page_size);
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite pagemaptests
      = CU_add_suite ("page map Testing Suite", init_suite, clean_suite);
  if (pagemaptests == NULL)
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
  if ((CU_add_test (pagemaptests, "Test creating an page map",
                    test_page_map_create)
       == NULL)
      || (CU_add_test (pagemaptests, "Test very large amount of bytes",
                       test_multi_page_map)
          == NULL)
      || (CU_add_test (pagemaptests,
                       "Test retrieving index and bit from offset",
                       test_page_map_indexing)
          == NULL)
      || (CU_add_test (pagemaptests, "Test resetting of page map",
                       test_reset_page_map)
          == NULL)
      || (CU_add_test (pagemaptests, "Test calculation of map size",
                       test_calc_map_size)
          == NULL)
      || (CU_add_test (pagemaptests,
                       "Tests function that gets number of bytes until the "
                       "next page begins",
                       test_bytes_from_next_page)
          == NULL)
      || (CU_add_test (pagemaptests,
                       "Check that bytes_until_next_page can be used to move "
                       "to start of next page.",
                       test_move_offset_to_next_page)
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

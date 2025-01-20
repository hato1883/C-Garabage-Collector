#include <CUnit/Basic.h>
#include <stdlib.h>

#include "../src/allocation.h"
#include "../src/allocation_map.h"
#include "../src/gc.h"
#include "../src/gc_utils.h"
#include "../src/get_header.h"
#include "../src/heap_internal.h"
#include "../src/is_pointer_in_alloc.h"

int
init_allocation_suite (void)
{
  // Change this function if you want to do something *before* you
  // run a test suite
  return 0;
}

int
clean_allocation_suite (void)
{
  // Change this function if you want to do something *after* you
  // run a test suite
  return 0;
}

void
alloc_struct_with_format (void)
{
  heap_t *h = h_init (512, true, 0.5);

  char *format_str24 = "i**"; // 24 bytes
  void *alloc1
      = h_alloc_struct (h, format_str24); // Make the allocation of 32 bytes.
  CU_ASSERT_PTR_NOT_NULL (alloc1);
  CU_ASSERT_PTR_EQUAL (
      (char *)alloc1,
      (char *)h->heap_start
          + sizeof (header_t)) // Check that the header is accounted for

  char *end_of_alloc1 = (char *)alloc1 + 48;
  for (char *ptr = (char *)alloc1; ptr < end_of_alloc1; ptr++)
    {
      CU_ASSERT_EQUAL (
          *(int *)ptr,
          0); // Check that the allocated memory is all set to 0 like calloc
    }
  // Check that the next_empty_mem_segment is right after the allocated struct
  CU_ASSERT_PTR_EQUAL ((char *)h->next_empty_mem_segment, (char *)alloc1 + 24);

  void *prev_start = h->next_empty_mem_segment;

  char *format_str56 = "2d4*";
  void *alloc2 = h_alloc_struct (h, format_str56);
  CU_ASSERT_PTR_NOT_NULL (alloc2);
  CU_ASSERT_PTR_EQUAL (
      (char *)alloc2,
      (char *)prev_start
          + sizeof (header_t)) // Check that the header is accounted for

  char *end_of_alloc2 = (char *)alloc2 + 56;
  for (char *ptr = (char *)alloc2; ptr < end_of_alloc2; ptr++)
    {
      CU_ASSERT_EQUAL (
          *(int *)ptr,
          0); // Check that the allocated memory is all set to 0 like calloc
    }
  // Check that the next_empty_mem_segment is right after the allocated struct
  CU_ASSERT_PTR_EQUAL ((char *)h->next_empty_mem_segment, (char *)alloc2 + 56);

  h_delete (h);
}

void
alloc_struct_too_big_for_vector ()
{
  heap_t *h = h_init (512, true, 0.5);

  char *format_big_struct = "40*";

  void *struct_p = h_alloc_struct (h, format_big_struct);

  CU_ASSERT_PTR_NOT_NULL (struct_p);

  header_t header = (get_header_value (get_header_pointer (struct_p)));
  void *p = get_pointer_in_header (header);

  CU_ASSERT_TRUE (is_heap_pointer ((uintptr_t)p, h));
  CU_ASSERT_PTR_NOT_EQUAL (format_big_struct, p);
  CU_ASSERT_STRING_EQUAL (format_big_struct, p);

  // Another allocation to make sure nothing gets messed up
  char *s = h_alloc_raw (h, 7);
  CU_ASSERT_PTR_NOT_NULL (s);

  CU_ASSERT_STRING_EQUAL (format_big_struct, p);

  h_delete (h);
}

void
check_alloc_map_marked (heap_t *h, char *ptr)
{
  size_t alloc_map_offset = calc_heap_offset (ptr, h);
  size_t map_index = find_index_in_alloc_map (alloc_map_offset);
  size_t bitmask = create_bitmask (alloc_map_offset);
  char *alloc_map = (char *)h->alloc_map;
  char byte = alloc_map[map_index];
  CU_ASSERT_EQUAL (byte & bitmask, bitmask);
}

void
alloc_struct_too_big_before_header (void)
{
  heap_t *h = h_init (512, true, 0.5);
  void *struct_p = h_alloc_struct (h, "18446744073709551616");
  CU_ASSERT_PTR_NULL (struct_p);
  h_delete (h);
}

void
alloc_struct_too_big_after_header (void)
{
  heap_t *h = h_init (16, true, 0.5);
  void *struct_p = h_alloc_struct (h, "30*cccccccccccccccccccccc");
  CU_ASSERT_PTR_NULL (struct_p);
  h_delete (h);
}

void
alloc_raw_too_big_before_header (void)
{
  heap_t *h = h_init (512, true, 0.5);
  void *raw_p = h_alloc_raw (h, (size_t)4611686018427387902);
  CU_ASSERT_PTR_NULL (raw_p);
  h_delete (h);
}

void
alloc_raw_too_big_after_header (void)
{
  heap_t *h = h_init (512, true, 0.5);
  void *raw_p = h_alloc_raw (h, 508);
  CU_ASSERT_PTR_NULL (raw_p);
  h_delete (h);
}

void
alloc_struct_updates_alloc_map (void)
{
  heap_t *h = h_init (512, true, 0.5);

  char *format_str1 = "i**";
  void *alloc1
      = h_alloc_struct (h, format_str1); // Make the allocation of 32 bytes.
  char *end_of_alloc1 = (char *)alloc1 + 32;

  for (char *ptr = (char *)alloc1; ptr < end_of_alloc1; ptr += 16)
    {
      // For each 16 bytes in the allocation, check that the corresponding bit
      // in the alloc_map is 1.
      check_alloc_map_marked (h, ptr);
    }

  char *format_str2 = "3d4*";
  void *alloc2 = h_alloc_struct (h, format_str2);

  char *end_of_alloc2 = (char *)alloc2 + 64;
  for (char *ptr = (char *)alloc2; ptr < end_of_alloc2; ptr += 16)
    {
      // For each 16 bytes in the allocation, check that the corresponding bit
      // in the alloc_map is 1.
      check_alloc_map_marked (h, ptr);
    }
  h_delete (h);
}

void
alloc_struct_with_invalid_format (void)
{
  heap_t *h = h_init (32, true, 0.5);
  char *invalid_format = "256*";

  void *alloc = h_alloc_struct (h, invalid_format);
  CU_ASSERT_PTR_NULL (alloc);

  char *too_large_format = "3d4*";
  void *alloc2 = h_alloc_struct (h, too_large_format);
  CU_ASSERT_PTR_NULL (alloc2);

  h_delete (h);
}

void
alloc_struct_updates_used_bytes (void)
{
  heap_t *h = h_init (512, true, 0.5);
  CU_ASSERT_EQUAL (h_used (h), 0);

  char *format_str24 = "i**";
  h_alloc_struct (h, format_str24); // Make the allocation of 32 bytes.
  CU_ASSERT_EQUAL (h_used (h), 24);

  char *format_str56 = "3d4*";
  h_alloc_struct (h, format_str56); // Make the allocation of 56 bytes.
  CU_ASSERT_EQUAL (h_used (h), 24 + 56);

  h_delete (h);
}

void
alloc_struct_over_page_limits (void)
{
  size_t page_size = 2048;
  heap_t *h = h_init (3 * page_size, true, 0.5);

  // Allocation of size 264 + 8;
  char *format_str264
      = "33l"; // Note: long formatstrings with pointers require more space.
  for (size_t i = 0; i < 7; i++)
    {
      h_alloc_struct (h, format_str264);
    }
  CU_ASSERT_EQUAL (h_used (h), 1848); // Including headers 1904
  size_t offset = calc_heap_offset (h->next_empty_mem_segment, h);

  CU_ASSERT_EQUAL (offset, 1904);

  // Allocate over page size
  h_alloc_struct (h, format_str264);
  CU_ASSERT_EQUAL (h_used (h), 2112);
  size_t offset2 = calc_heap_offset (h->next_empty_mem_segment, h);
  CU_ASSERT_EQUAL (offset2, 272 + page_size);

  h_delete (h);
}

void
test_struct_move_to_next_available_space (void)
{
  // Tests that allocations are moved to end of blocking allocations if any
  // pre-existing allocations block the desired allocation
  heap_t *h = h_init (PAGE_SIZE + 1, true, 0.8);
  char *format_str56 = "3d4*";

  update_alloc_map (h->alloc_map, 16,
                    true); // Imitates allocation on heap offsets 16-32
  char *alloc1 = h_alloc_struct (h, format_str56);
  size_t offset1 = calc_heap_offset (alloc1, h);
  CU_ASSERT_EQUAL (offset1, 32 + 8);

  update_alloc_map (h->alloc_map, 96,
                    true); // Imitates allocation on heap offsets 96-112
  update_alloc_map (h->alloc_map, 128,
                    true); // Imitates allocation on heap offsets 128-144
  char *alloc2 = h_alloc_struct (h, format_str56);
  size_t offset2 = calc_heap_offset (alloc2, h);
  CU_ASSERT_EQUAL (offset2, 144 + 8);

  h_delete (h);
}

/**
 * Test the allocation over page limit check still works after allocation has
 * been moved due to a previous allocation blocking desired space.
 */
void
alloc_struct_over_page_limit_after_allocation_move (void)
{
  size_t page_size = 2048;
  heap_t *h = h_init (3 * page_size, true, 0.5);

  char *format_str264 = "33l"; // Allocation of size 264 + 8;

  // Allocates a total 1904 bytes (including header)
  for (size_t i = 0; i < 7; i++)
    {
      h_alloc_struct (h, format_str264);
    }

  update_alloc_map (h->alloc_map, 2000,
                    true); // Imitates allocation on heap offsets 2000-2016
  // Allocation initially moves allocation to begin at 2016, but then page
  // limit check should move it to the next page (offset 2048).
  update_alloc_map (h->alloc_map, 2048,
                    true); // Imitates allocation on heap offsets 2048-2064,
                           // this should force allocation to be made 16 bytes
                           // from the start of the next page.
  h_alloc_struct (h, format_str264);
  CU_ASSERT_EQUAL (h_used (h), 2112);
  size_t offset2 = calc_heap_offset (h->next_empty_mem_segment, h);
  CU_ASSERT_EQUAL (offset2, 272 + 16 + page_size);

  h_delete (h);
}

void
test_alloc_raw (void)
{
  heap_t *h = h_init (512, true, 0.5);

  void *alloc1 = h_alloc_raw (h, 16); // Aligned to 24
  CU_ASSERT_PTR_NOT_NULL (alloc1);
  CU_ASSERT_EQUAL (h_used (h), 24);
  CU_ASSERT_TRUE (is_offset_allocated (h->alloc_map, 0))

  size_t offset = calc_heap_offset (h->next_empty_mem_segment, h);
  CU_ASSERT_EQUAL (offset, 24 + sizeof (header_t))

  void *alloc2 = h_alloc_raw (h, 100); // Should be aligned to 104
  CU_ASSERT_PTR_NOT_NULL (alloc2);
  CU_ASSERT_EQUAL (h_used (h), 128);
  CU_ASSERT_TRUE (is_offset_allocated (h->alloc_map, 32))

  size_t offset2 = calc_heap_offset (h->next_empty_mem_segment, h);
  CU_ASSERT_EQUAL (offset2, 128 + 2 * sizeof (header_t))

  h_delete (h);
}

void
test_alloc_raw_too_big (void)
{
  heap_t *h = h_init (512, true, 0.5);

  void *alloc1
      = h_alloc_raw (h, 504 + 1); // Max size (aligned to 512 + 8 = 520)
  CU_ASSERT_PTR_NULL (alloc1);
  CU_ASSERT_EQUAL (h_used (h), 0);
  CU_ASSERT_FALSE (is_offset_allocated (h->alloc_map, 0));

  void *alloc_just_fits
      = h_alloc_raw (h, 504); // Max size (aligned to 504 + 8 = 512)
  CU_ASSERT_PTR_NOT_NULL (alloc_just_fits);

  h_delete (h);
}

void
test_alloc_bigger_than_page_size (void)
{
  size_t page_size = 2048;
  heap_t *h = h_init (2 * page_size, true, 0.5);
  h->page_size = page_size;

  void *alloc1 = h_alloc_raw (h, page_size);
  CU_ASSERT_PTR_NULL (alloc1);
  CU_ASSERT_EQUAL (h_used (h), 0);
  CU_ASSERT_FALSE (is_offset_allocated (h->alloc_map, 0))

  void *alloc2 = h_alloc_raw (h, page_size - sizeof (header_t) + 1);
  CU_ASSERT_PTR_NULL (alloc2);
  CU_ASSERT_EQUAL (h_used (h), 0);
  CU_ASSERT_FALSE (is_offset_allocated (h->alloc_map, 0))

  h_delete (h);
}

void
test_alloc_raw_heap_not_enough_space (void)
{
  heap_t *h = h_init (32, true, 0.5);

  void *alloc1 = h_alloc_raw (h, 25); // Aligned to 40 bytes (too big)
  CU_ASSERT_PTR_NULL (alloc1);
  CU_ASSERT_EQUAL (h_used (h), 0);
  CU_ASSERT_FALSE (is_offset_allocated (h->alloc_map, 0))

  void *alloc2 = h_alloc_raw (h, 24); // 24 + 8 bytes alloc
  CU_ASSERT_PTR_NOT_NULL (alloc2);
  CU_ASSERT_EQUAL (h_used (h), 24);

  void *alloc3 = h_alloc_raw (h, 0); //  8 + 8 bytes alloc; (too big)
  CU_ASSERT_PTR_NULL (alloc3);
  CU_ASSERT_EQUAL (h_used (h), 24);

  h_delete (h);
}

void
alloc_raw_over_page_limits (void)
{
  size_t page_size = 2048;
  heap_t *h = h_init (3 * page_size, true, 0.5);
  h->page_size = page_size;

  for (size_t i = 0; i < 7; i++)
    {
      h_alloc_raw (h, 264);
    }
  CU_ASSERT_EQUAL (h_used (h), 1848); // Including headers 1904
  size_t offset = calc_heap_offset (h->next_empty_mem_segment, h);

  CU_ASSERT_EQUAL (offset, 1904);

  // Allocate over page size
  h_alloc_raw (h, 264);
  CU_ASSERT_EQUAL (h_used (h), 2112);
  size_t offset2 = calc_heap_offset (h->next_empty_mem_segment, h);
  CU_ASSERT_EQUAL (offset2, 272 + page_size);

  h_delete (h);
}

void
test_raw_move_to_next_available_space (void)
{
  // Tests that allocations are moved to end of blocking allocations if any
  // pre-existing allocations block the desired allocation
  heap_t *h = h_init (PAGE_SIZE + 1, true, 0.8);

  update_alloc_map (h->alloc_map, 16,
                    true); // Imitates allocation on heap offsets 16-32
  char *alloc1 = h_alloc_raw (h, 64);
  size_t offset1 = calc_heap_offset (alloc1, h);
  CU_ASSERT_EQUAL (offset1, 32 + 8);

  update_alloc_map (h->alloc_map, 96,
                    true); // Imitates allocation on heap offsets 96-112
  update_alloc_map (h->alloc_map, 128,
                    true); // Imitates allocation on heap offsets 128-144
  char *alloc2 = h_alloc_raw (h, 64);
  size_t offset2 = calc_heap_offset (alloc2, h);
  ;
  CU_ASSERT_EQUAL (offset2, 144 + 8);

  h_delete (h);
}

/**
 * Test the allocation over page limit check still works after allocation has
 * been moved due to a previous allocation blocking desired space.
 */
void
alloc_raw_over_page_limit_after_allocation_move (void)
{
  size_t page_size = 2048;
  heap_t *h = h_init (3 * page_size, true, 0.5);

  // Allocates a total 1904 bytes (including header)
  for (size_t i = 0; i < 7; i++)
    {
      h_alloc_raw (h, 264);
    }

  update_alloc_map (h->alloc_map, 2000,
                    true); // Imitates allocation on heap offsets 2000-2016

  // Allocation initially moves allocation to begin at 2016, but then page
  // limit check should move it to the next page (offset 2048).
  update_alloc_map (h->alloc_map, 2048,
                    true); // Imitates allocation on heap offsets 2048-2064,
                           // this should force allocation to be made 16 bytes
                           // from the start of the next page.
  h_alloc_raw (h, 264);
  CU_ASSERT_EQUAL (h_used (h), 2112);
  size_t offset2 = calc_heap_offset (h->next_empty_mem_segment, h);
  CU_ASSERT_EQUAL (offset2, 272 + 16 + page_size);

  h_delete (h);
}

int
main ()
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite allocation_tests
      = CU_add_suite ("Allocation Testing Suite", init_allocation_suite,
                      clean_allocation_suite);
  if (allocation_tests == NULL)
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
  if ((CU_add_test (allocation_tests, "Allocate struct using format string",
                    alloc_struct_with_format)
       == NULL)
      || (CU_add_test (allocation_tests,
                       "Allocating a struct that is to large fails",
                       alloc_struct_too_big_before_header)
          == NULL)
      || (CU_add_test (allocation_tests,
                       "Allocating a struct that is to large after adding "
                       "size for header fails",
                       alloc_struct_too_big_after_header)
          == NULL)
      || (CU_add_test (allocation_tests, "Allocating too many bytes fails",
                       alloc_raw_too_big_before_header)
          == NULL)
      || (CU_add_test (
              allocation_tests,
              "Allocating too large number number of bytes with header",
              alloc_raw_too_big_after_header)
          == NULL)
      || (CU_add_test (
              allocation_tests,
              "Allocating struct using format string updates allocation map",
              alloc_struct_updates_alloc_map)
          == NULL)
      || (CU_add_test (
              allocation_tests,
              "Allocate struct using invalid or too large format string",
              alloc_struct_with_invalid_format)
          == NULL)
      || (CU_add_test (allocation_tests, "Allocate struct updates used_bytes",
                       alloc_struct_updates_used_bytes)
          == NULL)
      || (CU_add_test (allocation_tests,
                       "Allocating struct over page limits moves to start of "
                       "next page.",
                       alloc_struct_over_page_limits)
          == NULL)
      || (CU_add_test (allocation_tests,
                       "Test function to move bump pointer to next available "
                       "space when allocating structs.",
                       test_struct_move_to_next_available_space)
          == NULL)
      || (CU_add_test (
              allocation_tests,
              "Test the allocation over page limit check still works "
              "after allocation has been moved due to a previous "
              "allocation blocking desired space when allocating struct.",
              alloc_struct_over_page_limit_after_allocation_move)
          == NULL)
      || (CU_add_test (allocation_tests, "Test allocation using h_alloc_raw",
                       test_alloc_raw)
          == NULL)
      || (CU_add_test (allocation_tests,
                       "Test too large allotion with h_alloc_raw",
                       test_alloc_raw_too_big)
          == NULL)
      || (CU_add_test (allocation_tests,
                       "Test allocation bigger than PAGE_SIZE h_alloc_raw",
                       test_alloc_bigger_than_page_size)
          == NULL)
      || (CU_add_test (allocation_tests,
                       "Test allocation with h_alloc_raw does no fit",
                       test_alloc_raw_heap_not_enough_space)
          == NULL)
      || (CU_add_test (allocation_tests,
                       "Allocating raw over page limits moves to start of "
                       "next page.",
                       alloc_raw_over_page_limits)
          == NULL)
      || (CU_add_test (allocation_tests,
                       "Test function to move bump pointer to next available "
                       "space when allocating structs.",
                       test_raw_move_to_next_available_space)
          == NULL)
      || (CU_add_test (
              allocation_tests,
              "Test the allocation over page limit check still works "
              "after allocation has been moved due to a previous "
              "allocation blocking desired space when allocating struct.",
              alloc_raw_over_page_limit_after_allocation_move)
          == NULL)
      || (CU_add_test (allocation_tests,
                       "Allocation too big for a bit vector copies format "
                       "string to heap and puts pointer in header",
                       alloc_struct_too_big_for_vector)
          == NULL)
      || 0)
    {
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

#include <CUnit/Basic.h>
#include <math.h>
#include <stdlib.h>

#include "../src/allocation_map.h"
#include "../src/gc.h"
#include "../src/gc_utils.h"
#include "../src/get_header.h"
#include "../src/heap_internal.h"

int
init_compacting_suite (void)
{
  // Change this function if you want to do something *before* you
  // run a test suite
  return 0;
}

int
clean_compacting_suite (void)
{
  // Change this function if you want to do something *after* you
  // run a test suite
  return 0;
}

void
compacting_not_move_unsafe_page ()
{
  heap_t *h = h_init (512, true, 1); // Unsafe stack
  void *alloc1 = h_alloc_raw (h, 32);
  void *alloc2 = h_alloc_raw (h, 64);
  size_t allocOffset1 = calc_heap_offset (alloc1, h);
  size_t allocOffset2 = calc_heap_offset (alloc2, h);
  uintptr_t num
      = (uintptr_t)alloc2; // Number that happens to correspond to address.
  double old_sin = sin ((double)num);

  alloc1 = h; // Removes all references to first alloc, it is garbage, but
              // alloc2 is not moved because it is unsafe. Thus num is not
              // changed as well.
  h_gc (h);
  CU_ASSERT_FALSE (is_offset_allocated (h->alloc_map, allocOffset1));
  CU_ASSERT_TRUE (is_offset_allocated (h->alloc_map, allocOffset2));
  CU_ASSERT_EQUAL (calc_heap_offset (alloc2, h), allocOffset2);
  CU_ASSERT_EQUAL (sin ((double)num), old_sin);

  h_delete (h);
}

void
compacting_move_to_start_of_heap_safe_stack ()
{
  heap_t *h = h_init (512, false, 1); // Stack is considered safe.
  void *alloc1 = h_alloc_raw (h, 32);
  void *alloc2 = h_alloc_raw (h, 64);
  alloc1 = h;
  (void)alloc1; // Used to remove warnings about unused variable.

  uintptr_t num
      = (uintptr_t)alloc2; // Number that happens to correspond to address.
  double old_sin
      = sin ((double)num); // Stores the sine value of the number to be used as
                           // comparison that it the number did not change.

  CU_ASSERT_NOT_EQUAL (alloc2, (char *)h->heap_start + sizeof (header_t));
  h_gc (h);
  CU_ASSERT_EQUAL (alloc2, (char *)h->heap_start + sizeof (header_t));

  CU_ASSERT_NOT_EQUAL (sin ((double)num), old_sin);

  h_delete (h);
}

void
compacting_move_to_start_of_heap_unsafe_stack ()
{
  heap_t *h = h_init (4 * PAGE_SIZE, true, 1); // Stack is considered unsafe.

  typedef struct test t;
  struct test
  {
    void *ptr;
  };

  size_t alloc2_size = 1016;

  t *alloc1 = (t *)h_alloc_struct (h, "*");    // On page 0
  void *alloc2 = h_alloc_raw (h, alloc2_size); // On page 0
  void *alloc3 = h_alloc_raw (h, 1524);        // On page 1
  size_t old_offset1 = calc_heap_offset (alloc1, h);
  size_t old_offset2 = calc_heap_offset (alloc2, h);
  size_t old_offset3 = calc_heap_offset (alloc3, h);

  alloc2 = NULL; // Alloc 2 is garbage
  alloc1->ptr = alloc3;
  alloc3 = NULL; // Alloc 3 is still referenced by the ptr in alloc1.
  (void)alloc2;
  (void)alloc3;

  size_t freed_bytes = h_gc (h);
  CU_ASSERT_EQUAL (freed_bytes, alloc2_size);

  size_t new_offset1 = calc_heap_offset (alloc1, h);
  size_t new_offset3 = calc_heap_offset (alloc1->ptr, h); // Moved to page 0
  CU_ASSERT_EQUAL (new_offset1, old_offset1);
  CU_ASSERT_EQUAL (new_offset3, old_offset2)
  CU_ASSERT_NOT_EQUAL (new_offset3, old_offset3);

  h_delete (h);
}

void
compacting_multiple_allocs ()
{
  heap_t *h = h_init (10 * PAGE_SIZE, false, 1);

  typedef struct test t;
  struct test
  {
    void *ptr;
    void *ptr2;
    char c;
  };

  size_t garbage_size1 = HEAP_ALIGNMENT * 20 - sizeof (header_t);
  size_t garbage_size2 = HEAP_ALIGNMENT * 100 - sizeof (header_t);
  size_t alloc2_size = 16 - sizeof (header_t);

  int oldNum = 105;
  int b = oldNum;

  t *alloc1 = (t *)h_alloc_struct (h, "2*c");
  void *garbage1 = h_alloc_raw (h, garbage_size1);
  void *garbage2 = h_alloc_raw (h, garbage_size2);
  void *alloc2 = h_alloc_raw (h, alloc2_size);
  t *alloc3 = (t *)h_alloc_struct (h, "2*c");

  size_t garbage1Offset = calc_heap_offset (garbage1, h);

  garbage1 = NULL;
  garbage2 = NULL;
  (void)garbage1;
  (void)garbage2;

  alloc1->ptr2 = alloc2;
  alloc3->ptr = alloc1;
  alloc3->ptr2 = &b;

  size_t old_alloc1_offset = calc_heap_offset (alloc1, h);
  size_t old_alloc3_offset = calc_heap_offset (alloc3, h);

  size_t collected_bytes = h_gc (h);
  CU_ASSERT_EQUAL (collected_bytes,
                   garbage_size1 + garbage_size2 + 2 * sizeof (header_t));

  CU_ASSERT_EQUAL (calc_heap_offset (alloc1, h), old_alloc1_offset);
  CU_ASSERT_NOT_EQUAL (calc_heap_offset (alloc3, h), old_alloc3_offset);
  CU_ASSERT_EQUAL (calc_heap_offset (alloc2, h), garbage1Offset);
  CU_ASSERT_EQUAL (calc_heap_offset (alloc3->ptr, h),
                   calc_heap_offset (alloc1, h));

  h_delete (h);
}

void
compacting_with_format_strings ()
{
  heap_t *h = h_init (10 * PAGE_SIZE, false, 1);
  char *too_large_format = "50*";

  void *garbage = h_alloc_raw (h, 532);
  void *alloc_with_format_string
      = h_alloc_struct (h, too_large_format); // Too large for bitvector.
  garbage = NULL;
  (void)garbage;

  header_t prev_header
      = get_header_value (get_header_pointer (alloc_with_format_string));
  header_type_t prev_header_type = get_header_type (prev_header);
  CU_ASSERT_EQUAL (prev_header_type, HEADER_POINTER_TO_FORMAT_STRING);
  CU_ASSERT_STRING_EQUAL (prev_header & ~(0x3), too_large_format);

  /* Add 10 * PAGE_SIZE to ensure "copy" is not garbage collected */
  char *alloc_ptr_copy = (char *)alloc_with_format_string + 10 * PAGE_SIZE;

  size_t prev_alloc_offset = calc_heap_offset (alloc_with_format_string, h);

  h_gc (h);

  alloc_ptr_copy -= 10 * PAGE_SIZE;

  /* Allocation was moved during garbage collection */
  CU_ASSERT_PTR_NOT_EQUAL (alloc_with_format_string, alloc_ptr_copy)

  header_t header
      = get_header_value (get_header_pointer (alloc_with_format_string));
  header_type_t header_type = get_header_type (header);
  CU_ASSERT_EQUAL (header_type, HEADER_POINTER_TO_FORMAT_STRING);

  char *string_ptr = (char *)(header & ~(0x3));
  CU_ASSERT_STRING_EQUAL (string_ptr, too_large_format);

  size_t offset_alloc = calc_heap_offset (alloc_with_format_string, h);
  size_t offset_string = calc_heap_offset (string_ptr, h);

  // /* One allocation should be at the start of the heap */
  CU_ASSERT_TRUE ((offset_alloc == sizeof (header_t))
                  || (offset_string == sizeof (header_t)))

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
  CU_pSuite compacting_tests
      = CU_add_suite ("Compacting/Garbage collection Testing Suite",
                      init_compacting_suite, clean_compacting_suite);
  if (compacting_tests == NULL)
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
  if ((CU_add_test (compacting_tests,
                    "Test that objects on unsafe pages are not moved.",
                    compacting_not_move_unsafe_page)
           == NULL
       || CU_add_test (compacting_tests,
                       "Test that a compacted item is moved to the start of "
                       "the heap using a safe stack.",
                       compacting_move_to_start_of_heap_safe_stack)
              == NULL
       || CU_add_test (compacting_tests,
                       "Test that a compacted item is moved to the start of "
                       "the heap using an unsafe stack.",
                       compacting_move_to_start_of_heap_unsafe_stack)
              == NULL
       || CU_add_test (
              compacting_tests,
              "Test that multiple allocations are correctly compacted.",
              compacting_multiple_allocs)
              == NULL
       || CU_add_test (compacting_tests,
                       "Test compacting when some allocations have allocated "
                       "format strings.",
                       compacting_with_format_strings)
              == NULL
       || 0))
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

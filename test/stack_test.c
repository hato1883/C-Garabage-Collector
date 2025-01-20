#include <CUnit/Basic.h>
#include <stdlib.h>

#include "../src/gc.h"
#include "../src/stack.h"

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
baseaddress_not_null (void)
{
  uintptr_t start = find_stack_beginning ();
  CU_ASSERT_TRUE (start != 0);
}

void
endaddress_not_null (void)
{
  uintptr_t end = find_stack_end ();
  CU_ASSERT_TRUE (end != 0);
}

void
local_var_in_stack_range (void)
{
  // Shown next to the variables are their stack addresses from a test run on
  // my machine The numbers at the end are the last five numbers of the address
  // converted into decimal
  int var1 = 4;    // 0x7fffffffd86c 45196
  double var2 = 5; // 0x7fffffffd870 45200

  uintptr_t start = find_stack_beginning (); // 0x7fffffffdb48 45928
  uintptr_t end = find_stack_end ();         // 0x7fffffffd850 // 45168

  CU_ASSERT_NOT_EQUAL (start, end);
  sort_stack_ends (&start, &end);

  // Check that the addresses of var1 and var2 are within start and end
  CU_ASSERT_TRUE ((uintptr_t)&var1 > start);
  CU_ASSERT_TRUE ((uintptr_t)&var1 < end);
  CU_ASSERT_TRUE ((uintptr_t)&var2 > start);
  CU_ASSERT_TRUE ((uintptr_t)&var2 < end);
}

/**
 * Helper function to check finding of end from a deeper frame.
 */
__attribute__ ((noinline)) uintptr_t
find_stack_end_from_new_frame (void)
{
  uintptr_t end = find_stack_end ();
  return end;
}

/**
 * Checks the same as local_var_in_stack_range except that the stack end
 * is called and found from a deeper frame.
 * This test might not actually test something new, if the compiler
 * for example optimizes away the deeper frame. However, in case it does not
 * then this test would let us know there is a problem.
 */
void
find_end_from_other_frame (void)
{
  int var1 = 4;
  double var2 = 5;

  uintptr_t start = find_stack_beginning ();
  uintptr_t end = find_stack_end_from_new_frame (); // Finds the end, but from
                                                    // a deeper frame.

  CU_ASSERT_NOT_EQUAL (start, end);
  sort_stack_ends (&start, &end);

  // Check that the addresses of var1 and var2 are within start and end
  CU_ASSERT_TRUE ((uintptr_t)&var1 > start);
  CU_ASSERT_TRUE ((uintptr_t)&var1 < end);
  CU_ASSERT_TRUE ((uintptr_t)&var2 > start);
  CU_ASSERT_TRUE ((uintptr_t)&var2 < end);
}

void
sort_start_less_than_end (void)
{
  uintptr_t initial_start = 1;
  uintptr_t initial_end = 3;

  uintptr_t start = initial_start;
  uintptr_t end = initial_end;
  sort_stack_ends (&start, &end);

  CU_ASSERT (start < end);
  CU_ASSERT_EQUAL (start, initial_start);
  CU_ASSERT_EQUAL (end, initial_end);
}

void
sort_start_greater_than_end (void)
{
  uintptr_t initial_start = 3;
  uintptr_t initial_end = 1;

  uintptr_t start = initial_start;
  uintptr_t end = initial_end;
  sort_stack_ends (&start, &end);

  CU_ASSERT (start < end);
  CU_ASSERT_EQUAL (start, initial_end);
  CU_ASSERT_EQUAL (end, initial_start);
}

void
sort_stack_ends_with_found_ends (void)
{
  uintptr_t start = find_stack_beginning ();
  uintptr_t end = find_stack_end ();
  sort_stack_ends (&start, &end);

  CU_ASSERT (start < end);
}

void
test_is_valid_pointer_valid (void)
{
  int var = 10;
  uintptr_t ptr = (uintptr_t)&var;

  CU_ASSERT_TRUE (is_stack_pointer (ptr));
}

void
test_heap_pointer_invalid (void)
{
  char *heap_pointer = malloc (sizeof (char));
  uintptr_t ptr = (uintptr_t)heap_pointer;

  CU_ASSERT_FALSE (is_stack_pointer (ptr));
  free (heap_pointer);
}

void
test_is_valid_pointer_null (void)
{
  uintptr_t ptr = 0;

  CU_ASSERT_FALSE (is_stack_pointer (ptr));
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite stack_tests
      = CU_add_suite ("Stack utility Testing Suite", init_suite, clean_suite);
  if (stack_tests == NULL)
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
  if ((CU_add_test (stack_tests, "Check if base address is not null",
                    baseaddress_not_null)
           == NULL
       || CU_add_test (stack_tests, "Check if end address is not null",
                       endaddress_not_null)
              == NULL
       || CU_add_test (stack_tests,
                       "Check if local variable is between base and end",
                       local_var_in_stack_range)
              == NULL
       || CU_add_test (stack_tests,
                       "Check if local stack var is between start and end "
                       "when end is called from a deeper frame",
                       find_end_from_other_frame)
              == NULL
       || CU_add_test (
              stack_tests,
              "Check that stack endpoints are sorted when start > end",
              sort_start_less_than_end)
              == NULL
       || CU_add_test (
              stack_tests,
              "Check that stack endpoints are sorted when start < end",
              sort_start_greater_than_end)
              == NULL
       || CU_add_test (
              stack_tests,
              "Check that stack endpoints are sorted when start and end are "
              "found using find_start_beginning and find_start_end",
              sort_stack_ends_with_found_ends)
              == NULL
       || CU_add_test (stack_tests, "Test valid stack pointer",
                       test_is_valid_pointer_valid)
              == NULL
       || CU_add_test (stack_tests, "Test heap pointer",
                       test_heap_pointer_invalid)
              == NULL
       || CU_add_test (stack_tests, "Test NULL pointer",
                       test_is_valid_pointer_null)
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

#include <CUnit/Basic.h>

#include "../src/format_encoding.h"
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
check_if_format_is_size (void)
{
  // 1111 1111 1111 1111 - 1111 1111 1111 1111 -
  // 1111 1111 1111 1111 - 1111 1111 1111 1111
  uint_fast64_t bitvector = 0xFFFFFFFFFFFFFFFF;

  bitvector = set_formating_encoding (bitvector, FORMAT_SIZE_NO_POINTERS);
  CU_ASSERT_EQUAL (get_format_type (bitvector), FORMAT_SIZE_NO_POINTERS);
  CU_ASSERT_EQUAL (mask_formating_encoding (bitvector),
                   FORMAT_SIZE_NO_POINTERS)

  bitvector = 0x0;

  bitvector = set_formating_encoding (bitvector, FORMAT_SIZE_NO_POINTERS);
  CU_ASSERT_EQUAL (get_format_type (bitvector), FORMAT_SIZE_NO_POINTERS);
  CU_ASSERT_EQUAL (mask_formating_encoding (bitvector),
                   FORMAT_SIZE_NO_POINTERS)

  bitvector = set_formating_encoding (bitvector, FORMAT_UNUSED);
  CU_ASSERT_NOT_EQUAL (get_format_type (bitvector), FORMAT_SIZE_NO_POINTERS);
  CU_ASSERT_NOT_EQUAL (mask_formating_encoding (bitvector),
                       FORMAT_SIZE_NO_POINTERS)
}

void
check_if_format_is_bitvector (void)
{
  // 1111 1111 1111 1111 - 1111 1111 1111 1111 -
  // 1111 1111 1111 1111 - 1111 1111 1111 1111
  uint_fast64_t bitvector = 0xFFFFFFFFFFFFFFFF;

  bitvector = set_formating_encoding (bitvector, FORMAT_VECTOR);
  CU_ASSERT_EQUAL (get_format_type (bitvector), FORMAT_VECTOR);
  CU_ASSERT_EQUAL (mask_formating_encoding (bitvector), FORMAT_VECTOR)

  bitvector = 0x0;

  bitvector = set_formating_encoding (bitvector, FORMAT_VECTOR);
  CU_ASSERT_EQUAL (get_format_type (bitvector), FORMAT_VECTOR);
  CU_ASSERT_EQUAL (mask_formating_encoding (bitvector), FORMAT_VECTOR)

  bitvector = set_formating_encoding (bitvector, FORMAT_UNUSED);
  CU_ASSERT_NOT_EQUAL (get_format_type (bitvector), FORMAT_VECTOR);
  CU_ASSERT_NOT_EQUAL (mask_formating_encoding (bitvector), FORMAT_VECTOR)
}

void
check_if_format_is_size_with_pointers (void)
{
  // 1111 1111 1111 1111 - 1111 1111 1111 1111 -
  // 1111 1111 1111 1111 - 1111 1111 1111 1111
  uint_fast64_t bitvector = 0xFFFFFFFFFFFFFFFF;

  bitvector = set_formating_encoding (bitvector, FORMAT_SIZE_HAS_POINTERS);
  CU_ASSERT_EQUAL (get_format_type (bitvector), FORMAT_SIZE_HAS_POINTERS);
  CU_ASSERT_EQUAL (mask_formating_encoding (bitvector),
                   FORMAT_SIZE_HAS_POINTERS)

  bitvector = 0x0;

  bitvector = set_formating_encoding (bitvector, FORMAT_SIZE_HAS_POINTERS);
  CU_ASSERT_EQUAL (get_format_type (bitvector), FORMAT_SIZE_HAS_POINTERS);
  CU_ASSERT_EQUAL (mask_formating_encoding (bitvector),
                   FORMAT_SIZE_HAS_POINTERS)

  bitvector = set_formating_encoding (bitvector, FORMAT_UNUSED);
  CU_ASSERT_NOT_EQUAL (get_format_type (bitvector), FORMAT_SIZE_HAS_POINTERS);
  CU_ASSERT_NOT_EQUAL (mask_formating_encoding (bitvector),
                       FORMAT_SIZE_HAS_POINTERS)
}

void
check_if_format_is_unused (void)
{
  // 1111 1111 1111 1111 - 1111 1111 1111 1111 -
  // 1111 1111 1111 1111 - 1111 1111 1111 1111
  uint_fast64_t bitvector = 0xFFFFFFFFFFFFFFFF;

  bitvector = set_formating_encoding (bitvector, FORMAT_UNUSED);
  CU_ASSERT_EQUAL (get_format_type (bitvector), FORMAT_UNUSED);
  CU_ASSERT_EQUAL (mask_formating_encoding (bitvector), FORMAT_UNUSED)

  bitvector = 0x0;

  bitvector = set_formating_encoding (bitvector, FORMAT_UNUSED);
  CU_ASSERT_EQUAL (get_format_type (bitvector), FORMAT_UNUSED);
  CU_ASSERT_EQUAL (mask_formating_encoding (bitvector), FORMAT_UNUSED)

  bitvector = set_formating_encoding (bitvector, FORMAT_SIZE_NO_POINTERS);
  CU_ASSERT_NOT_EQUAL (get_format_type (bitvector), FORMAT_UNUSED);
  CU_ASSERT_NOT_EQUAL (mask_formating_encoding (bitvector), FORMAT_UNUSED)
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
      = CU_add_suite ("Encoding enum Testing Suite", init_suite, clean_suite);
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
  if ((CU_add_test (stack_tests,
                    "Check if decoding a bitvector \"size without pointers\"",
                    check_if_format_is_size)
       == NULL)
      || (CU_add_test (stack_tests,
                       "Check if decoding a bitvector \"formating bitvector\"",
                       check_if_format_is_bitvector)
          == NULL)
      || (CU_add_test (stack_tests,
                       "Check if decoding a bitvector \"size with pointers\"",
                       check_if_format_is_size_with_pointers)
          == NULL)
      || (CU_add_test (
              stack_tests,
              "Check if decoding a bitvector \"unused formating mask\"",
              check_if_format_is_unused)
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

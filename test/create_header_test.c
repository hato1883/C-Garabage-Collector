#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/format_encoding.h"
#include "../src/gc.h"
#include "../src/header.h"

#define UNUSED(x) x __attribute__ ((__unused__))
#define SET_PTR_TEST_VAL 5 // A value used in test apply_func_to_ptrs
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

// tests for creating a header from a format string and a size

void
test_header_struct_only_char (void)
{
  struct test_struct
  {
    char c;
  };

  char *format_string = "c";
  bool success;

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  header_t header = create_header_struct (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_number (void)
{
  struct test_struct
  {
    char c, c2, c3;
  };

  char *format_string = "3c";
  bool success;

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  header_t header = create_header_struct (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_larger_number (void)
{
  struct test_struct
  {
    int i, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15;
  };

  char *format_string = "15i";
  bool success;

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  header_t header = create_header_struct (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_only_number (void)
{
  char *format_string = "351";
  bool success;

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  header_t header = create_header_struct (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_only_zero (void)
{
  char *format_string = "0";
  bool success;

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  header_t header = create_header_struct (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_only_pointer (void)
{
  struct test_struct
  {
    void *p;
  };

  char *format_string = "*";
  bool success;

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  header_t header = create_header_struct (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  uint64_t vector = convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (vector, header >> (BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_different_fields (void)
{
  struct test_struct
  {
    char c;
    double d;
    int i, i2;
    void *p;
    char c2;
  };

  char *format_string = "cd2i*c";
  bool success;

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  header_t header = create_header_struct (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  uint64_t vector = convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (vector, header >> (BITS_FOR_HEADER_TYPE));
}

// tests for creating a header only from a format string

void
test_header_struct_unknown_size_only_char (void)
{
  struct test_struct
  {
    char c;
  };

  char *format_string = "c";
  bool success;

  header_t header
      = create_header_struct_unknown_size (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_unknown_size_number (void)
{
  struct test_struct
  {
    char c, c2, c3;
  };

  char *format_string = "3c";
  bool success;

  header_t header
      = create_header_struct_unknown_size (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_unknown_size_larger_number (void)
{
  struct test_struct
  {
    int i, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15;
  };

  char *format_string = "15i";
  bool success;

  header_t header
      = create_header_struct_unknown_size (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_unknown_size_only_number (void)
{
  char *format_string = "351";
  bool success;

  header_t header
      = create_header_struct_unknown_size (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_unknown_size_only_zero (void)
{
  char *format_string = "0";
  bool success;

  header_t header
      = create_header_struct_unknown_size (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_unknown_size_only_pointer (void)
{
  struct test_struct
  {
    void *p;
  };

  char *format_string = "*";
  bool success;

  header_t header
      = create_header_struct_unknown_size (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint64_t vector = convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (vector, header >> (BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_unknown_size_different_fields (void)
{
  struct test_struct
  {
    char c;
    double d;
    int i, i2;
    void *p;
    char c2;
  };

  char *format_string = "cd2i*c";
  bool success;

  header_t header
      = create_header_struct_unknown_size (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint64_t vector = convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (vector, header >> (BITS_FOR_HEADER_TYPE));
}

void
test_header_struct_unknown_size_to_large_format_string (void)
{
  char *format_string = "c18446744073709551615*";
  bool success;

  create_header_struct_unknown_size (format_string, &success);
  CU_ASSERT_FALSE (success);
}

// tests for creating a header only from a size

void
test_header_raw_zero (void)
{
  uint_fast64_t size = 0;

  bool success;
  header_t header = create_header_raw (size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_raw_one (void)
{
  uint_fast64_t size = 1;

  bool success;
  header_t header = create_header_raw (size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_raw_one_kilobyte (void)
{
  uint_fast64_t size = 1024;

  bool success;
  header_t header = create_header_raw (size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_raw_max_size (void)
{
  uint_fast64_t size
      = UINT64_MAX >> (BITS_FOR_FORMAT_TYPE + BITS_FOR_HEADER_TYPE);

  bool success;
  header_t header = create_header_raw (size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size,
                   remove_formating_encoding (header >> BITS_FOR_HEADER_TYPE));
}

void
test_header_raw_one_too_big (void)
{
  uint_fast64_t size
      = (UINT64_MAX >> (BITS_FOR_FORMAT_TYPE + BITS_FOR_HEADER_TYPE)) + 1;

  bool success;
  create_header_raw (size, &success);

  CU_ASSERT_FALSE (success);
}

void
test_header_raw_much_too_big (void)
{
  uint_fast64_t size = UINT64_MAX;

  bool success;
  create_header_raw (size, &success);

  CU_ASSERT_FALSE (success);
}

int
main (void)
{
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  CU_pSuite header_struct_suite = CU_add_suite (
      "Header from format string and size", init_suite, clean_suite);
  CU_pSuite header_struct_no_size_suite = CU_add_suite (
      "Header from only format string", init_suite, clean_suite);
  CU_pSuite header_raw_suite
      = CU_add_suite ("Header from only size", init_suite, clean_suite);

  if (header_struct_suite == NULL || header_struct_no_size_suite == NULL
      || header_raw_suite == NULL)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  if ((CU_add_test (header_struct_suite,
                    "Only char in format string -> size in header",
                    test_header_struct_only_char)
       == NULL)
      || (CU_add_test (
              header_struct_suite,
              "Only number of chars in format string -> size in header",
              test_header_struct_number)
          == NULL)
      || (CU_add_test (header_struct_suite,
                       "Only two digit number of ints in format string -> "
                       "size in header",
                       test_header_struct_larger_number)
          == NULL)
      || (CU_add_test (
              header_struct_suite,
              "Only three digit number in format string -> size in header",
              test_header_struct_only_number)
          == NULL)
      || (CU_add_test (header_struct_suite,
                       "Only 0 in format string -> size in header",
                       test_header_struct_only_zero)
          == NULL)
      || (CU_add_test (header_struct_suite,
                       "Only pointer in format string -> bit vector in header",
                       test_header_struct_only_pointer)
          == NULL)
      || (CU_add_test (header_struct_suite,
                       "Pointer and other things in format string -> bit "
                       "vector in header",
                       test_header_struct_different_fields)
          == NULL)
      || (CU_add_test (header_struct_no_size_suite,
                       "Only char in format string -> size in header",
                       test_header_struct_unknown_size_only_char)
          == NULL)
      || (CU_add_test (
              header_struct_no_size_suite,
              "Only number of chars in format string -> size in header",
              test_header_struct_unknown_size_number)
          == NULL)
      || (CU_add_test (header_struct_no_size_suite,
                       "Only two digit number of ints in format string -> "
                       "size in header",
                       test_header_struct_unknown_size_larger_number)
          == NULL)
      || (CU_add_test (
              header_struct_no_size_suite,
              "Only three digit number in format string -> size in header",
              test_header_struct_unknown_size_only_number)
          == NULL)
      || (CU_add_test (header_struct_no_size_suite,
                       "Only 0 in format string -> size in header",
                       test_header_struct_unknown_size_only_zero)
          == NULL)
      || (CU_add_test (header_struct_no_size_suite,
                       "Only pointer in format string -> bit vector in header",
                       test_header_struct_unknown_size_only_pointer)
          == NULL)
      || (CU_add_test (header_struct_no_size_suite,
                       "Pointer and other things in format string -> bit "
                       "vector in header",
                       test_header_struct_unknown_size_different_fields)
          == NULL)
      || (CU_add_test (
              header_struct_no_size_suite,
              "Attempt to Create header with a too large format string fails",
              test_header_struct_unknown_size_to_large_format_string)
          == NULL)
      || (CU_add_test (header_raw_suite, "Size zero -> success",
                       test_header_raw_zero)
          == NULL)
      || (CU_add_test (header_raw_suite, "Size one", test_header_raw_one)
          == NULL)
      || (CU_add_test (header_raw_suite, "Size one kB",
                       test_header_raw_one_kilobyte)
          == NULL)
      || (CU_add_test (header_raw_suite,
                       "The maximum size for header to contain",
                       test_header_raw_max_size)
          == NULL)
      || (CU_add_test (header_raw_suite,
                       "One too big for header to contain -> failure",
                       test_header_raw_one_too_big)
          == NULL)
      || (CU_add_test (header_raw_suite,
                       "Much too big for header to contain -> failure",
                       test_header_raw_much_too_big)
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

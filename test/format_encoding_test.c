#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdint.h>

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

// -------- size from format string tests ------------

void
test_struct_size_string_only_char (void)
{
  struct test_struct
  {
    char c;
  };

  char *format_string = "c";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   sizeof (struct test_struct));
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_string_number (void)
{
  struct test_struct
  {
    char c, c2, c3;
  };

  char *format_string = "3c";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   sizeof (struct test_struct));
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_string_larger_number (void)
{
  struct test_struct
  {
    int i, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15;
  };

  char *format_string = "15i";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   sizeof (struct test_struct));
  CU_ASSERT_TRUE (success);
}

void
test_size_string_only_number (void)
{
  char *format_string = "351";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   351 * sizeof (char));
  CU_ASSERT_TRUE (success);
}

void
test_size_string_only_zero (void)
{
  char *format_string = "0";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success), 0);
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_string_int_pad (void)
{
  struct test_struct
  {
    char c, c2, c3;
    int x;
  };

  char *format_string = "3ci";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   sizeof (struct test_struct));
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_string_float_pad (void)
{
  struct test_struct
  {
    int i;
    char c;
    float f;
  };

  char *format_string = "icf";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   sizeof (struct test_struct));
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_string_pointer_pad (void)
{
  struct test_struct
  {
    char c;
    void *p;
  };

  char *format_string = "c*";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   sizeof (struct test_struct));
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_string_double_pad (void)
{
  struct test_struct
  {
    char c;
    double d;
  };

  char *format_string = "cd";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   sizeof (struct test_struct));
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_string_long_pad (void)
{
  struct test_struct
  {
    char c, c2, c3, c4;
    long l, l2, l3, l4, l5;
  };

  char *format_string = "cccc5l";
  bool success;

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   sizeof (struct test_struct));
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_string_char_ending (void)
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

  CU_ASSERT_EQUAL (size_from_string (format_string, &success),
                   sizeof (struct test_struct));
  CU_ASSERT_TRUE (success);
}

void
test_size_string_too_large_number (void)
{
  char *format_string = "18446744073709551616";
  bool success;

  size_from_string (format_string, &success);
  CU_ASSERT_FALSE (success);
}

void
test_size_string_too_large_with_ending_number (void)
{
  char *format_string = "**18446744073709551607";
  bool success;

  size_from_string (format_string, &success);
  CU_ASSERT_FALSE (success);
}

void
test_size_string_too_large_with_ending_number_alt (void)
{
  char *format_string = "*18446744073709551601";
  bool success;

  size_from_string (format_string, &success);
  CU_ASSERT_FALSE (success);
}

void
test_size_string_too_many_ints (void)
{
  char *format_string = "4611686018427387904i";
  bool success;

  size_from_string (format_string, &success);
  CU_ASSERT_FALSE (success);
}

void
test_size_string_ending_padding_makes_too_big (void)
{
  char *format_string = "2305843009213693951*c";
  bool success;

  size_from_string (format_string, &success);
  CU_ASSERT_FALSE (success);
}

// ------------ Conversion to bit vector tests -------------

void
test_to_vector_only_char (void)
{
  char *format_string = "c";
  bool success;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_TRUE (success);
}

void
test_to_vector_one_char (void)
{
  char *format_string = "1c";
  char *format_string2 = "c";
  bool success, success2;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (convert_to_bit_vector (format_string, size, &success),
                   convert_to_bit_vector (format_string2, size, &success2));
  CU_ASSERT_TRUE (success);
  CU_ASSERT_TRUE (success2);
}

void
test_to_vector_four_char (void)
{
  char *format_string = "4c";
  char *format_string2 = "cccc";
  bool success, success2;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (convert_to_bit_vector (format_string, size, &success),
                   convert_to_bit_vector (format_string2, size, &success2));
  CU_ASSERT_TRUE (success);
  CU_ASSERT_TRUE (success2);
}

void
test_to_vector_larger_number (void)
{
  char *format_string = "4c";
  char *format_string2 = "cccc";
  bool success, success2;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (convert_to_bit_vector (format_string, size, &success),
                   convert_to_bit_vector (format_string2, size, &success2));
  CU_ASSERT_TRUE (success);
  CU_ASSERT_TRUE (success2);
}

void
test_to_vector_too_big (void)
{
  // vector should not be able to support more than 30 8 byte blocks (assuming
  // compacting)
  char *format_string = "*240c";
  bool success;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_FALSE (success);
}

void
test_to_vector_too_many_pointers (void)
{
  // vector should not be able to support more than 30 pointers
  char *format_string = "31*";
  bool success;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_FALSE (success);
}

void
test_to_vector_only_number_valid (void)
{
  char *format_string = "103";
  char *format_string2 = "103c";
  bool success, success2;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (convert_to_bit_vector (format_string, size, &success),
                   convert_to_bit_vector (format_string2, size, &success2));
  CU_ASSERT_TRUE (success);
  CU_ASSERT_TRUE (success2);
}

void
test_to_vector_only_number_too_big (void)
{
  // 2^61 - should always be too big
  char *format_string = "2305843009213693952";
  bool success;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_FALSE (success);
}

void
test_convert_to_bit_vector_too_many_chars_before_pointer (void)
{
  // 2^61 - should always be too big
  char *format_string = "121c*";
  bool success;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_FALSE (success);
}

void
test_to_vector_only_zero (void)
{
  char *format_string = "0";
  bool success;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (convert_to_bit_vector (format_string, size, &success), 0);
  CU_ASSERT_TRUE (success);
}

void
test_to_vector_all_types_char_ending (void)
{
  char *format_string = "cd2i*c";
  char *format_string2 = "2cdif*ccc";
  bool success, success2;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (convert_to_bit_vector (format_string, size, &success),
                   convert_to_bit_vector (format_string2, size, &success2));
  CU_ASSERT_TRUE (success);
  CU_ASSERT_TRUE (success2);
}

void
test_to_vector_chars_before_pointer (void)
{
  char *format_string = "ccc*";
  char *format_string2 = "c*";
  bool success, success2;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (convert_to_bit_vector (format_string, size, &success),
                   convert_to_bit_vector (format_string2, size, &success2));
  CU_ASSERT_TRUE (success);
  CU_ASSERT_TRUE (success2);
}

void
test_to_vector_too_many_longs (void)
{
  // vector should not be able to support more than 30 longs / pointers
  char *format_string = "*30l";
  bool success;

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_FALSE (success);
}

// --------- size from bit vector tests -------------

void
test_struct_size_vector_number (void)
{
  struct test_struct
  {
    int i, i2, i3;
  };

  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector ("3i", size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size_from_vector (vector), size);
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_vector_larger_number (void)
{
  struct test_struct
  {
    int i, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15;
  };

  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector ("15i", size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size_from_vector (vector), size);
  CU_ASSERT_TRUE (success);
}

void
test_size_vector_only_zero (void)
{
  bool success;

  char *format_string = "0";

  uint_fast64_t size = size_from_string (format_string, &success);
  CU_ASSERT_TRUE (success);

  uint_fast64_t vector = convert_to_bit_vector ("0", size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size_from_vector (vector), 0);
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_vector_int_pad (void)
{
  struct test_struct
  {
    char c, c2, c3;
    int x;
  };

  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector ("3ci", size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size_from_vector (vector), size);
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_vector_float_pad (void)
{
  struct test_struct
  {
    int i;
    char c;
    float f;
  };

  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector ("icf", size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size_from_vector (vector), size);
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_vector_pointer_pad (void)
{
  struct test_struct
  {
    char c;
    void *p;
  };

  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector ("c*", size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size_from_vector (vector), size);
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_vector_double_pad (void)
{
  struct test_struct
  {
    char c;
    double d;
  };

  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector ("cd", size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size_from_vector (vector), size);
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_vector_long_pad (void)
{
  struct test_struct
  {
    char c, c2, c3, c4;
    long l, l2, l3, l4, l5;
  };

  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector ("cccc5l", size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size_from_vector (vector), size);
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_vector_char_ending (void)
{
  struct test_struct
  {
    char c;
    double d;
    int i, i2;
    void *p;
    char c2;
  };

  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector ("cd2i*c", size, &success);
  CU_ASSERT_TRUE (success);

  CU_ASSERT_EQUAL (size_from_vector (vector), size);
  CU_ASSERT_TRUE (success);
}

void
test_struct_size_vector_invalid_format (void)
{
  uint_fast64_t vector = FORMAT_UNUSED;

  CU_ASSERT_EQUAL (size_from_vector (vector), 0);
}

// Test for finding pointers from bit vector

void
test_no_pointers_from_bit_vector ()
{
  struct test_struct
  {
    char c;
    int i;
    long l;
  };

  struct test_struct t = { .c = 'o', .i = 7, .l = -82 };

  char *format_string = "cil";
  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  ptr_queue_t *pointers = get_pointers_from_bit_vector (vector, &t);

  CU_ASSERT_TRUE (is_empty_queue (pointers));

  destroy_ptr_queue (pointers);
}

void
test_one_pointer_from_bit_vector ()
{
  struct test_struct
  {
    char c;
    void *p;
  };

  char *str = "banan";
  struct test_struct t = { .c = 'o', .p = str };

  char *format_string = "c*";
  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  ptr_queue_t *pointers = get_pointers_from_bit_vector (vector, &t);

  CU_ASSERT_PTR_EQUAL (str, *dequeue_ptr (pointers));
  CU_ASSERT_TRUE (is_empty_queue (pointers));

  destroy_ptr_queue (pointers);
}

void
test_multiple_pointers_from_bit_vector ()
{
  struct test_struct
  {
    char c;
    double d;
    void *p;
    int i;
    void *p2;
    long l;
    char c2;
    void *p3;
  };

  char *str = "banan";
  bool b = true;
  struct test_struct t = { .c = 'o',
                           .d = -43,
                           .p = str,
                           .i = 290,
                           .p2 = &b,
                           .l = 827,
                           .c2 = 'h',
                           .p3 = str };

  char *format_string = "cd*i*lc*";
  bool success;

  uint_fast64_t size = sizeof (struct test_struct);

  uint_fast64_t vector = convert_to_bit_vector (format_string, size, &success);
  CU_ASSERT_TRUE (success);

  ptr_queue_t *pointers = get_pointers_from_bit_vector (vector, &t);

  void *lesser_ptr = (void *)str < (void *)&b ? (void *)str : (void *)&b;
  void *greater_ptr = (void *)str > (void *)&b ? (void *)str : (void *)&b;

  CU_ASSERT_PTR_EQUAL (lesser_ptr, *dequeue_ptr (pointers));
  CU_ASSERT_PTR_EQUAL (greater_ptr, *dequeue_ptr (pointers));

  destroy_ptr_queue (pointers);
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite size_string_tests = CU_add_suite ("Size from string - test suite",
                                              init_suite, clean_suite);
  CU_pSuite to_vector_tests
      = CU_add_suite ("Converting format string to bit vector - test suite",
                      init_suite, clean_suite);
  CU_pSuite size_vector_tests = CU_add_suite (
      "Size from bit vector - test suite", init_suite, clean_suite);
  CU_pSuite get_pointers_tests = CU_add_suite (
      "Get pointers from bit vector - test suite", init_suite, clean_suite);

  if (size_string_tests == NULL || to_vector_tests == NULL
      || size_vector_tests == NULL || get_pointers_tests == NULL)
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
  if (CU_add_test (size_string_tests, "Padding before int",
                   test_struct_size_string_int_pad)
          == NULL
      || CU_add_test (size_string_tests, "Size with only char in struct",
                      test_struct_size_string_only_char)
             == NULL
      || CU_add_test (size_string_tests, "Padding before float",
                      test_struct_size_string_float_pad)
             == NULL
      || CU_add_test (size_string_tests, "Padding before pointer",
                      test_struct_size_string_pointer_pad)
             == NULL
      || CU_add_test (size_string_tests, "Padding before double",
                      test_struct_size_string_double_pad)
             == NULL
      || CU_add_test (size_string_tests, "Padding before long",
                      test_struct_size_string_long_pad)
             == NULL
      || CU_add_test (size_string_tests, "Size of struct with char at end",
                      test_struct_size_string_char_ending)
             == NULL
      || CU_add_test (size_string_tests,
                      "Size of struct with number in format string",
                      test_struct_size_string_number)
             == NULL
      || CU_add_test (size_string_tests,
                      "Size of struct with larger number in format string",
                      test_struct_size_string_larger_number)
             == NULL
      || CU_add_test (size_string_tests,
                      "Pointer followed by too large unmer of chars fails",
                      test_struct_size_string_larger_number)
             == NULL
      || CU_add_test (size_string_tests,
                      "Size with only (larger) number in format string",
                      test_size_string_only_number)
             == NULL
      || CU_add_test (size_string_tests, "Size with only 0 in format string",
                      test_size_string_only_zero)
             == NULL
      || CU_add_test (size_string_tests,
                      "Size fails for too large number in format string",
                      test_size_string_too_large_number)
             == NULL
      || CU_add_test (size_string_tests,
                      "Size fails for format string ending in a number "
                      "causing overflow",
                      test_size_string_too_large_with_ending_number)
             == NULL
      || CU_add_test (size_string_tests,
                      "Size fails for format string ending in a number due to "
                      "alignment",
                      test_size_string_too_large_with_ending_number_alt)
             == NULL
      || CU_add_test (
             size_string_tests,
             "Size fails for too large number before i in format string",
             test_size_string_too_many_ints)
             == NULL
      || CU_add_test (
             size_string_tests,
             "Size fails when padding at end of struct causes overflow",
             test_size_string_ending_padding_makes_too_big)
             == NULL
      || CU_add_test (to_vector_tests,
                      "Can make bit vector for struct only containing a char",
                      test_to_vector_only_char)
             == NULL
      || CU_add_test (to_vector_tests,
                      "Bit vector for format strings 1c and c are equal",
                      test_to_vector_one_char)
             == NULL
      || CU_add_test (to_vector_tests,
                      "Bit vector for format strings 4c and cccc are equal",
                      test_to_vector_four_char)
             == NULL
      || CU_add_test (to_vector_tests, "Larger number in format string",
                      test_to_vector_larger_number)
             == NULL
      || CU_add_test (to_vector_tests, "Too big struct fails",
                      test_to_vector_too_big)
             == NULL
      || CU_add_test (to_vector_tests,
                      "Too many pointers in format string fails",
                      test_to_vector_too_many_pointers)
             == NULL
      || CU_add_test (
             to_vector_tests,
             "Only number in format string works (not divisible by four)",
             test_to_vector_only_number_valid)
             == NULL
      || CU_add_test (to_vector_tests, "Too big number in format string fails",
                      test_to_vector_only_number_too_big)
             == NULL
      || CU_add_test (to_vector_tests, "Too many chars before pointer",
                      test_convert_to_bit_vector_too_many_chars_before_pointer)
             == NULL
      || CU_add_test (to_vector_tests, "Only 0 in format string",
                      test_to_vector_only_zero)
             == NULL
      || CU_add_test (to_vector_tests,
                      "Same result for two equivalent strings, all types, "
                      "ending in char",
                      test_to_vector_all_types_char_ending)
             == NULL
      || CU_add_test (to_vector_tests, "Chars before pointer work",
                      test_to_vector_chars_before_pointer)
             == NULL
      || CU_add_test (to_vector_tests, "Fails for too many longs",
                      test_to_vector_too_many_longs)
             == NULL
      || CU_add_test (size_vector_tests, "Padding before int",
                      test_struct_size_vector_int_pad)
             == NULL
      || CU_add_test (size_vector_tests, "Padding before float",
                      test_struct_size_vector_float_pad)
             == NULL
      || CU_add_test (size_vector_tests, "Padding before pointer",
                      test_struct_size_vector_pointer_pad)
             == NULL
      || CU_add_test (size_vector_tests, "Padding before double",
                      test_struct_size_vector_double_pad)
             == NULL
      || CU_add_test (size_vector_tests, "Padding before long",
                      test_struct_size_vector_long_pad)
             == NULL
      || CU_add_test (size_vector_tests, "Size of struct with char at end",
                      test_struct_size_vector_char_ending)
             == NULL
      || CU_add_test (size_vector_tests,
                      "Size of struct with number in format string",
                      test_struct_size_vector_number)
             == NULL
      || CU_add_test (size_vector_tests,
                      "Size of struct with larger number in format string",
                      test_struct_size_vector_larger_number)
             == NULL
      || CU_add_test (size_vector_tests,
                      "Size of struct with invalid format (unused)",
                      test_struct_size_vector_invalid_format)
             == NULL
      || CU_add_test (size_vector_tests, "Size with only 0 in format string",
                      test_size_vector_only_zero)
             == NULL
      || CU_add_test (
             get_pointers_tests,
             "Finds pointer from bit vector with a char and a pointer",
             test_one_pointer_from_bit_vector)
             == NULL
      || CU_add_test (
             get_pointers_tests,
             "Get pointers on vector without pointers returns empty queue",
             test_no_pointers_from_bit_vector)
             == NULL
      || CU_add_test (get_pointers_tests,
                      "Finds pointers from struct with multiple pointers, "
                      "where two are the same",
                      test_multiple_pointers_from_bit_vector)
             == NULL
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

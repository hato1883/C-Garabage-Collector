#include "../src/get_header.h"
#include <CUnit/Basic.h>

/* Initialize suite  */
int
init_suite (void)
{
  return 0;
}

/* Clean up suite  */
int
clean_suite (void)
{
  return 0;
}

void
test_get_header_pointer (void)
{
  /* Allocate memory for header + data (this time only locally)  */
  char memory_block[sizeof (header_t) + 16];
  header_t mock_header = HEADER_POINTER_TO_FORMAT_STRING;

  /* Set the header at the start of the memory block  */
  header_t *header_ptr = (header_t *)memory_block;
  *header_ptr = mock_header;

  /* Create a pointer to the "data" we have allocated  */
  void *memory_pointer = memory_block + sizeof (header_t);

  /* Test header retrieval  */
  header_t *retrieved_header = get_header_pointer (memory_pointer);
  CU_ASSERT_PTR_EQUAL (retrieved_header, header_ptr);
  CU_ASSERT_EQUAL (*retrieved_header, mock_header);
  CU_ASSERT_EQUAL (get_header_value (retrieved_header), mock_header);
}

void
test_update_header_of_a_pointer (void)
{
  /* Allocate memory for header + data (this time only locally)  */
  char memory_block[sizeof (header_t) + 16];
  header_t mock_header = HEADER_POINTER_TO_FORMAT_STRING;

  /* Set the header at the start of the memory block  */
  header_t *header_ptr = (header_t *)memory_block;
  *header_ptr = mock_header;

  /* Initialize the data region with a pattern  */
  char *data_region = memory_block + sizeof (header_t);
  for (int i = 0; i < 16; i++)
    {
      data_region[i] = (char)(i + 1);
    }

  /* Create a pointer to the "data" we have allocated */
  void *memory_pointer = memory_block + sizeof (header_t);

  /* Update header  */
  header_t new_header = HEADER_FORWARDING_ADDRESS;
  update_header_of_a_pointer (memory_pointer, new_header);

  /* Verify that the 16 bytes of data is unchanged  */
  for (int i = 0; i < 16; i++)
    {
      CU_ASSERT_EQUAL (data_region[i], (char)(i + 1));
    }

  /* Test if the header was updated  */
  header_t *updated_header = get_header_pointer (memory_pointer);
  CU_ASSERT_EQUAL (*updated_header, new_header);
}

void
test_header_pointer_to_format_string (void)
{
  /* Last 2 bits are 00  */
  int_fast64_t binary_string = 0x4;
  CU_ASSERT_EQUAL (get_header_type (binary_string),
                   HEADER_POINTER_TO_FORMAT_STRING);
}

void
test_header_forwarding_address (void)
{
  /* Last 2 bits are 01  */
  int_fast64_t binary_string = 0x5;
  CU_ASSERT_EQUAL (get_header_type (binary_string), HEADER_FORWARDING_ADDRESS);
}

void
test_header_unused (void)
{
  /* Last 2 bits are 10  */
  int_fast64_t binary_string = 0x2;
  CU_ASSERT_EQUAL (get_header_type (binary_string), HEADER_UNUSED);
}

void
test_header_bit_vector (void)
{
  /* Last 2 bits are 11  */
  int_fast64_t binary_string = 0x7;
  CU_ASSERT_EQUAL (get_header_type (binary_string), HEADER_BIT_VECTOR);
}

void
test_change_header (void)
{
  /* Random binary string  */
  int_fast64_t binary_string = 0x7;
  uint64_t mask = ~((uint64_t)0x3);
  CU_ASSERT (mask != 3);

  CU_ASSERT_EQUAL (get_header_type (binary_string), HEADER_BIT_VECTOR);

  /* Change their headers  */
  uint64_t v_binary = set_header_bit_vector (binary_string);
  CU_ASSERT_EQUAL (get_header_type (v_binary), HEADER_BIT_VECTOR);
  CU_ASSERT_EQUAL (v_binary, binary_string);

  uint64_t for_binary = set_header_forwarding_address (binary_string);
  CU_ASSERT_EQUAL (get_header_type (for_binary), HEADER_FORWARDING_ADDRESS);
  CU_ASSERT_EQUAL (for_binary,
                   (binary_string & mask) | HEADER_FORWARDING_ADDRESS);

  uint64_t point_binary = set_header_pointer_to_format_string (binary_string);
  CU_ASSERT_EQUAL (get_header_type (point_binary),
                   HEADER_POINTER_TO_FORMAT_STRING);
  CU_ASSERT_EQUAL (point_binary,
                   (binary_string & mask) | HEADER_POINTER_TO_FORMAT_STRING);

  uint64_t unused_binary = set_header_unused (binary_string);
  CU_ASSERT_EQUAL (get_header_type (unused_binary), HEADER_UNUSED);
  CU_ASSERT_EQUAL (unused_binary, (binary_string & mask) | HEADER_UNUSED);
}

void
test_change_large_header (void)
{
  /* Random binary string  */
  uint64_t binary_string = ~((uint64_t)0x0);
  uint64_t mask = ~((uint64_t)0x3);
  CU_ASSERT (mask != 3);

  CU_ASSERT_EQUAL (get_header_type (binary_string), HEADER_BIT_VECTOR);

  /* Change their headers  */
  uint64_t v_binary = set_header_bit_vector (binary_string);
  CU_ASSERT_EQUAL (get_header_type (v_binary), HEADER_BIT_VECTOR);
  CU_ASSERT_EQUAL (v_binary, binary_string);

  uint64_t for_binary = set_header_forwarding_address (binary_string);
  CU_ASSERT_EQUAL (get_header_type (for_binary), HEADER_FORWARDING_ADDRESS);
  CU_ASSERT_EQUAL (for_binary,
                   (binary_string & mask) | HEADER_FORWARDING_ADDRESS);

  uint64_t point_binary = set_header_pointer_to_format_string (binary_string);
  CU_ASSERT_EQUAL (get_header_type (point_binary),
                   HEADER_POINTER_TO_FORMAT_STRING);
  CU_ASSERT_EQUAL (point_binary,
                   (binary_string & mask) | HEADER_POINTER_TO_FORMAT_STRING);

  uint64_t unused_binary = set_header_unused (binary_string);
  CU_ASSERT_EQUAL (get_header_type (unused_binary), HEADER_UNUSED);
  CU_ASSERT_EQUAL (unused_binary, (binary_string & mask) | HEADER_UNUSED);
}

void
test_pointer_in_header ()
{
  header_t h = 0x0;
  h = set_header_pointer_to_format_string (h);

  CU_ASSERT_PTR_NULL (get_pointer_in_header (h));

  int n = 2389;
  int *p = &n;

  h = change_pointer_in_header (h, p);

  CU_ASSERT_PTR_EQUAL (p, get_pointer_in_header (h));

  int m = 5728;
  int *p2 = &m;

  h = change_pointer_in_header (h, p2);

  CU_ASSERT_PTR_EQUAL (p2, get_pointer_in_header (h));
}

int
main ()
{
  /* Initialize CUnit test registry  */
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  /* Add suite to registry  */
  CU_pSuite suite
      = CU_add_suite ("Header Type Suite", init_suite, clean_suite);
  if (!suite)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  /* Add tests to the suite  */
  if (!CU_add_test (suite, "Test HEADER_POINTER_TO_FORMAT_STRING",
                    test_header_pointer_to_format_string)
      || !CU_add_test (suite, "Test HEADER_FORWARDING_ADDRESS",
                       test_header_forwarding_address)
      || !CU_add_test (suite, "Test HEADER_UNUSED", test_header_unused)
      || !CU_add_test (suite, "Test HEADER_BIT_VECTOR", test_header_bit_vector)
      || !CU_add_test (suite, "Test change", test_change_header)
      || !CU_add_test (suite, "Test change large header",
                       test_change_large_header)
      || !CU_add_test (suite, "Test Update the header",
                       test_update_header_of_a_pointer)
      || !CU_add_test (suite, "Test pointer get header",
                       test_get_header_pointer)
      || !CU_add_test (suite, "Test change and get pointer stored in header",
                       test_pointer_in_header)
      || 0)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  /* Run tests  */
  CU_basic_set_mode (CU_BRM_VERBOSE);
  CU_basic_run_tests ();

  /* Cleanup  */
  int exit_code = (CU_get_number_of_tests_failed () == 0)
                      ? CU_get_error ()
                      : CU_get_number_of_tests_failed ();
  CU_cleanup_registry ();
  return exit_code;
}

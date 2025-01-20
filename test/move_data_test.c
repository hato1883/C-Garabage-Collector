#include "../src/format_encoding.h"
#include "../src/gc.h"
#include "../src/move_data.h"
#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

int
init_suite (void)
{
  // Change this function if you want to do something *before* you
  // run a test suite
  return 0;
}

int
clean_suite (void)
{
  // Change this function if you want to do something *after* you
  // run a test suite
  return 0;
}

void
test_move_data_bit_vector (void)
{
  typedef struct test_struct t;
  struct test_struct
  {
    void *c;
    int i;
  };
  // return;
  heap_t *h = h_init (512, true, 0.5);

  char c1 = 'a';
  int i1 = 2;

  char c2 = 'b';
  int i2 = 3;

  t *alloc1 = (t *)h_alloc_struct (h, "*i");
  alloc1->c = &c1;
  alloc1->i = i1;

  t *alloc2 = (t *)h_alloc_raw (h, 32);
  alloc2->c = &c2;
  alloc2->i = i2;
  t *old_alloc1 = alloc1;
  t *old_alloc2 = alloc2;

  bool success = move_alloc ((void **)&alloc1, (void *)alloc2);
  CU_ASSERT_TRUE (success);
  CU_ASSERT_PTR_EQUAL (alloc1, alloc2);
  CU_ASSERT_PTR_EQUAL (old_alloc2, alloc2);
  CU_ASSERT_PTR_NOT_EQUAL (alloc1, old_alloc1);
  CU_ASSERT_PTR_EQUAL (alloc2->c, &c1);
  CU_ASSERT_EQUAL (alloc2->i, i1);

  header_t *old_alloc1_header_ptr
      = (header_t *)((char *)old_alloc1 - sizeof (header_t));
  header_t old_alloc1_header = get_header_value (old_alloc1_header_ptr);
  CU_ASSERT_EQUAL (get_header_type (old_alloc1_header),
                   HEADER_FORWARDING_ADDRESS);

  void *forwarding_address
      = (void *)(old_alloc1_header & ~(0x3)); // Sets the FORMAT bits to 00;
  CU_ASSERT_PTR_EQUAL (forwarding_address, alloc2);

  h_delete (h);
}

void
test_move_data_not_successful (void)
{
  typedef struct test_struct t;
  struct test_struct
  {
    void *c;
    int i;
  };
  // return;
  heap_t *h = h_init (512, true, 0.5);

  char c1 = 'a';
  int i1 = 2;

  char c2 = 'b';
  int i2 = 3;

  t *alloc1 = (t *)h_alloc_struct (h, "*i");
  alloc1->c = &c1;
  alloc1->i = i1;

  t *alloc2 = (t *)h_alloc_raw (h, 32);
  alloc2->c = &c2;
  alloc2->i = i2;
  t *old_alloc1 = alloc1;
  t *old_alloc2 = alloc2;

  header_t old_header = get_header_value (get_header_pointer (alloc1));
  update_header_of_a_pointer (
      alloc1,
      set_header_unused (
          old_header)); // Sets header to unused so that the move will fail.

  bool success = move_alloc ((void **)&alloc1, (void *)alloc2);
  CU_ASSERT_FALSE (success);
  CU_ASSERT_PTR_NOT_EQUAL (alloc1, alloc2);
  CU_ASSERT_PTR_EQUAL (old_alloc2, alloc2);
  CU_ASSERT_PTR_EQUAL (alloc1, old_alloc1);
  CU_ASSERT_PTR_NOT_EQUAL (alloc2->c, &c1);
  CU_ASSERT_NOT_EQUAL (alloc2->i, i1);

  header_t *old_alloc1_header_ptr
      = (header_t *)((char *)old_alloc1 - sizeof (header_t));
  header_t old_alloc1_header = get_header_value (old_alloc1_header_ptr);
  CU_ASSERT_NOT_EQUAL (get_header_type (old_alloc1_header),
                       HEADER_FORWARDING_ADDRESS);

  h_delete (h);
}

/* Main function to run the tests  */
int
main (void)
{
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  CU_pSuite suite = CU_add_suite ("Test Suite", init_suite, clean_suite);
  if (suite == NULL)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  if ((CU_add_test (suite, "Test moving data which has a bitvector header",
                    test_move_data_bit_vector)
       == NULL)
      || (CU_add_test (suite, "Test moving data unsuccessful",
                       test_move_data_not_successful)
          == NULL)
      || 0)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  CU_basic_set_mode (CU_BRM_VERBOSE);

  CU_basic_run_tests ();

  int exit_code = CU_get_number_of_tests_failed () == 0
                      ? CU_get_error ()
                      : CU_get_number_of_tests_failed ();

  CU_cleanup_registry ();

  return exit_code;
}

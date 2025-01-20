#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/gc.h"
#include "../src/header.h"
#include "../src/ptr_queue.h"

#define UNUSED(x) x __attribute__ ((__unused__))
#define SET_PTR_TEST_VAL 5 // A value used in test apply_func_to_ptrs

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
test_no_pointers_in_alloc ()
{
  struct test_struct
  {
    char c;
    int i;
    long l;
  };

  heap_t *h = h_init (512, true, 0.5);

  char *format_string = "cil";
  struct test_struct *t = h_alloc_struct (h, format_string);

  *t = (struct test_struct){ .c = 'o', .i = 7, .l = -82 };

  ptr_queue_t *pointers = get_pointers_in_allocation (t);

  CU_ASSERT_TRUE (is_empty_queue (pointers));

  destroy_ptr_queue (pointers);
  h_delete (h);
}

void
test_one_pointer_in_alloc ()
{
  struct test_struct
  {
    int i;
    void *p;
  };

  heap_t *h = h_init (512, true, 0.5);

  char *format_string = "i*";
  struct test_struct *t = h_alloc_struct (h, format_string);

  char *str = "banan";
  *t = (struct test_struct){ .i = 345, .p = str };

  ptr_queue_t *pointers = get_pointers_in_allocation (t);

  CU_ASSERT_PTR_EQUAL (str, *dequeue_ptr (pointers));
  CU_ASSERT_TRUE (is_empty_queue (pointers));

  destroy_ptr_queue (pointers);
  h_delete (h);
}

void
test_multiple_pointers_in_alloc ()
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

  heap_t *h = h_init (512, true, 0.5);

  char *format_string = "cd*i*lc*";
  struct test_struct *t = h_alloc_struct (h, format_string);

  char *str = "banan";
  bool b = true;
  *t = (struct test_struct){ .c = 'o',
                             .d = -43,
                             .p = str,
                             .i = 290,
                             .p2 = &b,
                             .l = 827,
                             .c2 = 'h',
                             .p3 = str };

  ptr_queue_t *pointers = get_pointers_in_allocation (t);

  void *lesser_ptr = (void *)str < (void *)&b ? (void *)str : (void *)&b;
  void *greater_ptr = (void *)str > (void *)&b ? (void *)str : (void *)&b;

  CU_ASSERT_PTR_EQUAL (lesser_ptr, *dequeue_ptr (pointers));
  CU_ASSERT_PTR_EQUAL (greater_ptr, *dequeue_ptr (pointers));
  CU_ASSERT_PTR_NOT_NULL (dequeue_ptr (pointers));
  CU_ASSERT_PTR_NULL (dequeue_ptr (pointers));

  destroy_ptr_queue (pointers);
  h_delete (h);
}

int
main (void)
{
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  CU_pSuite suite
      = CU_add_suite ("Get pointers in allocation", init_suite, clean_suite);

  if (suite == NULL)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  if (CU_add_test (suite,
                   "Finds pointer from bit vector with a char and a pointer",
                   test_one_pointer_in_alloc)
          == NULL
      || CU_add_test (
             suite,
             "Get pointers on vector without pointers returns empty queue",
             test_no_pointers_in_alloc)
             == NULL
      || CU_add_test (suite,
                      "Finds pointers from struct with multiple pointers, "
                      "where two are the same",
                      test_multiple_pointers_in_alloc)
             == NULL
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

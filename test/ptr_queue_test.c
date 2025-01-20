#include "../src/ptr_queue.h"
#include <CUnit/Basic.h>
#include <assert.h>

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
test_enqueue_dequeue_one_ptr ()
{
  ptr_queue_t *q = create_ptr_queue ();

  char a = 'a';

  CU_ASSERT_TRUE (enqueue_ptr (q, &a));

  CU_ASSERT_PTR_EQUAL (dequeue_ptr (q), &a);

  destroy_ptr_queue (q);
}

void
test_enqueue_dequeue_two_ptrs_in_order ()
{
  ptr_queue_t *q = create_ptr_queue ();

  char a = 'a';
  char b = 'b';

  CU_ASSERT_TRUE (enqueue_ptr (q, &a));
  CU_ASSERT_TRUE (enqueue_ptr (q, &b));

  // I assume &a will be smaller, this is just in case I'm wrong
  char *lesser_ptr = &a < &b ? &a : &b;
  char *greater_ptr = &a > &b ? &a : &b;

  CU_ASSERT_PTR_EQUAL (dequeue_ptr (q), lesser_ptr);
  CU_ASSERT_PTR_EQUAL (dequeue_ptr (q), greater_ptr);
  CU_ASSERT_PTR_NULL (dequeue_ptr (q));

  destroy_ptr_queue (q);
}

void
test_enqueue_dequeue_two_ptrs_in_opposite_order ()
{
  ptr_queue_t *q = create_ptr_queue ();

  char a = 'a';
  char b = 'b';

  CU_ASSERT_TRUE (enqueue_ptr (q, &b));
  CU_ASSERT_TRUE (enqueue_ptr (q, &a));

  // I assume &a will be smaller, this is just in case I'm wrong
  char *lesser_ptr = &a < &b ? &a : &b;
  char *greater_ptr = &a > &b ? &a : &b;

  CU_ASSERT_PTR_EQUAL (dequeue_ptr (q), lesser_ptr);
  CU_ASSERT_PTR_EQUAL (dequeue_ptr (q), greater_ptr);
  CU_ASSERT_PTR_NULL (dequeue_ptr (q));

  destroy_ptr_queue (q);
}

void
test_enqueue_dequeue_nine_pointers ()
{
  ptr_queue_t *q = create_ptr_queue ();

  char a = 'a';
  char b = 'b';
  char *hej = "hej";
  int one = 1;
  char c = 'c';
  int two = 2;
  char d = 'd';
  char *san = "san";
  int fortytwo = 2;

  CU_ASSERT_TRUE (enqueue_ptr (q, &san));
  CU_ASSERT_TRUE (enqueue_ptr (q, &fortytwo));
  CU_ASSERT_TRUE (enqueue_ptr (q, &a));
  CU_ASSERT_TRUE (enqueue_ptr (q, &c));
  CU_ASSERT_TRUE (enqueue_ptr (q, &one));
  CU_ASSERT_TRUE (enqueue_ptr (q, &b));
  CU_ASSERT_TRUE (enqueue_ptr (q, &hej));
  CU_ASSERT_TRUE (enqueue_ptr (q, &d));
  CU_ASSERT_TRUE (enqueue_ptr (q, &two));

  void *dequeued = dequeue_ptr (q);
  for (int i = 0; i < 8; i++)
    {
      void *next_dequeued = dequeue_ptr (q);
      CU_ASSERT_TRUE (dequeued < next_dequeued);
      dequeued = next_dequeued;
    }

  CU_ASSERT_PTR_NULL (dequeue_ptr (q));

  destroy_ptr_queue (q);
}

void
test_enqueue_pointer_already_in_queue ()
{
  ptr_queue_t *q = create_ptr_queue ();

  char a = 'a';
  char b = 'b';

  CU_ASSERT_TRUE (enqueue_ptr (q, &a));
  CU_ASSERT_FALSE (enqueue_ptr (q, &a));
  CU_ASSERT_TRUE (enqueue_ptr (q, &b));
  CU_ASSERT_FALSE (enqueue_ptr (q, &b));
  CU_ASSERT_FALSE (enqueue_ptr (q, &a));

  // I assume &a will be smaller, this is just in case I'm wrong
  char *lesser_ptr = &a < &b ? &a : &b;
  char *greater_ptr = &a > &b ? &a : &b;

  CU_ASSERT_PTR_EQUAL (dequeue_ptr (q), lesser_ptr);
  CU_ASSERT_PTR_EQUAL (dequeue_ptr (q), greater_ptr);
  CU_ASSERT_PTR_NULL (dequeue_ptr (q));

  destroy_ptr_queue (q);
}

void
test_is_empty ()
{
  ptr_queue_t *q = create_ptr_queue ();

  char a = 'a';
  char b = 'b';
  char *hej = "hej";
  int one = 1;

  CU_ASSERT_TRUE (is_empty_queue (q));

  enqueue_ptr (q, &a);
  CU_ASSERT_FALSE (is_empty_queue (q));

  enqueue_ptr (q, &one);
  CU_ASSERT_FALSE (is_empty_queue (q));

  enqueue_ptr (q, &b);
  CU_ASSERT_FALSE (is_empty_queue (q));

  enqueue_ptr (q, &one);
  CU_ASSERT_FALSE (is_empty_queue (q));

  enqueue_ptr (q, &hej);
  CU_ASSERT_FALSE (is_empty_queue (q));

  enqueue_ptr (q, &hej);
  CU_ASSERT_FALSE (is_empty_queue (q));

  enqueue_ptr (q, &b);
  CU_ASSERT_FALSE (is_empty_queue (q));

  for (int i = 0; i < 4; i++)
    {
      CU_ASSERT_FALSE (is_empty_queue (q));
      dequeue_ptr (q);
    }

  CU_ASSERT_TRUE (is_empty_queue (q));

  destroy_ptr_queue (q);
}

int
main ()
{
  /* Initialize CUnit test registry  */
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  /* Add suite to registry  */
  CU_pSuite suite
      = CU_add_suite ("Pointer queue test suite", init_suite, clean_suite);
  if (!suite)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  /* Add tests to the suite  */
  if ((CU_add_test (suite, "Enqueueing and dequeueing one pointer",
                    test_enqueue_dequeue_one_ptr)
       == NULL)
      || (CU_add_test (suite,
                       "Enqueueing two pointers in the order the variables "
                       "were created, dequeues correctly",
                       test_enqueue_dequeue_two_ptrs_in_order)
          == NULL)
      || (CU_add_test (suite,
                       "Enqueueing two pointers in the opposite order of when "
                       "the variables were created, dequeues correctly",
                       test_enqueue_dequeue_two_ptrs_in_opposite_order)
          == NULL)
      || (CU_add_test (suite,
                       "Enqueueing nine pointers in a 'random' order, "
                       "dequeues correctly",
                       test_enqueue_dequeue_nine_pointers)
          == NULL)
      || (CU_add_test (
              suite,
              "Enqueueing a pointer that is already in the queue does nothing",
              test_enqueue_pointer_already_in_queue)
          == NULL)
      || (CU_add_test (suite, "is_empty_queue works", test_is_empty) == NULL)
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

#include <CUnit/Basic.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/common.h"
#include "../../lib/fifo_queue.h"
#include "../../src/gc.h"

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

#define Create_Queue(name)                                                    \
  ioopm_queue_t *name = NULL;                                                 \
  int status = ioopm_queue_create (&name);                                    \
  CU_ASSERT_EQUAL (status, SUCCESS);                                          \
  CU_ASSERT_PTR_NOT_NULL_FATAL (name);

#define Destroy_Queue(name)                                                   \
  {                                                                           \
    int status = ioopm_queue_destroy (&name);                                 \
    CU_ASSERT_EQUAL (status, SUCCESS);                                        \
    CU_ASSERT_PTR_NULL (name);                                                \
  }

void
create_destroy_fifo_queue_test (void)
{
  ioopm_queue_t *queue = NULL;
  int status = ioopm_queue_create (&queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_PTR_NOT_NULL_FATAL (queue);

  status = ioopm_queue_destroy (&queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_PTR_NULL (queue);
}

void
enqueue_three_elements_test (void)
{
  Create_Queue (queue);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_enqueue (queue, int_to_elem (1));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_enqueue (queue, int_to_elem (2));
  CU_ASSERT_EQUAL (status, SUCCESS);

  Destroy_Queue (queue);
}

void
enqueue_three_elements_with_peek_test (void)
{
  elem_t valid = ptr_to_elem (NULL);
  Create_Queue (queue);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);
  status = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_queue_enqueue (queue, int_to_elem (1));
  CU_ASSERT_EQUAL (status, SUCCESS);
  status = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_queue_enqueue (queue, int_to_elem (2));
  CU_ASSERT_EQUAL (status, SUCCESS);
  status = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  Destroy_Queue (queue);
}

void
enqueue_three_and_dequeue_three_test (void)
{
  elem_t valid = ptr_to_elem (NULL);
  Create_Queue (queue);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_enqueue (queue, int_to_elem (1));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_enqueue (queue, int_to_elem (2));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 1);

  status = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 1);

  status = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 2);

  status = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 2);

  Destroy_Queue (queue);
}

void
enqueue_three_elements_and_check_size_test (void)
{
  size_t size = 0;
  Create_Queue (queue);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 0);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 1);

  status = ioopm_queue_enqueue (queue, int_to_elem (1));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 2);

  status = ioopm_queue_enqueue (queue, int_to_elem (2));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 3);

  Destroy_Queue (queue);
}

void
enqueue_three_and_dequeue_three_check_size_test (void)
{
  size_t size = 0;
  elem_t valid = ptr_to_elem (NULL);
  Create_Queue (queue);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 0);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 1);

  status = ioopm_queue_enqueue (queue, int_to_elem (1));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 2);

  status = ioopm_queue_enqueue (queue, int_to_elem (2));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 3);

  status = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 2);

  status = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 1);

  status = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 1);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 1);

  status = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 2);

  status = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 2);

  status = ioopm_queue_size (&size, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (size, 0);

  Destroy_Queue (queue);
}

void
newly_created_is_empty_test (void)
{
  bool is_empty = true;
  Create_Queue (queue);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  Destroy_Queue (queue);
}

void
enqueued_is_not_empty_test (void)
{
  bool is_empty = true;
  Create_Queue (queue);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_FALSE (is_empty);

  Destroy_Queue (queue);
}

void
used_is_empty_test (void)
{
  bool is_empty = true;
  elem_t valid = ptr_to_elem (NULL);
  Create_Queue (queue);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_FALSE (is_empty);

  status = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_enqueue (queue, int_to_elem (1));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_FALSE (is_empty);

  status = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  Destroy_Queue (queue);
}

void
clearing_used_becomes_empty_test (void)
{
  bool is_empty = true;
  Create_Queue (queue);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_FALSE (is_empty);

  status = ioopm_queue_clear (queue);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_enqueue (queue, int_to_elem (1));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_FALSE (is_empty);

  status = ioopm_queue_clear (queue);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  Destroy_Queue (queue);
}

void
clearing_empty_becomes_empty_test (void)
{
  bool is_empty = true;
  Create_Queue (queue);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  status = ioopm_queue_clear (queue);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_is_empty (&is_empty, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (is_empty);

  Destroy_Queue (queue);
}

void
iterator_on_empty_test (void)
{
  ioopm_iterator_t *iter = NULL;
  bool has_next = false;
  Create_Queue (queue);

  status = ioopm_queue_iterator (&iter, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_PTR_NOT_NULL_FATAL (iter);

  status = ioopm_iterator_has_next (&has_next, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_FALSE (has_next);

  ioopm_iterator_destroy (&iter);
  Destroy_Queue (queue);
}

void
iterator_on_non_empty_test (void)
{
  ioopm_iterator_t *iter = NULL;
  bool has_next = false;
  elem_t valid = ptr_to_elem (NULL);
  Create_Queue (queue);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_iterator (&iter, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_PTR_NOT_NULL_FATAL (iter);

  status = ioopm_iterator_has_next (&has_next, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (has_next);

  status = ioopm_iterator_next (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_iterator_has_next (&has_next, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_FALSE (has_next);

  ioopm_iterator_destroy (&iter);
  Destroy_Queue (queue);
}

void
iterator_on_length_three_test (void)
{
  ioopm_iterator_t *iter = NULL;
  bool has_next = false;
  elem_t valid = ptr_to_elem (NULL);
  Create_Queue (queue);

  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_enqueue (queue, int_to_elem (1));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_enqueue (queue, int_to_elem (2));
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_queue_iterator (&iter, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_PTR_NOT_NULL_FATAL (iter);

  status = ioopm_iterator_has_next (&has_next, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (has_next);

  status = ioopm_iterator_next (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_iterator_current (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_iterator_has_next (&has_next, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (has_next);

  status = ioopm_iterator_next (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 1);

  status = ioopm_iterator_current (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 1);

  status = ioopm_iterator_has_next (&has_next, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_TRUE (has_next);

  status = ioopm_iterator_next (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 2);

  status = ioopm_iterator_current (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 2);

  status = ioopm_iterator_has_next (&has_next, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_FALSE (has_next);

  status = ioopm_iterator_reset (iter);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_iterator_next (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_iterator_current (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 0);

  status = ioopm_iterator_next (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 1);

  status = ioopm_iterator_current (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_EQUAL (elem_to_int (valid), 1);

  ioopm_iterator_destroy (&iter);
  Destroy_Queue (queue);
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite fifo_queue_test_suite
      = CU_add_suite ("Hash Table Testing Suite", init_suite, clean_suite);
  if (fifo_queue_test_suite == NULL)
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
  if ((CU_add_test (fifo_queue_test_suite, "Create and destroy a fifo queue",
                    create_destroy_fifo_queue_test)
       == NULL)
      || (CU_add_test (fifo_queue_test_suite,
                       "Enqueue three elements to a fifo queue",
                       enqueue_three_elements_test)
          == NULL)
      || (CU_add_test (fifo_queue_test_suite,
                       "Enqueue three elements to a fifo queue and peek the "
                       "first element each time",
                       enqueue_three_elements_with_peek_test)
          == NULL)
      || (CU_add_test (
              fifo_queue_test_suite,
              "Enqueue three elements to a fifo queue and then dequeue them",
              enqueue_three_and_dequeue_three_test)
          == NULL)
      || (CU_add_test (fifo_queue_test_suite, "Check size when queue grows",
                       enqueue_three_elements_and_check_size_test)
          == NULL)
      || (CU_add_test (fifo_queue_test_suite,
                       "Check size when queue grows and shrinks",
                       enqueue_three_and_dequeue_three_check_size_test)
          == NULL)
      || (CU_add_test (fifo_queue_test_suite, "Newly created queue is empty",
                       newly_created_is_empty_test)
          == NULL)
      || (CU_add_test (fifo_queue_test_suite,
                       "Queue with item enqueued is not empty",
                       enqueued_is_not_empty_test)
          == NULL)
      || (CU_add_test (
              fifo_queue_test_suite,
              "Queue which had items but no longer dose is also empty",
              used_is_empty_test)
          == NULL)
      || (CU_add_test (fifo_queue_test_suite,
                       "Empty Queue remains empty after Clearing it",
                       clearing_empty_becomes_empty_test)
          == NULL)
      || (CU_add_test (fifo_queue_test_suite,
                       "Non Empty Queue becomes empty after Clearing it",
                       clearing_used_becomes_empty_test)
          == NULL)
      || (CU_add_test (
              fifo_queue_test_suite,
              "Creating a iterator on empty queue yields empty iterator",
              iterator_on_empty_test)
          == NULL)
      || (CU_add_test (fifo_queue_test_suite,
                       "Creating a iterator on non empty queue yields non "
                       "empty iterator",
                       iterator_on_non_empty_test)
          == NULL)
      || (CU_add_test (fifo_queue_test_suite,
                       "Iterator has same order as queue dequeue",
                       iterator_on_length_three_test)
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

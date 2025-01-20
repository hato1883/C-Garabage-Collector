#include <CUnit/Basic.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../lib/fifo_queue.h"
#include "../../src/gc.h"
#include "oom.h"

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
create_fifo_queue_allocation_failure_test (void)
{
  ioopm_queue_t *valid_queue = NULL;
  oom_next_alloc (false);
  int status = ioopm_queue_create (&valid_queue);
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();
}

void
create_fifo_queue_invalid_pointer_test (void)
{
  int status = ioopm_queue_create (NULL);
  CU_ASSERT_EQUAL (status, INVALID_QUEUE_POINTER);
}

void
destroy_fifo_queue_invalid_pointer_test (void)
{
  int status = ioopm_queue_destroy (NULL);
  CU_ASSERT_EQUAL (status, INVALID_QUEUE_POINTER);
}

void
enqueue_element_during_oom_test (void)
{
  Create_Queue (queue);

  oom_next_alloc (false);
  status = ioopm_queue_enqueue (queue, int_to_elem (0));
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  Destroy_Queue (queue)
}

void
enqueue_element_with_invalid_pointer_test (void)
{
  int status = ioopm_queue_enqueue (NULL, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INVALID_QUEUE_POINTER);
}

void
peek_with_invalid_queue_pointer_test (void)
{
  elem_t valid = ptr_to_elem (NULL);
  int status = ioopm_queue_peek (&valid, NULL);
  CU_ASSERT_EQUAL (status, INVALID_QUEUE_POINTER);
}

void
peek_with_invalid_return_pointer_test (void)
{
  Create_Queue (queue)

      status
      = ioopm_queue_peek (NULL, queue);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  Destroy_Queue (queue)
}

void
peek_on_empty_queue_test (void)
{
  elem_t valid = ptr_to_elem (NULL);
  Create_Queue (queue)

      status
      = ioopm_queue_peek (&valid, queue);
  CU_ASSERT_EQUAL (status, QUEUE_IS_EMPTY);

  Destroy_Queue (queue)
}

void
dequeue_with_invalid_queue_pointer_test (void)
{
  elem_t valid = ptr_to_elem (NULL);
  int status = ioopm_queue_dequeue (&valid, NULL);
  CU_ASSERT_EQUAL (status, INVALID_QUEUE_POINTER);
}

void
dequeue_with_invalid_return_pointer_test (void)
{
  Create_Queue (queue)

      status
      = ioopm_queue_dequeue (NULL, queue);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  Destroy_Queue (queue)
}

void
dequeue_on_empty_queue_test (void)
{
  elem_t valid = ptr_to_elem (NULL);
  Create_Queue (queue)

      status
      = ioopm_queue_dequeue (&valid, queue);
  CU_ASSERT_EQUAL (status, QUEUE_IS_EMPTY);

  Destroy_Queue (queue)
}

void
size_with_invalid_queue_pointer_test (void)
{
  size_t valid = 0;
  int status = ioopm_queue_size (&valid, NULL);
  CU_ASSERT_EQUAL (status, INVALID_QUEUE_POINTER);
}

void
size_with_invalid_return_pointer_test (void)
{
  Create_Queue (queue)

      status
      = ioopm_queue_size (NULL, queue);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  Destroy_Queue (queue)
}

void
is_empty_with_invalid_queue_pointer_test (void)
{
  bool valid = 0;
  int status = ioopm_queue_is_empty (&valid, NULL);
  CU_ASSERT_EQUAL (status, INVALID_QUEUE_POINTER);
}

void
is_empty_with_invalid_return_pointer_test (void)
{
  Create_Queue (queue)

      status
      = ioopm_queue_is_empty (NULL, queue);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  Destroy_Queue (queue)
}

void
clear_with_invalid_queue_pointer_test (void)
{
  int status = ioopm_queue_clear (NULL);
  CU_ASSERT_EQUAL (status, INVALID_QUEUE_POINTER);
}

void
creating_iterator_with_invalid_queue_pointer_test (void)
{
  ioopm_iterator_t *valid = NULL;
  int status = ioopm_queue_iterator (&valid, NULL);
  CU_ASSERT_EQUAL (status, INVALID_QUEUE_POINTER);
}

void
creating_iterator_with_invalid_return_pointer_test (void)
{
  Create_Queue (queue)

      status
      = ioopm_queue_iterator (NULL, queue);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  Destroy_Queue (queue)
}

void
creating_iterator_during_oom_test (void)
{
  ioopm_iterator_t *valid = NULL;
  Create_Queue (queue) oom_next_alloc (false);

  status = ioopm_queue_iterator (&valid, queue);
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);

  set_normal_alloc ();
  Destroy_Queue (queue)
}
void
creating_iterator_during_oom_internal_test (void)
{
  ioopm_iterator_t *valid = NULL;
  Create_Queue (queue) oom_nth_alloc_call (1, false);

  status = ioopm_queue_iterator (&valid, queue);
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);

  set_normal_alloc ();
  Destroy_Queue (queue)
}

void
fifo_iterator_next_on_empty_test (void)
{
  elem_t valid = ptr_to_elem (NULL);
  ioopm_iterator_t *iter = NULL;
  Create_Queue (queue);

  status = ioopm_queue_iterator (&iter, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_PTR_NOT_NULL_FATAL (iter);

  status = ioopm_iterator_next (&valid, iter);
  CU_ASSERT_EQUAL (status, ITERATOR_IS_EMPTY);

  ioopm_iterator_destroy (&iter);
  Destroy_Queue (queue);
}

void
fifo_iterator_current_in_invalid_state_test (void)
{
  elem_t valid = ptr_to_elem (NULL);
  ioopm_iterator_t *iter = NULL;
  Create_Queue (queue);

  status = ioopm_queue_iterator (&iter, queue);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_PTR_NOT_NULL_FATAL (iter);

  status = ioopm_iterator_current (&valid, iter);
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_STATE);

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
  CU_pSuite fifo_queue_error_test_suite = CU_add_suite (
      "Fifo Queue Error handling Test Suite", init_suite, clean_suite);
  if (fifo_queue_error_test_suite == NULL)
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
  if ((CU_add_test (fifo_queue_error_test_suite,
                    "Create a fifo queue during OOM event when creating fifo "
                    "queue pointer",
                    create_fifo_queue_allocation_failure_test)
       == NULL)
      || (CU_add_test (
              fifo_queue_error_test_suite,
              "Report error when a null pointer is sent to Create queue",
              create_fifo_queue_invalid_pointer_test)
          == NULL)
      || (CU_add_test (
              fifo_queue_error_test_suite,
              "Report error when a null pointer is sent to Destroy queue",
              destroy_fifo_queue_invalid_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Report error when enqueuing elements during OOM",
                       enqueue_element_during_oom_test)
          == NULL)
      || (CU_add_test (
              fifo_queue_error_test_suite,
              "Report error when a null pointer is sent to Enqueue Element",
              enqueue_element_with_invalid_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when peeking a empty queue",
                       peek_on_empty_queue_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when peeking a null pointer",
                       peek_with_invalid_queue_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when peeking with null pointer return address",
                       peek_with_invalid_return_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when dequeueing a empty queue",
                       dequeue_on_empty_queue_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when dequeueing a null pointer",
                       dequeue_with_invalid_queue_pointer_test)
          == NULL)
      || (CU_add_test (
              fifo_queue_error_test_suite,
              "Error when dequeueing with null pointer return address",
              dequeue_with_invalid_return_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when checking if a null pointer is empty",
                       is_empty_with_invalid_queue_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when checking if queue is empty with invalid "
                       "return address",
                       is_empty_with_invalid_return_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when checking size on null pointer",
                       size_with_invalid_queue_pointer_test)
          == NULL)
      || (CU_add_test (
              fifo_queue_error_test_suite,
              "Error when checking size with null pointer return address",
              size_with_invalid_return_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when clearing a null pointer",
                       clear_with_invalid_queue_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when creating iterator during OOM",
                       creating_iterator_during_oom_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when creating iterator during OOM internal",
                       creating_iterator_during_oom_internal_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when creating iterator from a null pointer",
                       creating_iterator_with_invalid_queue_pointer_test)
          == NULL)
      || (CU_add_test (
              fifo_queue_error_test_suite,
              "Error when creating iterator with invalid return address",
              creating_iterator_with_invalid_return_pointer_test)
          == NULL)
      || (CU_add_test (fifo_queue_error_test_suite,
                       "Error when trying to step thru empty fifo iterator",
                       fifo_iterator_next_on_empty_test)
          == NULL)
      || (CU_add_test (
              fifo_queue_error_test_suite,
              "Error when trying to get current without steping fifo iterator",
              fifo_iterator_current_in_invalid_state_test)
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

#include <CUnit/Basic.h>
#include <stdlib.h>

#include "../../lib/common.h"
#include "../../lib/iterator.h"
#include "../../lib/linked_list.h"
#include "../../src/gc.h"
#include "oom.h"

int
init_suite (void)
{
  h_init (1024 * 16, true, 0.75f);
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
setup_invalid_return (ioopm_iterator_t **return_iter, ioopm_list_t **list,
                      bool should_insert)
{
  int status = SUCCESS;

  status = ioopm_linked_list_create (list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);
  if (should_insert)
    {
      status = ioopm_linked_list_append (*list, int_to_elem (0));
      CU_ASSERT_EQUAL (status, SUCCESS);
    }

  status = ioopm_list_iterator (return_iter, *list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

void
setup_invalid_internal_list_iterator (ioopm_iterator_t **return_iter)
{
  int status = ioopm_iterator_create (return_iter, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

void
teardown_invalid_internal_list_iterator (ioopm_iterator_t **iter)
{
  // free (*iter);
}

void
teardown_invalid_return (ioopm_iterator_t *iter, ioopm_list_t *list)
{
  int status = SUCCESS;

  status = ioopm_iterator_destroy (&iter);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_linked_list_destroy (&list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_list_iterator reports invalid list pointer
void
create_iterator_invalid_list_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_iterator_t *iter = NULL;

  status = ioopm_list_iterator (&iter, NULL);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_list_iterator reports invalid iter pointer
void
create_iterator_invalid_iter_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *list = NULL;
  status = ioopm_linked_list_create (&list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_list_iterator (NULL, list);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_linked_list_destroy (&list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_list_iterator reports OOM Error during allocation of
/// ioopm_iterator_t pointer
void
create_iterator_early_allocation_failure_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;

  status = ioopm_linked_list_create (&list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  oom_next_alloc (false);
  status = ioopm_list_iterator (&iter, list);
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  status = ioopm_linked_list_destroy (&list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_list_iterator reports OOM Error during allocation of
/// ioopm_iterator_t pointer
void
create_iterator_late_allocation_failure_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;

  status = ioopm_linked_list_create (&list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // There are two malloc calls in ioopm_list_iterator
  // One to create a list_iterator_t which is the underlying data structure
  // Second is when we allocate memory for the ioopm_iterator_t pointer
  oom_nth_alloc_call (1, false);
  status = ioopm_list_iterator (&iter, list);
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  status = ioopm_linked_list_destroy (&list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_iterator_has_next reports invalid iter pointer
void
has_next_invalid_iter_ptr_test (void)
{
  int status = SUCCESS;
  bool valid_return = false;

  status = ioopm_iterator_has_next (&valid_return, NULL);
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_POINTER);
}

/// @brief Test if ioopm_iterator_has_next reports invalid return pointer
void
has_next_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_return (&iter, &list, false);

  status = ioopm_iterator_has_next (NULL, iter);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  teardown_invalid_return (iter, list);
}

/// @brief Test if ioopm_iterator_next reports invalid iter pointer
void
next_invalid_iter_ptr_test (void)
{
  int status = SUCCESS;
  elem_t valid_return;

  status = ioopm_iterator_next (&valid_return, NULL);
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_POINTER);
}

/// @brief Test if ioopm_iterator_next reports invalid return pointer
void
next_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_return (&iter, &list, false);

  status = ioopm_iterator_next (NULL, iter);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  teardown_invalid_return (iter, list);
}

/// @brief Test if ioopm_iterator_next reports if iterator is empty
void
next_on_empty_test (void)
{
  int status = SUCCESS;
  elem_t valid_return;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_return (&iter, &list, false);

  status = ioopm_iterator_next (&valid_return, iter);
  CU_ASSERT_EQUAL (status, ITERATOR_IS_EMPTY);

  teardown_invalid_return (iter, list);
}

/// @brief Test if ioopm_iterator_remove reports invalid iter pointer
void
remove_invalid_iter_ptr_test (void)
{
  int status = SUCCESS;
  elem_t valid_return;

  status = ioopm_iterator_remove (&valid_return, NULL);
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_POINTER);
}

/// @brief Test if ioopm_iterator_remove reports invalid return pointer
void
remove_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_return (&iter, &list, false);

  status = ioopm_iterator_remove (NULL, iter);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  teardown_invalid_return (iter, list);
}

/// @brief Test if ioopm_iterator_remove reports that iterator is in a invalid
/// state for this action
void
remove_invalid_state_test (void)
{
  int status = SUCCESS;
  elem_t valid_return;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_return (&iter, &list, false);

  status = ioopm_iterator_remove (&valid_return, iter);
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_STATE);

  teardown_invalid_return (iter, list);
}

/// @brief Test if ioopm_iterator_insert reports invalid iter pointer
void
insert_invalid_iter_ptr_test (void)
{
  int status = SUCCESS;

  status = ioopm_iterator_insert (NULL, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_POINTER);
}

/// @brief Test if ioopm_iterator_insert reports that iterator is in a invalid
/// state for this action
void
insert_invalid_state_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_return (&iter, &list, false);

  status = ioopm_iterator_insert (iter, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_STATE);

  teardown_invalid_return (iter, list);
}

/// @brief Test if ioopm_iterator_insert reports OOM error
void
insert_oom_test (void)
{
  int status = SUCCESS;
  elem_t valid;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_return (&iter, &list, true);

  status = ioopm_iterator_next (&valid, iter);
  CU_ASSERT_EQUAL (status, SUCCESS);

  oom_next_alloc (false);
  status = ioopm_iterator_insert (iter, int_to_elem (0));
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  teardown_invalid_return (iter, list);
}

/// @brief Test if ioopm_iterator_reset reports invalid iter pointer
void
reset_invalid_iter_ptr_test (void)
{
  int status = SUCCESS;

  status = ioopm_iterator_reset (NULL);
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_POINTER);
}

/// @brief Test if ioopm_iterator_current reports invalid iter pointer
void
current_invalid_iter_ptr_test (void)
{
  int status = SUCCESS;
  elem_t valid_return;

  status = ioopm_iterator_current (&valid_return, NULL);
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_POINTER);
}

/// @brief Test if ioopm_iterator_current reports invalid iter pointer
void
current_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_return (&iter, &list, false);

  status = ioopm_iterator_current (NULL, iter);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  teardown_invalid_return (iter, list);
}

/// @brief Test if ioopm_iterator_current reports that iterator is in a invalid
/// state for this action
void
current_invalid_state_test (void)
{
  int status = SUCCESS;
  elem_t valid_return;
  ioopm_list_t *list = NULL;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_return (&iter, &list, false);

  status = ioopm_iterator_current (&valid_return, iter);
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_STATE);

  teardown_invalid_return (iter, list);
}

/// @brief Test if ioopm_iterator_destroy reports invalid iter pointer
void
destroy_invalid_iter_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_iterator_t *invalid_iter = NULL;
  status = ioopm_iterator_destroy (&invalid_iter);
  CU_ASSERT_EQUAL (status, INVALID_ITERATOR_POINTER);
}

/// @brief Test if ioopm_list_iterator reports invalid list pointer
void
iterator_unsupported_function_has_next_test (void)
{
  bool has_next = false;
  int status = SUCCESS;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_internal_list_iterator (&iter);

  status = ioopm_iterator_has_next (&has_next, iter);
  CU_ASSERT_EQUAL (status, ITERATOR_OPERATION_UNSUPPORTED);

  teardown_invalid_internal_list_iterator (&iter);
}

/// @brief Test if ioopm_list_iterator reports invalid list pointer
void
iterator_unsupported_function_next_test (void)
{
  elem_t next_elem = ptr_to_elem (NULL);
  int status = SUCCESS;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_internal_list_iterator (&iter);

  status = ioopm_iterator_next (&next_elem, iter);
  CU_ASSERT_EQUAL (status, ITERATOR_OPERATION_UNSUPPORTED);

  teardown_invalid_internal_list_iterator (&iter);
}

/// @brief Test if ioopm_list_iterator reports invalid list pointer
void
iterator_unsupported_function_current_test (void)
{
  elem_t curr_elem = ptr_to_elem (NULL);
  int status = SUCCESS;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_internal_list_iterator (&iter);

  status = ioopm_iterator_current (&curr_elem, iter);
  CU_ASSERT_EQUAL (status, ITERATOR_OPERATION_UNSUPPORTED);

  teardown_invalid_internal_list_iterator (&iter);
}

/// @brief Test if ioopm_list_iterator reports invalid list pointer
void
iterator_unsupported_function_remove_test (void)
{
  elem_t rem_elem = ptr_to_elem (NULL);
  int status = SUCCESS;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_internal_list_iterator (&iter);

  status = ioopm_iterator_remove (&rem_elem, iter);
  CU_ASSERT_EQUAL (status, ITERATOR_OPERATION_UNSUPPORTED);

  teardown_invalid_internal_list_iterator (&iter);
}

/// @brief Test if ioopm_list_iterator reports invalid list pointer
void
iterator_unsupported_function_insert_test (void)
{
  int status = SUCCESS;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_internal_list_iterator (&iter);

  status = ioopm_iterator_insert (iter, ptr_to_elem (NULL));
  CU_ASSERT_EQUAL (status, ITERATOR_OPERATION_UNSUPPORTED);

  teardown_invalid_internal_list_iterator (&iter);
}

/// @brief Test if ioopm_list_iterator reports invalid list pointer
void
iterator_unsupported_function_reset_test (void)
{
  int status = SUCCESS;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_internal_list_iterator (&iter);

  status = ioopm_iterator_reset (iter);
  CU_ASSERT_EQUAL (status, ITERATOR_OPERATION_UNSUPPORTED);

  teardown_invalid_internal_list_iterator (&iter);
}

/// @brief Test if ioopm_list_iterator reports invalid list pointer
void
iterator_unsupported_function_destroy_test (void)
{
  int status = SUCCESS;
  ioopm_iterator_t *iter = NULL;
  setup_invalid_internal_list_iterator (&iter);

  status = ioopm_iterator_destroy (&iter);
  CU_ASSERT_EQUAL (status, ITERATOR_OPERATION_UNSUPPORTED);

  teardown_invalid_internal_list_iterator (&iter);
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite iterator_error_test_suite = CU_add_suite (
      "Iterator Error handling Test Suite", init_suite, clean_suite);
  if (iterator_error_test_suite == NULL)
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
  if ((CU_add_test (iterator_error_test_suite,
                    "Check if invalid list pointer is handled by create",
                    create_iterator_invalid_list_ptr_test)
       == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid iterator pointer is handled by create",
              create_iterator_invalid_iter_ptr_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Create a iterator during OOM event when creating "
                       "underlying iteration structure pointer",
                       create_iterator_early_allocation_failure_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Create a iterator during OOM event when creating "
                       "surface iterator pointer",
                       create_iterator_late_allocation_failure_test)
          == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid iterator pointer is handled by has next",
              has_next_invalid_iter_ptr_test)
          == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid return pointer is handled by has next",
              has_next_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if invalid iterator pointer is handled by next",
                       next_invalid_iter_ptr_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if invalid return pointer is handled by next",
                       next_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if nex on empty iterator is handled",
                       next_on_empty_test)
          == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid iterator pointer is handled by remove",
              remove_invalid_iter_ptr_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if invalid return pointer is handled by remove",
                       remove_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid state of iterator is handled by remove",
              remove_invalid_state_test)
          == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid iterator pointer is handled by insert",
              insert_invalid_iter_ptr_test)
          == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid state of iterator is handled by insert",
              insert_invalid_state_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "insert into iterator during OOM event",
                       insert_oom_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if invalid iterator pointer is handled by reset",
                       reset_invalid_iter_ptr_test)
          == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid iterator pointer is handled by current",
              current_invalid_iter_ptr_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if invalid return pointer is handled by current",
                       current_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid state of iterator is handled by current",
              current_invalid_state_test)
          == NULL)
      || (CU_add_test (
              iterator_error_test_suite,
              "Check if invalid iterator pointer is handled by destroy",
              destroy_invalid_iter_ptr_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if has_next being a unsupported function gets "
                       "reported to user",
                       iterator_unsupported_function_has_next_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if next being a unsupported function gets "
                       "reported to user",
                       iterator_unsupported_function_next_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if current being a unsupported function gets "
                       "reported to user",
                       iterator_unsupported_function_current_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if remove being a unsupported function gets "
                       "reported to user",
                       iterator_unsupported_function_remove_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if insert being a unsupported function gets "
                       "reported to user",
                       iterator_unsupported_function_insert_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if reset being a unsupported function gets "
                       "reported to user",
                       iterator_unsupported_function_reset_test)
          == NULL)
      || (CU_add_test (iterator_error_test_suite,
                       "Check if destroy being a unsupported function gets "
                       "reported to user",
                       iterator_unsupported_function_destroy_test)
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

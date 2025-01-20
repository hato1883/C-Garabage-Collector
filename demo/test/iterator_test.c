#include <CUnit/Basic.h>

#include "../../lib/iterator.h"
#include "../../lib/linked_list.h"
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

void
list_iterator_create_destroy (void)
{
  // Create a non empty linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);
  ioopm_linked_list_append (list, int_to_elem (0));

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

void
list_iterator_empty (void)
{
  // Create a non empty linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

void
list_iterator_has_next_empty (void)
{
  // Create a empty linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  bool has_next = false;
  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

void
list_iterator_has_next_one (void)
{
  // Create a linked list with one element
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);
  ioopm_linked_list_append (list, int_to_elem (0));

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  bool has_next = false;
  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

void
list_iterator_has_next_two (void)
{
  // Create a empty linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);
  ioopm_linked_list_append (list, int_to_elem (0));
  ioopm_linked_list_append (list, int_to_elem (1));

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  bool has_next = false;
  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

void
list_iterator_current_test (void)
{
  bool has_next = false;
  elem_t current;
  // Create a non empty linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);
  ioopm_linked_list_append (list, int_to_elem (0));

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  ioopm_iterator_current (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

void
list_iterator_next_test (void)
{
  bool has_next = false;
  elem_t current;

  // Create a non empty linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);
  ioopm_linked_list_append (list, int_to_elem (0));
  ioopm_linked_list_append (list, int_to_elem (1));

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  ioopm_iterator_current (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 1);

  ioopm_iterator_current (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 1);

  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

void
list_iterator_insert_test (void)
{
  bool has_next = false;
  elem_t current;
  // Create a non empty linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);
  ioopm_linked_list_append (list, int_to_elem (0));

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  ioopm_iterator_current (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  ioopm_iterator_insert (iter, int_to_elem (1));
  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  ioopm_iterator_reset (iter);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 1);

  ioopm_iterator_current (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 1);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  ioopm_iterator_current (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

void
list_iterator_remove_test (void)
{
  bool has_next = false;
  elem_t current;
  // Create a non empty linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);
  ioopm_linked_list_append (list, int_to_elem (0));
  ioopm_linked_list_append (list, int_to_elem (1));

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);
  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);
  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 1);

  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  // remove current element (1)
  ioopm_iterator_remove (&current, iter);
  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  // reset to start
  ioopm_iterator_reset (iter);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);
  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

void
list_iterator_reset_test (void)
{
  elem_t current;
  bool has_next = false;
  // Create a non empty linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, NULL);
  ioopm_linked_list_append (list, int_to_elem (0));
  ioopm_linked_list_append (list, int_to_elem (1));

  // Create iterator
  ioopm_iterator_t *iter = NULL;
  ioopm_list_iterator (&iter, list);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);
  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);
  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 1);

  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  ioopm_iterator_reset (iter);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);
  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 0);

  CU_ASSERT_TRUE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);
  ioopm_iterator_next (&current, iter);
  CU_ASSERT_EQUAL (elem_to_int (current), 1);

  CU_ASSERT_FALSE (ioopm_iterator_has_next (&has_next, iter) == 0 && has_next);

  // Destroy iterator
  ioopm_iterator_destroy (&iter);
  ioopm_linked_list_destroy (&list);
}

int
main ()
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite iterator_test_suite
      = CU_add_suite ("Iterator Testing Suite", init_suite, clean_suite);
  if (iterator_test_suite == NULL)
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
  if ((CU_add_test (iterator_test_suite,
                    "Create and destroy a linked list iterator ",
                    list_iterator_create_destroy)
       == NULL)
      || (CU_add_test (iterator_test_suite,
                       "Create a iterator on a empty list",
                       list_iterator_empty)
          == NULL)
      || (CU_add_test (iterator_test_suite, "View current iter value",
                       list_iterator_current_test)
          == NULL)
      || (CU_add_test (iterator_test_suite, "Empty iterator has no next",
                       list_iterator_has_next_empty)
          == NULL)
      || (CU_add_test (iterator_test_suite,
                       "Iterator with 1 element has no next",
                       list_iterator_has_next_one)
          == NULL)
      || (CU_add_test (iterator_test_suite,
                       "Iterator with 2 or more elements has next",
                       list_iterator_has_next_two)
          == NULL)
      || (CU_add_test (iterator_test_suite, "Step thru iterator once",
                       list_iterator_next_test)
          == NULL)
      || (CU_add_test (iterator_test_suite, "Insert element",
                       list_iterator_insert_test)
          == NULL)
      || (CU_add_test (iterator_test_suite, "Remove current",
                       list_iterator_remove_test)
          == NULL)
      || (CU_add_test (iterator_test_suite, "Reset iterator",
                       list_iterator_reset_test)
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
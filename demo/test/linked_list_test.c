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

bool
integer_eq (elem_t a, elem_t b)
{
  return elem_to_int (a) == elem_to_int (b);
}

void
create_destroy_empty_list_test (void)
{
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);

  CU_ASSERT_PTR_NOT_NULL_FATAL (list);

  ioopm_linked_list_destroy (&list);
}

// Appends 10 elements to linked list
void
append_contains_test (void)
{
  bool contains = false;
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);

  // Verify that contains function returns false for empty list
  ioopm_linked_list_contains (&contains, list, int_to_elem (1));
  CU_ASSERT_FALSE (contains);

  // Append 10 entries to list
  for (int i = 0; i < 10; i++)
    {
      ioopm_linked_list_append (list, int_to_elem (i));
    }

  // Verify that the elements indeed were appended
  for (int i = 0; i < 10; i++)
    {
      ioopm_linked_list_contains (&contains, list, int_to_elem (i));
      CU_ASSERT_TRUE (contains);
    }

  // Verify that contains function returns false for an element not in a
  // populated list
  ioopm_linked_list_contains (&contains, list, int_to_elem (10));
  CU_ASSERT_FALSE (contains);

  ioopm_linked_list_destroy (&list);
}

// prepends 10 elements to linked list
void
prepend_contains_test (void)
{
  bool contains = false;
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);

  // Verify that contains function returns false for empty list
  ioopm_linked_list_contains (&contains, list, int_to_elem (1));
  CU_ASSERT_FALSE (contains);

  // Prepend 10 entries to list
  for (int i = 0; i < 10; i++)
    {
      ioopm_linked_list_prepend (list, int_to_elem (i));
    }

  // Verify that the elements indeed were prepended
  for (int i = 0; i < 10; i++)
    {
      ioopm_linked_list_contains (&contains, list, int_to_elem (i));
      CU_ASSERT_TRUE (contains);
    }

  // Verify that contains function returns false for an element not in a
  // populated list
  ioopm_linked_list_contains (&contains, list, int_to_elem (10));
  CU_ASSERT_FALSE (contains);

  ioopm_linked_list_destroy (&list);
}

// Insert tests
void
insertion_test (void)
{
  bool contains = false;
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);
  elem_t result;
  // Insert 3 entries to end
  for (int i = 0; i < 3; i++)
    {
      ioopm_linked_list_insert (list, i, int_to_elem (i));
      ioopm_linked_list_get (&result, list, i);
      CU_ASSERT_TRUE (integer_eq (result, int_to_elem (i)));
    }

  // Insert 3 entries to beginning
  for (int i = 3; i < 6; i++)
    {
      ioopm_linked_list_insert (list, 0, int_to_elem (i));
      ioopm_linked_list_get (&result, list, 0);
      CU_ASSERT_TRUE (integer_eq (result, int_to_elem (i)));
    }

  // Insert 3 entries to middle
  for (int i = 3; i < 6; i++)
    {
      ioopm_linked_list_insert (list, i, int_to_elem (i + 3));
      ioopm_linked_list_get (&result, list, i);
      CU_ASSERT_TRUE (integer_eq (result, int_to_elem (i + 3)));
    }

  // Verify that all inserts worked
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_contains (&contains, list, int_to_elem (i));
      CU_ASSERT_TRUE (contains);
    }

  ioopm_linked_list_destroy (&list);
}

// remove tests
void
remove_test (void)
{
  // Create linked list with 9 nodes (8, 7, 6, 5, 4, 3, 2, 1, 0)
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);

  elem_t result;
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_insert (list, i, int_to_elem (i));
      ioopm_linked_list_get (&result, list, i);
      CU_ASSERT_TRUE (integer_eq (result, int_to_elem (i)));
    }

  // Remove from end and verify that it worked (elements 8, 7, 6)
  elem_t removed_value;
  for (int i = 8; i >= 6; i--)
    {
      ioopm_linked_list_get (&result, list, i);
      CU_ASSERT_TRUE (integer_eq (result, int_to_elem (i)));

      ioopm_linked_list_remove (&removed_value, list, i);

      ioopm_linked_list_get (&result, list, i - 1);
      CU_ASSERT_TRUE (integer_eq (result, int_to_elem (i - 1)));
    }
  bool contains = false;
  ioopm_linked_list_contains (&contains, list, int_to_elem (8));
  CU_ASSERT_FALSE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (7));
  CU_ASSERT_FALSE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (6));
  CU_ASSERT_FALSE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (5));
  CU_ASSERT_TRUE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (4));
  CU_ASSERT_TRUE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (3));
  CU_ASSERT_TRUE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (2));
  CU_ASSERT_TRUE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (1));
  CU_ASSERT_TRUE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (0));
  CU_ASSERT_TRUE (contains);

  // Remove from middle and verify that it worked (elements 4, 3, 2)
  for (int i = 4; i >= 2; i--)
    {
      ioopm_linked_list_remove (&removed_value, list, i);
    }
  ioopm_linked_list_contains (&contains, list, int_to_elem (5));
  CU_ASSERT_TRUE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (4));
  CU_ASSERT_FALSE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (3));
  CU_ASSERT_FALSE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (2));
  CU_ASSERT_FALSE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (1));
  CU_ASSERT_TRUE (contains);
  ioopm_linked_list_contains (&contains, list, int_to_elem (0));
  CU_ASSERT_TRUE (contains);

  // Remove from beginning and verify that it worked (elements 8, 7, 6)
  for (int i = 2; i >= 0; i--)
    {
      ioopm_linked_list_remove (&removed_value, list, 0);
    }

  // Double check all elements have been removed
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_contains (&contains, list, int_to_elem (i));
      CU_ASSERT_FALSE (contains);
    }

  ioopm_linked_list_destroy (&list);
}

// get tests
void
get_test (void)
{
  // Create linked list with 9 nodes
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_append (list, int_to_elem (i));
    }

  // Get elements using index and verify that correct element was retrieved
  elem_t get_element;
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_get (&get_element, list, i);
      CU_ASSERT_EQUAL (elem_to_int (get_element), i);
    }

  ioopm_linked_list_destroy (&list);
}

// size tests
void
size_test (void)
{
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);

  // Verify that size is 0 for empty list
  size_t size = 0;
  ioopm_linked_list_size (&size, list);
  CU_ASSERT_EQUAL (size, 0);

  // Check if size is correct after appending
  for (int i = 1; i <= 3; i++)
    {
      ioopm_linked_list_append (list, int_to_elem (i));
      ioopm_linked_list_size (&size, list);
      CU_ASSERT_EQUAL (size, i);
    }

  // Check if size is correct after prepending
  for (int i = 4; i <= 6; i++)
    {
      ioopm_linked_list_prepend (list, int_to_elem (i));
      ioopm_linked_list_size (&size, list);
      CU_ASSERT_EQUAL (size, i);
    }

  // Check if size is correct after inserting
  for (int i = 7; i <= 9; i++)
    {
      ioopm_linked_list_insert (list, 0, int_to_elem (i));
      ioopm_linked_list_size (&size, list);
      CU_ASSERT_EQUAL (size, i);
    }

  // Check if size is correct after removing
  elem_t removed_value;
  for (int i = 1; i <= 9; i++)
    {
      ioopm_linked_list_remove (&removed_value, list, 0);
      ioopm_linked_list_size (&size, list);
      CU_ASSERT_EQUAL (size, 9 - i);
    }

  ioopm_linked_list_destroy (&list);
}

// empty tests
void
empty_test (void)
{
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);

  // Verify that linked list is empty
  bool empty = false;
  ioopm_linked_list_is_empty (&empty, list);
  CU_ASSERT_TRUE (empty);

  // Verify that linked list is no longer empty
  for (int i = 0; i < 10; i++)
    {
      ioopm_linked_list_append (list, int_to_elem (i));
      ioopm_linked_list_is_empty (&empty, list);
      CU_ASSERT_FALSE (empty);
    }

  // Remove all elements and check if linked list is empty
  elem_t removed_value;
  for (int i = 0; i < 10; i++)
    {
      ioopm_linked_list_remove (&removed_value, list, 0);
    }
  ioopm_linked_list_is_empty (&empty, list);
  CU_ASSERT_TRUE (empty);

  ioopm_linked_list_destroy (&list);
}

// clear tests
void
clear_test (void)
{
  // Create a populated linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_append (list, int_to_elem (i));
    }

  // Clear linked list and verify that it is empty
  ioopm_linked_list_clear (list);
  bool empty = false;
  ioopm_linked_list_is_empty (&empty, list);
  CU_ASSERT_TRUE (empty);
  bool contains = false;
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_contains (&contains, list, int_to_elem (i));
      CU_ASSERT_FALSE (contains);
    }

  ioopm_linked_list_destroy (&list);
}

static bool
strictly_less (const elem_t element, const void *extra)
{
  return elem_to_int (element) < *((int *)extra);
}

// all tests
void
all_test (void)
{
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);

  // Returns true if linked list is empty
  int limit = 0;
  bool all = false;
  ioopm_linked_list_all (&all, list, strictly_less, &limit);
  CU_ASSERT_TRUE (all);

  // Populate linked list
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_append (list, int_to_elem (i));
    }

  // Check positive and negative return values for populated linked list
  limit = 10;
  ioopm_linked_list_all (&all, list, strictly_less, &limit);
  CU_ASSERT_TRUE (all);
  limit = 5;
  ioopm_linked_list_all (&all, list, strictly_less, &limit);
  CU_ASSERT_FALSE (all);

  ioopm_linked_list_destroy (&list);
}

// any tests
void
any_test (void)
{
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);

  // Returns false if linked list is empty
  int limit = 0;
  bool any = false;
  ioopm_linked_list_any (&any, list, strictly_less, &limit);
  CU_ASSERT_FALSE (any);

  // Populate linked list
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_append (list, int_to_elem (i));
    }

  // Check positive and negative return values for populated linked list
  limit = 5;
  ioopm_linked_list_any (&any, list, strictly_less, &limit);
  CU_ASSERT_TRUE (any);
  limit = 0;
  ioopm_linked_list_any (&any, list, strictly_less, &limit);
  CU_ASSERT_FALSE (any);

  ioopm_linked_list_destroy (&list);
}

void
increase_element (elem_t *element, void *number)
{
  element->integer += *((int *)number);
}

// apply to all tests
void
apply_to_all_test (void)
{
  // Create a populated linked list
  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, integer_eq);
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_append (list, int_to_elem (i));
    }

  // add 10 to each element in linked list
  int number = 10;
  ioopm_linked_list_apply_to_all (list, increase_element, &number);

  // Check that all elements have increased by 10
  bool contains = false;
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_contains (&contains, list, int_to_elem (i + 10));
      CU_ASSERT_TRUE (contains);
    }

  // Verify that the previous element values are no longer in linked list
  for (int i = 0; i < 9; i++)
    {
      ioopm_linked_list_contains (&contains, list, int_to_elem (i));
      CU_ASSERT_FALSE (contains);
    }

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
  CU_pSuite linked_list_test_suite
      = CU_add_suite ("Linked List Testing Suite", init_suite, clean_suite);
  if (linked_list_test_suite == NULL)
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
  if ((CU_add_test (linked_list_test_suite, "Create and destroy a linked list",
                    create_destroy_empty_list_test)
       == NULL)
      || (CU_add_test (linked_list_test_suite,
                       "Append entries to a linked list", append_contains_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite,
                       "Prepend entries to a linked list",
                       prepend_contains_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite,
                       "Inserts entries to a linked list", insertion_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite,
                       "Removes entries from a linked list", remove_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite,
                       "Gets entries from a linked list", get_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite, "Checks size of a linked list",
                       size_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite,
                       "Check if a linked list is empty", empty_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite, "Clears a linked list",
                       clear_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite, "Tests the all function ",
                       all_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite, "Tests the any function ",
                       any_test)
          == NULL)
      || (CU_add_test (linked_list_test_suite, "Tests the any function ",
                       apply_to_all_test)
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
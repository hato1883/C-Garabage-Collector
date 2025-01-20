#include <CUnit/Basic.h>

#include "../../lib/common.h"
#include "../../lib/iterator.h"
#include "../../lib/linked_list.h"
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

/// @brief Test if ioopm_linked_list_create reports invalid list pointer
void
create_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  status = ioopm_linked_list_create (NULL, NULL);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_create reports OOM Error during allocation
/// of ioopm_hash_table_t pointer
void
create_list_allocation_failure_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;

  // Force malloc to return NULL on next malloc usage
  oom_next_alloc (false);
  status = ioopm_linked_list_create (&valid_list, NULL);
  set_normal_alloc ();

  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
}

/// @brief Test if ioopm_linked_list_destroy reports invalid list pointer
void
destroy_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *invalid_list = NULL;
  status = ioopm_linked_list_destroy (&invalid_list);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_append reports invalid list pointer
void
append_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  status = ioopm_linked_list_append (NULL, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_append reports OOM failure
void
append_list_oom_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Test on empty list
  oom_next_alloc (false);
  status = ioopm_linked_list_append (valid_list, int_to_elem (0));
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  status = ioopm_linked_list_append (valid_list, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Test on non empty list
  oom_next_alloc (false);
  status = ioopm_linked_list_append (valid_list, int_to_elem (0));
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_prepend reports invalid list pointer
void
prepend_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  status = ioopm_linked_list_prepend (NULL, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_prepend reports OOM Failure
void
prepend_list_oom_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  oom_next_alloc (false);
  status = ioopm_linked_list_prepend (valid_list, int_to_elem (0));
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_insert reports invalid list pointer
void
insert_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  status = ioopm_linked_list_insert (NULL, 0, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_insert reports index out of bounds
void
insert_list_invalid_index_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // size is 0 so inserting can only be at 0 (1 is out of bounds or also known
  // as OOB)
  status = ioopm_linked_list_insert (valid_list, 1, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  // index is unsigned so -1 becomes a very large number which will be > 0
  // (OOB)
  status = ioopm_linked_list_insert (valid_list, -1, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_insert reports OOM Failure
void
insert_list_oom_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Insert first on empty list
  oom_next_alloc (false);
  status = ioopm_linked_list_insert (valid_list, 0, int_to_elem (0));
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  status = ioopm_linked_list_append (valid_list, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Insert first on non empty list
  oom_next_alloc (false);
  status = ioopm_linked_list_insert (valid_list, 0, int_to_elem (0));
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  // Insert not first on non empty list
  oom_next_alloc (false);
  status = ioopm_linked_list_insert (valid_list, 1, int_to_elem (0));
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_remove reports invalid list pointer
void
remove_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  elem_t valid_return;
  status = ioopm_linked_list_remove (&valid_return, NULL, 0);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_remove reports invalid list pointer
void
remove_list_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_linked_list_remove (NULL, valid_list, 0);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_remove reports invalid list pointer
void
remove_list_invalid_list_size_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  elem_t valid_return;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_linked_list_remove (&valid_return, valid_list, 0);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  status = ioopm_linked_list_remove (&valid_return, valid_list, 1);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  status = ioopm_linked_list_remove (&valid_return, valid_list, -1);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_remove reports index out of bounds
void
remove_list_invalid_index_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  elem_t valid_return;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Make sure list is not empty
  status = ioopm_linked_list_insert (valid_list, 0, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  // size is 1 so we can only remove element at index 0 (index 1 would be out
  // of bounds)
  status = ioopm_linked_list_remove (&valid_return, valid_list, 1);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  // index is unsigned so -1 becomes a very large number which will be > 0
  // (OOB)
  status = ioopm_linked_list_remove (&valid_return, valid_list, -1);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_get reports invalid list pointer
void
get_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  elem_t valid_return;
  status = ioopm_linked_list_get (&valid_return, NULL, 0);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_get reports invalid list pointer
void
get_list_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_linked_list_get (NULL, valid_list, 0);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_get reports invalid list pointer
void
get_list_invalid_list_size_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  elem_t valid_return;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_linked_list_get (&valid_return, valid_list, 0);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  status = ioopm_linked_list_get (&valid_return, valid_list, 1);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  status = ioopm_linked_list_get (&valid_return, valid_list, -1);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_get reports index out of bounds
void
get_list_invalid_index_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  elem_t valid_return;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Make sure list is not empty
  status = ioopm_linked_list_insert (valid_list, 0, int_to_elem (0));
  CU_ASSERT_EQUAL (status, SUCCESS);

  // size is 1 so we can only get element at index 0 (index 1 would be out of
  // bounds)
  status = ioopm_linked_list_get (&valid_return, valid_list, 1);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  // index is unsigned so -1 becomes a very large number which will be > 0
  // (OOB)
  status = ioopm_linked_list_get (&valid_return, valid_list, -1);
  CU_ASSERT_EQUAL (status, INDEX_OUT_OF_BOUNDS);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_contains reports invalid list pointer
void
contains_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  bool valid_return = false;
  status = ioopm_linked_list_contains (&valid_return, NULL, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_contains reports invalid list pointer
void
contains_list_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_linked_list_contains (NULL, valid_list, int_to_elem (0));
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_size reports invalid list pointer
void
size_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  size_t valid_return = 0;
  status = ioopm_linked_list_size (&valid_return, NULL);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_size reports invalid list pointer
void
size_list_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_linked_list_size (NULL, valid_list);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_is_empty reports invalid list pointer
void
is_empty_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  bool valid_return = false;
  status = ioopm_linked_list_is_empty (&valid_return, NULL);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_is_empty reports invalid list pointer
void
is_empty_list_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  status = ioopm_linked_list_is_empty (NULL, valid_list);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_clear reports invalid list pointer
void
clear_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  status = ioopm_linked_list_clear (NULL);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_all reports invalid list pointer
void
all_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  bool valid_return = false;
  int dummy_arg = 0;
  status = ioopm_linked_list_all (&valid_return, NULL, NULL, &dummy_arg);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_all reports invalid list pointer
void
all_list_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  int dummy_arg = 0;
  status = ioopm_linked_list_all (NULL, valid_list, NULL, &dummy_arg);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_any reports invalid list pointer
void
any_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  bool valid_return = false;
  int dummy_arg = 0;
  status = ioopm_linked_list_any (&valid_return, NULL, NULL, &dummy_arg);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

/// @brief Test if ioopm_linked_list_any reports invalid list pointer
void
any_list_invalid_return_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_list_t *valid_list = NULL;
  status = ioopm_linked_list_create (&valid_list, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  int dummy_arg = 0;
  status = ioopm_linked_list_any (NULL, valid_list, NULL, &dummy_arg);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_linked_list_destroy (&valid_list);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_linked_list_apply_to_all reports invalid list pointer
void
apply_to_all_list_invalid_ptr_test (void)
{
  int status = SUCCESS;
  int dummy_arg = 0;
  status = ioopm_linked_list_apply_to_all (NULL, NULL, &dummy_arg);
  CU_ASSERT_EQUAL (status, INVALID_LIST_POINTER);
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite linked_list_error_test_suite = CU_add_suite (
      "Linked List Error handling Test Suite", init_suite, clean_suite);
  if (linked_list_error_test_suite == NULL)
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
  if ((CU_add_test (linked_list_error_test_suite,
                    "Check if invalid list pointer is handled by create",
                    create_list_invalid_ptr_test)
       == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Create a linked list during OOM event when creating "
                       "list pointer",
                       create_list_allocation_failure_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by destroy",
                       destroy_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by append",
                       append_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (
              linked_list_error_test_suite,
              "Create a node_t during OOM event when appending to list",
              append_list_oom_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by prepend",
                       prepend_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (
              linked_list_error_test_suite,
              "Create a node_t during OOM event when prepending to list",
              prepend_list_oom_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by insert",
                       insert_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if a index out of bounds is caught by insert",
                       insert_list_invalid_index_test)
          == NULL)
      || (CU_add_test (
              linked_list_error_test_suite,
              "Create a node_t during OOM event when inserting to list",
              insert_list_oom_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by remove",
                       remove_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid return pointer is handled by remove",
                       remove_list_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if a empty list get caught by remove",
                       remove_list_invalid_list_size_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if a index out of bounds is caught by remove",
                       remove_list_invalid_index_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by get",
                       get_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid return pointer is handled by get",
                       get_list_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if a empty list get caught by get",
                       get_list_invalid_list_size_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if a index out of bounds is caught by get",
                       get_list_invalid_index_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by contains",
                       contains_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (
              linked_list_error_test_suite,
              "Check if invalid return pointer is handled by contains",
              contains_list_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by size",
                       size_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid return pointer is handled by size",
                       size_list_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by is empty",
                       is_empty_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (
              linked_list_error_test_suite,
              "Check if invalid return pointer is handled by is empty",
              is_empty_list_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by clear",
                       clear_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by all",
                       all_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid return pointer is handled by all",
                       all_list_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid list pointer is handled by any",
                       any_list_invalid_ptr_test)
          == NULL)
      || (CU_add_test (linked_list_error_test_suite,
                       "Check if invalid return pointer is handled by any",
                       any_list_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              linked_list_error_test_suite,
              "Check if invalid list pointer is handled by apply to all",
              apply_to_all_list_invalid_ptr_test)
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

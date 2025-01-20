#include <CUnit/Basic.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../lib/hash_table.h"
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

/// @brief Test if ioopm_create_hash_table reports OOM Error during allocation
/// of ioopm_hash_table_t pointer
void
create_list_allocation_failure_test (void)
{
  int status = SUCCESS;
  ioopm_hash_table_t *valid_ht = NULL;

  // Simulate ioopm_create_hash_table to fail due to OOM during malloc call
  // when creating pointer for ioopm_hash_table_t
  oom_next_alloc (false);
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();
}

/// @brief Test if ioopm_create_hash_table reports OOM Error during allocation
/// of pointer array
void
create_table_bucket_allocation_failure_test (void)
{
  int status = SUCCESS;
  ioopm_hash_table_t *valid_ht = NULL;

  // Simulate ioopm_create_hash_table to fail due to OOM during calloc call
  // when creating pointer array
  oom_nth_alloc_call (1, false);
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();
}

///@brief Test if ioopm_create_hash_table reports invalid valid_ht pointer
void
create_hash_table_invalid_ptr_test (void)
{
  int status = SUCCESS;

  // Give invalid pointer
  status = ioopm_create_hash_table (NULL, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

///@brief Test if ioopm_create_hash_table reports invalid valid_ht pointer
void
create_hash_table_invalid_bucket_size_test (void)
{
  int status = SUCCESS;
  ioopm_hash_table_t *valid_ht = NULL;

  // Give invalid pointer
  status = ioopm_create_hash_table (&valid_ht, 0, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, INVALID_BUCKET_COUNT);
}

/// @brief Test if ioopm_hash_table_insert reports invalid valid_ht pointer
void
insert_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  // Give invalid pointer
  status = ioopm_hash_table_insert (NULL, int_to_elem (0), ptr_to_elem ("0"));
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_lookup reports invalid hash table pointer
void
lookup_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  elem_t valid_return_variable;
  bool valid_found_variable = false;

  elem_t valid_key = int_to_elem (0);

  // Give invalid hash table pointer
  status = ioopm_hash_table_lookup (&valid_found_variable,
                                    &valid_return_variable, NULL, valid_key);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_lookup reports invalid boolean pointer
void
lookup_invalid_boolean_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_hash_table_t *valid_ht = NULL;
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  elem_t valid_return_variable;

  elem_t valid_key = int_to_elem (0);

  // Give invalid boolean pointer
  status = ioopm_hash_table_lookup (NULL, &valid_return_variable, valid_ht,
                                    valid_key);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_lookup reports invalid return value pointer
void
lookup_invalid_return_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_hash_table_t *valid_ht = NULL;
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  bool valid_found_variable = false;

  elem_t valid_key = int_to_elem (0);

  // Give invalid value return pointer
  status = ioopm_hash_table_lookup (&valid_found_variable, NULL, valid_ht,
                                    valid_key);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_keys reports invalid hash table pointer
void
get_keys_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_list_t *key_arr = NULL;

  // Give invalid hash table pointer
  status = ioopm_hash_table_keys (&key_arr, NULL);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_keys reports OOM Error
void
get_keys_omm_test (void)
{
  ioopm_hash_table_t *valid_ht = NULL;
  int status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  ioopm_list_t *key_list = NULL;

  // Force allocation to fail during key_list allocation
  oom_next_alloc (false);
  status = ioopm_hash_table_keys (&key_list, valid_ht);
  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);
  set_normal_alloc ();

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_keys reports invalid return pointer
void
get_keys_invalid_return_ptr_test (void)
{
  ioopm_hash_table_t *valid_ht = NULL;
  int status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Give invalid hash table pointer
  status = ioopm_hash_table_keys (NULL, valid_ht);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_values reports invalid hash table pointer
void
get_values_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_list_t *value_list = NULL;

  // Give invalid hash table pointer
  status = ioopm_hash_table_values (&value_list, NULL);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_values reports OOM Error
void
get_values_omm_test (void)
{
  ioopm_hash_table_t *valid_ht = NULL;
  int status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  ioopm_list_t *value_list = NULL;

  // Force allocation to fail during value_list allocation
  oom_next_alloc (false);
  status = ioopm_hash_table_values (&value_list, valid_ht);
  set_normal_alloc ();

  CU_ASSERT_EQUAL (status, MEMORY_ALLOCATION_FAILURE);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_values reports invalid return pointer
void
get_values_invalid_return_ptr_test (void)
{
  ioopm_hash_table_t *valid_ht = NULL;
  int status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Give invalid hash table pointer
  status = ioopm_hash_table_values (NULL, valid_ht);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_remove reports invalid hash table pointer
void
remove_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  elem_t valid_key = int_to_elem (0);

  // Give invalid hash table pointer
  status = ioopm_hash_table_remove (NULL, valid_key);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_destroy reports invalid hash table pointer
void
destroy_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;
  ioopm_hash_table_t *invalid_table = NULL;
  // Give invalid hash table pointer
  status = ioopm_hash_table_destroy (&invalid_table);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_has_key reports invalid hash table pointer
void
has_key_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  bool valid_found_variable = false;

  elem_t valid_key = int_to_elem (0);

  // Give invalid hash table pointer
  status = ioopm_hash_table_has_key (&valid_found_variable, NULL, valid_key);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_has_key reports invalid boolean pointer
void
has_key_invalid_boolean_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_hash_table_t *valid_ht = NULL;
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  elem_t valid_key = int_to_elem (0);

  // Give invalid boolean pointer
  status = ioopm_hash_table_has_key (NULL, valid_ht, valid_key);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_has_value reports invalid hash table
/// pointer
void
has_value_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  bool valid_found_variable = false;

  elem_t valid_value = ptr_to_elem ("0");

  // Give invalid hash table pointer
  status
      = ioopm_hash_table_has_value (&valid_found_variable, NULL, valid_value);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_has_value reports invalid boolean pointer
void
has_value_invalid_boolean_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_hash_table_t *valid_ht = NULL;
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  elem_t valid_value = ptr_to_elem ("0");

  // Give invalid boolean pointer
  status = ioopm_hash_table_has_value (NULL, valid_ht, valid_value);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_size reports invalid hash table pointer
void
size_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  size_t valid_return = 0;

  // Give invalid hash table pointer
  status = ioopm_hash_table_size (&valid_return, NULL);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_size reports invalid int pointer
void
size_invalid_return_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_hash_table_t *valid_ht = NULL;
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Give invalid boolean pointer
  status = ioopm_hash_table_size (NULL, valid_ht);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_is_empty reports invalid hash table pointer
void
is_empty_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  bool valid_return = false;

  // Give invalid hash table pointer
  status = ioopm_hash_table_is_empty (&valid_return, NULL);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_is_empty reports invalid boolean pointer
void
is_empty_invalid_return_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_hash_table_t *valid_ht = NULL;
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Give invalid boolean pointer
  status = ioopm_hash_table_is_empty (NULL, valid_ht);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_clear reports invalid hash table pointer
void
clear_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  // Give invalid hash table pointer
  status = ioopm_hash_table_clear (NULL);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_all reports invalid hash table pointer
void
all_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  bool valid_return = false;

  // Give invalid hash table pointer
  status = ioopm_hash_table_all (&valid_return, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_all reports invalid boolean pointer
void
all_invalid_return_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_hash_table_t *valid_ht = NULL;
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Give invalid boolean pointer
  status = ioopm_hash_table_all (NULL, valid_ht, NULL, NULL);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_any reports invalid hash table pointer
void
any_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  bool valid_return = false;

  // Give invalid hash table pointer
  status = ioopm_hash_table_any (&valid_return, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

/// @brief Test if ioopm_hash_table_any reports invalid boolean pointer
void
any_invalid_return_ptr_test (void)
{
  int status = SUCCESS;

  ioopm_hash_table_t *valid_ht = NULL;
  status = ioopm_create_hash_table (&valid_ht, 1, NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, SUCCESS);

  // Give invalid boolean pointer
  status = ioopm_hash_table_any (NULL, valid_ht, NULL, NULL);
  CU_ASSERT_EQUAL (status, INVALID_RETURN_POINTER);

  status = ioopm_hash_table_destroy (&valid_ht);
  CU_ASSERT_EQUAL (status, SUCCESS);
}

/// @brief Test if ioopm_hash_table_apply_to_all reports invalid hash table
/// pointer
void
apply_invalid_hash_table_ptr_test (void)
{
  int status = SUCCESS;

  // Give invalid hash table pointer
  status = ioopm_hash_table_apply_to_all (NULL, NULL, NULL);
  CU_ASSERT_EQUAL (status, INVALID_HASH_TABLE_POINTER);
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite hash_table_error_test_suite = CU_add_suite (
      "Hash Table Error handling Test Suite", init_suite, clean_suite);
  if (hash_table_error_test_suite == NULL)
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
  if ((CU_add_test (hash_table_error_test_suite,
                    "Create a hash table during OOM event when creating hash "
                    "table pointer",
                    create_list_allocation_failure_test)
       == NULL)
      || (CU_add_test (hash_table_error_test_suite,
                       "Create a hash table during OOM event when creating "
                       "pointer array",
                       create_table_bucket_allocation_failure_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by create",
              create_hash_table_invalid_ptr_test)
          == NULL)
      || (CU_add_test (hash_table_error_test_suite,
                       "Check if invalid bucket count is handled by create",
                       create_hash_table_invalid_bucket_size_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by insert",
              insert_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by lookup",
              lookup_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (hash_table_error_test_suite,
                       "Check if invalid boolean pointer is handled by lookup",
                       lookup_invalid_boolean_ptr_test)
          == NULL)
      || (CU_add_test (hash_table_error_test_suite,
                       "Check if invalid return pointer is handled by lookup",
                       lookup_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by get_keys",
              get_keys_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (hash_table_error_test_suite,
                       "Get hash table keys during OOM event when when "
                       "allocating key array",
                       get_keys_omm_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid return pointer is handled by get_keys",
              get_keys_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by get_values",
              get_values_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (hash_table_error_test_suite,
                       "Get hash table values during OOM event when when "
                       "allocating value array",
                       get_values_omm_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid return pointer is handled by get_values",
              get_values_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by remove",
              remove_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by destroy",
              destroy_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by has_key",
              has_key_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid boolean pointer is handled by has_key",
              has_key_invalid_boolean_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by has_value",
              has_value_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid boolean pointer is handled by has_value",
              has_value_invalid_boolean_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by size",
              size_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (hash_table_error_test_suite,
                       "Check if invalid return pointer is handled by size",
                       size_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by is_empty",
              is_empty_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid return pointer is handled by is_empty",
              is_empty_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by clear",
              clear_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by all_pred",
              all_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid return pointer is handled by all_pred",
              all_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by any_pred",
              any_invalid_hash_table_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid return pointer is handled by any_pred",
              any_invalid_return_ptr_test)
          == NULL)
      || (CU_add_test (
              hash_table_error_test_suite,
              "Check if invalid hash table pointer is handled by apply_func",
              apply_invalid_hash_table_ptr_test)
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

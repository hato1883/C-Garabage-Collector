#include <CUnit/Basic.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/hash_table.h"
#include "../../lib/iterator.h"
#include "../../lib/linked_list.h"
#include "../../src/gc.h"

int
init_suite (void)
{
  h_init (1024 * 128, true, 0.75f);
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

bool
string_eq (const elem_t a, const elem_t b)
{
  return strcmp (elem_to_string (a), elem_to_string (b)) == 0;
}

size_t
integer_hash (const elem_t key)
{
  return elem_to_size (key);
}

/*
 * @brief Test if we can create a hash table with 1 bucket (Edge case with only
 * )
 */
void
create_hash_table_single_bucket_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  int status
      = ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_PTR_NOT_NULL_FATAL (ht);

  size_t size = 0;
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 0);
  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Test if we can create a hash table with multiple (10) buckets
 */
void
create_hash_table_multiple_bucket_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  int status
      = ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);
  CU_ASSERT_EQUAL (status, SUCCESS);
  CU_ASSERT_PTR_NOT_NULL_FATAL (ht);

  size_t size = 0;
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 0);
  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Test if we can insert a single item into a hashtable with one bucket
 * (Testing edge case of inserting a item into a empty bucket)
 */
void
hash_table_single_insert_one_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  elem_t insertion = ptr_to_elem ("abc");

  // Test if we can insert a single item into our hashtable
  ioopm_hash_table_insert (ht, int_to_elem (0), insertion);

  elem_t result;
  bool found = false;
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (0));
  CU_ASSERT_TRUE (string_eq (result, insertion));

  ioopm_hash_table_destroy (&ht);
}

/*f
 * @brief Test if we can insert a multiple items into a hashtable with one
 * bucket (Testing case of inserting multiple items into the same bucket)
 */
void
hash_table_single_insert_multiple_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);
  elem_t result;

  elem_t insertion_values[4];
  insertion_values[0] = ptr_to_elem ("1");
  insertion_values[1] = ptr_to_elem ("2");
  insertion_values[2] = ptr_to_elem ("3");
  insertion_values[3] = ptr_to_elem ("4");

  // Test if we can insert a 4 items into our hash table
  bool found = false;
  for (int i = 0; i < 4; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i), insertion_values[i]);
      ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (i));
      CU_ASSERT_TRUE (string_eq (result, insertion_values[i]));
    }

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Test if we can insert a duplicate of a key in a hashtable with one
 * bucket (Testing edge case: updating a bucket with a single key value pair)
 */
void
hash_table_single_insert_duplicate_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  elem_t insertion = ptr_to_elem ("abc");
  elem_t insertion_new = ptr_to_elem ("def");

  // Test if duplicate key_list results in a updated value.
  ioopm_hash_table_insert (ht, int_to_elem (0), insertion);
  ioopm_hash_table_insert (ht, int_to_elem (0), insertion_new);

  elem_t result;
  bool found = false;
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (0));
  CU_ASSERT_TRUE (string_eq (result, insertion_new));

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Test if we can insert a single item into a hashtable with multiple
 * buckets (Testing edge case of inserting a item into a empty bucket)
 */
void
hash_table_multiple_insert_one_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);

  elem_t insertion = ptr_to_elem ("abc");
  elem_t result;

  // Test if we can insert into each bucket
  bool found = false;
  for (int bucket = 0; bucket < 10; bucket++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (0 + bucket), insertion);

      ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (0 + bucket));
      CU_ASSERT_TRUE (string_eq (result, insertion));
    }

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Test if we can insert a duplicate of a key in a hashtable with
 * multiple buckets (Testing edge case: updating a bucket with a single key
 * value pair)
 */
void
hash_table_multiple_insert_duplicate_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);

  elem_t insertion = ptr_to_elem ("abc");
  elem_t insertion_new = ptr_to_elem ("def");
  elem_t result;

  // Test if we can insert into each bucket
  bool found = false;
  for (int bucket = 0; bucket < 10; bucket++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (0 + bucket), insertion);
      ioopm_hash_table_insert (ht, int_to_elem (0 + bucket), insertion_new);
      ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (0 + bucket));
      CU_ASSERT_TRUE (string_eq (result, insertion_new));
    }

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Test if we can insert a multiple items into a hashtable with multiple
 * buckets (Testing case of inserting multiple items into the same bucket)
 */
void
hash_table_multiple_insert_multiple_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  elem_t insertion_values[20];
  insertion_values[0] = ptr_to_elem ("Bucket 0: Insertion: 1");
  insertion_values[1] = ptr_to_elem ("Bucket 0: Insertion: 2");
  insertion_values[2] = ptr_to_elem ("Bucket 0: Insertion: 3");
  insertion_values[3] = ptr_to_elem ("Bucket 0: Insertion: 4");
  insertion_values[4] = ptr_to_elem ("Bucket 1: Insertion: 1");
  insertion_values[5] = ptr_to_elem ("Bucket 1: Insertion: 2");
  insertion_values[6] = ptr_to_elem ("Bucket 1: Insertion: 3");
  insertion_values[7] = ptr_to_elem ("Bucket 1: Insertion: 4");
  insertion_values[8] = ptr_to_elem ("Bucket 2: Insertion: 1");
  insertion_values[9] = ptr_to_elem ("Bucket 2: Insertion: 2");
  insertion_values[10] = ptr_to_elem ("Bucket 2: Insertion: 3");
  insertion_values[11] = ptr_to_elem ("Bucket 2: Insertion: 4");
  insertion_values[12] = ptr_to_elem ("Bucket 3: Insertion: 1");
  insertion_values[13] = ptr_to_elem ("Bucket 3: Insertion: 2");
  insertion_values[14] = ptr_to_elem ("Bucket 3: Insertion: 3");
  insertion_values[15] = ptr_to_elem ("Bucket 3: Insertion: 4");
  insertion_values[16] = ptr_to_elem ("Bucket 4: Insertion: 1");
  insertion_values[17] = ptr_to_elem ("Bucket 4: Insertion: 2");
  insertion_values[18] = ptr_to_elem ("Bucket 4: Insertion: 3");
  insertion_values[19] = ptr_to_elem ("Bucket 4: Insertion: 4");
  elem_t result;
  bool found = false;

  // Test if we can insert into each bucket
  for (int bucket = 0; bucket < 5; bucket++)
    {
      // Test if we can insert a 4 items into our hash table bucket
      for (int i = 0; i < 4; i++)
        {

          ioopm_hash_table_insert (ht, int_to_elem (i * 10 + bucket),
                                   insertion_values[i + bucket * 4]);
          ioopm_hash_table_lookup (&found, &result, ht,
                                   int_to_elem (i * 10 + bucket));
          CU_ASSERT_TRUE (
              string_eq (result, insertion_values[i + bucket * 4]));
        }
    }

  ioopm_hash_table_destroy (&ht);
}

// Assumes ioopm_hash_table_lookup_test() passed
void
hash_table_single_remove_first_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  // Insert items to hash table
  ioopm_hash_table_insert (ht, int_to_elem (0), ptr_to_elem ("abc"));

  // Make sure insertion added items to hash table
  elem_t result;
  size_t size = 0;
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 1);
  bool found = false;
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (0));
  CU_ASSERT_TRUE (found);

  // Remove item again
  ioopm_hash_table_remove (ht, int_to_elem (0));
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 0);

  // Check that we get back false from lookup because item should not exist
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (0));
  CU_ASSERT_FALSE (found);

  ioopm_hash_table_destroy (&ht);
}

// Assumes ioopm_hash_table_lookup_test() passed
void
hash_table_single_remove_last_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  // Insert items to hash table
  ioopm_hash_table_insert (ht, int_to_elem (0), ptr_to_elem ("a"));
  ioopm_hash_table_insert (ht, int_to_elem (1), ptr_to_elem ("b"));
  ioopm_hash_table_insert (ht, int_to_elem (2), ptr_to_elem ("c"));

  // Make sure insertion added items to hash table
  elem_t result;
  size_t size = 0;
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 3);
  bool found = false;
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (2));
  CU_ASSERT_TRUE (found);

  // Remove item again
  ioopm_hash_table_remove (ht, int_to_elem (2));
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 2);

  // Check that we get back false from lookup because item should not exist
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (2));
  CU_ASSERT_FALSE (found);

  ioopm_hash_table_destroy (&ht);
}

// Assumes ioopm_hash_table_lookup_test() passed
void
hash_table_single_remove_empty_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  size_t size = 0;
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 0);

  // Remove some item
  ioopm_hash_table_remove (ht, int_to_elem (0));
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 0);

  // Check that we get back false from lookup because no item should exist
  elem_t result;
  bool found = false;
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (0));
  CU_ASSERT_FALSE (found);

  ioopm_hash_table_destroy (&ht);
}

// Assumes ioopm_hash_table_lookup_test() passed
void
hash_table_multiple_remove_first_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);

  // Insert items to hash table
  ioopm_hash_table_insert (ht, int_to_elem (3), ptr_to_elem ("abc"));

  // Make sure insertion added items to hash table
  elem_t result;
  size_t size = 0;
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 1);
  bool found = false;
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (3));
  CU_ASSERT_TRUE (found);

  // Remove item again
  ioopm_hash_table_remove (ht, int_to_elem (3));
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 0);

  // Check that we get back false from lookup because item should not exist
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (3));
  CU_ASSERT_FALSE (found);

  ioopm_hash_table_destroy (&ht);
}

// Assumes ioopm_hash_table_lookup_test() passed
void
hash_table_multiple_remove_last_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);

  // Insert items to hash table
  ioopm_hash_table_insert (ht, int_to_elem (3), ptr_to_elem ("a"));
  ioopm_hash_table_insert (ht, int_to_elem (13), ptr_to_elem ("b"));
  ioopm_hash_table_insert (ht, int_to_elem (23), ptr_to_elem ("c"));

  // Make sure insertion added items to hash table
  elem_t result;
  size_t size = 0;
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 3);
  bool found = false;
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (23));
  CU_ASSERT_TRUE (found);

  // Remove item again
  ioopm_hash_table_remove (ht, int_to_elem (23));
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 2);

  // Check that we get back false from lookup because item should not exist
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (23));
  CU_ASSERT_FALSE (found);

  ioopm_hash_table_destroy (&ht);
}

// Assumes ioopm_hash_table_lookup_test() passed
void
hash_table_multiple_remove_middle_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);

  // Insert items to hash table
  ioopm_hash_table_insert (ht, int_to_elem (3), ptr_to_elem ("a"));
  ioopm_hash_table_insert (ht, int_to_elem (13), ptr_to_elem ("b"));
  ioopm_hash_table_insert (ht, int_to_elem (23), ptr_to_elem ("c"));

  // Make sure insertion added items to hash table
  elem_t result;
  size_t size = 0;
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 3);
  bool found = false;
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (13));
  CU_ASSERT_TRUE (found);

  // Remove item again
  ioopm_hash_table_remove (ht, int_to_elem (13));
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 2);

  // Check that we get back false from lookup because item should not exist
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (13));
  CU_ASSERT_FALSE (found);

  ioopm_hash_table_destroy (&ht);
}

// Assumes ioopm_hash_table_lookup_test() passed
void
hash_table_multiple_remove_empty_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);

  size_t size = 0;
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 0);

  // Remove some item
  ioopm_hash_table_remove (ht, int_to_elem (0));
  ioopm_hash_table_size (&size, ht);
  CU_ASSERT_TRUE (size == 0);

  // Check that we get back false from lookup because no item should exist
  elem_t result;
  bool found = false;
  ioopm_hash_table_lookup (&found, &result, ht, int_to_elem (0));
  CU_ASSERT_FALSE (found)

  ioopm_hash_table_destroy (&ht);
}

void
hash_table_single_values_empty_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  size_t value_count = 0;
  ioopm_list_t *value_list = NULL;
  ioopm_hash_table_values (&value_list, ht);
  ioopm_linked_list_size (&value_count, value_list);

  CU_ASSERT_TRUE (value_count == 0);

  // free memory
  ioopm_linked_list_destroy (&value_list);
  ioopm_hash_table_destroy (&ht);
}

#define single_value_count 1
void
hash_table_single_values_single_value_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  bool found[single_value_count];
  found[0] = false;

  elem_t expected_values[single_value_count];
  expected_values[0] = ptr_to_elem ("abc");

  elem_t inserted = ptr_to_elem ("abc");
  ioopm_hash_table_insert (ht, int_to_elem (0), inserted);

  ioopm_list_t *value_list = NULL;
  ioopm_hash_table_values (&value_list, ht);

  size_t value_count = 0;
  ioopm_linked_list_size (&value_count, value_list);

  ioopm_iterator_t *value_iter = NULL;
  ioopm_list_iterator (&value_iter, value_list);

  CU_ASSERT_TRUE (value_count == 1);

  elem_t current;
  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, value_iter) == SUCCESS
         && has_next)
    {
      for (size_t expected_arr = 0; expected_arr < (size_t)single_value_count;
           expected_arr++)
        {
          ioopm_iterator_next (&current, value_iter);
          if (string_eq (current, expected_values[expected_arr]))
            {
              found[expected_arr] = true;
            }
        }
    }

  for (size_t expected_arr = 0; expected_arr < (size_t)single_value_count;
       expected_arr++)
    {
      CU_ASSERT_TRUE (found[expected_arr]);
    }

  // free memory
  ioopm_iterator_destroy (&value_iter);
  ioopm_linked_list_destroy (&value_list);
  ioopm_hash_table_destroy (&ht);
}

#define multiple_value_count 4
void
hash_table_single_values_multiple_value_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  bool found[multiple_value_count];
  found[0] = false;
  found[1] = false;
  found[2] = false;
  found[3] = false;

  elem_t expected_values[multiple_value_count];
  expected_values[0] = ptr_to_elem ("1");
  expected_values[1] = ptr_to_elem ("2");
  expected_values[2] = ptr_to_elem ("3");
  expected_values[3] = ptr_to_elem ("4");

  ioopm_hash_table_insert (ht, int_to_elem (0), ptr_to_elem ("1"));
  ioopm_hash_table_insert (ht, int_to_elem (1), ptr_to_elem ("2"));
  ioopm_hash_table_insert (ht, int_to_elem (2), ptr_to_elem ("3"));
  ioopm_hash_table_insert (ht, int_to_elem (3), ptr_to_elem ("4"));

  ioopm_list_t *value_list = NULL;
  ioopm_hash_table_values (&value_list, ht);

  size_t value_count = 0;
  ioopm_linked_list_size (&value_count, value_list);

  ioopm_iterator_t *value_iter = NULL;
  ioopm_list_iterator (&value_iter, value_list);

  CU_ASSERT_TRUE (value_count == 4);

  elem_t current;
  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, value_iter) == SUCCESS
         && has_next)
    {
      ioopm_iterator_next (&current, value_iter);
      for (size_t expected_arr = 0;
           expected_arr < (size_t)multiple_value_count; expected_arr++)
        {
          if (string_eq (current, expected_values[expected_arr]))
            {
              found[expected_arr] = true;
            }
        }
    }

  for (size_t expected_arr = 0; expected_arr < (size_t)multiple_value_count;
       expected_arr++)
    {
      CU_ASSERT_TRUE (found[expected_arr]);
    }

  // free memory
  ioopm_iterator_destroy (&value_iter);
  ioopm_linked_list_destroy (&value_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_multiple_values_empty_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  size_t value_count = 0;
  ioopm_list_t *value_list = NULL;
  ioopm_hash_table_values (&value_list, ht);
  ioopm_linked_list_size (&value_count, value_list);

  CU_ASSERT_TRUE (value_count == 0);

  // free memory
  ioopm_linked_list_destroy (&value_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_multiple_values_single_value_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);

  bool found[single_value_count];
  found[0] = false;

  elem_t expected_values[single_value_count];
  expected_values[0] = ptr_to_elem ("abc");

  elem_t inserted = ptr_to_elem ("abc");
  ioopm_hash_table_insert (ht, int_to_elem (5), inserted);

  ioopm_list_t *value_list = NULL;
  ioopm_hash_table_values (&value_list, ht);

  size_t value_count = 0;
  ioopm_linked_list_size (&value_count, value_list);

  ioopm_iterator_t *value_iter = NULL;
  ioopm_list_iterator (&value_iter, value_list);

  CU_ASSERT_TRUE (value_count == 1);

  elem_t current;
  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, value_iter) == SUCCESS
         && has_next)
    {
      ioopm_iterator_next (&current, value_iter);
      for (size_t expected_arr = 0; expected_arr < (size_t)single_value_count;
           expected_arr++)
        {
          if (string_eq (current, expected_values[expected_arr]))
            {
              found[expected_arr] = true;
            }
        }
    }

  for (size_t expected_arr = 0; expected_arr < (size_t)single_value_count;
       expected_arr++)
    {
      CU_ASSERT_TRUE (found[expected_arr]);
    }

  // free memory
  ioopm_iterator_destroy (&value_iter);
  ioopm_linked_list_destroy (&value_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_multiple_values_multiple_values_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  bool found[multiple_value_count];
  found[0] = false;
  found[1] = false;
  found[2] = false;
  found[3] = false;

  elem_t expected_values[multiple_value_count];
  expected_values[0] = ptr_to_elem ("1");
  expected_values[1] = ptr_to_elem ("2");
  expected_values[2] = ptr_to_elem ("3");
  expected_values[3] = ptr_to_elem ("4");

  ioopm_hash_table_insert (ht, int_to_elem (0), ptr_to_elem ("1"));
  ioopm_hash_table_insert (ht, int_to_elem (10), ptr_to_elem ("2"));
  ioopm_hash_table_insert (ht, int_to_elem (1), ptr_to_elem ("3"));
  ioopm_hash_table_insert (ht, int_to_elem (11), ptr_to_elem ("4"));

  ioopm_list_t *value_list = NULL;
  ioopm_hash_table_values (&value_list, ht);

  size_t value_count = 0;
  ioopm_linked_list_size (&value_count, value_list);

  ioopm_iterator_t *value_iter = NULL;
  ioopm_list_iterator (&value_iter, value_list);

  CU_ASSERT_TRUE (value_count == 4);

  elem_t current;
  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, value_iter) == SUCCESS
         && has_next)
    {
      ioopm_iterator_next (&current, value_iter);
      for (size_t expected_arr = 0;
           expected_arr < (size_t)multiple_value_count; expected_arr++)
        {
          if (string_eq (current, expected_values[expected_arr]))
            {
              found[expected_arr] = true;
            }
        }
    }

  for (size_t expected_arr = 0; expected_arr < (size_t)multiple_value_count;
       expected_arr++)
    {
      CU_ASSERT_TRUE (found[expected_arr]);
    }

  // free memory
  ioopm_iterator_destroy (&value_iter);
  ioopm_linked_list_destroy (&value_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_single_keys_empty_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  size_t key_count = 0;
  ioopm_list_t *key_list = NULL;
  ioopm_hash_table_keys (&key_list, ht);

  ioopm_linked_list_size (&key_count, key_list);

  CU_ASSERT_TRUE (key_count == 0);

  // free memory
  ioopm_linked_list_destroy (&key_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_single_keys_single_key_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  bool found[single_value_count];
  found[0] = false;

  elem_t expected_keys[single_value_count];
  expected_keys[0] = int_to_elem (0);

  elem_t inserted = ptr_to_elem ("abc");
  elem_t pos = int_to_elem (0);
  ioopm_hash_table_insert (ht, pos, inserted);

  ioopm_list_t *key_list = NULL;
  ioopm_hash_table_keys (&key_list, ht);

  ioopm_iterator_t *key_iter = NULL;
  ioopm_list_iterator (&key_iter, key_list);

  size_t key_count = 0;
  ioopm_linked_list_size (&key_count, key_list);

  CU_ASSERT_TRUE (key_count == 1);
  elem_t current;
  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, key_iter) == SUCCESS && has_next)
    {
      ioopm_iterator_next (&current, key_iter);
      for (size_t expected_arr = 0; expected_arr < (size_t)single_value_count;
           expected_arr++)
        {
          if (integer_eq (current, expected_keys[expected_arr]))
            {
              found[expected_arr] = true;
            }
        }
    }

  for (size_t expected_arr = 0; expected_arr < (size_t)single_value_count;
       expected_arr++)
    {
      CU_ASSERT_TRUE (found[expected_arr]);
    }

  // free memory
  ioopm_iterator_destroy (&key_iter);
  ioopm_linked_list_destroy (&key_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_single_keys_multiple_keys_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  bool found[multiple_value_count];
  found[0] = false;
  found[1] = false;
  found[2] = false;
  found[3] = false;

  elem_t expected_keys[multiple_value_count];
  expected_keys[0] = int_to_elem (0);
  expected_keys[1] = int_to_elem (1);
  expected_keys[2] = int_to_elem (2);
  expected_keys[3] = int_to_elem (3);

  ioopm_hash_table_insert (ht, int_to_elem (0), ptr_to_elem ("1"));
  ioopm_hash_table_insert (ht, int_to_elem (1), ptr_to_elem ("2"));
  ioopm_hash_table_insert (ht, int_to_elem (2), ptr_to_elem ("3"));
  ioopm_hash_table_insert (ht, int_to_elem (3), ptr_to_elem ("4"));

  ioopm_list_t *key_list = NULL;
  ioopm_hash_table_keys (&key_list, ht);

  ioopm_iterator_t *key_iter = NULL;
  ioopm_list_iterator (&key_iter, key_list);

  size_t key_count = 0;
  ioopm_linked_list_size (&key_count, key_list);

  CU_ASSERT_TRUE (key_count == 4);
  elem_t current;
  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, key_iter) == SUCCESS && has_next)
    {
      ioopm_iterator_next (&current, key_iter);
      for (size_t expected_arr = 0;
           expected_arr < (size_t)multiple_value_count; expected_arr++)
        {
          if (integer_eq (current, expected_keys[expected_arr]))
            {
              found[expected_arr] = true;
            }
        }
    }

  for (size_t expected_arr = 0; expected_arr < (size_t)multiple_value_count;
       expected_arr++)
    {
      CU_ASSERT_TRUE (found[expected_arr]);
    }

  // free memory
  ioopm_iterator_destroy (&key_iter);
  ioopm_linked_list_destroy (&key_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_multiple_keys_empty_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);

  ioopm_list_t *key_list = NULL;
  ioopm_hash_table_keys (&key_list, ht);

  size_t key_count = 0;
  ioopm_linked_list_size (&key_count, key_list);

  CU_ASSERT_TRUE (key_count == 0);

  // free memory
  ioopm_linked_list_destroy (&key_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_multiple_keys_single_key_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, integer_hash, integer_eq, string_eq);

  bool found[single_value_count];
  found[0] = false;

  elem_t expected_keys[single_value_count];
  expected_keys[0] = int_to_elem (5);

  elem_t inserted = ptr_to_elem ("abc");
  ioopm_hash_table_insert (ht, int_to_elem (5), inserted);

  ioopm_list_t *key_list = NULL;
  ioopm_hash_table_keys (&key_list, ht);

  ioopm_iterator_t *key_iter = NULL;
  ioopm_list_iterator (&key_iter, key_list);

  size_t key_count = 0;
  ioopm_linked_list_size (&key_count, key_list);

  CU_ASSERT_TRUE (key_count == 1);
  elem_t current;
  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, key_iter) == SUCCESS && has_next)
    {
      ioopm_iterator_next (&current, key_iter);
      for (size_t expected_arr = 0; expected_arr < (size_t)single_value_count;
           expected_arr++)
        {
          if (integer_eq (current, expected_keys[expected_arr]))
            {
              found[expected_arr] = true;
            }
        }
    }

  for (size_t expected_arr = 0; expected_arr < (size_t)single_value_count;
       expected_arr++)
    {
      CU_ASSERT_TRUE (found[expected_arr]);
    }

  // free memory
  ioopm_iterator_destroy (&key_iter);
  ioopm_linked_list_destroy (&key_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_multiple_keys_multiple_key_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  bool found[multiple_value_count];
  found[0] = false;
  found[1] = false;
  found[2] = false;
  found[3] = false;

  elem_t expected_keys[multiple_value_count];
  expected_keys[0] = int_to_elem (0);
  expected_keys[1] = int_to_elem (10);
  expected_keys[2] = int_to_elem (1);
  expected_keys[3] = int_to_elem (11);

  ioopm_hash_table_insert (ht, int_to_elem (0), ptr_to_elem ("1"));
  ioopm_hash_table_insert (ht, int_to_elem (10), ptr_to_elem ("2"));
  ioopm_hash_table_insert (ht, int_to_elem (1), ptr_to_elem ("3"));
  ioopm_hash_table_insert (ht, int_to_elem (11), ptr_to_elem ("4"));

  ioopm_list_t *key_list = NULL;
  ioopm_hash_table_keys (&key_list, ht);

  ioopm_iterator_t *key_iter = NULL;
  ioopm_list_iterator (&key_iter, key_list);

  size_t key_count = 0;
  ioopm_linked_list_size (&key_count, key_list);

  CU_ASSERT_TRUE (key_count == 4);
  elem_t current;
  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, key_iter) == SUCCESS && has_next)
    {
      ioopm_iterator_next (&current, key_iter);
      for (size_t expected_arr = 0;
           expected_arr < (size_t)multiple_value_count; expected_arr++)
        {
          if (integer_eq (current, expected_keys[expected_arr]))
            {
              found[expected_arr] = true;
            }
        }
    }

  for (size_t expected_arr = 0; expected_arr < (size_t)multiple_value_count;
       expected_arr++)
    {
      CU_ASSERT_TRUE (found[expected_arr]);
    }

  // free memory
  ioopm_iterator_destroy (&key_iter);
  ioopm_linked_list_destroy (&key_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_get_values_and_get_keys_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  ioopm_hash_table_insert (ht, int_to_elem (0), ptr_to_elem ("1"));
  ioopm_hash_table_insert (ht, int_to_elem (10), ptr_to_elem ("2"));
  ioopm_hash_table_insert (ht, int_to_elem (1), ptr_to_elem ("3"));
  ioopm_hash_table_insert (ht, int_to_elem (11), ptr_to_elem ("4"));

  ioopm_list_t *key_list = NULL;
  ioopm_hash_table_keys (&key_list, ht);

  ioopm_iterator_t *key_iter = NULL;
  ioopm_list_iterator (&key_iter, key_list);

  size_t key_count = 0;
  ioopm_linked_list_size (&key_count, key_list);

  ioopm_list_t *value_list = NULL;
  ioopm_hash_table_values (&value_list, ht);

  ioopm_iterator_t *value_iter = NULL;
  ioopm_list_iterator (&value_iter, value_list);

  size_t value_count = 0;
  ioopm_linked_list_size (&value_count, value_list);

  CU_ASSERT_TRUE (key_count == value_count);

  elem_t result;
  elem_t current_key;
  elem_t current_value;
  bool has_next_key = false;
  bool has_next_value = false;
  bool found = false;
  while (ioopm_iterator_has_next (&has_next_key, key_iter) == SUCCESS
         && has_next_key
         && ioopm_iterator_has_next (&has_next_value, value_iter) == SUCCESS
         && has_next_value)
    {
      ioopm_iterator_next (&current_key, key_iter);
      ioopm_iterator_next (&current_value, value_iter);
      ioopm_hash_table_lookup (&found, &result, ht, current_key);
      CU_ASSERT_TRUE (found);
      CU_ASSERT_TRUE (string_eq (current_value, result));
    }

  // free memory
  ioopm_iterator_destroy (&value_iter);
  ioopm_iterator_destroy (&key_iter);
  ioopm_linked_list_destroy (&value_list);
  ioopm_linked_list_destroy (&key_list);
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_delete_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 4, integer_hash, integer_eq, string_eq);
  // USE VALGRIND
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_is_empty_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);
  bool empty = false;
  for (int i = 1; i < 10; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i),
                               ptr_to_elem ("test string"));
      ioopm_hash_table_remove (ht, int_to_elem (i));
      ioopm_hash_table_is_empty (&empty, ht);
      CU_ASSERT_TRUE (empty);
    }
  ioopm_hash_table_destroy (&ht);
}

void
hash_table_size_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  size_t size = 0;
  for (size_t i = 1; i < 10; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i),
                               ptr_to_elem ("test string"));
      ioopm_hash_table_size (&size, ht);
      CU_ASSERT (size == (size_t)i);
    }

  for (size_t i = 9; i >= 1; i--) // TODO change to i>=0
    {
      ioopm_hash_table_remove (ht, int_to_elem (i));
      ioopm_hash_table_size (&size, ht);
      CU_ASSERT (size == (size_t)i - 1);
    }

  ioopm_hash_table_destroy (&ht);
}

void
hash_table_clear_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  for (int i = 1; i < 10; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i),
                               ptr_to_elem ("test string"));
    }
  bool empty = false;
  ioopm_hash_table_is_empty (&empty, ht);
  CU_ASSERT_FALSE (empty);

  ioopm_hash_table_clear (ht);
  ioopm_hash_table_is_empty (&empty, ht);
  CU_ASSERT_TRUE (empty);

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Checks if has value function returns false on empty hash table
 */
void
has_value_empty_ht_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  bool found = false;
  ioopm_hash_table_has_value (&found, ht, ptr_to_elem ("test value"));
  CU_ASSERT_FALSE (found);

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Tests has value function on a populated hash table, checking positive
 * return values
 */
void
has_value_populated_positive_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  elem_t test_values[10];
  test_values[0] = ptr_to_elem ("0");
  test_values[1] = ptr_to_elem ("1");
  test_values[2] = ptr_to_elem ("2");
  test_values[3] = ptr_to_elem ("3");
  test_values[4] = ptr_to_elem ("4");
  test_values[5] = ptr_to_elem ("5");
  test_values[6] = ptr_to_elem ("6");
  test_values[7] = ptr_to_elem ("7");
  test_values[8] = ptr_to_elem ("8");
  test_values[9] = ptr_to_elem ("9");
  for (int i = 0; i < 10; i++)
    {
      bool found = false;
      ioopm_hash_table_insert (ht, int_to_elem (i), test_values[i]);
      ioopm_hash_table_has_value (&found, ht, test_values[i]);
      CU_ASSERT_TRUE (found);
    }

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Tests has value function on a populated hash table, checking negative
 * return values
 */
void
has_value_populated_negative_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  elem_t test_values[10];
  test_values[0] = ptr_to_elem ("0");
  test_values[1] = ptr_to_elem ("1");
  test_values[2] = ptr_to_elem ("2");
  test_values[3] = ptr_to_elem ("3");
  test_values[4] = ptr_to_elem ("4");
  test_values[5] = ptr_to_elem ("5");
  test_values[6] = ptr_to_elem ("6");
  test_values[7] = ptr_to_elem ("7");
  test_values[8] = ptr_to_elem ("8");
  test_values[9] = ptr_to_elem ("9");
  for (int i = 0; i < 10; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i), test_values[i]);
    }
  bool found = false;
  ioopm_hash_table_has_value (&found, ht, ptr_to_elem ("10"));
  CU_ASSERT_FALSE (found);

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Checks if has key function returns false on empty hash table
 */
void
has_key_empty_ht_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  bool found = false;
  ioopm_hash_table_has_key (&found, ht, int_to_elem (0));
  CU_ASSERT_FALSE (found);

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Tests has key function on a populated hash table, checking negative
 * return values
 */
void
has_key_populated_negative_ht_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  for (int i = 0; i < 10; i++)
    {
      // ioopm_hash_table_insert(ht, int_to_elem(i), ptr_to_elem(""));
    }

  bool found = false;
  for (int i = 10; i < 20; i++)
    {
      ioopm_hash_table_has_key (&found, ht, int_to_elem (i));
      CU_ASSERT_FALSE (found);
    }

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Tests has key function on a populated hash table, checking positive
 * return values
 */
void
has_key_populated_positive_ht_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  for (int i = 0; i < 10; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i), ptr_to_elem (""));
    }

  bool found = false;
  for (int i = 0; i < 10; i++)
    {
      ioopm_hash_table_has_key (&found, ht, int_to_elem (i));
      CU_ASSERT_TRUE (found);
    }

  ioopm_hash_table_destroy (&ht);
}

/// Helper function for testing the all functions ///
// Checks if value is equivalent to the other_string
bool
value_equiv (const elem_t key_ignored __attribute__ ((unused)),
             const elem_t value, const void *other_string)
{
  return string_eq (value, *((elem_t *)other_string));
}

/*
 * @brief Checks if the all function returns true when hash table is empty
 */
void
all_empty_ht_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  elem_t test_value = ptr_to_elem ("test value");
  bool all = false;
  ioopm_hash_table_all (&all, ht, value_equiv, &test_value);
  CU_ASSERT_TRUE (all);

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Checks if the all function, checking positive return values
 */
void
all_populated_ht_positive_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  elem_t test_value = ptr_to_elem ("test value");

  // Only insert test_value value
  bool all = false;
  for (int i = 0; i < 10; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i), test_value);
      ioopm_hash_table_all (&all, ht, value_equiv, &test_value);
      CU_ASSERT_TRUE (all);
    }

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Checks if the all function, checking positive return values
 */
void
all_populated_ht_negative_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  elem_t test_value = ptr_to_elem ("test value");

  // Insert non test_value values
  for (int i = 0; i < 10; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i),
                               ptr_to_elem ("non test value"));
    }
  bool all = false;
  ioopm_hash_table_all (&all, ht, value_equiv, &test_value);
  CU_ASSERT_FALSE (all);

  // Insert test_value values
  for (int i = 10; i < 20; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i),
                               ptr_to_elem ("test value"));
    }
  ioopm_hash_table_all (&all, ht, value_equiv, &test_value);
  CU_ASSERT_FALSE (all);

  ioopm_hash_table_destroy (&ht);
}

/// Helper function for testing the any functions ///
// Checks if a given key 'key' is equivalent to the key 'x'
bool
key_equiv (const elem_t key,
           const elem_t value_ignored __attribute__ ((unused)), const void *x)
{
  return integer_eq (key, *((elem_t *)x));
}

/*
 * @brief Checks if the any function returns false when hash table is empty
 */
void
any_empty_ht_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  elem_t test_key = int_to_elem (0);
  bool any = false;
  ioopm_hash_table_any (&any, ht, key_equiv, &test_key);
  CU_ASSERT_FALSE (any);

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Tests the any function on a populated hash table, checking negative
 * return values
 */
void
any_populated_ht_negative_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  elem_t test_key = int_to_elem (0);

  // Insert non test_key key_list
  bool any = false;
  for (int i = 1; i < 10; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i), ptr_to_elem (""));
      ioopm_hash_table_any (&any, ht, key_equiv, &test_key);
      CU_ASSERT_FALSE (any);
    }

  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Test the any function on a populated hash table, checking positive
 * return values
 */
void
any_populated_ht_positive_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, NULL, integer_eq, string_eq);

  elem_t test_key = int_to_elem (0);

  // Insert non test_key key_list
  for (int i = 1; i < 10; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i), ptr_to_elem (""));
    }

  // Insert test_key
  ioopm_hash_table_insert (ht, test_key, ptr_to_elem ("test string"));
  bool any = false;
  ioopm_hash_table_any (&any, ht, key_equiv, &test_key);
  CU_ASSERT_TRUE (any);

  ioopm_hash_table_destroy (&ht);
}

/// Helper functions for testing apply to all function ///
// Sets the string value
int
reset_all_values (elem_t key_ignored __attribute__ ((unused)), elem_t *value,
                  void *extra)
{
  value->pointer = (char *)extra;
  return 0;
}

// Checks if value is equivalent to the string "Reset value"
bool
eq_to_reset_value (const elem_t key_ignored __attribute__ ((unused)),
                   const elem_t value, const void *extra)
{
  return 0 == strcmp (elem_to_string (value), (char *)extra);
}

/*
 * @brief Checks if apply to all function does not change an empty hash table
 */
void
apply_to_all_empty_ht_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 1, integer_hash, integer_eq, string_eq);

  // Set all hash table values to reset value
  char *reset_value = "Reset value";
  ioopm_hash_table_apply_to_all (ht, reset_all_values, reset_value);

  // Check if hash table is still empty
  bool empty = false;
  ioopm_hash_table_is_empty (&empty, ht);
  CU_ASSERT_TRUE (empty);
  ioopm_hash_table_destroy (&ht);
}

/*
 * @brief Checks if apply to all function updates all values
 */
void
apply_to_all_populated_ht_test (void)
{
  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 5, integer_hash, integer_eq, string_eq);

  // Insert 50 entries
  for (int i = 0; i < 50; i++)
    {
      ioopm_hash_table_insert (ht, int_to_elem (i),
                               ptr_to_elem ("test string"));
    }

  // Set all hash table values to reset value
  char *reset_value = "Reset value";
  ioopm_hash_table_apply_to_all (ht, reset_all_values, reset_value);

  // Check if hash table values are updated
  bool all = false;
  ioopm_hash_table_all (&all, ht, eq_to_reset_value, reset_value);
  CU_ASSERT_TRUE (all);
  ioopm_hash_table_destroy (&ht);
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite hash_table_test_suite
      = CU_add_suite ("Hash Table Testing Suite", init_suite, clean_suite);
  if (hash_table_test_suite == NULL)
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
  if ((CU_add_test (hash_table_test_suite,
                    "Create a hash table with one bucket",
                    create_hash_table_single_bucket_test)
       == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Create a hash table with multiple buckets",
                       create_hash_table_multiple_bucket_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Inserting a single item into a hash table with a single bucket",
              hash_table_single_insert_one_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Inserting existing key into a hash table with single bucket",
              hash_table_single_insert_duplicate_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Inserting multiple items into a hash table with single bucket",
              hash_table_single_insert_multiple_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Inserting a single item into a hash table with "
                       "multiple buckets",
                       hash_table_multiple_insert_one_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "inserting existing key into a hash table with multiple buckets",
              hash_table_multiple_insert_duplicate_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Inserting multiple items into a hash table with "
                       "multiple buckets",
                       hash_table_multiple_insert_multiple_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Removing first item of hash table with single bucket",
                       hash_table_single_remove_first_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Removing last item of hash table with single bucket",
                       hash_table_single_remove_last_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Removing item that dose not exist (single buckets)",
                       hash_table_single_remove_empty_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Removing first item of hash table with multiple buckets",
              hash_table_multiple_remove_first_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Removing last item of hash table with multiple buckets",
              hash_table_multiple_remove_last_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Removing middle item of hash table whit multiple buckets",
              hash_table_multiple_remove_middle_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Removing item that dose not exist (multiple buckets)",
                       hash_table_multiple_remove_empty_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Get all values in hash table that is empty (single bucket)",
              hash_table_single_values_empty_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Get all values in hash table with one item (single bucket)",
              hash_table_single_values_single_value_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Get all values in hash table that contains multiple "
                       "values (single bucket)",
                       hash_table_single_values_multiple_value_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Get all values in empty hash table (single bucket)",
                       hash_table_multiple_values_empty_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Get all values in hash table with one value (single bucket)",
              hash_table_multiple_values_single_value_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Get all values in hash table with multiple values "
                       "(single bucket)",
                       hash_table_multiple_values_multiple_values_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Get all key_list in hash table that is empty (single buckets)",
              hash_table_single_keys_empty_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Get all key_list in hash table that has one item "
                       "(single buckets)",
                       hash_table_single_keys_single_key_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Get all key_list in hash table that has multiple "
                       "items (single buckets)",
                       hash_table_single_keys_multiple_keys_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Get all key_list in hash table that is empty "
                       "(multiple buckets)",
                       hash_table_multiple_keys_empty_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Get all key_list in hash table with one item "
                       "(multiple buckets)",
                       hash_table_multiple_keys_single_key_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Get all key_list in hash table that contains multiple "
                       "values (multiple buckets)",
                       hash_table_multiple_keys_multiple_key_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Test order of values and key arrays",
                       hash_table_get_values_and_get_keys_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Deleting hash table, Use valgrind to know if memory is freed",
              hash_table_delete_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Checks if a hash table is empty",
                       hash_table_is_empty_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Creates a hash table of different sizes and checks its size",
              hash_table_size_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite, "Clears hash table",
                       hash_table_clear_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Checks positive return values the has value function "
                       "on a populated hash table",
                       has_value_empty_ht_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests positive return values the has value function "
                       "on a populated hash table",
                       has_value_populated_positive_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests negative return values the has value function "
                       "on a populated hash table",
                       has_value_populated_negative_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests the has key function on an empty hash table",
                       has_key_empty_ht_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests positive return values the has key function on "
                       "a populated hash table",
                       has_key_populated_positive_ht_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests negative return values the has key function on "
                       "a populated hash table",
                       has_key_populated_negative_ht_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests the has_key function on an empty hash table",
                       has_key_empty_ht_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Tests the all predicate function on an empty hash table",
              all_empty_ht_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests positive return values on the all predicate "
                       "function on a populated hash table",
                       all_populated_ht_positive_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests negative return values on the all predicate "
                       "function on a populated hash table",
                       all_populated_ht_negative_test)
          == NULL)
      || (CU_add_test (
              hash_table_test_suite,
              "Tests the any predicate function on an empty hash table",
              any_empty_ht_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests positive return values on the any predicate "
                       "function on a populated hash table",
                       any_populated_ht_positive_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests negative return values on the any predicate "
                       "function on a populated hash table",
                       any_populated_ht_negative_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests apply to all function on an empty hash table",
                       apply_to_all_empty_ht_test)
          == NULL)
      || (CU_add_test (hash_table_test_suite,
                       "Tests apply to all function on a populated hash table",
                       apply_to_all_populated_ht_test)
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

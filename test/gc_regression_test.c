#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lib/hash_table.h"
#include "../lib/linked_list.h"
#include "../lib/utils.h"
#include "../src/gc.h"

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

typedef struct array_of_pointers
{
  void **ptrs;
} array_of_pointers_t;

static size_t
string_sum_hash (elem_t e)
{
  char *str = elem_to_string (e);
  size_t result = 0;
  do
    {
      result += *str;
    }
  while (*++str != '\0');
  return result;
}

static bool
string_eq (elem_t e1, elem_t e2)
{
  return (strcmp (elem_to_string (e1), elem_to_string (e2)) == 0);
}

static bool
size_t_eq (elem_t a, elem_t b)
{
  return elem_to_size (a) == elem_to_size (b);
}

void
create_array_of_ten_pointers (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  // alloc root object
  array_of_pointers_t *entry = h_alloc_struct (heap, "*");

  // alloc array of 10 pointers
  entry->ptrs = h_alloc_struct (heap, "10*");
  void *garbage = NULL;
  (void)garbage;

  for (size_t n = 0; n < 10; n++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }
      garbage = NULL;

      // alloc data to insert into pointers
      int *i = h_alloc_raw (heap, sizeof (int));
      *i = n;
      entry->ptrs[n] = i;
      i = NULL;

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }
      garbage = NULL;

      h_gc (heap);
    }

  /* create garbage */
  garbage = h_alloc_raw (heap, sizeof (int));
  garbage = h_alloc_raw (heap, sizeof (int));
  garbage = h_alloc_raw (heap, sizeof (int));
  garbage = NULL;

  h_gc (heap);

  for (size_t i = 0; i < 10; i++)
    {
      CU_ASSERT_EQUAL (*((int *)entry->ptrs[i]), i);
    }

  h_delete (heap);
}

void
create_linked_list_with_heap_one (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, string_eq);

  CU_ASSERT_PTR_NOT_NULL (list);

  void *garbage = NULL;
  (void)garbage;
  for (size_t i = 0; i <= 'A' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      garbage = NULL;
      char string[2] = "A";
      string[0] = 'A' + i;
      ioopm_linked_list_append (list, string_to_elem (heap_strdup (string)));

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }
      garbage = NULL;
      h_gc (heap);
    }

  bool found;
  for (size_t i = 0; i <= 'A' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      ioopm_linked_list_contains (&found, list,
                                  string_to_elem (heap_strdup (string)));
      CU_ASSERT_TRUE (found);

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }
  ioopm_linked_list_destroy (&list);

  h_delete (heap);
}

void
create_linked_list_with_heap_two (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, string_eq);

  CU_ASSERT_PTR_NOT_NULL (list);

  void *garbage = NULL;
  (void)garbage;

  for (size_t i = 0; i <= 'B' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "A";
      string[0] = 'A' + i;
      ioopm_linked_list_append (list, string_to_elem (heap_strdup (string)));

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }

  bool found;
  for (size_t i = 0; i <= 'B' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      ioopm_linked_list_contains (&found, list,
                                  string_to_elem (heap_strdup (string)));
      CU_ASSERT_TRUE (found);

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }
  ioopm_linked_list_destroy (&list);

  h_delete (heap);
}

void
create_linked_list_with_heap_three (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, string_eq);

  CU_ASSERT_PTR_NOT_NULL (list);

  void *garbage = NULL;
  (void)garbage;

  for (size_t i = 0; i <= 'C' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "A";
      string[0] = 'A' + i;
      ioopm_linked_list_append (list, string_to_elem (heap_strdup (string)));

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }

  bool found;
  for (size_t i = 0; i <= 'C' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      ioopm_linked_list_contains (&found, list,
                                  string_to_elem (heap_strdup (string)));
      CU_ASSERT_TRUE (found);

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }
  ioopm_linked_list_destroy (&list);

  h_delete (heap);
}

void
create_linked_list_with_heap_four (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, string_eq);

  CU_ASSERT_PTR_NOT_NULL (list);

  void *garbage = NULL;
  (void)garbage;

  for (size_t i = 0; i <= 'D' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "A";
      string[0] = 'A' + i;
      ioopm_linked_list_append (list, string_to_elem (heap_strdup (string)));

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }

  bool found;
  for (size_t i = 0; i <= 'D' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      ioopm_linked_list_contains (&found, list,
                                  string_to_elem (heap_strdup (string)));
      CU_ASSERT_TRUE (found);

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }
  ioopm_linked_list_destroy (&list);

  h_delete (heap);
}

void
create_linked_list_with_heap_26 (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, string_eq);

  CU_ASSERT_PTR_NOT_NULL (list);

  void *garbage = NULL;
  (void)garbage;

  for (size_t i = 0; i <= 'Z' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "A";
      string[0] = 'A' + i;
      ioopm_linked_list_append (list, string_to_elem (heap_strdup (string)));

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }

  bool found;
  for (size_t i = 0; i <= 'Z' - 'A'; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      ioopm_linked_list_contains (&found, list,
                                  string_to_elem (heap_strdup (string)));
      CU_ASSERT_TRUE (found);

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }
  ioopm_linked_list_destroy (&list);

  h_delete (heap);
}

void
create_linked_list_with_heap_254 (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, string_eq);

  CU_ASSERT_PTR_NOT_NULL (list);

  void *garbage = NULL;
  (void)garbage;

  for (size_t i = 0; i < 255; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "\1";
      string[0] = '\1' + i;
      ioopm_linked_list_append (list, string_to_elem (heap_strdup (string)));

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }

  bool found;
  for (size_t i = 0; i < 255; i++)
    {
      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      char string[2] = "\1";
      string[0] = '\1' + i;
      found = false;
      ioopm_linked_list_contains (&found, list,
                                  string_to_elem (heap_strdup (string)));
      CU_ASSERT_TRUE (found);

      /* create garbage */
      for (size_t i = 0; i < 1024; i++)
        {
          garbage = h_alloc_raw (heap, sizeof (int));
        }

      h_gc (heap);
    }
  ioopm_linked_list_destroy (&list);

  h_delete (heap);
}

void
create_linked_list_with_heap_762 (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_list_t *list = NULL;
  ioopm_linked_list_create (&list, string_eq);

  CU_ASSERT_PTR_NOT_NULL (list);

  void *garbage = NULL;
  (void)garbage;

  for (size_t i = 0; i < 255; i++)
    {

      char string[3] = "\1";
      string[0] = '\1' + i;
      for (size_t i = 0; i < 3; i++)
        {

          /* create garbage */
          for (size_t i = 0; i < 1024; i++)
            {
              garbage = h_alloc_raw (heap, sizeof (int));
            }

          string[1] = '0' + i;
          ioopm_linked_list_append (list,
                                    string_to_elem (heap_strdup (string)));

          /* create garbage */
          for (size_t i = 0; i < 1024; i++)
            {
              garbage = h_alloc_raw (heap, sizeof (int));
            }

          h_gc (heap);
        }
    }

  bool found;
  for (size_t i = 0; i < 255; i++)
    {
      char string[3] = "\1";
      string[0] = '\1' + i;
      for (size_t i = 0; i < 3; i++)
        {

          /* create garbage */
          for (size_t i = 0; i < 1024; i++)
            {
              garbage = h_alloc_raw (heap, sizeof (int));
            }

          string[1] = '0' + i;
          found = false;
          ioopm_linked_list_contains (&found, list,
                                      string_to_elem (heap_strdup (string)));
          CU_ASSERT_TRUE (found);

          /* create garbage */
          for (size_t i = 0; i < 1024; i++)
            {
              garbage = h_alloc_raw (heap, sizeof (int));
            }

          h_gc (heap);
        }
    }
  ioopm_linked_list_destroy (&list);

  h_delete (heap);
}

void
create_hash_table_with_one (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, string_sum_hash, string_eq, size_t_eq);

  CU_ASSERT_PTR_NOT_NULL (ht);

  for (size_t i = 0; i <= 'A' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      char buf[4];
      sprintf (buf, "%d", (int)i);
      char *str_key = heap_strdup (string);
      char *str_val = heap_strdup (buf);
      ioopm_hash_table_insert (ht, string_to_elem (str_key),
                               string_to_elem (str_val));
      h_gc (heap);
    }

  bool found;
  elem_t ret_val = ptr_to_elem (NULL);
  for (size_t i = 0; i <= 'A' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      char *str_key = heap_strdup (string);
      ioopm_hash_table_lookup (&found, &ret_val, ht, string_to_elem (str_key));
      h_gc (heap);
      CU_ASSERT_TRUE (found);
      char buf[4];
      sprintf (buf, "%d", (int)i);
      CU_ASSERT_STRING_EQUAL (elem_to_size (ret_val), buf);
    }
  ioopm_hash_table_destroy (&ht);

  h_delete (heap);
}

void
create_hash_table_with_two (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, string_sum_hash, string_eq, size_t_eq);

  CU_ASSERT_PTR_NOT_NULL (ht);

  for (size_t i = 0; i <= 'B' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      char buf[4];
      sprintf (buf, "%d", (int)i);
      char *str_key = heap_strdup (string);
      char *str_val = heap_strdup (buf);
      ioopm_hash_table_insert (ht, string_to_elem (str_key),
                               string_to_elem (str_val));
      h_gc (heap);
    }

  bool found;
  elem_t ret_val = ptr_to_elem (NULL);
  for (size_t i = 0; i <= 'B' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      char *str_key = heap_strdup (string);
      ioopm_hash_table_lookup (&found, &ret_val, ht, string_to_elem (str_key));
      h_gc (heap);
      CU_ASSERT_TRUE (found);
      char buf[4];
      sprintf (buf, "%d", (int)i);
      CU_ASSERT_STRING_EQUAL (elem_to_size (ret_val), buf);
    }
  ioopm_hash_table_destroy (&ht);

  h_delete (heap);
}

void
create_hash_table_with_three (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, string_sum_hash, string_eq, size_t_eq);

  CU_ASSERT_PTR_NOT_NULL (ht);

  for (size_t i = 0; i <= 'C' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      char buf[4];
      sprintf (buf, "%d", (int)i);
      char *str_key = heap_strdup (string);
      char *str_val = heap_strdup (buf);
      ioopm_hash_table_insert (ht, string_to_elem (str_key),
                               string_to_elem (str_val));
      h_gc (heap);
    }

  bool found;
  elem_t ret_val = ptr_to_elem (NULL);
  for (size_t i = 0; i <= 'C' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      char *str_key = heap_strdup (string);
      ioopm_hash_table_lookup (&found, &ret_val, ht, string_to_elem (str_key));
      h_gc (heap);
      CU_ASSERT_TRUE (found);
      char buf[4];
      sprintf (buf, "%d", (int)i);
      CU_ASSERT_STRING_EQUAL (elem_to_size (ret_val), buf);
    }
  ioopm_hash_table_destroy (&ht);

  h_delete (heap);
}

void
create_hash_table_with_four (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, string_sum_hash, string_eq, size_t_eq);

  CU_ASSERT_PTR_NOT_NULL (ht);

  for (size_t i = 0; i <= 'D' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      char buf[4];
      sprintf (buf, "%d", (int)i);
      char *str_key = heap_strdup (string);
      char *str_val = heap_strdup (buf);
      ioopm_hash_table_insert (ht, string_to_elem (str_key),
                               string_to_elem (str_val));
      h_gc (heap);
    }

  bool found;
  elem_t ret_val = ptr_to_elem (NULL);
  for (size_t i = 0; i <= 'D' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      char *str_key = heap_strdup (string);
      ioopm_hash_table_lookup (&found, &ret_val, ht, string_to_elem (str_key));
      h_gc (heap);
      CU_ASSERT_TRUE (found);
      char buf[4];
      sprintf (buf, "%d", (int)i);
      CU_ASSERT_STRING_EQUAL (elem_to_size (ret_val), buf);
    }
  ioopm_hash_table_destroy (&ht);

  h_delete (heap);
}

void
create_hash_table_with_26 (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, string_sum_hash, string_eq, size_t_eq);

  CU_ASSERT_PTR_NOT_NULL (ht);

  for (size_t i = 0; i <= 'Z' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      char buf[4];
      sprintf (buf, "%d", (int)i);
      char *str_key = heap_strdup (string);
      char *str_val = heap_strdup (buf);
      ioopm_hash_table_insert (ht, string_to_elem (str_key),
                               string_to_elem (str_val));
      h_gc (heap);
    }

  bool found;
  elem_t ret_val = ptr_to_elem (NULL);
  for (size_t i = 0; i <= 'Z' - 'A'; i++)
    {
      char string[2] = "A";
      string[0] = 'A' + i;
      found = false;
      char *str_key = heap_strdup (string);
      ioopm_hash_table_lookup (&found, &ret_val, ht, string_to_elem (str_key));
      h_gc (heap);
      CU_ASSERT_TRUE (found);
      char buf[4];
      sprintf (buf, "%d", (int)i);
      CU_ASSERT_STRING_EQUAL (elem_to_size (ret_val), buf);
    }
  ioopm_hash_table_destroy (&ht);

  h_delete (heap);
}

void
create_hash_table_with_64 (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, string_sum_hash, string_eq, size_t_eq);

  CU_ASSERT_PTR_NOT_NULL (ht);

  for (size_t i = 0; i < 64; i++)
    {
      char string[2] = "\1";
      string[0] = '\1' + i;
      char buf[4];
      sprintf (buf, "%d", (int)i);
      char *str_key = heap_strdup (string);
      char *str_val = heap_strdup (buf);
      ioopm_hash_table_insert (ht, string_to_elem (str_key),
                               string_to_elem (str_val));
      h_gc (heap);
    }

  bool found;
  elem_t ret_val = ptr_to_elem (NULL);
  for (size_t i = 0; i < 64; i++)
    {
      char string[2] = "\1";
      string[0] = '\1' + i;
      found = false;
      char *str_key = heap_strdup (string);
      ioopm_hash_table_lookup (&found, &ret_val, ht, string_to_elem (str_key));
      h_gc (heap);
      CU_ASSERT_TRUE (found);
      char buf[4];
      sprintf (buf, "%d", (int)i);
      CU_ASSERT_STRING_EQUAL (elem_to_size (ret_val), buf);
    }
  ioopm_hash_table_destroy (&ht);

  h_delete (heap);
}

void
create_hash_table_with_128 (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, string_sum_hash, string_eq, size_t_eq);

  CU_ASSERT_PTR_NOT_NULL (ht);

  for (size_t i = 0; i < 128; i++)
    {
      char string[2] = "\1";
      string[0] = '\1' + i;
      char buf[4];
      sprintf (buf, "%d", (int)i);
      char *str_key = heap_strdup (string);
      char *str_val = heap_strdup (buf);
      ioopm_hash_table_insert (ht, string_to_elem (str_key),
                               string_to_elem (str_val));
      h_gc (heap);
    }

  bool found;
  elem_t ret_val = ptr_to_elem (NULL);
  for (size_t i = 0; i < 128; i++)
    {
      char string[2] = "\1";
      string[0] = '\1' + i;
      found = false;
      char *str_key = heap_strdup (string);
      ioopm_hash_table_lookup (&found, &ret_val, ht, string_to_elem (str_key));
      h_gc (heap);
      CU_ASSERT_TRUE (found);
      char buf[4];
      sprintf (buf, "%d", (int)i);
      CU_ASSERT_STRING_EQUAL (elem_to_size (ret_val), buf);
    }
  ioopm_hash_table_destroy (&ht);

  h_delete (heap);
}

void
create_hash_table_with_254 (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 10, string_sum_hash, string_eq, size_t_eq);

  CU_ASSERT_PTR_NOT_NULL (ht);

  for (size_t i = 0; i < 255; i++)
    {
      char string[2] = "\1";
      string[0] = '\1' + i;
      char buf[4];
      sprintf (buf, "%d", (int)i);
      char *str_key = heap_strdup (string);
      char *str_val = heap_strdup (buf);
      ioopm_hash_table_insert (ht, string_to_elem (str_key),
                               string_to_elem (str_val));
      h_gc (heap);
    }

  bool found;
  elem_t ret_val = ptr_to_elem (NULL);
  for (size_t i = 0; i < 255; i++)
    {
      char string[2] = "\1";
      string[0] = '\1' + i;
      found = false;
      char *str_key = heap_strdup (string);
      ioopm_hash_table_lookup (&found, &ret_val, ht, string_to_elem (str_key));
      h_gc (heap);
      CU_ASSERT_TRUE (found);
      char buf[4];
      sprintf (buf, "%d", (int)i);
      CU_ASSERT_STRING_EQUAL (elem_to_size (ret_val), buf);
    }
  ioopm_hash_table_destroy (&ht);

  h_delete (heap);
}

void
create_hash_table_with_ptr_format_string (void)
{
  size_t size = 1024 * 1024 * 2;
  heap_t *heap = h_init (size, true, 1.0f);

  ioopm_hash_table_t *ht = NULL;
  ioopm_create_hash_table (&ht, 100, string_sum_hash, string_eq, size_t_eq);

  CU_ASSERT_PTR_NOT_NULL (ht);

  for (size_t i = 0; i < 125; i++)
    {
      char string[4];
      char buf[4];
      sprintf (string, "%d", (int)i);
      sprintf (buf, "%d", (int)i);
      char *str_key = heap_strdup (string);
      char *str_val = heap_strdup (buf);
      ioopm_hash_table_insert (ht, string_to_elem (str_key),
                               string_to_elem (str_val));
      h_gc (heap);
    }

  bool found;
  elem_t ret_val = ptr_to_elem (NULL);
  for (size_t i = 0; i < 125; i++)
    {
      char string[4];
      sprintf (string, "%d", (int)i);
      found = false;
      char *str_key = heap_strdup (string);
      ioopm_hash_table_lookup (&found, &ret_val, ht, string_to_elem (str_key));
      h_gc (heap);
      CU_ASSERT_TRUE (found);
      char buf[4];
      sprintf (buf, "%d", (int)i);
      CU_ASSERT_STRING_EQUAL (elem_to_size (ret_val), buf);
    }
  ioopm_hash_table_destroy (&ht);

  h_delete (heap);
}

int
main (void)
{
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  CU_pSuite linked_list = CU_add_suite ("h_gc when working with linked list",
                                        init_suite, clean_suite);
  CU_pSuite hash_table = CU_add_suite ("h_gc when working with hash table",
                                       init_suite, clean_suite);
  CU_pSuite format_string = CU_add_suite ("h_gc when working with "
                                          "pointers to format string",
                                          init_suite, clean_suite);

  if (linked_list == NULL || hash_table == NULL)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  if (0
      || (CU_add_test (linked_list, "Create a array of 10 int pointers",
                       create_array_of_ten_pointers)
          == NULL)
      || (CU_add_test (linked_list,
                       "Create a linked list with 1 value in heap and see if "
                       "gc messes with values",
                       create_linked_list_with_heap_one)
          == NULL)
      || (CU_add_test (linked_list,
                       "Create a linked list with 2 values in heap and see if "
                       "gc messes with values",
                       create_linked_list_with_heap_two)
          == NULL)
      || (CU_add_test (linked_list,
                       "Create a linked list with 3 values in heap and see if "
                       "gc messes with values",
                       create_linked_list_with_heap_three)
          == NULL)
      || (CU_add_test (linked_list,
                       "Create a linked list with 4 values in heap and see if "
                       "gc messes with values",
                       create_linked_list_with_heap_four)
          == NULL)
      || (CU_add_test (linked_list,
                       "Create a linked list with 26 values in heap and see "
                       "if gc messes with values",
                       create_linked_list_with_heap_26)
          == NULL)
      // || (CU_add_test (linked_list,
      //                  "Create a linked list with 254 values in "
      //                  "heap and see if gc messes with values",
      //                  create_linked_list_with_heap_254)
      //     == NULL)
      // || (CU_add_test (linked_list,
      //                  "Create a linked list with 762 values in "
      //                  "heap and see if gc messes with values",
      //                  create_linked_list_with_heap_762)
      //     == NULL)
      || (CU_add_test (hash_table,
                       "Create a hashtable with 1 value and see if gc "
                       "messes with it",
                       create_hash_table_with_one)
          == NULL)
      || (CU_add_test (hash_table,
                       "Create a hashtable with 2 values and see if gc "
                       "messes with it",
                       create_hash_table_with_two)
          == NULL)
      || (CU_add_test (hash_table,
                       "Create a hashtable with 3 values and see if gc "
                       "messes with it",
                       create_hash_table_with_three)
          == NULL)
      || (CU_add_test (hash_table,
                       "Create a hashtable with 4 values and see if gc "
                       "messes with it",
                       create_hash_table_with_four)
          == NULL)
      || (CU_add_test (hash_table,
                       "Create a hashtable with 26 values and see if gc "
                       "messes with it",
                       create_hash_table_with_26)
          == NULL)
      || (CU_add_test (hash_table,
                       "Create a hashtable with 64 values and see if gc "
                       "messes with it",
                       create_hash_table_with_64)
          == NULL)
      || (CU_add_test (hash_table,
                       "Create a hashtable with 128 values and see if gc "
                       "messes with it",
                       create_hash_table_with_128)
          == NULL)
      || (CU_add_test (hash_table,
                       "Create a hashtable with 254 values and see if gc "
                       "messes with it",
                       create_hash_table_with_254)
          == NULL)
      || (CU_add_test (format_string,
                       "Create a hashtable with 100 buckects which will cause "
                       "it to use a pointer to forwarding string instead of "
                       "bitvector and see if gc messes with it",
                       create_hash_table_with_ptr_format_string)
          == NULL)
      || 0)
    {
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  CU_basic_set_mode (CU_BRM_VERBOSE);

  CU_basic_run_tests ();

  int exit_code = CU_get_number_of_tests_failed () == 0
                      ? CU_get_error ()
                      : CU_get_number_of_tests_failed ();

  CU_cleanup_registry ();

  return exit_code;
}

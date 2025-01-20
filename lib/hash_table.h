/**
 * @file hash_table.h
 * @author Hampus Toft & Vilja Schnell Melander
 * @date 17 september 2024
 * @brief Simple hash table that maps unique keys to values.
 *
 * An implemention of a hash table.
 * Handles conflicts using linked list chaining.
 * Uses modular hashing to find the index of a key-value pair.
 *
 * @see $CANVAS_OBJECT_REFERENCE$/assignments/gb54499f3b7b264e3af3b68c756090f52
 */

#pragma once

#include <stdbool.h>

#include "common.h"
#include "linked_list.h"

#define INVALID_HASH_TABLE_POINTER 11
#define INVALID_HASH_TABLE_SIZE 12
#define INVALID_BUCKET_COUNT 13

typedef struct hash_table ioopm_hash_table_t;

typedef size_t ioopm_hash_function (const elem_t key);

typedef bool ioopm_hash_table_predicate (const elem_t key, const elem_t value,
                                         const void *extra);
typedef int ioopm_hash_table_apply_function (elem_t key, elem_t *value,
                                             void *extra);

/**
 * @brief Create a hash table with an array of size bucket_count.
 * @note Bucket_count must be greater than zero.
 * @param return_ht A pointer to the adress where the hash table should be
 * created.
 * @param bucket_count The size of the hash table array.
 * @param hash_func The hashing function to convert elem_t uninon into size_t.
 * @param key_eq_check Function used when comparing keys against each other.
 * @param value_eq_check Function used when comparing values against each
 * other.
 * @return The exit status of the function:
 *  - SUCCESS:                      hash table was successfully created
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 *  - MEMORY_ALLOCATION_FAILURE:    memory allocation failed
 *  - INVALID_BUCKET_COUNT:         bucket_count is not valid
 */
int ioopm_create_hash_table (ioopm_hash_table_t **return_ht,
                             size_t bucket_count,
                             ioopm_hash_function *hash_func,
                             ioopm_eq_function *key_eq_check,
                             ioopm_eq_function *value_eq_check);

/**
 * @brief Insert a key-value pair into the hash table.
 * @param ht Hash table inserted into.
 * @param key The key from the key-value pair that is inserted.
 * @param value The value from the key-value pair that is inserted.
 * @attention To avoid aliasing, the caller should not continue to use any
 * pointers inserted into the hash table. One solution is to insert copies.
 * @return The exit status of the function:
 *  - SUCCESS:                      insert successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 */
int ioopm_hash_table_insert (ioopm_hash_table_t *ht, const elem_t key,
                             const elem_t value);

/**
 * @brief Get a value from a hash table using its key.
 * @param return_found A bool that shows whether or not the key was found is
 * returned via this parameter.
 * @param return_value If found, a pointer to the value in the hash table is
 * returned via this parameter.
 * @param ht The hash table in which the search is conducted.
 * @param key The key that will be looked up.
 * @attention The return_value parameter is updated to point to a value
 * belonging to the hashtable. To avoid aliasing, the caller should create and
 * use a copy of the return_value.
 * @return The exit status of the function:
 *  - SUCCESS:                      lookup successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 *  - INVALID_RETURN_POINTER:       found or return_value is NULL (invalid
 * pointer)
 */
int ioopm_hash_table_lookup (bool *return_found, elem_t *return_value,
                             const ioopm_hash_table_t *ht, const elem_t key);

/**
 * @brief Remove a key-value pair from a hash table.
 * @param ht The hash table from which a key-value pair is removed.
 * @param key The key from the key-value pair that will be removed.
 * @note If key is a pointer it is not freed.
 * @return The exit status of the function:
 *  - SUCCESS:                      remove successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 */
int ioopm_hash_table_remove (ioopm_hash_table_t *ht, const elem_t key);

/**
 * @brief Delete a hash table.
 * @param ht Pointer to the hash table to destroy. Sets hash table pointer to
 * NULL if successful.
 * @note Any pointers inserted into the hash table are not freed.
 * @return The exit status of the function:
 *  - SUCCESS:                      remove successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 */
int ioopm_hash_table_destroy (ioopm_hash_table_t **ht);

/**
 * @brief Return an list of all the keys in a hash table via the key_list
 * parameter (in no particular order, but same as ioopm_hash_table_values).
 * @param return_key_list A list of all the keys in the hash table will be
 * returned via this parameter.
 * @param ht The hash table to which the keys belong.
 * @return The exit status of the function:
 *  - SUCCESS:                      return_key_list created successfuly
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 *  - INVALID_RETURN_POINTER:       return_key_list is NULL (invalid pointer)
 *  - MEMORY_ALLOCATION_FAILURE:    memory allocation failed
 * @attention The return_key_list parameter may contain pointers belonging to
 * the hash table. Therefore, to avoid aliasing, the caller should create and
 * use copies of the any pointers in the returned key list.
 * @attention Returns a pointer to a list on the heap (via the return_key_list
 * parameter) and therefore must be freed after use as to not create a memory
 * leak. Example:
 *   ```
 *   ioopm_list_t *key_list;
 *   ioopm_hash_table_keys(&key_list, ht);
 *   ...
 *   ioopm_linked_list_destroy(key_list);
 *   ```
 */
int ioopm_hash_table_keys (ioopm_list_t **return_key_list,
                           const ioopm_hash_table_t *ht);

/**
 * @brief Return a list of all the values in a hash map via the value_list
 * parameter (in no particular order, but same as ioopm_hash_table_values).
 * @param return_value_list A list of all the values in the hash table will be
 * returned via this parameter.
 * @param ht The hash table to which the values belong.
 * @return The exit status of the function:
 * - SUCCESS:                      return_value_list list created successfully
 * - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 * - INVALID_RETURN_POINTER:       return_value_list is NULL (invalid pointer)
 * - MEMORY_ALLOCATION_FAILURE:    memory allocation failed
 * @attention The return_value_list parameter may contain pointers belonging to
 * the hash table. Therefore, to avoid aliasing, the caller should create and
 * use copies of the any pointers in the returned value list.
 * @attention Returns a pointer to an list on the heap (via the key_list
 * parameter) and therefore must be freed after use as to not create a memory
 * leak. Example:
 *  ```
 *  ioopm_list_t *value_list;
 *  ioopm_hash_table_values(&value_list, ht);
 *  ...
 *  ioopm_linked_list_destroy(value_list);
 *  ```
 */
int ioopm_hash_table_values (ioopm_list_t **return_value_list,
                             const ioopm_hash_table_t *ht);

/**
 * @brief Check if a hash table has a specified key in any of its key-value
 * pairs.
 * @param return_found The result of whether or not the key was found is
 * returned via this parameter.
 * @param ht The hash table in which the search occurs.
 * @param key The key sought.
 * @return The exit status of the function:
 *  - SUCCESS:                      function call successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 *  - INVALID_RETURN_POINTER:       return_found is NULL (invalid pointer)
 */
int ioopm_hash_table_has_key (bool *return_found, const ioopm_hash_table_t *ht,
                              const elem_t key);

/**
 * @brief Check if a hash table has a specified value in any of its key-value
 * pairs.
 *  @param return_found The result of whether or not the value was found is
 * returned via this parameter.
 *  @param ht The hash table in which the search occurs.
 *  @param value The value sought.
 *  @return The exit status of the function:
 *  - SUCCESS:                      function call successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 *  - INVALID_RETURN_POINTER:       return_found is NULL (invalid pointer)
 */
int ioopm_hash_table_has_value (bool *return_found,
                                const ioopm_hash_table_t *ht,
                                const elem_t value);

/**
 * @brief Return the number of key-value pairs in the hash table.
 * @param return_size The size of the hash table is returned via this
 * parameter.
 * @param ht The hash table.
 * @return The exit status of the function:
 *  - SUCCESS:                      function call successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 *  - INVALID_RETURN_POINTER:       return_size is NULL (invalid pointer)
 */
int ioopm_hash_table_size (size_t *return_size, const ioopm_hash_table_t *ht);

/**
 * @brief Check if a hash table is empty.
 * @param return_empty Whether or not the hash table is empty is returned via
 * this parameter.
 * @param ht The hash table.
 * @return The exit status of the function:
 *  - SUCCESS:                      function call successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 *  - INVALID_RETURN_POINTER:       return_empty is NULL (invalid pointer)
 */
int ioopm_hash_table_is_empty (bool *return_empty,
                               const ioopm_hash_table_t *ht);

/**
 * @brief Clear all the entries in a hash table.
 * @param ht The hash table to be cleared.
 * @note Any pointers inserted into the hash table are not freed.
 * @return The exit status of the function:
 *  - SUCCESS:                      function call successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 */
int ioopm_hash_table_clear (ioopm_hash_table_t *ht);

/**
 * @brief Check if all key-value pairs in a hash table satisfy a given
 * predicate.
 * @param return_all The result is returned via this parameter.
 *                   Is true iff all key-value pairs in the hash table satisfy
 * a given predicate.
 * @param ht The hash table.
 * @param pred The predicate.
 * @param arg Extra argument to predicate function.
 * @return The exit status of the function:
 *  - SUCCESS:                      function call successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 *  - INVALID_RETURN_POINTER:       return_all is NULL (invalid pointer)
 */
int ioopm_hash_table_all (bool *return_all, const ioopm_hash_table_t *ht,
                          ioopm_hash_table_predicate *pred, const void *arg);

/**
 * @brief Check if any key-value pairs in a hash table satisfies a given
 * predicate.
 * @param return_any The result is returned via this parameter.
 *                   Is true iff any key-value pairs in the hash table satisies
 * a given predicate.
 * @param ht The hash table.
 * @param pred The predicate.
 * @param arg Extra argument to predicate function.
 * @return The exit status of the function:
 *  - SUCCESS:                      function call successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 *  - INVALID_RETURN_POINTER:       return_any is NULL (invalid pointer)
 */
int ioopm_hash_table_any (bool *return_any, const ioopm_hash_table_t *ht,
                          ioopm_hash_table_predicate *pred, const void *arg);

/**
 * @brief Apply a given function to all key-value pairs in a hash table.
 * @param ht The hash table operated upon.
 * @param apply_fun The function that is applied to all key-value pairs in the
 * hash table.
 * @param arg Extra argument to predicate function.
 * @return The exit status of the function:
 *  - SUCCESS:                      function call successful
 *  - INVALID_HASH_TABLE_POINTER:   ht is NULL (invalid pointer)
 */
int ioopm_hash_table_apply_to_all (ioopm_hash_table_t *ht,
                                   ioopm_hash_table_apply_function *apply_fun,
                                   void *arg);

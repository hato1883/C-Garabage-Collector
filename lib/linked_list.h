/**
 * @file linked_list.h
 * @author Hampus Toft & Vilja Schnell Melander
 * @date 17 september 2024
 * @brief linked list structure that can hold any generic data in a homogeneous
 * list
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "common.h"
#include "iterator.h"

#define INVALID_LIST_POINTER 21
#define INDEX_OUT_OF_BOUNDS 22

typedef struct list ioopm_list_t;
typedef bool ioopm_predicate (const elem_t element, const void *extra);
typedef void ioopm_apply_function (elem_t *element, void *extra);

/// @brief Create a new empty list.
/// @param return_list The created, empty list is returned via this parameter.
/// @param eq_func Used to check if two elements are equivalent.
/// @return The exit status of the function:
/// - SUCCESS:                      list created successfuly
/// - INVALID_RETURN_POINTER:       return_list is NULL (invalid pointer)
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during creation
int ioopm_linked_list_create (ioopm_list_t **return_list,
                              ioopm_eq_function *eq_func);

/// @brief Destroy a linked list.
/// @param list pointer to the list to destroy. Sets list pointer to NULL if
/// successful
/// @return the exit status of the function:
/// - SUCCESS:                      list destroyed successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
int ioopm_linked_list_destroy (ioopm_list_t **list);

/// @brief Insert at the end of a linked list in O(1) time.
/// @param list The linked list that will be appended.
/// @param element The element that is appended.
/// @return The exit status of the function:
/// - SUCCESS:                      element appended successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during node
/// creation
int ioopm_linked_list_append (ioopm_list_t *list, const elem_t element);

/// @brief Insert at the front of a linked list in O(1) time.
/// @param list The linked list that will be prepended to.
/// @param element The element that is prepended.
/// @return the exit status of the function:
/// - SUCCESS:                      element prepended successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during node
/// creation
int ioopm_linked_list_prepend (ioopm_list_t *list, const elem_t element);

/// @brief Insert an element into a linked list in O(n) time.
///        The index values are [0,n] for a list of n elements,
///        where 0 means before the first element and n means after
///        the last element.
/// @param list The linked list that will be extended.
/// @param index The position in the list.
/// @param element The value to be inserted.
/// @attention Index must be non negative.
/// @return The exit status of the function:
/// - SUCCESS:                      element inserted at index successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - INDEX_OUT_OF_BOUNDS:          index was larger than list
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during node
/// creation
int ioopm_linked_list_insert (ioopm_list_t *list, const size_t index,
                              const elem_t element);

/// @brief Remove an element from a linked list in O(n) time.
///        The index values are [0,n-1] for a list of n elements,
///        where 0 means the first element and n-1 means the last element.
/// @param return_removed The removed element is returned via this parameter.
/// @param list The linked list.
/// @param index The position in the list.
/// @attention Index must be non negative and the list must be non-empty.
/// @return The exit status of the function:
/// - SUCCESS:                      removed an element successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_removed is NULL (invalid pointer)
/// - INDEX_OUT_OF_BOUNDS:          index was larger than list
/// - UNSPECIFIED_FAILURE:          unknown error
int ioopm_linked_list_remove (elem_t *return_removed, ioopm_list_t *list,
                              const size_t index);

/// @brief Retrieve an element from a linked list in O(n) time.
///        The valid values of index are [0,n-1] for a list of n elements,
///        where 0 means the first element and n-1 means the last element.
/// @param return_element The element is returned via this parameter.
/// @param list The linked list that will be extended.
/// @param index The position in the list.
/// @attention Index must be non negative and the list must be non-empty.
/// @return The exit status of the function:
/// - SUCCESS:                      retrived the element at index successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_element is NULL (invalid pointer)
/// - INDEX_OUT_OF_BOUNDS:          index was larger than list
/// - UNSPECIFIED_FAILURE:          unknown error
int ioopm_linked_list_get (elem_t *return_element, ioopm_list_t *list,
                           const size_t index);

/// @brief Check if an element is in the list.
/// @param return_contains A bool indicating whether or not an element is in
/// the list is returned via this parameter.
/// @param list The linked list.
/// @param element The element sought.
/// @return The exit status of the function:
/// - SUCCESS:                      function call was successful
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_contains is NULL (invalid pointer)
int ioopm_linked_list_contains (bool *return_contains,
                                const ioopm_list_t *list,
                                const elem_t element);

/// @brief Lookup the number of elements in the linked list in O(1) time.
/// @param return_size The number of elements in the list is returned via this
/// parameter.
/// @param list The linked list.
/// @return The exit status of the function:
/// - SUCCESS:                      retrived size of list successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_size is NULL (invalid pointer)
size_t ioopm_linked_list_size (size_t *return_size, const ioopm_list_t *list);

/// @brief Test whether a list is empty or not
/// @param return_empty a bool indicating whether or not the list is empty is
/// returned via this parameter
/// @param list the linked list
///              true if the number of elements int the list is 0, else false
/// @return the exit status of the function:
/// - SUCCESS:                      function call was successful
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_empty is NULL (invalid pointer)
int ioopm_linked_list_is_empty (bool *return_empty, const ioopm_list_t *list);

/// @brief Remove all elements from a linked list.
/// @param list The linked list.
/// @return The exit status of the function:
/// - SUCCESS:                      list was cleared successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
int ioopm_linked_list_clear (ioopm_list_t *list);

/// @brief Test if a supplied property holds for all elements in a list.
///        The function returns as soon as the return value can be determined.
/// @param list The linked list.
/// @param prop The property to be tested (function pointer to a predicate
/// function).
/// @param extra An additional argument (may be NULL) that will be passed to
/// all internal calls of prop.
/// @param return_all The result is returned via this parameter.
///                   true if prop holds for all elements in the list, else
///                   false
/// @return The exit status of the function:
/// - SUCCESS:                      check if predicate was valid for all
/// elements successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
int ioopm_linked_list_all (bool *return_all, const ioopm_list_t *list,
                           ioopm_predicate *prop, const void *extra);

/// @brief Test if a supplied property holds for any element in a list.
///        The function returns as soon as the return value can be determined.
/// @param list The linked list.
/// @param prop The property to be tested.
/// @param extra An additional argument (may be NULL) that will be passed to
/// all internal calls of prop.
/// @param return_any The functions result is returned via this parameter.
///                   true if prop holds for any element in the list, else
///                   false
/// @return The exit status of the function:
/// - SUCCESS:                      check if predicate was valid on any
/// elements successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
int ioopm_linked_list_any (bool *return_any, const ioopm_list_t *list,
                           ioopm_predicate *prop, const void *extra);

/// @brief Apply a supplied function to all elements in a list.
/// @param list The linked list.
/// @param fun The function to be applied.
/// @param extra an additional argument (may be NULL) that will be passed to
/// all internal calls of fun.
/// @return The exit status of the function:
/// - SUCCESS:                      function was applied to all elements
/// successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
int ioopm_linked_list_apply_to_all (ioopm_list_t *list,
                                    ioopm_apply_function *fun, void *extra);

/// @brief Create an iterator for a given list.
/// @param return_iter A pointer to an iterator positioned at the start of
/// list.
/// @param list The list to be iterated over.
/// @return The exit status of the function:
/// - SUCCESS:                      list iterator created successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_iter is NULL (invalid pointer)
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during creation
int ioopm_list_iterator (ioopm_iterator_t **return_iter, ioopm_list_t *list);

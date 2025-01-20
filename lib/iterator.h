/**
 * @file iterator.h
 * @author Hampus Toft & Vilja Schnell Melander
 * @date 17 september 2024
 * @brief iteration interface used by linked_list.c
 */

#pragma once
#include <stdbool.h>

#include "common.h"

#define INVALID_ITERATOR_POINTER 31
#define INVALID_ITERATOR_STATE 32
#define ITERATOR_IS_EMPTY 33
#define ITERATOR_OPERATION_UNSUPPORTED 34

typedef struct iterator ioopm_iterator_t;

typedef int (iterator_has_next_func) (bool *return_has_next,
                                      const void *underlying_data_structure);
typedef int (iterator_next_func) (elem_t *return_next,
                                  void *underlying_data_structure);
typedef int (iterator_current_func) (elem_t *return_current,
                                     const void *underlying_data_structure);
typedef int (iterator_remove_func) (elem_t *return_removed,
                                    void *underlying_data_structure);
typedef int (iterator_insert_func) (void *underlying_data_structure,
                                    const elem_t element);
typedef int (iterator_reset_func) (void *underlying_data_structure);
typedef int (iterator_destroy_func) (void *underlying_data_structure);

/// @brief Creates a ioopm_iterator_t from some data structure using given
/// functions to handel that data structure. this function should be used by
/// data structures to create a generic iterator. users should use data
/// structures create iterator function.
///
/// NOTE: this is only to allow your custom data structure to create a generic
/// iterator and NOT to create a iterator from some unsupported data structure.
///
/// Example Usage If you want to support iteration over your custom data
/// structure:
///
/// `ioopm_iterator_create(return_iter, data_struct, has_next, next, current,
/// NULL, NULL, reset, destroy)`
///
/// `// Create a iterator that uses custom functions to iterate over the custom
/// data structure`
///
/// @example USe asbasd
/// @param return_iter a iterator using the given data structure and iteration
/// functions.
/// @param underlying_data_structure some data structure in which the functions
/// operate upon.
/// @param has_next a function that checks if iterator has a next element.
/// @param next a function that steps the iterator forward (must be used after
/// remove, insert, reset, create).
/// @param current a function to fetch current element from the iterator.
/// @param remove a function to remove previously returned element from the
/// iterator.
/// @param insert a function to insert a element in the iterator.
/// Insertion occurs after previously returned element.
/// @param reset a function to restart the iterator, set to its initial state
/// (dose not undo inserts / removals).
/// @param destroy a function to destroys the iterator, needs to destroy
/// underlying structure.
/// @return The exit status of the function:
/// - SUCCESS:                      function call successful
/// - INVALID_ITERATOR_POINTER:     iter is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_has_next is NULL (invalid pointer)
int ioopm_iterator_create (
    ioopm_iterator_t **return_iter, void *underlying_data_structure,
    iterator_has_next_func *has_next, iterator_next_func *next,
    iterator_current_func *current, iterator_remove_func *remove,
    iterator_insert_func *insert, iterator_reset_func *reset,
    iterator_destroy_func *destroy);

/// @brief Check if there are more elements to iterate over.
/// @param return_has_next The result as to whether there is a next element is
/// returned via this parameter.
/// @param iter The iterator.
/// @return The exit status of the function:
/// - SUCCESS:                          function call successful
/// - INVALID_ITERATOR_POINTER:         iter is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:           return_has_next is NULL (invalid
/// pointer)
/// - ITERATOR_OPERATION_UNSUPPORTED:   the given iterator dose not support
/// this function
int ioopm_iterator_has_next (bool *return_has_next,
                             const ioopm_iterator_t *iter);

/// @brief Step the iterator forward one step.
/// @param return_next The next element in the iterator is returned via this
/// parameter.
/// @param iter The iterator.
/// @return The exit status of the function:
/// - SUCCESS:                          next element retrived successfuly
/// - INVALID_ITERATOR_POINTER:         iter is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:           return_next is NULL (invalid pointer)
/// - ITERATOR_IS_EMPTY:                iterator is empty
/// - ITERATOR_OPERATION_UNSUPPORTED:   the given iterator dose not support
/// this function
int ioopm_iterator_next (elem_t *return_next, ioopm_iterator_t *iter);

/// @brief Return the current element from the underlying list.
/// @param return_current The current element is returned via this parameter.
/// @param iter The iterator.
/// @return The exit status of the function:
/// - SUCCESS:                          current elem_t retrived successfuly
/// - INVALID_ITERATOR_POINTER:         iter is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:           return_current is NULL (invalid
/// pointer)
/// - INVALID_ITERATOR_STATE:           iterator is not in a vaild state to
/// retrive current elem_t from
/// - ITERATOR_OPERATION_UNSUPPORTED:   the given iterator dose not support
/// this function
int ioopm_iterator_current (elem_t *return_current,
                            const ioopm_iterator_t *iter);

/// @brief Remove the current element from the underlying list.
/// @param return_removed The removed element is returned via this parameter.
/// @param iter the iterator
/// @return The exit status of the function:
/// - SUCCESS:                          elem_t removed successfuly
/// - INVALID_ITERATOR_POINTER:         iter is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:           return_removed is NULL (invalid
/// pointer)
/// - INVALID_ITERATOR_STATE:           iterator is not in a vaild state to
/// remove a elem_t
/// - ITERATOR_OPERATION_UNSUPPORTED:   the given iterator dose not support
/// this function
int ioopm_iterator_remove (elem_t *return_removed, ioopm_iterator_t *iter);

/// @brief Insert a new element into the underlying list making the current
/// element it's next.
/// @param iter The iterator.
/// @param element The element to be inserted.
/// @return The exit status of the function:
/// - SUCCESS:                          elem_t inserted successfuly
/// - INVALID_ITERATOR_POINTER:         iter is NULL (invalid pointer)
/// - INVALID_ITERATOR_STATE:           iterator is not in a vaild state to
/// insert a elem_t
/// - MEMORY_ALLOCATION_FAILURE:        memory allocation failed during node
/// creation
/// - ITERATOR_OPERATION_UNSUPPORTED:   the given iterator dose not support
/// this function
int ioopm_iterator_insert (ioopm_iterator_t *iter, const elem_t element);

/// @brief Reposition the iterator at the start of the underlying list.
/// @param iter The iterator.
/// @return The exit status of the function:
/// - SUCCESS:                          iter reset successfuly
/// - INVALID_ITERATOR_POINTER:         iter is NULL (invalid pointer)
/// - ITERATOR_OPERATION_UNSUPPORTED:   the given iterator dose not support
/// this function
int ioopm_iterator_reset (ioopm_iterator_t *iter);

/// @brief Destroy the iterator and return its resources
/// @param iter pointer to the iterator to destroy. Sets iterator pointer to
/// NULL if successful
/// @return the exit status of the function:
/// - SUCCESS:                          elem_t removed successfuly
/// - INVALID_ITERATOR_POINTER:         iter is NULL (invalid pointer)
/// - ITERATOR_OPERATION_UNSUPPORTED:   the given iterator dose not support
/// this function
int ioopm_iterator_destroy (ioopm_iterator_t **iter);

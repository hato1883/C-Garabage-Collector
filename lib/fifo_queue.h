/**
 * @file queue.h
 * @author Hampus Toft & Vilja Schnell Melander
 * @date 17 september 2024
 * @brief queue structure that can hold any generic data in a homogeneous queue
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "common.h"
#include "iterator.h"

#define INVALID_QUEUE_POINTER 41
#define QUEUE_IS_EMPTY 42

typedef struct queue ioopm_queue_t;

/// @brief Create a new empty queue.
/// @param return_queue The created, empty queue is returned via this
/// parameter.
/// @return The exit status of the function:
/// - SUCCESS:                      queue created successfuly
/// - INVALID_RETURN_POINTER:       return_queue is NULL (invalid pointer)
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during creation
int ioopm_queue_create (ioopm_queue_t **return_queue);

/// @brief Destroy a queue.
/// @param queue pointer to the queue to destroy. Sets queue pointer to NULL if
/// successful
/// @return the exit status of the function:
/// - SUCCESS:                      queue destroyed successfuly
/// - INVALID_QUEUE_POINTER:         queue is NULL (invalid pointer)
int ioopm_queue_destroy (ioopm_queue_t **queue);

/// @brief places element last in queue in O(1) time.
/// @param queue The queue that element will be enqueued in.
/// @param element The element to enqueue.
/// @return The exit status of the function:
/// - SUCCESS:                      element enqueued successfuly
/// - INVALID_QUEUE_POINTER:        queue is NULL (invalid pointer)
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during node
/// creation
int ioopm_queue_enqueue (ioopm_queue_t *queue, const elem_t element);

/// @brief Retrieve the first element in the queue in O(1) time.
/// @param return_element The element is returned via this parameter.
/// @param queue The queue to peek in.
/// @return The exit status of the function:
/// - SUCCESS:                      retrived the element at index successfuly
/// - INVALID_QUEUE_POINTER:        queue is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_element is NULL (invalid pointer)
/// - QUEUE_IS_EMPTY
int ioopm_queue_peek (elem_t *return_element, ioopm_queue_t *queue);

/// @brief Retrieve and remove the first element in the queue in O(1) time.
/// @param return_element The element is returned via this parameter.
/// @param queue The queue to dequeu the first element from.
/// @return The exit status of the function:
/// - SUCCESS:                      retrived the element at index successfuly
/// - INVALID_QUEUE_POINTER:        queue is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_element is NULL (invalid pointer)
/// - QUEUE_IS_EMPTY
int ioopm_queue_dequeue (elem_t *return_element, ioopm_queue_t *queue);

/// @brief Lookup the number of elements in the queue in O(1) time.
/// @param return_size The number of elements in the queue is returned via this
/// parameter.
/// @param queue The queue.
/// @return The exit status of the function:
/// - SUCCESS:                      retrived size of queue successfuly
/// - INVALID_QUEUE_POINTER:         queue is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_size is NULL (invalid pointer)
int ioopm_queue_size (size_t *return_size, const ioopm_queue_t *queue);

/// @brief Test whether a queue is empty or not
/// @param return_empty a bool indicating whether or not the queue is empty is
/// returned via this parameter
/// @param queue the queue
///              true if the number of elements int the queue is 0, else false
/// @return the exit status of the function:
/// - SUCCESS:                      function call was successful
/// - INVALID_QUEUE_POINTER:         queue is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_empty is NULL (invalid pointer)
int ioopm_queue_is_empty (bool *return_empty, const ioopm_queue_t *queue);

/// @brief Remove all elements from a queue.
/// @param queue The queue.
/// @return The exit status of the function:
/// - SUCCESS:                      queue was cleared successfuly
/// - INVALID_QUEUE_POINTER:         queue is NULL (invalid pointer)
int ioopm_queue_clear (ioopm_queue_t *queue);

/// @brief Create an iterator for a given queue.
/// @param return_iter A pointer to an iterator positioned at the start of
/// queue.
/// @param queue The queue to be iterated over.
/// @return The exit status of the function:
/// - SUCCESS:                      queue iterator created successfuly
/// - INVALID_QUEUE_POINTER:         queue is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_iter is NULL (invalid pointer)
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during creation
int ioopm_queue_iterator (ioopm_iterator_t **return_iter,
                          ioopm_queue_t *queue);

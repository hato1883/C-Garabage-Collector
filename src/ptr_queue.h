/**
 * Creation of a queue used to store visited pointers during the mark-phase
 */

#pragma once
#include <stdbool.h>
#include <stdlib.h>

/**
 * A priority queue for pointers, where the smallest pointer is first in the
 * queue. Does not contain any duplicates.
 */
typedef struct ptr_queue ptr_queue_t;

/**
 * @brief Creates an empty pointer queue.
 * @return a pointer to the created queue
 */
ptr_queue_t *create_ptr_queue (void);

/**
 * @brief Frees all memory allocated to a pointer queue
 * @param queue the pointer queue to destroy
 */
void destroy_ptr_queue (ptr_queue_t *queue);

/**
 * @brief Enqueues a pointer in a pointer queue, if it is not already in the
 * queue
 * @param queue a pointer queue
 * @param ptr a pointer to enqueue
 * @note ptr cannot be NULL
 * @return true if the pointer was enqueued, false otherwise (if the pointer
 * was already in the queue)
 */
bool enqueue_ptr (ptr_queue_t *queue, void *ptr);

/**
 * @brief Removes and returns the smallest pointer from a pointer queue
 * @param queue a pointer queue
 * @return the pointer removed from the queue, which will have been the
 *  smallest, or NULL if the queue is empty
 */
void **dequeue_ptr (ptr_queue_t *queue);

/**
 * @brief gets value stored in the n'th position in the queue
 * @param q queue to get value from
 * @param index the nth index to retrive from
 * @return NULL if index is out of bounds of queue, or the queue is empty.
 * Otherwise the pointer at given index.
 */
void **get_ptr (ptr_queue_t *q, size_t index);

/**
 * @brief get the length of the queue.
 * @param q queue to check length of
 * @return length
 */
size_t get_len (ptr_queue_t *q);

/**
 * @brief Checks if a pointer queue is empty
 * @return true if the queue is empty, false otherwise
 */
bool is_empty_queue (ptr_queue_t *queue);

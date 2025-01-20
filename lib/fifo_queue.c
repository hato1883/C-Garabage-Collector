#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "../src/gc.h"
#include "common.h"
#include "fifo_queue.h"
#include "iterator.h"

typedef struct node node_t;
struct node
{
  elem_t entry; // An entry
  node_t *next; // Pointer to the next node in the queue
};

struct queue
{
  node_t *first; // pointer to head of the queue
  node_t *last;  // pointer to last link in the queue
  size_t size;   // the number of links in the queue
};

static void initialize_queue (ioopm_queue_t **queue);
static int create_node (node_t **new_node, const elem_t entry,
                        const node_t *next);

/////////////////////////////////////////////////
///////////         QUEUE        ////////////////
/////////////////////////////////////////////////

int
ioopm_queue_create (ioopm_queue_t **return_queue)
{
  // Check presumptions
  if (return_queue == NULL)
    {
      return INVALID_QUEUE_POINTER;
    }

  *return_queue = h_alloc_struct (global_heap, "**l");

  if (*return_queue == NULL)
    {
      return MEMORY_ALLOCATION_FAILURE;
    }

  initialize_queue (return_queue);

  return SUCCESS;
}

int
ioopm_queue_destroy (ioopm_queue_t **queue)
{
  // Check presumptions
  if (queue == NULL || *queue == NULL)
    {
      return INVALID_QUEUE_POINTER;
    }

  // Iterate through queue, freeing each node
  ioopm_queue_clear (*queue);

  // Free queue
  // free (*queue);
  *queue = NULL;

  return SUCCESS;
}

int
ioopm_queue_enqueue (ioopm_queue_t *queue, const elem_t element)
{
  // Check presumptions
  if (queue == NULL)
    {
      return INVALID_QUEUE_POINTER;
    }
  else
    {
      // update last element to point to new link.

      // create new link
      node_t *new_node = NULL;
      if (create_node (&new_node, element, NULL) != SUCCESS)
        {
          return MEMORY_ALLOCATION_FAILURE;
        }

      // get old last link
      node_t *old_last_link = queue->last;

      if (old_last_link) // Not NULL
        {
          // update last link to point to new link
          old_last_link->next = new_node;
        }
      else // Is NULL
        {
          queue->first = new_node;
        }

      queue->last = new_node;

      queue->size += 1;
      return SUCCESS;
    }
}

int
ioopm_queue_peek (elem_t *return_get, ioopm_queue_t *queue)
{
  // Check presumptions
  if (queue == NULL)
    {
      return INVALID_QUEUE_POINTER;
    }
  if (return_get == NULL)
    {
      return INVALID_RETURN_POINTER;
    }
  if (queue->first == NULL)
    {
      return QUEUE_IS_EMPTY;
    }

  *return_get = queue->first->entry;
  return SUCCESS;
}

int
ioopm_queue_dequeue (elem_t *return_get, ioopm_queue_t *queue)
{
  // Check presumptions
  if (queue == NULL)
    {
      return INVALID_QUEUE_POINTER;
    }
  if (return_get == NULL)
    {
      return INVALID_RETURN_POINTER;
    }
  if (queue->first == NULL)
    {
      return QUEUE_IS_EMPTY;
    }

  // Removing first link
  node_t *to_unlink = queue->first;
  *return_get = to_unlink->entry;
  if (queue->size > 1)
    {
      // We have more links,
      // Replace first link with the next link
      queue->first = to_unlink->next;
    }
  else
    {
      // No more links in queue,
      // Set to NULL
      queue->first = NULL;
      queue->last = NULL;
    }
  // free (to_unlink);
  queue->size -= 1;

  return SUCCESS;
}

int
ioopm_queue_size (size_t *return_size, const ioopm_queue_t *queue)
{
  // Check presumptions
  if (queue == NULL)
    {
      return INVALID_QUEUE_POINTER;
    }
  if (return_size == NULL)
    {
      return INVALID_RETURN_POINTER;
    }

  *return_size = queue->size;

  return SUCCESS;
}

int
ioopm_queue_is_empty (bool *return_empty, const ioopm_queue_t *queue)
{
  // Check if we have valid return adress
  if (return_empty == NULL)
    {
      return INVALID_RETURN_POINTER;
    }

  // try to get size
  size_t size = 0;
  int status = ioopm_queue_size (&size, queue);
  if (status == SUCCESS)
    {
      // We got size, set return value
      *return_empty = size == 0;
      return SUCCESS;
    }
  // Something was wrong
  // Propagate the error:
  // INVALID_QUEUE_POINTER
  // INVALID_RETURN_POINTER
  return status;
}

int
ioopm_queue_clear (ioopm_queue_t *queue)
{
  // Check presumptions
  if (queue == NULL)
    {
      return INVALID_QUEUE_POINTER;
    }

  // Iterate through queue, freeing each node
  node_t *current_node = queue->first;
  node_t *next_node = NULL;
  while (current_node != NULL)
    {
      next_node = current_node->next;
      // free (current_node);
      current_node = next_node;
    }

  // Reset fields
  queue->first = NULL;
  queue->last = NULL;
  queue->size = 0;

  return SUCCESS;
}

/// @brief Initializes the fields in a queue.
/// @param queue The queue to be initialized.
/// @param queue The queue's eqivalence function.
static void
initialize_queue (ioopm_queue_t **queue)
{
  (*queue)->first = NULL;
  (*queue)->last = NULL;
  (*queue)->size = 0;
}

/// @brief Creates a node in a queue.
/// @param return_new_node A pointer to the newly created node_t * that points
/// to our link is returned via this parameter.
/// @param entry The node entry.
/// @param next The next link to point to.
/// @return The exit status of the function:
/// - SUCCESS:                      node created successfuly
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during creation
static int
create_node (node_t **return_new_node, const elem_t entry, const node_t *next)
{
  *return_new_node = h_alloc_struct (global_heap, "**");
  if (*return_new_node == NULL)
    {
      return MEMORY_ALLOCATION_FAILURE;
    }

  **return_new_node = (node_t){ .entry = entry, .next = (node_t *)next };
  return SUCCESS;
}

/////////////////////////////////////////////////
///////////       ITERATOR       ////////////////
/////////////////////////////////////////////////

#define cast_to_queue_iter(x) ((queue_iterator_t *)x)

typedef struct queue_iterator queue_iterator_t;
struct queue_iterator
{
  node_t *current;
  node_t *next;
  ioopm_queue_t *queue;
};

static bool is_valid (queue_iterator_t *iter);
static int initialize_iterator (ioopm_iterator_t **iter, ioopm_queue_t *queue);
static int initialize_queue_iterator (queue_iterator_t **queue_iter,
                                      ioopm_queue_t *queue);

int
queue_iterator_has_next (bool *return_has_next,
                         const void *internal_data_struct)
{
  queue_iterator_t *queue_iter = cast_to_queue_iter (internal_data_struct);

  *return_has_next = queue_iter->next != NULL;

  return SUCCESS;
}

int
queue_iterator_next (elem_t *return_next, void *internal_data_struct)
{
  queue_iterator_t *queue_iter = cast_to_queue_iter (internal_data_struct);

  bool has_next = false;
  if (queue_iterator_has_next (&has_next, queue_iter) == SUCCESS && has_next)
    {
      queue_iter->current = queue_iter->next;
      queue_iter->next = queue_iter->next->next;

      // return new current element
      *return_next = queue_iter->current->entry;
    }
  else
    {
      // Error handling when calling next on a iterator that is empty
      return ITERATOR_IS_EMPTY;
    }
  return SUCCESS;
}

int
queue_iterator_current (elem_t *return_current,
                        const void *internal_data_struct)
{
  queue_iterator_t *queue_iter = cast_to_queue_iter (internal_data_struct);

  if (is_valid (queue_iter))
    {
      // Return the current item of the iterator
      *return_current = queue_iter->current->entry;
      return SUCCESS;
    }
  else
    {
      //    User has not called ioopm_iterator_next after using
      //    ioopm_queue_iterator (Creation)
      // OR User has not called ioopm_iterator_next after using
      // ioopm_iterator_reset (Reset) OR User has not called
      // ioopm_iterator_next after using ioopm_iterator_remove (Removal of
      // element) OR User has not called ioopm_iterator_next after using
      // ioopm_iterator_insert (Insertion of element)
      return INVALID_ITERATOR_STATE;
    }
}

int
queue_iterator_reset (void *internal_data_struct)
{
  queue_iterator_t *queue_iter = cast_to_queue_iter (internal_data_struct);

  queue_iter->current = NULL;
  queue_iter->next = queue_iter->queue->first;

  return SUCCESS;
}

int
queue_iterator_destroy (void *internal_data_struct)
{
  queue_iterator_t **queue_iter = ((queue_iterator_t **)internal_data_struct);

  // free (*queue_iter);
  *queue_iter = NULL;

  return SUCCESS;
}

int
ioopm_queue_iterator (ioopm_iterator_t **return_iter, ioopm_queue_t *queue)
{
  if (return_iter == NULL)
    {
      return INVALID_RETURN_POINTER;
    }
  if (queue == NULL)
    {
      return INVALID_QUEUE_POINTER;
    }

  int iter_init_status = initialize_iterator (return_iter, queue);
  return iter_init_status;
}

/// @brief Check if the iterator is in a valid state for retrival or changes.
/// @param iter The iterator to check if it has a elem_t in current.
/// @return true if the current node_t is not a NULL pointer
static bool
is_valid (queue_iterator_t *iter)
{
  return iter->current != NULL;
}

/// @brief Initializes the fields in an iterator.
/// @param queue The iterator to be initialized.
/// @param queue The queue that the iterator is based upon.
static int
initialize_iterator (ioopm_iterator_t **return_iter, ioopm_queue_t *queue)
{
  queue_iterator_t *queue_iter = NULL;

  int queue_itr_init_status = initialize_queue_iterator (&queue_iter, queue);
  if (queue_itr_init_status != SUCCESS)
    {
      // Something failed when initializing our queue iterator
      // Propagate to caller
      return queue_itr_init_status;
    }

  int itr_init_status = ioopm_iterator_create (
      return_iter, queue_iter, queue_iterator_has_next, queue_iterator_next,
      queue_iterator_current, NULL, NULL, queue_iterator_reset,
      queue_iterator_destroy);
  if (itr_init_status == SUCCESS)
    {
      return SUCCESS;
    }
  else
    {
      // Something failed when creating our general iterator
      // free (queue_iter);
      return itr_init_status;
    }
}

/// @brief Initializes the fields in an queue iterator.
/// @param queue The iterator to be initialized.
/// @param queue The queue that the iterator is based upon.
static int
initialize_queue_iterator (queue_iterator_t **queue_iter, ioopm_queue_t *queue)
{
  *queue_iter = h_alloc_struct (global_heap, "***");
  if (*queue_iter == NULL)
    {
      return MEMORY_ALLOCATION_FAILURE;
    }

  (*queue_iter)->current = NULL;
  (*queue_iter)->next = queue->first;
  (*queue_iter)->queue = queue;

  return SUCCESS;
}

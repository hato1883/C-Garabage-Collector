#include <stdlib.h>

#include "../src/gc.h"
#include "iterator.h"

#define ERROR_ELEM_T = ptr_to_elem (NULL)

struct iterator
{
  void *data_structure;
  iterator_has_next_func *has_next;
  iterator_next_func *next;
  iterator_current_func *current;
  iterator_remove_func *remove;
  iterator_insert_func *insert;
  iterator_reset_func *reset;
  iterator_destroy_func *destroy;
};

// Macro to check if iter or the function is null
#define check_preconditions(status, iter, func)                               \
  int status = SUCCESS;                                                       \
  if (iter == NULL)                                                           \
    {                                                                         \
      status = INVALID_ITERATOR_POINTER;                                      \
    }                                                                         \
  else if (func == NULL)                                                      \
    {                                                                         \
      status = ITERATOR_OPERATION_UNSUPPORTED;                                \
    }                                                                         \
  else                                                                        \
    {                                                                         \
      status = SUCCESS;                                                       \
    }

int
ioopm_iterator_create (
    ioopm_iterator_t **return_iter, void *underlying_data_structure,
    iterator_has_next_func *has_next, iterator_next_func *next,
    iterator_current_func *current, iterator_remove_func *remove,
    iterator_insert_func *insert, iterator_reset_func *reset,
    iterator_destroy_func *destroy)
{
  ioopm_iterator_t *iter_pointer = h_alloc_struct (global_heap, "*7l");
  if (iter_pointer != NULL)
    {
      iter_pointer->data_structure = underlying_data_structure;
      iter_pointer->has_next = has_next;
      iter_pointer->next = next;
      iter_pointer->current = current;
      iter_pointer->remove = remove;
      iter_pointer->insert = insert;
      iter_pointer->reset = reset;
      iter_pointer->destroy = destroy;
      *return_iter = iter_pointer;
      return SUCCESS;
    }
  else
    {
      return MEMORY_ALLOCATION_FAILURE;
    }
}

int
ioopm_iterator_has_next (bool *return_has_next, const ioopm_iterator_t *iter)
{
  check_preconditions (status, iter, iter->has_next);
  if (return_has_next == NULL)
    return INVALID_RETURN_POINTER;
  if (status == SUCCESS)
    {
      return iter->has_next (return_has_next, iter->data_structure);
    }
  // Propagate Error
  return status;
}

int
ioopm_iterator_next (elem_t *return_next, ioopm_iterator_t *iter)
{
  check_preconditions (status, iter, iter->next);
  if (return_next == NULL)
    return INVALID_RETURN_POINTER;
  if (status == SUCCESS)
    {
      return iter->next (return_next, iter->data_structure);
    }
  // Propagate Error
  return status;
}

int
ioopm_iterator_current (elem_t *return_current, const ioopm_iterator_t *iter)
{
  check_preconditions (status, iter, iter->current);
  if (return_current == NULL)
    return INVALID_RETURN_POINTER;
  if (status == SUCCESS)
    {
      return iter->current (return_current, iter->data_structure);
    }
  // Propagate Error
  return status;
}

int
ioopm_iterator_remove (elem_t *return_removed, ioopm_iterator_t *iter)
{
  check_preconditions (status, iter, iter->remove);
  if (return_removed == NULL)
    return INVALID_RETURN_POINTER;
  if (status == SUCCESS)
    {
      return iter->remove (return_removed, iter->data_structure);
    }
  // Propagate Error
  return status;
}

int
ioopm_iterator_insert (ioopm_iterator_t *iter, const elem_t element)
{
  check_preconditions (status, iter, iter->insert);
  if (status == SUCCESS)
    {
      return iter->insert (iter->data_structure, element);
    }
  // Propagate Error
  return status;
}

int
ioopm_iterator_reset (ioopm_iterator_t *iter)
{
  check_preconditions (status, iter, iter->reset);
  if (status == SUCCESS)
    {
      return iter->reset (iter->data_structure);
    }
  // Propagate Error
  return status;
}

int
ioopm_iterator_destroy (ioopm_iterator_t **iter_ptr_ptr)
{
  ioopm_iterator_t *iter_ptr = *iter_ptr_ptr;
  check_preconditions (status, iter_ptr, iter_ptr->destroy);
  if (status == SUCCESS)
    {
      status = iter_ptr->destroy (&(iter_ptr->data_structure));
      // Note that user should call with argument &iter meaning that setting it
      // to null will hopefully cause crashes instead of hidden bugs
      // free (iter_ptr);
      *iter_ptr_ptr = NULL;

      return status;
    }
  // if destroy is not supported there will be a memory leak,
  // nothing much we can do about it.
  // Propagate Error
  return status;
}

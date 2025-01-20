#include <stdlib.h>

#include "../src/gc.h"
#include "common.h"
#include "iterator.h"
#include "linked_list.h"

typedef struct node node_t;
struct node
{
  elem_t entry; // An entry
  node_t *next; // Pointer to the next node in the linked list
};

struct list
{
  node_t *first;              // pointer to head of the linked list
  node_t *last;               // pointer to last link in the linked list
  ioopm_eq_function *eq_func; // function used to compare stored values with
  size_t size;                // the number of links in the list
};

static void initialize_list (ioopm_list_t **list, ioopm_eq_function *eq_func);
static int create_node (node_t **new_node, const elem_t entry,
                        const node_t *next);
static node_t **get_ptr_to_index (node_t **head, const int index);
static node_t **get_ptr_before_index (node_t **head, const int index);
static int insert_at_index (ioopm_list_t *list, const size_t index,
                            const elem_t element, size_t size);
static int insert_at_head (ioopm_list_t *list, const elem_t element,
                           size_t size);
static void remove_at_index (elem_t *return_removed, ioopm_list_t *list,
                             size_t size, const size_t index);
static void remove_at_head (elem_t *return_removed, ioopm_list_t *list,
                            size_t size);

/////////////////////////////////////////////////
///////////     LINKED LIST      ////////////////
/////////////////////////////////////////////////

int
ioopm_linked_list_create (ioopm_list_t **return_list,
                          ioopm_eq_function *eq_func)
{
  // Check presumptions
  if (return_list == NULL)
    {
      return INVALID_LIST_POINTER;
    }

#ifdef linked_list_use_malloc
  *return_list = calloc (1, sizeof (ioopm_list_t));
#else
  *return_list = h_alloc_struct (global_heap, "**ll");
#endif

  if (*return_list == NULL)
    {
      return MEMORY_ALLOCATION_FAILURE;
    }

  initialize_list (return_list, eq_func);

  return SUCCESS;
}

int
ioopm_linked_list_destroy (ioopm_list_t **list)
{
  // Check presumptions
  if (list == NULL || *list == NULL)
    {
      return INVALID_LIST_POINTER;
    }

  // Iterate through linked list, freeing each node
  ioopm_linked_list_clear (*list);

// Free linked list
#ifdef linked_list_use_malloc
  free (*list);
#endif
  *list = NULL;

  return SUCCESS;
}

int
ioopm_linked_list_append (ioopm_list_t *list, const elem_t element)
{
  size_t size = 0;
  int status = ioopm_linked_list_size (&size, list);

  if (status == SUCCESS)
    {
      return ioopm_linked_list_insert (list, size, element);
    }

  return status;
}

int
ioopm_linked_list_prepend (ioopm_list_t *list, const elem_t element)
{
  return ioopm_linked_list_insert (list, 0, element);
}

int
ioopm_linked_list_insert (ioopm_list_t *list, const size_t index,
                          const elem_t element)
{
  // Check presumptions
  if (list == NULL)
    {
      return INVALID_LIST_POINTER;
    }

  size_t size = 0;
  ioopm_linked_list_size (&size, list);
  if (index > size)
    {
      return INDEX_OUT_OF_BOUNDS;
    }

  // Want to insert into index 0 which is the list->first node pointer
  if (index > 0)
    {
      int status = insert_at_index (list, index, element, size);
      if (status != SUCCESS)
        {
          return status;
        }
    }
  else
    {
      int status = insert_at_head (list, element, size);
      if (status != SUCCESS)
        {
          return status;
        }
    }

  list->size += 1;

  return SUCCESS;
}

int
ioopm_linked_list_remove (elem_t *return_removed, ioopm_list_t *list,
                          const size_t index)
{
  // Check presumptions
  if (list == NULL)
    {
      return INVALID_LIST_POINTER;
    }
  if (return_removed == NULL)
    {
      return INVALID_RETURN_POINTER;
    }

  size_t size = 0;
  ioopm_linked_list_size (&size, list);
  // Check if index is out of bounds
  if (index >= size)
    {
      return INDEX_OUT_OF_BOUNDS;
    }

  if (index > 0)
    {
      remove_at_index (return_removed, list, size, index);

      // Points to a pointer to the previous node
      return SUCCESS;
    }
  else
    {
      remove_at_head (return_removed, list, size);

      return SUCCESS;
    }
}

int
ioopm_linked_list_get (elem_t *return_get, ioopm_list_t *list,
                       const size_t index)
{
  // Check presumptions
  if (list == NULL)
    {
      return INVALID_LIST_POINTER;
    }
  if (return_get == NULL)
    {
      return INVALID_RETURN_POINTER;
    }

  size_t size = 0;
  ioopm_linked_list_size (&size, list);
  if (index >= size)
    {
      return INDEX_OUT_OF_BOUNDS;
    }

  // Points to a pointer to the previous node
  // Will never return NULL due to guard condition in line 250-253
  node_t **prev = get_ptr_to_index (&(list->first), index);

  *return_get = (*prev)->entry;
  return SUCCESS;
}

int
ioopm_linked_list_contains (bool *return_contains, const ioopm_list_t *list,
                            const elem_t element)
{
  // Check presumptions
  if (list == NULL)
    {
      return INVALID_LIST_POINTER;
    }
  if (return_contains == NULL)
    {
      return INVALID_RETURN_POINTER;
    }

  // Linearly search linked list
  node_t *node = list->first;
  while (node != NULL)
    {
      if (list->eq_func (node->entry, element))
        {
          *return_contains = true; // Key found
          return SUCCESS;
        }

      node = node->next; // Go to next node
    }
  *return_contains = false; // Key not found
  return SUCCESS;
}

size_t
ioopm_linked_list_size (size_t *return_size, const ioopm_list_t *list)
{
  // Check presumptions
  if (list == NULL)
    {
      return INVALID_LIST_POINTER;
    }
  if (return_size == NULL)
    {
      return INVALID_RETURN_POINTER;
    }

  *return_size = list->size;

  return SUCCESS;
}

int
ioopm_linked_list_is_empty (bool *return_empty, const ioopm_list_t *list)
{
  // Check if we have valid return adress
  if (return_empty == NULL)
    {
      return INVALID_RETURN_POINTER;
    }

  // try to get size
  size_t size = 0;
  int status = ioopm_linked_list_size (&size, list);
  if (status == SUCCESS)
    {
      // We got size, set return value
      *return_empty = size == 0;
      return SUCCESS;
    }
  // Something was wrong
  // Propagate the error:
  // INVALID_LIST_POINTER
  // INVALID_RETURN_POINTER
  return status;
}

int
ioopm_linked_list_clear (ioopm_list_t *list)
{
  // Check presumptions
  if (list == NULL)
    {
      return INVALID_LIST_POINTER;
    }

  // Iterate through linked list, freeing each node
  node_t *node = list->first;
  node_t *next_node = NULL;
  while (node != NULL)
    {
      next_node = node->next;
#ifdef linked_list_use_malloc
      free (node);
#endif
      node = next_node;
    }

  // Reset fields
  list->first = NULL;
  list->last = NULL;
  list->size = 0;

  return SUCCESS;
}

typedef bool operator_function (bool result, bool predicate_result);

int
ioopm_linked_list_general_iterate (operator_function *operator,
                                   bool * to_return, const ioopm_list_t *list,
                                   ioopm_predicate *prop, const void *extra)
{
  // Check presumptions
  if (list == NULL)
    {
      return INVALID_LIST_POINTER;
    }
  if (to_return == NULL)
    {
      return INVALID_RETURN_POINTER;
    }
  node_t *node = list->first;

  bool result = !operator(true, false);
  while (node != NULL && !operator(result, false))
    {
      result = operator(result, prop (node->entry, extra));
      node = node->next;
    }

  *to_return = result;
  return SUCCESS;
}

bool
logical_or (bool result, bool predicate_result)
{
  return result || predicate_result;
}

int
ioopm_linked_list_any (bool *return_any, const ioopm_list_t *list,
                       ioopm_predicate *prop, const void *extra)
{
  return ioopm_linked_list_general_iterate (logical_or, return_any, list, prop,
                                            extra);
}

bool
logical_and (bool result, bool predicate_result)
{
  return result && predicate_result;
}

int
ioopm_linked_list_all (bool *return_all, const ioopm_list_t *list,
                       ioopm_predicate *prop, const void *extra)
{
  return ioopm_linked_list_general_iterate (logical_and, return_all, list,
                                            prop, extra);
}

int
ioopm_linked_list_apply_to_all (ioopm_list_t *list, ioopm_apply_function *fun,
                                void *extra)
{
  // Check presumptions
  if (list == NULL)
    {
      return INVALID_LIST_POINTER;
    }

  node_t *node_ptr = list->first;
  while (node_ptr != NULL)
    {
      // Save pointer to next before calling function
      // This allows the function destroy node without issue.
      node_t *next = node_ptr->next;
      fun (&(node_ptr->entry), extra);
      // Set next to be the new current
      node_ptr = next;
    }

  return SUCCESS;
}

/// @brief Initializes the fields in a linked list.
/// @param list The list to be initialized.
/// @param list The list's eqivalence function.
static void
initialize_list (ioopm_list_t **list, ioopm_eq_function *eq_func)
{
  (*list)->first = NULL;
  (*list)->last = NULL;
  (*list)->size = 0;
  (*list)->eq_func = eq_func;
}

/// @brief Creates a node in a linked list.
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

#ifdef linked_list_use_malloc
  *return_new_node = calloc (1, sizeof (node_t));
#else
  *return_new_node = h_alloc_struct (global_heap, "**");
#endif
  if (*return_new_node == NULL)
    {
      return MEMORY_ALLOCATION_FAILURE;
    }

  **return_new_node = (node_t){ .entry = entry, .next = (node_t *)next };
  return SUCCESS;
}

/// @brief Return a pointer to the pointer to the node before a given index.
/// @param head A pointer to a pointer to the head of a linked list.
/// @param index The index.
/// @return Returns a pointer to the pointer to the node before a given index.
static node_t **
get_ptr_before_index (node_t **head, const int index)
{
  // by calling get_ptr_to_index with (index - 1) we will (given that function
  // works) get the index before the one we want
  return get_ptr_to_index (head, index - 1);
}

/// @brief Return a pointer to the pointer to the node of a given index.
/// @param head A pointer to a pointer to the head of a linked list.
/// @param index The index.
/// @return Returns a pointer to the pointer to the node of a given index.
static node_t **
get_ptr_to_index (node_t **head, const int index)
{
  // index of 0 is start of list there by ptr to index is equal to head
  int index_tracker = 0;
  while (*head != NULL && index_tracker != index)
    {
      head = &((*head)->next); // Go to next node
      index_tracker += 1;
    }
  return head;
}

/// @brief Inserting an element into a list at the given index.
/// @param list The linked list that will be inserted into.
/// @param index The index to insert to.
/// @param element The element to insert.
/// @param size The size of the linked list.
/// @return The exit status of the function:
/// - SUCCESS:                      element appended successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - MEMORY_ALLOCATION_FAILURE:    memory allocation failed during node
/// creation
static int
insert_at_index (ioopm_list_t *list, const size_t index, const elem_t element,
                 size_t size)
{
  // List is not empty get pointer to node before our index
  node_t **prev = get_ptr_before_index (&(list->first), index);

  // Replace pointers next value with our new node
  // which in turn is updated to point to old next value
  node_t *new_node;
  if (create_node (&new_node, element, (*prev)->next) != SUCCESS)
    {
      return MEMORY_ALLOCATION_FAILURE;
    }
  (*prev)->next = new_node;

  // Check if we are inserting at the end of the list
  if (size == index)
    {
      // Update our last pointer to point to our new node
      list->last = (*prev)->next;
    }
  return SUCCESS;
}

/// @brief Inserting an element into a list at the given index.
/// @param list The linked list that will be inserted into.
/// @param element The element to insert.
/// @param size The size of the linked list.
/// @return The exit status of the function:
///- SUCCESS:                      element appended successfuly
///- INVALID_LIST_POINTER:         list is NULL (invalid pointer)
///- MEMORY_ALLOCATION_FAILURE:    memory allocation failed during node
/// creation
static int
insert_at_head (ioopm_list_t *list, const elem_t element, size_t size)
{
  // Check if list is empty
  node_t *new_node = NULL;
  if (size > 0)
    {
      // Not empty
      // Only need to insert a new link before first that points to the old
      // link
      if (create_node (&new_node, element, list->first) != SUCCESS)
        {
          return MEMORY_ALLOCATION_FAILURE;
        }
    }
  else
    {
      // in that case set first and last to be equal to our new node
      if (create_node (&new_node, element, NULL) != SUCCESS)
        {
          return MEMORY_ALLOCATION_FAILURE;
        }
      list->last = new_node;
    }
  list->first = new_node;
  return SUCCESS;
}

///  @brief Remove an element from a linked list in O(n) time.
///         The index values are [0,n-1] for a list of n elements,
///         where 0 means the first element and n-1 means the last element.
///  @param return_removed The removed element is returned via this parameter.
///  @param list The linked list.
///  @param size Size of the linked list.
///  @param index The position in the list.
///  @attention Index must be non negative and the list must be non-empty.
///  @return The exit status of the function:
///  - SUCCESS:                      removed an element successfuly
///  - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
///  - INVALID_RETURN_POINTER:       return_removed is NULL (invalid pointer)
///  - INDEX_OUT_OF_BOUNDS:          index was larger than list
///  - UNSPECIFIED_FAILURE:          unknown error
static void
remove_at_index (elem_t *return_removed, ioopm_list_t *list, size_t size,
                 const size_t index)
{
  // Removing a link in list at some index
  node_t **prev = get_ptr_before_index (&(list->first), index);

  // *prev is the link before our removal target
  node_t *to_unlink = (*prev)->next;

  // Update return pointer for removed_element
  *return_removed = to_unlink->entry;

  // unlink previous from removal target
  (*prev)->next = to_unlink->next;

  // check if removal target is our last link
  if (index == (size - 1))
    {
      // Relink list->last to the one before
      list->last = *prev;
    }

// Free memory of removal target
#ifdef linked_list_use_malloc
  free (to_unlink);
#endif
  list->size -= 1;
}

///  @brief Remove an element from a linked list in O(n) time.
///        The index values are [0,n-1] for a list of n elements,
///        where 0 means the first element and n-1 means the last element.
/// @param return_removed The removed element is returned via this parameter.
/// @param list The linked list.
/// @param size Size of the linked list.
/// @attention Index must be non negative and the list must be non-empty.
/// @return The exit status of the function:
/// - SUCCESS:                      removed an element successfuly
/// - INVALID_LIST_POINTER:         list is NULL (invalid pointer)
/// - INVALID_RETURN_POINTER:       return_removed is NULL (invalid pointer)
/// - INDEX_OUT_OF_BOUNDS:          index was larger than list
/// - UNSPECIFIED_FAILURE:          unknown error
static void
remove_at_head (elem_t *return_removed, ioopm_list_t *list, size_t size)
{
  // Removing first link
  node_t *to_unlink = list->first;
  *return_removed = to_unlink->entry;
  if (size > 1)
    {
      // We have more links,
      // Replace first link with the next link
      list->first = to_unlink->next;
    }
  else
    {
      // No more links in list,
      // Set to NULL
      list->first = NULL;
      list->last = NULL;
    }
#ifdef linked_list_use_malloc
  free (to_unlink);
#endif
  list->size -= 1;
}

/////////////////////////////////////////////////
///////////       ITERATOR       ////////////////
/////////////////////////////////////////////////

#define cast_to_list_iter(x) ((list_iterator_t *)x)

typedef struct list ioopm_list_t;

typedef struct list_iterator list_iterator_t;
struct list_iterator
{
  node_t *current;
  node_t *next;
  ioopm_list_t *list;
  size_t index;
};

static bool is_valid (list_iterator_t *iter);
static int initialize_iterator (ioopm_iterator_t **iter, ioopm_list_t *list);
static int initialize_list_iterator (list_iterator_t **list_iter,
                                     ioopm_list_t *list);

int
list_iterator_has_next (bool *return_has_next,
                        const void *internal_data_struct)
{
  list_iterator_t *list_iter = cast_to_list_iter (internal_data_struct);

  *return_has_next = list_iter->next != NULL;

  return SUCCESS;
}

int
list_iterator_next (elem_t *return_next, void *internal_data_struct)
{
  list_iterator_t *list_iter = cast_to_list_iter (internal_data_struct);

  bool has_next = false;
  if (list_iterator_has_next (&has_next, list_iter) == SUCCESS && has_next)
    {
      list_iter->current = list_iter->next;
      list_iter->next = list_iter->next->next;

      // increment current index
      list_iter->index++;

      // return new current element
      *return_next = list_iter->current->entry;
    }
  else
    {
      // Error handling when calling next on a iterator that is empty
      return ITERATOR_IS_EMPTY;
    }
  return SUCCESS;
}

int
list_iterator_current (elem_t *return_current,
                       const void *internal_data_struct)
{
  list_iterator_t *list_iter = cast_to_list_iter (internal_data_struct);

  if (is_valid (list_iter))
    {
      // Return the current item of the iterator
      *return_current = list_iter->current->entry;
      return SUCCESS;
    }
  else
    {
      //    User has not called ioopm_iterator_next after using
      //    ioopm_list_iterator (Creation)
      // OR User has not called ioopm_iterator_next after using
      // ioopm_iterator_reset (Reset) OR User has not called
      // ioopm_iterator_next after using ioopm_iterator_remove (Removal of
      // element) OR User has not called ioopm_iterator_next after using
      // ioopm_iterator_insert (Insertion of element)
      return INVALID_ITERATOR_STATE;
    }
}

int
list_iterator_remove (elem_t *return_removed, void *internal_data_struct)
{
  list_iterator_t *list_iter = cast_to_list_iter (internal_data_struct);

  // Remove previous element recived by ioopm_iterator_next /
  // ioopm_iterator_current dose not update current iterator and therfore any
  // opperations on iterator that is not has_next or next is invalid.
  if (is_valid (list_iter))
    {
      // Remove the current link
      ioopm_linked_list_remove (return_removed, list_iter->list,
                                list_iter->index);

      // set next to be pointer after this one,
      // set current to be null after removal to mark it as invalid
      list_iter->current = NULL;
      list_iter->index--;

      return SUCCESS;
    }
  else
    {
      //    User has not called ioopm_iterator_next after using
      //    ioopm_list_iterator (Creation)
      // OR User has not called ioopm_iterator_next after using
      // ioopm_iterator_reset (Reset) OR User has not called
      // ioopm_iterator_next after using ioopm_iterator_remove (Removal of
      // element) OR User has not called ioopm_iterator_next after using
      // ioopm_iterator_insert (Insertion of element)
      return INVALID_ITERATOR_STATE;
    }
}

int
list_iterator_insert (void *internal_data_struct, const elem_t element)
{
  list_iterator_t *list_iter = cast_to_list_iter (internal_data_struct);

  if (is_valid (list_iter))
    {
      // insert item on our index and step forward to the current
      int status = ioopm_linked_list_insert (list_iter->list, list_iter->index,
                                             element);
      if (status == SUCCESS)
        {
          list_iter->current = NULL;
          return SUCCESS;
        }
      else
        {
          // insert failed, Propagate errors.
          // Possible errors not caught by our guards:
          // MEMORY_ALLOCATION_FAILURE
          return status;
        }
    }
  else
    {
      //    User has not called ioopm_iterator_next after using
      //    ioopm_list_iterator (Creation)
      // OR User has not called ioopm_iterator_next after using
      // ioopm_iterator_reset (Reset) OR User has not called
      // ioopm_iterator_next after using ioopm_iterator_remove (Removal of
      // element) OR User has not called ioopm_iterator_next after using
      // ioopm_iterator_insert (Insertion of element)
      return INVALID_ITERATOR_STATE;
    }
}

int
list_iterator_reset (void *internal_data_struct)
{
  list_iterator_t *list_iter = cast_to_list_iter (internal_data_struct);

  list_iter->current = NULL;
  list_iter->next = list_iter->list->first;
  list_iter->index = -1;

  return SUCCESS;
}

int
list_iterator_destroy (void *internal_data_struct)
{
  list_iterator_t **list_iter = ((list_iterator_t **)internal_data_struct);
#ifdef linked_list_use_malloc
  free (*list_iter);
#endif
  *list_iter = NULL;

  return SUCCESS;
}

int
ioopm_list_iterator (ioopm_iterator_t **return_iter, ioopm_list_t *list)
{
  if (return_iter == NULL)
    {
      return INVALID_RETURN_POINTER;
    }
  if (list == NULL)
    {
      return INVALID_LIST_POINTER;
    }

  int iter_init_status = initialize_iterator (return_iter, list);
  return iter_init_status;
}

/// @brief Check if the iterator is in a valid state for retrival or changes.
/// @param iter The iterator to check if it has a elem_t in current.
/// @return true if the current node_t is not a NULL pointer
static bool
is_valid (list_iterator_t *iter)
{
  return iter->current != NULL;
}

/// @brief Initializes the fields in an iterator.
/// @param list The iterator to be initialized.
/// @param list The list that the iterator is based upon.
static int
initialize_iterator (ioopm_iterator_t **return_iter, ioopm_list_t *list)
{
  list_iterator_t *list_iter = NULL;

  int list_itr_init_status = initialize_list_iterator (&list_iter, list);
  if (list_itr_init_status != SUCCESS)
    {
      // Something failed when initializing our list iterator
      // Propagate to caller
      return list_itr_init_status;
    }

  int itr_init_status = ioopm_iterator_create (
      return_iter, list_iter, list_iterator_has_next, list_iterator_next,
      list_iterator_current, list_iterator_remove, list_iterator_insert,
      list_iterator_reset, list_iterator_destroy);
  if (itr_init_status == SUCCESS)
    {
      return SUCCESS;
    }
  else
    {
// Something failed when creating our general iterator
#ifdef linked_list_use_malloc
      free (list_iter);
#endif
      return itr_init_status;
    }
}

/// @brief Initializes the fields in an list iterator.
/// @param list The iterator to be initialized.
/// @param list The list that the iterator is based upon.
static int
initialize_list_iterator (list_iterator_t **list_iter, ioopm_list_t *list)
{
#ifdef linked_list_use_malloc
  *list_iter = calloc (1, sizeof (list_iterator_t));
#else
  *list_iter = h_alloc_struct (global_heap, "***l");
#endif
  if (*list_iter == NULL)
    {
      return MEMORY_ALLOCATION_FAILURE;
    }

  (*list_iter)->current = NULL;
  (*list_iter)->next = list->first;
  (*list_iter)->list = list;
  (*list_iter)->index = -1;

  return SUCCESS;
}

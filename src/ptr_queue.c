/**
 * Creation of a queue used to store visited pointers during the mark-phase.
 */

#include "ptr_queue.h"
#include <assert.h>

typedef struct link link_t;
struct link
{
  void *elem;
  link_t *next;
};

struct ptr_queue
{
  link_t *first;
  size_t size;
};

ptr_queue_t *
create_ptr_queue (void)
{
  return calloc (1, sizeof (ptr_queue_t));
}

static link_t *
create_link (void *elem, link_t *next)
{
  link_t *new_link = calloc (1, sizeof (link_t));
  new_link->elem = elem;
  new_link->next = next;

  return new_link;
}

void
destroy_ptr_queue (ptr_queue_t *q)
{
  link_t *current_link = q->first;

  while (current_link)
    {
      link_t *next = current_link->next;
      free (current_link);
      current_link = next;
    }

  free (q);
}

bool
enqueue_ptr (ptr_queue_t *q, void *ptr)
{
  assert (q);
  assert (ptr && "Can't enqueue NULL in pointer queue.");

  link_t **next_p = &(q->first);

  while (*next_p && ((*next_p)->elem < ptr))
    {
      next_p = &((*next_p)->next);
    }

  if (*next_p && ((*next_p)->elem == ptr))
    {
      return false;
    }

  link_t *new_link = create_link (ptr, *next_p);
  *next_p = new_link;
  q->size++;
  return true;
}

void **
dequeue_ptr (ptr_queue_t *q)
{
  if (is_empty_queue (q))
    return NULL;

  link_t *first = q->first;
  void *first_elem = first->elem;

  q->first = first->next;
  free (first);
  q->size--;

  return first_elem;
}

void **
get_ptr (ptr_queue_t *q, size_t index)
{
  if (is_empty_queue (q) || index >= get_len (q))
    return NULL;

  /* if index == 0, return first elem*/
  if (index == 0)
    {
      return q->first->elem;
    }

  /* if index > 0, call next (index - 1) times due to first being 1 time */
  link_t *current = q->first;
  for (size_t i = 0; i < index; i++)
    {
      current = current->next;
    }
  return current->elem;
}

size_t
get_len (ptr_queue_t *q)
{
  return q->size;
}

bool
is_empty_queue (ptr_queue_t *q)
{
  return q->size == 0;
}

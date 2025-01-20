/**
 * The main file for the automatic memory garbage collector, called Vapid GC.
 */

#include <assert.h>
#include <math.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <valgrind/memcheck.h>

/* Stops gc.h from copying "extern heap_t *global_heap"  */
#define __gc__
#include "allocation.h"
#include "allocation_map.h"
#include "compacting.h"
#include "format_encoding.h"
#include "gc.h"
#include "gc_utils.h"
#include "get_header.h"
#include "header.h"
#include "heap_internal.h"
#include "is_pointer_in_alloc.h"
#include "move_data.h"
#include "page_map.h"
#include "ptr_queue.h"
#include "stack.h"

/* The amount of bytes to mark as defined for valgrind, in order to avoid
 * warnings for using uninitialized values when looping through stack pointers.
 */
#define MEM_TO_DEFINE 32

#define Dump_registers()                                                      \
  jmp_buf env;                                                                \
  if (setjmp (env))                                                           \
    {                                                                         \
      abort ();                                                               \
    }

struct apply_dbg_info
{
  heap_t *heap;
  void *dbg_value;
};

typedef struct apply_dbg_info apply_dbg_info_t;

heap_t *global_heap = NULL;

heap_t *
h_init (size_t bytes, bool unsafe_stack, float gc_threshold)
{
  /* Allocate heap struct on the heap.  */
  heap_t *heap = malloc (sizeof (heap_t));

  /* Ensure our size is atleast as big as our minimal size.  */
  bytes = bytes < MINIMUM_ALIGNMENT ? MINIMUM_ALIGNMENT : bytes;

  /* Round up to multiples of HEAP_ALIGNMENT.  */
  size_t aligned_size
      = ((bytes + HEAP_ALIGNMENT - 1) / HEAP_ALIGNMENT) * HEAP_ALIGNMENT;

  /* Set fields:  */
  /* Threshold before running GC.  */
  heap->gc_threshold = gc_threshold;

  /* Are stack pointers considered safe? true = yes, false = no  */
  heap->is_unsafe_stack = unsafe_stack;

  /* Size of allocated heap.  */
  heap->size = aligned_size;

  heap->page_size = PAGE_SIZE;
  /* Map of active/inactive pages.  */
  heap->page_map = create_page_map (aligned_size);

  /* Map of active/inactive allocations.  */
  heap->alloc_map = create_allocation_map (aligned_size);

  /* The actual heap which objects will be allocated on.  */
  heap->heap_start = calloc (aligned_size, sizeof (char));

  /* The bump pointer to where the next allocation will be made.  */
  heap->next_empty_mem_segment = heap->heap_start;

  /* Initializes the current number of used allocated bytes to 0.  */
  heap->used_bytes = 0;

  /* Store a reference to the first created heap in the global heap.  */
  if (global_heap == NULL)
    {
      global_heap = heap;
    }

  return heap;
}

void
h_delete (heap_t *h)
{
  if (h->alloc_map != NULL)
    { /* The check can be removed when the allocation map is implemented in
         h_init
         Frees the allocation map if it exists.  */
      free (h->alloc_map);
    }
  if (h->page_map != NULL)
    { /* The check can be removed when the allocation map is implemented in
         h_init
         Frees the allocation map if it exists.  */
      free (h->page_map);
    }

  /* if we are destroying the heap ref stored in global heap,
     we want to clear it to allow next h_init to set it.  */
  if (h == global_heap)
    {
      global_heap = NULL;
    }

  free (h->heap_start); /* Frees the heap.  */
  free (h);
}

void
set_heap_ptr_to_dbg_val (void *ptr, void *info)
{
  apply_dbg_info_t *apply_info = (apply_dbg_info_t *)info;
  uintptr_t *dbg_value = (uintptr_t *)apply_info->dbg_value;

  /* Get start- and endpoint of heap.  */
  heap_t *h = apply_info->heap;
  uintptr_t h_start = (uintptr_t)h->heap_start;
  uintptr_t h_end = h_start + h->size;
  sort_stack_ends (&h_start, &h_end);

  /* Check if the current ptr points to an address in the heap.  */
  uintptr_t *h_ptr = (uintptr_t *)ptr;
  uintptr_t stack_ptr = *h_ptr;
  bool lessThanEnd = stack_ptr < (uintptr_t)h_end;
  bool greaterThanStart = stack_ptr > (uintptr_t)h_start;
  if ((greaterThanStart && lessThanEnd))
    {
      *h_ptr = *dbg_value;
    }
}

void
h_delete_dbg (heap_t *h, void *dbg_value)
{
  Dump_registers ();

  uintptr_t start = find_stack_beginning ();
  uintptr_t end = find_stack_end ();
  sort_stack_ends (&start, &end);

  apply_dbg_info_t *apply_info = calloc (1, sizeof (apply_dbg_info_t));
  apply_info->heap = h;
  apply_info->dbg_value = dbg_value;

  /* NOTE: Marks the pointer as defined because valgrind does not like
     accessing memory that is not defined.  */
  VALGRIND_MAKE_MEM_DEFINED ((void *)start, end - start);

  apply_to_pointers_in_interval (
      start, end, (apply_to_ptr_func *)set_heap_ptr_to_dbg_val, apply_info);

  free (apply_info);
  h_delete (h);
}

size_t
h_avail (heap_t *h)
{
  size_t free_bytes = 0;
  size_t heap_size = h->size;
  for (size_t offset = 0; offset < heap_size; offset += HEAP_ALIGNMENT)
    {
      if (!is_offset_allocated (h->alloc_map, offset))
        {
          free_bytes += HEAP_ALIGNMENT;
        }
    }
  return free_bytes;
}

size_t
h_used (heap_t *h)
{
  return h->used_bytes;
}

void *
h_alloc_struct (heap_t *h, char *layout)
{
  void *allocation = alloc_struct (h, layout);
  return allocation;
}

void *
h_alloc_raw (heap_t *h, size_t bytes)
{
  void *allocation = alloc_raw (h, bytes);
  return allocation;
}

/**
 * Checks if the pointer points within a page and if so marks the page as
 * non-movable.
 */
void
mark_page_if_ptr (void *ptr, void *extra)
{
  heap_t *h = (heap_t *)extra;
  char *c_ptr = *(char **)ptr;
  /* NOTE: Marks the pointer as defined because valgrind does not like
     accessing memory that is not defined.  */
  VALGRIND_MAKE_MEM_DEFINED (&c_ptr, MEM_TO_DEFINE);

  if (is_heap_pointer ((uintptr_t)c_ptr, h))
    {
      size_t offset = calc_heap_offset (c_ptr, h);
      update_page_map (h->page_map, offset, false);
    }
}

/**
 * Loops through pointers on the stack, if it points into the heap and
 * unsafe_stack is true, then mark the page as non-movable in the page_map.
 * @param h - the heap.
 */
void
h_mark_pages (heap_t *h)
{
  h_reset_page_map (h);
  if (!h->is_unsafe_stack)
    {
      /* Stack pointers are treated as safe, all pages can be moved.  */
      return;
    }

  uintptr_t start = find_stack_beginning ();
  uintptr_t end = find_stack_end ();
  sort_stack_ends (&start, &end);
  apply_to_pointers_in_interval (start, end,
                                 (apply_to_ptr_func *)mark_page_if_ptr, h);
}

/**
 * A struct containing information about the compacting status.
 */
typedef struct compacting_info compacting_info_t;
struct compacting_info
{
  heap_t *h;
  char *new_alloc_map;
  size_t num_alive;
  ptr_queue_t *compacting_queue;
  ptr_queue_t *visited;
  ptr_queue_t *forwarding_addresses;
};

/**
 * Checks if alloc is a pointer to an allocation, if it is add it to the
 * compacting_queue and call this function for all pointers in the allocation.
 * @param alloc the ptr to add to queue
 * @param c_info the struct with info about the compacting process
 * @return true if compact was successful else false.
 */
void
add_alloc_to_move_queue (char **alloc_ptr, compacting_info_t *c_info)
{
  char *alloc = *alloc_ptr;
  heap_t *h = c_info->h;
  if (!is_heap_pointer ((uintptr_t)alloc, h))
    {
      return;
    }
  if (alloc == h->heap_start)
    {
      return;
    }

  header_t *header_p = get_header_pointer (alloc);

  if (!is_heap_pointer ((uintptr_t)header_p, h))
    {
      return;
    }

  header_t header = get_header_value (header_p);
  if (header == 0)
    {
      return;
    }

  header_type_t header_type = get_header_type (header);
  if (!(header_type == HEADER_BIT_VECTOR
        || header_type == HEADER_POINTER_TO_FORMAT_STRING))
    {
      return;
    }

  size_t offset = calc_heap_offset (alloc, h);
  bool is_moveable = is_offset_movable (h->page_map, offset);
  if (is_moveable)
    {
      bool already_in_queue = !enqueue_ptr (c_info->compacting_queue, alloc);
      if (already_in_queue)
        {
          return;
        }
    }
  else
    {
      bool already_in_queue = !enqueue_ptr (c_info->visited, alloc);
      if (already_in_queue)
        {
          return;
        }
      size_t alloc_size = calc_alloc_size (alloc);
      c_info->num_alive++;
      for (size_t curr_offset = offset; curr_offset <= offset + alloc_size;
           curr_offset += HEAP_ALIGNMENT)
        {
          update_alloc_map (c_info->new_alloc_map, curr_offset, true);
        }
    }

  if (header_type == HEADER_POINTER_TO_FORMAT_STRING)
    {
      char *ptr_to_format = (char *)(clear_formating_encoding (header));
      add_alloc_to_move_queue (&ptr_to_format, c_info);
    }

  ptr_queue_t *pointers_in_alloc = get_pointers_in_allocation (alloc);
  void **current_ptr = dequeue_ptr (pointers_in_alloc);
  while (current_ptr != NULL)
    {
      add_alloc_to_move_queue ((char **)current_ptr, c_info);
      current_ptr = dequeue_ptr (pointers_in_alloc);
    }
  destroy_ptr_queue (pointers_in_alloc);
}

/**
 * Check if a stack pointer points into the heap and if so add it and its own
 * pointers to the compacting queue.
 * @param ptr the stack pointer to check
 * @param extra an argument containing information about the compacting
 */
void
add_stack_ptr_to_move_queue (void *ptr, void *extra)
{
  compacting_info_t *c_info = (compacting_info_t *)extra;
  char *c_ptr = (char *)ptr;

  VALGRIND_MAKE_MEM_DEFINED (c_ptr, MEM_TO_DEFINE);

  add_alloc_to_move_queue ((char **)c_ptr, c_info);
}

/**
 * Populate the compacting queue by looping through all allocations that can be
 * reached from the stack and add them to the queue.
 * @param c_info the compacting info object
 */
void
populate_compacting_queue (compacting_info_t *c_info, uintptr_t stack_begin,
                           uintptr_t stack_end)
{
  /* Populate the compacting queue by looping through all allocations that can
     be reached from the stack and add them to the queue.  */
  apply_to_pointers_in_interval (
      stack_begin, stack_end, (apply_to_ptr_func *)add_stack_ptr_to_move_queue,
      c_info);
}

/**
 * Creates a struct containing data about the compacting stage.
 * @param h the heap
 * @return the struct of compacting data
 */
compacting_info_t *
create_compacting_info (heap_t *h)
{
  compacting_info_t *c_info = calloc (1, sizeof (compacting_info_t));
  c_info->h = h;
  c_info->compacting_queue
      = create_ptr_queue (); /*  A priority queue of pointers to be compacted.
                              */
  c_info->visited
      = create_ptr_queue (); /* A priority queue of visited stack pointers.  */
  c_info->forwarding_addresses
      = create_ptr_queue (); /* A queue containing all allocations for.
                                forwarding addresses. */
  c_info->new_alloc_map = create_allocation_map (h->size);
  return c_info;
}

/**
 * Frees all heap memory related to the c_info
 * @param c_info the compacting_info to be freed.
 */
void
destroy_compacting_info (compacting_info_t *c_info)
{
  destroy_ptr_queue (c_info->compacting_queue);
  destroy_ptr_queue (c_info->visited);
  destroy_ptr_queue (c_info->forwarding_addresses);
  free (c_info);
}

/**
 * Compacts the pointer to an allocation by moving the allocation to the next
 * available space near the start of the heap and marking the old location with
 * a forwarding address.
 * @param h the heap
 * @param alloc the pointer
 */
void
compact_ptr (compacting_info_t *c_info, void *alloc)
{
  heap_t *h = c_info->h;
  size_t alloc_size = sizeof (header_t) + calc_alloc_size (alloc);

  bool is_space_available = move_to_next_available_space (h, alloc_size);
  assert (is_space_available); /* Should always be true during compacting.  */

  char *dest = h->next_empty_mem_segment + sizeof (header_t);

  // Check if the next aavailable destination is the origin.
  if ((void *)dest == alloc)
    {
      // first available spot is the origin.
      // set space to allocated and do not move pointer
      size_t new_offset = calc_heap_offset (dest, h) - sizeof (header_t);
      for (size_t offset = 0; offset < alloc_size + sizeof (header_t);
           offset += HEAP_ALIGNMENT)
        {
          update_alloc_map (h->alloc_map, new_offset + offset, true);
        }
      c_info->num_alive++;
      return;
    }

  size_t old_offset = calc_heap_offset (alloc, h);
  update_alloc_map (h->alloc_map, old_offset,
                    true); /* Mark old address as allocated to not allow
                              overwrite of forwarding address.  */
  enqueue_ptr (c_info->forwarding_addresses, alloc);

  move_alloc (&alloc, dest);
  h->next_empty_mem_segment += alloc_size;
  c_info->num_alive++;

  size_t new_offset = calc_heap_offset (dest, h);
  for (size_t offset = 0; offset < alloc_size; offset += HEAP_ALIGNMENT)
    {
      update_alloc_map (h->alloc_map, new_offset + offset, true);
    }
}

/**
 * Checks if the pointer is pointing into the heap, to an object that is a
 * forwarding address, if so, make the pointer point at the new address.
 * @param ptr an address of a pointer
 * @param c_info the struct containing info about the compacting.
 */
void
update_ptr_to_forwarding_address (void **ptr, compacting_info_t *c_info,
                                  bool is_stack_ptr)
{
  heap_t *h = c_info->h;
  char *c_ptr = (char *)*ptr;

  if (!is_heap_pointer ((uintptr_t)c_ptr, h)
      || (c_ptr == (char *)h->heap_start))
    {
      return;
    }

  if (!(is_stack_ptr && h->is_unsafe_stack))
    {
      header_t header = get_header_value (get_header_pointer (c_ptr));
      header_type_t header_type = get_header_type (header);
      if (header_type == HEADER_FORWARDING_ADDRESS)
        {
          void *forwarding_address
              = (void *)(clear_formating_encoding (header));
          *ptr = forwarding_address;
          c_ptr = forwarding_address;
        }
    }

  header_t header = get_header_value (get_header_pointer (c_ptr));
  header_type_t header_type = get_header_type (header);
  if (header_type == HEADER_POINTER_TO_FORMAT_STRING)
    {
      char *ptr_to_format = (char *)(clear_formating_encoding (header));
      header_t *header_p = get_header_pointer (ptr_to_format);
      if (!is_heap_pointer ((uintptr_t)header_p, h))
        {
          return;
        }

      header_t format_header = get_header_value (header_p);
      header_type_t header_type = get_header_type (format_header);
      if (header_type == HEADER_FORWARDING_ADDRESS)
        {
          header_t *header_p = get_header_pointer (c_ptr);
          *header_p = set_header_pointer_to_format_string (format_header);
        }
    }

  ptr_queue_t *pointers_in_alloc = get_pointers_in_allocation (c_ptr);
  void **current_ptr = dequeue_ptr (pointers_in_alloc);
  while (current_ptr != NULL)
    {
      update_ptr_to_forwarding_address (current_ptr, c_info, false);
      current_ptr = dequeue_ptr (pointers_in_alloc);
    }
  destroy_ptr_queue (pointers_in_alloc);
}

/**
 * Loops through all pointers that can be reached from the stack that points
 * into the heap. If a pointer points toward an object that has a forwarding
 * address, make the pointer point at the new address.
 * @param c_info the struct containing info about the compacting.
 * @param stack_begin the start of the stack (lower address)
 * @param stack_end the end of the stack (higher address)
 */
void
update_ptrs_to_forwarding_address (compacting_info_t *c_info,
                                   uintptr_t stack_begin, uintptr_t stack_end)
{
  for (uintptr_t possible_ptr = stack_begin; possible_ptr < stack_end;
       possible_ptr += sizeof (void *))
    {
      update_ptr_to_forwarding_address ((void **)possible_ptr, c_info, true);
    }
}

/**
 * Counts the number of bytes collected during compacting.
 * @param c_info the struct containing info about the compacting
 * @return the number of bytes freed.
 */
size_t
count_collected_bytes (compacting_info_t *c_info)
{
  heap_t *h = c_info->h;
  size_t num_alloc = 0;
  for (size_t offset = 0; offset < h->size; offset += HEAP_ALIGNMENT)
    {
      if (is_offset_allocated (h->alloc_map, offset))
        {
          num_alloc += HEAP_ALIGNMENT;
        }
    }

  void *current_forwarding_address
      = dequeue_ptr (c_info->forwarding_addresses);
  while (current_forwarding_address != NULL)
    {
      size_t offset = calc_heap_offset (current_forwarding_address, h);
      num_alloc -= HEAP_ALIGNMENT;
      update_alloc_map (h->alloc_map, offset, false);
      current_forwarding_address = dequeue_ptr (c_info->forwarding_addresses);
    }

  /* Remove counting of metadata.  */
  num_alloc -= (c_info->num_alive) * sizeof (header_t);

  int diff = h->used_bytes - num_alloc;
  return (size_t)diff;
}

/**
 * Compacts the heap
 * @param h the heap
 * @return the number of bytes collected.
 */
size_t
h_compact (heap_t *h)
{
  Dump_registers ();
  uintptr_t stack_begin = find_stack_beginning ();
  uintptr_t stack_end = find_stack_end ();
  sort_stack_ends (&stack_begin, &stack_end);

  compacting_info_t *c_info = create_compacting_info (h);
  populate_compacting_queue (c_info, stack_begin, stack_end);

  char *prev_alloc_map = h->alloc_map;
  h->alloc_map = c_info->new_alloc_map;
  h->next_empty_mem_segment = h->heap_start;
  free (prev_alloc_map);

  ptr_queue_t *compacting_queue = c_info->compacting_queue;
  void *current_alloc = dequeue_ptr (compacting_queue);
  while (current_alloc != NULL)
    {
      compact_ptr (c_info, current_alloc);
      current_alloc = dequeue_ptr (compacting_queue);
    }

  update_ptrs_to_forwarding_address (c_info, stack_begin, stack_end);

  size_t bytes_collected = count_collected_bytes (c_info);
  destroy_compacting_info (c_info);

  h->used_bytes -= bytes_collected;

  return bytes_collected;
}

size_t
h_gc (heap_t *h)
{
  /* For some reason this works to clear any additional old stack variables
     that point onto allocations.  */
  void *i = 0;
  (void)i;
  Dump_registers ()

      uintptr_t start
      = find_stack_beginning ();
  uintptr_t end = find_stack_end ();
  sort_stack_ends (&start, &end);

  /* Find root pointers */
  ptr_queue_t *roots = find_root_pointers (h, start, end);

  /* Populate a compacting queue, that will hold each allocation with first
   * being nearest to heap start. */
  ptr_queue_t *compaction_queue = find_living_objects (h, roots);

  // TODO: For each object in compaction queue, find a new location and update
  // old header with forwarding to new dest.
  // TODO: find other way to store forwarding for object to allow compact data
  size_t bytes_collected = compact_objects (h, roots, compaction_queue);

  // TODO: after all objects have been copied and their headers updated,
  // iterate through using roots and update recursively.
  update_forwarded_pointers (h, roots);

  // TODO: after all pointers has been forwarded, go through heap removing
  // forwarding allocations.
  remove_forwarding_allocations (h, compaction_queue);

  /* clean up queues no longer used */
  destroy_ptr_queue (compaction_queue);
  destroy_ptr_queue (roots);

  return bytes_collected;

  // Old solution bellow
  // destroy_ptr_queue (roots);
  // h_mark_pages (h);
  // size_t bytes_collected = h_compact (h);
  // return bytes_collected;
}

size_t
h_gc_dbg (heap_t *h, bool unsafe_stack)
{
  /* Stores the old value of is_unsafe_stack */
  bool prev_val = h->is_unsafe_stack;

  /* Temporarily overwrite value of is_unsafe_stack and run gc */
  h->is_unsafe_stack = unsafe_stack;
  size_t bytes_collected = h_gc (h);

  /* Reset is_unsafe_stack to previous value */
  h->is_unsafe_stack = prev_val;

  return bytes_collected;
}

/**
 * @file
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
#include "ptr_queue.h"
#include "stack.h"

/* The amount of bytes to mark as defined for Valgrind, in order to avoid
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
  heap_t *heap = create_heap (bytes, unsafe_stack, gc_threshold);

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
  /* if we are destroying the heap ref stored in global heap,
     we want to clear it to allow next h_init to set it.  */
  if (h == global_heap)
    {
      global_heap = NULL;
    }
  destroy_heap (h);
}

void
h_delete_dbg (heap_t *h, void *dbg_value)
{
  Dump_registers ();

  uintptr_t start = find_stack_beginning ();
  uintptr_t end = find_stack_end ();
  sort_stack_ends (&start, &end);

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
  return num_free_bytes (h);
}

size_t
h_used (heap_t *h)
{
  return num_allocated_bytes (h->alloc_map);
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

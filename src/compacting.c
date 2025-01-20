
#include <assert.h>
#include <string.h>

#include "allocation.h"
#include "allocation_map.h"
#include "compacting.h"
#include "gc_utils.h"
#include "get_header.h"
#include "heap_internal.h"
#include "is_pointer_in_alloc.h"
#include "page_map.h"
#include "stack.h"

typedef struct find_root_data
{
  heap_t *heap;
  ptr_queue_t *roots;
} find_root_data_t;

typedef struct compact_data
{
  heap_t *heap;
  ptr_queue_t *roots;
  ptr_queue_t *visited;
} compact_data_t;

static void recurse_find_pointer (heap_t *h, void *alloc,
                                  ptr_queue_t *living_objects);

static bool compact_ptr (heap_t *heap, void *object);

static void apply_to_pointer_in_allocation (void *object,
                                            apply_to_ptr_func *fun,
                                            void *other);

/*
 * Checks if a given stack address holds a value to an active allocation.
 * If so, then enqueue the allocation to the pointer queue
 * given in the void* other argument.
 */
static void
enqueue_root_pointer (void *stack_address, void *other)
{
  find_root_data_t *data = (find_root_data_t *)other;
  heap_t *h = data->heap;

  /* Assumes stack_address holds a pointer to our heap,
     to read this pointer we must dereference stack address.*/
  void *potential_heap_ptr = *((void **)stack_address);

  /* Check if value stored in stack variable is a potential heap pointer */
  if (!is_heap_pointer ((uintptr_t)potential_heap_ptr, h))
    {
      return;
    }
  /* Pointer points to living object, add object as a root */
  ptr_queue_t *roots = data->roots;
  enqueue_ptr (roots, potential_heap_ptr);
}

/*
 * iterates over the stack and enqueues all objects
 * found in the heap as root objects
 */
ptr_queue_t *
find_root_pointers (heap_t *h, uintptr_t start, uintptr_t end)
{

  /* setup void *arg */
  find_root_data_t *data = malloc (sizeof (find_root_data_t));
  data->heap = h;
  data->roots = create_ptr_queue ();

  /* iterate over the stack populating our queue as it goes */
  apply_to_pointers_in_interval (
      start, end, (apply_to_ptr_func *)enqueue_root_pointer, data);

  return data->roots;
}

ptr_queue_t *
find_living_objects (heap_t *h, ptr_queue_t *roots)
{
  ptr_queue_t *living_objects = create_ptr_queue ();

  /* Find all pointers to allocations recursively */
  for (size_t i = 0; i < get_len (roots); i++)
    {
      void *root_object = get_ptr (roots, i);
      recurse_find_pointer (h, root_object, living_objects);
    }

  /* return sorted queue of living objects */
  return living_objects;
}

/**
 * @brief Recursively travles through the heap enqueueing all pointers found in
 * each allocation. travels depth first, queue is sorted according position in
 * heap, closest to heap start first.
 * @param h heap refrence used to verify that a pointer is within heap.
 * @param alloc the allocation the find allocations within.
 * @param living_objects a queue of all living objects,
 * also provides inforamtion to avoid rechecking a pointer
 */
static void
recurse_find_pointer (heap_t *h, void *alloc, ptr_queue_t *living_objects)
{
  bool is_new_ptr = enqueue_ptr (living_objects, alloc);
  if (!is_new_ptr)
    {
      return;
    }

  /* Check if alloc has a pointer to format string */
  header_t *h_ptr = get_header_pointer(alloc);
  header_t h_value = get_header_value(h_ptr);
  header_type_t h_type = get_header_type(h_value);
  if (h_type == HEADER_POINTER_TO_FORMAT_STRING)
  {
    /* visit format string object */
    void *format_string_ptr = get_pointer_in_header(h_value);
    enqueue_ptr (living_objects, format_string_ptr);
  }

  ptr_queue_t *possible_struct_ptrs = get_pointers_in_allocation (alloc);
  /* retrieve pointer to first internal pointer */
  void **struct_pointer_ptr = dequeue_ptr (possible_struct_ptrs);
  while (struct_pointer_ptr != NULL)
    {
      /* pointer is not null, create better variable name */
      void *struct_pointer = *struct_pointer_ptr;

      /* check if pointer is a refrence in our heap */
      // TODO: Check if "is_heap_pointer()" is a better check...
      if (is_in_range (((uintptr_t)struct_pointer), h))
        {
          /* recursively find pointers from inside the struct pointer */
          recurse_find_pointer (h, struct_pointer, living_objects);
        }

      /* go to next internal pointer of current struct */
      struct_pointer_ptr = dequeue_ptr (possible_struct_ptrs);
    }
}

// TODO: For each object in compaction queue, find a new location and update
// old header with forwarding to new dest.
// TODO: find other way to store forwarding for object to allow compact data
size_t
compact_objects (heap_t *h, ptr_queue_t *roots, ptr_queue_t *compaction_queue)
{
  /* set heap to completly empty */
  h_reset_page_map (h);
  reset_allocation_map (h);
  h->next_empty_mem_segment = h->heap_start;

  size_t old_size = h->used_bytes;
  h->used_bytes = 0;

  /* if stack is unsafe we need to update heap to show roots as allocated */
  if (h->is_unsafe_stack)
    {
      for (size_t i = 0; i < get_len (roots); i++)
        {
          void *root_ptr = get_ptr (roots, i);

          /* fix allocation map */
          size_t obj_offset_from_start
              = calc_heap_offset (root_ptr, h) - sizeof (header_t);
          /* set root object position as allocated */
          /* get size of allocation */
          size_t alloc_size = calc_alloc_size (root_ptr);

          /* align size with our alloc map (rounding up to nearest multiple of
           * 16) */
          alloc_size = align_alloc_size (alloc_size) + sizeof (header_t);
          assert (alloc_size % 16 == 0 && "Is divisable by 16");

          for (size_t i = 0; i < alloc_size; i += MIN_ALLOC_OBJECT_SIZE)
            {
              update_alloc_map (h->alloc_map, obj_offset_from_start + i, true);
            }

          /* fix bytes_used metric */
          h->used_bytes += alloc_size - sizeof (header_t);
        }
    }

  /* compact allocations in order of closest to heap first, then next and
   * lastly pointer allocated futherst away. */
  if (h->is_unsafe_stack)
    {
      /* stack is unsafe, so we are not allowed to move root pointers */
      for (size_t i = 0; i < get_len (compaction_queue); i++)
        {
          void *internal_pointer = get_ptr (compaction_queue, i);
          /* Check if current pointer is any of recived root pointers */
          bool is_root_obj = false;
          for (size_t i = 0; i < get_len (roots); i++)
            {
              void *root_ptr = get_ptr (roots, i);
              if (root_ptr == internal_pointer)
                {
                  is_root_obj = true;
                  break;
                }
            }

          /* if it is a root object, skip compacting this object */
          if (is_root_obj)
            {
              continue;
            }
          compact_ptr (h, internal_pointer);
        }
    }
  else
    {
      for (size_t i = 0; i < get_len (compaction_queue); i++)
        {
          void *internal_pointer = get_ptr (compaction_queue, i);
          compact_ptr (h, internal_pointer);
        }
    }
  size_t new_size = h->used_bytes;
  return old_size - new_size;
}

/**
 * @brief compacts the given pointer, updating heap accordingly.
 * @param heap the heap which the object should be allocated within
 * @param object the object to compact
 * @return true if object was moved, false if object is in correct place
 * already.
 */
// TODO: Orignial format string get overwritten due to it not being in living object queue.
// When Adding format string to living object problem presists, and instead seems to 
static bool
compact_ptr (heap_t *heap, void *alloc)
{
  /* get pointer to objects header, we will need to replace value later */
  header_t *header_ptr = get_header_pointer (alloc);
  void *origin = header_ptr;

  header_t h_value = get_header_value(header_ptr);
  header_type_t h_type = get_header_type(h_value);
  /* Check if string in header has been forwarded */
  if (h_type == HEADER_FORWARDING_ADDRESS)
  {
    void *new_format_location = get_pointer_in_header(h_value);
    header_t new_alloc_header = set_header_pointer_to_format_string((header_t)new_format_location);
    *header_ptr = new_alloc_header;
  }
  if (h_type == HEADER_POINTER_TO_FORMAT_STRING)
  {
    /* Move format string first */
    void *format_string_ptr = get_pointer_in_header(h_value);
    if (compact_ptr(heap, format_string_ptr))
    {
      /* format string was moved during compacting,
      need to update allocation header before we move it so the real header is copied correctly */
      header_t *format_header_ptr = get_header_pointer(format_string_ptr);
      header_t format_header_value = get_header_value(format_header_ptr);
      void *new_format_location = get_pointer_in_header(format_header_value);
      header_t new_alloc_header = set_header_pointer_to_format_string((header_t)new_format_location);
      *header_ptr = new_alloc_header;
      header_type_t h_form_type = get_header_type(format_header_value);
    }
    /* format string might not have been moved during compacting
    if that is the case then the current format string pointer is still correct. */
    
  }
  

  /* get size of allocation */
  size_t alloc_size = calc_alloc_size (alloc);

  /* align size with our alloc map (rounding up to nearest multiple of 16) */
  alloc_size = align_alloc_size (alloc_size);
  alloc_size += sizeof (header_t);
  assert (alloc_size % 16 == 0 && "Is divisable by 16");

  /* Moves bump pointer to next available space in the heap.  */
  bool space_found = move_to_next_available_space (heap, alloc_size);

  if (!space_found)
    {
      /* Not enough space to allocated object during compacting */
      abort ();
    }

  /* heap->next_empty_mem_segment is now set to next available position */
  void *dest = heap->next_empty_mem_segment;

  if ((uintptr_t)origin - (uintptr_t)dest < alloc_size)
  {
    /* first available spot is to close to existing allocation */
    /* skip moving this allocation by setting dest to origin */
    dest = origin;
    /* update bump pointer */
    heap->next_empty_mem_segment = origin;
  }
  
  /* set allocation destination as allocated in heap */
  size_t dest_offset_from_start = calc_heap_offset (dest, heap);
  for (size_t i = 0; i < alloc_size; i += MIN_ALLOC_OBJECT_SIZE)
    {
      update_alloc_map (heap->alloc_map, dest_offset_from_start + i, true);
    }

  /* check if object is already at destination */
  if (origin == dest)
    {
      /* skip moving object to same location */
      heap->used_bytes += alloc_size - sizeof (header_t);
      heap->next_empty_mem_segment += alloc_size;
      return false;
    }
  /* destination != origin */

  /* Set origin header as allocated to prevent destruction in later stages */
  size_t origin_offset_from_start = calc_heap_offset (origin, heap);
  update_alloc_map (heap->alloc_map, origin_offset_from_start, true);

  memmove (dest, origin, (alloc_size));

  /* increase used_bytes metric */
  heap->used_bytes += alloc_size - sizeof (header_t);

  /* increase bump pointer to allocation dest */
  heap->next_empty_mem_segment += sizeof (header_t);

  /* set old allocation header to a forwarding to the new allocation */
  // FIXME: if dest is less than alloc_size to the left the header pointer will overwrite data in allocation.
  // Fix could be to put forwarding somewhere else entierly than the heap, or by checking destination distance.
  *header_ptr
      = set_header_forwarding_address ((header_t)heap->next_empty_mem_segment);

  /* increase bump pointer to next allocation spot */
  heap->next_empty_mem_segment += alloc_size - sizeof (header_t);

  return true;
}

/**
 * @brief updates all pointers found when tracing using roots to thier new
 * location when following the forwarding adress.
 * TODO: can not forward stack adresses that point to roots. This is needed for safe stack
 * @param h heap which all objects exist within
 * @param roots queue of all roots
 */
void
update_forwarded_pointers (heap_t *h, ptr_queue_t *roots)
{
  if (h->is_unsafe_stack)
  {
    ptr_queue_t *visited = create_ptr_queue ();
    for (size_t i = 0; i < get_len (roots); i++)
      {
        void *ptr_to_root_obj = get_ptr (roots, i);
        if (enqueue_ptr (visited, ptr_to_root_obj))
          {
            apply_to_pointer_in_allocation (ptr_to_root_obj, NULL, visited);
          }
      }
    destroy_ptr_queue (visited);
  }
  else
  {
    // FIXEME: Add support for safe stack.
  }
  
}

/**
 * fetches header of object, checks if object contains any pointers.
 * If object contains pointers it will check if that pointer has been forwarded
 * by compact_objects(). If pointer was forwarded then it sets pointer to the
 * forwarded location. It then checks the forwarded location in the same way.
 *
 * This will result in recursivly (depth first) travesring the heap and
 * updating all pointers that were forwarded.
 */
static void
apply_to_pointer_in_allocation (void *object, apply_to_ptr_func *fun,
                                void *other)
{
  /* get allocation */
  void *alloc = object;
  ptr_queue_t *visited = (ptr_queue_t *)other;

  /* get all pointers inside of allocation */
  ptr_queue_t *internal_pointers = get_pointers_in_allocation (alloc);

  /* retrieve pointer to first internal pointer */
  void **internal_pointer = dequeue_ptr (internal_pointers);
  while (internal_pointer != NULL)
    {
      /* check if it is a designated pointer, but is not set yet */
      if (*internal_pointer == NULL)
        {
          /* check next pointer in struct */
          internal_pointer = dequeue_ptr (internal_pointers);
          continue;
        }

      /* Check header of child pointer */
      header_t child_obj_header
          = get_header_value (get_header_pointer (*internal_pointer));
      header_type_t type = get_header_type (child_obj_header);
      /* Check if child has been forwarded */
      if (type == HEADER_FORWARDING_ADDRESS)
        {
          /* update child ref to the forwarded address */
          *internal_pointer = get_pointer_in_header (child_obj_header);
        }

      /* check if allocation has been visited before */
      if (enqueue_ptr (visited, *internal_pointer))
        {
          /* it has not, update its internal pointers */
          apply_to_pointer_in_allocation (*internal_pointer, fun, other);
        }

      /* go to next internal pointer of current struct */
      internal_pointer = dequeue_ptr (internal_pointers);
    }
}

void
remove_forwarding_allocations (heap_t *h, ptr_queue_t *compaction_queue)
{
  /* retrieve pointer to first pointer */
  void **forwarded_alloc = dequeue_ptr (compaction_queue);
  while (forwarded_alloc != NULL)
    {
      /* get offset for previous object location */
      header_t *forwarded_obj_header = get_header_pointer (forwarded_alloc);

      /* get type of header */
      header_type_t type
          = get_header_type (get_header_value (forwarded_obj_header));

      /* if header is a HEADER_FORWARDING_ADDRESS we need to free its
       * allocation */
      if (type == HEADER_FORWARDING_ADDRESS)
        {
          size_t forwarded_obj_offset
              = calc_heap_offset (forwarded_obj_header, h);

          /* set root object position as allocated */
          update_alloc_map (h->alloc_map, forwarded_obj_offset, false);
        }
      /* if header is NOT a HEADER_FORWARDING_ADDRESS, the object was not
       * forwarded */

      /* go to next pointer of current struct */
      forwarded_alloc = dequeue_ptr (compaction_queue);
    }
}

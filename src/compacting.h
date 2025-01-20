/**
 * Utility functions for finding and compacting living objects.
 */

#pragma once

#include "heap.h"
#include "ptr_queue.h"

/**
 * @brief Finds all pointers in the stack
 * that points to an allocation object in heap
 * @param  EMPTY
 * @return A queue of pointers to allocations
 * starting with the object closets to heap_start
 */
ptr_queue_t *find_root_pointers (heap_t *h, uintptr_t start, uintptr_t end);

/**
 * @brief Traces all living objects from queue of root pointers.
 * for each root object it will check allocation header for anyt internal
 * pointers. If a internal pointer was found it will recursively search
 * internal pointer for more. Each allocation pointer found is enqueued into a
 * priority queue with the allocation closest to heap start first.
 * @param h the heap in which root objects exist in
 * @param roots a queue of roots to trace heap from.
 * @return a queue of all living objects found using given roots.
 */
ptr_queue_t *find_living_objects (heap_t *h, ptr_queue_t *roots);

/**
 * @brief iterates through the compacting queue moving each object found to the
 * earliest available spot in the heap. Due to the way we use forwarding
 * addresses a allocation of 16 bytes will be reserved for each allocation
 * moved. If allocations new position would be the same as the old, nothing is
 * done.
 * @param h heap which objects will be placed in
 * @param roots a queue of roots found, needed if stack is set to unsafe.
 * @param compaction_queue a queue of ALL living objects including roots.
 * @return number of bytes released. (dose not include allocation for
 * forwarding adress)
 */
size_t compact_objects (heap_t *h, ptr_queue_t *roots,
                        ptr_queue_t *compaction_queue);

void update_forwarded_pointers (heap_t *h, ptr_queue_t *roots);

void remove_forwarding_allocations (heap_t *h, ptr_queue_t *compaction_queue);
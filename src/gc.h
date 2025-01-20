/**
 * Main interface of the program.
 * Provides functions to create a heap,
 * destroy a heap and to allocate memory on the created heap.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "heap.h"

typedef uint64_t header_t;

/* Smallest heap possible to allocated.  */
#define MINIMUM_ALIGNMENT 16

/* Size of a page in bytes.  */
#define PAGE_SIZE 2048

/* Heap alignment that should be a power of 2.  */
#define HEAP_ALIGNMENT 16

#define DEBUG_VAL                                                             \
  0xDEADBEEF /* The value to give to stack pointers that pointed toward a     \
                deleted heap value.  */

#ifndef __gc__
/** @brief `global_heap` is set to `NULL` by default.
 * To initialize the heap use the method `h_init` once.
 * When set the global heap will not change unless it
 * is deleted using `h_delete` or `h_delete_dbg`.
 * If the heap was deleted, a new refrence can be created again using `h_init`.
 *
 * Multiple calls can be made to `h_init` but only the first call whilst
 * `global_heap` is set to `NULL` will be saved in the `global_heap`. All
 * sequential calls will only return a pointer refrence to the created heap.
 */
extern heap_t *global_heap;
#endif

/**
 * Create a new heap with bytes total size (including both spaces
 * and metadata), meaning strictly less than bytes will be
 * available for allocation.
 *
 * @param bytes the total size of the heap in bytes
 * @param unsafe_stack true if pointers on the stack are to be considered
 * unsafe pointers
 * @param gc_threshold the memory pressure at which gc should be triggered (1.0
 * = full memory)
 * @return the new heap
 */
heap_t *h_init (size_t bytes, bool unsafe_stack, float gc_threshold);

/**
 * Delete a heap.
 *
 * @param h the heap
 */
void h_delete (heap_t *h);

/**
 * Delete a heap and trace, killing off stack pointers.
 *
 * @param h the heap
 * @param dbg_value a value to be written into every pointer into h on the
 * stack
 */
void h_delete_dbg (heap_t *h, void *dbg_value);

/**
 * Allocate a new object on a heap with a given format string.
 *
 * Valid characters in format strings are:
 * - 'i' -- for sizeof(int) bytes 'raw' data
 * - 'l' -- for sizeof(long) bytes 'raw' data
 * - 'f' -- for sizeof(float) bytes 'raw' data
 * - 'c' -- for sizeof(char) bytes 'raw' data
 * - 'd' -- for sizeof(double) bytes 'raw' data
 * - '*' -- for a sizeof(void *) bytes pointer value
 * - '\0' -- null-character terminates the format string
 * - a number -- indicating repetition, e.g. "3i" is equivalent to "iii"
 * A format string only containing a number will be interpreted
 *     as a size in chars, e. g. "32" is equivalent to "32c"
 *
 * @param h the heap
 * @param layout the format string
 * @return the newly allocated object
 *
 * @note: the heap does *not* retain an alias to layout.
 */
void *h_alloc_struct (heap_t *h, char *layout);

/**
 * Allocate a new object on a heap with a given size.
 *
 * Objects allocated with this function will *not* be
 * further traced for pointers.
 *
 * @param h the heap
 * @param bytes the size in bytes
 * @return the newly allocated object
 */
void *h_alloc_raw (heap_t *h, size_t bytes);

/**
 * Manually trigger garbage collection.
 *
 * Garbage collection is otherwise run when an allocation is
 * impossible in the available consecutive free memory.
 *
 * @param h the heap
 * @return the number of bytes collected
 */
size_t h_gc (heap_t *h);

/* TODO: remove? This does not exit vv */
/**
 * Manually trigger garbage collection with the ability to
 * override the setting for how stack pointers are treated.
 *
 * Garbage collection is otherwise run when an allocation is
 * impossible in the available consecutive free memory.
 *
 * @param h the heap
 * @param unsafe_stack true if pointers on the stack are to be considered
 * unsafe pointers
 * @return the number of bytes collected
 */
size_t h_gc_dbg (heap_t *h, bool unsafe_stack);

/**
 * Returns the available free memory.
 *
 * @param h the heap
 * @return the available free memory
 */
size_t h_avail (heap_t *h);

/**
 * Returns the bytes currently in use by user structures. This
 * should not include the collector's own meta data. Notably,
 * this means that h_avail + h_used will not equal the size of
 * the heap passed to h_init.
 *
 * @param h the heap
 * @return the bytes currently in use by user structures
 */
size_t h_used (heap_t *h);

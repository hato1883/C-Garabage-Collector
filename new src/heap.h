/**
 * Opaque definition of heap_t struct.
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

/**
 * The opaque data type holding all the heap data
 */
typedef struct heap heap_t;

/**
 * @brief Creates a heap with given atleast given size and sets stack safety
 * and threshold
 * @return Newly created stack.
 */
heap_t *create_heap (size_t bytes, bool unsafe_stack, float threshold);

/**
 * @brief Destroys the given heap.
 */
void destroy_heap (heap_t *heap);

/**
 * @brief Calculates how much space is available in the heap using the
 * allocation map. This estimate excludes any data that is used for
 * alignments.
 *
 * @note Whilst the number of available bytes might be high, this dose
 * not equal such a object can be allocated. This is due to fragmentation
 * in the heap leaving spots which will be reported as free but might be
 * impossible to allocate a object on.
 *
 * @param heap Heap to count available space in.
 * @return Number of bytes that can be allocated.
 */
size_t num_free_bytes (heap_t *heap);

/**
 * @brief changes direction of the heap.
 * when heap is set to left_to_right = true the bump pointer will travel from
 * start to the end. When set to false it will instead travel from end to
 * start. This helps by avoid allocating in areas that have been forwarded
 * during h_gc.
 * @param heap Heap to change allocation direction in
 * @note when changing direction the bump pointer will always travel from the
 * start of the edge.
 */
void toggle_direction (heap_t *heap);
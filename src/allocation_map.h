/*
 * Functions for creating an allocation map.
 * Also includes information about minimum object size and allocation map.
 */

#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "heap.h"

/* Minimum object size in bytes. */
#define MIN_ALLOC_OBJECT_SIZE 16

/* Defines how much memory is represented by a boolean in the map.  */
#define BOOLEANS_PER_BYTE 8

/* Defines how much memory is represented by a byte in the map.  */
#define BYTE_DENSITY (MIN_ALLOC_OBJECT_SIZE * BOOLEANS_PER_BYTE)

/**
 * @brief Creates an allocation map for the memory of given amount of bytes.
 * Each of the bytes are represented by one bit in the map. The bytes needs to
 * be divisible by BOOLEANS_PER_BYTE and atleast MIN_OBJECT_SIZE (defined in
 * the .c file).
 * @param bytes the total amount of bytes that that the allocation map will
 * represent
 * @return an allocation map if the allocation was successful, assert happens
 * otherwise.
 */
char *create_allocation_map (size_t bytes);

/**
 * @brief converts a pointer offset from the heap start to allocated object
 * into a allocation map index that is responsible for target memory area.
 * @param offset: offset from the start of the heap. (ptr - heap->start)
 * @returns index of allocation map responsible for target address.
 */
size_t find_index_in_alloc_map (size_t offset);

/**
 * @brief converts a pointer offset from the heap start to allocated object
 * into a allocation map bitmask that is responsible for target memory area.
 * @param offset: offset from the start of the heap. (ptr - heap->start)
 * @returns bitmask that can be used to see if area is allocated (1) or not (0)
 */
char create_bitmask (size_t offset);

/**
 * Sets the bit associated with the heap offset in the allocation map to 0 if
 * is_allocated is false and 1 if it is true.
 * @param alloc_map - the allocation map
 * @param offset - the offset in the allocation map of the bit which is to be
 * changed.
 * @param is_allocated - if the bit should be marked as allocated or not
 */
void update_alloc_map (char *alloc_map, size_t offset, bool is_allocated);

/**
 * Checks if the given offset corresponds to an allocated area in the
 * allocation map.
 * @param alloc_map - the allocation map
 * @param offset - the offset to check
 * @return true if the area the offset corresponds to is allocated.
 */
bool is_offset_allocated (char *alloc_map, size_t offset);

void reset_allocation_map (heap_t *h);

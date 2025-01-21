/**
 * @file
 * Functions for creating an allocation map.
 * Also includes information about minimum object size and allocation map.
 */

#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "heap.h"

/**
 * @def Minimum object size in bytes.
 */
#define MIN_ALLOC_OBJECT_SIZE 8

/**
 * @def Defines how many allocations a single byte can represent.
 */
#define ALLOCATIONS_PER_BYTE 8

/**
 * @def Defines how much memory is represented by a byte in the map.
 */
#define ALLOCATION_MAP_DENSITY (MIN_ALLOC_OBJECT_SIZE * ALLOCATIONS_PER_BYTE)

typedef char *alloc_map_t;

/**
 * @brief Creates an allocation map for the memory of given amount of bytes.
 * Each of the bytes are represented by one bit in the map. The bytes needs to
 * be divisible by ALLOCATIONS_PER_BYTE and atleast MIN_OBJECT_SIZE (defined in
 * the .c file).
 * @param bytes the total amount of bytes that that the allocation map will
 * represent
 * @return an allocation map if the allocation was successful, assert happens
 * otherwise.
 */
alloc_map_t create_allocation_map (size_t bytes);

/**
 * @brief Sets all bytes to 0 in allocation map.
 * @return a cleared allocation map.
 */
void reset_allocation_map (alloc_map_t alloc_map);

/**
 * @brief Converts a pointer offset from the heap start to allocated object
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
void update_alloc_map (alloc_map_t alloc_map, size_t offset,
                       bool is_allocated);

/**
 * @brief Updates an entire range of bits in the allocation map.
 * Useful when needing to update alarge amount of bit in the map after
 * allocating or freeing a object.
 *
 * @param alloc_map Map of all allocated regions in the heap
 * @param start Starting offset within the heap.
 * @param size Size of the allocation that you want to modify.
 * @param is_allocated whether or not to set the range as allocated or not.
 */
void update_alloc_map_range (alloc_map_t alloc_map, size_t start, size_t size,
                             bool is_allocated);

/**
 * @brief Checks if the given offset corresponds to an allocated area in the
 * allocation map.
 * @param alloc_map - the allocation map
 * @param offset - the offset to check
 * @return true if the area the offset corresponds to is allocated.
 */
bool is_offset_allocated (alloc_map_t alloc_map, size_t offset);

/**
 * @brief Finds the first available offset that can store the given amount of
 * data in allocation map. This function dose not change the given allocation
 * map nor heap. To change the allocation map use:
 *
 * update_alloc_map_range()
 *
 * @param alloc_map Map of all allocated region in the heap
 * @param alloc_size Size of the allocation that you want to make in the heap.
 * @param left_to_right Direction to search in, if set to true; the function
 * will search from start to the end. If set to false it will instead search
 * from end to start.
 * @return Offset of next available space that can hold the given allocation
 * size.
 */
size_t find_offset_of_empty_region (alloc_map_t alloc_map, size_t alloc_size,
                                    bool left_to_right, bool *success);

/**
 * @brief Calculates how much space is used in the heap using the allocation
 * map. This estimate includes all allocation metadata and alignment bytes.
 *
 * @param alloc_map Map of all allocated region in the heap
 * @return Number of bytes estimated to have been allocated.
 */
size_t num_allocated_bytes (alloc_map_t alloc_map);

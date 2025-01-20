/**
 * Utility functions regarding creating and modifying a page map.
 */

#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "heap.h"

/* How many bytes each page represents */
#define PAGE_SIZE 2048

/* Defines how much memory is represented by a boolean in the map.  */
#define BOOLEANS_PER_BYTE 8

/* Defines how much memory is represented by a PAGE in the map.  */
#define PAGE_DENSITY (PAGE_SIZE * BOOLEANS_PER_BYTE)

/**
 * Calculates the size of the page map required to represent bytes number of
 * bytes.
 * @param bytes - the amount of bytes to be represented by the page map.
 * @return the size of the page map in bytes.
 */
size_t calc_required_map_size (size_t bytes);

/**
 * @brief Creates an page map for the memory of given amount of bytes.
 * Each page of the heap are represented by one bit in the map.
 * @param bytes the total amount of bytes that that the page map will
 * represent
 * @return an page map if the heap was successful, assert happens
 * otherwise.
 */
char *create_page_map (size_t bytes);

/**
 * Resets the page map by marking all pages as movable.
 * @param page_map the page map
 * @param map_size the size of the map in bytes
 */
void reset_page_map (char *page_map, size_t map_size);

/**
 * @brief converts a pointer offset from the heap start to allocated object
 * into a page map index that is responsible for target memory area.
 * @param offset: offset from the start of the heap. (ptr - heap->start)
 * @returns index of page map responsible for target address.
 */
size_t find_index_in_page_map (size_t offset);

/**
 * @brief converts a pointer offset from the heap start to allocated object
 * into a page map bitmask that is responsible for target memory area.
 * @param offset: offset from the start of the heap. (ptr - heap->start)
 * @returns bitmask that can be used to see if area can be moved (1) or not (0)
 */
char create_page_bitmask (size_t offset);

/**
 * Sets the bit associated with the heap offset in the page map to 0 if
 * is_movable is false and 1 if it is true.
 * @param page_map - the page map
 * @param offset - the offset in the page map of the bit which is to be
 * changed.
 * @param is_movable - if the page should be marked as movable or not
 */
void update_page_map (char *page_map, size_t offset, bool is_movable);

/**
 * Checks if the given offset corresponds to an movable page in the
 * page map.
 * @param page_map - the page map
 * @param offset - the offset to check
 * @return true if the area the offset corresponds to is movable.
 */
bool is_offset_movable (char *page_map, size_t offset);

/**
 * Function to calculate the number of bytes until the next page starts.
 * Returns a number in [1, ..., page_size]
 * @param page_size - size of a page in bytes.
 * @param offset - the offset from the heap start, in bytes.
 * @return the number of bytes until the next page begins.
 */
size_t bytes_from_next_page (size_t page_size, size_t offset);

/**
 * Resets the page map associated with the heap by marking all pages as
 * movable.
 * @param h the heap which page map is to be reset
 */
void h_reset_page_map (heap_t *h);


/**
 * Given two addresses moves the data from source to destination. The function
 * uses metadata in the header to know how much data to copy and what data to
 * copy.
 * The function does not check and simply overwrites data at destination.
 * After data have successfully been moved it changes the header in the source.
 * (forwarding header)
 */

#pragma once

#include "format_encoding.h"
#include "get_header.h"
#include "heap.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * Moves a supposed allocation with header data from one address in memory to
 * another. Also writes a pointer to the new address in the old location's
 * header.
 * @param origin - the pointer to original address of some data.
 * @param destination - the new address to move the data.
 * @return true if the data was successfully moved, false if not.*/
bool move_alloc (void **origin, void *destination);

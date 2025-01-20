#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/gc.h"

void gc_benchmark (void);
#define UNUSED(x) x __attribute__ ((__unused__))

int
main (void)
{
  gc_benchmark ();

  return 0;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void
gc_benchmark (void)
{
  size_t header_size = 8;
  size_t obj_size = sizeof (long);
  size_t total_size __attribute__ ((__unused__)) = obj_size + header_size;

  // heap of size 8 KB
  size_t heap_size = 2048 * 4;

  for (size_t i = 0; i < 20; i++)
    {
      heap_t *h = h_init (heap_size, true, 0.5f);

      // allocate 4 MB of data in heap triggering GC where needed
      for (size_t i = 0; i < 262144; i++)
        {
          h_alloc_raw (h, obj_size);
        }

      h_delete (h);
    }
}
#pragma GCC pop_options

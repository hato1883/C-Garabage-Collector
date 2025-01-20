#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/gc.h"

void gc_benchmark (void);

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
  size_t heap_size __attribute__ ((__unused__)) = 1024 * 1024 * 8;

  size_t header_size = 8;
  size_t obj_size = sizeof (long);
  size_t total_size = obj_size + header_size;

  for (size_t i = 0; i < 20; i++)
    {
      for (size_t i = 0; i < 262144; i++)
        {
          void *ptr = malloc (obj_size);
          free (ptr);
        }
    }
}
#pragma GCC pop_options

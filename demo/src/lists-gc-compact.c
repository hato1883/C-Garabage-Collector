#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../lib/common.h"
#include "../../lib/linked_list.h"
#include "../../src/gc.h"
#include "lists-gc.h"

static void gc_benchmark (size_t m, size_t n);
static bool size_t_eq (elem_t a, elem_t b);
static size_t random_numer (size_t min, size_t max);

int
main (int argc, char *argv[])
{
  if (argc > 2)
    {
      size_t m = atoll (argv[1]);
      size_t n = atoll (argv[2]);
      gc_benchmark (m, n);
    }
  else
    {
      abort ();
    }

  return 0;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
static void
gc_benchmark (size_t m, size_t n)
{
  heap_t *h = h_init (16 * m + 2048, true, 1.0f);

  ioopm_list_t *first = NULL;
  ioopm_list_t *secound = NULL;
  ioopm_list_t *third = NULL;
  ioopm_list_t *forth = NULL;
  ioopm_linked_list_create (&first, size_t_eq);
  ioopm_linked_list_create (&secound, size_t_eq);
  ioopm_linked_list_create (&third, size_t_eq);
  ioopm_linked_list_create (&forth, size_t_eq);

  for (size_t i = 0; i < m; i++)
    {
      size_t number = random_numer (first_start, fith_start - 1);
      if (number < secound_start)
        {
          ioopm_linked_list_append (first, size_to_elem (number));
        }
      else if (number >= secound_start && number < third_start)
        {
          ioopm_linked_list_append (secound, size_to_elem (number));
        }
      else if (number >= third_start && number < forth_start)
        {
          ioopm_linked_list_append (third, size_to_elem (number));
        }
      else if (number >= forth_start && number < fith_start)
        {
          ioopm_linked_list_append (forth, size_to_elem (number));
        }
      else
        {
          i--;
        }
    }

  h_gc (h);

  bool contains_elem = false;
  for (size_t i = 0; i < n; i++)
    {
      size_t number = random_numer (first_start, fith_start - 1);
      if (number < secound_start)
        {
          contains_elem = false;
          ioopm_linked_list_contains (&contains_elem, first,
                                      size_to_elem (number));
        }
      else if (number >= secound_start && number < third_start)
        {
          contains_elem = false;
          ioopm_linked_list_contains (&contains_elem, secound,
                                      size_to_elem (number));
        }
      else if (number >= third_start && number < forth_start)
        {
          contains_elem = false;
          ioopm_linked_list_contains (&contains_elem, third,
                                      size_to_elem (number));
        }
      else if (number >= forth_start && number < fith_start)
        {
          contains_elem = false;
          ioopm_linked_list_contains (&contains_elem, forth,
                                      size_to_elem (number));
        }
      else
        {
          i--;
        }
    }
  h_delete (h);
}
#pragma GCC pop_options

/// @brief Compares two size_t elem_t union instances for equality
/// @param a First size_t union instance
/// @param b Second size_t union instance
/// @return true if the two numbers are equal, else false
static bool
size_t_eq (elem_t a, elem_t b)
{
  return elem_to_size (a) == elem_to_size (b);
}

/// @brief Compares two size_t elem_t union instances for equality
/// @param a First size_t union instance
/// @param b Second size_t union instance
/// @return true if the two numbers are equal, else false
static size_t
random_numer (size_t min, size_t max)
{
  static bool init = true;
  if (init)
    {
      init = false;
      srand (time (NULL));
    }
  size_t upper_half = ((size_t)rand ()) << 32;
  size_t lower_half = ((size_t)rand ());
  size_t combined = upper_half | lower_half;
  size_t bound = combined % ((max + 1) - min) + min;
  return bound;
}

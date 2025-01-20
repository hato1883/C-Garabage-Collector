/**
 * @file common.h
 * @author Hampus Toft & Vilja Schnell Melander
 * @date 17 september 2024
 * @brief Common declarations for hash_table.c, linked_list.c and iterator.c
 */

#pragma once
#include <stdbool.h>

typedef union elem elem_t;

union elem
{
  int integer;
  unsigned int unsigned_integer;
  long long_value;
  unsigned long unsigned_long;
  size_t size;
  bool boolean;
  float floating_point;
  double double_point;
  char *string;
  void *pointer;
};

#define elem_to_int(x) ((int)(x).integer)
#define int_to_elem(x)                                                        \
  (elem_t) { .integer = (x) }

#define elem_to_uint(x) ((unsigned int)(x).unsigned_integer)
#define uint_to_elem(x)                                                       \
  (elem_t) { .unsigned_integer = (x) }

#define elem_to_long(x) ((long)(x).long_value)
#define long_to_elem(x)                                                       \
  (elem_t) { .long_value = (x) }

#define elem_to_ulong(x) ((unsigned long)(x).unsigned_long)
#define ulong_to_elem(x)                                                      \
  (elem_t) { .unsigned_long = (x) }

#define elem_to_size(x) ((size_t)(x).size)
#define size_to_elem(x)                                                       \
  (elem_t) { .size = (x) }

#define elem_to_bool(x) ((bool)(x).boolean)
#define bool_to_elem(x)                                                       \
  (elem_t) { .boolean = (x) }

#define elem_to_float(x) ((float)(x).floating_point)
#define float_to_elem(x)                                                      \
  (elem_t) { .floating_point = (x) }

#define elem_to_double(x) ((double)(x).double_point)
#define double_to_elem(x)                                                     \
  (elem_t) { .double_point = (x) }

#define elem_to_string(x) ((char *)(x).string)
#define string_to_elem(x)                                                     \
  (elem_t) { .string = (x) }

#define elem_to_ptr(x) ((void *)(x).pointer)
#define ptr_to_elem(x)                                                        \
  (elem_t) { .pointer = (x) }

#define SUCCESS 0
#define UNSPECIFIED_FAILURE 1
#define MEMORY_ALLOCATION_FAILURE 2
#define INVALID_RETURN_POINTER 3

typedef bool ioopm_eq_function (elem_t a, elem_t b);

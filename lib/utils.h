/**
 * @file utils.h
 * @author Frans Örneholm
 * @date 14 oktober 2024
 * @brief Collection of utilities for handling user I/O.
 */

#pragma once

#include <stdbool.h>
#include <string.h>

#include "../src/heap.h"
#include "common.h"

extern char *strdup (const char *);
char *heap_strdup (const char *str);

typedef bool check_func (char *);

typedef elem_t convert_func (char *);

elem_t ask_question (char *question, check_func *check, convert_func *convert);
void print (char *string);
void println (char *string);
int read_string (char *buf, int buf_siz);
bool is_number (char *str);
int ask_question_int (char *question);
char *ask_question_string (char *question);
void clear_input_buffer (void);
bool not_empty (char *str);
int copy_string (char *from, char *to, int buf_siz);
int put_last (char *ch, char *arr, int buf_siz);


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../src/gc.h"
#include "utils.h"
#define BUF_SIZE 255

extern char *strdup (const char *);

char *
heap_strdup (const char *str)
{
  size_t len = strlen (str) + 1;
  char *copy = (char *)h_alloc_raw (global_heap, len);
  if (copy != NULL)
    {
      strncpy (copy, str, len);
    }
  return copy;
}

bool
not_empty (char *str)
{
  return strlen (str) > 0;
}

bool
is_number (char *str)
{
  for (size_t i = 0; i < strlen (str); i += 1)
    {
      if (i == 0 && str[0] == 45)
        {
          i += 1;
        }
      if (!(isdigit (str[i])))
        {
          return false;
        }
    }
  return true;
}

int
read_string (char *buf, int buf_siz)
{
  int ch;
  for (int i = 0; i < buf_siz; i++)
    {
      ch = getchar ();
      if (ch != '\n' && ch != EOF)
        {
          buf[i] = ch;
        }
      else
        {
          buf[i] = '\0';
          // clear_input_buffer();
          return i;
        }
    }
  // clear_input_buffer();

  return buf_siz;
}

elem_t
ask_question (char *question, check_func *check, convert_func *convert)
{
  char buffer[BUF_SIZE];
  do
    {
      printf ("%s", question);
      read_string (buffer, BUF_SIZE);
    }
  while (!check (buffer));
  // Check if stdin was redirected
  // if (!isatty (0))
  //   {
  //     // if it was, then we print input to terminal
  //     puts (buffer);
  //   }

  elem_t answ = convert (buffer);
  return answ;
}

elem_t
make_str (char *str)
{
  return string_to_elem (heap_strdup (str));
}

elem_t
make_int (char *str)
{
  return int_to_elem (atoi (str));
}

char *
ask_question_string (char *question)
{
  elem_t answer = ask_question (question, not_empty, make_str);
  return elem_to_string (answer);
}

int
ask_question_int (char *question)
{
  elem_t answer = ask_question (question, is_number, make_int);

  return elem_to_int (answer);
}

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/fifo_queue.h"
#include "ui_mocking.h"

typedef struct string_mock_wraper string_mock_wrapper_t;

struct string_mock_wraper
{
  char *string;
  char *next_char;
};

/// @brief which string that has been printed to the terminal
ioopm_queue_t *output_strings;

/// @brief which strings to replace input with
ioopm_queue_t *input_strings;

/// @brief which strings to replace input with
static bool print_to_terminal_enabled;
static bool enabled;

extern int __real_printf (const char *const _Format, ...);
extern int __real_puts (const char *_Buffer);
extern int __real_putc (int _Character, FILE *_Stream);
extern int __real_getchar (void);

/// @brief capture string sent to printf and saves it in the fifo queue
/// @param _Format format string to use
/// @return status of printf
int
__wrap_printf (const char *const _Format, ...)
{
  // Declare result string that will hold format string with substituted
  // variables
  char *result_string = calloc (1024, sizeof (char));

  // Declare a va_list type variable
  va_list args;

  // Initialise the va_list variable with the ... after _Format
  va_start (args, _Format);

  // Forward the '...' to vasprintf to inline variables in format string
  int ret = vsprintf (result_string, _Format, args);
  // int ret = vprintf(_Format, myargs);

  if (print_to_terminal_enabled)
    {
      fputs (result_string, stdout);
    }
  // NOTE: vasprintf heap allocates result_string

  // Save format string to our queue
  if (enabled)
    {
      ioopm_queue_enqueue (output_strings, string_to_elem (result_string));
    }

  /* Clean up the va_list */
  va_end (args);

  return ret;
}

/// @brief capture string sent to puts and saves it in a fifo queue
/// @param _Buffer string to print
/// @return status of puts
int
__wrap_puts (const char *_Buffer)
{
  int result = 0;
  if (print_to_terminal_enabled)
    {
      result = __real_puts (_Buffer);
    }
  if (enabled)
    {
      ioopm_queue_enqueue (output_strings, string_to_elem (strdup (_Buffer)));
    }
  // puts was allowed to run normaly
  // return __real_puts(_Buffer);
  return result;
}

/// @brief capture string sent to puts and saves it in a fifo queue
/// @param _Buffer string to print
/// @return status of puts
int
__wrap_putc (int _Character, FILE *_Stream)
{
  // ioopm_queue_enqueue(output_strings, string_to_elem(strdup(_Buffer));
  //  puts was allowed to run normaly
  // return __real_puts(_Buffer);
  //__real_printf("Printing char: %c using putc\n", _Character);
  int result = 0;
  if (print_to_terminal_enabled)
    {
      result = __real_putc (_Character, _Stream);
    }
  return result;
}

/// @brief replaces getchar and uses a queue filled with string inputs
/// @return character from input queue
int
__wrap_getchar (void)
{
  if (enabled)
    {
      elem_t mocked_input_source;
      ioopm_queue_peek (&mocked_input_source, input_strings);

      string_mock_wrapper_t *mocking_struct
          = ((string_mock_wrapper_t *)elem_to_ptr (mocked_input_source));
      char *mocked_string_input = mocking_struct->next_char;

      // copy char to local var to avoid invalid reads
      char current_char = *mocked_string_input;

      // Check if current string is empty
      if (current_char == '\0')
        {
          // remove empty string from queue
          // input string is stack allocated
          ioopm_queue_dequeue (&mocked_input_source, input_strings);
          mocking_struct
              = (string_mock_wrapper_t *)elem_to_ptr (mocked_input_source);
          free (mocking_struct);

          // our read_line assumes \n or EOF not a \0
          // get next string to read
          ioopm_queue_peek (&mocked_input_source, input_strings);
          mocking_struct
              = (string_mock_wrapper_t *)elem_to_ptr (mocked_input_source);
          mocked_string_input = mocking_struct->next_char;
          current_char = *mocked_string_input;
        }

      // Increment pointer
      mocking_struct->next_char++;
      return current_char;
    }
  else
    {
      return __real_getchar ();
    }
}

void
io_mock_init (void)
{
  ioopm_queue_create (&output_strings);
  ioopm_queue_create (&input_strings);
  print_to_terminal_enabled = true;
  enabled = true;
}

void
io_mock_destroy (void)
{
  // Output string holds heap allocated memory that must be freed by us.
  io_mock_reset_output ();
  ioopm_queue_destroy (&output_strings);

  // Input string holds heap allocated memory that must be freed by us.
  io_mock_reset_input ();
  ioopm_queue_destroy (&input_strings);
  enabled = false;
}

void
io_mock_enable_printing (void)
{
  print_to_terminal_enabled = true;
}

void
io_mock_disable_printing (void)
{
  print_to_terminal_enabled = false;
}

void
io_mock_enable (void)
{
  enabled = true;
}

void
io_mock_disable (void)
{
  enabled = false;
}

void
io_mock_reset_input (void)
{
  // Input strings holds heap allocated memory that must be freed by us.
  ioopm_iterator_t *input_iter = NULL;
  ioopm_queue_iterator (&input_iter, input_strings);

  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, input_iter) == SUCCESS
         && has_next)
    {
      elem_t next;
      ioopm_iterator_next (&next, input_iter);
      free (elem_to_string (next));
    }

  ioopm_iterator_destroy (&input_iter);
  ioopm_queue_clear (input_strings);
}

void
io_mock_reset_output (void)
{
  // Output string holds heap allocated memory that must be freed by us.
  ioopm_iterator_t *output_iter = NULL;
  ioopm_queue_iterator (&output_iter, output_strings);

  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, output_iter) == SUCCESS
         && has_next)
    {
      elem_t next;
      ioopm_iterator_next (&next, output_iter);
      free (elem_to_string (next));
    }
  ioopm_iterator_destroy (&output_iter);
  ioopm_queue_clear (output_strings);
}

void
io_mock_add_input (char *str)
{
  string_mock_wrapper_t *mock_string = malloc (sizeof (string_mock_wrapper_t));
  *mock_string = (string_mock_wrapper_t){ .string = str, .next_char = str };
  ioopm_queue_enqueue (input_strings, ptr_to_elem (mock_string));
}

char *
io_mock_peek_output (void)
{
  elem_t next;
  ioopm_queue_peek (&next, output_strings);
  return elem_to_string (next);
}

char *
io_mock_pop_output (void)
{
  elem_t next;
  ioopm_queue_dequeue (&next, output_strings);
  return elem_to_string (next);
}

size_t
io_mock_output_length (void)
{
  size_t size;
  ioopm_queue_size (&size, output_strings);
  return size;
}

size_t
io_mock_input_length (void)
{
  size_t size;
  ioopm_queue_size (&size, input_strings);
  return size;
}

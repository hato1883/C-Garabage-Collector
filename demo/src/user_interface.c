#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../lib/utils.h"
#include "../../src/gc.h"
#include "user_interface.h"
#include "webstore.h"

static bool positive_confirmation (char *msg);
static bool negative_confirmation (char *msg);
static bool get_confirmation (char *msg, char *valid, bool invert_result);
static bool answer_starts_with_valid (char *input, char *valid_options);
static void print_menu (void);
static storage_location_id_t ask_question_location (char *question);
static bool is_valid_location_id (char *input);
static elem_t make_location_id (char *str);

merch_t *
ioopm_ask_merch (void)
{
  merch_name_t name = ioopm_ask_merch_name ();
  char *desc = ioopm_ask_merch_description ();
  int price = ioopm_ask_merch_price ();
  merch_t *merch = ioopm_create_merch (name, desc, price);
  // free (name);
  // free (desc);
  return merch;
}

void
ioopm_print_cart_cost (shopping_cart_id_t cart_id, int total_cost)
{
  printf ("Shopping cart %ld has items with a total cost of: %d kr\n", cart_id,
          total_cost);
}

void
ioopm_print_receipt_header (shopping_cart_id_t cart_id)
{
  printf ("Checking out shopping cart %ld\n", cart_id);
}

void
ioopm_print_receipt_entry (merch_name_t merch_name,
                           stock_quantity_t merch_quantity, int merch_cost)
{
  if (merch_quantity > 1)
    {
      printf ("%s: (%ld * %d kr) %ld kr\n", merch_name, merch_quantity,
              merch_cost, (merch_cost * merch_quantity));
    }
  else
    {
      printf ("%s: %d kr\n", merch_name, merch_cost);
    }
}

void
ioopm_print_receipt_footer (int total_cost)
{
  printf ("Total %d kr\n", total_cost);
}

merch_name_t
ioopm_ask_merch_name (void)
{
  return ask_question_string ("Name of merchandise: ");
}

char *
ioopm_ask_merch_description (void)
{
  return ask_question_string ("Merchandise description: ");
}

int
ioopm_ask_merch_price (void)
{
  int price = 0;
  do
    {
      if (price < 0)
        {
          printf ("The price must be greater or equal to 0!\n");
        }
      price = ask_question_int ("Price of merchandise: ");
    }
  while (price < 0);
  return price;
}

shopping_cart_id_t
ioopm_ask_cart_id (void)
{
  int cart_id = 0;
  do
    {
      if (cart_id < 0)
        {
          printf ("Cart number must be greater or equal to 0!\n");
        }
      cart_id = ask_question_int ("Cart number: ");
    }
  while (cart_id < 0);
  return (shopping_cart_id_t)cart_id;
}

storage_location_id_t
ioopm_ask_location (void)
{
  return ask_question_location ("Location name: ");
}

stock_quantity_t
ioopm_ask_quantity (void)
{
  int quantity = 1;
  do
    {
      if (quantity <= 0)
        {
          printf ("The Quantity must be greater than 0!\n");
        }
      quantity = ask_question_int ("Quantity: ");
    }
  while (quantity <= 0);
  return quantity;
}

void
ioopm_print_merch (merch_t *merch)
{
  char *name = ioopm_get_merch_name (merch);
  char *desc = ioopm_get_merch_desc (merch);
  int price = ioopm_get_merch_price (merch);
  printf ("%s, %s : %d kr\n", name, desc, price);
}

bool
ioopm_should_print_more (void)
{
  return negative_confirmation ("Continue printing?: ");
}

bool
ioopm_should_remove_merch (void)
{
  return positive_confirmation (
      "Are you sure you want to remove this item?: ");
}

bool
ioopm_should_edit_merch (void)
{
  return positive_confirmation ("Are you sure you want to edit this item?: ");
}

bool
ioopm_should_remove_cart (void)
{
  return positive_confirmation (
      "Are you sure you want to remove this shopping cart?: ");
}

void
ioopm_print_storage_location (storage_location_id_t location_id,
                              stock_quantity_t stock_quantity)
{
  printf ("%s: %lu\n", location_id, stock_quantity);
}

char
ioopm_get_menu_choice (void)
{
  print_menu ();
  char *user_choice = ask_question_string ("Menu Choice: ");
  char first_char = user_choice[0];
  // free (user_choice);
  return toupper (first_char);
}

/// @brief Prompts user with a given question and returns true or false
/// @param msg question to prompt user with
/// @return true if the answer starts with Y or y else false
static bool
positive_confirmation (char *msg)
{
  return get_confirmation (msg, "Yy", false);
}

/// @brief Prompts user with a given question and returns true or false
/// @param msg question to prompt user with
/// @return true if the answer dose NOT start with N or n else false
static bool
negative_confirmation (char *msg)
{
  return get_confirmation (msg, "Nn", true);
}

/// @brief Prompts user with a message and checks if the first character is one
/// of the valid options
/// @param msg message to prompt user with
/// @param valid array of valid first characters
/// @param invert_result if the result should get inverted (true inverts
/// result)
/// @return true if first character exist in valid char array, returns false if
/// it dose not. invert_result flips the result if set to true.
static bool
get_confirmation (char *msg, char *valid, bool invert_result)
{
  char *user_choice = ask_question_string (msg);
  bool result = answer_starts_with_valid (user_choice, valid);
  // free (user_choice);
  if (invert_result)
    {
      result = !result;
    }
  return result;
}

/// @brief Checks if the given input starts with one of the accepted characters
/// in the char array `valid_options`
/// @param input input string to check in
/// @param valid_options array of accepted characters
/// @return true if first character exist in valid_options char array.
static bool
answer_starts_with_valid (char *input, char *valid_options)
{
  char *valid_char_cursor = valid_options;
  while (*valid_char_cursor != '\0')
    {
      if (input[0] == *valid_char_cursor)
        {
          return true;
        }
      valid_char_cursor++;
    }
  return false;
}

static void
print_menu (void)
{
  puts ("\n============ Webstore Menu ============\n"
        "[A] Add a new merchandise to the store\n"
        "[L] List merchandise in the store\n"
        "[E] Edit a existing merchandise in the store\n"
        "[S] Show stock of a specific merchandise\n"
        "[P] Replenish stock of a given merchandise\n"
        "[C] Create a new shopping cart\n"
        "[+] Add merchandise to a shopping cart\n"
        "[-] Remove merchandise from a shopping cart\n"
        "[=] Calculate the total cost of a shopping cart\n"
        "[O] Checkout a shopping cart\n"
        "[D] Remove existing merchandise from the store\n"
        "[R] Remove an existing shopping cart\n"
        "[U] Undo previous action (NOT IMPLEMENTED)\n"
        "[Q] Quit\n");
}

static storage_location_id_t
ask_question_location (char *question)
{
  elem_t ans = ask_question (question, is_valid_location_id, make_location_id);
  return (storage_location_id_t)elem_to_string (ans);
}

#define LETTERS_IN_LOCATION_ID 1
#define DIGITS_IN_LOCATION_ID 2

/// @brief Checks if the given string matches format needed to be a
/// storage_location_id_t.
/// @param str the string to check.
/// @return true if string can be converted into a storage_location_id_t.
static bool
is_valid_location_id (char *str)
{
  if (strlen (str) != DIGITS_IN_LOCATION_ID + LETTERS_IN_LOCATION_ID)
    {
      return false;
    }
  for (size_t letter_index = 0; letter_index < LETTERS_IN_LOCATION_ID;
       letter_index++)
    {
      char letter = str[letter_index];
      if (!isalpha (letter))
        {
          return false;
        }
    }
  for (size_t digit_index = LETTERS_IN_LOCATION_ID;
       digit_index < DIGITS_IN_LOCATION_ID + LETTERS_IN_LOCATION_ID;
       digit_index++)
    {
      char digit = str[digit_index];
      if (!isdigit (digit))
        {
          return false;
        }
    }
  return true;
}

/// @brief Checks if the given string matches format needed to be a
/// storage_location_id_t.
/// @param str the string to check.
/// @return true if string can be converted into a storage_location_id_t.
static elem_t
make_location_id (char *str)
{
  char *allocated_str = heap_strdup (str); // Använder heap_ref
  return string_to_elem (allocated_str);
}

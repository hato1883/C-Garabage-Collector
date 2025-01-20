#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/gc.h"
#include "../src/user_interface.h"
#include "../src/webstore.h"
#include "ui_mocking.h"

int
init_suite (void)
{
  h_init (1024 * 32, true, 0.75f);
  // Change this function if you want to do something *before* you
  // run a test suite
  return 0;
}

int
clean_suite (void)
{
  h_delete (global_heap);
  // Change this function if you want to do something *after* you
  // run a test suite
  return 0;
}

/// @brief Asks user (mocked_input) for values to create a merch struct
void
ask_merch_accepts_string_string_int (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *name, *desc;
  int price;

  name = "Name";
  desc = "Desc";
  price = 0;

  io_mock_add_input ("Name\n"); // First input
  io_mock_add_input ("Desc\n"); // Second input
  io_mock_add_input ("0\n");    // Third input
  merch_t *merch = ioopm_ask_merch ();

  CU_ASSERT_STRING_EQUAL (name, ioopm_get_merch_name (merch));
  CU_ASSERT_STRING_EQUAL (desc, ioopm_get_merch_desc (merch));
  CU_ASSERT_EQUAL (price, ioopm_get_merch_price (merch));

  ioopm_destroy_merch (&merch);

  // Check question
  char *result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Asks user (mocked_input) for values to create a merch struct
void
ask_merch_rejects_string_string_string (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *name, *desc;
  int price;

  name = "Name";
  desc = "Desc";
  price = 0;

  io_mock_add_input ("Name\n"); // First input attempt 1
  io_mock_add_input ("Desc\n"); // Second input attempt 1
  io_mock_add_input ("abc\n");  // Third input attempt 1
  io_mock_add_input ("0\n");    // Third input attempt 2
  merch_t *merch = ioopm_ask_merch ();

  CU_ASSERT_STRING_EQUAL (name, ioopm_get_merch_name (merch));
  CU_ASSERT_STRING_EQUAL (desc, ioopm_get_merch_desc (merch));
  CU_ASSERT_EQUAL (price, ioopm_get_merch_price (merch));

  ioopm_destroy_merch (&merch);

  // Check question
  char *result = io_mock_pop_output (); // First output attempt 1
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Second output attempt 1
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // Third output attempt 1
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Third output attempt 2
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Check that we reject accepts anything tha is non empty
void
ask_merch_name_accepts_any_non_empty_string (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  char *name;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("1\n");

  // Use printf to "print" a simple format
  name = ioopm_ask_merch_name ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (name, "1");
  // free (name);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("Cool Name\n");

  // Use printf to "print" a simple format
  name = ioopm_ask_merch_name ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (name, "Cool Name");
  // free (name);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we reject anything that is empty
void
ask_merch_name_rejects_any_empty_string (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  char *name;
  size_t invalid_input_count = 0;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("\n");
  invalid_input_count++;
  io_mock_add_input ("\n");
  invalid_input_count++;
  io_mock_add_input ("\n");
  invalid_input_count++;
  io_mock_add_input ("Real Name\n");

  // Use printf to "print" a simple format
  name = ioopm_ask_merch_name ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (name, "Real Name");
  // free (name);

  // Check what was printed to terminal
  result = io_mock_pop_output (); // first print
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  for (size_t i = 0; i < invalid_input_count; i++)
    {
      // check if question was repeated
      result = io_mock_pop_output ();
      CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
      free (result);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we reject accepts anything tha is non empty
void
ask_merch_description_accepts_any_non_empty_string (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  char *desc;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("1\n");

  // Use printf to "print" a simple format
  desc = ioopm_ask_merch_description ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (desc, "1");
  // free (desc);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("Cool description\n");

  // Use printf to "print" a simple format
  desc = ioopm_ask_merch_description ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (desc, "Cool description");
  // free (desc);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we reject anything that is empty
void
ask_merch_description_rejects_any_empty_string (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  char *desc;
  size_t invalid_input_count = 0;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("\n");
  invalid_input_count++;
  io_mock_add_input ("\n");
  invalid_input_count++;
  io_mock_add_input ("\n");
  invalid_input_count++;
  io_mock_add_input ("Real description\n");

  // Use printf to "print" a simple format
  desc = ioopm_ask_merch_description ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (desc, "Real description");
  // free (desc);

  // Check what was printed to terminal
  result = io_mock_pop_output (); // first print
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  for (size_t i = 0; i < invalid_input_count; i++)
    {
      // check if question was repeated
      result = io_mock_pop_output ();
      CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
      free (result);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we accept 0
void
ask_merch_price_accepts_zero (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  int price;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("0\n");

  // Use printf to "print" a simple format
  price = ioopm_ask_merch_price ();

  // Check answer received
  CU_ASSERT_EQUAL (price, 0);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we accept 1 (or any larger)
void
ask_merch_price_accepts_one (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  int price;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("1\n");

  // Use printf to "print" a simple format
  price = ioopm_ask_merch_price ();

  // Check answer received
  CU_ASSERT_EQUAL (price, 1);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we reject negative values
void
ask_merch_price_rejects_negative_one (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  int price;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("-1\n");
  io_mock_add_input ("0\n");

  // Use printf to "print" a simple format
  price = ioopm_ask_merch_price ();

  // Check answer received
  CU_ASSERT_EQUAL (price, 0);

  // Check question
  result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  // check error that input was rejected
  result = io_mock_pop_output (); // second printf
  CU_ASSERT_STRING_EQUAL ("The price must be greater or equal to 0!", result);
  free (result);

  // check question reprint
  result = io_mock_pop_output (); // third printf
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we reject non integer values
void
ask_merch_price_rejects_non_int (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  int price;
  size_t invalid_input_count = 0;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("abc\n");
  invalid_input_count++;
  io_mock_add_input ("ABC\n");
  invalid_input_count++;
  io_mock_add_input ("1.23\n");
  invalid_input_count++;
  io_mock_add_input ("-1.23\n");
  invalid_input_count++;
  io_mock_add_input ("123abc\n");
  invalid_input_count++;
  io_mock_add_input ("abc123\n");
  invalid_input_count++;
  io_mock_add_input ("-\n");
  invalid_input_count++;

  io_mock_add_input ("0\n"); // Must have a valid input

  price = ioopm_ask_merch_price ();

  // Check answer received
  CU_ASSERT_EQUAL (price, 0);

  // Check question
  result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);
  for (size_t i = 0; i < invalid_input_count; i++)
    {
      // check if question was repeated
      result = io_mock_pop_output ();
      CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
      free (result);
    }
  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we accept 0
void
ask_cart_id_accepts_zero (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  shopping_cart_id_t cart_id;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("0\n");

  // Use printf to "print" a simple format
  cart_id = ioopm_ask_cart_id ();

  // Check answer received
  CU_ASSERT_EQUAL (cart_id, 0);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we accept 1 (or any larger)
void
ask_cart_id_accepts_one (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  shopping_cart_id_t cart_id;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("1\n");

  // Use printf to "print" a simple format
  cart_id = ioopm_ask_cart_id ();

  // Check answer received
  CU_ASSERT_EQUAL (cart_id, 1);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we reject negative values
void
ask_cart_id_rejects_negative_one (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  shopping_cart_id_t cart_id;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("-1\n");
  io_mock_add_input ("0\n");

  // Use printf to "print" a simple format
  cart_id = ioopm_ask_cart_id ();

  // Check answer received
  CU_ASSERT_EQUAL (cart_id, 0);

  // Check question
  result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  // check error that input was rejected
  result = io_mock_pop_output (); // second printf
  CU_ASSERT_STRING_EQUAL ("Cart number must be greater or equal to 0!",
                          result);
  free (result);

  // check question reprint
  result = io_mock_pop_output (); // third printf
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we reject non integer values
void
ask_cart_id_rejects_non_int (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  shopping_cart_id_t cart_id;
  size_t invalid_input_count = 0;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("abc\n");
  invalid_input_count++;
  io_mock_add_input ("ABC\n");
  invalid_input_count++;
  io_mock_add_input ("1.23\n");
  invalid_input_count++;
  io_mock_add_input ("-1.23\n");
  invalid_input_count++;
  io_mock_add_input ("123abc\n");
  invalid_input_count++;
  io_mock_add_input ("abc123\n");
  invalid_input_count++;
  io_mock_add_input ("-\n");
  invalid_input_count++;

  io_mock_add_input ("0\n"); // Must have a valid input

  cart_id = ioopm_ask_cart_id ();

  // Check answer received
  CU_ASSERT_EQUAL (cart_id, 0);

  // Check question
  result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);
  for (size_t i = 0; i < invalid_input_count; i++)
    {
      // check if question was repeated
      result = io_mock_pop_output ();
      CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
      free (result);
    }
  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we accept a00
void
ask_location_accepts_a00 (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  storage_location_id_t location_id;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("a00\n");

  // Use printf to "print" a simple format
  location_id = ioopm_ask_location ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (location_id, "a00");
  // free (location_id);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we accept Z99
void
ask_location_accepts_Z99 (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  storage_location_id_t location_id;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("Z99\n");

  // Use printf to "print" a simple format
  location_id = ioopm_ask_location ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (location_id, "Z99");
  // free (location_id);

  // Check question
  result = io_mock_pop_output (); // if multiple printf/puts has occured then
                                  // this will be the first one
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we reject empty string
void
ask_location_rejects_empty_string (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  storage_location_id_t location_id;
  size_t invalid_input_count = 0;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("\n");
  invalid_input_count++;

  io_mock_add_input ("A12\n"); // Must have a valid input

  location_id = ioopm_ask_location ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (location_id, "A12");
  // free (location_id);

  // Check question
  result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);
  for (size_t i = 0; i < invalid_input_count; i++)
    {
      // check if question was repeated
      result = io_mock_pop_output ();
      CU_ASSERT_STRING_EQUAL ("Location name: ", result);
      free (result);
    }
  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

/// @brief Check that we reject any invalid formats
void
ask_location_rejects_invalid_format (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *result;
  storage_location_id_t location_id;
  size_t invalid_input_count = 0;

  // set mocked input (read_string from utils end on EOF or \n)
  io_mock_add_input ("000\n");
  invalid_input_count++; // No letter
  io_mock_add_input ("555\n");
  invalid_input_count++; // No letter
  io_mock_add_input ("@12\n");
  invalid_input_count++; // No letter
  io_mock_add_input (".12\n");
  invalid_input_count++; // No letter
  io_mock_add_input ("-12\n");
  invalid_input_count++; // No letter
  io_mock_add_input ("ABC\n");
  invalid_input_count++; // No digits
  io_mock_add_input ("AB0\n");
  invalid_input_count++; // To few digits
  io_mock_add_input ("AB5\n");
  invalid_input_count++; // To few digits
  io_mock_add_input ("00A\n");
  invalid_input_count++; // invalid order
  io_mock_add_input ("00B\n");
  invalid_input_count++; // invalid order
  io_mock_add_input ("55A\n");
  invalid_input_count++; // invalid order
  io_mock_add_input ("55B\n");
  invalid_input_count++; // invalid order
  io_mock_add_input ("A123\n");
  invalid_input_count++; // To long
  io_mock_add_input ("A1\n");
  invalid_input_count++; // To short

  io_mock_add_input ("A12\n"); // Must have a valid input

  location_id = ioopm_ask_location ();

  // Check answer received
  CU_ASSERT_STRING_EQUAL (location_id, "A12");
  // free (location_id);

  // Check question
  result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);
  for (size_t i = 0; i < invalid_input_count; i++)
    {
      // check if question was repeated
      result = io_mock_pop_output ();
      CU_ASSERT_STRING_EQUAL ("Location name: ", result);
      free (result);
    }
  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

void
print_merch_prints_merch_correctly (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  char *name, *desc;
  int price;

  name = "Name";
  desc = "Desc";
  price = 0;
  merch_t *merch = ioopm_create_merch (name, desc, price);

  ioopm_print_merch (merch);
  ioopm_destroy_merch (&merch);

  // Check question
  char *result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL ("Name, Desc : 0 kr\n", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  name = "Smells Like Teen Spirit";
  desc
      = "A song by the American rock band Nirvana. "
        "It is the opening track and lead single from the band's second album";
  price = 583;
  merch = ioopm_create_merch (name, desc, price);

  ioopm_print_merch (merch);
  ioopm_destroy_merch (&merch);

  // Check question
  result = io_mock_pop_output (); // first printf
  CU_ASSERT_STRING_EQUAL (
      "Smells Like Teen Spirit, "
      "A song by the American rock band Nirvana. "
      "It is the opening track and lead single from the band's second album : "
      "583 kr\n",
      result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Check that inputs starting with N or n returns false
void
should_print_more_question_returns_false_on_Nn (void)
{

  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  size_t num_valid_inputs = 0;

  num_valid_inputs++;
  io_mock_add_input ("no\n");
  num_valid_inputs++;
  io_mock_add_input ("No.\n");
  num_valid_inputs++;
  io_mock_add_input ("n\n");
  num_valid_inputs++;
  io_mock_add_input ("N\n");
  num_valid_inputs++;
  io_mock_add_input ("Nobody should stop you from printing more\n");

  for (size_t i = 0; i < num_valid_inputs; i++)
    {
      bool result = ioopm_should_print_more ();
      CU_ASSERT_FALSE (result);

      char *terminal_text = io_mock_pop_output (); // first printf
      CU_ASSERT_STRING_EQUAL (terminal_text, "Continue printing?: ");
      free (terminal_text);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Check that inputs not starting with N or n returns true
void
should_print_more_question_returns_true_on_other (void)
{

  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  size_t num_valid_inputs = 0;

  num_valid_inputs++;
  io_mock_add_input ("yes\n");
  num_valid_inputs++;
  io_mock_add_input ("Yes.\n");
  num_valid_inputs++;
  io_mock_add_input ("y\n");
  num_valid_inputs++;
  io_mock_add_input ("Y\n");
  num_valid_inputs++;
  io_mock_add_input ("ye we need that\n");
  num_valid_inputs++;
  io_mock_add_input ("123456\n");
  num_valid_inputs++;
  io_mock_add_input ("qwerty\n");
  num_valid_inputs++;
  io_mock_add_input (".-.,.-.,\n");
  num_valid_inputs++;
  io_mock_add_input ("!\"!?!¤=!?!=!¤)\n");

  for (size_t i = 0; i < num_valid_inputs; i++)
    {
      bool result = ioopm_should_print_more ();
      CU_ASSERT_TRUE (result);

      char *terminal_text = io_mock_pop_output (); // first printf
      CU_ASSERT_STRING_EQUAL (terminal_text, "Continue printing?: ");
      free (terminal_text);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Check that inputs not starting with N or n returns true
void
should_remove_merch_question_returns_true_on_Yy (void)
{

  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  size_t num_valid_inputs = 0;

  num_valid_inputs++;
  io_mock_add_input ("yes\n");
  num_valid_inputs++;
  io_mock_add_input ("Yes.\n");
  num_valid_inputs++;
  io_mock_add_input ("y\n");
  num_valid_inputs++;
  io_mock_add_input ("Y\n");
  num_valid_inputs++;
  io_mock_add_input ("ye we need that\n");

  for (size_t i = 0; i < num_valid_inputs; i++)
    {
      bool result = ioopm_should_remove_merch ();
      CU_ASSERT_TRUE (result);

      char *terminal_text = io_mock_pop_output (); // first printf
      CU_ASSERT_STRING_EQUAL (terminal_text,
                              "Are you sure you want to remove this item?: ");
      free (terminal_text);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Check that inputs starting with N or n returns false
void
should_remove_merch_question_returns_false_on_other (void)
{

  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  size_t num_valid_inputs = 0;

  num_valid_inputs++;
  io_mock_add_input ("no\n");
  num_valid_inputs++;
  io_mock_add_input ("No.\n");
  num_valid_inputs++;
  io_mock_add_input ("n\n");
  num_valid_inputs++;
  io_mock_add_input ("N\n");
  num_valid_inputs++;
  io_mock_add_input ("Nobody should stop you from printing more\n");
  num_valid_inputs++;
  io_mock_add_input ("123456\n");
  num_valid_inputs++;
  io_mock_add_input ("qwerty\n");
  num_valid_inputs++;
  io_mock_add_input (".-.,.-.,\n");
  num_valid_inputs++;
  io_mock_add_input ("!\"!?!¤=!?!=!¤)\n");

  for (size_t i = 0; i < num_valid_inputs; i++)
    {
      bool result = ioopm_should_remove_merch ();
      CU_ASSERT_FALSE (result);

      char *terminal_text = io_mock_pop_output (); // first printf
      CU_ASSERT_STRING_EQUAL (terminal_text,
                              "Are you sure you want to remove this item?: ");
      free (terminal_text);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Check that inputs not starting with N or n returns true
void
should_edit_merch_question_returns_true_on_Yy (void)
{

  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  size_t num_valid_inputs = 0;

  num_valid_inputs++;
  io_mock_add_input ("yes\n");
  num_valid_inputs++;
  io_mock_add_input ("Yes.\n");
  num_valid_inputs++;
  io_mock_add_input ("y\n");
  num_valid_inputs++;
  io_mock_add_input ("Y\n");
  num_valid_inputs++;
  io_mock_add_input ("ye we need that\n");

  for (size_t i = 0; i < num_valid_inputs; i++)
    {
      bool result = ioopm_should_edit_merch ();
      CU_ASSERT_TRUE (result);

      char *terminal_text = io_mock_pop_output (); // first printf
      CU_ASSERT_STRING_EQUAL (terminal_text,
                              "Are you sure you want to edit this item?: ");
      free (terminal_text);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Check that inputs starting with N or n returns false
void
should_edit_merch_question_returns_false_on_other (void)
{

  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  size_t num_valid_inputs = 0;

  num_valid_inputs++;
  io_mock_add_input ("no\n");
  num_valid_inputs++;
  io_mock_add_input ("No.\n");
  num_valid_inputs++;
  io_mock_add_input ("n\n");
  num_valid_inputs++;
  io_mock_add_input ("N\n");
  num_valid_inputs++;
  io_mock_add_input ("Nobody should stop you from printing more\n");
  num_valid_inputs++;
  io_mock_add_input ("123456\n");
  num_valid_inputs++;
  io_mock_add_input ("qwerty\n");
  num_valid_inputs++;
  io_mock_add_input (".-.,.-.,\n");
  num_valid_inputs++;
  io_mock_add_input ("!\"!?!¤=!?!=!¤)\n");

  for (size_t i = 0; i < num_valid_inputs; i++)
    {
      bool result = ioopm_should_edit_merch ();
      CU_ASSERT_FALSE (result);

      char *terminal_text = io_mock_pop_output (); // first printf
      CU_ASSERT_STRING_EQUAL (terminal_text,
                              "Are you sure you want to edit this item?: ");
      free (terminal_text);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Check that inputs not starting with N or n returns true
void
should_remove_cart_question_returns_true_on_Yy (void)
{

  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  size_t num_valid_inputs = 0;

  num_valid_inputs++;
  io_mock_add_input ("yes\n");
  num_valid_inputs++;
  io_mock_add_input ("Yes.\n");
  num_valid_inputs++;
  io_mock_add_input ("y\n");
  num_valid_inputs++;
  io_mock_add_input ("Y\n");
  num_valid_inputs++;
  io_mock_add_input ("ye we need that\n");

  for (size_t i = 0; i < num_valid_inputs; i++)
    {
      bool result = ioopm_should_remove_cart ();
      CU_ASSERT_TRUE (result);

      char *terminal_text = io_mock_pop_output (); // first printf
      CU_ASSERT_STRING_EQUAL (
          terminal_text,
          "Are you sure you want to remove this shopping cart?: ");
      free (terminal_text);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

/// @brief Check that inputs starting with N or n returns false
void
should_remove_cart_question_returns_false_on_other (void)
{

  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  size_t num_valid_inputs = 0;

  num_valid_inputs++;
  io_mock_add_input ("no\n");
  num_valid_inputs++;
  io_mock_add_input ("No.\n");
  num_valid_inputs++;
  io_mock_add_input ("n\n");
  num_valid_inputs++;
  io_mock_add_input ("N\n");
  num_valid_inputs++;
  io_mock_add_input ("Nobody should stop you from printing more\n");
  num_valid_inputs++;
  io_mock_add_input ("123456\n");
  num_valid_inputs++;
  io_mock_add_input ("qwerty\n");
  num_valid_inputs++;
  io_mock_add_input (".-.,.-.,\n");
  num_valid_inputs++;
  io_mock_add_input ("!\"!?!¤=!?!=!¤)\n");

  for (size_t i = 0; i < num_valid_inputs; i++)
    {
      bool result = ioopm_should_remove_cart ();
      CU_ASSERT_FALSE (result);

      char *terminal_text = io_mock_pop_output (); // first printf
      CU_ASSERT_STRING_EQUAL (
          terminal_text,
          "Are you sure you want to remove this shopping cart?: ");
      free (terminal_text);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

void
get_menu_choice_returns_uppercase_char (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  //  stdout

  char *menu_question = "Menu Choice: ";
  char *menu_string = "\n============ Webstore Menu ============\n"
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
                      "[Q] Quit\n";

  char choice;

  io_mock_add_input ("abcdefg\n");

  choice = ioopm_get_menu_choice ();
  CU_ASSERT_EQUAL (choice, 'A');

  // Check question
  char *result = io_mock_pop_output (); // Menu
  CU_ASSERT_STRING_EQUAL (menu_string, result);
  free (result);

  result = io_mock_pop_output (); // Question
  CU_ASSERT_STRING_EQUAL (menu_question, result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup resources
  io_mock_destroy ();
}

void
test_print_storage_location (void)
{
  // Create mocking resources
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  storage_location_id_t sl_id = "A23";
  stock_quantity_t sc = 55;
  ioopm_print_storage_location (sl_id, sc);
  char *result = io_mock_pop_output ();
  CU_ASSERT_STRING_EQUAL ("A23: 55\n", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup mock resources
  io_mock_destroy ();
}

int
main (void)
{
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite user_interface_test_suite
      = CU_add_suite ("User interface Test Suite", init_suite, clean_suite);
  if (user_interface_test_suite == NULL)
    {
      // If the test suite could not be added, tear down CUnit and exit
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  // This is where we add the test functions to our test suite.
  // For each call to CU_add_test we specify the test suite, the
  // name or description of the test, and the function that runs
  // the test in question. If you want to add another test, just
  // copy a line below and change the information
  if ((CU_add_test (
           user_interface_test_suite,
           "ask_merch accepts any string, string and int input format",
           ask_merch_accepts_string_string_int)
       == NULL)
      || (CU_add_test (
              user_interface_test_suite,
              "ask_merch rejects any string, string and non int input format",
              ask_merch_rejects_string_string_string)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_merch_name accepts any non empty string",
                       ask_merch_name_accepts_any_non_empty_string)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_merch_name rejects any empty string",
                       ask_merch_name_rejects_any_empty_string)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_merch_description accepts any non empty string",
                       ask_merch_description_accepts_any_non_empty_string)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_merch_description rejects any empty string",
                       ask_merch_description_rejects_any_empty_string)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_merch_price accepts int 1",
                       ask_merch_price_accepts_one)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_merch_price accepts int 0",
                       ask_merch_price_accepts_zero)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_merch_price rejects int -1",
                       ask_merch_price_rejects_negative_one)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_merch_price rejects any non integer answer",
                       ask_merch_price_rejects_non_int)
          == NULL)
      || (CU_add_test (user_interface_test_suite, "ask_cart_id accepts int 1",
                       ask_cart_id_accepts_one)
          == NULL)
      || (CU_add_test (user_interface_test_suite, "ask_cart_id accepts int 0",
                       ask_cart_id_accepts_zero)
          == NULL)
      || (CU_add_test (user_interface_test_suite, "ask_cart_id rejects int -1",
                       ask_cart_id_rejects_negative_one)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_cart_id rejects any non integer answer",
                       ask_cart_id_rejects_non_int)
          == NULL)
      || (CU_add_test (user_interface_test_suite, "ask_location accepts a00",
                       ask_location_accepts_a00)
          == NULL)
      || (CU_add_test (user_interface_test_suite, "ask_location accepts Z99",
                       ask_location_accepts_Z99)
          == NULL)
      || (CU_add_test (user_interface_test_suite, "ask_location rejects \"\"",
                       ask_location_rejects_empty_string)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "ask_location rejects any format that dose not start "
                       "with 1 letter followed by 2 digits",
                       ask_location_rejects_invalid_format)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "print_merch has correct output format",
                       print_merch_prints_merch_correctly)
          == NULL)
      || (CU_add_test (
              user_interface_test_suite,
              "should_print_more returns false if input starts with N or n",
              should_print_more_question_returns_false_on_Nn)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "should_print_more returns true if input do not start "
                       "with N or n",
                       should_print_more_question_returns_true_on_other)
          == NULL)
      || (CU_add_test (
              user_interface_test_suite,
              "should_remove_merch returns true if input starts with Y or y",
              should_remove_merch_question_returns_true_on_Yy)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "should_remove_merch returns false if input do not "
                       "start with Y or y",
                       should_remove_merch_question_returns_false_on_other)
          == NULL)
      || (CU_add_test (
              user_interface_test_suite,
              "should_edit_merch returns true if input starts with Y or y",
              should_edit_merch_question_returns_true_on_Yy)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "should_edit_merch returns false if input do not start "
                       "with Y or y",
                       should_edit_merch_question_returns_false_on_other)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "should_remove_cart returns false if input do not "
                       "start with Y or y",
                       should_remove_cart_question_returns_true_on_Yy)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "should_remove_cart returns false if input do not "
                       "start with Y or y",
                       should_remove_cart_question_returns_false_on_other)
          == NULL)
      || (CU_add_test (
              user_interface_test_suite,
              "get_menu_choice returns uppercase char and prints menu",
              get_menu_choice_returns_uppercase_char)
          == NULL)
      || (CU_add_test (user_interface_test_suite,
                       "print_storage_location gives expected output",
                       test_print_storage_location)
          == NULL)
      ||

      0)
    {
      // If adding any of the tests fails, we tear down CUnit and exit
      CU_cleanup_registry ();
      return CU_get_error ();
    }

  // Set the running mode. Use CU_BRM_VERBOSE for maximum output.
  // Use CU_BRM_NORMAL to only print errors and a summary
  CU_basic_set_mode (CU_BRM_VERBOSE);

  // This is where the tests are actually run!
  CU_basic_run_tests ();

  int exit_code = CU_get_number_of_tests_failed () == 0
                      ? CU_get_error ()
                      : CU_get_number_of_tests_failed ();

  // Tear down CUnit before exiting
  CU_cleanup_registry ();

  return exit_code;
}

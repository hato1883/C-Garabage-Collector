#include <CUnit/Basic.h>
#include <stdlib.h>

#include "../../src/gc.h"
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

static void
add_merch (char *name, char *desc, char *price)
{
  io_mock_add_input ("a\n");
  io_mock_add_input (name);
  io_mock_add_input (desc);
  io_mock_add_input (price);
}

static void
list_merch (void)
{
  io_mock_add_input ("l\n");
}

static void
delete_merch (char *name, char *confirm)
{
  io_mock_add_input ("d\n");
  io_mock_add_input (name);
  io_mock_add_input (confirm);
}

static void
edit_merch (char *old_name, char *new_name, char *new_desc, char *new_price,
            char *confirm)
{
  io_mock_add_input ("e\n");
  io_mock_add_input (old_name);
  io_mock_add_input (new_name);
  io_mock_add_input (new_desc);
  io_mock_add_input (new_price);
  io_mock_add_input (confirm);
}

static void
show_stock_of_merch (char *name)
{
  io_mock_add_input ("s\n");
  io_mock_add_input (name);
}

static void
refill_stock_of_merch (char *name, char *location, char *amount)
{
  io_mock_add_input ("p\n");
  io_mock_add_input (name);
  io_mock_add_input (location);
  io_mock_add_input (amount);
}

static void
create_cart ()
{
  io_mock_add_input ("c\n");
}

static void
destroy_cart (char *cart_id, char *confirm)
{
  io_mock_add_input ("r\n");
  io_mock_add_input (cart_id);
  io_mock_add_input (confirm);
}

static void
add_to_cart (char *cart_id, char *merch_name, char *quantity)
{
  io_mock_add_input ("+\n");
  io_mock_add_input (cart_id);
  io_mock_add_input (merch_name);
  io_mock_add_input (quantity);
}

static void
remove_from_cart (char *cart_id, char *merch_name, char *quantity)
{
  io_mock_add_input ("-\n");
  io_mock_add_input (cart_id);
  io_mock_add_input (merch_name);
  io_mock_add_input (quantity);
}

static void
calculate_cost (char *cart_id)
{
  io_mock_add_input ("=\n");
  io_mock_add_input (cart_id);
}

static void
checkout_cart (char *cart_id)
{
  io_mock_add_input ("O\n");
  io_mock_add_input (cart_id);
}

static void
undo ()
{
  io_mock_add_input ("u\n");
}

static void
quit_program ()
{
  io_mock_add_input ("q\n");
}

static char *menu_question = "Menu Choice: ";
static char *menu_string = "\n============ Webstore Menu ============\n"
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

static void
add_expected_output (ioopm_list_t *expected_terminal_output,
                     char *expected_string)
{
  ioopm_linked_list_append (expected_terminal_output,
                            string_to_elem (expected_string));
}

static void
add_menu_output (ioopm_list_t *expected_terminal_output)
{
  add_expected_output (expected_terminal_output, menu_string);
  add_expected_output (expected_terminal_output, menu_question);
}

void
run_program_test (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();

  // Define program execution inputs
  add_merch ("Milk\n", "From cows\n", "10\n");   // Add Milk
  add_merch ("Bread\n", "From wheat\n", "20\n"); // Add Bread
  list_merch ();                                 // List all items

  delete_merch ("Bread\n", "y\n"); // Remove Bread
  list_merch ();                   // List all items

  add_merch ("Whole Wheat\n", "Unhealthy Wheat\n", "2\n"); // Add Whole Wheat
  list_merch ();                                           // List all items

  edit_merch ("Whole Wheat\n", "Whole Wheat\n", // Edit Whole Wheat
              "Healthy Wheat\n", "2\n", "y\n"); // to be healthy
  list_merch ();                                // List all items

  refill_stock_of_merch ("Milk\n", "A01\n",
                         "100\n"); // Add 100 items of Milk to A01
  show_stock_of_merch ("Milk\n");  // Show all stock locations for Milk

  refill_stock_of_merch ("Whole Wheat\n", "C10\n",
                         "10\n"); // Add 10 items of Whole Wheat to C10
  show_stock_of_merch (
      "Whole Wheat\n"); // Show all stock locations for Whole Wheat

  refill_stock_of_merch ("Whole Wheat\n", "B05\n",
                         "30\n"); // Add 30 items of Whole Wheat to B05
  show_stock_of_merch (
      "Whole Wheat\n"); // Show all stock locations for Whole Wheat

  create_cart (); // Create a new shopping cart (1)

  add_to_cart ("1\n", "Milk\n", "30\n"); // Add 30 Milk to shopping cart 1
  calculate_cost ("1\n");                // Get total cost of shopping cart 1

  add_to_cart ("1\n", "Whole Wheat\n",
               "40\n");   // Add 40 Whole Wheat to shopping cart 1
  calculate_cost ("1\n"); // Get total cost of shopping cart 1

  checkout_cart ("1\n"); // Checkout cart 1

  show_stock_of_merch ("Milk\n");        // Show remaining stock of Milk
  show_stock_of_merch ("Whole Wheat\n"); // Show remaining stock of Whole Wheat

  create_cart (); // Create a new shopping cart (2)

  add_to_cart ("2\n", "Milk\n", "70\n"); // Add 70 Milk to shopping cart 2
  calculate_cost ("2\n");                // Get total cost of shopping cart 2

  remove_from_cart ("2\n", "Milk\n",
                    "30\n"); // Remove 30 Milk from shopping cart 2
  calculate_cost ("2\n");    // Get total cost of shopping cart 2

  destroy_cart ("2\n", "y\n"); // Delete cart

  show_stock_of_merch ("Milk\n"); // Show remaining stock of Milk

  undo (); // Not implemented

  quit_program (); // Quit program

  // Run loop given inputs above
  ioopm_webstore_main_loop (store);
  io_mock_disable ();

  // Check if outputs from program matches expected
  ioopm_list_t *expected_output;
  ioopm_linked_list_create (&expected_output, NULL);

  // Add bread
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Merchandise description: ");
  add_expected_output (expected_output, "Price of merchandise: ");

  // Add Milk
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Merchandise description: ");
  add_expected_output (expected_output, "Price of merchandise: ");

  // List merch
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Bread, From wheat : 20 kr\n");
  add_expected_output (expected_output, "Milk, From cows : 10 kr\n");

  // Remove bread
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output,
                       "Are you sure you want to remove this item?: ");

  // List merch
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Milk, From cows : 10 kr\n");

  // Add Whole Wheat
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Merchandise description: ");
  add_expected_output (expected_output, "Price of merchandise: ");

  // List merch
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Milk, From cows : 10 kr\n");
  add_expected_output (expected_output,
                       "Whole Wheat, Unhealthy Wheat : 2 kr\n");

  // Edit Whole Wheat
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Merchandise description: ");
  add_expected_output (expected_output, "Price of merchandise: ");
  add_expected_output (expected_output,
                       "Are you sure you want to edit this item?: ");

  // List merch
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Milk, From cows : 10 kr\n");
  add_expected_output (expected_output, "Whole Wheat, Healthy Wheat : 2 kr\n");

  // Add Milk stock
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Location name: ");
  add_expected_output (expected_output, "Quantity: ");

  // Show Milk stock
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "A01: 100\n");

  // Add Whole Wheat stock
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Location name: ");
  add_expected_output (expected_output, "Quantity: ");

  // Show Whole Wheat stock
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "C10: 10\n");

  // Add Whole Wheat stock
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Location name: ");
  add_expected_output (expected_output, "Quantity: ");

  // Show Whole Wheat stock
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "B05: 30\n");
  add_expected_output (expected_output, "C10: 10\n");

  // Create shopping cart
  add_menu_output (expected_output);

  // Add Milk to cart 1
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Quantity: ");

  // Calculate cost cart 1
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (
      expected_output,
      "Shopping cart 1 has items with a total cost of: 300 kr\n");

  // Add Whole Wheat to cart 1
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Quantity: ");

  // Calculate cost cart 1
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (
      expected_output,
      "Shopping cart 1 has items with a total cost of: 380 kr\n");

  // checkout cart 1
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (expected_output, "Checking out shopping cart 1\n");
  add_expected_output (expected_output, "Milk: (30 * 10 kr) 300 kr\n");
  add_expected_output (expected_output, "Whole Wheat: (40 * 2 kr) 80 kr\n");
  add_expected_output (expected_output, "Total 380 kr\n");

  // Show Milk stock
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "A01: 70\n");

  // Show Whole Wheat stock
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");

  // Create shopping cart
  add_menu_output (expected_output);

  // Add Milk to cart 2
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Quantity: ");

  // Calculate cost cart 2
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (
      expected_output,
      "Shopping cart 2 has items with a total cost of: 700 kr\n");

  // Remove some Milk from cart 2
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "Quantity: ");

  // Calculate cost cart 2
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (
      expected_output,
      "Shopping cart 2 has items with a total cost of: 300 kr\n");

  // Remove cart 2
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Cart number: ");
  add_expected_output (
      expected_output,
      "Are you sure you want to remove this shopping cart?: ");

  // Show Milk stock
  add_menu_output (expected_output);
  add_expected_output (expected_output, "Name of merchandise: ");
  add_expected_output (expected_output, "A01: 70\n");

  // Undo (Not implemented)
  add_menu_output (expected_output);

  // Quit
  add_menu_output (expected_output);

  ioopm_iterator_t *output_iter = NULL;
  ioopm_list_iterator (&output_iter, expected_output);

  size_t input_id = 0;
  bool has_next = false;
  // for (size_t i = 0; i < 124; i++)
  // {
  /* code */
  //}
  while (ioopm_iterator_has_next (&has_next, output_iter) == SUCCESS
         && has_next)
    {
      elem_t output_srt_elem;
      ioopm_iterator_next (&output_srt_elem, output_iter);
      char *expected_str = elem_to_string (output_srt_elem);

      io_mock_enable_printing (); // Capture string and print it to terminal /
                                  // stdout
      char *result = io_mock_pop_output ();
      CU_ASSERT_STRING_EQUAL (expected_str, result);

      free (result);
      input_id++;
    }
  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_iterator_destroy (&output_iter);
  ioopm_linked_list_destroy (&expected_output);
  ioopm_destroy_webstore (&store);

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
  CU_pSuite webstore_run_test_suite
      = CU_add_suite ("Program Sample Run", init_suite, clean_suite);
  if (webstore_run_test_suite == NULL)
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
  if ((CU_add_test (webstore_run_test_suite,
                    "Check if test program receives expected outputs",
                    run_program_test)
       == NULL)
      || 0)
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

#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/gc.h"
#include "../src/webstore.h"
#include "oom.h"
#include "ui_mocking.h"

int
init_suite (void)
{
  // needs more than 8 KB to store everything
  h_init (1024 * 256, true, 0.75f);
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

void
create_and_destroy_store (void)
{
  webstore_t *store = NULL;
  store = ioopm_create_webstore ();

  CU_ASSERT_PTR_NOT_NULL (store);

  ioopm_destroy_webstore (&store);

  CU_ASSERT_PTR_NULL (store);
}

void
add_merch_accepts_new (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();

  io_mock_add_input ("Name\n"); // Merch Name attempt #1
  io_mock_add_input ("Desc\n"); // Merch Desc attempt #1
  io_mock_add_input ("0\n");    // Merch Price attempt #1

  ioopm_add_merch (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
add_merch_rejects_duplicate_name (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  size_t num_duplicate_attempts = 0;

  io_mock_add_input ("duplicate\n");   // 1st Merch Name attempt #1
  io_mock_add_input ("unique desc\n"); // 1st Merch Desc attempt #1
  io_mock_add_input ("123\n");         // 1st Merch Price attempt #1

  ioopm_add_merch (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  io_mock_add_input ("duplicate\n"); // 2nd Merch Name attempt #1
  io_mock_add_input ("new desc\n");  // 2nd Merch Desc attempt #1
  io_mock_add_input ("987\n");       // 2nd Merch Price attempt #1
  num_duplicate_attempts++;

  io_mock_add_input ("duplicate\n"); // 2nd Merch Name attempt #2
  io_mock_add_input (
      "Wow they really rejected me\n"); // 2nd Merch Desc attempt #2
  io_mock_add_input ("1337\n");         // 2nd Merch Price attempt #2
  num_duplicate_attempts++;

  io_mock_add_input ("new merch\n");   // 2nd Merch Name attempt #3
  io_mock_add_input ("unique desc\n"); // 2nd Merch Desc attempt #3
  io_mock_add_input ("123\n");         // 2nd Merch Price attempt #3

  ioopm_add_merch (store);

  // Check that every duplicate entry gets rejected
  for (size_t i = 0; i < num_duplicate_attempts; i++)
    {
      result = io_mock_pop_output (); // Name Question
      CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
      free (result);

      result = io_mock_pop_output (); // Desc Question
      CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
      free (result);

      result = io_mock_pop_output (); // Price Question
      CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
      free (result);
    }

  // check that last is accepted
  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
list_merch_prints_nothing_if_empty (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();

  ioopm_list_merch (store);

  CU_ASSERT_EQUAL_FATAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
list_merch_prints_less_than_20_merch (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  const size_t number_of_merch = 40;
  const size_t prints_before_confirmation = 20;
  const size_t loops_of_20_expected = 1;
  char desc[] = "A ";
  char price[] = "0 ";

  char *merch_names[number_of_merch];
  for (size_t i = 0; i < number_of_merch; i++)
    {
      merch_names[i] = malloc (sizeof (char) * 4);
      merch_names[i][0] = ((char)'0' + i / 10);
      merch_names[i][1] = ((char)'0' + i % 10);
      merch_names[i][2] = '\n';
      merch_names[i][3] = '\0';
      desc[1] = '\n';
      price[1] = '\n';

      io_mock_add_input (merch_names[i]);
      io_mock_add_input ("A\n");
      io_mock_add_input ("0\n");

      ioopm_add_merch (store);
    }

  io_mock_reset_output ();
  CU_ASSERT_EQUAL_FATAL (io_mock_output_length (), 0);

  // tell list_merch to stop after 20
  io_mock_add_input ("n\n");
  io_mock_add_input ("n\n");

  ioopm_list_merch (store);

  CU_ASSERT_EQUAL_FATAL (io_mock_output_length (),
                         (prints_before_confirmation + 1)
                             * loops_of_20_expected);

  for (size_t i = 0; i < loops_of_20_expected; i++)
    {
      char *result;
      for (size_t i = 0; i < prints_before_confirmation; i++)
        {
          merch_names[i][2] = '\0';
          desc[1] = '\0';
          char buf[255];
          sprintf (buf, "%s, %s : %d kr\n", merch_names[i], desc,
                   atoi (price));

          // Check if printed output matches what we expect
          result = io_mock_pop_output ();
          CU_ASSERT_STRING_EQUAL (buf, result);
          free (result);
        }
      // check if user receives a question prompt after 20 items
      result = io_mock_pop_output ();
      CU_ASSERT_STRING_EQUAL ("Continue printing?: ", result);
      free (result);
    }

  for (size_t i = 0; i < number_of_merch; i++)
    {
      free (merch_names[i]);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
list_merch_prints_more_than_20_merch (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  const size_t number_of_merch = 41;
  const size_t prints_before_confirmation = 20;
  const size_t loops_of_20_expected = 2;
  char desc[] = "A ";
  char price[] = "0 ";

  char *merch_names[number_of_merch];
  for (size_t i = 0; i < number_of_merch; i++)
    {
      merch_names[i] = malloc (sizeof (char) * 4);
      merch_names[i][0] = ((char)'0' + i / 10);
      merch_names[i][1] = ((char)'0' + i % 10);
      merch_names[i][2] = '\n';
      merch_names[i][3] = '\0';
      desc[1] = '\n';
      price[1] = '\n';

      io_mock_add_input (merch_names[i]);
      io_mock_add_input ("A\n");
      io_mock_add_input ("0\n");

      ioopm_add_merch (store);
    }

  io_mock_reset_output ();
  CU_ASSERT_EQUAL_FATAL (io_mock_output_length (), 0);

  // tell list_merch to stop after 20
  io_mock_add_input ("y\n");
  io_mock_add_input ("n\n");

  ioopm_list_merch (store);

  CU_ASSERT_EQUAL_FATAL (io_mock_output_length (),
                         (prints_before_confirmation + 1)
                             * loops_of_20_expected);

  for (size_t outer = 0; outer < loops_of_20_expected; outer++)
    {
      char *result;
      for (size_t inner = 0; inner < prints_before_confirmation; inner++)
        {
          merch_names[inner + (outer * prints_before_confirmation)][2] = '\0';
          desc[1] = '\0';
          char buf[255];
          sprintf (buf, "%s, %s : %d kr\n",
                   merch_names[inner + (outer * prints_before_confirmation)],
                   desc, atoi (price));

          // Check if printed output matches what we expect
          result = io_mock_pop_output ();
          CU_ASSERT_STRING_EQUAL (buf, result);
          free (result);
        }
      // check if user receives a question prompt after 20 items
      result = io_mock_pop_output ();
      CU_ASSERT_STRING_EQUAL ("Continue printing?: ", result);
      free (result);
    }

  for (size_t i = 0; i < number_of_merch; i++)
    {
      free (merch_names[i]);
    }

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
remove_merch_removes_existing (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name attempt #1
  io_mock_add_input ("Desc\n");  // Merch Desc attempt #1
  io_mock_add_input ("0\n");     // Merch Price attempt #1
  ioopm_add_merch (store);
  io_mock_reset_output ();

  io_mock_add_input ("exist\n"); // pick merch that exist
  io_mock_add_input ("y\n");     // confirm removal

  ioopm_remove_merch (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to remove this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
remove_merch_rejects_non_existing (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name attempt #1
  io_mock_add_input ("Desc\n");  // Merch Desc attempt #1
  io_mock_add_input ("0\n");     // Merch Price attempt #1
  ioopm_add_merch (store);
  io_mock_reset_output ();

  io_mock_add_input ("dose_not_exist\n"); // pick merch that dose not exist
  io_mock_add_input ("exist\n");          // pick merch that exist
  io_mock_add_input ("y\n");              // confirm removal

  ioopm_remove_merch (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to remove this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
remove_merch_stops_removal_if_not_confirmed (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name attempt #1
  io_mock_add_input ("Desc\n");  // Merch Desc attempt #1
  io_mock_add_input ("0\n");     // Merch Price attempt #1
  ioopm_add_merch (store);
  io_mock_reset_output ();

  io_mock_add_input ("exist\n"); // pick merch that exist
  io_mock_add_input ("n\n");     // deny removal

  ioopm_remove_merch (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to remove this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  io_mock_add_input ("exist\n"); // pick merch that exist
  io_mock_add_input ("y\n");     // confirm removal

  ioopm_remove_merch (store);

  // Check question
  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to remove this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
edit_merch_dose_not_change_stock (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing (); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("A\n");   // Merch Name
  io_mock_add_input ("123\n"); // Merch Desc
  io_mock_add_input ("1\n");   // Merch Price
  ioopm_add_merch (store);

  // Add Stock
  io_mock_add_input ("A\n");   // Merch Name
  io_mock_add_input ("A01\n"); // Merch Desc
  io_mock_add_input ("100\n"); // Merch Price
  ioopm_refill_stock (store);

  // show stock
  io_mock_add_input ("A\n"); // Merch Name
  ioopm_show_stock (store);

  io_mock_reset_output ();

  io_mock_add_input ("A\n"); // pick merch that exist

  io_mock_add_input ("B\n");   // New Merch Name
  io_mock_add_input ("123\n"); // New Merch Desc
  io_mock_add_input ("1\n");   // New Merch Price
  io_mock_add_input ("y\n");   // confirm edit

  ioopm_edit_merch (store);

  // Check question
  char *result
      = io_mock_pop_output (); // Name Question to know what item to edit
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // New Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Confirmation before edit
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to edit this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
edit_merch_removes_existing (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name
  io_mock_add_input ("Desc\n");  // Merch Desc
  io_mock_add_input ("0\n");     // Merch Price
  ioopm_add_merch (store);
  io_mock_reset_output ();

  io_mock_add_input ("exist\n"); // pick merch that exist

  io_mock_add_input ("New\n"); // New Merch Name
  io_mock_add_input ("wow\n"); // New Merch Desc
  io_mock_add_input ("1\n");   // New Merch Price

  io_mock_add_input ("y\n"); // confirm edit

  ioopm_edit_merch (store);

  // Check question
  char *result
      = io_mock_pop_output (); // Name Question to know what item to edit
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // New Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Confirmation before edit
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to edit this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
edit_merch_rejects_non_existing (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name attempt #1
  io_mock_add_input ("Desc\n");  // Merch Desc attempt #1
  io_mock_add_input ("0\n");     // Merch Price attempt #1
  ioopm_add_merch (store);
  io_mock_reset_output ();

  io_mock_add_input ("dose_not_exist\n"); // pick merch that dose not exist
  io_mock_add_input ("exist\n");          // pick merch that exist

  io_mock_add_input ("New\n"); // New Merch Name
  io_mock_add_input ("wow\n"); // New Merch Desc
  io_mock_add_input ("1\n");   // New Merch Price

  io_mock_add_input ("y\n"); // confirm edit

  ioopm_edit_merch (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // New Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to edit this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
edit_merch_stops_removal_if_not_confirmed (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name
  io_mock_add_input ("Desc\n");  // Merch Desc
  io_mock_add_input ("0\n");     // Merch Price
  ioopm_add_merch (store);
  io_mock_reset_output ();

  io_mock_add_input ("exist\n"); // pick merch that exist

  io_mock_add_input ("New\n"); // New Merch Name
  io_mock_add_input ("wow\n"); // New Merch Desc
  io_mock_add_input ("1\n");   // New Merch Price

  io_mock_add_input ("n\n"); // deny edit

  ioopm_edit_merch (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // New Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to edit this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  io_mock_add_input ("exist\n"); // pick merch that exist

  io_mock_add_input ("New\n"); // New Merch Name
  io_mock_add_input ("wow\n"); // New Merch Desc
  io_mock_add_input ("1\n");   // New Merch Price

  io_mock_add_input ("y\n"); // confirm removal

  ioopm_edit_merch (store);

  // Check question
  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // New Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to edit this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
edit_merch_asks_again_if_new_name_exists (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name
  io_mock_add_input ("Desc\n");  // Merch Desc
  io_mock_add_input ("0\n");     // Merch Price
  ioopm_add_merch (store);

  io_mock_add_input ("exist2\n"); // Merch Name
  io_mock_add_input ("Desc2\n");  // Merch Desc
  io_mock_add_input ("1\n");      // Merch Price
  ioopm_add_merch (store);
  io_mock_reset_output ();

  io_mock_add_input ("exist\n"); // pick merch that exist

  io_mock_add_input ("exist2\n"); // New Merch Name that exists
  io_mock_add_input ("wow\n");    // New Merch Desc
  io_mock_add_input ("1\n");      // New Merch Price

  io_mock_add_input ("exist3\n"); // New Merch Name that dose not exists
  io_mock_add_input ("wow\n");    // New Merch Desc
  io_mock_add_input ("1\n");      // New Merch Price

  io_mock_add_input ("y\n"); // deny edit

  ioopm_edit_merch (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // New Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // New Desc Question
  CU_ASSERT_STRING_EQUAL ("Merchandise description: ", result);
  free (result);

  result = io_mock_pop_output (); // New Price Question
  CU_ASSERT_STRING_EQUAL ("Price of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // Desc Question
  CU_ASSERT_STRING_EQUAL ("Are you sure you want to edit this item?: ",
                          result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
show_stock_works_on_non_empty (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  // Add new merch
  io_mock_add_input ("Merch 1\n"); // Merch Name
  io_mock_add_input ("Desc\n");    // Merch Desc
  io_mock_add_input ("0\n");       // Merch Price
  ioopm_add_merch (store);

  // Add a last storage location
  io_mock_add_input ("Merch 1\n"); // Add stock to Merch 1
  io_mock_add_input ("Q95\n");     // New Location Name
  io_mock_add_input ("3\n");       // Location Quantity
  ioopm_refill_stock (store);

  // Add 1 storage location
  io_mock_add_input ("Merch 1\n"); // Add stock to Merch 1
  io_mock_add_input ("A01\n");     // New Location Name
  io_mock_add_input ("1\n");       // Location Quantity
  ioopm_refill_stock (store);

  // Add another storage location
  io_mock_add_input ("Merch 1\n"); // Add stock to Merch 1
  io_mock_add_input ("A02\n");     // New Location Name
  io_mock_add_input ("2\n");       // Location Quantity
  ioopm_refill_stock (store);

  io_mock_reset_output ();

  io_mock_add_input ("Merch 1\n"); // Shock stock of Merch 1
  ioopm_show_stock (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("A01: 1\n", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("A02: 2\n", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Q95: 3\n", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
refill_stock_refills_existing (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name
  io_mock_add_input ("Desc\n");  // Merch Desc
  io_mock_add_input ("0\n");     // Merch Price
  ioopm_add_merch (store);
  io_mock_reset_output ();

  io_mock_add_input ("exist\n"); // pick merch that exist
  io_mock_add_input ("A00\n");   // New Location Name
  io_mock_add_input ("1\n");     // Location Quantity

  ioopm_refill_stock (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
refill_stock_rejects_non_existing_merch (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name
  io_mock_add_input ("Desc\n");  // Merch Desc
  io_mock_add_input ("0\n");     // Merch Price
  ioopm_add_merch (store);
  io_mock_reset_output ();

  io_mock_add_input ("Dose Not Exist\n"); // pick merch that dose not exist
  io_mock_add_input ("exist\n");          // pick merch that exist
  io_mock_add_input ("A00\n");            // New Location Name
  io_mock_add_input ("1\n");              // Location Quantity

  ioopm_refill_stock (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
refill_stock_rejects_zero_or_bellow_quantity (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name
  io_mock_add_input ("Desc\n");  // Merch Desc
  io_mock_add_input ("0\n");     // Merch Price
  ioopm_add_merch (store);
  io_mock_reset_output ();

  size_t invalid_quantity_inputs = 0;

  io_mock_add_input ("exist\n"); // pick merch that exist
  io_mock_add_input ("A00\n");   // New Location Name

  io_mock_add_input ("0\n"); // Invalid Quantity of 0
  invalid_quantity_inputs++;
  io_mock_add_input ("-1\n"); // Invalid Quantity of -1
  invalid_quantity_inputs++;

  io_mock_add_input ("1\n"); // Valid Quantity

  ioopm_refill_stock (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);

  for (size_t i = 0; i < invalid_quantity_inputs; i++)
    {
      result
          = io_mock_pop_output (); // repeat Question due to merch not existing
      CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
      free (result);

      result
          = io_mock_pop_output (); // repeat Question due to merch not existing
      CU_ASSERT_STRING_EQUAL ("The Quantity must be greater than 0!", result);
      free (result);
    }

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
refill_stock_accepts_existing_location_same_merch (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  io_mock_add_input ("exist\n"); // Merch Name
  io_mock_add_input ("Desc\n");  // Merch Desc
  io_mock_add_input ("0\n");     // Merch Price
  ioopm_add_merch (store);

  io_mock_add_input ("exist\n"); // pick merch that exist
  io_mock_add_input ("A00\n");   // New Location Name
  io_mock_add_input ("10\n");    // Valid Quantity

  ioopm_refill_stock (store);
  io_mock_reset_output ();

  io_mock_add_input ("exist\n"); // pick merch that exist
  io_mock_add_input ("A00\n");   // existing Location Name
  io_mock_add_input ("100\n");   // Valid Quantity

  ioopm_refill_stock (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
refill_stock_accepts_existing_location_different_merch (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  // create data storage
  webstore_t *store = ioopm_create_webstore ();
  // Create Merch 1
  io_mock_add_input ("Merch 1\n"); // Merch 1 Name
  io_mock_add_input ("a\n");       // Merch 1 Desc
  io_mock_add_input ("0\n");       // Merch 1 Price
  ioopm_add_merch (store);

  // Create Merch 2
  io_mock_add_input ("Merch 2\n"); // Merch 2 Name
  io_mock_add_input ("b\n");       // Merch 2 Desc
  io_mock_add_input ("0\n");       // Merch 2 Price
  ioopm_add_merch (store);

  // Add stock for merch 1
  io_mock_add_input ("Merch 1\n"); // Merch 1 (exists)
  io_mock_add_input ("A00\n");     // New empty Location
  io_mock_add_input ("10\n");      // Valid Quantity

  ioopm_refill_stock (store);
  io_mock_reset_output ();

  // Add stock for Merch 2 in same place as Merch 1
  io_mock_add_input ("Merch 2\n"); // Merch 2 (exists)
  io_mock_add_input ("A00\n");     // existing Location Name that holds Merch 1
  io_mock_add_input ("A01\n");     // New empty Location
  io_mock_add_input ("100\n");     // Valid Quantity

  ioopm_refill_stock (store);

  // Check question
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Location name: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
create_and_destroy_merch (void)
{
  merch_t *merch = NULL;
  merch = ioopm_create_merch ("name", "description", 0);

  CU_ASSERT_PTR_NOT_NULL (merch);

  ioopm_destroy_merch (&merch);

  CU_ASSERT_PTR_NULL (merch);
}

void
get_merch_name_returns_name_on_non_empty (void)
{
  char *expected = "name";
  merch_t *merch = ioopm_create_merch (expected, "desc", 0);

  char *received = ioopm_get_merch_name (merch);
  CU_ASSERT_STRING_EQUAL (received, expected);

  ioopm_destroy_merch (&merch);

  expected = "137";
  merch = ioopm_create_merch (expected, "desc", 0);

  received = ioopm_get_merch_name (merch);
  CU_ASSERT_STRING_EQUAL (received, expected);

  ioopm_destroy_merch (&merch);
}

void
get_merch_desc_returns_description_on_non_empty (void)
{
  char *expected = "description";
  merch_t *merch = ioopm_create_merch ("name", expected, 0);

  char *received = ioopm_get_merch_desc (merch);
  CU_ASSERT_STRING_EQUAL (received, expected);

  ioopm_destroy_merch (&merch);

  expected = "137";
  merch = ioopm_create_merch ("name", expected, 0);

  received = ioopm_get_merch_desc (merch);
  CU_ASSERT_STRING_EQUAL (received, expected);

  ioopm_destroy_merch (&merch);
}

void
get_merch_price_returns_price_on_non_empty (void)
{
  int expected = 0;
  merch_t *merch = ioopm_create_merch ("name", "description", expected);

  int received = ioopm_get_merch_price (merch);
  CU_ASSERT_EQUAL (received, expected);

  ioopm_destroy_merch (&merch);

  expected = 137;
  merch = ioopm_create_merch ("name", "description", expected);

  received = ioopm_get_merch_price (merch);
  CU_ASSERT_EQUAL (received, expected);

  ioopm_destroy_merch (&merch);
}

void
create_and_destroy_cart (void)
{
  webstore_t *store = ioopm_create_webstore ();

  ioopm_create_cart (store);

  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  io_mock_add_input ("1\n");
  io_mock_add_input ("y\n");
  ioopm_destroy_cart (store);

  ioopm_destroy_webstore (&store);

  // cleanup resources
  io_mock_destroy ();
}

void
create_cart_and_hope_no_leaks (void)
{
  webstore_t *store = ioopm_create_webstore ();

  ioopm_create_cart (store);

  ioopm_destroy_webstore (&store);

  // cleanup resources
  io_mock_destroy ();
}

void
add_to_cart_accepts_less_than_total (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  webstore_t *store = ioopm_create_webstore ();

  // Add a item
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("Desc\n");
  io_mock_add_input ("10\n");
  ioopm_add_merch (store);

  // Add stock
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A01\n");
  io_mock_add_input ("10\n");
  ioopm_refill_stock (store);

  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A02\n");
  io_mock_add_input ("20\n");
  ioopm_refill_stock (store);

  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A03\n");
  io_mock_add_input ("30\n");
  ioopm_refill_stock (store);

  ioopm_create_cart (store); // Cart id 1
  ioopm_create_cart (store); // Cart id 2

  io_mock_reset_output ();

  // Take out 10 which exist in single location
  io_mock_add_input ("1\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("10\n");
  ioopm_add_to_cart (store);

  // Check terminal output
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // Take out 50 which is split in multiple locations
  io_mock_add_input ("2\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("50\n");
  ioopm_add_to_cart (store);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // Remove cart 1
  io_mock_add_input ("1\n");
  io_mock_add_input ("y\n");
  ioopm_destroy_cart (store);

  io_mock_reset_output ();

  // add remaining items to cart 2
  io_mock_add_input ("2\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("10\n"); // 10 in stock (if remove works), 10 should pass
  ioopm_add_to_cart (store);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
add_to_cart_rejects_greater_than_total (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  webstore_t *store = ioopm_create_webstore ();

  // Add a item
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("Desc\n");
  io_mock_add_input ("10\n");
  ioopm_add_merch (store);

  // Add stock to location A01
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A01\n");
  io_mock_add_input ("10\n");
  ioopm_refill_stock (store);

  // Add stock to location A02
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A02\n");
  io_mock_add_input ("20\n");
  ioopm_refill_stock (store);

  // Add stock to location A03
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A03\n");
  io_mock_add_input ("30\n");
  ioopm_refill_stock (store);

  // Create multiple carts
  ioopm_create_cart (store); // Cart ID 1
  ioopm_create_cart (store); // Cart ID 2

  io_mock_reset_output ();

  // Try to take 61 and fail, then take 59 instead
  // add to Cart 1
  io_mock_add_input ("1\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("61\n"); // 60 in stock, 61 should get rejected
  io_mock_add_input ("59\n"); // 60 in stock, 59 should pass
  ioopm_add_to_cart (store);

  // Check terminal output
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL (
      "Quantity: ", result); // Reject quantity of 61 when we have 60 left
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL (
      "Quantity: ", result); // Accept quantity of 59 when we have 60 left
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // Try to take 2 and fail, then take 1 instead
  // add to Cart 2
  // Should check that the previous 60 has 59 occupied items in cart 1
  io_mock_add_input ("2\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("2\n"); // 1 in stock, 2 should get rejected
  io_mock_add_input ("1\n"); // 1 in stock, 1 should pass
  ioopm_add_to_cart (store);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result
      = io_mock_pop_output (); // Reject quantity of 2 when we only have 1 left
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  result
      = io_mock_pop_output (); // Accept quantity of 1 when we only have 1 left
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
remove_from_cart_accepts_less_than_cart_inventory (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  webstore_t *store = ioopm_create_webstore ();

  // Add a item
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("Desc\n");
  io_mock_add_input ("10\n");
  ioopm_add_merch (store);

  // Add stock
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A01\n");
  io_mock_add_input ("10\n");
  ioopm_refill_stock (store);

  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A02\n");
  io_mock_add_input ("20\n");
  ioopm_refill_stock (store);

  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A03\n");
  io_mock_add_input ("30\n");
  ioopm_refill_stock (store);

  ioopm_create_cart (store); // Cart id 1
  ioopm_create_cart (store); // Cart id 2

  // Take out 10 which exist in single location
  io_mock_add_input ("1\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("10\n");
  ioopm_add_to_cart (store);

  io_mock_reset_output ();

  io_mock_add_input ("1\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("5\n");
  ioopm_remove_from_cart (store);

  // Check terminal output
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // Take out 50 which is split in multiple locations
  io_mock_add_input ("1\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("5\n");
  ioopm_remove_from_cart (store);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL ("Quantity: ", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
remove_from_cart_rejects_greater_than_cart_inventory (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  webstore_t *store = ioopm_create_webstore ();

  // Add a item
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("Desc\n");
  io_mock_add_input ("10\n");
  ioopm_add_merch (store);

  // Add stock to location A01
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A01\n");
  io_mock_add_input ("10\n");
  ioopm_refill_stock (store);

  // Add stock to location A02
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A02\n");
  io_mock_add_input ("20\n");
  ioopm_refill_stock (store);

  // Add stock to location A03
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("A03\n");
  io_mock_add_input ("30\n");
  ioopm_refill_stock (store);

  // Create multiple carts
  ioopm_create_cart (store); // Cart ID 1
  ioopm_create_cart (store); // Cart ID 2

  io_mock_reset_output ();

  // add 10 items to Cart 1
  io_mock_add_input ("1\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("10\n"); // 60 in stock, 59 should pass
  ioopm_add_to_cart (store);

  io_mock_reset_output ();

  io_mock_add_input ("1\n");
  io_mock_add_input ("Merch 1\n");
  io_mock_add_input ("11\n"); // try to remove 11 items and fail
  io_mock_add_input ("10\n"); // remove 10 items from Cart 1 and pass
  ioopm_remove_from_cart (store);

  // Check terminal output
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Name of merchandise: ", result);
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL (
      "Quantity: ", result); // Reject quantity of 61 when we have 60 left
  free (result);

  result = io_mock_pop_output (); // repeat Question due to merch not existing
  CU_ASSERT_STRING_EQUAL (
      "Quantity: ", result); // Accept quantity of 59 when we have 60 left
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
calculate_cost_prints_0_on_empty (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  webstore_t *store = ioopm_create_webstore ();

  ioopm_create_cart (store); // Cart id 1

  io_mock_reset_output ();

  // Take out 10 which exist in single location
  io_mock_add_input ("1\n");
  ioopm_calculate_cost (store);

  // Check terminal output
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL (
      "Shopping cart 1 has items with a total cost of: 0 kr\n", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
calculate_cost_prints_expected_on_non_empty (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  webstore_t *store = ioopm_create_webstore ();

  // Add a item
  io_mock_add_input ("cheap\n");
  io_mock_add_input ("Desc\n");
  io_mock_add_input ("10\n");
  ioopm_add_merch (store);

  // Add a item with different cost
  io_mock_add_input ("expensive\n");
  io_mock_add_input ("Desc\n");
  io_mock_add_input ("100\n");
  ioopm_add_merch (store);

  // Add stock

  io_mock_add_input ("cheap\n");
  io_mock_add_input ("A01\n");
  io_mock_add_input ("20\n");
  ioopm_refill_stock (store);

  io_mock_add_input ("cheap\n");
  io_mock_add_input ("A02\n");
  io_mock_add_input ("30\n");
  ioopm_refill_stock (store);

  io_mock_add_input ("expensive\n");
  io_mock_add_input ("A03\n");
  io_mock_add_input ("10\n");
  ioopm_refill_stock (store);

  ioopm_create_cart (store); // Cart id 1

  // add 5 of cheap costing 10 each
  io_mock_add_input ("1\n");
  io_mock_add_input ("cheap\n");
  io_mock_add_input ("5\n");
  ioopm_add_to_cart (store);

  // add 5 of expensive costing 100 each
  io_mock_add_input ("1\n");
  io_mock_add_input ("expensive\n");
  io_mock_add_input ("5\n");
  ioopm_add_to_cart (store);

  io_mock_reset_output ();

  io_mock_add_input ("1\n");
  ioopm_calculate_cost (store);

  // Check terminal output
  char *result = io_mock_pop_output ();
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output ();
  CU_ASSERT_STRING_EQUAL (
      "Shopping cart 1 has items with a total cost of: 550 kr\n", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
checkout_cart_prints_0_on_empty (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  webstore_t *store = ioopm_create_webstore ();

  ioopm_create_cart (store); // Cart id 1

  io_mock_reset_output ();

  // Take out 10 which exist in single location
  io_mock_add_input ("1\n");

  ioopm_checkout_cart (store);

  // Check terminal output
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Checking out shopping cart 1\n", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Total 0 kr\n", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
  ioopm_destroy_webstore (&store);

  // cleanup mock resources
  io_mock_destroy ();
}

void
checkout_cart_prints_expected_on_non_empty (void)
{
  io_mock_init ();
  io_mock_disable_printing (); // do not forward text to terminal / stdout
  // io_mock_enable_printing(); // Capture string and print it to terminal /
  // stdout

  webstore_t *store = ioopm_create_webstore ();

  // Add a item
  io_mock_add_input ("cheap\n");
  io_mock_add_input ("Desc\n");
  io_mock_add_input ("10\n");
  ioopm_add_merch (store);

  // Add a item with different cost
  io_mock_add_input ("expensive\n");
  io_mock_add_input ("Desc\n");
  io_mock_add_input ("100\n");
  ioopm_add_merch (store);

  // Add stock

  io_mock_add_input ("cheap\n");
  io_mock_add_input ("A01\n");
  io_mock_add_input ("2\n");
  ioopm_refill_stock (store);

  io_mock_add_input ("cheap\n");
  io_mock_add_input ("A02\n");
  io_mock_add_input ("4\n");
  ioopm_refill_stock (store);

  io_mock_add_input ("expensive\n");
  io_mock_add_input ("A03\n");
  io_mock_add_input ("1\n");
  ioopm_refill_stock (store);

  ioopm_create_cart (store); // Cart id 1

  // add 5 of cheap costing 10 each
  io_mock_add_input ("1\n");
  io_mock_add_input ("cheap\n");
  io_mock_add_input ("5\n");
  ioopm_add_to_cart (store);

  // add 5 of expensive costing 100 each
  io_mock_add_input ("1\n");
  io_mock_add_input ("expensive\n");
  io_mock_add_input ("1\n");
  ioopm_add_to_cart (store);

  io_mock_reset_output ();

  io_mock_add_input ("1\n");

  ioopm_checkout_cart (store);

  // Check terminal output
  char *result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Cart number: ", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Checking out shopping cart 1\n", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("cheap: (5 * 10 kr) 50 kr\n", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("expensive: 100 kr\n", result);
  free (result);

  result = io_mock_pop_output (); // Name Question
  CU_ASSERT_STRING_EQUAL ("Total 150 kr\n", result);
  free (result);

  CU_ASSERT_EQUAL (io_mock_output_length (), 0);

  // cleanup data storage
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
  CU_pSuite webstore_test_suite
      = CU_add_suite ("Webstore Test Suite", init_suite, clean_suite);
  if (webstore_test_suite == NULL)
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
           webstore_test_suite,
           "Webstore creation sets pointer and deletion clears pointer",
           create_and_destroy_store)
       == NULL)
      || (CU_add_test (webstore_test_suite,
                       "add_merch accepts unique merch names",
                       add_merch_accepts_new)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "add_merch rejects duplicate merch names",
                       add_merch_rejects_duplicate_name)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "list_merch prints nothing if store is empty",
                       list_merch_prints_nothing_if_empty)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "list_merch prints less than 20 if no confirmation",
                       list_merch_prints_less_than_20_merch)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "list_merch prints more than 20 if we get confirmation",
                       list_merch_prints_more_than_20_merch)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "remove_merch removes a existing merch",
                       remove_merch_removes_existing)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "remove_merch asks again if merch dose not exist",
                       remove_merch_rejects_non_existing)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "remove_merch dose not delete if not confirmed",
                       remove_merch_stops_removal_if_not_confirmed)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "edit_merch edits a existing merch",
                       edit_merch_removes_existing)
          == NULL)
      || (CU_add_test (webstore_test_suite, "edit_merch dose not remove stock",
                       edit_merch_dose_not_change_stock)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "edit_merch asks again if merch dose not exist",
                       edit_merch_rejects_non_existing)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "edit_merch dose not edit if not confirmed",
                       edit_merch_stops_removal_if_not_confirmed)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "edit_merch asks again if new merch name is taken",
                       edit_merch_asks_again_if_new_name_exists)
          == NULL)
      || (CU_add_test (
              webstore_test_suite,
              "show_stock prints all locations an quantities on non empty",
              show_stock_works_on_non_empty)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "refill_stock accepts existing merch, new location",
                       refill_stock_refills_existing)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "refill_stock rejects non existing merch",
                       refill_stock_rejects_non_existing_merch)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "refill_stock rejects quantity of 0 or bellow",
                       refill_stock_rejects_zero_or_bellow_quantity)
          == NULL)
      || (CU_add_test (
              webstore_test_suite,
              "refill_stock accepts existing location that holds same merch",
              refill_stock_accepts_existing_location_same_merch)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "refill_stock rejects existing location that holds "
                       "different merch",
                       refill_stock_accepts_existing_location_different_merch)
          == NULL)
      || (CU_add_test (
              webstore_test_suite,
              "Merch creation sets pointer and deletion clears pointer",
              create_and_destroy_merch)
          == NULL)
      || (CU_add_test (
              webstore_test_suite,
              "get_merch_name returns correct name on non empty merch",
              get_merch_name_returns_name_on_non_empty)
          == NULL)
      || (CU_add_test (
              webstore_test_suite,
              "get_merch_desc returns correct description on non empty merch",
              get_merch_desc_returns_description_on_non_empty)
          == NULL)
      || (CU_add_test (
              webstore_test_suite,
              "get_merch_price returns correct price on non empty merch",
              get_merch_price_returns_price_on_non_empty)
          == NULL)
      || (CU_add_test (webstore_test_suite, "creates and destroys cart",
                       create_and_destroy_cart)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "creates a cart and then destroy webstore",
                       create_cart_and_hope_no_leaks)
          == NULL)
      || (CU_add_test (
              webstore_test_suite,
              "add_to_cart accepts adding quantity of merch less than total",
              add_to_cart_accepts_less_than_total)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "add_to_cart rejects adding quantity of merch greater "
                       "than total",
                       add_to_cart_rejects_greater_than_total)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "add_to_cart accepts removing quantity of merch less "
                       "than exist in cart",
                       remove_from_cart_accepts_less_than_cart_inventory)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "add_to_cart rejects removing quantity of merch "
                       "greater than exist in cart",
                       remove_from_cart_rejects_greater_than_cart_inventory)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "calculate_cost can handle empty carts",
                       calculate_cost_prints_0_on_empty)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "calculate_cost can handle non empty carts",
                       calculate_cost_prints_expected_on_non_empty)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "checkout_cart can handle empty carts",
                       checkout_cart_prints_0_on_empty)
          == NULL)
      || (CU_add_test (webstore_test_suite,
                       "checkout_cart can handle non empty carts",
                       checkout_cart_prints_expected_on_non_empty)
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

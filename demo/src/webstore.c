#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/hash_table.h"
#include "../../lib/linked_list.h"
#include "../../lib/utils.h"
#include "../../src/gc.h"
#include "user_interface.h"
#include "webstore.h"

typedef ioopm_hash_table_t *shopping_cart_t;
typedef ioopm_hash_table_t *shopping_cart_table_t;
typedef ioopm_hash_table_t *translation_lookup_table_t;
typedef ioopm_hash_table_t *available_merch_table_t;
typedef ioopm_hash_table_t *storage_location_table_t;
typedef struct location_stock location_stock_t;

typedef struct location_stock location_stock_t;

struct webstore
{
  shopping_cart_id_t next_shopping_cart_id;
  merch_id_t next_merch_id;
  translation_lookup_table_t merch_translation_lookup_table;
  available_merch_table_t available_merch;
  storage_location_table_t storage_locations;
  shopping_cart_table_t shopping_carts;
};

struct merch
{
  merch_name_t name;
  char *desc;
  int price;
  ioopm_list_t *locations;
};

struct location_stock
{
  merch_id_t merch_id;
  stock_quantity_t stock_quantity;
};

static size_t return_size_t (elem_t element);
static size_t string_sum_hash (elem_t e);
static bool string_eq (elem_t a, elem_t b);
static bool size_t_eq (elem_t a, elem_t b);
static int destroy_merch_apply_func (elem_t key, elem_t *value, void *extra);
static int destroy_location_stock_apply_func (elem_t key, elem_t *value,
                                              void *extra
                                              __attribute__ ((unused)));
static int cmpstringp (const void *p1, const void *p2);

static shopping_cart_id_t get_next_unique_cart_id (webstore_t *store);
static merch_id_t get_next_unique_merch_id (webstore_t *store);
static translation_lookup_table_t get_lookup_table (webstore_t *store);
static available_merch_table_t get_merch_table (webstore_t *store);
static storage_location_table_t get_location_table (webstore_t *store);
static shopping_cart_table_t get_cart_table (webstore_t *store);

static bool merch_name_exist_in_store (webstore_t *store,
                                       merch_name_t merch_name);
static bool merch_exist_in_store (webstore_t *store, merch_t *merch);
static bool location_exist_in_store (webstore_t *store,
                                     storage_location_id_t location_id);
static bool shopping_cart_exist (webstore_t *store,
                                 shopping_cart_id_t cart_id);

static shopping_cart_id_t ask_for_existing_cart_id (webstore_t *store);
static merch_name_t ask_for_existing_merch_name (webstore_t *store);
static merch_t *ask_for_new_or_same_merch (webstore_t *store,
                                           merch_name_t previous_name);
static merch_t *ask_for_new_merch (webstore_t *store);
static storage_location_id_t
ask_for_new_or_similar_location (webstore_t *store, merch_id_t id);

static void add_merch_name_translation (webstore_t *store, merch_name_t name,
                                        merch_id_t id);
static void remove_merch_name_translation (webstore_t *store,
                                           merch_name_t name);
static void replace_merch_name_in_lookup (webstore_t *store, merch_name_t old,
                                          merch_name_t new);

static merch_id_t get_merch_id_from_translation (webstore_t *store,
                                                 merch_name_t name);
static merch_t *get_merch_from_available (webstore_t *store, merch_id_t id);

static void add_merch_to_available (webstore_t *store, merch_id_t id,
                                    merch_t *merch);
static void remove_merch_from_available (webstore_t *store, merch_id_t id,
                                         bool delete_locations);
static void update_merch_in_available (webstore_t *store, merch_id_t id,
                                       merch_t *new_merch);

static merch_id_t
get_merch_id_from_location (webstore_t *store,
                            storage_location_id_t storage_id);
static bool add_quantity_to_location (webstore_t *store,
                                      storage_location_id_t storage_id,
                                      merch_id_t merch_id,
                                      stock_quantity_t quantity);
static bool remove_quantity_from_location (webstore_t *store,
                                           storage_location_id_t storage_id,
                                           stock_quantity_t quantity);
static void remove_quantity_from_storage (webstore_t *store,
                                          merch_id_t merch_id,
                                          stock_quantity_t quantity);
static stock_quantity_t get_quantity_from_storage (webstore_t *store,
                                                   merch_id_t merch_id);
static stock_quantity_t get_quantity_in_carts (webstore_t *store,
                                               merch_id_t merch_id);
static stock_quantity_t get_quantity_in_this_cart (webstore_t *store,
                                                   shopping_cart_id_t cart_id,
                                                   merch_id_t merch_id);

// Define macros for creating elem_t from our custom types
#define cart_to_elem(x) ptr_to_elem (x)
#define elem_to_cart(x) ((shopping_cart_t)elem_to_ptr (x))

#define location_stock_ptr_to_elem(x) ptr_to_elem (x)
#define elem_to_location_stock_ptr(x) ((location_stock_t *)elem_to_ptr (x))

#define location_id_to_elem(x) string_to_elem (x)
#define elem_to_location_id(x) ((storage_location_id_t)elem_to_string (x))

#define merch_ptr_to_elem(x) ptr_to_elem (x)
#define elem_to_merch_ptr(x) ((merch_t *)elem_to_ptr (x))

#define merch_name_to_elem(x) string_to_elem (x)
#define elem_to_merch_name(x) ((merch_name_t)elem_to_string (x))

#define merch_id_to_elem(x) size_to_elem (x)
#define elem_to_merch_id(x) ((merch_id_t)elem_to_size (x))

#define quantity_to_elem(x) size_to_elem (x)
#define elem_to_quantity(x) ((stock_quantity_t)elem_to_size (x))

#define cart_id_to_elem(x) size_to_elem (x)
#define elem_to_cart_id(x) ((shopping_cart_id_t)elem_to_size (x))

webstore_t *
ioopm_create_webstore (void)
{
  webstore_t *store = h_alloc_struct (global_heap, "2l4*");

  // Merch data segment
  store->next_merch_id = 1;

  ioopm_hash_table_t *lookup_table = NULL;
  ioopm_create_hash_table (&lookup_table, 1, string_sum_hash, string_eq, NULL);
  store->merch_translation_lookup_table = lookup_table;

  ioopm_hash_table_t *available_merch = NULL;
  ioopm_create_hash_table (&available_merch, 1, return_size_t, size_t_eq,
                           NULL);
  store->available_merch = available_merch;

  // Storage location data segment
  ioopm_hash_table_t *locations = NULL;
  ioopm_create_hash_table (&locations, 1, string_sum_hash, string_eq, NULL);
  store->storage_locations = locations;

  // Shopping carts data segment
  store->next_shopping_cart_id = 1;

  ioopm_hash_table_t *shopping_carts = NULL;
  ioopm_create_hash_table (&shopping_carts, 1, return_size_t, size_t_eq, NULL);
  store->shopping_carts = shopping_carts;

  return store;
}

void
ioopm_destroy_webstore (webstore_t **store)
{
  webstore_t *to_remove = *store;

  // Free memory from lookup table
  translation_lookup_table_t lookup = get_lookup_table (to_remove);
  ioopm_hash_table_destroy (&lookup);

  // Free memory from all merch_t structs
  available_merch_table_t available_merch = get_merch_table (to_remove);
  ioopm_hash_table_apply_to_all (available_merch, destroy_merch_apply_func,
                                 NULL);
  ioopm_hash_table_destroy (&available_merch);

  // Free memory from storage locations
  storage_location_table_t locations = get_location_table (to_remove);
  ioopm_hash_table_apply_to_all (locations, destroy_location_stock_apply_func,
                                 NULL);
  ioopm_list_t *key_list = NULL;
  ioopm_hash_table_keys (
      &key_list, locations); // get list of keys to free after table removal
  ioopm_hash_table_destroy (
      &locations); // uses keys so we can't free them before removal -.-

  ioopm_iterator_t *key_iter = NULL;
  ioopm_list_iterator (&key_iter, key_list); // use iterator to make it easier

  // free keys from that we have in list
  bool has_next = false;
  while (ioopm_iterator_has_next (&has_next, key_iter) == SUCCESS && has_next)
    {
      elem_t return_val = ptr_to_elem (NULL);
      ioopm_iterator_next (&return_val, key_iter);
      // free (elem_to_location_id (return_val));
    }
  ioopm_iterator_destroy (&key_iter);
  ioopm_linked_list_destroy (&key_list);

  // Free memory from all shopping cart structs
  shopping_cart_table_t carts = get_cart_table (to_remove);
  ioopm_list_t *cart_inventories_list = NULL;
  ioopm_hash_table_values (&cart_inventories_list,
                           carts); // get list of values to destroy

  ioopm_iterator_t *cart_inventories_iter = NULL;
  ioopm_list_iterator (
      &cart_inventories_iter,
      cart_inventories_list); // use iterator to make it easier

  // free keys from that we have in list
  has_next = false;
  while (ioopm_iterator_has_next (&has_next, cart_inventories_iter) == SUCCESS
         && has_next)
    {
      elem_t return_val = ptr_to_elem (NULL);
      ioopm_iterator_next (&return_val, cart_inventories_iter);
      shopping_cart_t inventory = elem_to_cart (return_val);
      ioopm_hash_table_destroy (&inventory);
    }
  ioopm_iterator_destroy (&cart_inventories_iter);
  ioopm_linked_list_destroy (&cart_inventories_list);

  ioopm_hash_table_destroy (&carts);

  // free memory used by this structure
  // free (to_remove);

  // make this pointe invalid to create segfaults instead of undefined
  // behaviour
  *store = NULL;

  return;
}

void
ioopm_webstore_main_loop (webstore_t *store)
{
  char menu_choice;
  do
    {
      menu_choice = ioopm_get_menu_choice ();
      switch (menu_choice)
        {
        case 'A':
          ioopm_add_merch (store);
          break;

        case 'L':
          ioopm_list_merch (store);
          break;

        case 'D':
          ioopm_remove_merch (store);
          break;

        case 'E':
          ioopm_edit_merch (store);
          break;

        case 'S':
          ioopm_show_stock (store);
          break;

        case 'P':
          ioopm_refill_stock (store);
          break;

        case 'C':
          ioopm_create_cart (store);
          break;

        case 'R':
          ioopm_destroy_cart (store);
          break;

        case '+':
          ioopm_add_to_cart (store);
          break;

        case '-':
          ioopm_remove_from_cart (store);
          break;

        case '=':
          ioopm_calculate_cost (store);
          break;

        case 'O':
          ioopm_checkout_cart (store);
          break;

        case 'U':
          // TODO: NOT IMPLEMENTED
          break;

        default:
          break;
        }
    }
  while (menu_choice != 'Q');
}

void
ioopm_add_merch (webstore_t *store)
{
  merch_t *new_merch = ask_for_new_merch (store);

  // merch dose not exist
  // get a unique id to identify merch
  merch_id_t new_unique_merch_id = get_next_unique_merch_id (store);

  // Add translation from name to id
  merch_name_t merch_name = ioopm_get_merch_name (new_merch);
  add_merch_name_translation (store, merch_name, new_unique_merch_id);

  // add merch to list of available merch using its id as key
  add_merch_to_available (store, new_unique_merch_id, new_merch);

  // merch is successfully added :)
  return;
}

void
ioopm_list_merch (webstore_t *store)
{
  available_merch_table_t available_merch = get_merch_table (store);

  size_t num_unique_merch = 0;
  ioopm_hash_table_size (&num_unique_merch, available_merch);

  ioopm_list_t *merch_list = NULL;
  ioopm_hash_table_values (&merch_list, available_merch);

  ioopm_iterator_t *merch_iterator = NULL;
  ioopm_list_iterator (&merch_iterator, merch_list);

  // Create a array that we will sort
  merch_name_t merch_names_sorted[num_unique_merch];
  for (size_t index = 0; index < num_unique_merch; index++)
    {
      elem_t current = ptr_to_elem (NULL);
      ioopm_iterator_next (&current, merch_iterator);
      merch_t *merch = elem_to_merch_ptr (current);
      merch_names_sorted[index] = ioopm_get_merch_name (merch);
    }

  ioopm_iterator_destroy (&merch_iterator);
  ioopm_linked_list_destroy (&merch_list);

  // actually sort the array
  qsort (merch_names_sorted, num_unique_merch, sizeof (merch_name_t),
         cmpstringp);

  // Number of prints before asking for a confirmation
  const size_t prints_before_confirmation = 20;
  // Number of printed items so far
  size_t number_of_printed_merch = 0;
  // Number of completed loops
  size_t completed_loops = 0;
  do
    {
      for (size_t i = 0; i < prints_before_confirmation
                         && number_of_printed_merch < num_unique_merch;
           i++)
        {
          merch_name_t merch_name = merch_names_sorted
              [i + (completed_loops * prints_before_confirmation)];

          merch_id_t id = get_merch_id_from_translation (store, merch_name);

          merch_t *merch = get_merch_from_available (store, id);

          ioopm_print_merch (merch);

          number_of_printed_merch++;
        }
      completed_loops++;
    }
  while ((number_of_printed_merch < num_unique_merch)
         && (ioopm_should_print_more ()));

  return;
}

void
ioopm_remove_merch (webstore_t *store)
{
  merch_name_t removal_target = ask_for_existing_merch_name (store);

  if (ioopm_should_remove_merch ())
    {
      merch_id_t id = get_merch_id_from_translation (store, removal_target);
      remove_merch_name_translation (store, removal_target);
      remove_merch_from_available (store, id, true);
    }
  // free (removal_target);

  return;
}

void
ioopm_edit_merch (webstore_t *store)
{
  // Asks user for the name of the merch
  // name must exist in the store
  merch_name_t edit_target = ask_for_existing_merch_name (store);

  // Asks user for new merch details
  // new merch name can not exist in the store already
  merch_t *replacement_merch = ask_for_new_or_same_merch (store, edit_target);

  if (ioopm_should_edit_merch ())
    {
      merch_name_t new_name = ioopm_get_merch_name (replacement_merch);

      // replace old name lookup with new name
      merch_id_t id = get_merch_id_from_translation (store, edit_target);
      merch_t *old_merch = get_merch_from_available (store, id);
      replace_merch_name_in_lookup (store, edit_target, new_name);

      ioopm_linked_list_destroy (&replacement_merch->locations);
      replacement_merch->locations = old_merch->locations;

      // Destroy previous merch linked to the unique id
      update_merch_in_available (store, id, replacement_merch);
    }
  else
    {
      // We must delete the newly created merch because we will not store it
      ioopm_destroy_merch (&replacement_merch);
    }

  // free name used to find id
  // free (edit_target);
  return;
}

void
ioopm_show_stock (webstore_t *store)
{
  merch_name_t merch_name = ask_for_existing_merch_name (store);

  merch_id_t merch_id = get_merch_id_from_translation (store, merch_name);
  // free (merch_name);

  merch_t *merch = get_merch_from_available (store, merch_id);

  size_t num_merch_locations = 0;
  ioopm_linked_list_size (&num_merch_locations, merch->locations);

  ioopm_iterator_t *location_id_iterator = NULL;
  ioopm_list_iterator (&location_id_iterator, merch->locations);

  // Create a array that we will sort
  storage_location_id_t location_ids_sorted[num_merch_locations];
  for (size_t index = 0; index < num_merch_locations; index++)
    {
      elem_t current = ptr_to_elem (NULL);
      ioopm_iterator_next (&current, location_id_iterator);
      storage_location_id_t location_id = elem_to_location_id (current);
      location_ids_sorted[index] = location_id;
    }

  ioopm_iterator_destroy (&location_id_iterator);

  // actually sort the array
  qsort (location_ids_sorted, num_merch_locations,
         sizeof (storage_location_id_t), cmpstringp);

  bool was_found = false;
  for (size_t i = 0; i < num_merch_locations; i++)
    {
      storage_location_id_t location_id = location_ids_sorted[i];
      location_stock_t *loc_stock;
      elem_t loc_stock_elem = ptr_to_elem (NULL);
      ioopm_hash_table_lookup (&was_found, &loc_stock_elem,
                               get_location_table (store),
                               string_to_elem (location_id));
      loc_stock = elem_to_location_stock_ptr (loc_stock_elem);
      stock_quantity_t stock_quantity = loc_stock->stock_quantity;

      ioopm_print_storage_location (location_id, stock_quantity);
    }

  return;
}

void
ioopm_refill_stock (webstore_t *store)
{
  merch_name_t merch_name = ask_for_existing_merch_name (store);
  merch_id_t merch_id = get_merch_id_from_translation (store, merch_name);
  // free (merch_name);

  storage_location_id_t storage_id
      = ask_for_new_or_similar_location (store, merch_id);

  stock_quantity_t quantity = ioopm_ask_quantity ();
  bool new_location_added
      = add_quantity_to_location (store, storage_id, merch_id, quantity);

  if (new_location_added)
    {
      // add location to merch
      merch_t *merch = get_merch_from_available (store, merch_id);
      ioopm_linked_list_append (merch->locations, string_to_elem (storage_id));
    }
  else
    {
      // free (storage_id);
    }

  return;
}

void
ioopm_create_cart (webstore_t *store)
{
  shopping_cart_t new_cart = NULL;
  ioopm_create_hash_table (&new_cart, 1, return_size_t, size_t_eq, NULL);
  shopping_cart_id_t new_cart_id = get_next_unique_cart_id (store);
  ioopm_hash_table_insert (get_cart_table (store),
                           cart_id_to_elem (new_cart_id),
                           cart_to_elem (new_cart));

  return;
}

void
ioopm_destroy_cart (webstore_t *store)
{
  shopping_cart_id_t to_remove = ask_for_existing_cart_id (store);
  if (ioopm_should_remove_cart ())
    {
      shopping_cart_table_t shopping_carts = get_cart_table (store);

      // Retrieve internal shopping_cart_t (hash_table_t *)
      // And destroy it
      elem_t cart_inventory_elem = ptr_to_elem (NULL);
      bool was_found = false;
      ioopm_hash_table_lookup (&was_found, &cart_inventory_elem,
                               shopping_carts, cart_id_to_elem (to_remove));
      shopping_cart_t inventory = elem_to_cart (cart_inventory_elem);
      ioopm_hash_table_destroy (&inventory);

      // Remove cart from system
      ioopm_hash_table_remove (shopping_carts, cart_id_to_elem (to_remove));
    }
  return;
}

void
ioopm_add_to_cart (webstore_t *store)
{
  shopping_cart_id_t cart_id = ask_for_existing_cart_id (store);

  merch_name_t merch_name = ask_for_existing_merch_name (store);
  merch_id_t merch_id = get_merch_id_from_translation (store, merch_name);
  // free (merch_name); // No longer need name when we have unique id

  // TODO: create helper function for bellow
  stock_quantity_t existing_merch_quantity
      = get_quantity_from_storage (store, merch_id);
  stock_quantity_t merch_quantity_reserved
      = get_quantity_in_carts (store, merch_id);
  stock_quantity_t available_merch_quantity
      = existing_merch_quantity - merch_quantity_reserved;
  stock_quantity_t requested_quantity;
  do
    {
      requested_quantity = ioopm_ask_quantity ();
    }
  while (requested_quantity > available_merch_quantity);
  // END OF HELPER FUNCTION

  // TODO: create helper function for adding a quantity of merch to cart
  shopping_cart_table_t shopping_carts = get_cart_table (store);

  elem_t cart_elem = ptr_to_elem (NULL);
  bool was_found = false;
  ioopm_hash_table_lookup (&was_found, &cart_elem, shopping_carts,
                           cart_id_to_elem (cart_id));

  shopping_cart_t cart = elem_to_cart (cart_elem);

  elem_t in_cart_quantity_elem = quantity_to_elem (0);
  ioopm_hash_table_lookup (&was_found, &in_cart_quantity_elem, cart,
                           merch_id_to_elem (merch_id));
  stock_quantity_t in_cart = elem_to_quantity (in_cart_quantity_elem);

  requested_quantity = requested_quantity + in_cart;
  ioopm_hash_table_insert (cart, merch_id_to_elem (merch_id),
                           quantity_to_elem (requested_quantity));
  // END OF HELPER FUNCTION

  return;
}

void
ioopm_remove_from_cart (webstore_t *store)
{
  shopping_cart_id_t cart_id = ask_for_existing_cart_id (store);

  merch_name_t merch_name = ask_for_existing_merch_name (store);
  merch_id_t merch_id = get_merch_id_from_translation (store, merch_name);
  // free (merch_name); // No longer need name when we have unique id

  // TODO: create helper function for bellow
  stock_quantity_t merch_quantity_in_cart
      = get_quantity_in_this_cart (store, cart_id, merch_id);
  stock_quantity_t requested_quantity;
  do
    {
      requested_quantity = ioopm_ask_quantity ();
    }
  while (requested_quantity > merch_quantity_in_cart);
  // END OF HELPER FUNCTION

  // TODO: create helper function for adding a quantity of merch to cart
  shopping_cart_table_t shopping_carts = get_cart_table (store);

  elem_t cart_elem = ptr_to_elem (NULL);
  bool was_found = false;
  ioopm_hash_table_lookup (&was_found, &cart_elem, shopping_carts,
                           cart_id_to_elem (cart_id));

  shopping_cart_t cart = elem_to_cart (cart_elem);

  elem_t in_cart_quantity_elem = quantity_to_elem (0);
  ioopm_hash_table_lookup (&was_found, &in_cart_quantity_elem, cart,
                           merch_id_to_elem (merch_id));
  stock_quantity_t in_cart = elem_to_quantity (in_cart_quantity_elem);

  in_cart = in_cart - requested_quantity;
  if (in_cart <= 0)
    {
      ioopm_hash_table_remove (cart, merch_id_to_elem (merch_id));
    }
  else
    {
      ioopm_hash_table_insert (cart, merch_id_to_elem (merch_id),
                               quantity_to_elem (requested_quantity));
    }
  // END OF HELPER FUNCTION

  return;
}

void
ioopm_calculate_cost (webstore_t *store)
{
  shopping_cart_id_t cart_id = ask_for_existing_cart_id (store);
  shopping_cart_table_t shopping_carts_table = get_cart_table (store);

  bool was_found = false;
  elem_t cart_inventory_elem = ptr_to_elem (NULL);
  ioopm_hash_table_lookup (&was_found, &cart_inventory_elem,
                           shopping_carts_table, cart_id_to_elem (cart_id));
  shopping_cart_t cart_inventory = elem_to_cart (cart_inventory_elem);

  // Iterate thru shopping cart inventory
  // Getting the merch_id_t used to check price
  // And stock_quantity_t to get quantity of the merch
  ioopm_list_t *merch_id_list = NULL;
  ioopm_hash_table_keys (&merch_id_list, cart_inventory);
  ioopm_iterator_t *merch_id_iter = NULL;
  ioopm_list_iterator (&merch_id_iter, merch_id_list);

  ioopm_list_t *merch_quantity_list = NULL;
  ioopm_hash_table_values (&merch_quantity_list, cart_inventory);
  ioopm_iterator_t *merch_quantity_iter = NULL;
  ioopm_list_iterator (&merch_quantity_iter, merch_quantity_list);

  int running_sum = 0;

  bool has_next_id = false;
  bool has_next_quantity = false;
  while ((ioopm_iterator_has_next (&has_next_id, merch_id_iter) == SUCCESS)
         && has_next_id
         && (ioopm_iterator_has_next (&has_next_quantity, merch_quantity_iter)
             == SUCCESS)
         && has_next_quantity)
    {
      // For each iteration we need to convert elem_t to shopping_cart_t
      // Then check if shopping cart has mapped the merch_id to a quantity
      // if it has then add the quantity to our running sum
      elem_t merch_id_elem;
      ioopm_iterator_next (&merch_id_elem, merch_id_iter);
      merch_id_t merch_id = elem_to_merch_id (merch_id_elem);

      elem_t merch_quantity_elem;
      ioopm_iterator_next (&merch_quantity_elem, merch_quantity_iter);
      stock_quantity_t merch_quantity = elem_to_quantity (merch_quantity_elem);

      merch_t *merch = get_merch_from_available (store, merch_id);
      int merch_price = ioopm_get_merch_price (merch);

      running_sum = running_sum + merch_quantity * merch_price;
    }

  ioopm_iterator_destroy (&merch_id_iter);
  ioopm_linked_list_destroy (&merch_id_list);

  ioopm_iterator_destroy (&merch_quantity_iter);
  ioopm_linked_list_destroy (&merch_quantity_list);

  ioopm_print_cart_cost (cart_id, running_sum);

  return;
}

void
ioopm_checkout_cart (webstore_t *store)
{
  shopping_cart_id_t cart_id = ask_for_existing_cart_id (store);
  shopping_cart_table_t shopping_carts_table = get_cart_table (store);

  // get inventory of specific cart
  bool was_found = false;
  elem_t cart_inventory_elem = ptr_to_elem (NULL);
  ioopm_hash_table_lookup (&was_found, &cart_inventory_elem,
                           shopping_carts_table, cart_id_to_elem (cart_id));
  shopping_cart_t cart_inventory = elem_to_cart (cart_inventory_elem);

  // Iterate thru shopping cart inventory
  // Getting stock_quantity_t of the items in the merch
  // Getting the merch_id_t to remove the given quantity from storage
  ioopm_list_t *merch_id_list = NULL;
  ioopm_hash_table_keys (&merch_id_list, cart_inventory);
  ioopm_iterator_t *merch_id_iter = NULL;
  ioopm_list_iterator (&merch_id_iter, merch_id_list);

  ioopm_list_t *merch_quantity_list = NULL;
  ioopm_hash_table_values (&merch_quantity_list, cart_inventory);
  ioopm_iterator_t *merch_quantity_iter = NULL;
  ioopm_list_iterator (&merch_quantity_iter, merch_quantity_list);

  ioopm_print_receipt_header (cart_id);

  int running_sum = 0;

  bool has_next_id = false;
  bool has_next_quantity = false;
  while ((ioopm_iterator_has_next (&has_next_id, merch_id_iter) == SUCCESS)
         && has_next_id
         && (ioopm_iterator_has_next (&has_next_quantity, merch_quantity_iter)
             == SUCCESS)
         && has_next_quantity)
    {
      // For each iteration we need to get the quantity and ther merch
      // Then remove that quantity from the storage
      // Lastly print a receipt
      elem_t merch_id_elem;
      ioopm_iterator_next (&merch_id_elem, merch_id_iter);
      merch_id_t merch_id = elem_to_merch_id (merch_id_elem);

      elem_t merch_quantity_elem;
      ioopm_iterator_next (&merch_quantity_elem, merch_quantity_iter);
      stock_quantity_t merch_quantity = elem_to_quantity (merch_quantity_elem);

      // Remove the bought merch from storage
      remove_quantity_from_storage (store, merch_id, merch_quantity);

      merch_t *merch = get_merch_from_available (store, merch_id);
      int merch_price = ioopm_get_merch_price (merch);

      ioopm_print_receipt_entry (ioopm_get_merch_name (merch), merch_quantity,
                                 merch_price);

      running_sum = running_sum + merch_quantity * merch_price;
    }

  // Remove shopping cart
  ioopm_hash_table_destroy (&cart_inventory);

  // Remove cart from system
  ioopm_hash_table_remove (shopping_carts_table, cart_id_to_elem (cart_id));

  ioopm_iterator_destroy (&merch_id_iter);
  ioopm_linked_list_destroy (&merch_id_list);

  ioopm_iterator_destroy (&merch_quantity_iter);
  ioopm_linked_list_destroy (&merch_quantity_list);

  ioopm_print_receipt_footer (running_sum);

  return;
}

merch_t *
ioopm_create_merch (char *name, char *description, int price)
{
  merch_t *new_merch = h_alloc_struct (global_heap, "**i*");
  new_merch->name = heap_strdup (name);
  new_merch->desc = heap_strdup (description);
  new_merch->price = price;
  ioopm_list_t *merch_locations = NULL;
  // we won't be using list's eq_function
  ioopm_linked_list_create (&merch_locations, string_eq);
  new_merch->locations = merch_locations;
  return new_merch;
}

void
ioopm_destroy_merch (merch_t **merch)
{
  merch_t *to_destroy = *merch;
  ioopm_linked_list_destroy (&(to_destroy->locations));
  // free (to_destroy->name);
  // free (to_destroy->desc);
  // free (to_destroy);
  *merch = NULL;
  return;
}

merch_name_t
ioopm_get_merch_name (merch_t *merch)
{
  return merch->name;
}

char *
ioopm_get_merch_desc (merch_t *merch)
{
  return merch->desc;
}

int
ioopm_get_merch_price (merch_t *merch)
{
  return merch->price;
}

/// @brief Gets size_t from the union elem_t
/// @param element the union instance to extract size_t from
/// @return size_t stored in union instance
static size_t
return_size_t (elem_t element)
{
  return elem_to_size (element);
}

/// @brief Hashes a string into size_t
/// @param e A union instance that contains a string to hash
/// @return Hash of the given string taken from the given union
static size_t
string_sum_hash (elem_t e)
{
  size_t hash = 5381;
  char *string = elem_to_string (e);
  int c = *string;
  while (c)
    {
      hash = ((hash << 5) + hash) + c;
      string++;
      c = *string;
    }
  return hash;
}

/// @brief Compares two string elem_t union instances for equality
/// @param a First string union instance
/// @param b Second string union instance
/// @return true if the strings are equal, else false
static bool
string_eq (elem_t a, elem_t b)
{
  bool res = strcmp (elem_to_string (a), elem_to_string (b)) == 0;
  return res;
}

/// @brief Compares two size_t elem_t union instances for equality
/// @param a First size_t union instance
/// @param b Second size_t union instance
/// @return true if the two numbers are equal, else false
static bool
size_t_eq (elem_t a, elem_t b)
{
  return elem_to_size (a) == elem_to_size (b);
}

/// @brief Gets a unique id for the next cart (from this webstore instance)
/// @param store Webstore to get next unique cart id from
/// @return A unique cart id
static shopping_cart_id_t
get_next_unique_cart_id (webstore_t *store)
{
  shopping_cart_id_t c_id = store->next_shopping_cart_id;
  store->next_shopping_cart_id++;
  return c_id;
}

/// @brief Gets a unique id for the next merch (from this webstore instance)
/// @param store Webstore to get next unique merch id from
/// @return A unique merch id
static merch_id_t
get_next_unique_merch_id (webstore_t *store)
{
  merch_id_t next_id = store->next_merch_id;
  store->next_merch_id++;
  return next_id;
}

/// @brief Returns a hash table that contains translations from merch name to
/// merch_id from the given webstore instance
/// @param store Webstore to get translation lookup table from
/// @return Hash table with translation from merch name to unique merch id
static translation_lookup_table_t
get_lookup_table (webstore_t *store)
{
  return store->merch_translation_lookup_table;
}

/// @brief Returns a hash table of all existing merchandise and their details
/// from the given webstore instance
/// @param store Webstore to get merchandise hash table from
/// @return Hash table of all existing merchandise and their details
static available_merch_table_t
get_merch_table (webstore_t *store)
{
  return store->available_merch;
}

/// @brief Returns a hash table of all storage locations and their contents
/// from the given webstore instance
/// @param store Webstore to get location hash table from
/// @return Hash table of all storage locations and their contents
static storage_location_table_t
get_location_table (webstore_t *store)
{
  return store->storage_locations;
}

/// @brief Returns a hash table of all shopping carts and their contents of the
/// given webstore instance
/// @param store Webstore to get shopping carts hash table from
/// @return Hash table of all shopping carts on their contents
static shopping_cart_table_t
get_cart_table (webstore_t *store)
{
  return store->shopping_carts;
}

/// @brief Checks if the given webstore contains a merch with the name
/// merch_name.
/// @param store the webstore to search in
/// @param merch_name the name to compare with
/// @return true if merch with merch_name exist in the given webstore
static bool
merch_name_exist_in_store (webstore_t *store, merch_name_t merch_name)
{
  bool result = false;
  translation_lookup_table_t lookup_table = get_lookup_table (store);
  ioopm_hash_table_has_key (&result, lookup_table,
                            string_to_elem (merch_name));
  return result;
}

/// @brief Checks if the given webstore contains a merch with the same name as
/// check_merch.
/// @param store the webstore to search in
/// @param check_merch the merch to compare names with.
/// @return true if merch with same name as check_merch exist in the given
/// webstore
static bool
merch_exist_in_store (webstore_t *store, merch_t *check_merch)
{
  bool result
      = merch_name_exist_in_store (store, ioopm_get_merch_name (check_merch));
  return result;
}

/// @brief Checks if the given webstore contains a location with the same
/// identification as location_id.
/// @param store the webstore to search in
/// @param location_id the location to search for
/// @return true if location exist in the given webstore
static bool
location_exist_in_store (webstore_t *store, storage_location_id_t location_id)
{
  bool result = false;
  storage_location_table_t location_table = get_location_table (store);
  ioopm_hash_table_has_key (&result, location_table,
                            string_to_elem (location_id));
  return result;
}

/// @brief Checks if the given webstore contains a shopping cart with the same
/// identification as cart_id.
/// @param store webstore to search in
/// @param cart_id shopping cart id to search for
/// @return true if shopping cart exist in the given webstore
static bool
shopping_cart_exist (webstore_t *store, shopping_cart_id_t cart_id)
{
  bool result = false;
  shopping_cart_table_t cart_table = get_cart_table (store);
  ioopm_hash_table_has_key (&result, cart_table, cart_id_to_elem (cart_id));
  return result;
}

/// @brief
/// @param key
/// @param value
/// @param extra
/// @return
static int
destroy_merch_apply_func (elem_t key __attribute__ ((unused)), elem_t *value,
                          void *extra __attribute__ ((unused)))
{
  merch_t *merch = (merch_t *)value->pointer;
  ioopm_destroy_merch (&merch);
  return SUCCESS;
}

/// @brief
/// @param key
/// @param value
/// @param extra
/// @return
static int
destroy_location_stock_apply_func (elem_t key __attribute__ ((unused)),
                                   elem_t *value,
                                   void *extra __attribute__ ((unused)))
{
  // location_stock_t *loc_stock = elem_to_location_stock_ptr (*value);
  // free (loc_stock);
  return SUCCESS;
}

/// @brief
/// @param p1
/// @param p2
/// @return
static int
cmpstringp (const void *p1, const void *p2)
{
  return strcmp (*(char *const *)p1, *(char *const *)p2);
}

/// @brief
/// @param store
/// @return
static shopping_cart_id_t
ask_for_existing_cart_id (webstore_t *store)
{
  shopping_cart_id_t cart_id;
  do
    {
      cart_id = ioopm_ask_cart_id ();
    }
  while (!shopping_cart_exist (store, cart_id));
  return cart_id;
}

/// @brief
/// @param store
/// @return
static merch_name_t
ask_for_existing_merch_name (webstore_t *store)
{
  merch_name_t new_merch = "";
  do
    {
      if (strcmp (new_merch, "") != 0)
        {
          // report error that merch exists
          // free (new_merch);
        }

      new_merch = ioopm_ask_merch_name ();
    }
  while (!merch_name_exist_in_store (store, new_merch));
  return new_merch;
}

/// @brief
/// @param store
/// @return
static merch_t *
ask_for_new_merch (webstore_t *store)
{
  merch_t *new_merch = NULL;
  do
    {
      if (new_merch != NULL)
        {
          // report error that merch exists
          ioopm_destroy_merch (&new_merch);
        }

      new_merch = ioopm_ask_merch ();
    }
  while (merch_exist_in_store (store, new_merch));
  return new_merch;
}

/// @brief
/// @param store
/// @param id
/// @return
static storage_location_id_t
ask_for_new_or_similar_location (webstore_t *store, merch_id_t id)
{
  storage_location_id_t location_id = "";
  do
    {
      if (strcmp (location_id, "") != 0)
        {
          // report error that merch exists
          // free (location_id);
        }

      location_id = ioopm_ask_location ();
      // repeat until location dose not exist OR location exists and contains
      // the same ID as specified
    }
  while ((location_exist_in_store (store, location_id))
         && get_merch_id_from_location (store, location_id) != id);

  return location_id;
}

/// @brief
/// @param store
/// @param previous_name
/// @return
static merch_t *
ask_for_new_or_same_merch (webstore_t *store, merch_name_t previous_name)
{
  merch_t *new_merch = NULL;
  merch_name_t new_name;
  do
    {
      if (new_merch != NULL)
        {
          // report error that merch exists
          ioopm_destroy_merch (&new_merch);
        }

      new_merch = ioopm_ask_merch ();
      new_name = ioopm_get_merch_name (new_merch);
    }
  while (strcmp (previous_name, new_name)
             != 0 // Repeat if new name is not equal to old name
         && merch_exist_in_store (
             store, new_merch) // AND if merch dose exists already
  );
  return new_merch;
}

/// @brief
/// @param store
/// @param name
/// @param id
static void
add_merch_name_translation (webstore_t *store, merch_name_t name,
                            merch_id_t id)
{
  translation_lookup_table_t merch_translation_lookup
      = get_lookup_table (store);
  ioopm_hash_table_insert (merch_translation_lookup, merch_name_to_elem (name),
                           merch_id_to_elem (id));
  return;
}

/// @brief
/// @param store
/// @param name
static void
remove_merch_name_translation (webstore_t *store, merch_name_t name)
{
  translation_lookup_table_t merch_translation_lookup
      = get_lookup_table (store);
  ioopm_hash_table_remove (merch_translation_lookup,
                           merch_name_to_elem (name));
  return;
}

/// @brief
/// @param store
/// @param old
/// @param new
static void
replace_merch_name_in_lookup (webstore_t *store, merch_name_t old,
                              merch_name_t new)
{
  merch_id_t id = get_merch_id_from_translation (store, old);
  remove_merch_name_translation (store, old);
  add_merch_name_translation (store, new, id);
  return;
}

/// @brief
/// @param store
/// @param old
/// @return
static merch_id_t
get_merch_id_from_translation (webstore_t *store, merch_name_t name)
{
  bool found = false;
  elem_t return_value = ptr_to_elem (NULL);
  translation_lookup_table_t merch_translation_lookup
      = get_lookup_table (store);

  ioopm_hash_table_lookup (&found, &return_value, merch_translation_lookup,
                           string_to_elem (name));
  return elem_to_merch_id (return_value);
}

/// @brief
/// @param store
/// @param id
/// @return
static merch_t *
get_merch_from_available (webstore_t *store, merch_id_t id)
{
  bool found = false;
  elem_t return_value = ptr_to_elem (NULL);
  available_merch_table_t merch = get_merch_table (store);

  ioopm_hash_table_lookup (&found, &return_value, merch,
                           merch_id_to_elem (id));
  return elem_to_merch_ptr (return_value);
}

/// @brief
/// @param store
/// @param id
/// @param merch
static void
add_merch_to_available (webstore_t *store, merch_id_t id, merch_t *merch)
{
  available_merch_table_t available_merch = get_merch_table (store);
  ioopm_hash_table_insert (available_merch, merch_id_to_elem (id),
                           merch_ptr_to_elem (merch));
  return;
}

/// @brief
/// @param store
/// @param id
static void
remove_merch_from_available (webstore_t *store, merch_id_t id,
                             bool delete_locations)
{
  bool found = false;
  elem_t return_value = ptr_to_elem (NULL);
  available_merch_table_t available_merch = get_merch_table (store);
  ioopm_hash_table_lookup (&found, &return_value, available_merch,
                           merch_id_to_elem (id));
  if (found)
    {
      merch_t *old_merch = elem_to_merch_ptr (return_value);
      if (delete_locations)
        {
          ioopm_destroy_merch (&old_merch);
        }
      else
        {
          // free (old_merch->desc);
          // free (old_merch->name);
          // free (old_merch);
        }
      ioopm_hash_table_remove (available_merch, merch_id_to_elem (id));
    }
  return;
}

/// @brief
/// @param store
/// @param id
/// @param new_merch
static void
update_merch_in_available (webstore_t *store, merch_id_t id,
                           merch_t *new_merch)
{
  remove_merch_from_available (store, id, false);
  add_merch_to_available (store, id, new_merch);
  return;
}

/// @brief Adds a given quantity of an item to the given storage location
/// @param store Webstore to modify
/// @param storage_id Storage to add items in
/// @param quantity Number of items to add to the storage
/// @return Location stock struct if location exists, else NULL
static location_stock_t *
get_location_stock_from_location (webstore_t *store,
                                  storage_location_id_t storage_id)
{
  storage_location_table_t storage_locations = get_location_table (store);
  elem_t location_stock_elem = ptr_to_elem (NULL);
  bool was_found = false;
  ioopm_hash_table_lookup (&was_found, &location_stock_elem, storage_locations,
                           string_to_elem (storage_id));
  location_stock_t *location_stock = NULL;
  if (was_found)
    {
      location_stock = elem_to_location_stock_ptr (location_stock_elem);
    }
  return location_stock;
}

/// @brief Adds a given quantity of an item to the given storage location
/// @param store Webstore to modify
/// @param storage_id Storage to add items in
/// @param quantity Number of items to add to the storage
/// @return Quantity of stock in location, returns 0 if location did not exist
static stock_quantity_t
get_quantity_from_location (webstore_t *store,
                            storage_location_id_t storage_id)
{
  location_stock_t *location_stock
      = get_location_stock_from_location (store, storage_id);
  stock_quantity_t stock = 0;
  if (location_stock)
    {
      stock = ((stock_quantity_t)location_stock->stock_quantity);
    }
  return stock;
}

/// @brief Returns the Merch ID connected to the storage location
/// @param store Webstore to search in
/// @param storage_id Storage to look in
/// @return Merch ID that location stores, returns 0 if location did not exist
static merch_id_t
get_merch_id_from_location (webstore_t *store,
                            storage_location_id_t storage_id)
{
  location_stock_t *location_stock
      = get_location_stock_from_location (store, storage_id);
  merch_id_t id = 0;
  if (location_stock)
    {
      id = ((merch_id_t)location_stock->merch_id);
    }
  return id;
}

/// @brief Adds a given quantity of an item to the given storage location
/// @param store Webstore to modify
/// @param storage_id Storage to add items in
/// @param quantity Number of items to add to the storage
/// @return true if location did not exist, false if it did
static bool
add_quantity_to_location (webstore_t *store, storage_location_id_t storage_id,
                          merch_id_t merch_id, stock_quantity_t quantity)
{
  storage_location_table_t storage_locations = get_location_table (store);
  location_stock_t *location_stock
      = get_location_stock_from_location (store, storage_id);
  if (location_stock)
    { // Not Null, it exists
      stock_quantity_t existing_stock = location_stock->stock_quantity;
      location_stock->stock_quantity = quantity + existing_stock;

      // update location with given stock
      ioopm_hash_table_insert (storage_locations,
                               location_id_to_elem (storage_id),
                               location_stock_ptr_to_elem (location_stock));
      return false; // true if location did not exist, else false
    }
  else
    { // NULL, it did not exist
      location_stock = h_alloc_struct (global_heap, "ll");
      location_stock->merch_id = merch_id;
      location_stock->stock_quantity = quantity;

      // Create location with given stock
      ioopm_hash_table_insert (storage_locations,
                               location_id_to_elem (storage_id),
                               location_stock_ptr_to_elem (location_stock));
      return true; // true if location did not exist, else false
    }
}

/// @brief Removes a given quantity of an item from the given storage location
/// @attention Assumes that quantity is less or equal to the amount in storage
/// @param store Webstore to modify
/// @param storage_id Storage to remove from
/// @param quantity Number of items to remove from storage
/// @return true if storage was emptied, false if some quantity remains
static bool
remove_quantity_from_location (webstore_t *store,
                               storage_location_id_t storage_id,
                               stock_quantity_t quantity)
{
  storage_location_table_t storage_locations = get_location_table (store);
  location_stock_t *location_stock
      = get_location_stock_from_location (store, storage_id);
  location_stock->stock_quantity = location_stock->stock_quantity - quantity;
  if (location_stock->stock_quantity <= 0)
    {
      ioopm_hash_table_remove (storage_locations,
                               location_id_to_elem (storage_id));
      // free (location_stock);
      // free (storage_id);
      return true;
    }
  else
    {
      // already changed value, is ther a point to insert?
      ioopm_hash_table_insert (storage_locations,
                               location_id_to_elem (storage_id),
                               location_stock_ptr_to_elem (location_stock));
      return false;
    }
}

/// @brief Removes a given quantity of a given merch across the entire storage
/// @param store Webstore to modify
/// @param merch_id Id of merch to remove specifed quantity of
/// @param quantity Quantity to remove
static void
remove_quantity_from_storage (webstore_t *store, merch_id_t merch_id,
                              stock_quantity_t quantity)
{
  merch_t *merch = get_merch_from_available (store, merch_id);
  // get list of locations that contain stock of this merch
  ioopm_iterator_t *location_id_iter = NULL;
  ioopm_list_iterator (&location_id_iter, merch->locations);

  stock_quantity_t left_to_remove = quantity;

  bool has_next = false;
  while ((ioopm_iterator_has_next (&has_next, location_id_iter) == SUCCESS)
         && has_next && left_to_remove > 0)
    {
      elem_t location;
      ioopm_iterator_next (&location, location_id_iter);
      storage_location_id_t storage_id = elem_to_location_id (location);

      stock_quantity_t in_stock
          = get_quantity_from_location (store, storage_id);
      if (left_to_remove >= in_stock)
        {
          remove_quantity_from_location (store, storage_id, in_stock);
          left_to_remove = left_to_remove - in_stock;

          // we have removed all items from this location
          // remove location from merch
          elem_t unused;
          ioopm_iterator_remove (&unused, location_id_iter);
        }
      else
        {
          remove_quantity_from_location (store, storage_id, left_to_remove);
          left_to_remove = left_to_remove - left_to_remove;
        }
    }

  ioopm_iterator_destroy (&location_id_iter);
  return;
}

/// @brief Gives total quantity of a given merch_id
/// @param store Webstore to search in
/// @param merch_id Id of merch to get quantity
/// @return Total Quantity of the given merch
static stock_quantity_t
get_quantity_from_storage (webstore_t *store, merch_id_t merch_id)
{
  merch_t *merch = get_merch_from_available (store, merch_id);
  // get list of locations that contain stock of this merch
  ioopm_iterator_t *location_id_iter = NULL;
  ioopm_list_iterator (&location_id_iter, merch->locations);

  stock_quantity_t running_sum = 0;

  bool has_next = false;
  while ((ioopm_iterator_has_next (&has_next, location_id_iter) == SUCCESS)
         && has_next)
    {
      elem_t location;
      ioopm_iterator_next (&location, location_id_iter);
      storage_location_id_t storage_id = elem_to_location_id (location);

      running_sum
          = running_sum + get_quantity_from_location (store, storage_id);
    }

  ioopm_iterator_destroy (&location_id_iter);
  return running_sum;
}

/// @brief
/// @param store
/// @param merch_id
/// @return
static stock_quantity_t
get_quantity_in_carts (webstore_t *store, merch_id_t merch_id)
{
  // Get table with all shopping carts
  shopping_cart_table_t shopping_carts = get_cart_table (store);

  // Create iterator over all shopping carts inventory
  ioopm_list_t *shopping_carts_inventory_list = NULL;
  ioopm_hash_table_values (&shopping_carts_inventory_list, shopping_carts);
  ioopm_iterator_t *shopping_carts_inventory_iter = NULL;
  ioopm_list_iterator (&shopping_carts_inventory_iter,
                       shopping_carts_inventory_list);

  // Variable to hold running sum
  stock_quantity_t running_sum = 0;

  bool has_next = false;
  while ((ioopm_iterator_has_next (&has_next, shopping_carts_inventory_iter)
          == SUCCESS)
         && has_next)
    {
      // For each iteration we need to convert elem_t to shopping_cart_t
      // Then check if shopping cart has mapped the merch_id to a quantity
      // if it has then add the quantity to our running sum
      elem_t shopping_cart_inventory_elem;
      ioopm_iterator_next (&shopping_cart_inventory_elem,
                           shopping_carts_inventory_iter);
      shopping_cart_t shopping_cart_inventory
          = elem_to_cart (shopping_cart_inventory_elem);

      elem_t cart_quantity_elem = quantity_to_elem (0);
      bool unused = false;
      ioopm_hash_table_lookup (&unused, &cart_quantity_elem,
                               shopping_cart_inventory,
                               merch_id_to_elem (merch_id));
      stock_quantity_t cart_quantity = elem_to_quantity (cart_quantity_elem);

      running_sum = running_sum + cart_quantity;
    }

  ioopm_iterator_destroy (&shopping_carts_inventory_iter);
  ioopm_linked_list_destroy (&shopping_carts_inventory_list);
  return running_sum;
}

static stock_quantity_t
get_quantity_in_this_cart (webstore_t *store, shopping_cart_id_t cart_id,
                           merch_id_t merch_id)
{
  // Get table with all shopping carts
  shopping_cart_table_t shopping_carts = get_cart_table (store);

  // Create iterator over all shopping carts inventory
  elem_t cart_inventory_elem = ptr_to_elem (NULL);
  bool was_found = false;
  ioopm_hash_table_lookup (&was_found, &cart_inventory_elem, shopping_carts,
                           cart_id_to_elem (cart_id));
  shopping_cart_t cart_inventory = elem_to_cart (cart_inventory_elem);

  elem_t merch_quantity_elem = quantity_to_elem (0);
  ioopm_hash_table_lookup (&was_found, &merch_quantity_elem, cart_inventory,
                           merch_id_to_elem (merch_id));

  return elem_to_quantity (merch_quantity_elem);
}

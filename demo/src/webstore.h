#pragma once

#include "../../lib/linked_list.h"

typedef size_t merch_id_t;
typedef char *merch_name_t;

typedef size_t shopping_cart_id_t;

typedef char *storage_location_id_t;
typedef size_t stock_quantity_t;

typedef struct webstore webstore_t;
typedef struct merch merch_t;

/// @brief Creates a webstore.
/// A webstore holds merch, for each merch we hold information regarding
/// where it is stored and how much is stored at that location.
/// Webstore can also create shopping carts and allow users to purchase merch
/// @return Pointer to a heap allocated webstore. Use ioopm_destroy_webstore to
/// free memory used by a webstore.
webstore_t *ioopm_create_webstore (void);

/// @brief Destroys a webstore and free all its memory.
/// destruction sets pointer to NULL and should be ideally called
/// using the '&' operator on a webstore pointer received from
/// ioopm_create_webstore.
/// @param store Pointer to webstore pointer to destroy. Sets webstore pointer
/// to NULL upon completion.
void ioopm_destroy_webstore (webstore_t **store);

/// @brief Starts the main loop for a webstore.
/// Main loop consists of a loop that prompts User for action
/// the loop continues prompting the user for action until it receives the quit
/// action Upon the Quit action the loop will halt and return to caller.
/// @param store Pointer to webstore pointer that the main loop will use.
void ioopm_webstore_main_loop (webstore_t *store);

/// @brief Asks user for details needed to create a merch
/// When user has entered valid information for all requested fields we will
/// add it to the given store
/// @param store Webstore to add the newly created item to
void ioopm_add_merch (webstore_t *store);

/// @brief Lists all merch stored in the given Webstore in alphabetical order
/// @param store Webstore to list available merch from
void ioopm_list_merch (webstore_t *store);

/// @brief Prompts user for a Merch to remove, if user confirms removal we
/// remove it otherwise we do nothing
/// @param store Webstore to remove a merch from
void ioopm_remove_merch (webstore_t *store);

/// @brief Prompts user for which merch they want to edit,
/// If merch exist then we ask for new name, description and price.
/// Replace old merch with the new information
/// @param store Webstore in which we want to edit a merch item in
void ioopm_edit_merch (webstore_t *store);

/// @brief Prompts user for the name of the merch to show stock for.
/// If merch exist, show all locations that hold some stock of given merch
/// @param store Webstore to use
void ioopm_show_stock (webstore_t *store);

/// @brief Adds som quantity of stock to a given storage location
/// Asks user for merch name, storage location and quantity to add
/// @param store Webstore to modify
void ioopm_refill_stock (webstore_t *store);

/// @brief Creates a new unique shopping cart
/// @param store Webstore to add a cart to
void ioopm_create_cart (webstore_t *store);

/// @brief Destroys a existing shopping cart.
/// Prompts user for which cart to destroy
/// @param store Webstore to destroy a cart in
void ioopm_destroy_cart (webstore_t *store);

/// @brief Reserves som amount of merch to a given cart
// Prompts user for a shopping cart, merch name, and quantity to reserve
/// @param store Webstore instance to modify
void ioopm_add_to_cart (webstore_t *store);

/// @brief Removes reservation of merch from a shopping cart.
/// Prompts user for a shopping cart, merch name, and quantity to remove the
/// reservation for
/// @param store
void ioopm_remove_from_cart (webstore_t *store);

/// @brief Calculates the cost of the currently reserved merch in the shopping
/// cart Prompts user for a shopping cart
/// @param store Webstore to use
void ioopm_calculate_cost (webstore_t *store);

/// @brief Buys the resevered merch from a shopping cart removing them from the
/// system Prompts user for a shopping cart
/// @param store Webstore to use
void ioopm_checkout_cart (webstore_t *store);

/// @brief Creates a merch item using the given name, description and price
/// @param name Name of merch
/// @param description Description of Merch
/// @param price Price of Merch
/// @return A pointer to the newly created merch item
merch_t *ioopm_create_merch (char *name, char *description, int price);

/// @brief Destroys a Merch item, freeing its memory usage and sets its pointer
/// to NULL
/// @param merch A pointer to the merch pointer you want to remove
void ioopm_destroy_merch (merch_t **merch);

/// @brief Extracts the name of the given merch pointer
/// @param merch The merch pointer to grab the name from
/// @return Name of the given merch
merch_name_t ioopm_get_merch_name (merch_t *merch);

/// @brief Extracts the description of the given merch pointer
/// @param merch The merch pointer to grab the description from
/// @return Description of the given merch
char *ioopm_get_merch_desc (merch_t *merch);

/// @brief Extracts the price of the given merch pointer
/// @param merch The merch pointer to grab the price from
/// @return Price of the given merch
int ioopm_get_merch_price (merch_t *merch);

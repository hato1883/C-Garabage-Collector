#pragma once

#include "webstore.h"

/// @brief Propmts user for information needed to create a merch_t and returns
/// it
/// @sa ioopm_ask_merch_name(void)
/// @sa ioopm_ask_merch_description(void)
/// @sa ioopm_ask_merch_price(void)
/// @return newly created merch_t using data received
merch_t *ioopm_ask_merch (void);

/// @brief Prints the total cost of items in a cart
/// @param cart_id Cart which cost was calculated on
/// @param total_cost Total cost of items
void ioopm_print_cart_cost (shopping_cart_id_t cart_id, int total_cost);

/// @brief Prints the header of a recipt
/// @param cart_id The cart that was checked out
void ioopm_print_receipt_header (shopping_cart_id_t cart_id);

/// @brief Prints a entry line of the recipt
/// @param merch_name Name of merch
/// @param merch_quantity Amount of given merch
/// @param merch_cost Cost of given merch
void ioopm_print_receipt_entry (merch_name_t merch_name,
                                stock_quantity_t merch_quantity,
                                int merch_cost);

/// @brief Prints the footer of a recipt
/// @param total_cost The total cost
void ioopm_print_receipt_footer (int total_cost);

/// @brief Propmts user for a merch name
/// @return merch_name_t holding name of a merch
merch_name_t ioopm_ask_merch_name (void);

/// @brief Propmts user for a description
/// @return char * holding a description
char *ioopm_ask_merch_description (void);

/// @brief Propmts user for a price
/// @return int holding the price user entered
int ioopm_ask_merch_price (void);

/// @brief Propmts user for what shopping cart they want to use
/// @return Shopping cart id the user entered
shopping_cart_id_t ioopm_ask_cart_id (void);

/// @brief Propmts user for a location_id (A01 or B32 etc)
/// @return the storage location id the user entered
storage_location_id_t ioopm_ask_location (void);

/// @brief Propmts user a for a quantity of merch
/// @return Quantity of merch to add or remove
stock_quantity_t ioopm_ask_quantity (void);

/// @brief Displays the name, description and the price of a given merchandise
/// @param merch Mrchendise to print to terminal.
void ioopm_print_merch (merch_t *merch);

/// @brief Asks user if they want to print more items
/// @return true if the user wants more items printed otherwise returns false
bool ioopm_should_print_more (void);

/// @brief Asks user for confirmation before removing a specific merch
/// @return true if user wants to remove it, else false
bool ioopm_should_remove_merch (void);

/// @brief Asks user for confirmation before editing a specific merch
/// @return true if user wants to edit it, else false
bool ioopm_should_edit_merch (void);

/// @brief Asks user for confirmation before removing a shopping cart
/// @return true if user wants to remove it, else false
bool ioopm_should_remove_cart (void);

/// @brief Prints the given location id and the quantity of merch stored to the
/// user
/// @param location_id the location that the merch is stored in
/// @param stock_quantity the quantity of merch the location contains
void ioopm_print_storage_location (storage_location_id_t location_id,
                                   stock_quantity_t stock_quantity);

/// @brief Prints the program menu to the user and then prompt user to pick a
/// option
/// @return a single uppercase character (or special/digt character) that the
/// user picked
char ioopm_get_menu_choice (void);

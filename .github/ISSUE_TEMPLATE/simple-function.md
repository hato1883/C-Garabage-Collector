---
name: "⚠️ Manual: SIMPLE Function implementaion specification"
about: Template for documenting a function implementation that is included in a SIMPLE
  Module
title: "[Specification] "
labels: ["🎯 Roadmap", "Status: Triage"]
assignees: 'hato1883'

---

# Module: Module.c <br> Function: ioopm_create_some_data_structure

### Purpose
What will this function do?  
What will it handle?

### Responsibility
- briefly describe purpose.  
Example:  
_Creates a data_struct_t with size of `or_more` input. Used to abstract allocation for internal linked list structure._

### Abstraction Level
- Tier:  
Choose on of the following:
  - 5 used in UI to interact with data
  - 4 used when deciding how to store data from UI (hash table, list, stack, etc)
  - 3 used when operating on a given data structure (hash table, list, stack, etc) (Ex: `add_item_to_hash_table(ht, item)`)
  - 2 used to implement / create data structures (`ht->next = new_link`)
  - 1 used to define helper functions that act bewteen system and data structure (Ex: `while(c && c != 'EOF') { str[i] = c; c = getchar() }`)  

  Note: a Tier 3 abstraction level only uses the same level or one bellow.  
  If a function is Tier 5 and uses some thing on Tier 3, 2 or 1, then it is time to rethink implementation.

### Assumptions
- `x != NULL`
- `y >= 0`

### Returns
- Returns `data_struct_t *` with size `or_more`
- Reports following error code in `error`
  - `ARGUMENT_IS_NULL` error
  - `SIZE_TO_SMALL` error

## Public Dependecies
- [ ] <File_Name.h>

- [ ] <some_data_structure.h>  
  - [ ] `<ioopm_some_public_function>` (from file some_data_structure.c)
  - [ ]  `<ioopm_some_public_other_function>` (from file some_data_structure.c)  

- [ ] <some_utility.h>
  - [ ] `<ioopm_some_public_function>` (from file some_utility.c)  

- [ ] <some_inteface.h>
  - [ ]  `<ioopm_some_public_struct>` (from file some_inteface.c)

## Private (internal Module) Dependecies
- [ ] `<some_internal_helper_function>`
- [ ]  `<some_other_internal_helper_function>`

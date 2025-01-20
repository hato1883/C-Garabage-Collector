---
name: "⚠️ Manual: SIMPLE Module"
about: Template for creating a SIMPLE Module
title: "[Module] "
labels: ["🎯 Roadmap", "Status: Triage"]
assignees: 'hato1883'

---

# <File_Name.c>

## Purpose
What will this module do?  
What will it handle



## Public interface   


### `data_struct_t *ioopm_create_some_data_structure(void *some_argument, int or_more, error_t *error)`
##### Is Completed
- [ ] [Specification] ioopm_create_some_data_structure

##### Responsibility
- briefly describe purpose.  
Example:  
_Creates a data_struct_t with size of `or_more` input. Used to abstract allocation for internal linked list structure._

##### Parameters
- `some_argument` data to be stored
- `or_more` how much data to hold
- `error` error status return pointer


##### Returns
- Returns `data_struct_t *` with size `or_more`
- Reports following error code in `error`
  - `ARGUMENT_IS_NULL` error
  - `SIZE_TO_SMALL` error


### `void ioopm_destroy_some_data_structure(void * some_argument)`
##### Is Completed
- [ ] [Specification] ioopm_destroy_some_data_structure

##### Responsibility
- Text here

##### Parameters
- `some_argument` data struct to destroy

##### Returns
- Reports following error code in `some_error_variable_pointer`
  - `Error_1` error
  - `Error_2` error



## Private interface

### `int get_int_data_from_struct(some_struct_t *some_argument)`
##### Is Completed
- [ ] [Specification] get_int_data_from_struct

##### Responsibility  
- Text here

##### Parameters
- `some_argument` struct to get integer from

##### Returns
- Returns `integer` from given `some_struct_t`


## Depends on
- [ ] <File_Name.h>

- [ ] <some_data_structure.h>  
  - [ ] `<ioopm_some_public_function>` (from file some_data_structure.c)
  - [ ]  `<ioopm_some_public_other_function>` (from file some_data_structure.c)  

- [ ] <some_utility.h>
  - [ ] `<ioopm_some_public_function>` (from file some_utility.c)  

- [ ] <some_inteface.h>
  - [ ]  `<ioopm_some_public_struct>` (from file some_inteface.c)

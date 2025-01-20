#include "../../src/gc.h"
#include "user_interface.h"
#include "webstore.h"

heap_t *heap_ref;

int
main (void)
{
  // Initialize the heap with a specified size (e.g., 1024 bytes), set stack as
  // unsafe, and set a GC threshold.
  h_init (1024 * 1024 * 4, true, 0.75f);

  // Create the webstore using the heap memory.
  webstore_t *store = ioopm_create_webstore ();

  // Run the main loop for the webstore.
  ioopm_webstore_main_loop (store);

  // Clean up by deleting the webstore and heap.
  ioopm_destroy_webstore (&store);
  h_delete (global_heap);

  return 0;
}

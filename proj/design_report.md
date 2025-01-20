# Design Report

## Central functions

### User interface:
    h_init(): Creates a heap for storing objects.
    h_delete(): Deletes the heap and free the allocated memory.
    h_delete_dbg(): Deletes the heap replacing all pointers from the stack with given value.
    h_avail(): Calculates the available memory of the heap.
    h_used(): Calculates the used memory of the heap.
    h_gc(): Manually calls garbage collection.
    h_gc_dbg: Manually calls garbage collection and overrides if stack is considered safe or not.
    h_alloc_struct(): Allocates memory according to the provide format string.
    h_alloc_raw(): Allocates memory according to the size requested.

### Internal key functions:
    h_mark_pages(): Marks intervals that can not be moved due to pointers from unsafe stack.
    h_compact(): Compacts all living object to the beginning of the heap and sets inactive objects to free space.
    apply_to_pointers_in_interval(): function that applies a function to the given interval, used when scanning stack with different functions.
    find_stack_beginning(): finds the start of the stack (lowest stack frame)
    find_stack_end(): finds the end of the stack (highest stack frame)
    size_from_string(): calculates in bytes how by a format string needs.
    move_to_next_available_space(): Increments bump pointer to the first spot that can fit the given size.
    bytes_from_next_page(): Calculates the number of bytes until the next page starts.
    calc_heap_offset(): Calculates the heap pointers offset in bytes from the start of the heap.
    convert_to_bit_vector(): Converts a format string to a bit vector for more compact storage.

### Abstract description of the internal workflow:
- 1.0 Heap initialization
At the start, a user creates a heap, which has a size defined by the user. 

- 2.1 Allocation
The program calculates the amount of memory needed for the allocation.
If the usage of the heap after allocation would be greater than gc_threshold,
then trigger garbage collection before continuing.
The size of the allocation is encoded in a header, if the header is a format string supplied by the user,
the format string may be allocated and stored on the heap first if it can not be encoded
in the header. 
Find an available space in the heap.

- 2.2 Finding available space in heap
The program checks if the current space is free of allocations.
If not, the heap's bump pointer is moved to the first position after the allocated area.
When a free area is found, check if allocation extends into another page,
if that is true, move bump pointer to start of next page and repeat search for available space.

- 2.3 Memory allocation
Write the header to the heap and mark the space as allocated.
Return pointer to the allocated address.

- 3.1 Garbage collection init
Loop through stack variables to mark all pages pointed to by the stack as non-movable.
Mark all spaces in the heap as free
Loop through stack variables to add all alive objects to a queue to be compacted.
Move through all objects in the queue and compact them in order of their heap position.

- 3.2 Compacting an allocation
Calculate size of the allocation using the header
Find the next free space from the start of the heap using 2.2
Note the new address.
Move allocation to the found space.

- 3.3 Garbage collection cleanup
Loop through all pointers on the stack
If it points to an object that has been moved, update the pointer to 
point to the new location.
Count the number of bytes that have been collected

### Central data structures

Vapid GC is a garbage collection system used for handling memory allocation. Our system uses
these central data structures: 

    1. heap_t (Heap structure): This data structure represents a heap where objects kan be dynamically allocated and deallocated. It also handles memory compacting and garbage collection. This structure contains:
    a.	gc_threshold: The percentage of memory that has to be utilized before garbage collection begins.
    b.	uses_safe_pointers: Boolean that describes whether or not stack pointers should be treated as safe.
    c.	size: The size of the heap in bytes.
    d.	page_size: The size of a page in the heap, in bytes. Is defaulted to 2048 bytes.
    e.  page_map: A bitvector array with each bit indicating whether each page is active or not.
    f.	alloc_map: An array of booleans representing if each location (16 bytes) in the heap is used or empty.
    h.	heap_start: The pointer to the heap.
    i.	next_empty_mem_segment: A bump pointer to the next empty available space in the heap that can be used for       allocation.
    j.  used_bytes: The amount of bytes currently allocated by the user.

    2.	ptr_queue_t: A priority queue of pointers structured as a linked list where pointers are enqueued in order of the pointers address. Lower addresses are dequeued first. This is used to be able to compact objects in order, starting from heap_start. This structure contains:
    a.	first: A pointer to the first element in the queue, this element has the lowest address of all pointers in the queue.
    b.	size: How many pointers are in the queue.

    3. compacting_info_t: Contains the data and information that is needed for the compacting phase in the Garbage Collector. This structure contains:
    a. h: a pointer to the heap that is being compacted.
    b. new_alloc_map: A new allocation map for the updated allocations.
    c. num_alive: Count of the objects that are alive and are reachable.
    d. compacting_queue: Queue of the objects that are about to be moved during the compacting.
    e. visited: A queue for tracking the objects that have been visited already.
    f. forwarding_addresses: A queue of the locations for the old objects, needed to be able to update the pointers.

## Program division into modules
    The program is divided into several folders and files, to provide a higher flexibility and to separate all functionalities. We have a separate folder for tests, library files, the main functions and demo code with inlupp2 and other profiling targets.
    
    In the source folder, we divided the functions into the following modules: Allocation functions, heap functions, centralized heap management, stack functions, header functions, pointer checkers, data moving, page mapping and utility functions related to memory management and calculating offsets. All functions are defined in header files which allows them to be used in other files and other folders. 
    
    In demo, the inlupp 2 assignment code is divided into its own src and test folder.
    In the library folder, the program is divided into separate data structures created during inlupp 1.
#ifndef LIST_H
#define LIST_H
#include "include/library/types.h"

// Macro to calculate the offset of a member within a struct type
#define offset(struct_type, member) (int)(&((struct_type *)0)->member)

// Macro to convert an element pointer to the corresponding struct type pointer
#define elem2entry(struct_type, struct_member_name, elem_ptr) \
    (struct_type *)((int)elem_ptr - offset(struct_type, struct_member_name))

// Structure representing a single element in the doubly linked list
typedef struct __list_elem {
    struct __list_elem *prev; // Pointer to the previous element in the list
    struct __list_elem *next; // Pointer to the next element in the list
} list_elem;

// Structure representing the doubly linked list with a head and tail element
typedef struct {
    list_elem head; // Head element of the list (before the first element)
    list_elem tail; // Tail element of the list (after the last element)
} list;

// Type definition for a function pointer that takes a list element and an
// integer as arguments
typedef bool (*functor)(list_elem *, int);

/* Function declarations */

// Initializes a list by setting up the head and tail elements
void list_init(list *_list);

// Inserts an element before another element in the list
void list_insert_before(list_elem *before, list_elem *elem);

// Pushes an element to the front of the list (inserting at the head)
void list_push(list *plist, list_elem *elem);

// Appends an element to the end of the list (inserting at the tail)
void list_append(list *plist, list_elem *elem);

// Removes a specified element from the list
void list_remove(list_elem *pelem);

// Pops (removes and returns) the first element from the list
list_elem *list_pop(list *plist);

// Checks if the list is empty (has no elements)
bool list_empty(list *plist);

// Returns the number of elements in the list
uint32_t list_len(list *plist);

// Traverses the list and applies the given function to each element, passing an
// additional argument
list_elem *list_traversal(list *plist, functor func, int arg);

// Finds a specific element in the list by comparing pointers
bool elem_find(list *plist, list_elem *obj_elem);

#endif

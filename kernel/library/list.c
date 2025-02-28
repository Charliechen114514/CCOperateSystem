#include "include/library/list.h"
#include "include/kernel/interrupt.h"

/* Initialize the doubly linked list */
void list_init(list *list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

/* Insert an element before a given element (before the 'before' element) */
void list_insert_before(list_elem *before, list_elem *elem) {
    Interrupt_Status old_status =
        set_intr_state(INTR_OFF); // Disable interrupts to ensure atomicity

    /* Update the previous node of 'before' to point to 'elem' */
    before->prev->next = elem;

    /* Set 'elem' to point to the previous of 'before' and 'before' itself */
    elem->prev = before->prev;
    elem->next = before;

    /* Update 'before' to point back to 'elem' */
    before->prev = elem;

    set_intr_state(old_status); // Restore the interrupt state
}

/* Push an element to the front of the list (similar to stack push operation) */
void list_push(list *plist, list_elem *elem) {
    list_insert_before(plist->head.next,
                       elem); // Insert at the head of the list
}

/* Append an element to the end of the list (similar to FIFO queue) */
void list_append(list *plist, list_elem *elem) {
    list_insert_before(&plist->tail, elem); // Insert before the tail node
}

/* Remove an element from the list */
void list_remove(list_elem *pelem) {
    Interrupt_Status old_status =
        set_intr_state(INTR_OFF); // Disable interrupts to ensure atomicity

    /* Update the previous node of 'pelem' to point to its next node */
    pelem->prev->next = pelem->next;
    pelem->next->prev = pelem->prev;

    set_intr_state(old_status); // Restore the interrupt state
}

/* Pop the first element from the list and return it (similar to stack pop
 * operation) */
list_elem *list_pop(list *plist) {
    list_elem *elem = plist->head.next;
    list_remove(elem); // Remove the first element
    return elem;
}

/* Find an element in the list. Return true if found, false if not. */
bool elem_find(list *plist, list_elem *obj_elem) {
    list_elem *elem = plist->head.next;
    while (elem != &plist->tail) {
        if (elem == obj_elem) {
            return true; // Element found
        }
        elem = elem->next;
    }
    return false; // Element not found
}

/* Traverse the list and pass each element and the argument to a callback
   function 'func'. If an element satisfies the callback condition, return the
   element; otherwise return NULL. */
list_elem *list_traversal(list *plist, functor func, int arg) {
    list_elem *elem = plist->head.next;
    /* If the list is empty, there's no element to traverse */
    if (list_empty(plist)) {
        return NULL;
    }

    while (elem != &plist->tail) {
        if (func(elem,
                 arg)) { // If callback returns true, return the current element
            return elem;
        }
        elem = elem->next; // Otherwise, continue traversing
    }
    return NULL; // No matching element found
}

/* Return the length of the list */
uint32_t list_len(list *plist) {
    list_elem *elem = plist->head.next;
    uint32_t length = 0;
    while (elem != &plist->tail) {
        length++; // Increment length for each element in the list
        elem = elem->next;
    }
    return length; // Return the total length
}

/* Check if the list is empty. Return true if empty, false otherwise */
bool list_empty(list *plist) {
    return plist->head.next == &plist->tail; // If head points to tail, the list is empty
}

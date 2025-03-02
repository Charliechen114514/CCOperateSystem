#include "include/library/list.h"
#include "include/kernel/interrupt.h"
#include "test/include/test.h"

/* Define a simple callback function for list traversal */
bool test_func(list_elem *elem, unsigned long long arg) {
    return (elem == (list_elem *)(uint64_t)arg); // Check if the element matches the provided argument
}

void test_list_init(void) {
    list test_list;
    list_init(&test_list);

    // Check if the list is initialized correctly (head and tail should be connected)
    TEST_ASSERT(test_list.head.next == &test_list.tail); // head points to tail
    TEST_ASSERT(test_list.tail.prev == &test_list.head); // tail points to head
}

void test_list_push_pop(void) {
    list test_list;
    list_init(&test_list);

    list_elem elem1, elem2;
    
    // Push elements to the list
    list_push(&test_list, &elem1);
    list_push(&test_list, &elem2);

    // Check if the elements are pushed correctly
    TEST_ASSERT(test_list.head.next == &elem2); // The first element should be elem2
    TEST_ASSERT(test_list.tail.prev == &elem1); // The last element should be elem1

    // Pop elements and check
    list_elem *popped_elem = list_pop(&test_list);
    TEST_ASSERT(popped_elem == &elem2); // The popped element should be elem2
    popped_elem = list_pop(&test_list);
    TEST_ASSERT(popped_elem == &elem1); // The popped element should be elem1

    // Check if the list is empty after popping
    TEST_ASSERT(list_empty(&test_list));
}

void test_list_append_remove(void) {
    list test_list;
    list_init(&test_list);

    list_elem elem1, elem2;

    // Append elements to the list
    list_append(&test_list, &elem1);
    list_append(&test_list, &elem2);

    // Check if the elements are appended correctly
    TEST_ASSERT(test_list.head.next == &elem1); // The first element should be elem1
    TEST_ASSERT(test_list.tail.prev == &elem2); // The last element should be elem2

    // Remove the first element and check
    list_remove(&elem1);
    TEST_ASSERT(test_list.head.next == &elem2); // The first element should now be elem2
    TEST_ASSERT(test_list.tail.prev == &elem2); // The last element should also be elem2

    // Remove the second element and check
    list_remove(&elem2);
    TEST_ASSERT(list_empty(&test_list)); // The list should be empty now
}

void test_list_find(void) {
    list test_list;
    list_init(&test_list);

    list_elem elem1, elem2;
    
    // Append elements to the list
    list_append(&test_list, &elem1);
    list_append(&test_list, &elem2);

    // Test the find function
    TEST_ASSERT(elem_find(&test_list, &elem1)); // elem1 should be found
    TEST_ASSERT(elem_find(&test_list, &elem2)); // elem2 should be found

    list_elem elem3;
    TEST_ASSERT(!elem_find(&test_list, &elem3)); // elem3 should not be found
}

void test_list_len(void) {
    list test_list;
    list_init(&test_list);

    list_elem elem1, elem2, elem3;
    
    // Check the length of an empty list
    TEST_ASSERT(list_len(&test_list) == 0);

    // Append elements and check the length
    list_append(&test_list, &elem1);
    TEST_ASSERT(list_len(&test_list) == 1);

    list_append(&test_list, &elem2);
    TEST_ASSERT(list_len(&test_list) == 2);

    list_append(&test_list, &elem3);
    TEST_ASSERT(list_len(&test_list) == 3);

    // Remove an element and check the length
    list_remove(&elem1);
    TEST_ASSERT(list_len(&test_list) == 2);
}

void run_list_test(void) {
    test_list_init();
    TEST_PRINT("list init done");
    test_list_push_pop();
    TEST_PRINT("list push pop done");
    test_list_append_remove();
    TEST_PRINT("list remove down");
    test_list_find();
    TEST_PRINT("list find done");
    test_list_len();
    TEST_PRINT("list len test done");
}

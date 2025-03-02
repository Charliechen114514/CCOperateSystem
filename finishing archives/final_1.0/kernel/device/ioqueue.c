#include "include/io/ioqueue.h"
#include "include/kernel/interrupt.h"
#include "include/library/types.h"
#include "include/library/kernel_assert.h"

/* Initializes the I/O queue structure */
void init_IOQueue(IOQueue* ioq) {
    lock_init(&ioq->locker);  // Initialize the lock for the I/O queue
    ioq->producer = ioq->consumer = NULL;  // Set both producer and consumer to NULL initially
    ioq->index_head = ioq->index_tail = 0; // Set the head and tail indices of the buffer to 0
}

/* Returns the next position in the circular buffer for the given position */
static int32_t next_pos(int32_t pos) {
    return (pos + 1) % IO_BUF_SIZE; // Wraps around when the position reaches the buffer size
}

/* Checks if the queue is full */
bool ioq_full(IOQueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF); // Ensure that interrupts are disabled
    return next_pos(ioq->index_head) == ioq->index_tail; // Full when head's next position is tail
}

/* Checks if the queue is empty */
bool ioq_empty(IOQueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF); // Ensure that interrupts are disabled
    return ioq->index_head == ioq->index_tail; // Empty when head and tail are at the same position
}

/* Makes the current producer or consumer wait on this queue */
static void ioq_wait(TaskStruct** waiter) {
    ASSERT(*waiter == NULL && waiter != NULL); // Ensure that the waiter is currently unset
    *waiter = running_thread(); // Set the current thread as the waiter
    thread_block(TASK_BLOCKED); // Block the thread until it is unblocked
}

/* Wakes up a waiting producer or consumer */
static void wakeup(TaskStruct** waiter) {
    ASSERT(*waiter != NULL); // Ensure that there is a thread to wake up
    thread_unblock(*waiter); // Unblock the waiting thread
    *waiter = NULL; // Reset the waiter pointer to NULL
}

/* Retrieves a character from the I/O queue (used by consumers) */
char ioq_getchar(IOQueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF); // Ensure that interrupts are disabled

    // If the queue is empty, the consumer will wait
    while (ioq_empty(ioq)) {
        lock_acquire(&ioq->locker); // Acquire the queue lock
        ioq_wait(&ioq->consumer);  // Wait until the producer provides data
        lock_release(&ioq->locker); // Release the lock
    }

    char byte = ioq->buf[ioq->index_tail]; // Get the character at the tail index
    ioq->index_tail = next_pos(ioq->index_tail); // Move the tail index to the next position

    // If there is a waiting producer, wake it up
    if (ioq->producer != NULL) {
        wakeup(&ioq->producer);
    }

    return byte; // Return the retrieved character
}

/* Adds a character to the I/O queue (used by producers) */
void ioq_putchar(IOQueue* ioq, char byte) {
    ASSERT(intr_get_status() == INTR_OFF); // Ensure that interrupts are disabled

    // If the queue is full, the producer will wait
    while (ioq_full(ioq)) {
        lock_acquire(&ioq->locker); // Acquire the queue lock
        ioq_wait(&ioq->producer);  // Wait until the consumer consumes data
        lock_release(&ioq->locker); // Release the lock
    }

    ioq->buf[ioq->index_head] = byte; // Add the character to the buffer at the head index
    ioq->index_head = next_pos(ioq->index_head); // Move the head index to the next position

    // If there is a waiting consumer, wake it up
    if (ioq->consumer != NULL) {
        wakeup(&ioq->consumer);
    }
}

/* Returns the current length of the circular buffer */
uint32_t ioq_length(IOQueue* ioq) {
    uint32_t len = 0;

    // Calculate the length based on the position of head and tail indices
    if (ioq->index_head >= ioq->index_tail) {
        len = ioq->index_head - ioq->index_tail; // Simple subtraction when head is ahead of tail
    } else {
        len = IO_BUF_SIZE - (ioq->index_tail - ioq->index_head); // Wrap around case
    }

    return len; // Return the length of the buffer
}

#include "include/kernel/lock.h"
#include "include/kernel/interrupt.h"
#include "include/library/kernel_assert.h"
#include "include/library/list.h"
#include "include/library/types.h"

/* Initialize semaphore */
void sema_init(Semaphore *psema, uint8_t value) {
    psema->value = value;       // Set the initial value of the semaphore
    list_init(&psema->waiters); // Initialize the semaphore's waiting list
}

/* Initialize lock plock */
void lock_init(CCLocker *plock) {
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_init(&plock->semaphore, 1); // Initialize the semaphore with value 1
}

/* Semaphore down operation (P operation) */
void sema_down(Semaphore *psema) {
    /* Disable interrupts to ensure atomic operation */
    Interrupt_Status old_status = set_intr_status(INTR_OFF);
    while (psema->value ==
           0) { // If value is 0, it means the semaphore is held by someone else
        KERNEL_ASSERT(!elem_find(&psema->waiters, &current_thread()->general_tag));
        /* The current thread should not already be in the semaphore's waiters
         * list */
        if (elem_find(&psema->waiters, &current_thread()->general_tag)) {
            KERNEL_PANIC_SPIN(
                "sema_down: thread blocked has been in waiters_list\n");
        }
        /* If semaphore's value is 0, the current thread adds itself to the
         * waiters list and blocks itself */
        list_append(&psema->waiters, &current_thread()->general_tag);
        thread_block(TASK_BLOCKED); // Block the thread until it is awakened
    }
    /* If value is 1 or the thread is awakened, proceed with acquiring the lock
     */
    psema->value--;
    KERNEL_ASSERT(psema->value == 0);
    /* Restore the previous interrupt status */
    set_intr_status(old_status);
}

/* Semaphore up operation (V operation) */
void sema_up(Semaphore *psema) {
    /* Disable interrupts to ensure atomic operation */
    Interrupt_Status old_status = set_intr_status(INTR_OFF);
    KERNEL_ASSERT(psema->value == 0);
    if (!list_empty(&psema->waiters)) {
        TaskStruct *thread_blocked =
            elem2entry(TaskStruct, general_tag, list_pop(&psema->waiters));
        thread_unblock(thread_blocked); // Unblock the thread that was waiting
                                        // for the semaphore
    }
    psema->value++;
    KERNEL_ASSERT(psema->value == 1);
    /* Restore the previous interrupt status */
    set_intr_status(old_status);
}

/* Acquire lock plock */
void lock_acquire(CCLocker *plock) {
    /* Exclude the case where the current thread already holds the lock but
     * hasn't released it */
    if (plock->holder != current_thread()) {
        sema_down(
            &plock->semaphore); // Perform P operation on semaphore (atomic)
        plock->holder = current_thread();
        KERNEL_ASSERT(plock->holder_repeat_nr == 0);
        plock->holder_repeat_nr = 1;
    } else {
        plock->holder_repeat_nr++; // If the thread already holds the lock,
                                   // increment the repeat count
    }
}

/* Release lock plock */
void lock_release(CCLocker *plock) {
    KERNEL_ASSERT(plock->holder == current_thread());
    if (plock->holder_repeat_nr > 1) {
        plock->holder_repeat_nr--; // Decrease repeat count if the thread holds
                                   // the lock multiple times
        return;
    }
    KERNEL_ASSERT(plock->holder_repeat_nr == 1);

    plock->holder =
        NULL; // Set lock holder to NULL before performing V operation
    plock->holder_repeat_nr = 0;
    sema_up(&plock->semaphore); // Perform V operation on semaphore (atomic)
}

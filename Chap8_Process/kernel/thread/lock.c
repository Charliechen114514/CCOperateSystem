#include "include/kernel/lock.h"
#include "include/kernel/interrupt.h"
#include "include/kernel/thread.h"
#include "include/library/assert.h"

void init_semaphore(CCSemaphore* psema, uint8_t value)
{
    psema->core_value = value;
    list_init(&psema->waiters);
}

void down_semaphore(CCSemaphore* psema)
{
/* 关中断来保证原子操作 */
    Interrupt_Status status = disable_intr();

    while(!psema->core_value){
        ASSERT(!elem_find(&psema->waiters, &running_thread()->general_tag));
        if (elem_find(&psema->waiters, &running_thread()->general_tag)){
            // Ah! shit, how could the holdings is also been waiting!
            KERNEL_PANIC(   "down_semaphore dumped!: "
                            "thread blocked has been in waiters_list!"
                            "as waiting himself is stupid :(\n");
        }
        list_append(&psema->waiters, &running_thread()->general_tag); 
        thread_block(TASK_BLOCKED);    // 阻塞线程,直到被唤醒
    }
    psema->core_value--;
    ASSERT(!psema->core_value);

    set_intr_state(status);
}

void up_semaphore(CCSemaphore* psema)
{
    Interrupt_Status status = disable_intr();
    ASSERT(!psema->core_value);
    if (!list_empty(&psema->waiters)) {
        TaskStruct* thread_blocked = elem2entry(TaskStruct, general_tag, list_pop(&psema->waiters));
        thread_unblock(thread_blocked);
    }

    psema->core_value++;
    ASSERT(psema->core_value);
    set_intr_state(status);
}

void init_locker(CCLocker* plock)
{
    plock->holding_thread = NULL;
    plock->locker_depth = 0;
    init_semaphore(&plock->semaphore, 1);
}

void acquire_lock(CCLocker* plock)
{
    if(plock->holding_thread != running_thread()){
        down_semaphore(&plock->semaphore);
        plock->holding_thread = running_thread();
        ASSERT(!plock->locker_depth);
        plock->locker_depth = 1;
    }else{
        plock->locker_depth++;
    }
}

void release_lock(CCLocker* plock){
    // should be itself release!
    ASSERT(plock->holding_thread == running_thread());
    if(plock->locker_depth > 1){
        plock->locker_depth--;
        return;
    }
    ASSERT(plock->locker_depth == 1);
    plock->holding_thread = NULL;
    plock->locker_depth = 0;
    up_semaphore(&plock->semaphore);
}
    
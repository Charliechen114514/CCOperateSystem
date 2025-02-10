#ifndef LOCK_H
#define LOCK_H

#include "include/library/list.h"
#include "include/library/types.h"
#include "include/kernel/thread.h"


typedef struct {
    uint8_t     core_value;
    list        waiters;
}CCSemaphore;


typedef struct {
    TaskStruct*     holding_thread;
    CCSemaphore     semaphore;
    uint32_t        locker_depth;
}CCLocker;


void init_semaphore(CCSemaphore* psema, uint8_t value); 
void down_semaphore(CCSemaphore* psema);
void up_semaphore(CCSemaphore* psema);
void init_locker(CCLocker* plock);
void acquire_lock(CCLocker* plock);
void release_lock(CCLocker* plock);
#endif
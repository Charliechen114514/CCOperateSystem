#ifndef LOCK_H
#define LOCK_H
#include "include/library/list.h"
#include "include/library/types.h"
#include "include/kernel/thread.h"

/* 信号量结构 */
typedef struct {
   uint8_t  value;
   list     waiters;
}Semaphore;

/* 锁结构 */
typedef struct {
   TaskStruct* holder;	    // 锁的持有者
   Semaphore   semaphore;	    // 用二元信号量实现锁
   uint32_t holder_repeat_nr;		    // 锁的持有者重复申请锁的次数
}CCLocker;

void sema_init(Semaphore* psema, uint8_t value); 
void sema_down(Semaphore* psema);
void sema_up(Semaphore* psema);
void lock_init(CCLocker* plock);
void lock_acquire(CCLocker* plock);
void lock_release(CCLocker* plock);

#endif
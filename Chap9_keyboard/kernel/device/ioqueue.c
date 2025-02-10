#include "include/io/ioqueue.h"
#include "include/kernel/interrupt.h"
#include "include/library/assert.h"

/* 返回pos在缓冲区中的下一个位置值 */
static inline int32_t next_pos(int32_t pos) {
    return (pos + 1) % IO_BUF_SIZE; 
}

/* 判断队列是否已空 */
bool ioq_empty(IOQueue* ioq) {
    ASSERT(get_intr_state() == INTR_OFF);
    return ioq->index_head == ioq->index_tail;
 }
 
// init the io queue
void    init_IOQueue(IOQueue* ioq)
{
    init_locker(&ioq->locker);
    ioq->producer = NULL;
    ioq->consumer = NULL;
    ioq->index_head = 0;
    ioq->index_tail = 0;
}

bool    ioq_full(IOQueue* ioq)
{
    ASSERT(get_intr_state() == INTR_OFF);
    return next_pos(ioq->index_head) == ioq->index_tail;
}

/* 使当前生产者或消费者在此缓冲区上等待 */
static void ioq_wait(struct task_struct** waiter) {
    ASSERT(waiter != NULL && *waiter == NULL);
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}

/* 唤醒waiter */
static void wakeup(struct task_struct** waiter) {
    ASSERT(*waiter != NULL);
    thread_unblock(*waiter); 
    *waiter = NULL;
}

/* 消费者从ioq队列中获取一个字符 */
char ioq_getchar(IOQueue* ioq) {
   ASSERT(get_intr_state() == INTR_OFF);

/* 若缓冲区(队列)为空,把消费者ioq->consumer记为当前线程自己,
 * 目的是将来生产者往缓冲区里装商品后,生产者知道唤醒哪个消费者,
 * 也就是唤醒当前线程自己*/
    while (ioq_empty(ioq)) {
        acquire_lock(&ioq->locker);	 
        ioq_wait(&ioq->consumer);
        release_lock(&ioq->locker);
    }

    char byte = ioq->buf[ioq->index_tail];	  // 从缓冲区中取出
    ioq->index_tail = next_pos(ioq->index_tail);	  // 把读游标移到下一位置

    if (ioq->producer != NULL) {
        wakeup(&ioq->producer);		  // 唤醒生产者
    }

    return byte; 
}

/* 生产者往ioq队列中写入一个字符byte */
void ioq_putchar(IOQueue* ioq, char byte) {
    ASSERT(get_intr_state() == INTR_OFF);

/* 若缓冲区(队列)已经满了,把生产者ioq->producer记为自己,
 * 为的是当缓冲区里的东西被消费者取完后让消费者知道唤醒哪个生产者,
 * 也就是唤醒当前线程自己*/
    while (ioq_full(ioq)) {
        acquire_lock(&ioq->locker);	 
        ioq_wait(&ioq->producer);
        release_lock(&ioq->locker);
    }
    ioq->buf[ioq->index_head] = byte;      // 把字节放入缓冲区中
    ioq->index_head = next_pos(ioq->index_head); // 把写游标移到下一位置

    if (ioq->consumer != NULL) {
        wakeup(&ioq->consumer);          // 唤醒消费者
    }
}

/* 返回环形缓冲区中的数据长度 */
uint32_t ioq_length(IOQueue* ioq) {
    uint32_t len = 0;
    if (ioq->index_head >= ioq->index_tail) {
        len = ioq->index_head - ioq->index_tail;
    } else {
        len = IO_BUF_SIZE - (ioq->index_tail - ioq->index_head);     
    }
    return len;
}

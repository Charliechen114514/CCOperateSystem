#ifndef IOQUEUE_H
#define IOQUEUE_H

#include "include/library/types.h"
#include "include/thread/thread.h"
#include "include/kernel/lock.h"


#define IO_BUF_SIZE     (2048)

typedef struct{
    TaskStruct* producer;
    TaskStruct* consumer;
    CCLocker    locker;
    char        buf[IO_BUF_SIZE];
    int32_t     index_head;
    int32_t     index_tail;
}IOQueue;

void        init_IOQueue(IOQueue* ioq);
bool        ioq_full(IOQueue* ioq);
bool        ioq_empty(IOQueue* ioq);
char        ioq_getchar(IOQueue* ioq);
void        ioq_putchar(IOQueue* ioq, char byte);
uint32_t    ioq_length(IOQueue* ioq);
#endif
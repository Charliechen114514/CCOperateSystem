#include "include/library/types.h"
#include "include/kernel/lock.h"
#include "include/settings.h"
typedef struct __cctaskstruct TaskStruct;
// kernel io buffer size
#define IO_BUF_SIZE     (32)

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
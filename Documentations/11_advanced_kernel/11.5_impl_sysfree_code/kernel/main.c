#include "include/device/console_tty.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/thread/thread.h"
#include "include/user/process/process.h"
#include "include/user/syscall/syscall.h"
#include "include/user/syscall/syscall_init.h"
#include "include/user/stdio/stdio.h"
#include "include/memory/memory.h"
#include "include/library/ccos_print.h"
void thread_a(void *args);
void thread_b(void *args);
void u_prog_a(void);
void u_prog_b(void);
int prog_a_pid = 0, prog_b_pid = 0; 

int main(void) {
    init_all();
    interrupt_enabled();
    thread_start("k_thread_a", 31, thread_a, "I am thread_a");
    thread_start("k_thread_b", 31, thread_b, "I am thread_b");
    while (1);
    return 0;
}

void thread_a(void* arg) {
    char* para = arg;
    void *addr1, *addr2, *addr3, *addr4, *addr5, *addr6, *addr7;
    int index = 0, max = 100;
    console_ccos_puts(" thread_a start\n");

    while (max-- > 0) {
        int size = 128;
        addr1 = sys_malloc(size);
        addr2 = sys_malloc(size *= 2);
        addr3 = sys_malloc(size *= 2);
        sys_free(addr1);
        addr4 = sys_malloc(size);
        size *= 128;
        addr5 = sys_malloc(size);
        addr6 = sys_malloc(size);
        sys_free(addr5);
        addr7 = sys_malloc(size *= 2);
        sys_free(addr6);
        sys_free(addr7);
        sys_free(addr2);
        sys_free(addr3);
        sys_free(addr4);
        ccos_puts(" thread_a end: ");
        __ccos_display_int(index++);
        __ccos_putchar('\n');
    }

    while (1);
}

void thread_b(void* arg) {
    char* para = arg;
    void *addr1, *addr2, *addr3, *addr4, *addr5, *addr6, *addr7, *addr8, *addr9;
    int index = 0, max = 100;
    console_ccos_puts(" thread_b start\n");

    while (max-- > 0) {
        int size = 9;
        addr1 = sys_malloc(size);
        addr2 = sys_malloc(size *= 2);
        sys_free(addr2);
        addr3 = sys_malloc(size *= 2);
        sys_free(addr1);
        addr4 = sys_malloc(size);
        addr5 = sys_malloc(size);
        addr6 = sys_malloc(size);
        sys_free(addr5);
        addr7 = sys_malloc(size *= 2);
        sys_free(addr6);
        sys_free(addr7);
        sys_free(addr3);
        sys_free(addr4);

        size *= 8;
        addr1 = sys_malloc(size);
        addr2 = sys_malloc(size);
        addr3 = sys_malloc(size);
        addr4 = sys_malloc(size);
        addr5 = sys_malloc(size);
        addr6 = sys_malloc(size);
        addr7 = sys_malloc(size);
        addr8 = sys_malloc(size);
        addr9 = sys_malloc(size);

        sys_free(addr1);
        sys_free(addr2);
        sys_free(addr3);
        sys_free(addr4);
        sys_free(addr5);
        sys_free(addr6);
        sys_free(addr7);
        sys_free(addr8);
        sys_free(addr9);
        ccos_puts(" thread_b end: ");
        __ccos_display_int(index++);
        __ccos_putchar('\n');
    }

    while (1);
}

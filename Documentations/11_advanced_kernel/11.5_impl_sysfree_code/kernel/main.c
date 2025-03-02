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
    thread_start("k_thread_b", 31, thread_b, "I am thread_b "); 
    while(1); 
    return 0; 
} 

/* 在线程中运行的函数 */ 
void thread_a(void* arg) { 
    char* para = arg; 
    void* addr1; 
    void* addr2; 
    void* addr3; 
    void* addr4; 
    void* addr5; 
    void* addr6; 
    void* addr7; 
    console_ccos_puts(" thread_a start\n"); 
    int index = 0;
    int max = 100; 
    while (max-- > 0) { 
        int size = 128; 
        addr1 = sys_malloc(size); 
        size *= 2; 
        addr2 = sys_malloc(size); 
        size *= 2; 
        addr3 = sys_malloc(size); 
        sys_free(addr1); 
        addr4 = sys_malloc(size); 
        size *= 2; size *= 2; size *= 2; size *= 2; 
        size *= 2; size *= 2; size *= 2; 
        addr5 = sys_malloc(size); 
        addr6 = sys_malloc(size); 
        sys_free(addr5); 
        size *= 2; 
        addr7 = sys_malloc(size); 
        sys_free(addr6); 
        sys_free(addr7); 
        sys_free(addr2); 
        sys_free(addr3); 
        sys_free(addr4); 
        ccos_puts(" thread_a end: ");
        __ccos_display_int(index);
        __ccos_putchar('\n');
        index++;
    } 
    while(1); 
} 

/* 在线程中运行的函数 */ 
void thread_b(void* arg) { 
    char* para = arg; 
    void* addr1; 
    void* addr2; 
    void* addr3; 
    void* addr4; 
    void* addr5; 
    void* addr6; 
    void* addr7; 
    void* addr8; 
    void* addr9; 
    int max = 100; 
    int index = 0;
    console_ccos_puts(" thread_b start\n"); 
    while (max-- > 0) { 
        int size = 9;  
        
        addr1 = sys_malloc(size); 
        
        size *= 2; 
        addr2 = sys_malloc(size); 
        size *= 2; 
        
        sys_free(addr2); 
        addr3 = sys_malloc(size); 
        sys_free(addr1); 
        addr4 = sys_malloc(size); 
        addr5 = sys_malloc(size); 
        addr6 = sys_malloc(size); 
        sys_free(addr5); 
        size *= 2; 
        addr7 = sys_malloc(size); 
        sys_free(addr6); 
        sys_free(addr7); 
        sys_free(addr3); 
        sys_free(addr4); 

        size *= 2; size *= 2; size *= 2; 
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
        __ccos_display_int(index);
        __ccos_putchar('\n');
        index++;
    } 
    while(1);
}

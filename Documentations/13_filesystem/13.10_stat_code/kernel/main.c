#include "include/device/console_tty.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/thread/thread.h"
#include "include/user/stdio/stdio.h"
#include "include/memory/memory.h"
#include "include/library/ccos_print.h"
#include "include/filesystem/filesystem.h"
#include "include/library/string.h"
#include "include/filesystem/dir.h"
// void thread_a(void *args);
// void thread_b(void *args);
// void u_prog_a(void);
// void u_prog_b(void);
// int prog_a_pid = 0, prog_b_pid = 0;

int main(void)
{
    init_all();
    Stat obj_stat;
    sys_stat("/", &obj_stat);
    printf("/`s info\n   i_no:%d\n   size:%d\n   filetype:%s\n",
           obj_stat.st_ino, obj_stat.st_size,
           obj_stat.st_filetype == 2 ? "directory" : "regular");
    sys_stat("/dir1", &obj_stat);
    printf("/dir1`s info\n   i_no:%d\n   size:%d\n   filetype:%s\n",
           obj_stat.st_ino, obj_stat.st_size,
           obj_stat.st_filetype == 2 ? "directory" : "regular");

    while (1)
        ;
    return 0;
}

// /* 在线程中运行的函数 */
// void thread_a(void* arg) {
//     void* addr1 = sys_malloc(256);
//     void* addr2 = sys_malloc(255);
//     void* addr3 = sys_malloc(254);
//     console_ccos_puts(" thread_a malloc addr:0x");
//     console__ccos_display_int((int)addr1);
//     console__ccos_putchar(',');
//     console__ccos_display_int((int)addr2);
//     console__ccos_putchar(',');
//     console__ccos_display_int((int)addr3);
//     console__ccos_putchar('\n');

//     int cpu_delay = 100000;
//     while(cpu_delay-- > 0);
//     sys_free(addr1);
//     sys_free(addr2);
//     sys_free(addr3);
//     while(1);
// }

// /* 在线程中运行的函数 */
// void thread_b(void* arg) {
//     void* addr1 = sys_malloc(256);
//     void* addr2 = sys_malloc(255);
//     void* addr3 = sys_malloc(254);
//     console_ccos_puts(" thread_b malloc addr:0x");
//     console__ccos_display_int((int)addr1);
//     console__ccos_putchar(',');
//     console__ccos_display_int((int)addr2);
//     console__ccos_putchar(',');
//     console__ccos_display_int((int)addr3);
//     console__ccos_putchar('\n');

//     int cpu_delay = 100000;
//     while(cpu_delay-- > 0);
//     sys_free(addr1);
//     sys_free(addr2);
//     sys_free(addr3);
//     while(1);
// }

// void u_prog_a(void) {
//     void* addr1 = malloc(256);
//     void* addr2 = malloc(255);
//     void* addr3 = malloc(254);
//     printf(" prog_a malloc addr:0x%x,0x%x,0x%x\n",
//         (int)addr1, (int)addr2, (int)addr3);

//     int cpu_delay = 100000;
//     while(cpu_delay-- > 0);
//     free(addr1);
//     free(addr2);
//     free(addr3);
//     while(1);
// }

// /* 测试用户进程 */
// void u_prog_b(void) {
//     void* addr1 = malloc(256);
//     void* addr2 = malloc(255);
//     void* addr3 = malloc(254);
//     printf(" prog_a malloc addr:0x%x,0x%x,0x%x\n",
//         (int)addr1, (int)addr2, (int)addr3);

//     int cpu_delay = 100000;
//     while(cpu_delay-- > 0);
//     free(addr1);
//     free(addr2);
//     free(addr3);
//     while(1);
// }

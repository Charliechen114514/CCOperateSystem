#include "include/thread/thread_relative_print.h"
#include "include/library/types.h"
#include "include/library/string.h"
#include "include/filesystem/file.h"
#include "include/library/kernel_assert.h"
#include "include/filesystem/filesystem.h"
#include "include/user/stdio/stdio.h"
/* Output buf with padded spaces */
static void pad_print(char *buf, int32_t buf_len, void *ptr, char format) {
    k_memset(buf, 0, buf_len); // Clear the buffer
    uint8_t out_pad_0idx = 0;
    switch (format) {
    case 's':
        out_pad_0idx = sprintf(buf, "%s", ptr); // Print string
        break;
    case 'd':
        out_pad_0idx =
            sprintf(buf, "%d", *((int16_t *)ptr)); // Print decimal integer
        goto PRT_HEX;
        break;
    case 'x':
    PRT_HEX:
        out_pad_0idx =
            sprintf(buf, "%x", *((uint32_t *)ptr)); // Print hexadecimal
    }
    while (out_pad_0idx <
           buf_len) { // Pad with spaces until the buffer is filled
        buf[out_pad_0idx] = ' ';
        out_pad_0idx++;
    }
    sys_write(stdout_no, buf, buf_len - 1); // Output the formatted string
}

/* Callback function used in list_traversal for processing thread queue */
static bool elem2thread_info(list_elem *pelem, int arg) {
    (void)arg; // Unused argument
    TaskStruct *pthread = elem2entry(TaskStruct, all_list_tag,
                                     pelem); // Convert list element to thread
    char out_pad[16] = {0};

    /* Print PID of the thread */
    pad_print(out_pad, 16, &pthread->pid, 'd');

    /* Print Parent PID, "NULL" if the parent PID is -1 */
    if (pthread->parent_pid == -1) {
        pad_print(out_pad, 16, "NULL", 's');
    } else {
        pad_print(out_pad, 16, &pthread->parent_pid, 'd');
    }

    /* Print thread status */
    switch (pthread->status) {
    case 0:
        pad_print(out_pad, 16, "RUNNING", 's');
        break;
    case 1:
        pad_print(out_pad, 16, "READY", 's');
        break;
    case 2:
        pad_print(out_pad, 16, "BLOCKED", 's');
        break;
    case 3:
        pad_print(out_pad, 16, "WAITING", 's');
        break;
    case 4:
        pad_print(out_pad, 16, "HANGING", 's');
        break;
    case 5:
        pad_print(out_pad, 16, "DIED", 's');
    }

    /* Print elapsed ticks in hexadecimal format */
    pad_print(out_pad, 16, &pthread->elapsed_ticks, 'x');

    /* Print thread name */
    k_memset(out_pad, 0, 16);
    KERNEL_ASSERT(k_strlen(pthread->name) < 17);
    k_memcpy(out_pad, pthread->name, k_strlen(pthread->name));
    k_strcat(out_pad, "\n");
    sys_write(stdout_no, out_pad, k_strlen(out_pad));

    return false; // Return false to continue traversing the list
}

/* Print the task list */
void sys_ps(void) {
    char *ps_title =
        "PID            PPID           STAT           TICKS          COMMAND\n";
    sys_write(stdout_no, ps_title, k_strlen(ps_title)); // Print the header
    list_traversal(&thread_all_list, elem2thread_info,
                   0); // Traverse and print each thread's info
}
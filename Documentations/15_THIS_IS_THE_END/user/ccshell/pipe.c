#include "include/user/ccshell/pipe.h"
#include "include/filesystem/file.h"
#include "include/filesystem/filesystem.h"
#include "include/io/ioqueue.h"
#include "include/thread/thread.h"
#include "include/memory/memory.h"

/* Checks if the file descriptor 'local_fd' is a pipe */
bool is_pipe(uint32_t local_fd) {
    uint32_t global_fd = fd_local2global(local_fd);
    return file_table[global_fd].fd_flag == PIPE_FLAG;
}

/* Creates a pipe. Returns 0 on success, -1 on failure */
int32_t sys_pipe(int32_t pipefd[2]) {
    int32_t global_fd = get_free_slot_in_global();

    /* Allocate a kernel memory page for the ring buffer */
    file_table[global_fd].fd_inode = get_kernel_pages(1);

    /* Initialize the ring buffer */
    init_IOQueue((IOQueue *)file_table[global_fd].fd_inode);
    if (!(file_table[global_fd].fd_inode)) {
        return -1;
    }

    /* Reuse 'fd_flag' as the pipe flag */
    file_table[global_fd].fd_flag = PIPE_FLAG;

    /* Reuse 'fd_pos' as the pipe open count */
    file_table[global_fd].fd_pos = 2;
    pipefd[0] = pcb_fd_install(global_fd);
    pipefd[1] = pcb_fd_install(global_fd);
    return 0;
}

/* Reads data from the pipe */
uint32_t pipe_read(int32_t fd, void *buf, uint32_t count) {
    char *buffer = buf;
    uint32_t bytes_read = 0;
    uint32_t global_fd = fd_local2global(fd);

    /* Get the ring buffer of the pipe */
    IOQueue *ioq = (IOQueue *)file_table[global_fd].fd_inode;

    /* Select the smaller data reading size to avoid blocking */
    uint32_t ioq_len = ioq_length(ioq);
    uint32_t size = ioq_len > count ? count : ioq_len;
    while (bytes_read < size) {
        *buffer = ioq_getchar(ioq);
        bytes_read++;
        buffer++;
    }
    return bytes_read;
}

/* Writes data to the pipe */
uint32_t pipe_write(int32_t fd, const void *buf, uint32_t count) {
    uint32_t bytes_write = 0;
    uint32_t global_fd = fd_local2global(fd);
    IOQueue *ioq = (IOQueue *)file_table[global_fd].fd_inode;

    /* Select the smaller data writing size to avoid blocking */
    uint32_t ioq_left = IO_BUF_SIZE - ioq_length(ioq);
    uint32_t size = ioq_left > count ? count : ioq_left;

    const char *buffer = buf;
    while (bytes_write < size) {
        ioq_putchar(ioq, *buffer);
        bytes_write++;
        buffer++;
    }
    return bytes_write;
}

/* Redirects the file descriptor 'old_local_fd' to 'new_local_fd' */
void sys_fd_redirect(uint32_t old_local_fd, uint32_t new_local_fd) {
    TaskStruct *cur = current_thread();
    /* Handle restoring standard descriptors */
    if (new_local_fd < 3) {
        cur->fd_table[old_local_fd] = new_local_fd;
    } else {
        uint32_t new_global_fd = cur->fd_table[new_local_fd];
        cur->fd_table[old_local_fd] = new_global_fd;
    }
}

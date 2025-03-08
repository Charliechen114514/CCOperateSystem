#include "include/user/ccshell/ccshell.h"
#include "include/defines.h"
#include "include/filesystem/file.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/syscall/syscall.h"
#include "include/user/stdio/stdio.h"
#include "include/filesystem/filesystem_settings.h"
#include "include/user/library/user_assertion.h"
#define CMD_LEN MAX_PATH_LEN
#define MAX_ARG_NR (16)

/* Stores the input command */
static char cmd_line[CMD_LEN] = {0};

/* Used to record the current directory; it is updated every time the cd command
 * is executed */
char cwd_cache[MAX_PATH_LEN] = {0};

/* Outputs the shell prompt */
void print_prompt(void)
{
    printf("[" HOST_NAME "@localhost %s]$ ", cwd_cache);
}

/* Reads up to 'count' bytes from the keyboard buffer into 'buf' */
static void readline(char *buf, int32_t count)
{
    user_assert(buf && count > 0);
    char *pos = buf;

    while (read(stdin_no, pos, 1) != -1 &&
           (pos - buf) < count)
    { // Read until enter key is found
        switch (*pos)
        {
            /* If enter or newline is found, treat it as the end of the command
             */
        case '\n':
        case '\r':
            *pos = 0; // Add null terminator to cmd_line
            putchar('\n');
            return;

        case '\b':
            if (cmd_line[0] != '\b')
            {          // Prevent deleting non-inputted data
                --pos; // Move back to the previous character in the buffer
                putchar('\b');
            }
            break;

        /* For other characters, output normally */
        default:
            putchar(*pos);
            pos++;
        }
    }
    printf("readline: can't find enter_key in the cmd_line, max num of char is "
           "128\n");
}

void ccshell(void)
{
    cwd_cache[0] = '/';
    while (1)
    {
        print_prompt();
        k_memset(cmd_line, 0, CMD_LEN);
        readline(cmd_line, CMD_LEN);
        if (cmd_line[0] == 0)
        {
            continue;
        }
    }
    user_panic("Man!: you should not be here!!!");
}
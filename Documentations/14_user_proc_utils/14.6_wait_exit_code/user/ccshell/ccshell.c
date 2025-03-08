#include "include/user/ccshell/ccshell.h"
#include "include/defines.h"
#include "include/filesystem/file.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/syscall/syscall.h"
#include "include/user/stdio/stdio.h"
#include "include/filesystem/filesystem_settings.h"
#include "include/user/library/user_assertion.h"
#include "include/user/ccshell/buildin_cmd.h"
#define CMD_LEN MAX_PATH_LEN
#define MAX_ARG_NR (16)

/* Stores the input command */
static char cmd_line[CMD_LEN] = {0};
char final_path[MAX_PATH_LEN] = {0}; // Buffer for cleaning paths
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
        /* ctrl+l clears the screen */
        case 'l' - 'a':
            *pos = 0;
            clear();           // Clear the screen
            print_prompt();    // Print the prompt
            printf("%s", buf); // Print the previous input
            break;

        /* ctrl+u clears the input */
        case 'u' - 'a':
            while (buf != pos)
            {
                putchar('\b');
                *(pos--) = 0;
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

/* Parses the string 'cmd_str' into words, using 'token' as the delimiter,
 * storing the pointers to words in 'argv' */
static int32_t cmd_parse(char *cmd_str, char **argv, char token)
{
    user_assert(cmd_str);
    int32_t arg_idx = 0;
    while (arg_idx < MAX_ARG_NR)
    {
        argv[arg_idx] = NULL;
        arg_idx++;
    }
    char *next = cmd_str;
    int32_t argc = 0;
    /* Outer loop processes the entire command line */
    while (*next)
    {
        /* Skip spaces between command words or arguments */
        while (*next == token)
        {
            next++;
        }
        /* Handle case where the last argument is followed by a space, e.g., "ls
         * dir2 " */
        if (*next == 0)
        {
            break;
        }
        argv[argc] = next;

        /* Inner loop processes each command word and argument */
        while (*next &&
               *next != token)
        { // Find the delimiter before the string ends
            next++;
        }

        /* If not at the end of the string (token character), replace token with
         * null terminator */
        if (*next)
        {
            *next++ = 0; // End the word and move 'next' to the next character
        }

        /* Avoid out-of-bounds access to argv array if too many arguments */
        if (argc > MAX_ARG_NR)
        {
            return -1;
        }
        argc++;
    }
    return argc;
}

// using for the exec
char *argv[MAX_ARG_NR];
int32_t argc = -1;

void ccshell(void)
{
    cwd_cache[0] = '/';
    cwd_cache[1] = 0;
    while (1) { 
        print_prompt(); 
        k_memset(final_path, 0, MAX_PATH_LEN); 
        k_memset(cmd_line, 0, MAX_PATH_LEN); 
        readline(cmd_line, MAX_PATH_LEN); 
        if (cmd_line[0] == 0) {    // 若只键入了一个回车 
            continue; 
        } 
        argc = -1; 
        argc = cmd_parse(cmd_line, argv, ' '); 
        if (argc == -1) { 
            printf("num of arguments exceed %d\n", MAX_ARG_NR); 
            continue; 
        } 
        if (!k_strcmp("ls", argv[0])) { 
            buildin_ls(argc, argv); 
        } else if (!k_strcmp("cd", argv[0])) { 
            if (buildin_cd(argc, argv)) { 
                k_memset(cwd_cache, 0, MAX_PATH_LEN); 
                k_strcpy(cwd_cache, final_path); 
            } 
        } else if (!k_strcmp("pwd", argv[0])) { 
            buildin_pwd(argc, argv); 
        } else if (!k_strcmp("ps", argv[0])) { 
            buildin_ps(argc, argv); 
        } else if (!k_strcmp("clear", argv[0])) { 
            buildin_clear(argc, argv); 
        } else if (!k_strcmp("mkdir", argv[0])) { 
            buildin_mkdir(argc, argv); 
        } else if (!k_strcmp("rmdir", argv[0])) { 
            buildin_rmdir(argc, argv); 
        } else if (!k_strcmp("rm", argv[0])) { 
            buildin_rm(argc, argv); 
        } else { 
            printf("external command\n"); 
        } 
    }
    
    user_panic("Man!: you should not be here!!!"); // This should never be reached
}
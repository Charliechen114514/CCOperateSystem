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
    user_assert(buf != NULL && count > 0);
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
    user_assert(cmd_str != NULL);
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

/* Executes the command */
static void cmd_execute(uint32_t argc, char **argv) {
    if (!k_strcmp("ls", argv[0])) {
        buildin_ls(argc, argv);
    } else if (!k_strcmp("cd", argv[0])) {
        if (buildin_cd(argc, argv) != NULL) {
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
    } else if (!k_strcmp("help", argv[0])) {
        buildin_help(argc, argv);
    } else { // If it's an external command, it needs to be loaded from disk
        int32_t pid = fork();
        if (pid) { // Parent process
            int32_t status;
            int32_t child_pid = wait(&status); // If the child hasn't executed
                                               // exit, the shell is blocked
            if (child_pid ==
                -1) { // This shouldn't be reached under normal conditions
                user_panic("ccos_baseshell: no child\n");
            }
            printf("child_pid %d, its status: %d\n", child_pid, status);
        } else { // Child process
            make_clear_abs_path(argv[0], final_path);
            argv[0] = final_path;

            /* Check if the file exists */
            Stat file_stat;
            k_memset(&file_stat, 0, sizeof(Stat));
            if (stat(argv[0], &file_stat) == -1) {
                printf("ccos_baseshell: cannot access %s: No such file or "
                       "directory\n",
                       argv[0]);
                exit(-1);
            } else {
                execv(argv[0], argv); // Execute the command
            }
        }
    }
}



// using for the exec
char *argv[MAX_ARG_NR];
int32_t argc = -1;

void ccshell(void)
{
    cwd_cache[0] = '/';
    while (1) {
        print_prompt();                      // Print the shell prompt
        k_memset(final_path, 0, MAX_PATH_LEN); // Clear the final path buffer
        k_memset(cmd_line, 0, MAX_PATH_LEN);   // Clear the command line buffer
        readline(cmd_line, MAX_PATH_LEN); // Read a line of input from the user
        if (cmd_line[0] == 0) { // If only enter was pressed (empty command)
            continue;           // Skip to the next iteration
        }

        /* Handle pipe operations */
        char *pipe_symbol =
            k_strchr(cmd_line, '|'); // Check for a pipe symbol '|'
        if (pipe_symbol) {
            /* Support multiple pipes, such as cmd1|cmd2|..|cmdn,
             * where the standard output of cmd1 and the standard input of cmdn
             * need to be handled separately */

            /* 1. Create a pipe */
            int32_t fd[2] = {-1}; // fd[0] for input, fd[1] for output
            pipe(fd);             // Create a pipe

            /* Redirect standard output to fd[1], so subsequent output goes to
             * the kernel circular buffer */
            fd_redirect(1, fd[1]);

            /* 2. First command */
            char *each_cmd = cmd_line;
            pipe_symbol = k_strchr(each_cmd, '|'); // Find the first pipe symbol
            *pipe_symbol =
                0; // Terminate the first command string at the pipe symbol

            /* Execute the first command, its output will go to the circular
             * buffer */
            argc = -1;
            argc = cmd_parse(each_cmd, argv,
                             ' ');   // Parse the command into arguments
            cmd_execute(argc, argv); // Execute the first command

            /* Skip over the '|' symbol and handle the next command */
            each_cmd = pipe_symbol + 1;

            /* Redirect standard input to fd[0], pointing to the kernel circular
             * buffer */
            fd_redirect(0, fd[0]);

            /* 3. Intermediate commands, both input and output go to the
             * circular buffer */
            while ((pipe_symbol = k_strchr(each_cmd, '|'))) {
                *pipe_symbol =
                    0; // Terminate the command string at the pipe symbol
                argc = -1;
                argc = cmd_parse(each_cmd, argv, ' '); // Parse the command
                cmd_execute(argc, argv);    // Execute the intermediate command
                each_cmd = pipe_symbol + 1; // Move to the next command
            }

            /* 4. Handle the last command in the pipe */
            /* Restore standard output to the screen */
            fd_redirect(1, 1);

            /* Execute the last command */
            argc = -1;
            argc = cmd_parse(each_cmd, argv, ' '); // Parse the last command
            cmd_execute(argc, argv);               // Execute the last command

            /* 5. Restore standard input to the keyboard */
            fd_redirect(0, 0);

            /* 6. Close the pipe */
            close(fd[0]);
            close(fd[1]);
        } else { // Command without pipe operation
            argc = -1;
            argc = cmd_parse(cmd_line, argv, ' '); // Parse the command
            if (argc == -1) {
                printf("num of arguments exceed %d\n",
                       MAX_ARG_NR); // Error if too many arguments
                continue;
            }
            cmd_execute(argc, argv); // Execute the command
        }
    }
    user_panic("Man!: you should not be here!!!"); // This should never be reached
}
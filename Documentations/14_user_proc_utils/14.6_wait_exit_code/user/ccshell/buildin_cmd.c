#include "include/user/ccshell/buildin_cmd.h"
#include "include/filesystem/dir.h"
#include "include/filesystem/filesystem.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/user/ccshell/ccshell.h"
#include "include/syscall/syscall.h"
#include "include/user/library/user_assertion.h"
#include "include/user/stdio/stdio.h"

/* Converts the path old_abs_path by resolving ".." and "." to the actual path
 * and stores it in new_abs_path */
static void wash_path(char *old_abs_path, char *new_abs_path) {
    user_assert(old_abs_path[0] == '/'); // Ensure the path starts with "/"
    char name[MAX_FILE_NAME_LEN] = {0};
    char *sub_path = old_abs_path;
    sub_path = path_parse(sub_path, name);
    if (name[0] ==
        0) { // If the input is just "/", directly set new_abs_path to "/"
        new_abs_path[0] = '/';
        new_abs_path[1] = 0;
        return;
    }
    new_abs_path[0] = 0; // Clear the new_abs_path to avoid residual data
    k_strcat(new_abs_path, "/");
    while (name[0]) {
        /* If it's the parent directory ".." */
        if (!k_strcmp("..", name)) {
            char *slash_ptr = k_strrchr(new_abs_path, '/');
            /* If we're not at the top of the directory, remove the last path
             * component */
            if (slash_ptr !=
                new_abs_path) { // For example, "/a/b" becomes "/a" after ".."
                *slash_ptr = 0;
            } else { // If we're at the top level, reset the path to "/"
                *(slash_ptr + 1) = 0;
            }
        } else if (k_strcmp(".", name)) { // If the path is not ".", append it to
                                        // new_abs_path
            if (k_strcmp(new_abs_path,
                       "/")) { // Avoid double slashes at the beginning
                k_strcat(new_abs_path, "/");
            }
            k_strcat(new_abs_path, name);
        } // Do nothing if name is ".", as it's the current directory

        /* Continue to the next path segment */
        k_memset(name, 0, MAX_FILE_NAME_LEN);
        if (sub_path) {
            sub_path = path_parse(sub_path, name);
        }
    }
}

/* Converts the path to a clear absolute path without ".." and "." and stores it
 * in final_path */
void make_clear_abs_path(char *path, char *final_path) {
    char abs_path[MAX_PATH_LEN] = {0};
    /* Check if the input is an absolute path */
    if (path[0] != '/') { // If it's not an absolute path, prepend the current
                          // working directory
        k_memset(abs_path, 0, MAX_PATH_LEN);
        if (getcwd(abs_path, MAX_PATH_LEN) != NULL) {
            if (!((abs_path[0] == '/') &&
                  (abs_path[1] == 0))) { // Avoid a non-root directory path
                k_strcat(abs_path, "/");
            }
        }
    }
    k_strcat(abs_path, path);
    wash_path(abs_path,
              final_path); // Resolve the path and store it in final_path
}

/* Built-in pwd command function */
void buildin_pwd(uint32_t argc, char **argv) {
    (void)argc;
    (void)argv;
    if (argc != 1) {
        printf("pwd: no argument support!\n"); // Error if there are arguments
        return;
    } else {
        if (NULL != getcwd(final_path, MAX_PATH_LEN)) {
            printf("%s\n", final_path); // Print the current working directory
        } else {
            printf(
                "pwd: get current work directory failed.\n"); // Error if unable
                                                              // to get the
                                                              // current
                                                              // directory
        }
    }
}

/* Built-in cd command function */
char *buildin_cd(uint32_t argc, char **argv) {
    if (argc > 2) {
        printf("cd: only support 1 argument!\n"); // Error if more than one
                                                  // argument is given
        return NULL;
    }

    /* If no argument is given, set path to root directory */
    if (argc == 1) {
        final_path[0] = '/';
        final_path[1] = 0;
    } else {
        make_clear_abs_path(
            argv[1], final_path); // Resolve the given path to an absolute path
    }

    if (chdir(final_path) == -1) { // Change the directory
        printf("cd: no such directory %s\n",
               final_path); // Error if the directory does not exist
        return NULL;
    }
    return final_path;
}

/* Built-in ls command function */
void buildin_ls(uint32_t argc, char **argv) {

    char *pathname = NULL;
    Stat file_stat;
    k_memset(&file_stat, 0, sizeof(Stat));
    bool long_info = false;
    uint32_t arg_path_nr = 0;
    uint32_t arg_idx = 1; // Skip argv[0], which is the "ls" command
    while (arg_idx < argc) {
        if (argv[arg_idx][0] == '-') { // If it's an option, starting with "-"
            if (!k_strcmp("-l", argv[arg_idx])) { // "-l" option for long format
                long_info = true;
            } else if (!k_strcmp("-h", argv[arg_idx])) { // "-h" option for help
                printf("usage: -l list all infomation about the file.\n-h for "
                       "help\nlist all files in the current directory if no "
                       "option\n");
                return;
            } else { // Invalid option
                printf("ls: invalid option %s\nTry `ls -h' for more "
                       "information.\n",
                       argv[arg_idx]);
                return;
            }
        } else { // Path argument for ls
            if (arg_path_nr == 0) {
                pathname = argv[arg_idx];
                arg_path_nr = 1;
            } else {
                printf("ls: only support one path\n"); // Error if more than one
                                                       // path is provided
                return;
            }
        }
        arg_idx++;
    }

    if (pathname == NULL) { // If no path is provided, use the current directory
        if (NULL != getcwd(final_path, MAX_PATH_LEN)) {
            pathname = final_path;
        } else {
            printf("ls: getcwd for default path failed\n"); // Error if unable
                                                            // to get the
                                                            // current directory
            return;
        }
    } else {
        make_clear_abs_path(pathname,
                            final_path); // Resolve the path to an absolute path
        pathname = final_path;
    }

    if (stat(pathname, &file_stat) == -1) { // Check if the path exists
        printf("%s", pathname);
        printf("ls: cannot access %s: No such file or directory\n", pathname);
        return;
    }
    if (file_stat.st_filetype ==
        FT_DIRECTORY) { // If it's a directory, list its contents
        Dir *dir = opendir(pathname);
        DirEntry *dir_e = NULL;
        char sub_pathname[MAX_PATH_LEN] = {0};
        uint32_t pathname_len = k_strlen(pathname);
        uint32_t last_char_idx = pathname_len - 1;
        k_memcpy(sub_pathname, pathname, pathname_len);
        if (sub_pathname[last_char_idx] != '/') {
            sub_pathname[pathname_len] = '/';
            pathname_len++;
        }
        rewinddir(dir);
        if (long_info) {
            char ftype;
            printf("total: %d\n", file_stat.st_size);
            while ((dir_e = readdir(dir))) {
                ftype = 'd';
                if (dir_e->f_type == FT_REGULAR) {
                    ftype = '-';
                }
                sub_pathname[pathname_len] = 0;
                k_strcat(sub_pathname, dir_e->filename);
                k_memset(&file_stat, 0, sizeof(Stat));
                if (stat(sub_pathname, &file_stat) == -1) {
                    printf("ls: cannot access %s: No such file or directory\n",
                           dir_e->filename);
                    return;
                }
                printf("%c  %d  %d  %s\n", ftype, dir_e->i_no,
                       file_stat.st_size, dir_e->filename);
            }
        } else {
            while ((dir_e = readdir(dir))) {
                printf("%s ",
                       dir_e->filename); // Print file names in a simple format
            }
            printf("\n");
        }
        closedir(dir);
    } else {
        if (long_info) {
            printf("-  %d  %d  %s\n", file_stat.st_ino, file_stat.st_size,
                   pathname);
        } else {
            printf("%s\n", pathname); // Print the file name
        }
    }
}

/* Built-in ps command function */
void buildin_ps(uint32_t argc, char **argv) {
    (void)argc;
    (void)argv;
    if (argc != 1) {
        printf("ps: no argument support!\n"); // Error if there are arguments
        return;
    }
    ps(); // Call the ps function to display process status
}

/* Built-in clear command function */
void buildin_clear(uint32_t argc, char **argv) {
    (void)argc;
    (void)argv;
    if (argc != 1) {
        printf("clear: no argument support!\n"); // Error if there are arguments
        return;
    }
    clear(); // Call the clear function to clear the terminal screen
}

/* Built-in mkdir command function */
int32_t buildin_mkdir(uint32_t argc, char **argv) {
    int32_t ret = -1;
    if (argc != 2) {
        printf("mkdir: only support 1 argument!\n"); // Error if there are not
                                                     // exactly 1 argument
    } else {
        make_clear_abs_path(argv[1],
                            final_path); // Resolve the path to an absolute path
        /* If not creating the root directory */
        if (k_strcmp("/", final_path)) {
            if (mkdir(final_path) == 0) { // Create the directory
                ret = 0;
            } else {
                printf("mkdir: create directory %s failed.\n",
                       argv[1]); // Error if directory creation fails
            }
        }
    }
    return ret;
}

/* Built-in rmdir command function */
int32_t buildin_rmdir(uint32_t argc, char **argv) {
    int32_t ret = -1;
    if (argc != 2) {
        printf("rmdir: only support 1 argument!\n"); // Error if there are not
                                                     // exactly 1 argument
    } else {
        make_clear_abs_path(argv[1],
                            final_path); // Resolve the path to an absolute path
        /* If not removing the root directory */
        if (k_strcmp("/", final_path)) {
            if (rmdir(final_path) == 0) { // Remove the directory
                ret = 0;
            } else {
                printf("rmdir: remove %s failed.\n",
                       argv[1]); // Error if directory removal fails
            }
        }
    }
    return ret;
}

/* Built-in rm command function */
int32_t buildin_rm(uint32_t argc, char **argv) {
    (void)argc;
    (void)argv;
    int32_t ret = -1;
    if (argc != 2) {
        printf("rm: only support 1 argument!\n"); // Error if there are not
                                                  // exactly 1 argument
    } else {
        make_clear_abs_path(argv[1],
                            final_path); // Resolve the path to an absolute path
        /* If not deleting the root directory */
        if (k_strcmp("/", final_path)) {
            if (unlink(final_path) == 0) { // Delete the file
                ret = 0;
            } else {
                printf("rm: delete %s failed.\n",
                       argv[1]); // Error if file deletion fails
            }
        }
    }
    return ret;
}

// /* Built-in help command function to display the list of built-in commands */
// void buildin_help(uint32_t argc, char **argv) {
//     (void)argc;
//     (void)argv;
//     help(); // Call the help function to display available commands and their
//             // usage
// }

#ifndef SHELL_H
#define SHELL_H

/* Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>

/* Readline headers */
#include <readline/readline.h>
#include <readline/history.h>

/* Config */
#define PROMPT "myshell> "
#define MAX_JOBS 128
#define MAX_CMD_LEN 1024
#define MAX_ARGV 128
#define MAX_COMMANDS 2048

/* Job struct */
typedef struct {
    pid_t pid;
    char cmd[MAX_CMD_LEN];
    int active;     /* 1 = active, 0 = free */
} Job;

/* Function declarations */
char *read_cmdline(void);               /* wrapper for readline */
char **tokenize(char *line);
int handle_builtin(char **argv);
int execute_cmd(char **argv, int background);
void add_job(pid_t pid, const char *cmd, int *jobno_out);
void show_jobs(void);
void cleanup_jobs(void);

/* Completion functions (internal) */
char **myshell_completion(const char *text, int start, int end);
char *command_generator(const char *text, int state);
void init_command_list(void);
void free_command_list(void);

#endif

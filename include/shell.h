#ifndef SHELL_H
#define SHELL_H

/* Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>

/* Readline headers */
#include <readline/readline.h>
#include <readline/history.h>

/* Configuration constants */
#define PROMPT "myshell> "
#define MAX_ARGS 100
#define MAX_ARGV 128
#define MAX_CMD_LEN 1024
#define MAX_JOBS 128
#define MAX_COMMANDS 2048

/* Job struct (used by job-control functions) */
typedef struct {
    pid_t pid;
    char cmd[MAX_CMD_LEN];
    int active;     /* 1 = active, 0 = free */
} Job;

/* Parser / main functions */
char **tokenize(char *line);
int handle_builtin(char **args);

/* Execution functions */
void execute(char **args);                /* regular execution (no redir/pipe) */
void execute_redirect(char **args);       /* handles < and > redirection */
void execute_pipe(char **args);           /* handles single pipe 'cmd1 | cmd2' */

/* Job management functions (used elsewhere) */
void add_job(pid_t pid, const char *cmd, int *jobno_out);
void show_jobs(void);
void cleanup_jobs(void);

/* Readline / completion helpers */
void init_command_list(void);
void free_command_list(void);
char **myshell_completion(const char *text, int start, int end);
char *command_generator(const char *text, int state);

#endif

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_COMMANDS 128
#define MAX_ARGS 128
#define MAX_LINE 2048
#define MAX_JOBS 128
#define MAX_CMDLEN 1024

typedef struct Command {
    char *args[MAX_ARGS];   // Null-terminated argv for execvp
    char *input_file;       // If non-NULL => input redirection file
    char *output_file;      // If non-NULL => output redirection file
} Command;

typedef struct Job {
    pid_t pid;
    char cmdline[MAX_CMDLEN];
} Job;

/* Job list is maintained in shell.c */
extern Job jobs[MAX_JOBS];
extern int job_count;

/* job management */
void add_job(pid_t pid, const char *cmdline);
void remove_job(pid_t pid);
void check_background_jobs(); /* reap finished background children */

/* parsing & execution */
int parse_pipeline(char *segment, Command commands[], int *num_commands);
pid_t execute_pipeline(Command commands[], int num_commands, int background);

/* helper */
void trim_spaces(char *s);

#endif /* SHELL_H */

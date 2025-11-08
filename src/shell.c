#include "shell.h"

/* Job list */
Job jobs[MAX_JOBS];
int job_count = 0;

/* Add a background job (pid, command line) */
void add_job(pid_t pid, const char *cmdline) {
    if (job_count >= MAX_JOBS) return;
    jobs[job_count].pid = pid;
    strncpy(jobs[job_count].cmdline, cmdline, MAX_CMDLEN - 1);
    jobs[job_count].cmdline[MAX_CMDLEN - 1] = '\0';
    job_count++;
}

/* Remove job by pid */
void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; ++i) {
        if (jobs[i].pid == pid) {
            for (int j = i; j < job_count - 1; ++j) jobs[j] = jobs[j + 1];
            job_count--;
            return;
        }
    }
}

/* Reap any finished background children (non-blocking) */
void check_background_jobs() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_job(pid);
        /* Optional: print message about completion
           if (WIFEXITED(status)) {
               printf("[bg] %d done (exit %d)\n", pid, WEXITSTATUS(status));
           } else if (WIFSIGNALED(status)) {
               printf("[bg] %d terminated by signal %d\n", pid, WTERMSIG(status));
           }
        */
    }
}

/* Trim leading/trailing spaces, tabs, and trailing newline */
void trim_spaces(char *s) {
    if (s == NULL) return;
    char *start = s;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n')) start++;
    char *end = s + strlen(s) - 1;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        *end = '\0';
        end--;
    }
    if (start != s) memmove(s, start, strlen(start) + 1);
}

/*
 parse_pipeline: parse a single segment (no ';') into commands separated by '|'.
 Fills commands[] and sets num_commands. Returns 0 on success, -1 on parse error.
*/
int parse_pipeline(char *segment, Command commands[], int *num_commands) {
    if (!segment || !commands || !num_commands) return -1;

    /* initialize */
    for (int i = 0; i < MAX_COMMANDS; ++i) {
        commands[i].input_file = NULL;
        commands[i].output_file = NULL;
        for (int j = 0; j < MAX_ARGS; ++j) commands[i].args[j] = NULL;
    }

    int cmd_index = 0, arg_index = 0;
    char *token = strtok(segment, " \t\n");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t\n");
            if (!token) return -1;
            commands[cmd_index].input_file = strdup(token);
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\n");
            if (!token) return -1;
            commands[cmd_index].output_file = strdup(token);
        } else if (strcmp(token, "|") == 0) {
            /* end current command */
            commands[cmd_index].args[arg_index] = NULL;
            cmd_index++;
            if (cmd_index >= MAX_COMMANDS) return -1;
            arg_index = 0;
            /* ensure next command is zeroed (already done in init) */
        } else {
            if (arg_index < MAX_ARGS - 1) {
                commands[cmd_index].args[arg_index++] = strdup(token);
            } else {
                return -1;
            }
        }
        token = strtok(NULL, " \t\n");
    }
    commands[cmd_index].args[arg_index] = NULL;
    *num_commands = cmd_index + 1;
    return 0;
}

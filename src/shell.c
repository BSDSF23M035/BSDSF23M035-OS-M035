#define _GNU_SOURCE
#include "shell.h"

/* Job table */
static Job jobs[MAX_JOBS] = {0};

/* Command list built from PATH for completion */
static char *command_list[MAX_COMMANDS];
static int command_count = 0;

/* Read a line using readline */
char *read_cmdline(void) {
    char *input = readline(PROMPT); /* caller must free */
    return input;
}

/* Tokenize using whitespace. Caller must free returned pointers. */
char **tokenize(char *line) {
    if (line == NULL) return NULL;
    int bufsize = MAX_ARGV;
    char **tokens = malloc(sizeof(char*) * bufsize);
    if (!tokens) { perror("malloc"); exit(EXIT_FAILURE); }

    int idx = 0;
    char *tok = strtok(line, " \t");
    while (tok != NULL && idx < bufsize - 1) {
        tokens[idx++] = strdup(tok);
        tok = strtok(NULL, " \t");
    }
    tokens[idx] = NULL;
    return tokens;
}

/* Builtins: exit, cd, help, jobs */
int handle_builtin(char **argv) {
    if (argv == NULL || argv[0] == NULL) return 1;
    if (strcmp(argv[0], "exit") == 0) {
        rl_clear_history();
        exit(0);
    } else if (strcmp(argv[0], "cd") == 0) {
        if (argv[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(argv[1]) != 0) perror("cd");
        }
        return 1;
    } else if (strcmp(argv[0], "help") == 0) {
        printf("Built-ins:\n  cd <dir>\n  exit\n  help\n  jobs\n");
        printf("Use '&' suffix to run in background.\n");
        return 1;
    } else if (strcmp(argv[0], "jobs") == 0) {
        show_jobs();
        return 1;
    }
    return 0;
}

/* Job functions */
void add_job(pid_t pid, const char *cmd, int *jobno_out) {
    for (int i = 0; i < MAX_JOBS; ++i) {
        if (!jobs[i].active) {
            jobs[i].pid = pid;
            jobs[i].active = 1;
            strncpy(jobs[i].cmd, cmd, sizeof(jobs[i].cmd)-1);
            jobs[i].cmd[sizeof(jobs[i].cmd)-1] = '\0';
            if (jobno_out) *jobno_out = i + 1;
            return;
        }
    }
    if (jobno_out) *jobno_out = -1;
    fprintf(stderr, "jobs: job table full\n");
}

void show_jobs(void) {
    printf("Job list:\n");
    for (int i = 0; i < MAX_JOBS; ++i) {
        if (jobs[i].active) {
            printf("[%d] PID: %d\t%s\n", i+1, jobs[i].pid, jobs[i].cmd);
        }
    }
}

/* Reap finished background children (non-blocking) */
void cleanup_jobs(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; ++i) {
            if (jobs[i].active && jobs[i].pid == pid) {
                jobs[i].active = 0;
                if (WIFEXITED(status)) {
                    printf("\n[Done] PID %d Exit %d CMD: %s\n", pid, WEXITSTATUS(status), jobs[i].cmd);
                } else if (WIFSIGNALED(status)) {
                    printf("\n[Terminated] PID %d Signal %d CMD: %s\n", pid, WTERMSIG(status), jobs[i].cmd);
                } else {
                    printf("\n[Finished] PID %d CMD: %s\n", pid, jobs[i].cmd);
                }
                fflush(stdout);
                break;
            }
        }
    }
}

/* ---------------- Command list (PATH) for completion ---------------- */

/* Helper: add unique command to list */
static void add_command_name(const char *name) {
    if (command_count >= MAX_COMMANDS) return;
    for (int i = 0; i < command_count; ++i) {
        if (strcmp(command_list[i], name) == 0) return; /* already present */
    }
    command_list[command_count++] = strdup(name);
}

/* Scan PATH and add all executable names (unique) */
void init_command_list(void) {
    const char *path = getenv("PATH");
    if (!path) return;
    char *pathdup = strdup(path);
    char *dir = strtok(pathdup, ":");
    while (dir) {
        DIR *d = opendir(dir);
        if (d) {
            struct dirent *entry;
            while ((entry = readdir(d)) != NULL) {
                if (entry->d_name[0] == '.') continue;
                /* Create full path */
                char full[MAX_CMD_LEN];
                snprintf(full, sizeof(full), "%s/%s", dir, entry->d_name);
                if (access(full, X_OK) == 0) {
                    add_command_name(entry->d_name);
                }
            }
            closedir(d);
        }
        dir = strtok(NULL, ":");
    }
    free(pathdup);

    /* also add builtins */
    add_command_name("cd");
    add_command_name("exit");
    add_command_name("help");
    add_command_name("jobs");
}

/* Free command list */
void free_command_list(void) {
    for (int i = 0; i < command_count; ++i) free(command_list[i]);
    command_count = 0;
}

/* ---------------- Readline completion callbacks ---------------- */

/* Generator for command names (called by rl_completion_matches) */
char *command_generator(const char *text, int state) {
    static int idx;
    static size_t len;
    if (state == 0) {
        idx = 0;
        len = strlen(text);
    }
    while (idx < command_count) {
        const char *name = command_list[idx++];
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    return NULL;
}

/* Main completion function:
 * - if at start of line => complete commands (from PATH + builtins)
 * - else => use filename completion
 */
char **myshell_completion(const char *text, int start, int end) {
    /* If completion is at first word, complete commands */
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    } else {
        /* Default filename completion provided by readline */
        return rl_completion_matches(text, rl_filename_completion_function);
    }
}

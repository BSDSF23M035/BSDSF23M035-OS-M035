#include "shell.h"

/* builtin handler: returns 1 if handled, 0 otherwise */
static int handle_builtin(Command *cmd, const char *full_cmdline) {
    if (cmd->args[0] == NULL) return 0;
    if (strcmp(cmd->args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(cmd->args[0], "cd") == 0) {
        char *path = cmd->args[1];
        if (!path) path = getenv("HOME");
        if (chdir(path) != 0) perror("cd");
        return 1;
    } else if (strcmp(cmd->args[0], "jobs") == 0) {
        for (int i = 0; i < job_count; ++i) {
            printf("[%d] %d  %s\n", i + 1, jobs[i].pid, jobs[i].cmdline);
        }
        return 1;
    }
    return 0;
}

int main() {
    char line[MAX_LINE];

    while (1) {
        /* Reap any finished background jobs before prompt */
        check_background_jobs();

        /* Prompt */
        printf("myshell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break; /* EOF */
        }

        /* Split by semicolons to support chaining */
        char *saveptr1;
        char *segment = strtok_r(line, ";", &saveptr1);
        while (segment != NULL) {
            trim_spaces(segment);
            if (strlen(segment) == 0) {
                segment = strtok_r(NULL, ";", &saveptr1);
                continue;
            }

            int background = 0;
            int seglen = strlen(segment);
            /* check for background '&' at end */
            if (seglen > 0 && segment[seglen - 1] == '&') {
                background = 1;
                segment[seglen - 1] = '\0';
                trim_spaces(segment);
            }

            /* save a cleaned copy for jobs listing */
            char saved_cmd[MAX_CMDLEN];
            strncpy(saved_cmd, segment, MAX_CMDLEN - 1);
            saved_cmd[MAX_CMDLEN - 1] = '\0';

            /* parse pipeline (this will split on | and set input/output redirections) */
            Command commands[MAX_COMMANDS];
            int num_commands = 0;
            /* strtok in parse_pipeline will modify the segment in-place */
            if (parse_pipeline(segment, commands, &num_commands) != 0) {
                fprintf(stderr, "myshell: parse error\n");
                segment = strtok_r(NULL, ";", &saveptr1);
                continue;
            }

            /* If single command and builtin -> handle in shell */
            if (num_commands == 1 && commands[0].args[0] != NULL && handle_builtin(&commands[0], saved_cmd)) {
                /* builtin executed */
            } else {
                pid_t jobpid = execute_pipeline(commands, num_commands, background);
                if (background && jobpid > 0) {
                    add_job(jobpid, saved_cmd);
                    printf("[bg] %d\n", jobpid);
                }
            }

            /* free allocated strings from parse_pipeline */
            for (int c = 0; c < num_commands; ++c) {
                for (int a = 0; a < MAX_ARGS && commands[c].args[a] != NULL; ++a) {
                    free(commands[c].args[a]);
                }
                if (commands[c].input_file) {
                    free(commands[c].input_file);
                }
                if (commands[c].output_file) {
                    free(commands[c].output_file);
                }
            }

            segment = strtok_r(NULL, ";", &saveptr1);
        }
    }

    return 0;
}

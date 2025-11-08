#include "../include/shell.h"

// Global variables for IF blocks
int if_mode = 0;
char if_cmd[MAX_LINE_LEN];
char then_block[MAX_BLOCK_LINES][MAX_LINE_LEN];
char else_block[MAX_BLOCK_LINES][MAX_LINE_LEN];
int then_count = 0, else_count = 0;
int in_then = 0, in_else = 0;

void reset_if_state() {
    if_mode = 0;
    if_cmd[0] = '\0';
    then_count = else_count = 0;
    in_then = in_else = 0;
}

int main() {
    char line[MAX_CMD_LEN];

    printf("myshell> ");
    while (fgets(line, sizeof(line), stdin)) {
        trim_spaces(line);
        if (strlen(line) == 0) {
            printf("myshell> ");
            continue;
        }

        // --- IF STRUCTURE DETECTION ---
        if (strncmp(line, "if ", 3) == 0 && !if_mode) {
            if_mode = 1;
            strcpy(if_cmd, line + 3);
            continue;
        }

        if (if_mode) {
            if (strcmp(line, "then") == 0) {
                in_then = 1;
                continue;
            } else if (strcmp(line, "else") == 0) {
                in_then = 0;
                in_else = 1;
                continue;
            } else if (strcmp(line, "fi") == 0) {
                execute_if_block();
                reset_if_state();
                printf("myshell> ");
                continue;
            }

            // Store inside the current block
            if (in_then) {
                strcpy(then_block[then_count++], line);
            } else if (in_else) {
                strcpy(else_block[else_count++], line);
            }
            continue;
        }

        // --- NORMAL COMMAND EXECUTION ---
        char *args[MAX_TOKENS];
        int i = 0;
        char *token = strtok(line, " ");
        while (token) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        execute_command(args);
        printf("myshell> ");
    }

    return 0;
}

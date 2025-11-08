#include "../include/shell.h"

// Normal command execution
void execute_command(char **args) {
    if (args[0] == NULL) return;

    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("exec");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }
}

// IF-THEN-ELSE-FI execution
void execute_if_block() {
    if (strlen(if_cmd) == 0) return;

    int status;
    pid_t pid = fork();
    if (pid == 0) {
        execlp("sh", "sh", "-c", if_cmd, (char *)NULL);
        perror("exec");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        int exit_status = WEXITSTATUS(status);

        char (*block)[MAX_LINE_LEN];
        int block_count;
        if (exit_status == 0) {
            block = then_block;
            block_count = then_count;
        } else {
            block = else_block;
            block_count = else_count;
        }

        for (int i = 0; i < block_count; i++) {
            pid_t child = fork();
            if (child == 0) {
                execlp("sh", "sh", "-c", block[i], (char *)NULL);
                perror("exec");
                exit(EXIT_FAILURE);
            } else if (child > 0) {
                waitpid(child, &status, 0);
            } else {
                perror("fork");
            }
        }
    } else {
        perror("fork");
    }
}

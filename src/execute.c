#include "shell.h"

void execute_commands(Command commands[], int num_commands) {
    int pipefd[2*(num_commands-1)];
    pid_t pid;
    int i;

    // Create all required pipes
    for (i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefd + i*2) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < num_commands; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process
            // Input redirection
            if (commands[i].input_file) {
                int fd = open(commands[i].input_file, O_RDONLY);
                if (fd < 0) { perror("open input"); exit(EXIT_FAILURE); }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // Output redirection
            if (commands[i].output_file) {
                int fd = open(commands[i].output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("open output"); exit(EXIT_FAILURE); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // Pipes
            if (i != 0) { // Not first command
                dup2(pipefd[(i-1)*2], STDIN_FILENO);
            }
            if (i != num_commands - 1) { // Not last command
                dup2(pipefd[i*2 + 1], STDOUT_FILENO);
            }

            // Close all pipe fds in child
            for (int j = 0; j < 2*(num_commands-1); j++) close(pipefd[j]);

            // Execute command
            execvp(commands[i].args[0], commands[i].args);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    }

    // Parent closes all pipes
    for (i = 0; i < 2*(num_commands-1); i++) close(pipefd[i]);

    // Parent waits for all children
    for (i = 0; i < num_commands; i++) wait(NULL);
}

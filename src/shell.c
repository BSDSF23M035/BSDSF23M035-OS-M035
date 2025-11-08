#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_TOKENS 100
#define MAX_COMMANDS 10

typedef struct {
    char *args[MAX_TOKENS];
    char *input_file;
    char *output_file;
} Command;

// Function to parse the command line into commands, handling <, >, and |
int parse_line(char *line, Command commands[], int *num_commands) {
    char *token;
    int cmd_index = 0, arg_index = 0;

    commands[cmd_index].input_file = NULL;
    commands[cmd_index].output_file = NULL;

    token = strtok(line, " \t\n");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t\n");
            if (token) commands[cmd_index].input_file = strdup(token);
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\n");
            if (token) commands[cmd_index].output_file = strdup(token);
        } else if (strcmp(token, "|") == 0) {
            commands[cmd_index].args[arg_index] = NULL;
            cmd_index++;
            arg_index = 0;
            commands[cmd_index].input_file = NULL;
            commands[cmd_index].output_file = NULL;
        } else {
            commands[cmd_index].args[arg_index++] = strdup(token);
        }
        token = strtok(NULL, " \t\n");
    }

    commands[cmd_index].args[arg_index] = NULL;
    *num_commands = cmd_index + 1;
    return 0;
}

// Function to execute parsed commands with pipes and redirection
void execute_commands(Command commands[], int num_commands) {
    int i;
    int in_fd = 0; // initial input is stdin
    int pipefd[2];
    pid_t pid;

    for (i = 0; i < num_commands; i++) {
        if (i < num_commands - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pid = fork();
        if (pid == 0) {
            // --- Child process ---

            // Input redirection
            if (commands[i].input_file) {
                int fd_in = open(commands[i].input_file, O_RDONLY);
                if (fd_in < 0) {
                    perror("open input_file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            } else if (in_fd != 0) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // Output redirection
            if (commands[i].output_file) {
                int fd_out = open(commands[i].output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("open output_file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            } else if (i < num_commands - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            if (i < num_commands - 1) close(pipefd[0]);

            execvp(commands[i].args[0], commands[i].args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // --- Parent process ---
        wait(NULL);
        if (in_fd != 0) close(in_fd);
        if (i < num_commands - 1) {
            close(pipefd[1]);
            in_fd = pipefd[0];
        }
    }
}

int main() {
    char line[1024];
    Command commands[MAX_COMMANDS];
    int num_commands;

    while (1) {
        printf("myshell> ");
        if (!fgets(line, sizeof(line), stdin)) break;

        // Exit command
        if (strncmp(line, "exit", 4) == 0) break;

        parse_line(line, commands, &num_commands);
        execute_commands(commands, num_commands);
    }

    return 0;
}

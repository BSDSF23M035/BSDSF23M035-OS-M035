#include "shell.h"

/* Execute a simple command (no pipe or redirection).
 * args: NULL-terminated argv list
 */
void execute(char **args) {
    if (args == NULL || args[0] == NULL) return;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* Child: restore default signals */
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        execvp(args[0], args);
        perror("exec");
        _exit(EXIT_FAILURE);
    } else {
        /* Parent: wait for child to finish */
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
        }
    }
}

/* Execute single command with possible input/output redirection.
 * This function expects args to contain redir tokens '<' or '>' (and filenames)
 * and will nullify the operator token so execvp gets correct argv.
 */
void execute_redirect(char **args) {
    if (args == NULL || args[0] == NULL) return;

    int in_redirect = 0, out_redirect = 0;
    char *input_file = NULL, *output_file = NULL;

    /* Find redirection operators and record filenames, nullify operator position */
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            in_redirect = 1;
            input_file = args[i + 1];
            args[i] = NULL;
            break; /* assume one input file only */
        } else if (strcmp(args[i], ">") == 0) {
            out_redirect = 1;
            output_file = args[i + 1];
            args[i] = NULL;
            break; /* assume one output file only */
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* Child: perform redirections before exec */
        if (in_redirect) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("open input file");
                _exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDIN_FILENO) < 0) {
                perror("dup2 input");
                close(fd);
                _exit(EXIT_FAILURE);
            }
            close(fd);
        }
        if (out_redirect) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open output file");
                _exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("dup2 output");
                close(fd);
                _exit(EXIT_FAILURE);
            }
            close(fd);
        }

        /* Execute the command */
        execvp(args[0], args);
        perror("exec");
        _exit(EXIT_FAILURE);
    } else {
        /* Parent waits for child */
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
        }
    }
}

/* Execute a single pipe: left_cmd | right_cmd
 * Assumes args contains '|' token between two commands. This function splits at the first '|'.
 */
void execute_pipe(char **args) {
    if (args == NULL) return;

    /* find pipe position */
    int i = 0;
    while (args[i] != NULL && strcmp(args[i], "|") != 0) i++;

    if (args[i] == NULL) {
        /* no pipe found, fallback to redirection detection or simple exec */
        execute_redirect(args);
        return;
    }

    /* split the argv arrays */
    args[i] = NULL; /* terminate left command */
    char **left_cmd = args;
    char **right_cmd = &args[i + 1];

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid1 == 0) {
        /* Left child: write end -> stdout */
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            perror("dup2 left");
            _exit(EXIT_FAILURE);
        }
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(left_cmd[0], left_cmd);
        perror("exec left");
        _exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        /* cleanup */
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid2 == 0) {
        /* Right child: read end -> stdin */
        if (dup2(pipefd[0], STDIN_FILENO) < 0) {
            perror("dup2 right");
            _exit(EXIT_FAILURE);
        }
        close(pipefd[1]);
        close(pipefd[0]);
        execvp(right_cmd[0], right_cmd);
        perror("exec right");
        _exit(EXIT_FAILURE);
    }

    /* Parent: close both ends and wait for children */
    close(pipefd[0]);
    close(pipefd[1]);

    int status;
    if (waitpid(pid1, &status, 0) == -1) perror("waitpid pid1");
    if (waitpid(pid2, &status, 0) == -1) perror("waitpid pid2");
}

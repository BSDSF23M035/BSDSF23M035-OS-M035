#include "shell.h"

/*
 execute_pipeline:
   - commands[] with num_commands entries (each with args, input/output files)
   - background: 1 => run in background (return last child's pid), 0 => wait for completion and return 0
 Returns: pid of last child if background started, else 0 on foreground completion (or 0 on error)
*/
pid_t execute_pipeline(Command commands[], int num_commands, int background) {
    if (num_commands <= 0) return 0;

    int num_pipes = (num_commands > 1) ? (num_commands - 1) : 0;
    int pipefd[2 * (num_pipes)];
    pid_t children[MAX_COMMANDS];

    /* create pipes */
    for (int i = 0; i < num_pipes; ++i) {
        if (pipe(pipefd + i*2) < 0) {
            perror("pipe");
            return 0;
        }
    }

    /* fork each command */
    for (int i = 0; i < num_commands; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            /* close pipes */
            for (int j = 0; j < 2 * num_pipes; ++j) close(pipefd[j]);
            return 0;
        }

        if (pid == 0) {
            /* Child process */

            /* If not first, stdin <- read end of previous pipe */
            if (i > 0) {
                int read_fd = pipefd[(i-1)*2];
                if (dup2(read_fd, STDIN_FILENO) < 0) { perror("dup2 stdin"); exit(EXIT_FAILURE); }
            } else if (commands[i].input_file) {
                /* first command and has input redirection */
                int fd = open(commands[i].input_file, O_RDONLY);
                if (fd < 0) { perror("open input"); exit(EXIT_FAILURE); }
                if (dup2(fd, STDIN_FILENO) < 0) { perror("dup2 infile"); exit(EXIT_FAILURE); }
                close(fd);
            }

            /* If not last, stdout -> write end of current pipe */
            if (i < num_commands - 1) {
                int write_fd = pipefd[i*2 + 1];
                if (dup2(write_fd, STDOUT_FILENO) < 0) { perror("dup2 stdout"); exit(EXIT_FAILURE); }
            } else if (commands[i].output_file) {
                /* last command and has output redirection */
                int fd = open(commands[i].output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("open output"); exit(EXIT_FAILURE); }
                if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2 outfile"); exit(EXIT_FAILURE); }
                close(fd);
            }

            /* close all pipe fds in child */
            for (int j = 0; j < 2 * num_pipes; ++j) close(pipefd[j]);

            /* exec */
            if (commands[i].args[0] == NULL) exit(EXIT_FAILURE);
            execvp(commands[i].args[0], commands[i].args);
            /* if exec fails */
            fprintf(stderr, "myshell: exec failed for '%s': %s\n", commands[i].args[0], strerror(errno));
            exit(EXIT_FAILURE);
        } else {
            /* parent */
            children[i] = pid;
        }
    }

    /* parent: close all pipe fds */
    for (int j = 0; j < 2 * num_pipes; ++j) close(pipefd[j]);

    if (background) {
        pid_t last = children[num_commands - 1];
        return last;
    } else {
        /* wait for all children */
        int status;
        for (int i = 0; i < num_commands; ++i) {
            waitpid(children[i], &status, 0);
        }
        return 0;
    }
}

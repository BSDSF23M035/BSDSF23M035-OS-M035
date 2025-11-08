#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COMMANDS 100
#define MAX_ARGS 100
#define MAX_LINE 1024

typedef struct Command {
    char *args[MAX_ARGS];   // Command arguments
    char *input_file;       // Input redirection
    char *output_file;      // Output redirection
} Command;

// Function declarations
void shell_loop();
int parse_line(char *line, Command commands[], int *num_commands);
void execute_commands(Command commands[], int num_commands);

#endif

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CMD_LEN 1024
#define MAX_TOKENS 64
#define MAX_LINE_LEN 512
#define MAX_BLOCK_LINES 64

// IF-THEN-ELSE-FI global state
extern int if_mode;
extern char if_cmd[MAX_LINE_LEN];
extern char then_block[MAX_BLOCK_LINES][MAX_LINE_LEN];
extern char else_block[MAX_BLOCK_LINES][MAX_LINE_LEN];
extern int then_count, else_count;
extern int in_then, in_else;

// Function prototypes
void trim_spaces(char *str);
void execute_command(char **args);
void execute_if_block();
void reset_if_state();

#endif

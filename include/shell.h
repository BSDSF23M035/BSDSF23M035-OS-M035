#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_LEN 1024
#define MAXARGS 64
#define ARGLEN  64
#define PROMPT  "myshell> "
#define HISTORY_SIZE 20

char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char* arglist[]);
int handle_builtin(char** arglist);

// History functions
void add_history(const char* cmd);
void show_history();
char* get_history_command(int index);

#endif

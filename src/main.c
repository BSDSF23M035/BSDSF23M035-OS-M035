#include "shell.h"

void shell_loop() {
    char line[MAX_LINE];
    Command commands[MAX_COMMANDS];
    int num_commands;

    while (1) {
        printf("myshell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) break;
        if (strcmp(line, "\n") == 0) continue;

        num_commands = 0;
        parse_line(line, commands, &num_commands);
        execute_commands(commands, num_commands);
    }
}

int main() {
    shell_loop();
    return 0;
}

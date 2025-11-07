#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // --- Handle !n history recall before tokenization ---
        if (cmdline[0] == '!' && isdigit(cmdline[1])) {
            int index = atoi(&cmdline[1]);
            char* hist_cmd = get_history_command(index);
            if (hist_cmd != NULL) {
                printf("Re-executing: %s\n", hist_cmd);
                free(cmdline);
                cmdline = hist_cmd;
            } else {
                free(cmdline);
                continue;
            }
        }

        // --- Add command to history ---
        add_history(cmdline);

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (!handle_builtin(arglist)) {
                execute(arglist);
            }

            for (int i = 0; arglist[i] != NULL; i++)
                free(arglist[i]);
            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

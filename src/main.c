#include "shell.h"

/* main loop:
 * - read line with readline()
 * - detect trailing '&' to run background
 * - add non-empty lines to history via add_history()
 * - tokenize and handle builtins or execute
 * - cleanup finished background jobs
 */

int main(void) {
    char *line = NULL;
    char **argv = NULL;

    /* Initialize readline completion function */
    rl_attempted_completion_function = myshell_completion;
    rl_completion_append_character = ' ';
    init_command_list(); /* scan $PATH once to prepare command list */

    while (1) {
        line = read_cmdline();
        if (line == NULL) { /* EOF (Ctrl+D) */
            printf("\n");
            break;
        }

        /* Trim leading spaces */
        char *s = line;
        while (*s && isspace((unsigned char)*s)) s++;
        if (*s == '\0') { free(line); cleanup_jobs(); continue; }

        /* Detect trailing '&' */
        int background = 0;
        int len = strlen(s);
        while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
        if (len > 0 && s[len-1] == '&') {
            background = 1;
            s[--len] = '\0';
            while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
        }

        if (s[0] == '\0') { free(line); cleanup_jobs(); continue; }

        /* Add to readline history */
        add_history(s);

        /* Tokenize (note: tokenize duplicates tokens) */
        argv = tokenize(s);
        if (argv == NULL) { free(line); cleanup_jobs(); continue; }

        if (!handle_builtin(argv)) {
            execute_cmd(argv, background);
        }

        /* Free argv */
        for (int i = 0; argv[i] != NULL; i++) free(argv[i]);
        free(argv);

        free(line);
        cleanup_jobs();
    }

    /* cleanup */
    free_command_list();
    cleanup_jobs();
    rl_clear_history();
    printf("Shell exited.\n");
    return 0;
}

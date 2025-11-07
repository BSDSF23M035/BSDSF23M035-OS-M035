#include "shell.h"

/* Note: main uses readline() directly.  Make sure your Makefile links -lreadline. */

int main(void) {
    char *line = NULL;
    char **argv = NULL;

    /* Initialize completion table for readline */
    rl_attempted_completion_function = myshell_completion;
    rl_completion_append_character = ' ';
    init_command_list();

    while (1) {
        line = readline(PROMPT);
        if (line == NULL) { /* EOF / Ctrl+D */
            printf("\n");
            break;
        }

        /* Trim leading spaces */
        char *s = line;
        while (*s && isspace((unsigned char)*s)) s++;
        if (*s == '\0') { free(line); cleanup_jobs(); continue; }

        /* Add to readline history */
        add_history(s);

        /* Tokenize - tokenize will strdup tokens so we can free line */
        argv = tokenize(s);
        if (argv == NULL) { free(line); cleanup_jobs(); continue; }

        /* Handle builtin commands first */
        if (handle_builtin(argv)) {
            /* builtin handled: free argv & line and continue */
            for (int k = 0; argv[k] != NULL; k++) free(argv[k]);
            free(argv);
            free(line);
            cleanup_jobs();
            continue;
        }

        /* Detect whether there's a pipe or redirection in argv */
        int has_pipe = 0, has_redirect = 0;
        for (int i = 0; argv[i] != NULL; i++) {
            if (strcmp(argv[i], "|") == 0) has_pipe = 1;
            if (strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">") == 0) has_redirect = 1;
        }

        if (has_pipe) {
            execute_pipe(argv);
        } else if (has_redirect) {
            execute_redirect(argv);
        } else {
            execute(argv);
        }

        /* free argv tokens */
        for (int k = 0; argv[k] != NULL; k++) free(argv[k]);
        free(argv);
        free(line);

        /* reap any finished background jobs (non-blocking) */
        cleanup_jobs();
    }

    /* final cleanup */
    free_command_list();
    cleanup_jobs();
    rl_clear_history();
    printf("Shell exited.\n");
    return 0;
}

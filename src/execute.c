#include "shell.h"

/* Execute a command.
 * - argv: NULL-terminated argument list
 * - background: 0 = foreground (wait), 1 = background (do not wait)
 */
int execute_cmd(char **argv, int background) {
    if (argv == NULL || argv[0] == NULL) return -1;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        /* In child: reset signals to defaults for interactive behavior */
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        /* Execute program */
        execvp(argv[0], argv);
        perror("exec");
        _exit(EXIT_FAILURE);
    } else {
        if (background) {
            int jobno = -1;
            char cmdbuf[MAX_CMD_LEN] = "";
            for (int i = 0; argv[i] != NULL && i < MAX_ARGV - 1; ++i) {
                if (i) strncat(cmdbuf, " ", sizeof(cmdbuf)-strlen(cmdbuf)-1);
                strncat(cmdbuf, argv[i], sizeof(cmdbuf)-strlen(cmdbuf)-1);
            }
            add_job(pid, cmdbuf, &jobno);
            if (jobno > 0) printf("[%d] %d\n", jobno, pid);
            else printf("[BG] %d\n", pid);
            return 0;
        } else {
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                return -1;
            }
            return 0;
        }
    }
}

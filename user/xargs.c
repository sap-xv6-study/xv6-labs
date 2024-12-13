#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define BUFFER_SIZE 512

void run_command(int argc, char **argv, char *line) {
    char *cmd_argv[MAXARG];
    int i;

    // Copy the command and its arguments
    for (i = 1; i < argc; i++) {
        cmd_argv[i - 1] = argv[i];
    }

    // Append the line from standard input
    cmd_argv[argc - 1] = line;
    cmd_argv[argc] = 0; // Null-terminate the argv array

    if (fork() == 0) {
        // Child process
        exec(cmd_argv[0], cmd_argv);
        printf("xargs: exec failed for command %s\n", cmd_argv[0]);
        exit(1);
    } else {
        // Parent process
        wait(0); // Wait for the child to finish
    }
}

// find . b | xargs grep hello
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: xargs <command> [args...]\n");
        exit(1);
    }

    char buf[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    int n, line_idx = 0;

    // Read from standard input
    while ((n = read(0, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                // End of line
                line[line_idx] = 0; // Null-terminate the line
                run_command(argc, argv, line);
                line_idx = 0; // Reset line buffer
            } else {
                line[line_idx++] = buf[i];
                if (line_idx >= BUFFER_SIZE - 1) {
                    printf("xargs: line too long\n");
                    exit(1);
                }
            }
        }
    }

    // Handle the last line if it doesn't end with a newline
    if (line_idx > 0) {
        line[line_idx] = 0; // Null-terminate
        run_command(argc, argv, line);
    }

    exit(0);
}

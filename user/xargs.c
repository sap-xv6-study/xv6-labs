#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define BUFFER_SIZE 512

void remove_quotes(char *str) {
    int i, j = 0;
    for (i = 0; str[i] != 0; i++) {
        if (str[i] != '"') {
            str[j++] = str[i];
        }
    }
    str[j] = 0;
}

void run_command(int cmd_argc, char **cmd_argv, int line_count, char **lines) {
    char *exec_argv[MAXARG];
    int i, j = 0;

    // Copy the command and its arguments
    for (i = 0; i < cmd_argc; i++) {
        exec_argv[j++] = cmd_argv[i];
    }

    // Append the input lines
    for (i = 0; i < line_count; i++) {
        exec_argv[j++] = lines[i];
    }

    exec_argv[j] = 0; // Null terminate

    if (fork() == 0) {
        // Child process
        exec(exec_argv[0], exec_argv);
        printf("xargs: exec failed for command %s\n", exec_argv[0]);
        exit(1);
    } else {
        // Parent process
        wait(0);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: xargs [-n num] <command> [args...]\n");
        exit(1);
    }

    int i, n, line_idx = 0;
    int max_args_per_run = 1;  // default is 1 line per run
    int start_index = 1;       // where the command might start

    // Handle -n option if given
    if (argc > 2 && strcmp(argv[1], "-n") == 0) {
        if (argc < 4) {
            printf("Usage: xargs [-n num] <command> [args...]\n");
            exit(1);
        }

        max_args_per_run = atoi(argv[2]);
        if (max_args_per_run < 1) {
            printf("xargs: invalid value for -n\n");
            exit(1);
        }
        start_index = 3;
    }

    if (start_index >= argc) {
        printf("xargs: no command specified\n");
        exit(1);
    }

    char buf[BUFFER_SIZE];
    char line[BUFFER_SIZE];

    int cmd_argc = 0;
    char *cmd_argv[MAXARG];

    // Store the fixed command and its arguments
    for (i = start_index; i < argc && cmd_argc < MAXARG - 1; i++) {
        cmd_argv[cmd_argc++] = argv[i];
    }
    cmd_argv[cmd_argc] = 0;

    char *lines[MAXARG];
    int line_count = 0;

    // Read from stdin
    while ((n = read(0, buf, sizeof(buf))) > 0) {
        for (i = 0; i < n; i++) {
            // Check if we have a backslash followed by 'n' which we want to treat as newline
            if (buf[i] == '\\' && (i + 1) < n && buf[i+1] == 'n') {
                // Treat this as newline
                line[line_idx] = 0;
                char *saved_line = malloc(line_idx + 1);
                if (!saved_line) {
                    printf("xargs: malloc failed\n");
                    exit(1);
                }
                memmove(saved_line, line, line_idx + 1);
                remove_quotes(saved_line);
                lines[line_count++] = saved_line;

                if (line_count == max_args_per_run) {
                    run_command(cmd_argc, cmd_argv, line_count, lines);
                    for (int k = 0; k < line_count; k++) {
                        free(lines[k]);
                    }
                    line_count = 0;
                }

                line_idx = 0;
                i++; // Skip the 'n' character as well
            } else if (buf[i] == '\n') {
                // Actual newline from input
                line[line_idx] = 0;
                char *saved_line = malloc(line_idx + 1);
                if (!saved_line) {
                    printf("xargs: malloc failed\n");
                    exit(1);
                }
                memmove(saved_line, line, line_idx + 1);
                remove_quotes(saved_line);
                lines[line_count++] = saved_line;

                if (line_count == max_args_per_run) {
                    run_command(cmd_argc, cmd_argv, line_count, lines);
                    for (int k = 0; k < line_count; k++) {
                        free(lines[k]);
                    }
                    line_count = 0;
                }

                line_idx = 0;
            } else {
                // Normal character, just add to line buffer
                line[line_idx++] = buf[i];
                if (line_idx >= BUFFER_SIZE - 1) {
                    printf("xargs: line too long\n");
                    exit(1);
                }
            }
        }
    }

    // If there's leftover data in line buffer (and no trailing newline),
    // treat it as a line
    if (line_idx > 0) {
        line[line_idx] = 0;
        char *saved_line = malloc(line_idx + 1);
        if (!saved_line) {
            printf("xargs: malloc failed\n");
            exit(1);
        }
        memmove(saved_line, line, line_idx + 1);
        lines[line_count++] = saved_line;
    }

    // Run command for leftover lines
    if (line_count > 0) {
        run_command(cmd_argc, cmd_argv, line_count, lines);
        for (int k = 0; k < line_count; k++) {
            free(lines[k]);
        }
    }

    exit(0);
}

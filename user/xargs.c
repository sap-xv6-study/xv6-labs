#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define BUFFER_SIZE 512

void remove_quotes(char *str);
void parse_arguments(int argc, char **argv, int *max_args_per_run, int *start_index, char **cmd_argv, int *cmd_argc);
void process_input(char **cmd_argv, int cmd_argc, int max_args_per_run);
void allocate_and_save_line(char *line, char **lines, int *line_count);
void execute_lines(char **cmd_argv, int cmd_argc, char **lines, int line_count);
void cleanup_lines(char **lines, int line_count);

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

    for (i = 0; i < cmd_argc; i++) {
        exec_argv[j++] = cmd_argv[i];
    }
    for (i = 0; i < line_count; i++) {
        exec_argv[j++] = lines[i];
    }

    exec_argv[j] = 0;

    if (fork() == 0) {
        exec(exec_argv[0], exec_argv);
        printf("xargs: exec failed for command %s\n", exec_argv[0]);
        exit(1);
    } else {
        wait(0);
    }
}

void parse_arguments(int argc, char **argv, int *max_args_per_run, int *start_index, char **cmd_argv, int *cmd_argc) {
    if (argc > 2 && strcmp(argv[1], "-n") == 0) {
        *max_args_per_run = atoi(argv[2]);
        if (*max_args_per_run < 1) {
            printf("xargs: invalid value for -n\n");
            exit(1);
        }
        *start_index = 3;
    }
    for (int i = *start_index; i < argc && *cmd_argc < MAXARG - 1; i++) {
        cmd_argv[(*cmd_argc)++] = argv[i];
    }
    cmd_argv[*cmd_argc] = 0;
}

void allocate_and_save_line(char *line, char **lines, int *line_count) {
    char *saved_line = malloc(strlen(line) + 1);
    if (!saved_line) {
        printf("xargs: malloc failed\n");
        exit(1);
    }
    strcpy(saved_line, line);
    remove_quotes(saved_line);
    lines[(*line_count)++] = saved_line;
}

void execute_lines(char **cmd_argv, int cmd_argc, char **lines, int line_count) {
    if (line_count > 0) {
        run_command(cmd_argc, cmd_argv, line_count, lines);
        cleanup_lines(lines, line_count);
    }
}

void cleanup_lines(char **lines, int line_count) {
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
}

void process_input(char **cmd_argv, int cmd_argc, int max_args_per_run) {
    char buf[BUFFER_SIZE], line[BUFFER_SIZE];
    int n, line_idx = 0, line_count = 0;
    char *lines[MAXARG];

    while ((n = read(0, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                line[line_idx] = 0;
                allocate_and_save_line(line, lines, &line_count);
                if (line_count == max_args_per_run) {
                    execute_lines(cmd_argv, cmd_argc, lines, line_count);
                    line_count = 0;
                }
                line_idx = 0;
            } else {
                line[line_idx++] = buf[i];
            }
        }
    }
    if (line_idx > 0) {
        line[line_idx] = 0;
        allocate_and_save_line(line, lines, &line_count);
    }
    execute_lines(cmd_argv, cmd_argc, lines, line_count);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: xargs [-n num] <command> [args...]\n");
        exit(1);
    }

    int max_args_per_run = 1, start_index = 1, cmd_argc = 0;
    char *cmd_argv[MAXARG];
    parse_arguments(argc, argv, &max_args_per_run, &start_index, cmd_argv, &cmd_argc);

    process_input(cmd_argv, cmd_argc, max_args_per_run);

    exit(0);
}

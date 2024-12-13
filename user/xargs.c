// clang-format off
#include "kernel/types.h"
// clang-format on
#include "kernel/param.h"
#include "user/user.h"

#define BUF_SIZE 1000
#define NULL ((void*)0)
#define STDIN_FD 0
#define TRUE 1
#define FALSE 0

typedef enum {
    EOF,
    ERROR,
    SUCCESS,
} ReadLineStatus;

typedef struct {
    ReadLineStatus status;
    char* buf;
} ReadLineResult;

ReadLineResult read_line(int fd)
{
    static char buf[BUF_SIZE];
    int i = 0;
    char c;
    ReadLineResult res;
    int n;
    int is_buf_filled_in_cur_call = FALSE;

    while ((n = read(fd, &c, 1)) > 0) {
        if (c == '\n') {
            buf[i] = '\0';
            res.status = SUCCESS;
            res.buf = buf;
            return res;
        }
        buf[i] = c;
        is_buf_filled_in_cur_call = TRUE;
        ++i;
    }

    if (n == 0) {
        buf[i] = '\0'; // EOF
        if (is_buf_filled_in_cur_call) {
            res.status = SUCCESS;
            res.buf = buf;
            return res;
        }
        res.status = EOF;
        res.buf = buf;
        return res;
    }
    res.status = ERROR;
    res.buf = NULL;
    return res; // error
}

char** parse_argv(int argc, char* original_argv[], char* buf)
{
    // move args passed to xargs
    static char* exec_argv[MAXARG];

    int i = 1;
    for (; i < argc; ++i) {
        exec_argv[i - 1] = original_argv[i];
    }
    --i;

    // split input line
    int st = 0;
    int en = 0;
    while (buf[en] != '\0') {
        if (buf[en] == ' ') {
            buf[en] = '\0';
            exec_argv[i] = &buf[st];
            ++i;
            st = en + 1;
        }
        ++en;
    }
    exec_argv[i] = &buf[st];
    exec_argv[++i] = NULL;

    return exec_argv;
}

int main(int argc, char* argv[])
{
    if (argc <= 1) {
        fprintf(2, "Usage: xargs <command> [args...]\n");
        exit(1);
    }

    while (1) {
        ReadLineResult res = read_line(STDIN_FD);
        if (res.status == ERROR) {
            fprintf(2, "error while reading line.\n");
            exit(1);
        }

        if (res.status == EOF) {
            break;
        }

        // move args passed to xargs
        char** exec_argv = parse_argv(argc, argv, res.buf);

        // exec
        int pid = fork();
        if (pid > 0) {
            // parent
            pid = wait((int*)0);
        } else if (pid == 0) {
            // child
            exec(argv[1], exec_argv);
            fprintf(2, "exec error\n");
        } else {
            fprintf(2, "fork error\n");
            exit(1);
        }
    }

    return 1;
}

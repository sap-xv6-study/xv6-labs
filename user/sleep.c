#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// argc -> argument count, argv -> argument vector
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: sleep <time>\n");
        exit(1);
    }

    int time = atoi(argv[1]);
    if (time <= 0) {
        printf("Error: sleep time must be a positive integer\n");
        exit(1);
    }

    // Call the sleep system call
    sleep(time);

    // Exit the program
    exit(0);
}

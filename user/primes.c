#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void filter_primes(int read_fd) {
    int prime;

    // no more primes to read
    if (read(read_fd, &prime, sizeof(prime)) == 0) {
        close(read_fd);
        exit(0);
    }

    printf("prime %d\n", prime);

    int pipe_fd[2];
    pipe(pipe_fd);

    int pid = fork();
    switch (pid) {
      // new child process
      case 0: {
        close(pipe_fd[1]); // close write end in the child
        filter_primes(pipe_fd[0]);
      }
      case -1: {
          printf("Error: Fork failed\n");
          exit(1);
          break;
      }
      // current process
      default: {
        close(pipe_fd[0]); // close read end 

        int num;
        while (read(read_fd, &num, sizeof(num)) > 0) {
            if (num % prime != 0) {
                write(pipe_fd[1], &num, sizeof(num)); // pass non-multiples to the next process
            }
        }

        close(pipe_fd[1]); // close write end when done
        close(read_fd);    // close read end
        wait(0);           // wait for the child process to finish
        exit(0);
      }
    }
}

int main() {
    int pipe_fd[2];
    pipe(pipe_fd);

    int pid = fork();

    switch (pid) {
      // child -> filter primes
      case 0: {
        close(pipe_fd[1]); // Close write end in the child
        filter_primes(pipe_fd[0]);
      }
      case -1: {
          printf("Error: Fork failed\n");
          exit(1);
          break;
      }
      // parent -> generate numbers
      default: {
        close(pipe_fd[0]); // Close read end in the parent

        // generate numbers 2 through 35
        for (int i = 2; i <= 35; i++) {
            write(pipe_fd[1], &i, sizeof(i));
        }

        close(pipe_fd[1]); // close write end
        wait(0);           // wait for the child process to finish
        exit(0);
      }
    }
    return 0;
}
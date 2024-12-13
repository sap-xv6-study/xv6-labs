#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    int pipe1[2]; 
    int pipe2[2]; 


    pipe(pipe1); // parent -> child
    pipe(pipe2); // child -> parent
    if (pipe1[0] < 0 || pipe1[1] < 0 || pipe2[0] < 0 || pipe2[1] < 0) {
        printf("Error: Pipe failed\n");
        exit(1);
    }

    // Fork a child process 
    int pid = fork();

    switch (pid) {
      // child
      case 0: {
          char buf;

          // Read from parent, initialize buf to 'x' after reading from parent
          read(pipe1[0], &buf, 1);
          printf("%d: received ping\n", getpid());

          // Send response to parent
          write(pipe2[1], &buf, 1);

          // Exit
          exit(0);
          break;
      }
      case -1:
          printf("Error: Fork failed\n");
          exit(1);
          break;
      // parent
      default: {
          char buf = 'i';

          // Send byte to child
          write(pipe1[1], &buf, 1);

          // Read response from child
          read(pipe2[0], &buf, 1);
          printf("%d: received pong\n", getpid());

          // Wait for child to exit
          wait(0);

          exit(0);
          break;
      }
    }
}

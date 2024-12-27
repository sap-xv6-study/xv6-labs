#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"


int
main(int argc, char *argv[])
{
  if(argc != 2){
    printf("Usage: secret the-secret\n");
    exit(1);
  }
  char *end = sbrk(PGSIZE*32); // alloc 32 pages (breakpoint move)
  end = end + 9 * PGSIZE; // go to 9th page address
  strcpy(end, "my very very very secret pw is:   ");
  strcpy(end+32, argv[1]); // write secret after 32byte offset
  exit(0);
}


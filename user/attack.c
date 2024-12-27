#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  char *end = sbrk(PGSIZE * 32);
  end = end + 8 * PGSIZE;
  write(2, end + 16, 8);

  exit(1);
}

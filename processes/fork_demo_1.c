// Chapter 7 Process Architecture and Control
// Program that demonstrates a bit about for fork() works
// The point of the program is simple to demonstrate that because the child
// changed their values in the copy of the memory image, not in a shared memory.
// page 18

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global = 10;

int main(int argc, char *argv[]) {
  int local = 0;
  pid_t pid;

  printf("Parent process: (Before fork())");
  printf(" local = %d, global = %d \n", local, global);

  if ((pid = fork()) == -1) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    // child executes this branch
    printf("After the fork in the child:");
    local++;
    global++;
    printf(" local = %d, global = %d\n", local, global);
  } else {
    // parent executes this branch
    sleep(2);  // sleep long enough for child's output to appear
  }

  // both prcesses execute this print statement
  printf("pid = %d, local = %d, global = %d \n", getpid(), local, global);

  return 0;
}

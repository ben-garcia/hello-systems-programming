// Chapter 7 Process Architecture and Control
// This program is an example that puts together the uee of fork(), exit(),
// and wait(). It is the typical way in which these three primitives are used.
// In this example the user is prompted to suppoly an exit value for the child,
// which is then passed to the exit() call, to show that the value is then
// available to the parent in the wait() call.

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void child() {
  int exit_code;
  printf("I am the child and my process id is %d.\n", getpid());
  sleep(2);
  printf("Enter a value for the child exit code:");
  scanf("%d", &exit_code);
  exit(exit_code);
}

int main(int argc, char *argv[]) {
  int pid;
  int status;

  printf("Starting up...\n");
  if ((pid = fork()) == -1) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    child();
  } else {
    printf("My child has pid %d and my pid is %d.\n", pid, getpid());
    if ((pid = wait(&status)) == -1) {
      perror("wait failed");
      exit(2);
    }

    if (WIFEXITED(status)) {  // low order byte of status equals 0
      printf("Parent: Child %d exited with status %d.\n", pid,
             WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      printf("Parent: Child %d exited with error code %d.\n", pid,
             WTERMSIG(status));

#ifndef WCOREDUMP
      if (WCOREDUMP(status)) {
        printf("Parent: A core dump took place.\n");
      }
    }
#endif
  }
}

return 0;
}

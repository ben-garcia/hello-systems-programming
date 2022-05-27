// Chapter 7 Process Architecture and Control
// This program demonstrates how to use fork() and signals to synchronize
// a child and its parent.
// on page 24

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void c_action(int signum) {
  // nothing to do here
}

int main(int argc, char *argv[]) {
  pid_t pid;
  int status;
  static struct sigaction child_act;

  switch (pid = fork()) {
    case -1:
      // fork failed
      perror("fork() failed!");
      exit(1);
    case 0: {
      // child executes this branch
      // set SIGUSR1 action for child
      int i;
      int x = 1;
      child_act.sa_handler = c_action;
      sigaction(SIGUSR1, &child_act, NULL);
      pause(); // wait for parent to send signal
      printf("Child process: starting computation...\n");
      for (i = 0; i < 10; i++) {
        printf("2^%d = %d\n", i, x);
        x = 2 * x;
      }
      exit(0);
    }
    default:
      // parent node
      printf(
          "Parent process: "
          "Will wait 2 seconds to prove child waits.\n");
      sleep(2);  // to prove that child waits for signal
      printf(
          "Parent process: "
          "Sending child notice to start computation.\n");
      kill(pid, SIGUSR1);

      // parent waits for child to return here
      if ((pid = wait(&status)) == -1) {
        perror("wait failed");
        exit(2);
      }
      printf("Parent process: child terminated.\n");
      exit(0);
  }
}

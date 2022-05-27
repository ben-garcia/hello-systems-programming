// Chapter 7 Process Architecture and Control
// This program demonstrates the use of waitpid() with the WNOHANG flag
// on page 39

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void child() {
  int exit_code;

  printf("I am the child and my process id is %d.\n", getpid());
  sleep(2);
  printf("Enter a value for the child exit code followd by <ENTER>.\n");
  scanf("%d", &exit_code);
  exit(exit_code);
}

int main(int argc, char *argv[]) {
  pid_t pid;
  int status;
  int signum;

  printf("Starting up...\n");
  if ((pid = fork()) == -1) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    child();
  } else {
    // wait for specific child process with waitpid()
    // If no child has terminated, do not block in waitpid()
    // Instead just sleep. (World do something usefull instead.)
    // The WNOHANG flag allows it to continue polling the waitpid() call
    // and so something else in the meanwhile.
    while (waitpid(pid, &status, WNOHANG) == 0) {
      printf("still waiting for child\n");
      sleep(1);
    }

    // pid is the pid of the child that terminated
    if (WIFEXITED(status)) {
      printf("Exit status of child %d was %d,\n", pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      signum = WTERMSIG(status);
      printf("Parent: Child %d exited by signal %d.\n", pid, signum);

#ifdef WCOREDUMP
      if (WCOREDUMP(status)) {
        printf("Parent: A core dump took place.\n");
      }
#endif
    }
  }

  return 0;
}

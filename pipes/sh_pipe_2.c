// Chapter 8 Interprocess Communications Part 1
// This program uses dup2() syscall to redirect I/O
// The problem with dup():
//    There is a small window of time between closing standard ouput and
//    duplicating the write-end of the pipe in which the child could be
//    interupted by a signal whose handler might close file descriptors so that
//    descriptor returned by dup() will not e the one that was just closed.
//    on page 22

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fd[2];
  int i;
  pid_t child1_pid;
  pid_t child2_pid;

  if (argc < 3) {
    fprintf(stderr, "usage: %s command1 command2\n", argv[0]);
    exit(1);
  }

  if (pipe(fd) == -1) {
    perror("pipe call");
    exit(2);
  }

  switch (child1_pid = fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:                            // child1
      dup2(fd[1], fileno(stdout));     // new stdout points to fd[1]
      close(fd[0]);                    // close input end of pipe
      close(fd[1]);                    // close output end of pipe
      execlp(argv[1], argv[1], NULL);  // run the first command
      perror("execlp");
      exit(1);
    default:
      switch (child2_pid = fork()) {
        case -1:
          perror("child2 fork");
          exit(1);
        case 0:                            // child2
          dup2(fd[0], fileno(stdin));      // now stdin points to fd[0]
          close(fd[0]);                    // close input end of pipe
          close(fd[1]);                    // close output end of pipe
          execlp(argv[2], argv[2], NULL);  // run the second command
          perror("execlp");
          exit(2);
        defautt:
          close(fd[0]);  // parent must close its end of the first pipe
          close(fd[1]);
          for (i = 1; i < 2; i++) {
            if (wait(NULL) == -1) {
              perror("wait failed");
              exit(3);
            }
          }
          return 0;
      }
  }
  return 0;
}

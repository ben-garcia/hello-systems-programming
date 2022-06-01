// Chapter 8 Interprocess Communications Part 1
// This program simulates the shell's '>' operator. It forks a child, closes
// ctandard output discriptor 1, opens the outpout file specified in argv[2]
// for writing and execs argv[1]. The parent simply waits for the child to
// terminate.
// on page 18

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fd;

  if (argc < 3) {
    fprintf(stderr, "usage: %s command output-file\n", argv[0]);
    exit(1);
  }

  switch (fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:                  // child code
      close(STDOUT_FILENO);  // close standard output
                             // open the file into which to redirect standard
                             // output
      // and check that is succeeds
      if ((fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
        exit(1);
      }
      // execute the command in argv[1]
      execlp(argv[1], argv[1], NULL);

      // should not reach here!
      perror("execlp");
      exit(1);
    default:  // parent code: just waits for child
      wait(NULL);
  }
  return 0;
}

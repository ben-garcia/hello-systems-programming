// Chapter 8 Interprocess Communications Part 1
// This program uses dup() syscall to redirect I/O
// on page 20

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fd[2];

  if (argc < 3) {
    fprintf(stderr, "usage: %s command1 command2\n", argv[0]);
    exit(1);
  }

  if (pipe(fd) == -1) {
    perror("pipe call");
    exit(2);
  }

  switch (fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      close(fileno(stdout));  // by closing fd 1
      dup(fd[1]);             // pipe write is replaced with stdout
      close(fd[0]);           // close read end since child does not use it
      close(fd[1]);           // close write end since it is not needed now
      execlp(argv[1], argv[1], NULL);
      perror("execlp");
      exit(1);
    default:
      close(fileno(stdin));  // by closing fd 0
      dup(fd[0]);            // pipe read is replaced with stdin
      close(fd[1]);          // close write end to prevent child from blocking
      close(fd[0]);          // close read end since it is not needed now
      execlp(argv[2], argv[2], NULL);
      exit(2);
  }
  return 0;
}

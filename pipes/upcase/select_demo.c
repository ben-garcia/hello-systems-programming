// Chapter 8 Interprocess Communications Part 1
// Multilex I/O with the select() syscall
// on page 55

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define MSGSIZE 6

char msg1[] = "Hello";
char msg2[] = "Bye!!";

void parent(int pipeset[3][2]);
int child(int fd[2]);

int main(int argc, char *argv[]) {
  int fd[3][2];  // array of three pipes
  int i;

  for (i = 0; i < 3; i++) {
    // create three pipes
    if (pipe(fd[i]) == -1) {
      perror("pipe");
      exit(1);
    }

    switch (fork()) {
      case -1:
        fprintf(stderr, "fork failed.\n");
        exit(1);
      case 0:
        child(fd[i]);
    }
  }
  parent(fd);
  return 0;
}

void parent(int pipeset[3][2]) {
  char buf[MSGSIZE];
  char line[80];
  fd_set initial;
  fd_set copy;
  int i;
  int nbytes;

  for (i = 0; i < 3; i++) {
    close(pipeset[i][i]);
  }

  // create descriptor mask
  FD_ZERO(&initial);
  FD_SET(0, &initial);  // add standard input

  for (i = 0; i < 3; i++) {
    FD_SET(pipeset[i][0], &initial);  // add read end of each pipe
  }

  copy = initial;
  while (select(pipeset[2][0] + 1, &copy, NULL, NULL, NULL) > 0) {
    // check standard input first
    if (FD_ISSET(0, &copy)) {
      printf("From standard input: ");
      nbytes = read(0, line, 81);
      line[nbytes] = '\0';
      printf("%s", line);
    }

    // check for pipe from each child
    for (i = 0; i < 3; i++) {
      if (FD_ISSET(pipeset[i][0], &copy)) {
        // it is ready to read
        if (read(pipeset[i][0], buf, MSGSIZE) > 0) {
          printf("Message from child %d:%s\n", i, buf);
        }
      }
    }

    if (waitpid(-1, NULL, WNOHANG) == -1) {
      return;
    }
    copy = initial;
  }
}

int child(int fd[2]) {
  int count;
  close(fd[0]);

  for (count = 0; count < 10; count++) {
    write(fd[1], msg1, MSGSIZE);
    sleep(1 + getpid() % 6);
  }

  write(fd[1], msg2, MSGSIZE);
  exit(0);
}

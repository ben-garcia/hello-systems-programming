// Chapter 8 Interprocess Communications Part 1
// Program that demonstrates the use of the pipe() syscall
// on page 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1
#define NUM 5
#define BUFSIZE 32

int main(int argc, char *argv[]) {
  int i;
  int n_bytes;
  int fd[2];
  char message[BUFSIZE + 1];

  if (pipe(fd) == -1) {
    perror("pipe call");
    exit(2);
  }

  for (i = 1; i <= NUM; i++) {
    sprintf(message, "hello #%2d\n", i);
    write(fd[WRITE_END], message, strlen(message));
  }

  close(fd[WRITE_END]);

  printf("%d messages sent: sleeping a bit. Please wait...\n", NUM);
  sleep(3);

  while ((n_bytes = read(fd[READ_END], message, BUFSIZE)) != 0) {
    if (n_bytes > 0) {
      message[n_bytes] = '\0';
      printf("%s", message);
    } else {
      exit(1);
    }

    fflush(stdout);
    exit(0);
  }
}

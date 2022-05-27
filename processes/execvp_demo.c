// Chapter 7 Process Architecture and Control
// Program illustrates the use of the execvp() syscall
// on page 32 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s program args\n", argv[0]);
    exit(1);
  }

  execvp(argv[1], argv + 1);
  perror("execvp");
  exit(1);
}

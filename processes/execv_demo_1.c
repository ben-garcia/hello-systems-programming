// Chapter 7 Process Architecture and Control
// This program looks at how to use the execve() system call.
// It execs the echo command
// on page 29

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[]) {
  if (argc < 2) {
    printf("usage: execdemo1 arg1 [arg2 ...]\n");
    exit(1);
  }

  execve("/bin/echo", argv, envp);
  fprintf(stderr, "execve() failed to run.\n");
  exit(1);
}

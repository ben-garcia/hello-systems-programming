// Chapter 7 Process Architecture and Control
// This program uses execve() to execute the firs tcommand line argument of
// the program, passing to it the remaining arguments from the command line.
// In other words, if we supply a line like
// $ execvedemo2 /bin/ls -l ..
// it will execute it as if you typed the /bin/ls -l .. on a line by itself.
// on page 30

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s proram args\n", argv[0]);
    exit(1);
  }

  execve(argv[1], argv + 1, envp);
  fprintf(stderr, "execve() failed to run.\n");
  exit(1);
}

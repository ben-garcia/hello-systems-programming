// Chapter 7 Process Architecture and Control 
// Program that uses write syscall because it's unbuffered
// on page 23

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

int main(int argc, char *argv[]) {
  int i;
  int N;
  char str[32];
  int flags;

  // Put standard output into atomic append mode
  flags = fcntl(1, F_GETFL);
  flags |= (O_APPEND);
  if (fcntl(1, F_SETFL, flags) == -1) {
    exit(1);
  }

  // Get the command line value and convert to an int
  // If none, use default of 4. If invalid, exit
  N = (argc > 1) ? atoi(argv[1]) : 4;
  if (N == 0) {
    exit(1);
  }

  // Print a message and flush it if this is not a terminal
  printf("About to create many processes...\n");
  if (!isatty(fileno(stdout))) {
    fflush(stdout);
  }

  // Now fork the child processes. Check return values and exit
  // if we have a problem. Note that the exit() may be executed
  // only for some children and not others.
  for (i = 0; i < N; i++) {
    if (fork() == -1) {
      exit(1);
    }
  }

  // Create the output string that the process will write, and write using
  // system call.
  sprintf(str, "Process id = %d\n", getpid());
  write(1, str, strlen(str));
  fflush(stdout); // to force output before shell prompt
  sleep(1); // to give time to the shell to display prompt

  return 0;
}

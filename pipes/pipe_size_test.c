// Chapter 8 Interprocess Communications Part 1
// The following program can be run on any system to test the maximum capacity
// of a pipe, and also to prove that a process cannot write to a pipe unless
// it has at least PIPE_BUF bytes available.

#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int count = 0;
sig_atomic_t full = 0;

/**
 * The SIGALRM handler. This sets the full flag to indicate that the
 * write call blocked, and it prints the number of aharacters written to
 * pipe so far.
 */
void on_alarm(int signo) {
  printf("\nwrite() blocked with %d chars in the pipe.\n", count);
  full = 1;
}

int main(int argc, char *argv[]) {
  int fd[2];
  int pipe_size;
  int bytes_read;
  int amount_to_remove;
  char buffer[PIPE_BUF];
  char c = 'x';
  static struct sigaction sig_action;

  sig_action.sa_handler = on_alarm;
  sigfillset(&(sig_action.sa_mask));
  sigaction(SIGALRM, &sig_action, NULL);

  if (pipe(fd) == -1) {
    perror("pipe failed");
    exit(1);
  }

  // Check whether the _PC_PIPE_BUF constnat returns the pipe capacity
  // or the atomic write size
  pipe_size = fpathconf(fd[0], _PC_PIPE_BUF);

  while (1) {
    // Set an alarm long enough that if write fails it will fail
    // within this amount of time. 8 seconds is long enough.
    alarm(4);
    write(fd[1], &c, 1);
    // Unset the alarm
    alarm(0);

    // Did alarm expire? If so, write failed and we stop the loop
    if (full) {
      break;
    }

    // Report how many chars written so far
    if ((++count % 1024) == 0) {
      printf("%d chars in pipe.\n", count);
    }
  }

  printf("The maximum number of bytes that the pipe stored is %d.\n", count);
  printf(
      "Now we remove characters from the pipe and demonstrate that"
      " we cannot\n"
      "write into the pipe unless it has %d (PIPE_BUF) free bytes.\n",
      PIPE_BUF);

  amount_to_remove = PIPE_BUF - 1;
  printf(
      "First we remove %d characters (PIPE_BUF - 1) and try to "
      "write into the pipe.\n",
      amount_to_remove);

  full = 0;
  bytes_read = read(fd[0], &buffer, amount_to_remove);
  if (bytes_read < 0) {
    perror("error reading pipe");
    exit(1);
  }
  count = count - bytes_read;
  alarm(4);
  write(fd[1], &c, 1);
  // Unset the alarm
  alarm(0);
  if (full) {
    printf("We could not write into the pipe.\n");
  } else {
    printf("We successfully wrote into the pipe.\n");
  }
  amount_to_remove = PIPE_BUF - amount_to_remove;
  full = 0;

  printf(
      "\nNow we remove one more character and try to "
      "write into the pipe.\n");

  bytes_read = read(fd[0], &buffer, amount_to_remove);
  if (bytes_read < 0) {
    perror("error reading pipe");
    exit(1);
  }
  count = count - bytes_read;
  alarm(4);
  write(fd[1], &c, 1);
  // Unset the alarm
  alarm(0);
  if (full) {
    printf("We could not write into the pipe.\n");
  } else {
    printf("We successfully wrote into the pipe.\n");
  }
  return 0;
}

// Chapter 5 Interactive Programs and Signals
// This program can be run to demonstrate using sigprocmask() system call to
// block or unblock signals sent to a process.
// If you type Ctrl C during the first loop, the process will continue to loop
// and it will exit before the second loop is executed. If you change the
// SIG_BLOCK to SIG_UNBLOCK then the Ctrl C will kill the process when you
// type it.

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  int i;
  sigset_t sigs;
  sigset_t prev_sigs;

  sigemptyset(&sigs);
  sigaddset(&sigs, SIGINT);
  sigprocmask(SIG_BLOCK, &sigs, &prev_sigs);

  for (i = 0; i < 5; i++) {
    printf("Waiting %d\n", i);
    sleep(1);
  }

  sigprocmask(SIG_SETMASK, &prev_sigs, NULL);

  for (i = 0; i < 5; i++) {
    printf("After %d\n", i);
    sleep(1);
  }

  return 0;
}

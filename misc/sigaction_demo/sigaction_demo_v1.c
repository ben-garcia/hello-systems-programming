// Chapter 5 Interactive Programs and Signals
// This program shows how SA_SIGINFO flag can be used. page 38

#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void sig_handler(int signo, siginfo_t *info, void *context) {
  printf("Signal number %d\n", info->si_signo);
  printf("Error number %d\n", info->si_errno);
  printf("PID sender %d\n", info->si_pid);
  printf("PID sender %d\n", info->si_pid);
  exit(1);
}

int main(int argc, char *argv[]) {
  struct sigaction the_action;

  the_action.sa_flags = SA_SIGINFO;
  the_action.sa_sigaction = sig_handler;

  sigaction(SIGINT, &the_action, NULL);

  printf("Type Ctrl C within the next minute or send signal 2. \n");
  sleep(60);

  return 0;
}

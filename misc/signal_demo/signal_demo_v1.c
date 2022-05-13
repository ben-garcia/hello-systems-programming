// Chapter 5 Interactive Programs and Signals
// simple program that installs signal handlers for SiGIN and SIGQUIT
// using signal() on  page 33

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void catch1(int signum) {
  printf("You can do better than that!\n");
}

void catch2(int signum) {
  printf("I'm no quitter!\n");
}

int main() {
  int i;

  signal(SIGINT, catch1);
  signal(SIGQUIT, catch2);

  for (i = 20; i > 0; i--) {
    printf("Try to kill me with ^C or ^\\.\n");
    sleep(1);
  }

  return 0;
}

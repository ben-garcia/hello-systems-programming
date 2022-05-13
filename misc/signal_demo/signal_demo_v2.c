// Chapter 5 Interactive Programs and Signals
// simple program that ignores signals for SiGIN and SIGQUIT
// using signal() on  page 33

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main() {
  int i;

  signal(SIGINT, SIG_IGN); // ignore Ctrl+C
  signal(SIGQUIT, SIG_IGN); // ignore Ctrl+\

  for (i = 20; i > 0; i--) {
    printf("Try to kill me with ^C or ^\\.\n");
    sleep(1);
  }

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int N = 4;

int main(int argc, char *argv[]) {
  int i;

  printf("About to create many processes...\n");

  for (i = 0; i < N; i++) {
    if (fork() == -1) {
      exit(1);
    }
  }

  printf("Process id = %d\n", getpid());
  fflush(stdout);  // force output before shell prompt
  sleep(1);        // give time to the shell to display prompt

  return 0;
}

// Chapter 7 Process Architecture and Control
// The following program shows how atexit() can be used to register
// a few exit functions.
// If more than one fucntion is registerd, they are run in the reverse order
// of the order in which they were registered last in first out(LIFO)
// on page 33

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void worker() { printf("worker #1: Finished for the day.\n"); }
void foreman() { printf("foreman: workers can stop for the day.\n"); }
void boss() { printf("first boss: forman, tell all workers can stop.\n"); }

int main() {
  long max_exit_functions = sysconf(_SC_ATEXIT_MAX);

  printf("Maximum number of exit functions is %ld\n", max_exit_functions);

  if ((atexit(worker)) != 0) {
    fprintf(stderr, "cannot set exit function\n");
    return EXIT_FAILURE;
  }

  if ((atexit(foreman)) != 0) {
    fprintf(stderr, "cannot set exit function\n");
    return EXIT_FAILURE;
  }

  if ((atexit(boss)) != 0) {
    fprintf(stderr, "cannot set exit function\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;

}


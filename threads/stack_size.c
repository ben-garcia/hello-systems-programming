// Chapter 10 Threads
// Program that prints the default stack size then set the new stack size
// based on a command line argument, and from within the thread, displays
// the actual stack size it is uing, using the GNU pthread_getattr_np()
// function. To save space, some error checking has been removed.

#define _GNU_SOURCE  // to get pthread_getattr_np() declaration
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *thread_start(void *arg) {
  size_t stack_size;
  pthread_attr_t gattr;

  pthread_getattr_np(pthread_self(), &gattr);
  pthread_attr_getstacksize(&gattr, &stack_size);
  printf("Actual stack size is %ld\n", stack_size);
  pthread_exit(0);
}

int main(int argc, char *argv[]) {
  pthread_t thr;
  pthread_attr_t attr;
  int ret_val;
  size_t new_stack_size;
  size_t stack_size;
  void *sp;

  if (argc < 2) {
    printf("usage: %s stacksize\n", argv[0]);
    exit(1);
  }

  new_stack_size = strtoul(argv[1], NULL, 0);

  ret_val = pthread_attr_init(&attr);
  if (ret_val) {
    exit(1);
  }

  pthread_attr_getstacksize(&attr, &stack_size);
  printf("Default stack size = %ld\n", stack_size);
  printf("New stack size will be %ld\n", new_stack_size);

  ret_val = pthread_attr_setstacksize(&attr, new_stack_size);
  if (ret_val) {
    exit(1);
  }

  ret_val = pthread_create(&thr, &attr, &thread_start, NULL);
  if (ret_val) {
    exit(1);
  }

  pthread_join(thr, NULL);

  return 0;
}

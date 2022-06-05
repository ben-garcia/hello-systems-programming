// Chapter 10 Threads
// Example of a detached child
// the thread doesn't stop simple because the main has exited.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *thread_routine(void *arg) {
  int i;
  int buf_size = strlen(arg);
  int fd = 1;

  printf("Child is running...\n");

  for (i = 0; i < buf_size; i++) {
    usleep(500000);
    write(fd, arg + i, 1);
  }

  printf("\nChild is now exiting.\n");

  return NULL;
}

int main(int argc, char *argv[]) {
  char *buf = "abcdefghijklmnopqrstuvwxyz";
  pthread_t thread;
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  if (pthread_create(&thread, NULL, thread_routine, (void *)(buf))) {
    fprintf(stderr, "error creating a new thread \n");
    exit(1);
  }

  printf("Main is now exiting.\n");
  pthread_exit(NULL);
}

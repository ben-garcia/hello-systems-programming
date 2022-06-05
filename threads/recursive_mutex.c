// Chapter 10 Threads
// A Program that uses recursive mutex.
// It doesn't do anything other than print some diagnostic messages.
// no page 25

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 5  // fixed number of threads

pthread_mutex_t mutex;
int counter = 0;

void bar(int tid) {
  pthread_mutex_lock(&mutex);
  printf("Thread %d: In far(); mutex locked\n", tid);
  counter++;
  printf("Thread %d: In far(); counter = %d\n", tid, counter);
  pthread_mutex_unlock(&mutex);
  printf("Thread %d: In far(); mutex unlocked\n", tid);
}

void foo(int tid) {
  pthread_mutex_lock(&mutex);
  printf("Thread %d: In foo(); mutex locked\n", tid);
  counter++;
  printf("Thread %d: In foo(); counter = %d\n", tid, counter);
  bar(tid);
  pthread_mutex_unlock(&mutex);
  printf("Thread %d: In foo(); mutex unlocked\n", tid);
}

void *thread_routine(void *data) {
  int t = (int)data;
  foo(t);
  pthread_exit(NULL);
}

/******************************************************************************/
/*                          Main Programe                                     */
/******************************************************************************/
int main(int argc, char *argv[]) {
  int ret_val;
  int t;
  pthread_t threads[NUM_THREADS];
  pthread_mutexattr_t attr;

  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&mutex, &attr);

  // initialize task_data for each thread and then create the thread
  for (t = 0; t < NUM_THREADS; t++) {
    if ((pthread_create(&threads[t], NULL, thread_routine, (void *)t)) != 0) {
      perror("Creating thread");
      exit(EXIT_FAILURE);
    }
  }

  for (t = 0; t < NUM_THREADS; t++) {
    pthread_join(threads[t], (void **)NULL);
  }

  return 0;
}

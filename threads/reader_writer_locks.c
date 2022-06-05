// Chapter 10 Threads
// This program demonstrates the sue of reader/writer locks. Tis would be very
// simple if we did not attemp to prevent starvation, either of readers or
// writer. Is uses barrier symchronization to ensure that no thread enters its
// main loop until all threads have at least been created. Without the barrier,
// the threads that are created first in the main program will always get the
// lock first, and is these are writers, the readers will starve.
// If the number of writers is changed to be greater than on e, they will
// starve the readers whenever the first writer grabs the lock. This is
// because writers are given proprity oever readers in the code below.
// on page 44

#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/******************************************************************************/
/*                          Data Types and Constnats                          */
/******************************************************************************/

#define NUM_READERS 10
#define NUM_WRITERS 1

pthread_rwlock_t rw_lock;   // the reader/writer lock
pthread_barrier_t barrier;  // to try to improve fairness

int done;                 // to terminate all threads
int num_threads_in_lock;  // for the monitor code

/******************************************************************************/
/*                          Thread and Helper Functions                       */
/******************************************************************************/

/**
 * handle_error(num, msg)
 *
 * Print to standard error the system message associated with error number num
 * as well as a custom message, and then exit the program with EXIT_FAILURE
 */
void handle_error(int num, char *msg) {
  errno = num;
  perror(msg);
  exit(EXIT_FAILURE);
}

/**
 * reader()
 *
 * A reader repeatedly gets the lock, sleps a bit, and then realeases the lock,
 * until done becomes true.
 */
void *reader(void *data) {
  int rc;
  int t = (int)data;

  // wait here until all threads are created
  rc = pthread_barrier_wait(&barrier);
  if (rc != PTHREAD_BARRIER_SERIAL_THREAD && rc != 0) {
    handle_error(rc, "pthread_barrier_wait");
  }

  // repeat until user says to quit
  while (!done) {
    rc = pthread_rwlock_rdlock(&rw_lock);
    if (rc) {
      handle_error(rc, "pthread_rwlock_rdlock");
    }
    printf("Reader %d got the read lock\n", t);
    sleep(1);
    rc = pthread_rwlock_unlock(&rw_lock);
    if (rc) {
      handle_error(rc, "pthread_rwlock_unlock");
    }
    sleep(1);
  }
  pthread_exit(NULL);
}

/**
 * writer()
 *
 * A writer does the same thing as a reader -- it repeatedly gets the lock,
 * sleeps a bit, and then releases the lock, until done becomes true.
 */
void *writer(void *data) {
  int rc;
  int t = (int)data;

  // wait here until all threads are created
  rc = pthread_barrier_wait(&barrier);
  if (rc != PTHREAD_BARRIER_SERIAL_THREAD && rc != 0) {
    handle_error(rc, "pthread_barrier_wait");
  }

  // repeat until user says to quit
  while (!done) {
    rc = pthread_rwlock_rdlock(&rw_lock);
    if (rc) {
      handle_error(rc, "pthread_rwlock_rdlock");
    }
    printf("Writer %d got the read lock\n", t);
    sleep(2);
    rc = pthread_rwlock_unlock(&rw_lock);
    if (rc) {
      handle_error(rc, "pthread_rwlock_unlock");
    }
    sleep(2);
  }
  pthread_exit(NULL);
}

/******************************************************************************/
/*                          Main Program                                      */
/******************************************************************************/

int main(int argc, char *argv[]) {
  pthread_t threads[NUM_READERS + NUM_WRITERS];
  int ret_val;
  int t;
  unsigned int num_threads = NUM_READERS + NUM_WRITERS;

  done = 0;
  printf(
      "This program will start up a number of threads that wil run \n"
      "until you enter a character. Type any character(followed by<Enter>) to "
      "quit\n");

  pthread_rwlockattr_t rwlock_attributes;
  pthread_rwlockattr_init(&rwlock_attributes);

  // The following non-portable function is a GNU extension that alters the
  // thread priorities when raders and writers are both waiting on a rwlock,
  // giving preference to writers.
  pthread_rwlockattr_setkind_np(&rwlock_attributes,
                                PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
  pthread_rwlock_init(&rw_lock, &rwlock_attributes);

  // inititalize a barrier with a ocunt equal to the number of threads
  ret_val = pthread_barrier_init(&barrier, NULL, num_threads);
  if (ret_val) {
    handle_error(ret_val, "pthread_barrier_init");
  }

  for (t = 0; t < NUM_READERS; t++) {
    ret_val = pthread_create(&threads[t], NULL, reader, (void *)t);
    if (ret_val) {
      handle_error(ret_val, "pthread_create");
    }
  }

  for (t = NUM_READERS; t < NUM_READERS + NUM_WRITERS; t++) {
    ret_val = pthread_create(&threads[t], NULL, writer, (void *)t);
    if (ret_val) {
      handle_error(ret_val, "pthread_create");
    }
  }

  getchar();
  done = 1;

  for (t = 0; t < NUM_READERS + NUM_WRITERS; t++) {
    pthread_join(threads[t], NULL);
  }

  return 0;
}

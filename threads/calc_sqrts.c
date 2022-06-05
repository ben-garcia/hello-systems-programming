// Chapter 10 Threads
// A multi-threaded program to compute the first N square roots.

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_THREADS 20                        // number of threads
#define NUMS_PER_THREAD 50                    // number of roots per thread
#define SIZE (NUM_THREADS * NUMS_PER_THREAD)  // total roots to compute

// Declare a structrue to pass multiple variables to the threads in the
// pthread_create() function and for the thread routing to access in its
// single argument.
typedef struct _thread_data {
  int count;            // shared counter, incremented by each thread
  int size;             // length of the roots array
  int nums_per_thread;  // number of roots computed by each thread
  double *roots;        // pointer to the roots array
} thread_data;

pthread_mutex_t update_mutex;  // declare a global mutex

/******************************************************************************/
/*                          Thread and Helper Functions                       */
/******************************************************************************/

/**
 * handler_erro(num, msgge)
 * A convenient error handling function
 * Prints to standard error the system message associated with errno num
 * as well as a custom message, and then exits the program with EXIT_FAILURE
 */
void handler_error(int num, char *msg) {
  errno = num;
  perror(msg);
  exit(EXIT_FAILURE);
}

/**
 * calc_square_roots()
 * A thread routing that calculates then square roots of N integers and stores
 * them in an array. The integers are not necesssarily consecutive;
 * as it depends how the threads are scheduled.
 * @param [out] double data->roots[] is the array in which to store the roots
 * @param [inout] int data->count is the rirst integer whose root should be
 *                                calculated
 * This increments data->count N times
 *
 * Loops to waste time a bit so that the threads may be sceduled out of order.
 */
void *calc_square_roots(void *data) {
  int i;
  int j;
  int temp;
  int size;
  int nums_to_compute;
  thread_data *t_data = (thread_data *)data;

  size = t_data->size;
  nums_to_compute = t_data->nums_per_thread;

  for (i = 0; i < nums_to_compute; i++) {
    pthread_mutex_lock(&update_mutex);  // lock mutex
    temp = t_data->count;
    t_data->count = temp + 1;
    pthread_mutex_unlock(&update_mutex);  // unlock mutex

    // updating the array can be done outisde the CS since temp is a local
    // variable to the thread.
    t_data->roots[temp] = sqrt(temp);

    // idle loop
    for (j = 0; j < 1000; j++)
      ;
  }
  pthread_exit(NULL);
}

/**
 * compute_roots()
 * computes the square roots of teh first num_thrads * roots-per_thread many
 * integrers. It hides the fact that it uses multiple threads to do this.
 */
void compute_roots(double sqrts[], int size, int num_threads) {
  pthread_t threads[num_threads];
  int t;
  int ret_val;
  static thread_data t_data;

  t_data.count = 0;
  t_data.size = size;
  t_data.nums_per_thread = size / num_threads;
  t_data.roots = &sqrts[0];

  // initialize the mutex
  pthread_mutex_init(&update_mutex, NULL);

  // initialize task_data for each thread and then create the thread
  for (t = 0; t < num_threads; t++) {
    ret_val =
        pthread_create(&threads[t], NULL, calc_square_roots, (void *)&t_data);

    if (ret_val) {
      handler_error(ret_val, "pthread_create");
    }
  }

  // join all threads and then print sum
  for (t = 0; t < num_threads; t++) {
    pthread_join(threads[t], (void **)NULL);
  }
}

/******************************************************************************/
/*                               Main Program                                 */
/******************************************************************************/

int main(int argc, char *argv[]) {
  int t;
  double roots[SIZE];

  memset((void *)&roots[0], 0, SIZE * sizeof(double));
  compute_roots(roots, SIZE, NUM_THREADS);

  for (t = 0; t < SIZE; t++) {
    printf("Square root of %5d is %6.3f\n", t, roots[t]);
  }

  return 0;
}

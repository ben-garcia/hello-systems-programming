// Chapter 10 Threads
// Reduction algorithm with barrier synchronization
// on page 37

#include <libintl.h>
#include <locale.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************/
/*                          Data Types adn Constants                          */
/******************************************************************************/
double *sum;      // array of partial sums of data
double *array;    // dynamically allocated araray of data
int num_threads;  // number of threads this program will use
pthread_barrier_t barrier;

// a task_data structure contains the data required for a thread to compute
// the sum of the segment of the array it has been delegated to total, storing
// the sum in its cell in an array of sums. The data array and the sum array
// are allocated on the heap. The threads get the starting addresses of each,
// and their task number and the first and last entries of their segments.
typedef struct _task_data {
  int first;    // index of first element for task
  int last;     // index of last element for task
  int task_id;  // id of thread
} task_data;

/******************************************************************************/
/*                          Thread and Helper Functions                       */
/******************************************************************************/

// print usage statement
void usage(char *s) {
  char *p = strrchr(s, '/');
  fprintf(stderr, "usage: %s arraysize numthreads \n", p ? p + 1 : s);
}

// The thread routing
void *add_array(void *thread_data) {
  task_data *t_data;
  int k;
  int tid;
  int half;
  int ret_val;

  t_data = (task_data *)thread_data;
  tid = t_data->task_id;

  sum[tid] = 0;
  for (k = t_data->first; k <= t_data->last; k++) {
    sum[tid] += array[k];

    half = num_threads;
    while (half > 1) {
      ret_val = pthread_barrier_wait(&barrier);
      if (ret_val != PTHREAD_BARRIER_SERIAL_THREAD && ret_val != 0) {
        pthread_exit((void *)0);
      }

      if (half % 2 == 1 && tid == 0) {
        sum[0] = sum[0] + sum[half - 1];
      }

      half = half / 2;  // integer division
      if (tid < half) {
        sum[tid] = sum[tid] + sum[tid + half];
      }
    }
  }
  pthread_exit((void *)0);
}

/******************************************************************************/
/*                                Main                                        */
/******************************************************************************/
int main(int argc, char *argv[]) {
  int array_size;
  int size;
  int k;
  int ret_val;
  int t;
  pthread_t *threads;
  task_data *thread_data;
  pthread_attr_t attr;

  // Instead of asuming that the system creates threads as joinalbe by default,
  // this sets them to be joinalbe explicitly
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  if (argc < 3) {
    usage(argv[0]);
    exit(1);
  }

  // Get command line arguments, convert to ints, and compute size of each
  // thread's segment of the array
  array_size = atoi(argv[1]);
  num_threads = atoi(argv[2]);
  size = (int)ceil(array_size * 1.0 / num_threads);

  // allocate the array of threads, task_data structures, data and sums
  threads = calloc(num_threads, sizeof(pthread_t));
  thread_data = calloc(num_threads, sizeof(task_data));
  array = calloc(array_size, sizeof(double));
  sum = calloc(num_threads, sizeof(double));

  if (threads == NULL || thread_data == NULL || array == NULL || sum == NULL) {
    exit(1);
  }

  // synthesize array data here
  for (k = 0; k < array_size; k++) {
    array[k] = (double)k;
  }

  // initalize a berrier with a count equal to the number of threads
  pthread_barrier_init(&barrier, NULL, num_threads);

  // initialize task_data for each thread and then create the thread
  for (t = 0; t < num_threads; t++) {
    thread_data[t].first = t * size;
    thread_data[t].last = (t + 1) * size - 1;
    if (thread_data[t].last > array_size - 1) {
      thread_data[t].last = array_size - 1;
    }
    thread_data[t].task_id = t;

    ret_val =
        pthread_create(&threads[t], &attr, add_array, (void *)&thread_data[t]);
    if (ret_val) {
      printf("ERROR; return code from pthread_create() is %d\n", ret_val);
      exit(1);
    }
  }

  // join all threads so that we can add up their partial sums
  for (t = 0; t < num_threads; t++) {
    pthread_join(threads[t], (void **)NULL);
  }

  pthread_barrier_destroy(&barrier);

  printf("The array total is %7.2f\n", sum[0]);

  // Free all memeory allocated to program
  free(threads);
  free(thread_data);
  free(array);
  free(sum);

  return 0;
}

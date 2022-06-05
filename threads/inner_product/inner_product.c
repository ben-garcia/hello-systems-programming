// Chapter 10 Threads
// Mutex example that computes the inner product of two vectors
#include <errno.h>
#include <libintl.h>
#include <locale.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 20

typedef struct _task_data {
  int first;
  int last;
  double *a;
  double *b;
  double *sum;
} task_data;

pthread_mutex_t mutex_sum;  // declare the mutex globally

/******************************************************************************/
/*                          Thread and Helper Functions                       */
/******************************************************************************/
void usage(char *s) {
  char *p = strchr(s, '/');
  fprintf(stderr, "usage: %s length datafile1 datafile2 \n", p ? p + 1 : s);
}

void handler_error(int num, char *msg) {
  errno = num;
  perror(msg);
  exit(EXIT_FAILURE);
}

/** This function computes the inner product of the sub-vectors
 * thread_data->[first..last] and thread_data->b[first..last],
 * adding that sum to thread_data->sum within the critical section
 * protected by the shared mutex.
 */
void *inner_product(void *thread_data) {
  task_data *t_data;
  int k;
  double temp_sum = 0;

  t_data = (task_data *)thread_data;

  for (k = t_data->first; k <= t_data->last; k++) {
    temp_sum += t_data->a[k] * t_data->b[k];
  }

  pthread_mutex_lock(&mutex_sum);
  *(t_data->sum) += temp_sum;
  pthread_mutex_unlock(&mutex_sum);

  pthread_exit((void *)0);
}

/******************************************************************************/
/*                                   Main Program                             */
/******************************************************************************/
int main(int argc, char *argv[]) {
  static double *a_vector;
  static double *b_vector;
  FILE *fp;
  float x;
  int num_threads = NUM_THREADS;
  int length;
  int segment_size;
  static double total;
  int k;
  int ret_val;
  int t;
  pthread_t *threads;
  task_data *thread_data;
  pthread_attr_t attr;

  if (argc < 4) {  // check usage
    usage(argv[0]);
    exit(1);
  }

  // get command line args, no input validation here
  length = atoi(argv[1]);
  a_vector = calloc(length, sizeof(double));
  b_vector = calloc(length, sizeof(double));

  // zero the two vectors
  // memset(a_vector, 0, length * sizeof(double));
  // memset(b_vector, 0, length * sizeof(double));

  // open the first file, do check for failure and read the numbers
  // from the file. Assume that it is in proper format
  if ((fp = fopen(argv[2], "r")) == NULL) {
    handler_error(errno, "fopen");
  }

  k = 0;
  // fill a_vector with the float numbers provided by argv[1]
  while ((fscanf(fp, " %f ", &x) > 0) && (k < length)) {
    a_vector[k++] = x;
  }
  fclose(fp);  // done with file

  // open the second file, do check for failure and read the numbers
  // from the file. Assume that it is in proper format
  if ((fp = fopen(argv[3], "r")) == NULL) {
    handler_error(errno, "fopen");
  }

  k = 0;
  // fill b_vector with the float numbers provided by argv[2]
  while ((fscanf(fp, " %f ", &x) > 0) && (k < length)) {
    b_vector[k++] = x;
  }
  fclose(fp);  // done with file

  // allocate the array of threads and task_data structures
  threads = calloc(num_threads, sizeof(pthread_t));
  thread_data = calloc(num_threads, sizeof(task_data));

  if (threads == NULL || thread_data == NULL) {
    exit(1);
  }

  // compute the size each thread will get
  segment_size = (int)ceil(length * 1.0 / num_threads);

  // initilize the mutex
  pthread_mutex_init(&mutex_sum, NULL);

  // get ready -- initialize the thread atributes
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // initialize thread_data for each thread and then create the thread
  for (t = 0; t < num_threads; t++) {
    thread_data[t].first = t * segment_size;
    thread_data[t].last = (t + 1) * segment_size - 1;

    if (thread_data[t].last > length - 1) {
      thread_data[t].last = length - 1;
    }

    thread_data[t].a = &a_vector[0];
    thread_data[t].b = &b_vector[0];
    thread_data[t].sum = &total;

    ret_val = pthread_create(&threads[t], &attr, inner_product,
                             (void *)&thread_data[t]);

    if (ret_val) {
      handler_error(ret_val, "pthread_create");
    }
  }

  // join all threads and print sum
  for (t = 0; t < num_threads; t++) {
    pthread_join(threads[t], (void **)NULL);
  }

  printf("The array total is %8.2f\n", total);

  // free all memory allocated to program
  free(threads);
  free(thread_data);
  free(a_vector);
  free(b_vector);

  return 0;
}

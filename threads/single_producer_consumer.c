// Chapter 10 Threads
// A multi-threaded solution to the single-producer/single-consudmer problem
// that uses a mutex and two condition variables. For simplicity, it is
// designed to terminate after a fixed number of iterations of each thread.

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>

/******************************************************************************/
/*                          Global, Shared Data                               */
/******************************************************************************/

#define NUM_ITERATIONS 500  // number of loop each thread iterates
#define BUFFER_SIZE 20      // size of buffer

// buffer_mutex controls buffer accesss
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

// space available is a condition that is true when the buffer is not full
pthread_cond_t space_available = PTHREAD_COND_INITIALIZER;

// data available is a condition that is true when the buffer is not empty
pthread_cond_t data_available = PTHREAD_COND_INITIALIZER;

int producer_exists;  // true when producer is still running
FILE *fp;             // log file pointer for messages

/******************************************************************************/
/*                          Buffer Object                                     */
/******************************************************************************/

int buffer[BUFFER_SIZE];  // the buffer of data -- just ints here
int buf_size;             // number of filled slots in buffer

void add_buffer(int data) {
  static int rear = 0;
  buffer[rear] = data;
  rear = (rear + 1) % BUFFER_SIZE;
  buf_size++;
}

int get_buffer() {
  static int front = 0;
  int i;
  i = buffer[front];
  front = (front + 1) % BUFFER_SIZE;
  buf_size--;
  return i;
}

/******************************************************************************/
/*                          Error Handling Functions                          */
/******************************************************************************/

void handle_error(int num, char *msg) {
  errno = num;
  perror(msg);
  exit(EXIT_FAILURE);
}

/******************************************************************************/
/*                          Thread Functions                                  */
/******************************************************************************/

void *producer(void *data) {
  int i;
  for (i = 1; i <= NUM_ITERATIONS; i++) {
    pthread_mutex_lock(&buffer_mutex);
    while (buf_size == BUFFER_SIZE) {
      pthread_cond_wait(&space_available, &buffer_mutex);
    }
    add_buffer(i);
    fprintf(fp, "Producer added %d to buffer; buffer size = %d.\n", i,
            buf_size);
    pthread_cond_signal(&data_available);
    pthread_mutex_unlock(&buffer_mutex);
  }
  pthread_mutex_lock(&buffer_mutex);
  producer_exists = 0;
  pthread_cond_signal(&data_available);
  pthread_mutex_unlock(&buffer_mutex);

  pthread_exit(NULL);
}

void *consumer(void *data) {
  int i;
  for (i = 1; i < NUM_ITERATIONS; i++) {
    pthread_mutex_lock(&buffer_mutex);
    while (buf_size == 0) {
      if (producer_exists) {
        pthread_cond_wait(&data_available, &buffer_mutex);
      } else {
        pthread_mutex_unlock(&buffer_mutex);
        pthread_exit(NULL);
      }
    }
    i = get_buffer();
    fprintf(fp, "Consumer got data element %d; buffer size = %d.\n", i,
            buf_size);
    pthread_cond_signal(&space_available);
    pthread_mutex_unlock(&buffer_mutex);
  }
  pthread_exit(NULL);
}

/******************************************************************************/
/*                          Main Program                                      */
/******************************************************************************/

int main(int argc, char *argv[]) {
  pthread_t producer_thread;
  pthread_t consumer_thread;

  producer_exists = 1;
  buf_size = 0;

  if ((fp = fopen("./prodcons_mssges", "w")) == NULL) {
    handle_error(errno, "prodcons_mssges");
  }

  pthread_create(&consumer_thread, NULL, consumer, NULL);
  pthread_create(&producer_thread, NULL, producer, NULL);

  pthread_join(producer_thread, NULL);
  pthread_join(consumer_thread, NULL);

  fclose(fp);
  return 0;
}

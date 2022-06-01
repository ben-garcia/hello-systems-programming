// Chapter 8 Interprocess Communication, Part 1
// The following program is designed to demonstrate (but not prove) that writes
// of up to PIPE_BUF bytes are atomic, and that larger writes may not be
// atomic. It also demonstrates how to create multiple writers and a single
// reader.
// on page 8

#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_FD 0
#define WRITE_FD 1
#define RD_CHUNK 10
#define ATOMIC  // this flag determines who large the chunk is

#ifndef PIPE_BUF
#define PIPE_BUF POSIX_PIPE_BUF
#endif

void do_nothing(int signo) { return; }

int main(int argc, char *argv[]) {
  int i;
  int repeat;
  int bytes_read;
  int message_length;
  pid_t child1_pid;
  pid_t child2_pid;
  int fd[2];
  int out_fd;
  char message[RD_CHUNK + 1];
  char *child1_chunk;
  char *child2_chunk;
  long chunk_size;
  static struct sigaction sig_act;

  sig_act.sa_handler = do_nothing;
  sigfillset(&(sig_act.sa_mask));
  sigaction(SIGUSR1, &sig_act, NULL);

  // check proper usage
  if (argc < 2) {
    fprintf(stderr, "usage: %s size\n", argv[0]);
    exit(1);
  }

  // try to create pipe
  if (pipe(fd) == -1) {
    perror("pipe call");
    exit(2);
  }

  repeat = atoi(argv[1]);
#if defined ATOMIC
  chunk_size = PIPE_BUF;
#else
  chunk_size = PIPE_BUF + 200;
#endif

  printf("Chunk size = %ld\n", chunk_size);
  printf("Value of PIPE_BUF is %d\n", PIPE_BUF);

  child1_chunk = calloc(chunk_size, sizeof(char));
  child2_chunk = calloc(chunk_size, sizeof(char));
  if ((child1_chunk == NULL) || (child2_chunk == NULL)) {
    perror("calloc");
    exit(2);
  }

  // create the string that child1 writes
  child1_chunk[0] = '\0';  // just to be safe
  for (i = 0; i < chunk_size - 2; i++) {
    strcat(child1_chunk, "X");
  }
  strcat(child1_chunk, "\n");

  // create the string that child2 writes
  child2_chunk[0] = '\0';  // just to be safe
  for (i = 0; i < chunk_size - 2; i++) {
    strcat(child2_chunk, "y");
  }
  strcat(child2_chunk, "\n");

  // create first child process
  switch (child1_pid = fork()) {
    case -1:  // fork failed -- exit
      perror("fork()");
      exit(3);
    case 0:  // child1 code
      message_length = strlen(child1_chunk);
      pause();  // wait for signal before continuing
      for (i = 0; i < repeat; i++) {
        if (write(fd[WRITE_FD], child1_chunk, message_length) !=
            message_length) {
          perror("write");
          exit(4);
        }
      }
      close(fd[WRITE_FD]);
      exit(0);
    default:  // parent creates second child process
      switch (child2_pid = fork()) {
        case -1:
          perror("fork()");
          exit(5);
        case 0:  // child2 code
          message_length = strlen(child2_chunk);
          pause();
          for (i = 0; i < repeat; i++) {
            if (write(fd[WRITE_FD], child2_chunk, message_length) !=
                message_length) {
              perror("write");
              exit(6);
            }
          }
          close(fd[WRITE_FD]);
          exit(0);
        default:  // parent code
          printf("Opening file...\n");
          out_fd = open("pd2_output", O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if (out_fd == -1) {
            perror("open");
            exit(7);
          }
          printf("File is ready to be written...\n");
          close(fd[WRITE_FD]);
          kill(child1_pid, SIGUSR1);  // send signal to child1
          kill(child2_pid, SIGUSR1);  // send signal to child2
          printf("Writing to file...\n");
          while ((bytes_read = read(fd[READ_FD], message, RD_CHUNK)) != 0) {
            if (bytes_read > 0) {  // more data
              write(out_fd, message, bytes_read);
            } else {  // read error
              perror("read()");
              exit(8);
            }
          }
          printf("Writing complete\n");
          printf("Closing file...\n");
          close(out_fd);
          printf("Closed file\n");
          // collect zombies
          for (i = 1; i <= 2; i++) {
            if (wait(NULL) == -1) {
              perror("wait failed");
              exit(9);
            }
          }
          close(fd[READ_FD]);
          free(child1_chunk);
          free(child2_chunk);
      }
      exit(0);
  }
}

// Chapter 8 Interprocess Communications Part 1
// server code
// on page 28

#include <signal.h>

#include "fifo_1.h"

int dummy_fifo;   // file descriptor to write-end of PUBLIC
int public_fifo;  // file descriptor to read-end of PUBLIC

/**
 * on_signal()
 * This closes both ends of the FIFO and then removes it, after
 * when it exits the program.
 */
void on_signal(int sig) {
  close(public_fifo);
  close(dummy_fifo);
  unlink(PUBLIC);
  exit(0);
}

int main(int argc, char *argv[]) {
  int n_bytes;  // number of bytes read from popen()
  int count = 0;
  static char buffer[PIPE_BUF];  // buffer to store output of command
  struct sigaction handler;      // sigaction for registering handlers

  // Register the signal handler to handler a few signals
  handler.sa_handler = on_signal;  // handler function
  handler.sa_flags = SA_RESTART;
  if (sigaction(SIGINT, &handler, NULL) == -1 ||
      sigaction(SIGHUP, &handler, NULL) == -1 ||
      sigaction(SIGQUIT, &handler, NULL) == -1 ||
      sigaction(SIGTERM, &handler, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  // create public FIFO. If it exists already, the call will return -1 and
  // set errno to EEXIST. This is not an error in our case. It just means
  // we can reuse an existing FIFO that we created but never removed. All
  // other errors cause the program to exit.
  if (mkfifo(PUBLIC, 0666) < 0) {
    if (errno != EEXIST) {
      perror(PUBLIC);
      exit(1);
    }
  }

  // We open the FIFO for reading, with the O_NONBLOCK flag clear. The POSIX
  // semantics state the process will be blocked on the open() until some
  // server (to be precise, some thread) opens it for writing. Therefore, the
  // server will be stuck in this open() until a client starts up.
  if ((public_fifo = open(PUBLIC, O_RDONLY)) == -1) {
    perror(PUBLIC);
    exit(1);
  }

  // We now open the FIFO for writing, even though we have no intention of
  // writing to the FIFO. We will not reach the call to open()
  // until a client runs, but once the clients runs, the server open the FIFO
  // for writing, If we do not do this, when the client terminates and closes
  // its write-end of the FIFO, the server's read loop would exit and the
  // server would also exit. This "dummy" open keeps the server alive.
  if ((dummy_fifo = open(PUBLIC, O_WRONLY | O_NONBLOCK)) == -1) {
    perror(PUBLIC);
    exit(1);
  }

  // Block waiting for a message from a client
  while (1) {
    memset(buffer, 0, PIPE_BUF);
    if ((n_bytes = read(public_fifo, buffer, PIPE_BUF)) > 0) {
      buffer[n_bytes] = '\0';
      printf("Message %d received by server: %s", ++count, buffer);
      fflush(stdout);
    } else {
      break;
    }
  }
  return 0;
}

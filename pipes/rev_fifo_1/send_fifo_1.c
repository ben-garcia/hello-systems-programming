// Chapter 8 Interprocess Communications Part 1
// client code
// on page 30

#include "fifo_1.h"

#define QUIT "quit"

int main(int argc, char *argv[]) {
  int n_bytes;      // number of bytes read
  int public_fifo;  // file descriptor to write-end of PUBLIC
  char text[PIPE_BUF];

  // open the public FIFO for writing
  if ((public_fifo = open(PUBLIC, O_WRONLY)) == -1) {
    perror(PUBLIC);
    exit(1);
  }

  printf("Type 'quit' to quit.\n");

  // repeatedly prompt user for command, read it, and send to server
  while (1) {
    memset(text, 0, PIPE_BUF);  // zero string
    n_bytes = read(fileno(stdin), text, PIPE_BUF);
    if (!strncmp(QUIT, text, n_bytes - 1)) {  // is it quit?
      break;
    }

    if (write(public_fifo, text, n_bytes) < 0) {
      perror("Server is no longer running");
      break;
    }
  }

  // user quit, so close write-end of public FIFO
  close(public_fifo);
  return 0;
}

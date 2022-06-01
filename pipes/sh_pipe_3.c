// Chapter 8 Interprocess Communications Part 1
// This program uses popen() and pclose() instead of the pipe(), fork(), dup()
// sequence
// on page 24

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int n_bytes;
  FILE *fin;              // read-end of pipe
  FILE *fout;             // write-end of pipe
  char buffer[PIPE_BUF];  // buffer for transferring data

  if (argc < 3) {
    fprintf(stderr, "usage: %s command1 command2\n", argv[0]);
    exit(1);
  }

  if ((fin = popen(argv[1], "r")) == NULL) {
    fprintf(stderr, "popen() failed\n");
    exit(1);
  }

  if ((fout = popen(argv[2], "w")) == NULL) {
    fprintf(stderr, "popen() failed\n");
    exit(1);
  }

  while ((n_bytes = read(fileno(fin), buffer, PIPE_BUF)) > 0) {
    write(fileno(fout), buffer, n_bytes);
  }

  pclose(fin);
  pclose(fout);
  return 0;
}

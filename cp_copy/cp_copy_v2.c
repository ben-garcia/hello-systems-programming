// Chapter 2 Login Records, File I/O, and Performance
// version 2 on page 39
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define COPY_MODE 0644

void die(char *string_1, char *string_2); // print error and quit

int main(int argc, char *argv[]) {
  int BUFFER_SIZE;
  char end_ptr[255];
  int source_fd;
  int target_fd;
  int n_chars;
  char *buffer;

  // need to check for 3 arguments instead of 2.
  if (argc != 4) {
    fprintf(stderr, "usage: %s buffersize source destination\n", *argv);
    exit(1);
  }

  // extract number from string.
  BUFFER_SIZE = strtol(argv[3], (char**)&end_ptr, 0);

  if (BUFFER_SIZE == 0) {
    fprintf(stderr, "usage: buffer size must be a number\n");
    exit(1);
  }

  // try to open file with read only persmission
  if ((source_fd = open(argv[1], O_RDONLY)) == -1) {
    die("Cannot open ", argv[1]);
  }

  // try to open file to copy
  // if the file already exists, the 'creat' function
  // will set it's length to 0.
  // if the file doesn't exist, it will create it.
  if ((target_fd = creat(argv[2], COPY_MODE)) == -1) {
    die("Cannot creat ", argv[2]);
  }

  buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));

  if (buffer == NULL) {
    fprintf(stderr, "Could not allocate memory for buffer.\n");
    exit(1);
  }

  // copy from source to target
  // 'read' function returns the number of bytes read
  // so when there is no more lines to read, it returns 0.
  while ((n_chars = read(source_fd, buffer, BUFFER_SIZE)) > 0) {
    if (n_chars != write(target_fd, buffer, n_chars)) {
      die("Write error to ", argv[2]);
    }
  }

  if (n_chars == -1) {
    die("Read error from ", argv[1]);
  }

  // close both files
  if (close(source_fd) == -1 || close(target_fd) == -1) {
    die("Error closing files", "");
  }

  return 0;
}

void die(char *string_1, char *string_2) {
  fprintf(stderr, "Error: %s ", string_1);
  perror(string_2);
  exit(1);
}

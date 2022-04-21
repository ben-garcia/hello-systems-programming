// Chapter 2 Login Records, File I/O, and Performance
// version 1 on page 36
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096 // maximum number read by the read() call.
#define COPY_MODE 0644

void die(char *string_1, char *string_2); // print error and quit

int main(int argc, char *argv[]) {
  int source_fd;
  int target_fd;
  int n_chars;
  char buf[BUFFER_SIZE];

  if (argc != 3) {
    fprintf(stderr, "usage: %s source destination\n", *argv);
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

  // copy from source to target
  // 'read' function returns the number of bytes read
  // so when there is no more lines to read, it returns 0.
  while ((n_chars = read(source_fd, buf, BUFFER_SIZE)) > 0) {
    if (n_chars != write(target_fd, buf, n_chars)) {
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

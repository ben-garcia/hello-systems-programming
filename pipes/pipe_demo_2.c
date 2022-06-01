// Chapter 8 Interprocess Communication, Part 1
// The following program is the first example of two-process communication
// through a pipe. The parrent process reads the command line arguments
// and sends them to the child process, wihch prints them on the screen.
// on page 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_FD 0
#define WRITE_FD 1

int main(int argc, char *argv[]) {
  int i;
  int bytes_read;
  int fd[2];
  char message[BUFSIZ];

  // check proper usage
  if (argc < 2) {
    fprintf(stderr, "usage: %s message\n", argv[0]);
    exit(1);
  }

  // try to create pipe
  if (pipe(fd) == -1) {
    perror("pipe call");
    exit(2);
  }

  // create child process
  switch (fork()) {
    case -1:
      // fork failed -- exit
      perror("fork()");
      exit(3);
    case 0:  // child code
      // close write end, otherwise child will never terminate
      close(fd[WRITE_FD]);
      // loop while not end of file or not a read error
      while ((bytes_read = read(fd[READ_FD], message, BUFSIZ)) != 0) {
        if (bytes_read > 0) {  // more data
          message[bytes_read] = '\0';
          printf("Child received the word: '%s'\n", message);
          fflush(stdout);
        } else {  // read error
          perror("read()");
          exit(4);
        }
      }
      exit(0);
    default:               // parent code
      close(fd[READ_FD]);  // close read end, since parent is writing
      for (i = 1; i < argc; i++) {
        // send each word separately
        if (write(fd[WRITE_FD], argv[i], strlen(argv[i])) != -1) {
          printf("Parent sent the word: '%s'\n", argv[i]);
          fflush(stdout);
        } else {
          perror("write()");
          exit(5);
        }
      }
      close(fd[WRITE_FD]);

      // wait for child so it does not remain a zombile
      // dont't care about it's status, so pass a NULL pointer
      if (wait(NULL) == -1) {
        perror("wait failed");
        exit(2);
      }
  }
  exit(0);
}

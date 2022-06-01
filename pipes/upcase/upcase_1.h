// Chapter 8 Interprocess Communications Part 1
// the message structure used by the server and client, as well as all
// neccessary include files and common definitions, are contained here.
// on page 32

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define PUBLIC "/tmp/UPCASE1_PIPE"
#define HALF_PIPE_BUF (PIPE_BUF / 2)

struct message {
  char fifo_name[HALF_PIPE_BUF];  // private FIFO pathname
  char text[HALF_PIPE_BUF];       // message text
};

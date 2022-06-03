// Chapter 9 Interprocess Communications Part 2
// This program is a simple concurrent server that, yes, once again, does lower
// to upper case conversion. This time it will handle just one character at
// a time.
// on page 7

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SOCKADDR (struct sockaddr *)
#define SIZE sizeof(struct sockaddr_in)
#define DEFAULT_HOST "localhost"
#define PORT 25555
#define ERROR_EXIT(MSSG, NUM) \
  perror(MSSG);               \
  exit(NUM);
#define MAXLINE 4096

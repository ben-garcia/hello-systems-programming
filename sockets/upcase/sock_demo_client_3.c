// Chapter 9 Interprocess Communications Part 2
// Socket version of the previous version
// on page 14

#include "sock_demo_1.h"

#define MAXFD(X, Y) ((X) > (Y) ? (X) : (Y))

int main(int argc, char *argv[]) {
  int sock_fd;
  char ip_name[256] = "";
  fd_set read_set;
  int max_fd;
  int n;
  char recv_line[MAXLINE];
  char send_line[MAXLINE];
  int s;
  int stdin_oef = 0;

  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *resulting_address;
  char port_str[20];

  // Check if there is a host name on command line;
  // if not use default
  if (argc < 2) {
    strcpy(ip_name, DEFAULT_HOST);
  } else {
    strcpy(ip_name, argv[1]);
  }

  printf("Searching for server %s\n", ip_name);

  // Initialize the hints addrinfo structure before calling
  // getaddrinfo(). This is used to define criteria to use
  // when it searches for a suitable host/port/service for
  // the client.
  memset(&hints, 0, sizeof(struct addrinfo));  // zero it out
  hints.ai_family = AF_UNSPEC;                 // allow IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;             // stream connection
  hints.ai_flags = 0;                          // no flags
  hints.ai_protocol = 0;                       // any protocol

  // convert numeric port to a string
  sprintf(port_str, "%d", PORT);

  // Get the network info; if non-zero return, there was an error
  if ((s = getaddrinfo(ip_name, port_str, &hints, &result)) != 0) {
    // call gai_strerror() to get string for error number
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  // Search through every adrinfo structure in the list pointed to by the
  // result pointer in the call to getaddrinfo(). These are the strctures
  // containing ossible family/socket-type/protocol combinations. The
  // structures are ordered within the list by the rules specified in the
  // RFC 3484 standard, so the first on efor which a socket can be created
  // is the one to use.
  resulting_address = result;
  while (resulting_address != NULL) {
    sock_fd =
        socket(resulting_address->ai_family, resulting_address->ai_socktype,
               resulting_address->ai_protocol);

    if (sock_fd == -1) {
      resulting_address = resulting_address->ai_next;
      continue;
    }

    if (connect(sock_fd, resulting_address->ai_addr,
                resulting_address->ai_addrlen) != -1) {
      break;  // success
    }

    close(sock_fd);
    resulting_address = resulting_address->ai_next;
  }

  if (resulting_address == NULL) {  // No address succeeded
    fprintf(stderr, "Could not connect\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);  // no longer needed

  printf("Connection made to server\n");

  max_fd = MAXFD(fileno(stdin), sock_fd) + 1;

  for (;;) {
    FD_ZERO(&read_set);
    if (stdin_oef == 0) {
      FD_SET(fileno(stdin), &read_set);
    }
    FD_SET(sock_fd, &read_set);
    if (select(max_fd, &read_set, NULL, NULL, NULL) > 0) {
      if (FD_ISSET(sock_fd, &read_set)) {
        if ((n = read(sock_fd, recv_line, MAXLINE - 1)) == 0) {
          if (stdin_oef == 1) {
            return 0;
          } else {
            ERROR_EXIT("Server terminated prematurely.", 1);
          }
        }
        recv_line[n] = '\0';
        fputs(recv_line, stdout);
      }

      if (FD_ISSET(fileno(stdin), &read_set)) {
        if (fgets(send_line, MAXLINE - 1, stdin) == NULL) {
          stdin_oef = 1;
          shutdown(sock_fd, SHUT_WR);
          FD_CLR(fileno(stdin), &read_set);
          continue;
        }
        write(sock_fd, send_line, strlen(send_line));
      }
    }
  }
  return 0;
}

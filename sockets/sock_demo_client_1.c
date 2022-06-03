// Chapter 9 Interprocess Communications Part 2
// on page 11

#include "sock_demo_1.h"

int main(int argc, char *argv[]) {
  int sock_fd;
  char c;
  char rc;
  char ip_name[256] = "";
  struct sockaddr_in server;
  struct hostent *host;

  if (argc < 2) {
    strcpy(ip_name, DEFAULT_HOST);
  } else {
    strcpy(ip_name, argv[1]);
  }

  if ((host = gethostbyname(ip_name)) == NULL) {
    ERROR_EXIT("gethostbyname", 1);
  }

  memset(&server, 0, sizeof(server));
  memcpy(&server.sin_addr, SOCKADDR * host->h_addr_list, SIZE);
  server.sin_family = AF_INET;
  server.sin_port = PORT;

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    ERROR_EXIT("socket call failed", 1);
  }

  if ((connect(sock_fd, SOCKADDR & server, sizeof(server))) == -1) {
    ERROR_EXIT(" connect call failed", 1);
  }

  for (rc = '\n';;) {
    if (rc == '\n') {
      printf("Input a lowercase character\n");
    }
    c = getchar();
    write(sock_fd, &c, 1);
    if ((read(sock_fd, &rc, 1)) > 0) {
      printf("%c", rc);
    } else {
      printf("server has died.\n");
      close(sock_fd);
      exit(1);
    }
  }
}

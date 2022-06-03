// Chapter 9 Interprocess Communication Part 2
// on page 17

#include <sys/wait.h>

#include "sock_demo_1.h"

#define LISTEN_QUEUE_SIZE 5

// the follwing typedef simplifies the function definition after it
typedef void Sigfunc(int);  // for signal handlers

// override existing signal function to handle non-BSD
Sigfunc *Signal(int signo, Sigfunc *func);

// Signal handlers
void on_sigchld(int signo);
void str_echo(int sock_fd);

int main(int argc, char *argv[]) {
  int listen_fd;
  int conn_fd;
  pid_t child_pid;
  socklen_t clilen;
  struct sockaddr_in client_addr;
  struct sockaddr_in server_addr;
  void sig_chld(int);

  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    ERROR_EXIT("socket call failed", 1);
  }

  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = PORT;

  if (bind(listen_fd, SOCKADDR & server_addr, sizeof(server_addr)) == -1) {
    ERROR_EXIT(" bind call failed", 1);
  }

  if (listen(listen_fd, LISTEN_QUEUE_SIZE) == -1) {
    ERROR_EXIT(" listen call failed", 1);
  }

  Signal(SIGCHLD, on_sigchld);

  for (;;) {
    clilen = sizeof(client_addr);
    if ((conn_fd = accept(listen_fd, SOCKADDR & client_addr, &clilen)) < 0) {
      if (errno == EINTR) {
        continue;  // back to for()
      } else {
        ERROR_EXIT("accept erro", 1);
      }
    }

    printf("before fork()\n");

    if ((child_pid = fork() == 0)) {  // child process
      close(listen_fd);               // close listening socket
      str_echo(conn_fd);              // process the request
      exit(0);
    }
    close(conn_fd);  // parent closes connection socket
  }
}

void on_sigchld(int signo) {
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    ;

  return;
}

Sigfunc *Signal(int signo, Sigfunc *func) {
  struct sigaction act;
  struct sigaction oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if (SIGALRM != signo) {
    act.sa_flags |= SA_RESTART;
  }

  if ((sigaction(signo, &act, &oact)) < 0) {
    return (SIG_ERR);
  }

  return (oact.sa_handler);
}

void str_echo(int sock_fd) {
  ssize_t n;
  int i;
  char line[MAXLINE];

  for (;;) {
    if ((n = read(sock_fd, line, MAXLINE - 1)) == 0) {
      return;  // connection closed by other end
    }

    for (i = 0; i < n; i++) {
      if (islower(line[i])) {
        line[i] = toupper(line[i]);
      }
    }
    write(sock_fd, line, n);
  }
}

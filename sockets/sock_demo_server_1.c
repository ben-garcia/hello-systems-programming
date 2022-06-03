// Chapter 9 Interprocess Communications Part 2
// on page 7

#include <sys/wait.h>

#include "sock_demo_1.h"

#define LISTEN_QUEUE_SIZE 5

// the following typedef simplifies the function definition after it
typedef void Sigfunc(int);  // for signal handlers

Sigfunc *Signal(int signo, Sigfunc *func);

// Signal handlers
void on_sigpipe(int signo);
void on_sigchld(int signo);

// This needs to be global because the signal handler has to access it
int connection_fd;

int main(int argc, char *argv[]) {
  int listen_fd;  // holds the file descriptor for the socket
  char c;         // this example is a ToUpcase server

  // A sockaddr_in struct is a struct that stores address and
  // port information for a socket for network communications.
  // The sockaddr_in IS the socket. In this case it is an Internet
  // socket (AF_INET) using port 7000, and accepting connections
  // on any network interface (INADDR_ANY) just in case the hst
  // has multiple interfaces.
  struct sockaddr_in server = {AF_INET, PORT, {INADDR_ANY}};

  // The following 2 lines are here to deal with signals
  // that the server tries to send data "down the socket" but the
  // process on the other end has died, or the connection was
  // broken for some other reason, the server will receive a SIGPIPE
  // signal. To keep it alive, it handles the signal.
  //
  // The server will also receive SIGCHLD signals when its children terminate.
  Signal(SIGCHLD, on_sigchld);
  Signal(SIGPIPE, on_sigpipe);

  // The socket() call creates and endpoint of communication.
  // In the call below, the endpoint is for an Internet socket
  // (PF_INET) of the connection-oriented tyep (i.e, TCP rather than UDP).
  // The third paremeter is the protocol. A 0 tells
  // the compiler to use the default protocol for the SOCK_STREAM,
  // which the process can use for listening to the socket.
  if ((listen_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    ERROR_EXIT("socket call failed", 1);
  }

  // The server now has to bind to the socket file descriptor, listen_fd, to
  // the socket data structure. This, in effect, connects the file
  // descriptor to the actual network/port address.
  if ((bind(listen_fd, (struct sockaddr *)&server, SIZE)) == -1) {
    ERROR_EXIT(" bind call failed", 1);
  }

  // the next step for the server is to listen for incoming connection
  // requests. The listen() call establishes the number of simultaneous
  // connections that the server will handler. I.e., it is the size of
  // the ueue of received, but not accepted requests. Here we accept
  // LISTEN_QUEUE_SIZE requests.
  if (listen(listen_fd, LISTEN_QUEUE_SIZE) == -1) {
    ERROR_EXIT(" listen call failed", 1);
  }

  // start the infinite loop to listen for and accept incoming
  // connection requests
  for (;;) {
    // The accept call returns the next completed connection from the
    // front of the completed connection queue. If there are no
    // completed connections in the queue, the process blocks
    // the accept call returns a file descriptor that can be used
    // to read (recv) and/or write (send) data in the socket.
    // The returned descriptor is the connected socket descriptor;
    // the listening descriptor remains available to listen to the socket
    if ((connection_fd = accept(listen_fd, NULL, NULL)) == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        perror(" accept call failed");
      }
    }

    switch (fork()) {
      case -1:
        ERROR_EXIT(" fork call failed", 1);
      case 0:
        // The child executes this code.
        // You can use the ordinary read and write calls, but
        // recv and send are more flexible. recv allows peeking without reading,
        // waiting for full buffers, and discarding
        // all but out-of-band data.
        while (recv(connection_fd, &c, 1, 0) > 0) {
          c = toupper(c);                 // convert c to uppercase
          send(connection_fd, &c, 1, 0);  // send it back
        }
        close(connection_fd);
        exit(0);
      default:
        // server code
        close(connection_fd);
        // note that the serer cannot wait for the chlid processes,
        // otherwise it will not return to the top of the loop to
        // accept new connections. Instead it has a SIGCHLD handler
    }
  }
}

// if a SIGPIPE is received
void on_sigpipe(int sig) {
  close(connection_fd);
  exit(0);
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

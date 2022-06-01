// Chapter 8 Interprocess Communications Part 1
// Now we turn to the server, which is simpler that the client in this example.
// The steps that the server takes can be summarized as follows.
// 1. It registers its signal handlers
// 2. It creates the public FIFO. If it finds it already exists, it display
//    a message and exists.
// 3. It opens the public FIFO for both reading and writing, even though it
//    will only read from it.
// 4. It enters its main-loop, where it repeatedly
//    (a) dloes a blocking read on the public FIFO,
//    (b) on receiving a message from the read(), tires to open the private FIFO
//        of the client that sent it the message. (It tries 5 times, sleeping
//        a bit between each try, in case the client was delayed in opening it
//        for writing. After 5 attemps it gives up on this client.)
//    (c) converts the message to uppercase,
//    (d) writes it tot he private FIFO of the client, and
//    (e) closes the write-end of the private FIFO
//
// It will loop forever because it will never receive an end-of-file on the
// pipe, since it is keeping the write-end open itself. It is terminated by
// sending it a singal.

#include "upcase_1.h"

#define WARNING "\nNOTE: SERVER ** NEVER ** accessed private FIFO\n"
#define MAX_TRIES 5

int dummy_fifo;    // file descriptor to write-end of PUBLIC
int private_fifo;  // file descriptor to write-end of PRIVATE
int public_fifo;   // file descriptor to read-end of PUBLIC

void on_sigpipe(int signo) {
  fprintf(stderr, "Client is not reading the pipe.\n");
}

void on_signal(int sig) {
  close(public_fifo);
  close(dummy_fifo);
  if (private_fifo != -1) {
    close(private_fifo);
  }
  unlink(PUBLIC);
  exit(0);
}

/******************************************************************************/
/*                                Main Program                                */
/******************************************************************************/

int main(int argc, char *argv[]) {
  int tries;    // nubmer of tries to open private FIFO
  int n_bytes;  // nubmer of bytes read from popen()
  int i;
  int done;                  // flag to stop loop
  struct message msg;        // stores private fifo name and command
  struct sigaction handler;  // sigaction for registering handlers

  // Register the signal handler
  handler.sa_handler = on_signal;
  handler.sa_flags = SA_RESTART;
  if (sigaction(SIGINT, &handler, NULL) == -1 ||
      sigaction(SIGHUP, &handler, NULL) == -1 ||
      sigaction(SIGQUIT, &handler, NULL) == -1 ||
      sigaction(SIGTERM, &handler, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  handler.sa_handler = on_sigpipe;
  if (sigaction(SIGPIPE, &handler, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  // Create public FIFO
  if (mkfifo(PUBLIC, 0666) < 0) {
    if (errno != EEXIST) {
      perror(PUBLIC);
    } else {
      fprintf(stderr, "%s already exists. Delete it and restart.\n", PUBLIC);
    }
    exit(1);
  }

  // Open public FIFO for reading and writing so that it does not get an EOF
  // on the read-end while waiting for a client to send data.
  // To prevent it from handing on the open, the write-end is opened in
  // non-blocking mode. It never writes to it.
  if ((public_fifo = open(PUBLIC, O_RDONLY)) == -1 ||
      (dummy_fifo = open(PUBLIC, O_WRONLY | O_NDELAY) == -1)) {
    perror(PUBLIC);
    exit(1);
  }

  // Block waiting for a msg struct from a client
  while (read(public_fifo, (char *)&msg, sizeof(msg)) > 0) {
    // A msg arrived, so start trying to open write end of private FIFO
    tries = done = 0;
    private_fifo = -1;
    do {
      if ((private_fifo = open(msg.fifo_name, O_WRONLY | O_NDELAY)) == -1) {
        sleep(1);
      } else {
        // Convert the text to uppercase
        n_bytes = strlen(msg.text);
        for (i = 0; i < n_bytes; i++) {
          if (islower(msg.text[i])) {
            msg.text[i] = toupper(msg.text[i]);
          }
        }

        // Send converted text to client
        if (write(private_fifo, msg.text, n_bytes) == -1) {
          if (errno == EPIPE) {
            done = 1;
          }
        }
        close(private_fifo);  // close write-end of private FIFO
        done = 1;             // terminate loop
      }
    } while (++tries < MAX_TRIES && !done);

    if (!done) {
      // Failed to open client private FIFO for writing
      write(fileno(stderr), WARNING, sizeof(WARNING));
    }
  }
  return 0;
}

// Chapter 8 Interprocess Communications Part 1
// The steps that the server takes can be summarized as follows.
// 1. It registers its signal handlers
// 2. It creates the public FIFO. If it finds it already exists, it display
//    a message and exists.
// 3. It opens the public FIFO for both reading and writing, even though it
//    will only read from it.
// 4. It enters its main-loop, where it repeatedly
//    (a) does a blocking read on the public FIFO,
//    (b) on receiving a message from the read(), forks a child process to
//        handle the client request.
// on page 45

#include "upcase_2.h"  // All required header files are included in this file

#define WARNING "Server could not access client FIFO"
#define MAX_TRIES 5

int dummy_fifo;                // file descriptor to write-end of PUBLIC
int client_converted_text_fd;  // file descriptor to write-end fo PRIVATE
int client_raw_text_fd;        // file descriptor to write-end of PRIVATE
int public_fifo;               // file descriptor to read-end of PUBLIC
FILE *upcase_log_fp;           // points to log file for server
pid_t server_pid;              // to store server's process id

/******************************************************************************/
/*                           Signal Handler Prototypes                        */
/******************************************************************************/

/**
 * on_sigpipe()
 * This handles the SIGPIPE signals, just writes to standard error.
 */
void on_sigpipe(int signo);

/**
 * on_signal()
 * This handles the interrupt signals. It closes open IFOFs and files,
 * remove the public FIFO and exits.
 */
void on_signal(int sig);

/**
 * on_sigchld()
 * Because this is a concurrent server, the parent process has to collect the
 * exit status of each child. The SIGCHLD handler issues waits and writes to
 * the log file
 */
void on_sigchld(int signo);

/******************************************************************************/
/*                                Main Program                                */
/******************************************************************************/

int main(int argc, char *argv[]) {
  int tries;    // number of tries to open private FIFO
  int n_bytes;  // number of bytes read from private FIFO
  int i;
  struct message msg;        // message structure with FIFO names
  struct sigaction handler;  // sigaction for registering handlers
  char buffer[PIPE_BUF];
  char log_file_path[PATH_MAX];
  char *home_path;  // path to home directory
  pid_t child_pid;  // process id of each spawned child

  // Open the log file in the users's home directory for appending.
  home_path = getenv("HOME");
  sprintf(log_file_path, "%s/.upcase_log", home_path);

  if ((upcase_log_fp = fopen(log_file_path, "a")) == NULL) {
    perror(log_file_path);
    exit(1);
  }

  // Register the interrupt signal handler
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

  handler.sa_handler = on_sigchld;
  if (sigaction(SIGCHLD, &handler, NULL) == -1) {
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
  // To prevent it from handling on the open, the write-end is opened in
  // non-blocking mode. It never writes to it.
  if ((public_fifo = open(PUBLIC, O_RDONLY)) == -1 ||
      (dummy_fifo = open(PUBLIC, O_WRONLY | O_NDELAY) == -1)) {
    perror(PUBLIC);
    exit(1);
  }

  server_pid = getpid();

  // Block waiting for a msg structure from a client
  while (read(public_fifo, (char *)&msg, sizeof(msg)) > 0) {
    // spawn a child proess to handle this client
    if (fork() == 0) {
      // We get the pid for message identification.
      child_pid = getpid();

      // We use the value of client_raw_text_fd to detect
      client_raw_text_fd = -1;

      // Client should have opened raw_text_fd for writing before sending the
      // message structure, so the following open should succeed immediately.
      // If not it blocks until the client opens it.
      if ((client_raw_text_fd = open(msg.raw_text_fifo, O_RDONLY)) == -1) {
        fprintf(upcase_log_fp, "Client did not have pipe open for writing\n");
        exit(1);
      }

      // Clear the buffer used for reading the client's text
      memset(buffer, 0x0, PIPE_BUF);

      // Attempt to read from client's raw_text_fifo. This read will block
      // until either input is available or it receives an EOF. An EOF is
      // delivered only when the client closes the write-end of its
      // raw_text_file
      while ((n_bytes = read(client_raw_text_fd, buffer, PIPE_BUF)) > 0) {
        // Convert the text to uppercase
        for (i = 0; i < n_bytes; i++) {
          if (islower(buffer[i])) {
            buffer[i] = toupper(buffer[i]);
          }
        }

        // Open client's converted_text FIFO for writing. To allow for delays,
        // we try 5 times. Here it is critical that the O_NONBLOCK flag is set,
        // otherwise it will hang in the loop and we will not be able to abandon
        // the attempt if the client has died.
        tries = 0;
        while (((client_converted_text_fd = open(msg.converted_text_fifo,
                                                 O_WRONLY | O_NDELAY)) == -1 &&
                (tries < MAX_TRIES))) {
          sleep(2);
          tries++;
        }

        if (tries == MAX_TRIES) {
          // Failed to open client converted_text FIFO for writing
          fprintf(upcase_log_fp, "%d: " WARNING, child_pid);
          exit(1);
        }

        // Send converted text to client in its read fifo
        if ((write(client_converted_text_fd, buffer, n_bytes)) == -1) {
          if (errno == EPIPE) {
            exit(1);
          }
        }

        // See the notes below
        close(client_converted_text_fd);
        client_converted_text_fd = -1;

        // Clear the buffer used for reading the client's text
        memset(buffer, 0x0, PIPE_BUF);
      }
      exit(0);
    }
  }
  return 0;
}

void on_sigchld(int signo) {
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    printf((char *)upcase_log_fp, "Child process %d terminated.\n", pid);
  }
  fflush(upcase_log_fp);
  return;
}

void on_sigpipe(int signo) {
  fprintf(stderr, "Client is not reading the pipe.\n");
}

void on_signal(int sig) {
  close(public_fifo);
  close(dummy_fifo);

  if (client_converted_text_fd != -1) {
    close(client_converted_text_fd);
  }

  if (client_raw_text_fd != -1) {
    close(client_raw_text_fd);
  }

  if (getpid() == server_pid) {
    unlink(PUBLIC);
  }
  fclose(upcase_log_fp);
  exit(0);
}

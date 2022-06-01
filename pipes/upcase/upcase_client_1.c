// Chapter 8 Interprocess Communications Part 1
// The basic steps that the client takes are as follows
// 1. Its makes sure that neither standard input nor output is redirected.
// 2. It registers its singal handlers
// 3. It creates its private FIFO in /tmp
// 4. It tries to open the public FIFO for writing in non-blocking mode.
// 5. It enters a loop in which it repeatedly
//   (1) reads a line from standard input, and
//   (2) repeatedly
//      i. gets the next HALF_PIPE_BUF -1 sized chunk in the input text,
//      ii. sends a message to the server through the public FIFO,
//      iii. opens its private FIFO for reading,
//      iv. reads the server's reply from the private FIFO,
//      v. copies the server's reply to its standard output, and
//      vi. closes the read-end of its private FIFO
// 6. It closes the write-end of the public FIFO and removes its private FIFO.
// on page 33

#include "upcase_1.h"  // All required header files are included in this file

#define PROMPT "string: "
#define UPCASE "UPCASE: "
#define QUIT "quit"

const char startup_msg[] =
    "upcase1 does not seem to be running. "
    "Please start the service.\n";

volatile sig_atomic_t sig_received = 0;
struct message msg;

/******************************************************************************/
/*                              Signal Handlers                               */
/******************************************************************************/

void on_sigpipe(int signo) {
  fprintf(stderr, "upcase_server is not reading the pipe.\n");
  unlink(msg.fifo_name);
  exit(1);
}

void on_signal(int sig) { sig_received = 1; }

/******************************************************************************/
/*                              Main Program                                  */
/******************************************************************************/

int main(int argc, char *argv[]) {
  int str_length;    // number of bytes in text to convert
  int n_chunk;       // index of text chunk to send to server
  int bytes_read;    // bytes received in read from server
  int private_fifo;  // file descriptor to read-end of PRIVATE
  int public_fifo;   // file descriptor to write-end of PUBLIC
  static char buffer[PIPE_BUF];
  static char text_buf[BUFSIZ];

  struct sigaction handler;

  // Only run if we are using the terminal
  if (!isatty(fileno(stdin)) || !isatty(fileno(stdout))) {
    exit(1);
  }

  // Register the on_signal handler to handle all keyboard signals
  handler.sa_handler = on_signal;  // handle function
  if (sigaction(SIGINT, &handler, NULL) == -1 ||
      sigaction(SIGHUP, &handler, NULL) == -1 ||
      sigaction(SIGQUIT, &handler, NULL) == -1 ||
      sigaction(SIGTERM, &handler, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  // Create hopefully unique name for private FIFO using process-id
  sprintf(msg.fifo_name, "/tmp/fifo_%d", getpid());

  // Create the private FIFO
  if (mkfifo(msg.fifo_name, 0666) < 0) {
    perror(msg.fifo_name);
    exit(1);
  }

  // Open the public FIFO for writing
  if ((public_fifo = open(PUBLIC, O_WRONLY | O_NONBLOCK)) == -1) {
    if (errno == ENXIO) {
      fprintf(stderr, "%s", startup_msg);
    } else {
      perror(PUBLIC);
    }
    exit(1);
  }
  printf("Type 'quit' to quit.\n");

  // Repeatedly prompt user for input, read it, and send to server
  while (1) {
    // Check if SIGINT received first, and if so, close write-end
    // of public fifo, remove private fifo and then quit
    if (sig_received) {
      close(public_fifo);
      unlink(msg.fifo_name);
      exit(1);
    }

    // Display a prompt on the terminal and read the input text
    write(fileno(stdout), PROMPT, sizeof(PROMPT));
    memset(msg.text, 0x0, HALF_PIPE_BUF);  // zero string
    fgets(text_buf, BUFSIZ, stdin);
    str_length = strlen(text_buf);
    if (!strncmp(QUIT, text_buf, str_length - 1)) {  // is it quit?
      break;
    }

    // Display label for returned upper case text
    write(fileno(stdout), UPCASE, sizeof(UPCASE));

    for (n_chunk = 0; n_chunk < str_length; n_chunk += HALF_PIPE_BUF) {
      memset(msg.text, 0x0, HALF_PIPE_BUF);
      strncpy(msg.text, text_buf + n_chunk, HALF_PIPE_BUF - 1);
      write(public_fifo, (char *)&msg, sizeof(msg));

      // Open the private FIFO for reading to get output of command
      // from the server
      if ((private_fifo = open(msg.fifo_name, O_RDONLY)) == -1) {
        perror(msg.fifo_name);
        exit(1);
      }

      // Read maximum number of bytes possible atomically
      // and copy them to standard output.
      while ((bytes_read = read(private_fifo, buffer, PIPE_BUF)) > 0) {
        write(fileno(stdout), buffer, bytes_read);
      }

      close(private_fifo);  // close the read-end of private FIFO
    }
  }

  // User quit, so close write-end of public FIFO and delete private FIFO
  close(public_fifo);
  unlink(msg.fifo_name);

  return 0;
}

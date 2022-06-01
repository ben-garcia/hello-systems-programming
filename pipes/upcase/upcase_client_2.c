// Chapter 8 Interprocess Communications Part 1
// The client is also structurally different from the previous client. The
// major steps that it takes as follows.
// 1. It registers its signal handlers
// 2. It creates two private FIFOs in the /tmp directory with unique names.
// 3. It opens the server's public FIFO for writing.
// 4. It sends the initial message structure containing the names of its two
//    FIFOs to the server to establish the two-way communication.
// 5. It attempts to open its raw_text FIFO in non-blocking, write-only mode.
//    If it fails, it delays a second and retries. It retries a few times and
//    then gives up and exists. If it fails it means that the server is
//    probably terminated.
// 6. Until it receives an end-of-file on its standard input, it repeatedly
//    (a) reads a line rom standard input,
//    (b) breaks the line into PIPE_BUF - sized chunks,
//    (c) sends each chunk successively to the server through its raw_text FIFO,
//    (d) opens the converted_text FIFO for reading,
//    (e) reads the converted_text FIFO, and copies its contents to its
//        standard output, and
//    (f) closes the read-end of the converted_text FIFO
// 7. It closes all of its FIFOs and removes the files.
//   // on page 41

#include "upcase_2.h"  // All required header files are included in this file

#define MAXTRIES 5

const char startup_msg[] =
    "The upcase2 server does not seem to be running. "
    "Please start the service.\n";
const char server_no_read_msg[] = "The server is not reading the pipe.\n";

int converted_text_fd;  // file descriptor for READ PRIVATE FIFO
int dummy_read_fifo;    // to hold fifo open
int raw_text_fd;        // file descriptor to WRITE PRIVATE FIFO
int dummy_raw_fifo_fd;  // to hold the raw text fifo open
int public_fifo;        // file descriptor to write-end of PUBLIC
FILE *input_srcp;       // File pointer to input stream
struct message msg;     // 2-way communication structure

/******************************************************************************/
/*                        Signal Handlers and Utilities                       */
/******************************************************************************/

void on_sigpipe(int signo) {
  fprintf(stderr, "upcase_server_2 is not reading the pipe.\n");
  unlink(msg.raw_text_fifo);
  unlink(msg.converted_text_fifo);
  exit(1);
}

void on_signal(int sig) {
  close(public_fifo);
  if (converted_text_fd != -1) {
    close(converted_text_fd);
  }
  if (raw_text_fd != -1) {
    close(raw_text_fd);
  }
  unlink(msg.converted_text_fifo);
  unlink(msg.raw_text_fifo);
  exit(0);
}

void clean_up() {
  close(public_fifo);
  close(raw_text_fd);
  unlink(msg.converted_text_fifo);
  unlink(msg.raw_text_fifo);
}

/******************************************************************************/
/*                              Main Program                                  */
/******************************************************************************/

int main(int argc, char *argv[]) {
  int str_length;  // number of bytes in text to convert
  int n_chunk;     // index of text chunk to send to server
  int bytes_read;  // bytes received in read from server
  static char buffer[PIPE_BUF];
  static char text_buf[BUFSIZ];
  struct sigaction handler;
  int tries;  // for counting tries to open raw text fifo

  // Check whether there is a command line argument, and if so, use it as
  // the input source.
  if (argc > 1) {
    if ((input_srcp = fopen(argv[1], "r")) == NULL) {
      perror(argv[1]);
      exit(1);
    }
  } else {
    input_srcp = stdin;
  }

  // Initialize the file descriptors for error handling
  public_fifo = -1;
  converted_text_fd = -1;
  raw_text_fd = -1;

  // Register the on_signal handler to handle all keyboard signals
  handler.sa_handler = on_signal;  // handle function
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

  // Create hopefully unique name for private FIFO using process-id
  sprintf(msg.converted_text_fifo, "/tmp/fifo_rd%d", getpid());
  sprintf(msg.raw_text_fifo, "/tmp/fifo_wr%d", getpid());

  // Create the private FIFOs
  if (mkfifo(msg.converted_text_fifo, 0666) < 0) {
    perror(msg.converted_text_fifo);
    exit(1);
  }

  if (mkfifo(msg.raw_text_fifo, 0666) < 0) {
    perror(msg.raw_text_fifo);
    exit(1);
  }

  // Open the public FIFO for writing
  if ((public_fifo = open(PUBLIC, O_WRONLY | O_NDELAY)) == -1) {
    if (errno == ENXIO) {
      fprintf(stderr, "%s", startup_msg);
    } else {
      perror(PUBLIC);
    }
    exit(1);
  }

  // Send a message to server with names of two FIFOs
  write(public_fifo, (char *)&msg, sizeof(msg));

  // Try to open the raw text FIFO for writing. After MAXTRIES
  // attemps we give up.
  tries = 0;
  while ((raw_text_fd = open(msg.raw_text_fifo, O_WRONLY | O_NDELAY) == -1) &&
         (tries < MAXTRIES)) {
    sleep(1);
    tries++;
  }

  if (tries == MAXTRIES) {
    fprintf(stderr, "%s", server_no_read_msg);
    clean_up();
    exit(1);
  }

  // Get one line of input at a time from the input source
  while (1) {
    memset(text_buf, 0x0, BUFSIZ);
    if (fgets(text_buf, BUFSIZ, input_srcp) == NULL) {
      break;
    }

    str_length = strlen(text_buf);

    // Break input lines into chunks and send them one at a time through
    // the client's write FIFO
    for (n_chunk = 0; n_chunk < str_length; n_chunk += PIPE_BUF - 1) {
      memset(buffer, 0x0, PIPE_BUF);
      strncpy(buffer, text_buf + n_chunk, PIPE_BUF - 1);
      buffer[PIPE_BUF - 1] = '\0';
      write(raw_text_fd, buffer, strlen(buffer));

      // open the private FIFO for reading to get output of command
      // from the server
      if ((converted_text_fd =
               open(msg.converted_text_fifo, O_RDONLY | O_NDELAY)) == -1) {
        perror(msg.converted_text_fifo);
        exit(1);
      }
      memset(buffer, 0x0, PIPE_BUF);
      while ((bytes_read = read(converted_text_fd, buffer, PIPE_BUF)) > 0) {
        write(fileno(stdout), buffer, bytes_read);
      }
      close(converted_text_fd);
      converted_text_fd = -1;
    }
  }

  // User quit, so close write-end of public FIFO and delete private FIFO
  close(public_fifo);
  close(raw_text_fd);
  unlink(msg.converted_text_fifo);
  unlink(msg.raw_text_fifo);

  return 0;
}

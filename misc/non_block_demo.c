// Chapter 5 Interactive Programs and Signals
// Non-Blocking Input
// The O_NONBLOCK flag controls whether reads and writes are blocking or
// non-blocking. When a read is blocking, the process that executes the read 
// waits until input is available, in the buffer, and only then does it continue.
// When a process opens a non-blocking connection to an input source, whether
// it is a file or a device, calls to read data from that source retrieve
// whatever data is in the buffer at the time of the call, up to the amount
// requested in the read request, and return immediately. If the buffer is
// empty, they return immediately with no data.

#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define RETRIEVE 1
#define RESTORE 2
#define FALSE 0
#define TRUE 1

void save_restore_tty(int fd, int action);
void modify_termios(int fd, int echo, int canon);
void get_window_size(int fd, int *rows, int *cols);

void set_non_block(int fd) {
  int flagset;

  flagset = fcntl(fd, F_GETFL);
  flagset |= O_NONBLOCK;
  fcntl(fd,F_SETFL, flagset);
}

int main(int argc, char *argv[]) {
  char ch; // store user's char
  char period = '.';
  size_t bytecount;
  int count = 0;
  int done = 0; // to control when to stop loop
  int pause = 0; // to control pausing of output
  char PARK[20]; // ANSI escape sequence for parking cursor
  int numrows; // number of rows in window
  int numcols; // number of columns in window

  const char CURSOR_HOME[] = "\033[1;1H";
  const char CLEAR_SCREEN[] = "\033[2J";
  const char SAVE_CURSOR[] = "\033[s";
  const char REST_CURSOR[] = "\033[u";
  const char MENU[] = "Type q to quit, r to resume or p to pause";
  char dots[20];

  // Check whether input or output has been redirected
  if (!isatty(0) || !isatty(1)) {
    fprintf(stderr, "Output has been redirected!\n");
    exit(EXIT_FAILURE);
  }

  // Save the original tty state
  save_restore_tty(STDIN_FILENO, RETRIEVE);

  // Modify the terminal
  // turn off echo, keybd sigs, and canonical mode
  modify_termios(STDIN_FILENO, 0, 0);

  // Turn off blocking mode
  set_non_block(STDIN_FILENO);

  // Get the window's size
  get_window_size(STDIN_FILENO, &numrows, &numcols);

  // Create string to park cursor
  sprintf(PARK, "\033[%d;1H", numrows + 1);

  // Clear the screen and put cursor in upper left corner
  write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
  write(STDOUT_FILENO, CURSOR_HOME, strlen(CURSOR_HOME));

  // Start drawing. Stop when the screen is full
  while (!done) {
    if (!pause) {
      count++;
      // Is screen full except for bottom row
      if (count > (numcols * (numrows - 1))) {
        pause = 1;
        count--;
      } else {
        write(STDOUT_FILENO, &period, 1);
      }
    }
    usleep(100000); // delay a bit
    sprintf(dots, " dots: %d ", count);

    // Save the cursor, park it, write the menu prompt
    write(STDOUT_FILENO, SAVE_CURSOR, strlen(SAVE_CURSOR));
    write(STDOUT_FILENO, PARK, strlen(PARK));
    write(STDOUT_FILENO, MENU, strlen(MENU));
    write(STDOUT_FILENO, dots, strlen(dots));

    // Do the red. If nothing was typed, do nothing
    if ((bytecount = read(STDIN_FILENO, &ch, 1)) > 0) {
      if (ch == 'q') {
        done = 1;
      } else if (ch == 'p') {
        pause = 1;
      } else if (ch == 'r') {
        pause = 0;
      }
    }

    // Resotre the cursor so the next dot follows the previous
    write(STDOUT_FILENO, REST_CURSOR, strlen(REST_CURSOR));
  }
  // Cleanup -- flush queue, clear the screen, and restore terminal
  tcflush(STDIN_FILENO, TCIFLUSH);
  write(1, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
  write(1, CURSOR_HOME, strlen(CURSOR_HOME));

  save_restore_tty(STDIN_FILENO, RESTORE);

  return 0;
}

/**
 * Modify the terminal by setting relevant flags
 *
 * @param fd - file descriptor
 *           - STDIN_FILENO for stdin
 *           - STDOUT_FILENO for stdout
 * @param echo - flag to indicate whether if echoing should be enabled 
 *             - set to 0 when the program is running, then turned on after
 *               program exists.
 * @param canon - flag that indicates whether to use in canonical mode
 *              - in canonical input is processed after the 'Enter'
 *                key is pressed.
 *              - non canonical input is processed after each key press and
 *                it is up to the program to choose how to respond.
 */
void modify_termios(int fd, int echo, int canon) {
  struct termios cur_tty;
  tcgetattr(fd, &cur_tty); // get copy of the state for the terminal

  if (canon) { // nanonical mode
    cur_tty.c_lflag |= ICANON;
  } else { // non-canonical mode
    cur_tty.c_lflag &= ~ICANON;
  }

  if (echo) { // enable echoing 
    cur_tty.c_lflag |= ECHO;
  } else { // disable echoing
    cur_tty.c_lflag &= (~ECHO & ~ECHOE);
  }

  cur_tty.c_lflag &= ~ISIG;
  // set MIN and TIME line discipline values, so that the
  // program reads a single character at a time and does not time out.
  cur_tty.c_cc[VMIN] = 1;
  cur_tty.c_cc[VTIME] = 0;

  tcsetattr(fd, TCSADRAIN, &cur_tty); // update the terminal attributes
}

/**
 * Update or restore terminal
 *
 * @param fd - file descriptor
 * @param action - the action to perform
 *               - when 1 or RETRIEVED, get a copy of the current state
 *               - when 0 or RESTORE, set the termnial to the original state
 */
void save_restore_tty(int fd, int action) {
  static struct termios original_state;
  static int retrieved = FALSE;

  if (action == RETRIEVE) {
    retrieved = TRUE;
    tcgetattr(fd, &original_state);
  } else if (retrieved && RESTORE == action) {
    tcsetattr(fd, TCSADRAIN, &original_state);
  } else {
    fprintf(stderr, "Illegal action to save_restore_tty().\n");
  }
}


void get_window_size(int fd, int *rows, int *cols) {
  struct winsize size;

  if (ioctl(fd, TIOCGWINSZ, &size) < 0) { // get window fields
    perror("TIOCGWINSZ error");
    return;
  }
  *rows = size.ws_row;
  *cols = size.ws_col;
}

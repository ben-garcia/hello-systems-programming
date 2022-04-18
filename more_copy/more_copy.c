#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define SCREEN_ROWS 23 // assume 24 lines per screen .
#define LINELEN 512
#define SPACEBAR 1
#define RETURN 2
#define QUIT 3
#define INVALID 4

/** do_more_of(FILE *file_ptr)
*   Give a File* arguement fp, display up to a page of the
*   file fp, and then display a prompt and wait for user input.
*   If user inputs SPACEBAR, display next page.
*   IF user inputs RETURN, display one more line.
*   IF user inputs QUIT, terminate program.
*/
void do_more_of(FILE *file_ptr);

/** get_user_input()
*   Displays more's status and prompt and waits for user response,
*   Requires that user press return key to receive input
*   Returns on of SPACEBAR, RETURN, or QUIT on valid keypresses
*/
int get_user_input(FILE *file_ptr);

void get_tty_size(FILE *tty_fp, int *num_rows, int *num_columns);

int main(int argc, char *argv[]) {
  FILE *file_ptr;
  int i = 0;

  if (1 == argc) {
    do_more_of(stdin); // no args, read from standard input.
  } else {
      while (++i < argc) {
        file_ptr = fopen(argv[i], "r");

        if (NULL != file_ptr) {
          do_more_of(file_ptr);
          fclose(file_ptr);
        } else {
          printf("Skipping %s\n", argv[i]);
        }
      }
  }

  return 0;
}

void do_more_of(FILE *file_ptr) {
  char line[LINELEN]; // buffer to store line of input.
  int num_of_lines; // number of lines left on screen.
  int reply; // input from user 
  int tty_rows; // number of rows in the terminal
  int tty_columns; // number of columns in the terminal
  FILE *file_ptr_tty; // device file pointer

  file_ptr_tty = fopen("/dev/tty", "r"); // NEW: FILE stream argument

  if (file_ptr_tty == NULL) { // if open fails
    exit(1); // exit
  }

  // Get the size of the terminal window 
  get_tty_size(file_ptr_tty, &tty_rows, &tty_columns);
  num_of_lines = tty_rows;

  while (fgets(line, LINELEN, file_ptr)) {
    // fgets() returns a pointer to string that was read
    if (num_of_lines == 0) { // reached screen capapcity
      // reached screen capacity of display prompt
      reply = get_user_input(file_ptr_tty); // not call here

      switch(reply) {
        case SPACEBAR:
          // allow full screen
          num_of_lines = tty_rows;
          printf("\033[1A\033[2K\033[1G");
          break;
        case RETURN:
          // allow one more line
          printf("\033[1A\033[2K\033[1G");
          num_of_lines++;
          break;
        case QUIT:
          printf("\033[1A\033[2K\033[1B\033[7D");
          break;
        default: // in case of invalid input
          break;
      }
    }

    if (fputs(line, stdout) == EOF) {
      exit(1);
    }

    num_of_lines--;
  }
}

void get_tty_size(FILE *tty_fp, int *num_rows, int *num_columns) {
  struct winsize window_arg;

  if (ioctl(fileno(tty_fp), TIOCGWINSZ, &window_arg) == -1) {
    exit(1);
  }

  *num_rows = window_arg.ws_row;
  *num_columns = window_arg.ws_col;
}

/**
*  display message, wait for response, return key entered as int
*  Read user input from stream file_ptr
*  Returns SPACEBAR, RETURN, QUIT, or INVALID
*/
int get_user_input(FILE *file_ptr) {
  int c;
  int tty_rows;
  int tty_columns;

  /*
   * Get the size of the terminal window dynamically, in case it changed.
   * Then use it to put the cursor in the bottom row, leftmost column
   * and print the prompt in "standout mode" i.e. reverse video.
   */
  get_tty_size(file_ptr, &tty_rows, &tty_columns);
  printf("\033[%d;1H", tty_rows);
  printf("\033[7m more? \033[m"); // reverse on a VT100
  
  /**
  * Use fgetc() instead of getc(). It is the same except
  * that is always a function call, not a macro, and it is in general
  * safer to use.
  */
  while ((c = fgetc(file_ptr)) != EOF) { // wait for response
    /**
    * There is no need to use a loop here, since all possible paths
    * lead to a return statement. It remains since there is no downside
    * to using it.
    */
    switch(c) {
      case 'q': // 'q' pressed
        return QUIT;
      case ' ': //  ' ' or spacebar is pressed
        return SPACEBAR;
      case '\n': // ENTER key pressed
        return RETURN;
    }
  }

  return INVALID; // invalid if anything else is pressed
}




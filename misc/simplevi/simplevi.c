// Chapter 5 Interactive Programs and Signals

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif

#define RETRIEVE 1
#define RESTORE 2
#define FALSE 0
#define TRUE 1
#define ESCAPE '\033'
#define CONTROL_C '\003'
#define CONTROL_D '\004'
#define CONTROL_H '\010'
#define KEY_UP 65
#define KEY_DOWN 66
#define KEY_RIGHT 67
#define KEY_LEFT 68
#define MAXLINES 1000
#define MAXCHARS 255
#define OUT_OF_LINES -1
#define OUT_OF_MEM -2
#define UNHANDLEDCHAR -3

const char CLEAR_DOWN[] = "\033[0J";
const int ICLEAR_DOWN = 4;
const char CLEAR_RIGHT[] = "\033[0K";
const int ICLEAR_RIGHT = 4;
const char CURSOR_HOME[] = "\033[1;1H";
const int ICURSOR_HOME = 6;
const char CLEAR_SCREEN[] = "\033[2J";
const int ICLEAR_SCREEN = 4;
const char CLEAR_LINE[] = "\033[2K";
const int ICLEAR_LINE = 4;
const char RIGHT[] = "\033[1C";
const int IRIGHT = 4;
const char LEFT[] = "\033[1D";
const int ILEFT = 4;
const char BACKSPACE[] = "\033[1D \033[1D";
const int IBACKSPACE = 9;
char PARK[20]; // string to park cursor at lower left
int IPARK; // length of PARK string

// Miscellaneous strings for output
const char CTRLC[] = "You typed Control-C.";
const char CTRLD[] = "You typed Control-D.";
const char CTRLH[] = "This is the Help Command. Not much help, sorry!";
const char BLANK = ' ';
const char INSERT[] = "---INSERT---";
const int IINSERT = 10;
const char MAXLINES_MSSGE[] =
  "You reached the maximum number of lines."
  " Exiting input mode.";
const char OUT_OF_MEM_MSSGE[] = 
  "You reached the maximum buffer size."
  " Exiting input mode.";
const char UNHANDLEDCHAR_MSSGE[] =
  "This input not yet implemented."
  " Exiting input mode.";

/**
 * Clear the screen and put cursor in upper left corner.
 */
static inline void clear_and_home() {
  write(1, CLEAR_SCREEN, ICLEAR_SCREEN);
  write(1, CURSOR_HOME, ICURSOR_HOME);
}

static inline void park() {
  write(1, PARK, IPARK);
  write(1, CLEAR_LINE, ICLEAR_LINE);
}

/******************************************************************************/
/*                                 Data Types                                 */
/******************************************************************************/
typedef struct _cursor {
  int r;
  int c;
} Cursor;

typedef struct _window {
  unsigned short rows;
  unsigned short cols;
  int line_at_top;
  char erase_char;
} Window;


typedef struct _buffer {
  char text[BUFSIZ];
  int line_len[MAXLINES]; // lengths of text lines, including newline characters
  int line_start[MAXLINES]; // starts of each line
  // number of text lines in buffer. This includes lines that have not yet been
  // terminated with a newline character. It is the number of newline
  // characters + 1 if the last character in the buffer is not a newline.
  int num_lines;
  int index; // index of cursor in text buffer
  int size; // total chars in buffer
  int cur_line; // current text line, not screen line
  int index_in_cur_line; // index in current line of cursor
} Buffer;

/******************************************************************************/
/*                          Function Prototypes                               */
/******************************************************************************/

// Window/Terminal Functions
void init_window(int fd, Window *win);
void move_to(int line, int column);
void write_status_message(const char *message, Cursor curs);
void save_restore_tty(int fd, int action);
void modify_termios(int fd, int echo, int canon);
void set_erase_char(int termfd, Window *win);

// Buffer Functions
int insert(Buffer *buf, Window win, char ch);
void init_buffer(Buffer *buffer);
void update_buffer_index(Buffer *buffer);
int buffer_index(int index_in_line, int cur_line, int linelength[]);
void redraw_buffer(Buffer buffer, Window *win, Cursor *curs);
void scroll_buffer(Buffer buf, Window win);
int line_in_buffer(Buffer buf, Window win, int pos);
void save_buffer(const char path[], Buffer buf, char *statusstr);
int handle_insertion(Buffer *buf, Window *win, Cursor *curs, char c);
void get_lastline_in_win(Buffer buffer, Window win, int *lastline);

// Lastline Mode
int parse_lastline(char *str, int len, Buffer buf, char *statusstr);
int do_lastline_mode(Buffer buf, Window win, Cursor curs);

// Cursor Functions
void init_cursor(Cursor *cursor);
void show_cursor(Buffer buf,
                 Window win,
                 Cursor cursor,
                 int index_in_line,
                 int line_number);
void advance_cursor(Cursor *cursor, Window win, char ch);
void get_cursor_at(Buffer buf, Window win, int index, int lineno, Cursor *curs);
void handle_escape_char(Buffer *buf, Window *win, Cursor *curs);
void move_up(Buffer *buf, Window *win, Cursor *curs);
void move_down(Buffer *buf, Window *win, Cursor *curs);
void move_right(Buffer *buf, Window *win, Cursor *curs);
void move_left(Buffer *buf, Window *win, Cursor *curs);

/******************************************************************************/
/*                                  Main                                      */
/******************************************************************************/
int main(int argc, char *argv[]) {
  int quit = 0;
  int in_input_mode = 0;
  int in_lastline_mode = 0;
  Buffer buf;
  Window win;
  Cursor curs; // cursor position (0, 0) in upper left
  char prompt = ':'; // prompt character
  char c;
  int status;

  if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
    fprintf(stderr, "Not a terminal\n");
    exit(1);
  }

  save_restore_tty(STDIN_FILENO, RETRIEVE);
  modify_termios(STDIN_FILENO, 0, 0);
  init_buffer(&buf);
  init_cursor(&curs);
  init_window(STDIN_FILENO, &win);

  // send formated string to PARK.
  sprintf(PARK, "\033[%d;1H", win.rows);
  IPARK = strlen(PARK);

  // Clear the screen and put cursor in upper left corner
  clear_and_home();

  while (!quit) {
    if (in_input_mode) {
      if (read(STDIN_FILENO, &c, 1) > 0) {
        if (c == ESCAPE) {
          in_input_mode = 0;
          write_status_message("          ", curs);
        } else {
            // insert typed char and echo it
            in_input_mode = handle_insertion(&buf, &win, &curs, c);
            if (in_input_mode ==  UNHANDLEDCHAR) {
              in_input_mode = 1;
            } else {
                write_status_message(INSERT, curs);
          }
        }
      }
    } else {
        if (read(STDIN_FILENO, &c, 1) > 0) {
          switch (c) {
            case 'i':
              in_input_mode = 1;
              park();
              update_buffer_index(&buf);
              move_to(curs.r, curs.c);
              write_status_message(INSERT, curs);
              break;
            case ':':
              in_input_mode = 1;
              park();
              write(1, &prompt, 1);
              status = do_lastline_mode(buf, win, curs);
              if (status >= 0) {
                quit = status; 
              }
              move_to(curs.r, curs.c);
              break;
            case CONTROL_C:
              write_status_message(CTRLC, curs);
              break;
            case CONTROL_D:
              write_status_message(CTRLD, curs);
              break;
            case CONTROL_H:
              write_status_message(CTRLH, curs);
              break;
            case 'j':
              move_down(&buf, &win, &curs);
              break;
            case 'k':
              move_up(&buf, &win, &curs);
              break;
            case 'l':
            case ' ':
              move_right(&buf, &win, &curs);
              break;
            case 'h':
              move_left(&buf, &win, &curs);
              break;
            case ESCAPE:
              handle_escape_char(&buf, &win, &curs);
              break;
            default:
              // check for backspace
              if (c == win.erase_char) {
                move_left(&buf, &win, &curs);
              }
          }
        }
    }
  }
  printf("\n");
  tcflush(STDIN_FILENO, TCIFLUSH); 
  clear_and_home();
  save_restore_tty(STDIN_FILENO, RESTORE);
  return 0;
}

/******************************************************************************/

int parse_lastline(char *str, int len, Buffer buf, char *statusstr) {
  int i = 0;  
  int foundquit = 0;
  int foundwrite = 0;
  int badchar = 0;
  int done = 0;
  char *filename = NULL;
  int state;

  state = 1;

  while ((i < len) && !done) {
    switch (state) {
      case 1:
        if (str[i] == ' ') {
          state = 1;
        } else if (str[i] == 'w') { // user wishes to save file 
          foundwrite = 1;
          state = 2;
        } else if (str[i] == 'q') {
          foundquit = 1;
          state = 7;
        } else {
          state = 5;
        }
        i++;
        break;
      case 2:
        if (str[i] == 'q') {
          foundquit = 1;
          state = 3;
        } else if (str[i] == ' ') {
            state = 4;
        } else {
            state = 5;
        }
        i++;
        break;
      case 3:
        if (str[i] == ' ') {
          state = 4; 
        } else {
          state = 5;
        }
        i++;
        break;
      case 4:
        if (str[i] == ' ') {
          state = 4; 
        } else if (isalnum(str[i]) || str[i] == '_') {
          filename = &(str[i]);
          state = 6;
        } else {
          state = 5;
        }
        i++;
        break;
      case 5:
        badchar = 1;
        sprintf(statusstr, "\033[7m: %s Not an editor command\033[27m", str);
        return -1;
      case 6:
        filename = &(str[i - 1]);
        while ((i < len) && (isalnum(str[i])) || str[i] == '_') {
          i++;
        }
        str[i] = '\0';
        done = 1;
        break;
      case 7:
        if (str[i] == ' ') {
          state = 7;
        } else {
          badchar = 1;
          sprintf(statusstr, "\033[7m: %s Not an editor command\033[27m", str);
          return -1;
        }
        i++;
    }
  }

  if (foundwrite) {
    if (filename != NULL) {
      save_buffer(filename, buf, statusstr);
    } else {
      sprintf(statusstr, "\033[7m: %s Not an editor command\033[27m", str);
      return -1;
    }
  }

  if (foundquit) {
    return 1;
  } else {
    return 0;
  }
}

/******************************************************************************/

int do_lastline_mode(Buffer buf, Window win, Cursor curs) {
  char tempstr[MAXCHARS]; // store user command
  char statusstr[MAXCHARS];
  char c;
  int i = 0;
  int status;
  int in_lastline_mode = 1;

  while (in_lastline_mode) {
    read(STDIN_FILENO, &c, 1);
    if (c == '\n') {
      tempstr[i] = '\0'; // indicate the end of the string
      status = parse_lastline(tempstr, strlen(tempstr), buf, statusstr);
      in_lastline_mode = 0;
      write_status_message(statusstr, curs);
      statusstr[0] = '\0';
    } else if (c == win.erase_char) {
        write(1, BACKSPACE, IBACKSPACE);
        if (i > 0 ) {
          i--;
        } else {
          in_lastline_mode = 0;
        }
    } else {
        tempstr[i++] = c; // add char to the user string
        write(1, &c, 1); // echo it to the screen at the bottom
    }
  }
  return status;
}

void handle_escape_char(Buffer *buf, Window *win, Cursor *curs) {
  char c;

  read(STDIN_FILENO, &c, 1);

  if (c == 91) {
    read(STDIN_FILENO, &c, 1);
    switch (c) {
      case KEY_UP:
        move_up(buf, win, curs);
        break;
      case KEY_DOWN:
        move_down(buf, win, curs);
        break;
      case KEY_RIGHT:
        move_right(buf, win, curs);
        break;
      case KEY_LEFT:  
        move_left(buf, win, curs);
        break;
    }
  }
}

/**
 * Set the erase character for the terminal
 *
 * @param termfd - file descriptor of the terminal
 * @param win - window struct which represents the terminal itself
 */
void set_erase_char(int termfd, Window *win) {
  struct termios cur_tty;
  tcgetattr(termfd, &cur_tty);

  win->erase_char = cur_tty.c_cc[VERASE];
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


/******************************************************************************/
/*                              Window Functions                              */
/******************************************************************************/


/**
 * Initialize the window by settings its fields.
 *
 * @param fd - file descriptor 
 * @param win - window struct which represents the terminal itself
 */
void init_window(int fd, Window *win) {
  struct winsize size;

  // check for TIOCGWINSZ 
  if (ioctl(fd, TIOCGWINSZ, &size) < 0) { // get window fields
    perror("TIOCGWINSZ error");
    return;
  }
  win->rows = size.ws_row;
  win->cols = size.ws_col;
  win->line_at_top = 0;

  set_erase_char(fd, win);
}

/**
 * Prints message to bottom section of the screen.
 *
 * @param message - the message to print
 * @param curs - cursor
 */
void write_status_message(const char *message, Cursor curs) {
  write(1, PARK, IPARK);
  write(1, CLEAR_LINE, ICLEAR_LINE);
  write(1, message, strlen(message));
  move_to(curs.r, curs.c);
}

/**
 *  Move the cursor to the specified position.
 *
 *  @param line - the row number
 *  @param column - the column number
 */
void move_to(int line, int column) {
  char seq_str[20];

  sprintf(seq_str, "\033[%d;%dH", line + 1, column + 1);
  write(1, seq_str, strlen(seq_str));
}

/******************************************************************************/
/*                            Buffer Prototypes                               */
/******************************************************************************/

/**
 * Initialize the buffer by settings its fields to 0.
 *
 * @param buffer - the buffer to initialize
 */
void init_buffer(Buffer *buffer) {
  buffer->num_lines = 0;
  buffer->cur_line = 0;
  buffer->line_len[0] = 0;
  buffer->line_start[0] = 0;
  buffer->size = 0;
  buffer->index_in_cur_line = 0;
  buffer->index = 0;
}

void save_buffer(const char path[], Buffer buf, char *statusstr) {
  char newline = '\n';
  int fd;
  // char statusstr[80];
  
  fd = creat(path, 0644);
  if (fd != -1) {
    sprintf(statusstr,
            "\"%s\" %dL %dC written",
            path,
            buf.num_lines + 1,
            buf.size);
    write(fd, buf.text, strlen(buf.text));
    if (buf.text[buf.size - 1] != '\n') {
      write(fd, &newline, 1);
    }
    close(fd);
  } else {
      exit(1);
  }
}

void update_buffer_index(Buffer *buffer) {
  int totalchars = 0;
  int i = 0;

  while (i < buffer->cur_line) {
    totalchars += buffer->line_len[i];
    i++;
  }
  totalchars += buffer->index_in_cur_line;
  buffer->index = totalchars;
}

/**
 * Get the lastline position of the window
 */
void get_lastline_in_win(Buffer buffer, Window win, int *lastline) {
  int totallines = 0;
  int i;
  int max_possible = win.rows - 1; // rows minus status line
  
  i = win.line_at_top;
  while (i < buffer.num_lines) {
    if (buffer.line_len[i] <= win.cols) {
      totallines++;
    } else {
      totallines += (int) ceil((double)buffer.line_len[i] / win.cols); 
    }

    if (totallines > max_possible) {
      break;
    } else {
      i++;
    }
  }
  // total > max_possible, so we use previous line, which is i - 1
  *lastline = i - 1;
}

/**
 * Write the updated buffer to the screen
 */
void redraw_buffer(Buffer buffer, Window *win, Cursor *curs) {
  int i;
  int lastline;
  int lastchar;
  int firstchar;
  int line_of_cursor;

  /**
   * If the current position in the buffer, buffer.index, is not within the
   * visible lines of the window, the window must be shifted. The shift might
   * be up or down, depending on whether index is above or below the window.
   *
   * We first need to get the number of the line containing pos. Then we
   * check whether that line is between win.line_at_top and lastline.
   * We need to calculate the difference and shift win.line_at_top that
   * difference, and recalculate lastline, after which we can draw the
   * buffer.
   */

  // Compute the last visible complete text line in the buffer
  get_lastline_in_win(buffer, *win, &lastline);

  // Get the index of the text line containing the insertion position
  line_of_cursor = line_in_buffer(buffer, *win, buffer.index);

  // Check if the window needs to be scrolled
  if (line_of_cursor < win->line_at_top) {
    lastline -= (win->line_at_top - line_of_cursor);
    curs->r += (win->line_at_top - line_of_cursor);
    win->line_at_top = line_of_cursor;
  } else if (line_of_cursor > lastline) {
    win->line_at_top += (line_of_cursor - lastline);
    curs->r -= (line_of_cursor - lastline);
    lastline = line_of_cursor;
  }

  /**
   * Get the first and last characters of the visible screen. The lastchar
   * is the index of the last character that can appear in the
   * window - the slat character in the last visible line. The first char
   * is the start of the line at the top of the screen
   */
  lastchar = buffer.line_start[lastline] + buffer.line_len[lastline];
  firstchar = buffer.line_start[win->line_at_top];

  // Prepare to redraw the window. First clear the scree
  write(1, CLEAR_SCREEN, ICLEAR_SCREEN);

  // Do the redraw
  move_to(0, 0);

  for (i = firstchar; i < lastchar; i++) {
    write(1, &buffer.text[i], 1);
  }
}

void scroll_buffer(Buffer buf, Window win) {
  /**
   * This calculates the position of the first character on the screen
   * as the leftmost character in the current line_at_top, and then
   * calls get_lastline_in_win() to get the index ofhte last text line
   * that can fit in its entirety within the window. It then computes
   * the index ofhte last character in that line.
   *
   * It then clears the scren adn writes the contents of the text buffer,
   * starting at the computed firstchar until the lastchar. The cursor
   * has to be moved to the upper left-hand corner before starting.
   * The caller is responsible for restoring the previous cursor poisition
   */

  int i;
  int lastline;
  int lastchar;
  int firstchar = buf.line_start[win.line_at_top];

  get_lastline_in_win(buf, win, &lastline);
  lastchar = buf.line_start[lastline] + buf.line_len[lastline];

  write(1, CLEAR_SCREEN, ICLEAR_SCREEN);
  move_to(0, 0);

  for (i = firstchar; i < lastchar; i++) {
    write(1, &buf.text[i], 1);
  }
}

int line_in_buffer(Buffer buf, Window win, int pos) {
  int i = 0;

  while (i < buf.num_lines) {
    if (buf.line_start[i] <= pos) {
      i++;
    } else {
      break;
    }
  }
  // If the inserted character is a newline, add
  // the extra line
  if (buf.text[pos] == '\n') {
    i++;
  }
  return i - 1;
}

/**
 * Synchronize buffer, window, cursor when in input mode
 *
 * @return - 0 if success 
 *         - -1 if current line has maxed out characters length
 *         - -2 if the buffer has run out of space
 *         - -3 if the erase char is pressed
 */
int insert(Buffer *buf, Window win, char c) {
  int i;

  if ((c == '\n') && (MAXLINES == buf->num_lines)) {
    return OUT_OF_LINES; // -1
  } else if (buf->size == BUFSIZ) {
    return OUT_OF_MEM; // -2
  }

  if (c == win.erase_char) {
    return UNHANDLEDCHAR; // -3
  }

  for (i = buf->size; i > buf->index; i--) {
    buf->text[i] = buf->text[i - 1];
  }

  buf->text[buf->index] = c; // add new character to the buffer
  buf->size++; // increment the number of characters in the buffer
  buf->index++; // advance the position of the cursor
  buf->line_len[buf->cur_line]++; // increment the number chars in the line

  // the first character sets line count to 1
  if (buf->size == 1) {
    buf->num_lines++;
  }

  if (c == '\n') {
    // Save the length of the line being split by the newline
    int temp = buf->line_len[buf->cur_line]; 

    // the new length of current line is the current index position + 1
    buf->line_len[buf->cur_line] = buf->index_in_cur_line + 1;
    // increse number of lines
    buf->num_lines++;

    // Shift all line starts and liegnts upward in the array, but
    // add 1 to the line starts since they are 1 character further
    // than before because of the new line. Do this from the last line
    // down to curt_line + 1, whilch is the line just after the split line.
    for (i = buf->num_lines - 1; i > buf->cur_line + 1; i--) {
      buf->line_len[i] = buf->line_len[i - 1];
      buf->line_start[i] = buf->line_start[i - 1] + 1;
    }
    // set the start of the new line created here. It is the sum of the
    // start of cur_line plus the length of cur_line.
    buf->line_start[buf->cur_line + 1] = buf->line_start[buf->cur_line]
      + buf->line_len[buf->cur_line];
    buf->cur_line++; // advance to new line
    // The length of the newly created line is the number
    // of characters that were to the right of the current
    // index position
    buf->line_len[buf->cur_line] = temp - buf->line_len[buf->cur_line - 1];
    buf->index_in_cur_line = 0;
  } else if (isprint(c)) { // non-newline character
      buf->index_in_cur_line++; // advance index in line
      // increment all line starts after this line
      for (i = buf->cur_line + 1; i < buf->num_lines; i++) {
        buf->line_start[i]++;
      }
  } else {
      return UNHANDLEDCHAR;
  }
  return 0;
}

/**
 * Wrapper function to handle user input when in input mode
 *
 * @return - 0 error
 *         - 1 success 
 */
int handle_insertion(Buffer *buf, Window *win, Cursor *curs, char c) {
  int retvalue;
  
  // insert typed char and echo it
  retvalue = insert(buf, *win, c);
  if (retvalue < 0) {
    if (retvalue == OUT_OF_LINES) {
      write_status_message(MAXLINES_MSSGE, *curs);
    } else if (retvalue == OUT_OF_MEM) {
      write_status_message(OUT_OF_MEM_MSSGE, *curs);
    } else if (retvalue == UNHANDLEDCHAR) {
      write_status_message(UNHANDLEDCHAR_MSSGE, *curs);
      return retvalue;
    }
    return 0;
  }
  advance_cursor(curs, *win, c);
  redraw_buffer(*buf, win, curs);
  move_to(curs->r, curs->c);

  return 1;
}


/******************************************************************************/
/*                             Cursor Functions                               */
/******************************************************************************/
void get_cursor_at(
  Buffer buf,
  Window win,
  int index,
  int lineno,
  Cursor *curs) {

  int total_lines_before = 0;
  int rows_in_current_textline = 0;
  int i;

  // the first line is the one at th etop of the window, whose index is
  // win.line_at_top , initially 0
  for (i = win.line_at_top; i < lineno; i++) {
    if (buf.line_len[i] < win.cols) {
      total_lines_before++;
    } else {
      total_lines_before += (int) ceil((double)buf.line_len[i] / win.cols);
    }
  }
  rows_in_current_textline = index / win.cols;
  curs->r = total_lines_before + rows_in_current_textline;
  curs->c = index - rows_in_current_textline * win.cols;
}

/**
 * Advance the cursor depending on ch 
 *
 * @param cursor
 * @param win
 * @param ch character pressed
 */
void advance_cursor(Cursor *cursor, Window win, char ch) {
  if (ch == '\n') {
    // when 'Enter' is pressed, place cursor in the beggining of
    // the next row
    cursor->r++;
    cursor->c = 0;
  } else {
    cursor->c++;
    if (cursor->c == win.cols) { // wrap the line
      cursor->c = 0;
      cursor->r++;
    }
  }
}

/**
 * Initialize the cursor by settings its fields to 0.
 *
 * @param cursor - the cursor to initialize
 */
void init_cursor(Cursor *cursor) {
  cursor->r = 0;
  cursor->c = 0;
}

void show_cursor(
  Buffer buf,
  Window win,
  Cursor cursor,
  int index_in_line,
  int line_number) {

  char curs_str[80];
  sprintf(
      curs_str,
      "Cursor: [%d,%d] line index: %d win topline: %d "
      "buf #lines: %d",
      cursor.r + 1,
      cursor.c + 1,
      line_number,
      win.line_at_top,
      buf.num_lines);
  write(1, PARK, IPARK);
  write(1, CLEAR_LINE, ICLEAR_LINE);
  write(1, curs_str, strlen(curs_str));
  move_to(cursor.r, cursor.c);
}

void move_up(Buffer *buf, Window *win, Cursor *curs) {
  // if buf.cur_line == 0, we cannot go up further
  if (buf->cur_line > 0) {
    buf->cur_line--;

    if (buf->index_in_cur_line >= buf->line_len[buf->cur_line]) {
      buf->index_in_cur_line = buf->line_len[buf->cur_line] - 1;
    }

    if (buf->cur_line >= win->line_at_top) {
      get_cursor_at(*buf, *win, buf->index_in_cur_line, buf->cur_line, curs);
    } else {
      win->line_at_top = buf->cur_line;
      get_cursor_at(*buf, *win, buf->index_in_cur_line, buf->cur_line, curs);
      scroll_buffer(*buf, *win);
    }
    move_to(curs->r, curs->c);
  }
}

void move_down(Buffer *buf, Window *win, Cursor *curs) {
  int lastline;

  if (buf->cur_line < buf->num_lines - 1) {
    buf->cur_line++;
    // Check whether the cursor would be past the rightmost character
    // of the now current line. If so, position it jsut past the rightmost
    // character
    if (buf->index_in_cur_line >= buf->line_len[buf->cur_line]) {
      buf->index_in_cur_line = buf->line_len[buf->cur_line] - 1;
    }

    get_lastline_in_win(*buf, *win, &lastline);

    if (buf->cur_line > lastline) {
      // need to scroll
      win->line_at_top += buf->cur_line - lastline;
      get_cursor_at(*buf, *win, buf->index_in_cur_line, buf->cur_line, curs);
      scroll_buffer(*buf, *win);
    } else {
        get_cursor_at(*buf, *win, buf->index_in_cur_line, buf->cur_line, curs);
    }
    move_to(curs->r, curs->c);
  }
}

void move_right(Buffer *buf, Window *win, Cursor *curs) {
  if ((buf->index_in_cur_line < buf->line_len[buf->cur_line] - 1)
    || (buf->index_in_cur_line < buf->line_len[buf->cur_line])
    && (buf->cur_line == buf->num_lines - 1)) {

    buf->index_in_cur_line++;
    if (buf->index_in_cur_line % win->cols == 0) {
      curs->r++;
      curs->c = 0;
      if (curs->r > win->rows - 2) {
        win->line_at_top += curs->r - (win->rows - 2);
        scroll_buffer(*buf, *win);
      }
      move_to(curs->r, curs->c);
    } else {
        curs->c++;
        write(1, RIGHT, IRIGHT);
    }
  }
}

void move_left(Buffer *buf, Window *win, Cursor *curs) {
  if (buf->index_in_cur_line > 0) {
    if (buf->index_in_cur_line % win->cols == 0) {
      curs->r--;
      curs->c = win->cols - 1;
      move_to(curs->r, curs->c);
    } else {
        curs->c--;
        write(1, LEFT, ILEFT);
    }
    buf->index_in_cur_line--;
  }
}

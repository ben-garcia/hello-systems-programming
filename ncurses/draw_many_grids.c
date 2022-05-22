// Chapter 6 Event Driven Programming
// page 15
//
// This program demonstrates how to use multiple windows. Note that this program
// does not use the standard screen

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CENTERY (LINES / 2 - 2)  // the middle line in terminal
#define CENTERX (COLS / 2 - 2)   // the middle column in terminal
#define NUMROWS (LINES / 2)      // number of rows we use
#define NUMCOLS (COLS / 2)       // number of columns we use
#define REPEATS 5
#define GRIDCHARS ".*@+#"  // should ahve ERPEATS many chars

int main(int argc, char *argv[]) {
  char MESSAGE[] =
      "Type the character of the grid to print it forward, 'q' to exit:";
  int length, i, j, k;
  WINDOW *message_win;
  WINDOW *windows[REPEATS];
  char gridchar[REPEATS] = GRIDCHARS;
  int row_shift, col_shift;
  int ch;

  initscr();  // initialize screen
  noecho();   // turn off character echo

  // make sure that the window is wide enough for message at the bottom
  length = strlen(MESSAGE);
  if (length > COLS - 2) {
    endwin();
    printf("This program needs a wider window.\n");
    exit(1);
  }

  // calculate the amount by which we shift each window when we draw it
  row_shift = (LINES - NUMROWS) / 5;
  col_shift = (COLS - NUMCOLS) / 5;

  // in this loop, we create a new window, fill it with a grid of
  // unique characters
  for (j = 0; j < REPEATS; j++) {
    // create a new window at an offset from (0,0) determiend by the
    // row and column shift.
    windows[j] = newwin(NUMROWS, NUMCOLS, row_shift * j, col_shift * j);
    if (windows[j] == NULL) {
      endwin();
      fprintf(stderr, "Error creating window\n");
      exit(1);
    }

    // draw each grid row as string into windows[j]
    for (i = 0; i < NUMROWS; i++) {
      for (k = 0; k < NUMCOLS; k++) {
        wmove(windows[j], i, k);
        if (waddch(windows[j], gridchar[j]) == ERR) {
          // ignore the error; it means we are in the bottom right corner
          // of the window and the cursor was adnvaced to a non-window position
        }
      }
      // update the virtual screen with this window's content
      wnoutrefresh(windows[j]);
    }
  }
  // now send the virtual screen to the physical screen
  doupdate();

  // create a window to hold a message and put it in the bottom row
  message_win = newwin(1, COLS, LINES - 1, 0);

  // write the message into the window; mvwasddstr positions the cursor
  mvwaddstr(message_win, 0, 0, MESSAGE);
  wrefresh(message_win);

  while (1) {
    // read a chraacter from the message window, not from stdscr. The
    // call to wgetch forces a refresh on its window argument. If we refresh
    // stdscr, our grids will disappear.
    ch = wgetch(message_win);  // wait for the user to type something
    if (ch == 'q')             //  time to quit
      break;
    // check if they typed a grid character
    for (j = 0; j < REPEATS; j++) {
      if (ch == gridchar[j]) {
        wmove(message_win, 0, length);  // move cursor to bottom
        touchwin(windows[j]);           // force the update
        wrefresh(windows[j]);           // refresh, bringing it forward
        break;
      }
    }
  }

  clear();   // clear the screen
  endwin();  // delete curses window and quit
  return 0;
}

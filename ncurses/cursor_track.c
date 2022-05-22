// Chapter 6 Event Driven Programming
// page 13

// Lines adn COLS are NCurses variables that get initialized when
// initscr() is called.

#include <ncurses.h>
#include <string.h>

#define CENTERY (LINES / 2 - 2)  // the middle line in terminal
#define CENTERX (COLS / 2 - 2)   // the middle column in terminal

int main(int argc, char *arg[]) {
  int x, y;  // to retrieve coordinates of cursor
  int ch;    // to receive user input character
  int r, c;  // to store coordinates of cursor

  // A string to display in the "status bar" at the bottom of the screen
  char MESSAGE[] =
      "Use the arrow keys to move the cursor. "
      "Press F1 to exit";
  int length = strlen(MESSAGE);  // compute this once

  initscr();             // initialize screen
  clear();               // clear the screen
  noecho();              // turn off character echo
  cbreak();              // disable line buffering
  keypad(stdscr, TRUE);  // turn on function keys

  // move to bottom left corner of screen, write message there
  move(LINES - 1, 0);
  addstr(MESSAGE);

  // start the cursor at the screen center
  r = CENTERY;
  c = CENTERX;
  move(r, c);

  // print the user's coordinates at the lower right
  move(LINES - 1, COLS - 8);
  printw("(%02d,%02d)", r, c);
  refresh();

  // then move the cursor to the center
  move(r, c);

  // Rpeatedly wait for user input using getch(). Because we turned off
  // echo and put curses into cbreak mode, getch() will return without
  // needing to get a newline char and will not echo the character.
  // When the user presses the F1 key, the program quits.
  while ((ch = getch()) != KEY_F(1)) {
    switch (ch) {
      // When keypad() turns on function keys, the arrow keys are enalbed
      // and are named KEY_X, where X is LEFT, RIGHT, etc.
      // This switch update the row or column as needed, modulo COLS
      // horizontally to wrap, and LINES - 1 to wrap vertically without
      // entering the sanctity of the status bar.
      case KEY_LEFT:
        c = (0 == c) ? COLS - 1 : c - 1;
        break;
      case KEY_RIGHT:
        c = (c == COLS - 1) ? 0 : c + 1;
        break;
      case KEY_UP:
        r = (0 == r) ? LINES - 2 : r - 1;
        break;
      case KEY_DOWN:
        r = (r == LINES - 2) ? 0 : r + 1;
        break;
    }

    // Now we move the cursor to the new position, get its coordinates
    // and then move to the lower right to print the new position
    move(r, c);
    getyx(stdscr, y, x);
    move(LINES - 1, COLS - 8);
    printw("(%02d,%02d)", y, x);
    refresh();
    // now me have to move back to wshere we were the cursor was
    // in the lower right after the printw()
    move(r, c);
  }

  endwin();  // exit curses
  return 0;
}

#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>

#define CENTERY (LINES / 2 - 2) // the middle line in terminal
#define CENTERX (COLS / 2 - 2) // the middle column in terminal

void add_happy_face(int *y, int *x) {
  int original_y = *y;

  addstr(" ^ ^ "); move(++(*y), *x);
  addstr(" o o "); move(++(*y), *x);
  addstr("  ^  "); move(++(*y), *x);
  addstr("\\___/"); move(++(*y), *x);
  addstr("     ");

  *y = original_y;
  *x = (*x) + 5;

  move(*y, *x);
}

int main(int argc, char *argv[]) {
  int r;
  int c;
  char MESSAGE[] = "Press any character to exit:";
  int length;
  FILE *fp; // for writing window contents
  
  length = strlen(MESSAGE);

  if (argc < 2) {
    printf("usage: %s window - file\n", argv[0]);
    return 0;
  }

  fp = fopen(argv[1], "w");
  if (fp == NULL) {
    printf("Error opening %s for writing.\n", argv[1]);
    return 0;
  }

  initscr(); // Initialize curses library and the drawing screen
  clear(); // Clear the screen
  
  // Move to bottom of screen and post message to display
  move(LINES - 1, 0);
  addstr(MESSAGE);

  // move to center of screen - width of face
  r = CENTERY;
  c = CENTERX - 5;
  move(r, c);
  add_happy_face(&r, &c);
  add_happy_face(&r, &c);
  add_happy_face(&r, &c);

  // Park cursor at bototm at the right side of the message
  move(LINES - 1, length);
  refresh();

  // Write the standard screen to a file
  if (putwin(stdscr, fp) == ERR) {
    printw("Error saving window.\n");
  }
  fclose(fp);

  getch(); // wait for the user to type something
  clear(); // clear the screen
  endwin(); // delete curses window and quit

  return 0;
}

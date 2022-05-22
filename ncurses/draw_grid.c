// Chapter 6 Event Driven Programming
// Program that draws a grid of periods centered on the screen
// page 9

#include <curses.h>
#include <string.h>

#define CENTERY (LINES / 2 - 2) // the middle line in terminal
#define CENTERX (COLS / 2 - 2) // the middle column in terminal
#define NUMROWS (LINES / 2)
#define NUMCOLS (COLS / 2)

int main() {
  int r;
  int c;
  char MESSAGE[] = "Press any character to exit:";
  int length = strlen(MESSAGE);
  int i;
  int j;

  initscr(); // Initialize screen
  clear();
  noecho(); // turn off character echo
  
  char grid[NUMROWS][NUMCOLS];

  for (i = 0; i < NUMROWS; i++) {
    for (j = 0; j < NUMCOLS - 1; j++) {
      grid[i][j] = '.';
    }
    grid[i][NUMCOLS - 1] = '\0';
  }

  // move to center to draw
  r = CENTERY - (NUMROWS / 2);
  c = CENTERX - (NUMCOLS / 2);
  move(r, c);

  // Draw each row of grid as a string
  for (i = 0; i < NUMROWS; i++) {
    mvaddstr(r + i, c, grid[i]);
  }

  // Move to bottom of screen, post mesage to display
  move(LINES - 1, 0);
  addstr(MESSAGE);

  getch(); // wait for the user to type something
  clear(); // clear the screen
  endwin(); // delete curses window and quit

  return 0;
}

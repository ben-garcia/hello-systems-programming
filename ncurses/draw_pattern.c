#include <stdio.h>
#include <curses.h>

int main() {
  char pattern[] = "1234567890";
  int i;

  // Initialize NCurses and clear the screen
  initscr();
  clear();

  // This will wrap across the screen
  move(LINES / 2, 0);

  for (i = 1; i <= 8; i++) {
    addstr(pattern);
    addch(' ');
  }

  // Park the cursor at bottom
  move(LINES - 1, 0);
  addstr("Type and char to quit:");
  refresh(); // not needed

  // Wait for the user to type something, otherwise the screen will clear
  getch();
  endwin();

  return 0;
}

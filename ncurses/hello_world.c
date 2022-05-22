#include <ncurses.h>

int main() {
  initscr(); // initialize the library
  printw("Hello World !!!\n"); // print at cursor
  refresh(); // update screen (unnecessary)
  getch(); // wait for a keypres
  endwin(); // clean up and quit curses

  return 0;
}

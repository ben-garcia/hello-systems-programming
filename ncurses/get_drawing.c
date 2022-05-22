// Chapter 6 Event Drive Programming
// Program that can read any file created by an NCurses program that saved 
// data using putwin() and print it to screen. page 12

#include <stdio.h>
#include <string.h>
#include <curses.h>

int main(int argc, char *argv[]) {
  FILE *fp;
  WINDOW *win;

  if (argc < 2) {
    printf("usage: %s window - file\n", argv[0]);
    return 0;
  }

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    printf("Error opening %s\n", argv[1]);
    return 0;
  }

  initscr(); // Initialize curses library and the drawing screen
  cbreak(); // So that the character is available immediately
  noecho(); // Turn off echo
  clear(); // Clear the screen

  move(LINES - 1, 0);
  addstr("Enter a character to see the faces:");
  getch();

  win = getwin(fp);
  if (win == NULL) {
    clear();
    move(LINES - 2, 0);
    printw(
      "The file %s was not created using putwin()."
      " Type any character to exit.\n",
      argv[1]
    );
  }
  wrefresh(win);
  fclose(fp);

  getch(); // wait for the user to type something
  clear(); // clear the screen
  endwin(); // delete curses window and quit
  
  return 0; 
}

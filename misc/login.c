// Chapter 4 Control of Disk and Terminal I/O
// Program that prompts the user to enter a password and makes the typing
// invisible when the password in entered, as the login program does. The
// program turns off the echo switch and resets it afterward.
// page 40

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

int main(int argc, char *argv[]) {
  struct termios info;
  struct termios orig;
  char username[33];
  char passwd[33];
  FILE *fp;

  // get a FILE* to the control terminal - don't assume stdin
  if ((fp = fopen(ctermid(NULL), "r+")) == NULL) {
    return 1;
  }

  printf("login: "); // display message
  fgets(username, 32, stdin); // get user's input
  
  // now turn off echo 
  tcgetattr(fileno(fp), &info); // get current terminal state
  orig = info; // save a copy of it
  info.c_lflag &= ~ECHO; // turn off echo bit
  tcsetattr(fileno(fp), TCSANOW, &info); // set state in line discipline

  printf("password: ");
  fgets(passwd, 32, stdin); // get user's non-echoed typing
  
  tcsetattr(fileno(fp), TCSANOW, &orig); // restore saved settings

  printf("\n"); // print a fake message

  printf("Last Login: Tue Apr 31 21:29:54 2088 from the twilight zone.\n");
  return 0;
}

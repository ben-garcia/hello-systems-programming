// Chapter 4 Control of Disk and Terminal I/O
// 4.4.1 An Experiment
// Rewriting the simple I/O program from Chapter 1 so that
// is does use the C Standard I/O Library getchar() and printf() functions
// on page 26
//
// as you type a character is's being stored in a buffer to be displayed
// after the 'Enter' key is pressed.
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char * argv[]) {
  int ch;

  printf("Type any characters followd by the 'Enter' key.");
  printf(" Use Ctrl-D to exit.\n");

  while ((ch = fgetc(stdin)) != EOF) {
    printf("char = '%c' code = %03d\n", ch, ch);
  }

  return 0;
}


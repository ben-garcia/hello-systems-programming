// Chapter 7 Process Architecture and Control
// A program to display the virtual memory boundaries
// on page 15

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/******************************************************************************/
/*                                 Global Constants                           */
/******************************************************************************/
#define SHW_ADR(ID, I)                                               \
  printf("    %s \t is at addr:%8X\t%20u\n", ID, (unsigned int)(&I), \
         (unsigned int)(&I))

// These are system variables, defined in the unistd.h header file
extern int etext, edata, end;

char *cptr = "Hello World\n";  // cptr is an initialized global
char buffer1[40];              // uninitialized global

void showit(char *);  // function prototype - has no storage

int main(int argc, char *argv[], char *envp[]) {
  int i = 0;        // on stack
  static int diff;  // global in uninitialized data segment

  strcpy(buffer1, "   Layout of virtual memory\n");
  write(1, buffer1, strlen(buffer1) + 1);

  printf("Adr etext: %8X \t Adr edata: %8X \t Adr end: %8X \n\n",
         (unsigned int)&etext, (unsigned int)&edata, (unsigned int)&end);
  printf("       ID \t             HEX_ADDR\t       DECIMAL_ADDR\n");

  SHW_ADR("main", main);
  SHW_ADR("showit", showit);
  SHW_ADR("etext", etext);

  diff = (int)&showit - (int)&main;
  printf("           showit is %d bytes above main\n", diff);

  SHW_ADR("cptr", cptr);

  diff = (int)&cptr - (int)&showit;
  printf("          cptr is %d bytes above showit\n", diff);

  SHW_ADR("buffer1", buffer1);
  SHW_ADR("diff", diff);
  SHW_ADR("edata", edata);
  SHW_ADR("buffer1", buffer1);
  SHW_ADR("end", end);
  SHW_ADR("argc", argc);
  SHW_ADR("argv", argv);
  SHW_ADR("envp", envp);
  SHW_ADR("i", i);

  showit(cptr);
  return 0;
}

/******************************************************************************/
void showit(char *p) {
  char *buffer2;
  SHW_ADR("buffer2", buffer2);

  if ((buffer2 = (char *)malloc((unsigned)(strlen(p) + 1))) != NULL) {
    strcpy(buffer2, p);
    printf("%s", buffer2);
    free(buffer2);
  } else {
    printf("Allocation error.\n");
    exit(1);
  }
}

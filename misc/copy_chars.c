// Chapter 4 Control of Disk and Terminal I/O
// 4.4.1 An Experiment
// Rewriting the simple I/O program from Chapter 1 so that
// is does not use C FILE stream using the kernel's
// read() and write() system calls on page 24

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
  char inbuf;
  char prompt[] =
        "Type any character followd by the 'Enter' Key. "
        "Use Ctrl-D to exit.\n";

  if (write(1, prompt, strlen(prompt)) == -1) {
    write(1, "write failed\n", 13);
    exit(1);
  }

  while (read(0, &inbuf, 1) > 0) {
    write(1, &inbuf, 1);
  }

  return 0;
}


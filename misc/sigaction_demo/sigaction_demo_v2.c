// Chapter 5 Interactive Programs and Signals
// This program will demonstrate the use of the sigaction structure for
// old-style handlers as well as the new-style handlers.

/**
 * Usage:
 *        sigaction_demo [ reset | noblock | restart ]
 *
 * i.e., 0 or more of the words reset, noblock, and reset may appear
 * on the command line, and multiple instances of the same word as the same
 * effect as a single instance.
 *
 * reset turns on SA_RESETHAND
 * noblock turns on SA_NODEFER
 * restart turns on SA_RESTART
 *
 * NOTES
 *  (1) If you supple the world "reset" on the command line it will set the 
 *      handling to SIG_DFL for signals that arrive when the process is
 *      in the handler. If noblock is also set, the signal will ahve the
 *      default behavior immediately. If is it not set, the default will delay
 *      until after the handler exits. If noblock is set but reset is
 *      not, it will recursively enter the handler.
 * (2) The interrupt_handler pruposely delays for a few seconds in order to
 *      give the user time to enter a few interrupts on the keyboard.
 * (3) interrupt_handler is the handler for both SIGINT and SIGQUIT, so if it
 *     is not reset, neither Ctrl C nor Ctrl \ will kill it.
 * (4) It will ask you to enter the numberic values of signals to block. If
 *     you don't give any, no signals are blocked.
 *
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define INPUTLEN 100

int main(int argc, char *argv[]) {
  struct sigaction new_handler; // new settings
  sigset_t blocked; // set of blocked sigs
  void interrupt_handler(); // the handler
  char x[INPUTLEN];
  int flags = 0;
  int signo;
  int n;
  char s[] = "Entered text: ";
  int s_len = strlen(s);

  while (argc > 1) {
    if (strncmp("reset", argv[argc - 1], strlen(argv[argc - 1])) == 0) {
      flags |= SA_RESETHAND;
    } else if ((strncmp("noblock", argv[argc - 1], strlen(argv[argc - 1]))) == 0) {
      flags |= SA_NODEFER;
    } else if ((strncmp("restart", argv[argc - 1], strlen(argv[argc - 1]))) == 0) {
      flags |= SA_NODEFER;
    }
    argc--;
  }
  // load these two members first
  new_handler.sa_sigaction = interrupt_handler; // handler function
  new_handler.sa_flags = SA_SIGINFO | flags; // new style handler

  // then build the list of blocked signals
  sigemptyset(&blocked);

  printf("Type the numeric value of a signal to block (0 to stop):");

  while (1) {
    scanf("%d", &signo);
    if (signo == 0) {
      break;
    }
    sigaddset(&blocked, signo); // add signo to list
    printf("next signal number (0 to stop): ");
  }
  new_handler.sa_mask = blocked; // store blockmask
  
  // install the handler as the SIGINT handler
  if (sigaction(SIGINT, &new_handler, NULL) == -1) {
    perror("sigaction");
  }
  else if (sigaction(SIGQUIT, &new_handler, NULL) == -1) {
    // if successful, install handler as the SIGQUIT handler too
    perror("sigaction");
  } else {
    while (1) {
      x[0] = '\0';
      tcflush(0, TCIOFLUSH);
      printf("Enter text then <RET>: (quit to quit)\n");
      n = read(0, &x, INPUTLEN);
      if (n == -1 && errno == EINTR) {
        printf("read cll was interrupted\n");
        x[0] = '\0';
        write(1, &x, n + 1);
      } else if (strncmp("quit", x, 4) == 0) {
        break;
      } else {
        x[n] = '\0';
        write(1, &s, s_len);
        write(1, &x, n + 1);
        printf("\n");
      }
    }
  }
  return 0;
}

void interrupt_handler(int signo, siginfo_t *info, void *context) {
  int local_id; // stores a number to uniquely identify signal
  time_t time_now; // current time, used to generate id
  static int ticker = 0; // use for id also
  struct tm *tp;

  time(&time_now);
  tp = localtime(&time_now);
  local_id = 36000 * tp->tm_hour + 600 * tp->tm_min + 10 * tp->tm_sec +
             ticker++ % 10;
  printf("Entered handler: sig = %d \tid = %d\n", info->si_signo, local_id);
  sleep(3);
  printf("Leaving handler: sig = %d \tid = %d\n", info->si_signo, local_id);
}

// Chapter 7 Process Architecture and Control
// This program demonstrates how the parent and child can work in locksep using
// the SIGUSR1 signal. It also shows that the child process inherits the open
// files of the parent, and that writes by the child and parent to the
// same desriptor advance the shared file position pointer.
// on page 26

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void p_action(int sig);
void c_action(int sig);
void on_sigint(int sig);

int n_signals = 0;
volatile sig_atomic_t sigint_received = 0;

int main(int argc, char *argv[]) {
  pid_t pid;
  pid_t ppid;

  static struct sigaction parent_act;
  static struct sigaction child_act;
  int fd;
  int counter = 0;
  char child_buf[40];
  char parent_buf[40];

  if (argc < 2) {
    printf("usage: synchdemo2 filename\n");
    exit(1);
  }

  if ((fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1) {
    perror(argv[1]);
    exit(1);
  }

  switch (pid = fork()) {
    case -1:
      perror("failed");
      exit(1);
    case 0:
      // set action for child
      child_act.sa_handler = c_action;
      sigaction(SIGUSR1, &child_act, NULL);
      ppid = getppid();  // get parent id

      for (;;) {
        sprintf(child_buf, "Child counter = %d\n", counter++);
        write(fd, child_buf, strlen(child_buf));
        printf("Sending signal to parent --- ");
        fflush(stdout);
        kill(ppid, SIGUSR1);
        sleep(3);
      }
    default:
      // set SIGUSR1 action for parent
      parent_act.sa_handler = p_action;
      sigaction(SIGUSR1, &parent_act, NULL);

      // set SIGINT handler for parent
      parent_act.sa_handler = on_sigint;
      sigaction(SIGINT, &parent_act, NULL);

      for (;;) {
        sleep(3);
        sprintf(parent_buf, "Parent counter = %d\n", counter++);
        write(fd, parent_buf, strlen(parent_buf));
        printf("Sending signal to child --- ");
        fflush(stdout);
        kill(pid, SIGUSR1);

        if (sigint_received) {
          close(fd);
          exit(0);
        }
      }
  }
}

void p_action(int sig) { printf("Parent caught signal %d\n", ++n_signals); }

void c_action(int sig) { printf("Child caught signal %d\n", ++n_signals); }

void on_sigint(int sig) { sigint_received = 1; }

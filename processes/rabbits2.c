// Chapter 7 Process Architecture and Control
// Instead of calling wait() or waitpid(), a process can establish a SIGCHLD
// handler that will run when a child terminates. The SIGCHLD handler can then
// call wait(). This frees the process form having to poll the wait() function.
// It only calls wait() when it is guaranteed to succeed. This program
// demonstrates how this works.
// on page 40

#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#define NUM_CHILDREN 5
#define SLEEPTIME 30

/**
 *  child() The code that is executed by each child process
 *
 *  All this does is register the SIGINT signal handler and then
 *  sleep SLEEPTIME seconds. Is a child is delivered a SIGINT, it
 *  exits with the exit code 99. See on_sigint() below.
 */
void child();

/**
 * on_sigint() Signal handler for SIGINT
 *
 * All it does is call exit with a code of 99.
 */
void on_sigint(int signo);

/**
 * on_signchld() Signal handler for SIGCHLD
 *
 * This calls wait() to retrieve the status of the terminated child and
 * get its pid. These are both stored in global variables that the parent
 * can access in the main program. It also sets a global flag.
 */
void on_sigchld(int signum);

// These variables are declared with the volatile qualifier to tell the
// compiler that they are used in a singal handler and their values
// change asynchronously. This prevents the compiler from performing an
// optimization that might corrupt the program state. All three are shared
// by the main parent process and the SIGCHLD handler.
volatile int status;
volatile pid_t pid;
volatile sig_atomic_t child_terminated;

int main(int argc, char *argv[]) {
  int count = 0;
  const int num_children = NUM_CHILDREN;
  int i;
  struct sigaction new_handler;  // for installing handlers

  printf("About to create many little rabbits...\n");
  for (i = 0; i < num_children; i++) {
    if ((pid = fork()) == -1) {
      perror("fork");
      exit(-1);
    } else if (pid == 0) {  // child code
      // Close standard output so that children don't print
      // parent's output again.
      close(1);
      child();
      exit(1);
    } else {  // parent code
      if (i == 0) {
        printf("Another ");
      } else if (i < num_children - 1) {
        printf("and another ");
      } else {
        printf("and another.\n");
      }
    }
  }

  // parent continues here
  // Set up signal handling
  new_handler.sa_handler = on_sigchld;
  sigemptyset(&new_handler.sa_mask);
  if (sigaction(SIGCHLD, &new_handler, NULL) == -1) {
    perror("sigaction");
    return 1;
  }

  // Enter a loop in which work could happen wile the global flag
  // is checked to see if any child has terminated.
  child_terminated = 0;  // set flag to 0
  while (count < num_children) {
    if (child_terminated) {
      if (WIFEXITED(status)) {
        printf("Rabbit %d died with code %d.\n", pid, WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("Rabbit %d was killed by signal %d.\n", pid, WTERMSIG(status));
      } else {
        printf("Rabbit %d dies with status %d.\n", pid, status);
        child_terminated = 0;
        count++;
      }
    } else {
      // do something useful here. for now just delay a bit
      sleep(1);
    }
  }

  printf("All rabbits ahve terminated and been laid to rest.\n");
  return 0;  // main returns; child never reaches here
}

void on_sigint(int signo) { exit(99); }

void child() {
  struct sigaction new_handler;

  new_handler.sa_handler = on_sigint;
  sigemptyset(&new_handler.sa_mask);
  if (sigaction(SIGINT, &new_handler, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  sleep(SLEEPTIME);
}

void on_sigchld(int signum) {
  int child_status;

  if ((pid = wait(&child_status)) == -1) {
    perror("wait failed");
  }
  child_terminated = 1;
  status = child_status;
}

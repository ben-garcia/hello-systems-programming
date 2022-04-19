#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define _GNU_SOURCE
#include <utmp.h>
#include <fcntl.h>
#include <time.h>

// prototypes
void show_time(long);
void show_info(struct utmp *);

int main(int argc, char *argv[]) {
  struct utmp ut_buffer;
  struct utmp *ut_buffer_pointer;
  int ut_mp_fd;

  if ((argc > 1) && (strcmp(argv[1], "twmp")) == 0) {
    utmpname(_PATH_WTMP);
  } else {
    utmpname(_PATH_UTMP);
  }

  setutent();

  while((getutent_r(&ut_buffer, &ut_buffer_pointer)) == 0) {
    show_info(&ut_buffer);
  }

  endutent();

  return 0;
}

/**
  * Displays the contents of the utmp struct only if a user
  * login, with time in uman readable form, and host if
  * not null
  */
void show_info(struct utmp *ut_buffer_pointer) {
  if (ut_buffer_pointer->ut_type != USER_PROCESS) {
    return;
  }

  printf("%-8.8s", ut_buffer_pointer->ut_name); // the logname
  printf("_");
  printf("%-8.8s", ut_buffer_pointer->ut_line); // the tty
  printf("_");

  show_time(ut_buffer_pointer->ut_time); // login time
  printf("_");

  if (ut_buffer_pointer->ut_host[0] != '\0') { // the host
    printf("_(%s)", ut_buffer_pointer->ut_host);
  }

  printf("\n");
}

/*
 * Displays time in a format fit for human consumption
 * uses ctime to build a string then pcks parts out of it
 * Note: %12.12s prints a string 12 chars wide and LIMITS
 * it ot 12 chars.
 */
void show_time(long time_eval) {
  char *time_str = ctime(&time_eval);
  // string looks like "Sat Sep  3 16:43:29 EDT 2011"
  
  // print 12 chars starting at char 4
  printf("%12.12s", time_str + 4);
  
}

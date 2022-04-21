#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utmp_utils.h"


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

int main(int argc, char *argv[]) {
  utmp_record *ut_buf_ptr; // pointer to a utmp record

  if (open_utmp(UTMP_FILE) == -1) {
    perror(UTMP_FILE);
    exit(1);
  }

  while ((ut_buf_ptr = next_utmp()) != NULL_UTMP_RECORD_PTR) {
    show_info(ut_buf_ptr);
  }

  close_utmp();

  return 0;
}

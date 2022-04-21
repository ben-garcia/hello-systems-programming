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
  struct utmp ut_buffer; // stores a single utmp record
  int file_descriptor; // file descriptor for utmp file
  int ut_size = sizeof(ut_buffer);
  int ut_line_size = sizeof(ut_buffer.ut_line);

  if (argc < 3) { // check usage
    fprintf(stderr, "usage: %s <utmp-file> <line>\n", argv[0]);
    exit(1);
  }

  // try to open utmp file
  if ((file_descriptor = open(argv[1], O_RDWR)) == -1) {
    fprintf(stderr, "Cannot open %s\n", argv[1]);
    exit(1);
  }

  // If the line is longer than a ut_line permits do not
  // continue
  if (strlen(argv[2]) >= UT_LINESIZE) {
    fprintf(stderr, "Improper argument:%s\n", argv[1]);
    exit(1);
  }

  while (read(file_descriptor, &ut_buffer, ut_size) == ut_size) {
    if ((strncmp(ut_buffer.ut_line, argv[2], ut_line_size) == 0)
        && (ut_buffer.ut_user[0] == '\0')) {
      ut_buffer.ut_type = DEAD_PROCESS;
      ut_buffer.ut_user[0] = '\0';
      ut_buffer.ut_host[0] = '\0';

      if (gettimeofday((struct timeval *)&ut_buffer.ut_tv, NULL) == 0) {
        if (lseek(file_descriptor, -ut_size, SEEK_CUR) != -1) {
          if (write(file_descriptor, &ut_buffer, ut_size) != ut_size) {
           exit(1);
          }
        }
      } else {
        fprintf(stderr, "Error getting time of day\n");
        exit(1);
      }

      break;
    }
  }
  close(file_descriptor);

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

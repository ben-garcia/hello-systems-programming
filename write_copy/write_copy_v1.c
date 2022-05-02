// Chapter 4 Control of Disk and Terminal I/O
// version 1 on page 20
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <utmp.h>
#include <time.h>

char* get_user_tty(char *logname);
void create_message(char buf[]);


int main(int argc, char *argv[]) {
  int fd;
  char buf[BUFSIZ];
  char *user_tty;
  char eof[] = "EOF\n";

  if (argc < 2) {
    fprintf(stderr, "usage: write1 username\n");
    exit(1);
  }

  if ((user_tty = get_user_tty(argv[1])) == NULL) {
    fprintf(stderr, "User %s is not logged in.\n", argv[1]);
    return 1;
  }

  sprintf(buf, "/dev/%s", user_tty);

  fd = open(buf, O_WRONLY);
  if (fd == -1) {
    perror(buf);
    exit(1);
  }

  create_message(buf);

  if (write(fd, buf, strlen(buf)) == -1) {
    perror("write");
    close(fd);
    exit(1);
  }

  while (fgets(buf, BUFSIZ, stdin) != NULL) {
    if (write(fd, buf, strlen(buf)) == -1) {
      break;  
    }
  }

  write(fd, eof, strlen(eof));
  close(fd);
  return 0;
}

char *get_user_tty(char *logname) {
  static struct utmp utrec;
  int utrec_size = sizeof(utrec);
  int utmp_fd;
  int namelen = sizeof(utrec.ut_name);
  char *retval = NULL;

  if ((utmp_fd = open(UTMP_FILE, O_RDONLY)) == -1) {
    return NULL;
  }

  while ((read(utmp_fd, &utrec, utrec_size)) == utrec_size) {
    if (strncmp(logname, utrec.ut_name, namelen) == 0) {
      retval = utrec.ut_line;
      break;
    }
  }

  close(utmp_fd);
  return retval;
}

void create_message(char buf[]) {
  char *sender_tty;
  char *sender_name;
  char sender_host[256];
  time_t now;
  struct tm *timeval;

  sender_name = getlogin();
  sender_tty = ttyname(STDIN_FILENO);
  gethostname(sender_host, 256);
  time(&now);
  timeval = localtime(&now);
  sprintf(buf,
          "Message from %s@%s on %s at %2d:%02d:%02d ... \n",
          sender_name,
          sender_host,
          sender_tty + 5,
          timeval->tm_hour,
          timeval->tm_min,
          timeval->tm_sec);
}

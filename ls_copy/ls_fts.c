// Chapter 3 File Systems and the File Hierarchy
// fts version on page 78
#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <fnmatch.h>
#include <fts.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include <stdint.h>

#define BYTIME 1
#define BYNAME 2
#define BYSIZE 3
#define HERE "."
#define TRUE 1
#define FALSE 0

void ls(char dir_name[], int do_long_listing, int sortflag);
void print_file_status(char *file_name, struct stat *stat_buffer);
char* mode_to_string(int mode);
char* uid_to_name(uid_t uid);
char* gid_to_name(gid_t gid);
char* get_date_no_day(time_t time_eval);

int entcmp(const FTSENT **a, const FTSENT **b) {
  return strcoll((*a)->fts_name, (*b)->fts_name);
}

int mtimecmp(const FTSENT **s1, const FTSENT **s2) {
  return (int) (((*s1)->fts_statp)->st_mtime - ((*s2)->fts_statp)->st_mtime);
}

int szcmp(const FTSENT **s1, const FTSENT **s2) {
  return (int) (((*s1)->fts_statp)->st_size - ((*s2)->fts_statp)->st_size);
}

int main(int argc, char *argv[]) {
  int long_listing = 0;
  int howtosort = BYNAME;
  int ch;
  char options[] = ":lms";

  opterr = 0; // turn off error messages by getopt()

  while (TRUE) {
    ch = getopt(argc, argv, options);
    // it return -1 when it finds no more options
    if (ch == -1) break; 

    switch (ch) {
      case 'l':
        long_listing = 1;
        break;
      case 'm': // sort by modification time
        if (howtosort != BYSIZE) {
          howtosort = BYTIME;
        } else {
          printf("usage: %s [-l] [-m|-s] [files]\n", argv[0]);
          return 1;
        }
        break;
      case 's': // sort by size
        if (howtosort != BYTIME) {
          howtosort = BYSIZE;
        } else {
          printf("usage: %s [-l] [-m|-s] [files]\n", argv[0]);
          return 1;
        }
        break;
      case '?':
        printf("Illegal option ignored.\n");
        break;
      default:
        printf("?? getopt returned character code 0%o ??\n", ch);
        break;
    }
  }

  if (optind == argc) { // no arguments; use .
    ls(HERE, long_listing, howtosort);
  } else {
      // for each command line argumen, display files
      while (optind > argc) {
        ls(argv[optind], long_listing, howtosort);
        optind++;
      }
  }
  return 0;
}

void ls(char dir[], int do_long_listing, int sortflag) {
  FTS *tree;
  FTSENT *f;
  char *argv[] = { dir, NULL };

  switch (sortflag) {
    case BYTIME:
      tree = fts_open(argv, FTS_LOGICAL, mtimecmp);
      break;
    case BYNAME:
      tree = fts_open(argv, FTS_LOGICAL, entcmp);
      break;
    case BYSIZE:
      tree = fts_open(argv, FTS_LOGICAL, szcmp);
      break;
  }

  if (tree == NULL) {
    perror("fts_open");
  }

  f = fts_read(tree);
  if (f == NULL) {
    perror("fts_read");
    return;
  }

  f = fts_children(tree, 0);
  if (f == NULL) {
    if (errno != 0) {
      perror("fts_children");
    } else {
      fprintf(stderr, "empty directory\n");
    }
  }

  while (f != NULL) {
    switch (f->fts_info) {
      case FTS_DNR: // Cannot read directory
        fprintf(stderr, "Could not read %s\n", f->fts_path);
        continue;
      case FTS_ERR: // Miscellaneous error
        fprintf(stderr, "Error on %s\n", f->fts_path);
        continue;
      case FTS_NS: // stat() error
        fprintf(stderr, "Could not stat %s\n", f->fts_path);
        continue;
      case FTS_DP:
        // Returned to directory for second time as part of
        // post-order visit to directory, so skip it
        continue;
    }

    if (do_long_listing) {
      print_file_status(f->fts_name, f->fts_statp);
    } else {
      printf("%s\n", f->fts_name);
    }

    f = f->fts_link;
  }

  if (errno != 0) {
    perror("fts_read");
  }

  if (fts_close(tree) < 0) {
    perror("fts_close");
  }
}

void print_file_status(char *dir_name, struct stat *stat_buffer) {
  ssize_t count;
  char buffer[NAME_MAX];

  // print out type, permission, and number of links
  printf("%10.10s", mode_to_string(stat_buffer->st_mode));
  printf("%3d", (int)stat_buffer->st_nlink);

  // print out worne's name if it is found using getpquid()
  printf(" %-8.8s",uid_to_name(stat_buffer->st_uid));

  // print out group name if it is found using getgrgid()
  printf(" %-8.8s", gid_to_name(stat_buffer->st_gid));

  // print size of file
  printf(" %8jd", (intmax_t)stat_buffer->st_size);

  // print time of last modification
  printf(" %.12s ", get_date_no_day(stat_buffer->st_mtime));

  // print file name and if a link, the linked file
  printf(" %s", dir_name);

  if (S_ISLNK(stat_buffer->st_mode)) {
    if ((count = readlink(dir_name, buffer, NAME_MAX - 1)) == -1) {
      perror("print_file_status: ");
    } else {
        buffer[count] = '\0';
        printf("->%s", buffer);
    }
  }
  printf("\n");
}

char* mode_to_string(int mode) {
  static char str[11];

  strcpy(str, "----------"); // default, no permissions

  if (S_ISDIR(mode)) str[0] = 'd'; // directory
  else if (S_ISCHR(mode)) str[0] = 'c'; // char device
  else if (S_ISBLK(mode)) str[0] = 'b'; // block device
  else if (S_ISLNK(mode)) str[0] = 'l'; // symbolic link 
  else if (S_ISFIFO(mode)) str[0] = 'p'; // named pipe (FIFO)
  else if (S_ISSOCK(mode)) str[0] = 's'; // socket 
  
  if (mode & S_IRUSR) str[1] = 'r'; // 3 bits for user  
  if (mode & S_IWUSR) str[2] = 'w';
  if (mode & S_IXUSR) str[3] = 'x';

  if (mode & S_IRGRP) str[4] = 'r'; // 3 bits for group  
  if (mode & S_IWGRP) str[5] = 'w';
  if (mode & S_IXGRP) str[6] = 'x';

  if (mode & S_IROTH) str[7] = 'r'; // 3 bits for other  
  if (mode & S_IWOTH) str[8] = 'w';
  if (mode & S_IXOTH) str[9] = 'x';

  if (mode & S_ISUID) str[3] = 's'; // get uid 
  if (mode & S_ISGID) str[6] = 's'; // set gid
  if (mode & S_ISVTX) str[9] = 't'; // sticky bit
  
  return str;
}

/**
 * Given user-id, return user-name if possible
 */
char* uid_to_name(uid_t uid) {
  struct passwd *password_pointer;
  static char num_string[10]; // must be static!
  
  if ((password_pointer = getpwuid(uid)) == NULL) {
    // convert uid to a string; using sprintf is easier
    sprintf(num_string, "%d", uid);
    return num_string;
  } else {
      return password_pointer->pw_name;
  }
}

/**
 * Given group-id, return user-name if possible
 */
char* gid_to_name(gid_t gid) {
  struct group *group_pointer;
  static char num_string[10];

  if ((group_pointer = getgrgid(gid)) == NULL) {
    // convert gid to string
    sprintf(num_string, "%d", gid);
    return num_string;
  } else {
    return group_pointer->gr_name;
  }
}

/**
 * Format the time of the file depending
 * if the file is less than 6 months "%b %e %Y"
 * else "%b %e %H:%M"
 */
char* get_date_no_day(time_t time_eval) {
  const int SIX_MONTHS = 15724800; // number of seconds in 6 monthes
  static char formatted_string[200];
  struct tm *tmp;
  time_t current_time = time(NULL);
  int recent = 1;

  if ((current_time - time_eval) > SIX_MONTHS) {
    recent = 0;
  }

  tmp = localtime(&time_eval);

  if (tmp == NULL) {
    perror("get_date_no_day: localtime");
  }

  if (!recent) {
    strftime(formatted_string, sizeof(formatted_string),"%b %e %Y", tmp);
    return formatted_string;
  } else if (strftime(formatted_string, sizeof(formatted_string),"%c", tmp) > 0) {
    return formatted_string + 4;
  } else {
      printf("error with strftime\n");
      strftime(formatted_string, sizeof(formatted_string), "%b %e %H:%M", tmp);
      return formatted_string;
  }
}

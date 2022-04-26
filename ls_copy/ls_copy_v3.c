// Chapter 3 File Systems and the File Hierarchy
// version 3, on page 35
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void ls(char dir_name[], int do_long_listing);
void print_file_status(char *file_name, struct stat stat_buffer);
char* mode_to_string(int mode);
char* uid_to_name(uid_t uid);
char* gid_to_name(gid_t gid);
char* get_date_no_day(time_t time_eval);

int main(int argc, char *argv[]) {
  int long_listing = 0;
  int ch;
  char options[] = ":l";

  opterr = 0; // turn off error messages by getopt()

  while (1) {
    ch = getopt(argc, argv, options);

    // it return -1 when it finds no more options
    if (ch == -1) break; 

    switch (ch) {
      case 'l':
        long_listing = 1;
        break;
      case '?':
        printf("Illegal option ignored.]n");
        break;
      default:
        printf("getopt returned characted code 0%o ??\n", ch);
        break;
    }
  }

  if (optind == argc) { // no arguments; use .
    ls(".", long_listing);
  } else {
      while (optind < argc) {
        ls(argv[optind], long_listing);
        optind++;
      }
  }
  return 0;
}

void ls(char dir_name[], int do_long_listing) {
  DIR *dir; // pointer to directory struct
  struct dirent *dirent_pointer; // pointer to the directory entry
  char file_name[PATH_MAX]; // string to hold path name
  struct stat stat_buffer; // to store stat results

  // test if a regular file, and if so, just display it
  if (lstat(dir_name, &stat_buffer) == -1) {
    perror(file_name);
    return; // stat call failed so we quit
  } else if (!S_ISDIR(stat_buffer.st_mode)) {
      if (do_long_listing) {
        print_file_status(dir_name, stat_buffer);
      } else {
          printf("%s\n", dir_name);
      }
      return;
  }

  if ((dir = opendir(dir_name)) == NULL) {
    fprintf(stderr, "Cannot open %s\n", dir_name);
  } else {
      printf("\n%s:\n", dir_name);
      // loop through directory entries
      while ((dirent_pointer = readdir(dir)) != NULL) {
        if (strcmp(dirent_pointer->d_name, ".") == 0 || strcmp(dirent_pointer->d_name, "..") == 0) {
          // skip dot and dot-dot entries
          continue;
        }

        if (do_long_listing) {
          // contruct a pathname for the file using the
          // directory name passed to the program and the
          // directory entry
          sprintf(file_name, "%s/%s", dir_name, dirent_pointer->d_name);

          // fill the stat buffer
          if (lstat(file_name, &stat_buffer) == -1) {
            perror(file_name);
            continue; // stat call failed but we go on
          }

          print_file_status(dirent_pointer->d_name, stat_buffer);
        } else {
            printf("%s\n",dirent_pointer->d_name);
        }
      }
  }
}

void print_file_status(char *dir_name, struct stat stat_buffer) {
  ssize_t count;
  char buffer[NAME_MAX];

  // print out type, permission, and number of links
  printf("%10.10s", mode_to_string(stat_buffer.st_mode));
  printf("%3d", (int)stat_buffer.st_nlink);

  // print out worne's name if it is found using getpquid()
  printf(" %-8.8s",uid_to_name(stat_buffer.st_uid));

  // print out group name if it is found using getgrgid()
  printf(" %-8.8s", gid_to_name(stat_buffer.st_gid));

  // print size of file
  printf(" %8jd", (intmax_t)stat_buffer.st_size);

  // print time of last modification
  printf(" %.12s ", get_date_no_day(stat_buffer.st_mtime));

  // print file name and if a link, the linked file
  printf(" %s",dir_name);

  if (S_ISLNK(stat_buffer.st_mode)) {
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

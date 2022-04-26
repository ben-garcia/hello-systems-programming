// Chapter 3 File Systems and the File Hierarchy
// version 2, on page 29 which introduces stat()
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define HERE "."

void print_file_status(char *, struct stat *);
void mode_to_string(int, char []);
char* get_date_no_day(time_t time_eval);
char* uid_to_name(uid_t);
char* gid_to_name(gid_t);
void ls(char[], int);

int main(int argc, char *argv[]) {
  int i = 1;

  if (argc == 1) { // no arguments; use current directory (.)
    ls(HERE, 1);
  } else {
      while (argc > i) {
        printf("%s:\n", argv[i]);
        ls(argv[i], 1);
        i++;
      }
  }
}

/**
 * Does not check whether entries are files or directories
 * or . files
 */
void ls(char dir_name[], int do_long_listing) {
  DIR *dir_pointer; // directory stream
  struct dirent *dir_ent_pointer; // holds one entry
  struct stat stat_buffer; // stores stat results
  
  if ((dir_pointer = opendir(dir_name)) == NULL) {
    // could not open -- maybe it was not a directory
    fprintf(stderr, "ls_copy_v2: cannot open %s\n", dir_name);
  } else {
      while ((dir_ent_pointer = readdir(dir_pointer)) != NULL) {
        if (do_long_listing) {
          if (lstat(dir_ent_pointer->d_name, &stat_buffer) == -1) {
            perror(dir_ent_pointer->d_name);
            continue; // stat call failed but we go on
          }
          print_file_status(dir_ent_pointer->d_name, &stat_buffer);
        } else { // not long -- just print name
            printf("%s\n",dir_ent_pointer->d_name);
        }
      }
      closedir(dir_pointer);
  }
}

void print_file_status(char *file_name, struct stat *info_pointer) {
  char mode_string[11];
  ssize_t count;
  char buffer[NAME_MAX];

  mode_to_string(info_pointer->st_mode, mode_string);

  printf("%s", mode_string);
  printf("%4d ", (int)info_pointer->st_nlink);
  printf("%-8s ", uid_to_name(info_pointer->st_uid));
  printf("%-8s ", gid_to_name(info_pointer->st_gid));
  printf("%8ld ", (long)info_pointer->st_size);
  printf("%.12s ", get_date_no_day(info_pointer->st_mtime));
  printf("%s", file_name);

  if (S_ISLNK(info_pointer->st_mode)) {
    if ((count = readlink(file_name, buffer, NAME_MAX - 1))) {
      perror("print_file_status: ");
    } else {
        buffer[count] = '\0';
        printf("->%s", buffer);
    }
  }
  printf("\n");
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
 * Given a mode format the string
 */
void mode_to_string(int mode, char str[]) {
  strcpy(str, "----------");

  if (S_ISDIR(mode)) str[0] = 'd'; // directory
  else if (S_ISCHR(mode)) str[0] = 'c'; // char device
  else if (S_ISBLK(mode)) str[0] = 'b'; // block device 
  else if (S_ISLNK(mode)) str[0] = 'l'; // symbolic link 
  else if (S_ISFIFO(mode)) str[0] = 'p'; // named pipe (FIFO) 
  else if (S_ISSOCK(mode)) str[0] = 's'; // socket 
  
  if (mode & S_IRUSR) str[1] = 'r'; // 3 bites for user 
  if (mode & S_IWUSR) str[2] = 'w';
  if (mode & S_IXUSR) str[3] = 'x';

  if (mode & S_IRGRP) str[4] = 'r'; // 3 bites for group
  if (mode & S_IWGRP) str[5] = 'w';
  if (mode & S_IXGRP) str[6] = 'x';

  if (mode & S_IROTH) str[7] = 'r'; // 3 bites for other
  if (mode & S_IWOTH) str[8] = 'w';
  if (mode & S_IXOTH) str[9] = 'x';
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

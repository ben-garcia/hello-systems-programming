// Chapter 3 File Systems and the File Hierarchy
// version 1, on page 50 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// BUFSIZ is defined in stdio.h: it is maximum string size
#define MAX_PATH BUFSIZ

// prints the pwd to the string abs_pathname and
// NULL terminates it
void print_pwd(char *abs_pathname);
// prints to str the path from / to the file
// specified by cur_inum
void print_path(char *str, ino_t cur_inum);
// puts the filename of the file with i-node inum
// into string buf, at most len chars long and NULL terminates it.
void inum_to_name(ino_t inum, char *buf, int len);
// get the i-node number of file fname, if that fname
// is the name of a file in the current working directory
// Return 0 if successful, -1 if not.
int get_ino(char *fname, ino_t *inum);

int main(int argc, char *argv[]) {
  char path[MAX_PATH] = "\0"; // string to store pwd

  print_pwd(path); // print pwd to string path
  printf("%s\n", path); // print path to stdout
  return 0;
}

void print_pwd(char *pathname) {
  ino_t inum;

  get_ino(".", &inum);
  print_path(pathname, inum);
}

void print_path(char *abs_pathname, ino_t this_inode) {
  ino_t parent_inode;
  char its_name[BUFSIZ];

  // get inumber of parent
  get_ino("..", &parent_inode);

  // At root if parent inum == current inum
  if (parent_inode != this_inode) {
    chdir(".."); // cd up to parent
    // get filename of current file
    inum_to_name(this_inode, its_name, BUFSIZ);
    // recursively get path to parent directory
    print_path(abs_pathname, parent_inode);
    strcat(abs_pathname, "/");
    strcat(abs_pathname, its_name);
  } else {
    strcat(abs_pathname, "/");
  }
}

void inum_to_name(ino_t inode_to_find, char *namebuf, int buflen) {
  DIR *dir_ptr;
  struct dirent *direntp;

  dir_ptr = opendir(".");
  if (dir_ptr == NULL) {
    perror(".");
    exit(1);
  }

  while ((direntp = readdir(dir_ptr)) != NULL) {
    if (direntp->d_ino == inode_to_find) {
      strncpy(namebuf, direntp->d_name, buflen);
      namebuf[buflen - 1] = '\0';
      closedir(dir_ptr);
      return;
    }
  }
  fprintf(stderr, "\nError looking for i-node number %d\n", (int)inode_to_find);
  exit(1);
}

int get_ino(char *fname, ino_t *inum) {
  struct stat info;

  if (stat(fname, &info) == -1) {
    fprintf(stderr, "Cannot stat ");
    perror(fname);
    return -1;
  }
  *inum = info.st_ino;
  return 0;
}

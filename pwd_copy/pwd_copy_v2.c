// Chapter 3 File Systems and the File Hierarchy
// version 2, on page 56 
// fixes the issues
// 1. correctly formats the pwd string('/...' instead of '//...')
// 2. takes into account mounted devices(by checking for device id)
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
// prints to str the path from / to the filespecified by curr_inum
// on the device with device id dev_num.
void print_path(char *str, ino_t cur_inum, dev_t dev_num);
// puts the filename of the file with inode inum
// on the device with device number dev_num into string buf, at
// most len chars long and 0 terminates it.
void inum_to_name(ino_t inum, dev_t dev_num, char *buf, int len);
// get the device id and the inode number of
// file fname, if that fname is the name of a file in the
// current working directory. Returns 0 if success, -1 if not.
int get_deviceid_inode(const char *fname, dev_t *dev_id, ino_t *inum); 

int main(int argc, char *argv[]) {
  char path[MAX_PATH] = "\0"; // string to store pwd

  print_pwd(path); // print pwd to string path
  printf("%s\n", path); // print path to stdout
  return 0;
}

void print_pwd(char *pathname) {
  ino_t inum;
  dev_t devnum;

  get_deviceid_inode(".", &devnum, &inum);
  print_path(pathname, inum, devnum);
}

void print_path(char *abs_pathname, ino_t this_inode, dev_t this_dev) {
  // Recursively prints path leading down to file with this
  // inode on this_dev.
  // Uses static int height to determine
  // which recusrive level it is in
  ino_t parent_inode;
  char its_name[BUFSIZ];
  dev_t dev_of_node;
  dev_t dev_of_parent;
  static int height = 0;

  // get device id and inumber of parent
  get_deviceid_inode("..", &dev_of_parent, &parent_inode);

  // At root if parent inum == cur inum & device ids are the same
  if ((parent_inode != this_inode) || (dev_of_parent != this_dev)) {
    chdir(".."); // cd up to parent
    // get filename of current file
    inum_to_name(this_inode, this_dev, its_name, BUFSIZ);
    height++; // about to make recursive call
    // recursively get path to parent directory
    print_path(abs_pathname, parent_inode, dev_of_parent);
    strcat(abs_pathname, its_name);

    if (height > 1) {
      // Since height is decremented whenever we
      // leave call it can only be > 1 if we have not
      // yet popped all calls from the stack
      strcat(abs_pathname, "/");
    }
    height--;
  } else { // must be at root
    strcat(abs_pathname, "/");
  }
}

void inum_to_name(ino_t inode_to_find, dev_t devnum, char *name, int buflen) {
  DIR *dir_ptr;
  struct dirent *direntp;
  struct stat statbuf;

  if ((dir_ptr = opendir(".")) == NULL) {
    perror(".");
    exit(1);
  }
  
  while ((direntp = readdir(dir_ptr)) != NULL) {
    if ((stat(direntp->d_name, &statbuf)) == -1) {
      fprintf(stderr, "could not stat");
      perror(direntp->d_name);
      exit(1);
    }
    if ((statbuf.st_ino == inode_to_find) && (statbuf.st_dev == devnum)) {
      strncpy(name, direntp->d_name, buflen);
      name[buflen - 1] = '\0'; // just in case
      closedir(dir_ptr);
      return;
    }
  }
  fprintf(stderr, "\nError looking for i-node %d\n", (int)inode_to_find);
  exit(1);
}

int get_deviceid_inode(const char *fname, dev_t *dev_id, ino_t *inum) {
  struct stat info;

  if (stat(fname, &info) == -1) {
    fprintf(stderr, "Cannot stat "); 
    perror(fname);
    exit(1);
  }
  *inum = info.st_ino;
  *dev_id = info.st_dev;
  return 0;
} 

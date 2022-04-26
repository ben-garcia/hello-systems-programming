// Chapter 3 File Systems and the File Hierarchy
// version 1, on page 18 using readdir()
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#define HERE "."

void ls(char []);

int main(int argc, char *argv[]) {
  int i = 1;

  if (argc == 1) { // no arguments use "."
    ls(HERE);
  } else {
      // for each command line argument, display contents
      while (argc > i) {
        printf("%s:\n", argv[i]);
        ls(argv[i]);
        i++;
      }
  }

  return 0;
}

/**
  * List the contents of the directory name dir_name
  * Uses opendir to check whether argument is a directory
  * Doesn't check is arguemnt is or ".' or ".."
  */
void ls(char dir_name[]) {
  DIR *dir_ptr; // directory stream
  struct dirent *dirent_ptr; // hold one entry

  if ((dir_ptr = opendir(dir_name)) == NULL) {
    // Could not open -- maybe it was not a directory
    fprintf(stderr, "Cannot open %s\n", dir_name);
  } else {
      while ((dirent_ptr = readdir(dir_ptr)) != NULL) {
        printf("%s\n", dirent_ptr->d_name);
      }
      closedir(dir_ptr);
  }
}

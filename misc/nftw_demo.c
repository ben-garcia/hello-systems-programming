// Chapter 3 File Systems and the File Hierarchy
// traversing the file system(tree) with nftw() on page 63
#define _XOPEN_SOURCE 500

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0
#define MAX_DEPTH 20

// For getopt() we need to use this feature test macro
#if (_POSIX_C_SOURCE >= 2 || _XOPEN_SOURCE)

static int display_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
  char blanks[PATH_MAX];
  const char *filename = fpath + ftwbuf->base;
  int width = 4 * ftwbuf->level;

  // fill blanks with a string of 4 * level blanks
  memset(blanks, ' ', width);
  blanks[width] = '\0';

  // print out blanks then filename (not full path)
  printf("%s%s", blanks, filename);

  // Check flags and print a message if need be
  if (tflag == FTW_DNR) {
    printf(" (unreadable directory)");
  } else if (tflag == FTW_SL) {
    printf(" (symbolic link)");
  } else if (tflag == FTW_SLN) {
    printf(" (broken symbolic link)");
  } else if (tflag == FTW_NS) {
    printf("stat failed ");
  }
  printf("\n");
  return 0;
}

int main(int argc, char *argv[]) {
  int flags = 0;
  int status;
  int ch;
  char options[] = ":cdpm";
  opterr = 0; // turn off error messages by getopt()

  while (TRUE) {
    // call getopt, passing argc and argv and the options string
    ch = getopt(argc, argv, options);

    // it returns -1 when it finds no more options
    if (ch == -1) {
      break;
    }

    switch(ch) {
      case 'c':
        flags |= FTW_CHDIR; // change to current directory for processing
        break;
      case 'd': // post-order instead of pre-order traversal
        flags |= FTW_DEPTH;
        break;
      case 'p': // follow symbolic links
        flags |= FTW_PHYS;
        break;
      case 'm': // cross mounts points
        flags |= FTW_MOUNT;
        break;
      default:
        fprintf(stderr, "Bad option found.\n");
        return 1;
    }
  }

  if (optind < argc) {
    while (optind < argc) {
      status = nftw(argv[optind], display_info, MAX_DEPTH, flags); 

      if (status == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
      } else {
        optind++;
      }
    }
  } else {
    status = nftw("-", display_info, MAX_DEPTH, flags);

    if (status == -1) {
      perror("nftw");
      exit(EXIT_FAILURE);
    }
  }
  exit(EXIT_SUCCESS);
}

#endif

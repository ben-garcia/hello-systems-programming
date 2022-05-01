// Chapter 3 File Systems and the File Hierarchy
// traversing the file system(tree) with fts family
// of functions on page 74

#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <fnmatch.h>
#include <fts.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/******************************************************************************
 *************************** Utility Function Prototypes **********************
******************************************************************************/
/**
 * Print usage messgae to stdout
 */
void usage(char *progname);

/**
 * Compare files by name
 */
int entcmp(const FTSENT **s1, const FTSENT **s2);

/**
* Print all files in teh directory tree that match the glob pattern
* Example: pmatch("/usr/src", "*.c");
*/
void pmatch(char *dir, const char *pattern);

/******************************************************************************
 ******************************** Main Program ********************************
******************************************************************************/
int main(int argc, char *argv[]) {
  if (argc > 3) {
    usage(argv[0]);
    exit(1);
  }
  pmatch(argv[1], argv[2]);
  return 0;
}

/*****************************************************************************/
int entcmp(const FTSENT **s1, const FTSENT **s2) {
  return (strcoll((*s1)->fts_name, (*s2)->fts_name));
}

/*****************************************************************************/
void usage(char *progname) {
  printf("usage: %s directory pattern\n", progname);
}

/*****************************************************************************/
void pmatch(char *dir, const char *pattern) {
  FTS *tree; // pointer to file tree stream returned by fts_open
  FTSENT *f; // pointer to structure returned by fts_read
  char *argv[] = { dir, NULL };

  // Call fts_open(0 with FTS_LOGICAL, to follow symbolic links
  // including links to other directories. Since it detects cycles,
  // we do not have to worry about infinite loops.
  tree = fts_open(argv, FTS_LOGICAL, entcmp);
  if (tree == NULL) {
    perror("fts_open");
  }

  // Rpeatedly get next file skipping "." and ".." because
  // FTS_SEEDOT was not set.
  while ((f = fts_read(tree))) {
    switch (f->fts_info) {
      case FTS_DNR: // Cannot read directory
        fprintf(stderr, "Could not read %s\n", f->fts_path);
        continue; 
      case FTS_ERR: // MIscellaneous error 
        fprintf(stderr, "Error on %s\n", f->fts_path);
        continue; 
      case FTS_NS: // stat() error
        // show error, then continue to next file
        fprintf(stderr, "Could not stat %s\n", f->fts_path);
        continue; 
      case FTS_DP:
        // Returned to directory for second time as part of
        // post-order visit to directory, so skip
        continue; 
    }

    // Check if the name matches patter, and if so, print out its
    // path. This check uses FNM_PERIOD, so "*.c" will not
    // match ".invisible.c".
    if (fnmatch(pattern, f->fts_name, FNM_PERIOD) == 0) {
      printf("%s\n", f->fts_path);
    }
    if (f->fts_info == FTS_DC) {
      fprintf(stderr, "%s: cycle in directory tree", f->fts_path);
    }

    // fts_read() sets errno = 0 unless it has an error.
    if (errno != 0) {
      perror("fts_read");
    }
    if (fts_close(tree) < 0) {
      perror("fts_close");
    }
  }
}

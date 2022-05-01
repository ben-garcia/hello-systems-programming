// Chapter 3 File Systems and the File Hierarchy
// traversing the file system(tree) with nftw() on page 63
//
// NOTES:
// Tree Traversals
// https://www.geeksforgeeks.org/tree-traversals-inorder-preorder-and-postorder
// Depth First Traversals (Inorder, Preorder and Postorder)
//
// given the following tree
//                 1
//                / \
//               /   \
//              2     3
//             / \
//            /   \
//           4     5
//
// Depth First Traversals: 
// (a) Inorder (Left, Root, Right) : 4 2 5 1 3 
// (b) Preorder (Root, Left, Right) : 1 2 4 5 3 
// (c) Postorder (Left, Right, Root) : 4 5 2 3 1
// Breadth-First or Level Order Traversal: 1 2 3 4 5 

#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0
#define MAXDEPTH 200

// for getopt() se need to used this feature test macro
#if (_POSIX_C_SOURCE >= 2 || _XOPEN_SOURCE)

static uintmax_t total_size[MAXDEPTH];

int file_usage(const char *fpath,
               const struct stat *sb,
               int tflag,
               struct FTW *ftwbuf) {
  static int prev_level = -1;
  int cur_level;
  uintmax_t cur_size;

  cur_level = ftwbuf->level; 

  if (cur_level >= MAXDEPTH) {
    fprintf(stderr, "Exceeded maximum depth.\n");
    return -1;
  }

  if (prev_level == cur_level) {
    cur_size = sb->st_size;
  } else if (prev_level > cur_level) {
    cur_size = total_size[prev_level] + sb->st_size;
    total_size[cur_level] += cur_size;
    total_size[prev_level] = 0;
  } else {
    cur_size = sb->st_size;
    total_size[cur_level] = cur_size;
  }

  printf("%ju\t%s", cur_size, fpath);
  prev_level = cur_level;

  if (tflag == FTW_DNR) {
    printf(" (unreadable directory )");
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
  int flags = FTW_DEPTH | FTW_PHYS | FTW_MOUNT;
  int status;
  int i = 1;

  if (argc < 2) {
    memset(total_size, 0, MAXDEPTH * sizeof(uintmax_t));
    status = nftw(".", file_usage, 20, flags);

    if (status == -1) {
      fprintf(stderr, "nftw exited abnormally.\n");
      exit(EXIT_FAILURE);
    }
  } else {
      while (argc > i) {
        memset(total_size, 0, MAXDEPTH * sizeof(uintmax_t));
        status = nftw(argv[i], file_usage, MAXDEPTH, flags);

        if (status == -1) {
          fprintf(stderr, "nftw exited abnormally.\n");
          exit(EXIT_FAILURE);
        } else {
            i++;
        }
      }
  }
  exit(EXIT_SUCCESS);
}

#endif

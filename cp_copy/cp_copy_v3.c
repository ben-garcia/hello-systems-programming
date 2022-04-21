// Chapter 2 Login Records, File I/O, and Performance
// version 2, on page 45, using memory-mapped I/O
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define COPY_MODE 0666 

void die(char *string_1, char *string_2); // print error and quit

int main(int argc, char *argv[]) {
  int in_fd;
  int out_fd;
  size_t file_size;
  char null_byte;
  void *source_address;
  void *destination_address;
  
  // check arguments 
  if (argc != 3) {
    fprintf(stderr, "usage: %s source destination\n", *argv);
    exit(1);
  }

  // open files
  if ((in_fd = open(argv[1], O_RDONLY)) == -1) {
    die("Cannot open ", argv[1]);
  }

  // The file to be created must be opened in read/write mode
  // because of how mmap()'s POT_WRITE works on i386 architectures.
  // According to the man page, on some hardware architectures (e.g. i386),
  // PROT_WRITE impolies PROT_READ. Therefore setting the proection flag to
  // PROT_WRITE is equivalent to setting it to PROT_WRITE | PROT_READ if the
  // the machine architecture is i386 or the like. Since this flag has to match
  // the flags by which the mapped file was opened. I set the opening flags
  // differently for the i386 architecture than for others.
  
  #if defined (i386) || defined (__x86_64) || defined (__x86_64__) \
      || defined (i686)
    if ((out_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, COPY_MODE)) == -1) {
      die("Cannot create ", argv[2]);
    }
  #else
    if ((out_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, COPY_MODE)) == -1) {
      die("Cannot create", argv[2]);
    }
  #endif

  // get the size of the source file by seeking to the end of it.
  // lseek() returns the offset location of the file pointer after
  // the seek relative to the beginning of the file, so this is a
  // good way to get an opened file's size.
  if ((file_size = lseek(in_fd, 0, SEEK_END)) == -1) {
    die("Could not seek to end of file", argv[1]);
  }

  // By seeking to file_size in the new file, the file can be grown to that
  // size. Its size does not change until w write occurs there.
  lseek(out_fd, file_size - 1, SEEK_SET);

  // So we write the NULL byte and file size is now set to file_size.
  write(out_fd, &null_byte, 1);

  // Time to set up the memory maps.
  if ((source_address = mmap(NULL, file_size, PROT_READ,
         MAP_SHARED, in_fd, 0)) == (void*) -1) {
    die("Error mapping file ", argv[1]);
  }

  if ((destination_address = mmap(NULL, file_size, PROT_WRITE,
          MAP_SHARED, out_fd, 0)) == (void*) -1) {
    die("Error mapping file ", argv[2]);
  }    

  // copy the input to output by doing a memcpy
  memcpy(destination_address, source_address, file_size);

  // ummap the files
  munmap(source_address, file_size);
  munmap(destination_address, file_size);

  // close the files
  close(in_fd);
  close(out_fd);

  return 0;
}

void die(char *string_1, char *string_2) {
  fprintf(stderr, "Error: %s ", string_1);
  perror(string_2);
  exit(1);
}

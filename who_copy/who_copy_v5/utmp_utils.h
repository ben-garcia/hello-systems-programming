#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <utmp.h>

typedef struct utmp utmp_record;

// macros
#define NUM_RECORDS 20
#define NULL_UTMP_RECORD_PTR ((utmp_record*) NULL)
#define SIZE_OF_UTMP_RECORD (sizeof(utmp_record))
#define BUFF_SIZE (NUM_RECORDS * SIZE_OF_UTMP_RECORD)

// global variables
static char utmp_buf[BUFF_SIZE]; // buffer of records
static int number_of_recs_in_buffer; // number records in buffer
static int current_record; // next record to read
static int fd_utmp = -1; // file descriptor for utmp file

static inline void die(char *string_1, char *string_2) {
  fprintf(stderr, "Error: %s ", string_1);
  perror(string_2);
  exit(1);
}

/**
  * Opens the given umtp_file for buffered reading
  * @return: a valid file descriptor on success
  *          -1 on error
  */
static inline int open_utmp(char *utmp_file) {
  fd_utmp = open(utmp_file, O_RDONLY);
  current_record = 0;
  number_of_recs_in_buffer = 0;

  return fd_utmp; // either a valid file descriptor or -1
}

/**
  *
  */
static inline int utmp_fill() {
  int bytes_read;

  // read NUM_RECORDS records form the utmp file into buffer
  // bytes read is the actual number of bytes read
  bytes_read = read(fd_utmp, utmp_buf, BUFF_SIZE);

  if (bytes_read < 0) {
    die((char*)"Failed to read form umtp file", (char*)"");
  }

  // If we read here, the read was successful
  // Convert the bytes count in to a number of records
  number_of_recs_in_buffer = bytes_read / SIZE_OF_UTMP_RECORD;

  // reset current record to start at the buffer
  current_record = 0;

  return number_of_recs_in_buffer;
}

/**
  * @return: a pointer to the next utmp record form the 
  *          opened file and advances to the next record
  *          NULL if no more records are in the file
  */
static inline utmp_record *next_utmp() {
  utmp_record *record_ptr;
  int byte_position;

  if (fd_utmp == -1) {
    // file was not opened correctly
    return NULL_UTMP_RECORD_PTR;
  }

  if (current_record == number_of_recs_in_buffer) {
    // there are no unread records in the buffer
    // need to refill the buffer
    if (utmp_fill() == 0) {
      // no umtp records left in the file
      return NULL_UTMP_RECORD_PTR;
    }
  }

  // There is at least one record in the buffer,
  // so we can read it
  byte_position = current_record * SIZE_OF_UTMP_RECORD;
  record_ptr = (utmp_record*) &utmp_buf[byte_position];

  // advance current_record pointer and return record pointer
  current_record++;
  
  return record_ptr;
}

/**
  * Closes the utmp file and fress the file descriptor
  */
static inline void close_utmp() {
  // if file descriptor is a valid one, close the connection
  if (fd_utmp != -1) {
    close(fd_utmp);
  }
}

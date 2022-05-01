# 'du' copy 

> understand how to interface with various system functions,
and writing `ls` correctly was a way to get experience with a
few importnat structures: the `dirent` object and the i-node.

## system calls used
* `opendir()`
  - open a connection to a directory 
* `readdir()`
  - read the directory
  - successive calls iterate through the directory
* `closedir()`
  - closes the connection to a directory 
* `telldir()`
  - return the offset of the directory stream from the start
* `rewinddir()`
  - reset the directory stream to the beginning
* `seekdir()`
  - move the pointer to a specific index
* `getpwuid()`
  - the broken-out fields of the record in the password database
    that matches the user ID
* `getgrgid()`
  - the broken-out fields of the record in the group database
    that matches the group ID

## macros
* `IS_ISDIR`
  - checks if file is directory
* `IS_ISCHR`
  - checks if file is a char device
* `S_ISBLK`
  - checks if file is a block device
* `S_ISLNK`
  - checks if file is a symbolic link 
* `S_ISFIFO`
  - checks if file is a named pipe 
* `S_ISSOCK`
  - checks if file is a socket 
* `S_IRUSR`
  - checks for the read bite of the user 
* `S_IWUSR`
  - checks for the write bite of the user 
* `S_IXUSR`
  - checks for the executable bite of the user 
* `S_IRGRP`
  - checks for the read bite of the group 
* `S_IWGRP`
  - checks for the write bite of the group 
* `S_IXGRP`
  - checks for the executable bite of the group 
* `S_IROTH`
  - checks for the read bite of the other 
* `S_IWOTH`
  - checks for the write bite of the other 
* `S_IXOTH`
  - checks for the executable bite of the other 

#include "Assignment1.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define PERMS 0666 // RW for owner, group, others

// fopen: open file, return file ptr
FILE *fopen(char *name, char *mode)
{
  int fd; // file descriptor
  FILE *fp;

  // Check if the correct mode is given or not
  if(*mode != 'r' && *mode != 'w' && *mode != 'a')
    return NULL;

  // Find a free slot in the file pointer array
  // What does a free slot mean ?
  for (fp = _iob; fp < _iob + OPEN_MAX; fp++)
    if((fp->flag & (_READ | _WRITE)) == 0)
      break; // found free slot
  if(fp >= _iob + OPEN_MAX) // no free slots
    return NULL;

  // Create the file (if needed) and open it using the requested mode
  if(*mode == 'w')
    fd = creat(name, PERMS); // creat is a system call in C that creates the file or truncates an existing one
  else if(*mode == 'a')
  {
    if((fd= open(name, O_WRONLY, 0)) == -1) // O_WRONLY open a file so that it is write only
      fd = creat(name,PERMS); // did not find file so create
    lseek (fd, 0L,2); // seek to the end of the file for writing
    // lseek is a system call that allows a user to write at an offset from
    // an origin. Origin is specified by third parameter and can be 0,1,2
    // corresponding to beginning, current position or end of the file.
    // 0L is the offset in this case.
  }
  else
    fd = open(name, O_RDONLY, 0); // open a file to read only

  // If we still can't find a name, exit
  if(fd == -1)
    return NULL;
  fp->fd = fd;
  fp->cnt = 0; // Initially, no characters are buffered and...
  fp->base = NULL; // a buffer to hold such has not been initialized
  fp->ptr = NULL; // so we certainly cannot be pointing at any contents
  fp->flag = (*mode == 'r') ? _READ : _WRITE;
  return fp;
}

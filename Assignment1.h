#include <fcntl.h> // Refers to the header that contains constructs e.g
// opening a file, retrieving and changing the permissions of file.
#include <stdlib.h> // Contains header for Memory Allocation(malloc) functions
#include <unistd.h> // Equivalent to "syscalls.h"

#define NULL 0
#define EOF (-1)
#define BUFSIZ 1024
#define OPEN_MAX 20 // max files open at once

typedef struct _iobuf // this statement defines _iobuf first and then gives it
// an alias of FILE. Now variables of type _iobuf can be simply defined by FILE
{
  int cnt; // characters left
  char *ptr; // next character position
  char *base; // location of buffer
  int flag; // mode of file access
  int fd; // file descriptor
} FILE;
extern FILE _iob[OPEN_MAX];
// _iob is a array of FILE with length OPEN_MAX and it can be accessed in any
// .c file which import it.
// extern is a keyword that extends the scope of the variable to global.
// It is only a declaration not a definition i.e it does not allocate memory

#define stdin (&_iob[STDIN_FILENO])
#define stdout (&_iob[STDOUT_FILENO])
#define stderr (&_iob[STDERR_FILENO])

enum _flags // enum is used to define new enumeration data types
{
  _READ = 01, // file open for reading
  _WRITE = 02, // file open for writing
  _UNBUF = 04, // file is unbuffered
  _EOF = 010, // EOF has occurred on this file
  _ERR = 020, // error occurred on this file
};

int _fillbuf(FILE *);
int _flushbuf(int, FILE *);

// The following are macro definitions
#define feof(p) (((p)->flag & _EOF) != 0)
#define ferror(p) (((p)->flag & _ERR) != 0)
#define fileno(p) ((p)->fd)
#define getc(p) (--(p)->cnt >= 0 ? (unsigned char) *(p)->ptr++ : _fillbuf(p))
#define putc(x,p) (--(p)->cnt >= 0 ? *(p)->ptr++ = (x) : _flushbuf((x),p))
#define getchar() getc(stdin)
#define putchar(x) putc((x),stdout)

FILE *fopen(char *name, char *mode);

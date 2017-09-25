#include <fcntl.h> // Refers to the header that contains constructs e.g
// opening a file, retrieving and changing the permissions of file.
#include <stdlib.h> // Contains header for Memory Allocation(malloc) functions
#include <unistd.h> // Equivalent to "syscalls.h"

#define EOF (-1)
#define BUFSIZ 1024
#define OPEN_MAX 20 // max files open at once

typedef struct _iobuf // this statement defines _iobuf first and then gives it
// an alias of MyFILE. Now variables of type _iobuf can be simply defined by MyFILE
{
  int cnt; // characters left in the buffer
  char *ptr; // next character position
  char *base; // location of buffer
  int flag; // mode of file access
  int fd; // file descriptor
} MyFILE;
MyFILE _iob[OPEN_MAX];
// _iob is a array of MyFILE with length OPEN_MAX and it can be accessed in any
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

int _fillbuf(MyFILE *);
int _flushbuf(int, MyFILE *);

// The following are macro definitions
#define feof(p) (((p)->flag & _EOF) != 0)
#define ferror(p) (((p)->flag & _ERR) != 0)
#define fileno(p) ((p)->fd)
#define Mygetc(p) (--(p)->cnt >= 0 ? (unsigned char) *(p)->ptr++ : _fillbuf(p))
#define Myputc(x,p) (--(p)->cnt >= 0 ? *(p)->ptr++ = (x) : _flushbuf((x),p))
#define Mygetchar() Mygetc(stdin)
#define Myputchar(x) Myputc((x),stdout)

MyFILE *myfopen(char *name, char *mode);
int Myfseek(MyFILE *fp, long offset, int origin);
int fflush(MyFILE *fp);
int Myfclose(MyFILE *fp);

size_t Myfread(void *ptr, size_t size, size_t nobj, MyFILE *fp);
size_t Myfwrite(const void *ptr, size_t size, size_t nobj, MyFILE *fp);

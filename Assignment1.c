#include "Assignment1.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
// #include <stdio.h>

#define PERMS 0666 // RW for owner, group, others

// myfopen: open file, return file ptr
MyFILE *myfopen(char *name, char *mode)
{
  int fd; // file descriptor
  MyFILE *fp;

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

// _fillbuf(): Allocate and fill an input buffer
int _fillbuf(MyFILE *fp)
{
  int bufsize = 0; // Size of buffer allocation

  // Make sure we are in read mode and neither EOF not ERR indicators have been set.
  if((fp->flag & (_READ | _EOF | _ERR )) != _READ)
    return EOF;

  // Calculate buffer size and attempt to allocate it.
  bufsize = (fp->flag & _UNBUF) ? 1 : BUFSIZ;
  // This is 1 because we dont want to use a buffer if the _UNBUF flag is on
  if(fp->base == NULL)
    if((fp->base = (char *)malloc(bufsize)) == NULL)
      return EOF;

  // Fill the buffer as much as we can
  fp->ptr = fp->base;
  fp->cnt = read(fp->fd, fp->ptr, bufsize);
  // read is a system call. The second argument is the character array in your
  // program where the data to is to go to. Each call returns a count of the number
  // of bytes transferred. A value of zero implies EOF and -1 implies an error.

  // Handle if either an end of file or error condition occurred
  if(--fp->cnt < 0)
  {
    if(fp->cnt == -1)
      fp->flag |= _EOF;
    else
      fp->flag |= _ERR;
    fp->cnt = 0;
    return EOF;
  }

  return (unsigned char) *fp->ptr++;
}

int _flushbuf(int x, MyFILE *fp)
{
  unsigned nc; // Number of characters to flush
  int bufsize; // Size of buffer Allocation

  // Confirm we were passed a valid file pointer, that the ERR indicator is not set,
  // and that we are in write mode.  If not the case, politely return EOF condition
  if (fp < _iob || fp >= _iob + OPEN_MAX)
    return EOF;
  if ((fp->flag & (_WRITE | _ERR)) != _WRITE)
    return EOF;

  // Set buffer size
  bufsize = (fp->flag & _UNBUF) ? 1 : BUFSIZ;

  // Check to see if a buffer has been allocated and act accordingly
    if (fp->base == NULL) {
    // No buffer has been allocated yet, so attempt to allocate one. If we
    // cannot, set ERR indicator and return EOF condition; otherwise
      if ((fp->base = (char *)malloc(bufsize)) == NULL) {
        fp->flag |= _ERR;
        return EOF;
      }
    } else {
      // A buffer has been allocated so go aehad and write it out and
      // deal with any consequences
      nc = fp->ptr - fp->base;
      if (write(fp->fd, fp->base, nc) != nc) {
        fp->flag |= _ERR;
        return EOF;
      }
    }
    // Maintain the file pointer and save the character
    fp->ptr = fp->base;
    fp->cnt = bufsize - 1;
    *fp->ptr++ = (char) x;

    return x;
}

int fflush(MyFILE *fp) {
  int rc = 0;
  // Make sure we are being asked to flush a valid file pointer
  if (fp < _iob || fp >= _iob + OPEN_MAX)
    return EOF;
  if (fp->flag & _WRITE)
    rc = _flushbuf(0, fp);
  fp->ptr = fp->base;
  fp->cnt = (fp->flag & _UNBUF) ? 1 : BUFSIZ;
  return rc;
}

int Myfclose(MyFILE *fp) {
  int rc;
  if ((rc = fflush(fp)) != EOF) {
    free(fp->base); // free function frees up the memory space pointed to by a ptr
    fp->ptr = NULL;
    fp->cnt = 0;
    fp->base = NULL;
    fp->flag &= ~(_READ | _WRITE);
  }
  return rc;
}

int Myfseek(MyFILE *fp, long offset, int origin) {
  unsigned nc;  // Number of characters to flush
  long rc = 0;  // Return code
  // Take action based upon the current access mode (read or write)
  // of the file
  if (fp->flag & _READ) {
  // We're in a read access mode
  // If origin is set to '1' we are reading from the current position
  // but need to take the characters in the buffer into account, seek to
  // our adjusted position, and discard any buffered characters.  Note: if
  // origin is set to '0' (beginning of file) or '2' (end of file) then
  // the current buffer contents do not need to be accounted for.
    if (origin == 1)
      offset -= fp->cnt;
    rc = lseek(fp->fd, offset, origin);
    fp->cnt = 0;

    } else if (fp->flag & _WRITE) {
    // We're in a write access mode (includes append) so all nwee need to do is
    // write out any buffered characters then, if successful, seek to our new
    // position.  Origin does not matter in any case.
      nc = (fp->ptr - fp->base);
      if (nc > 0)
        if (write(fp->fd, fp->base, nc) != nc)
          rc = -1;
      if (rc != -1)
        rc = lseek(fp->fd, offset, origin);
    }
    // return appropriate return code
    return (rc == -1) ? -1 : 0;
}



// Myfread - read into the *ptr memory array, nobj objects of type whose sizeOf is size bytes
// Read from the file associated with MyFILE *fp
size_t Myfread(void *ptr, size_t size, size_t nobj, MyFILE *fp){
  int _fd = fp->fd;
  // if the file doesn't have read access --> set error
  if((fp->flag & _READ) == 0){
    fp->flag = _ERR;
    return 0;
  }
  // number of bytes requested
  int bytesreq = size*nobj;
  if(bytesreq == 0)
    return 0;

  // read into the ptr buffer
  size_t bytesread = read(_fd, (char *) ptr, bytesreq);

  return bytesreq==bytesread? nobj: (bytesread/size);
}

size_t Myfwrite(const void *ptr, size_t size, size_t nobj, MyFILE *fp){
  int _fd = fp->fd; // file descriptor
  // check for write permissions
  if((fp->flag & _WRITE) == 0){
    // fp->flag = _ERR;
    return 0;
  }

  // get the number of bytes written
  int numwritten = write(_fd, (char *) ptr, size*nobj);

  // if write system call had an error in execution -- there was an error
  if(numwritten<0){
    // fp->flag = _ERR;
    return 0;
  }

  return (numwritten/size);
}

int main()
{
  MyFILE *fp;
  // Testing for myfopen and Mygetc
  fp = myfopen("words.txt","r");
  int a = Mygetc(fp);
  printf("%c\n",a);
  Myfclose(fp);
  fp = myfopen("test1.txt","r");
  int b = Mygetc(fp);
  printf("%c\n", b);
  Myfclose(fp);
  fp = myfopen("test1.txt","r");
  int c,c1;
  MyFILE* f3 = myfopen("MyputcTest.txt","w");
  // Testing for multiple Myputc and Mygetc
  for(int i=0;i<8;i++)
  {
    c = Mygetc(fp);
    printf("***********\n");
    printf("%c\n",c);
    c1 = Myputc(c,f3);
    printf("%c\n",c);
  }
  // int c = Mygetc(fp);
  // printf("%c\n", c);
  // int d = Mygetc(fp);
  // printf("%c\n", d);
  Myfclose(fp);
  Myfclose(f3);
  fp = myfopen("words.txt","r");
  // Testing for Myfread and Myfwrite
  char* buf = malloc(sizeof(char)*15);
  int num = Myfread(buf, sizeof(char), 15, fp);
  MyFILE* f2;
  f2 = myfopen("created.txt","w");
  int wnum = Myfwrite(buf, sizeof(char),15, f2);
  printf("%d\n", num);
  printf("%d\n", wnum);
  Myfclose(f2);
  // Testing for append
  MyFILE* f4 = myfopen("created.txt","a");
  int wnum2 = Myfwrite(buf, sizeof(char),15, f2);
  printf("%d\n", wnum2);
  Myfclose(f4);
  Myfclose(fp);
  // Testing for lseek
  MyFILE* f5 = myfopen("test2.txt","r");
  MyFILE* f6 = myfopen("fseekTest.txt","w");
  MyFILE* f7 = myfopen("fseekTest2.txt","w");
  if(Myfseek(f5,5,0)>=0)
  {
    char* buf2 = malloc(sizeof(char)*15);
    int l = Myfread(buf2, sizeof(char), 15, f5);
    int l2 = Myfwrite(buf2, sizeof(char),15, f6);
    Myfseek(f5,1,1);
    char* buf3 = malloc(sizeof(char)*15);
    int ll = Myfread(buf3, sizeof(char), 15, f5);
    int ll2 = Myfwrite(buf3, sizeof(char),15, f7);
    Myfseek(f7,0L,2);
    int ll4 = Myfwrite(buf3, sizeof(char),15, f7);
    // printf("%d\n", l);
  }
  else
    printf("-1");
  Myfclose(f5);
  Myfclose(f6);
  Myfclose(f7);
  return 0;
}

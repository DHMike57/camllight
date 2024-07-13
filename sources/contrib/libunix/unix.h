#define Nothing ((value) 0)

#ifndef NULL
#ifdef ANSI
#define NULL ((void *) 0)
#else
#define NULL ((char *) 0)
#endif
#endif

#ifdef ANSI
extern void unix_error(int errcode, char * cmdname, value arg);
extern void uerror(char * cmdname, value arg);
#else
void unix_error();
void uerror();
#endif

#ifdef HAS_UNISTD
#include <unistd.h>
#endif

#ifdef HAS_SOCKETS
#include <sys/socket.h>
#endif /* HAS_SOCKETS */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <stdio.h>
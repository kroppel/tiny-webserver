/*
 * csapp.h - prototypes and definitions for the CS:APP3e book
 */
/* $begin csapp.h */
#ifndef __CSAPP_H__
#define __CSAPP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h> 	//open(), close() etc.
#include <string.h> 	//strlen(), strerror()
#include <errno.h>  	//strerror(errno)
#include <sys/socket.h> //klar
#include <sys/types.h> 	//sbuf struct
#include <sys/stat.h>	//open() OFLAGs
#include <netdb.h>  	//getnameinfo() etc.
#include <fcntl.h>	//open() OFLAGs
#include <sys/mman.h>	//mmap() function
#include <sys/wait.h>	//wait()


/* Simplifies calls to bind(), connect(), and accept() */
/* $begin sockaddrdef */
typedef struct sockaddr SA;
/* $end sockaddrdef */


/* External variables */
extern int h_errno;    /* Defined by BIND for DNS errors */ 
extern char **environ; /* Defined by libc */

/* Misc constants */
#define	MAXLINE	 8192  /* Max text line length */
#define MAXBUF   8192  /* Max I/O buffer size */
#define LISTENQ  1024  /* Second argument to listen() */


/* Reentrant protocol-independent client/server helpers */
int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port);

//https://man7.org/tlpi/code/online/dist/sockets/read_line.c
ssize_t readLine(int fd, void *buffer, size_t n);

#endif /* __CSAPP_H__ */
/* $end csapp.h */

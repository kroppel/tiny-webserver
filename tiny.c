#/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * http://localhost:8000/
 * http://localhost:8000/cgi-bin/adder?1&2
 */

#include "csapp.h"

/*Function prototypes*/
void doit(int fd); //überwacht den Serverprozess, managed unterprozesse
void read_requesthdrs(int fd);
int parse_uri(int fd, char *uri, char *filename, char *cgiargs); //Welcger content wird angefordert? Statisch/dynamishc?
void serve_static(int fd, char *filename, int filesize); //wrappe content in response
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs); //-"-
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *loongmsg); //gibt fehler an client zurück

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
			fprintf(stderr, "usage: %s <port>\n", argv[0]);
			exit(1);
    }

    listenfd = open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (SA*)&clientaddr, &clientlen);
        getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        close(connfd);
    }
}

/*
 * doit - handle one HTTP request/response transaction
 */

void doit(int fd)
{
    int is_static;
    struct stat sbuf, *sbufptr = &sbuf; //https://linux.die.net/man/2/stat
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];

    /* Read request line and headers */
    if (!readLine(fd, buf, MAXLINE))
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    //Is it a "GET" request?
    if (strcasecmp(method, "GET")) {                     //see: https://man7.org/linux/man-pages/man3/strcasecmp.3.html
        clienterror(fd, method, "501", "Not implemented", "Method is not implemented");
        return;
    }
    read_requesthdrs(fd);

    /* Parse URI (Uniform Resource Identifier) from GET request */
    is_static = parse_uri(fd, uri, filename, cgiargs);

    if (stat(filename, sbufptr) < 0) {
	    clienterror(fd, filename, "404", "Not found", "File was not found");
	    return;
    }
    /* Serve static content*/
    if (is_static) { /* Serve static content */
        //is it a valid file and do we have permissions?
	    if (!S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode)) { //https://linux.die.net/man/2/stat
            clienterror(fd, filename, "403", "Forbidden", "Could not read file");
            return;
	    }
	    serve_static(fd, filename, sbuf.st_size);
    }
    /* Serve dynamic content - using CGI (Common Gateway Interface) -> http://localhost:8000/cgi-bin/adder?1&2*/
    else {
        //is it a valid file and do we have permissions?
        if (!S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode)) {              //https://linux.die.net/man/2/stat and https://man7.org/linux/man-pages/man7/inode.7.html
	        clienterror(fd, filename, "403", "Forbidden", "Could not execute file");
	        return;
	    }
	    serve_dynamic(fd, filename, cgiargs);            //line:netp:doit:servedynamic
    }
}

/*
 * read_requesthdrs - read HTTP request headers
 */

void read_requesthdrs(int fd)
{
    char buf[MAXLINE];

    readLine(fd, buf, MAXLINE);
    printf("%s", buf);
    //HTTP request header ends with \r\n
    while(strcmp(buf, "\r\n")) {
        readLine(fd, buf, MAXLINE);
	printf("%s", buf);
    }
    return;
}


/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */

int parse_uri(int fd, char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    char buf[MAXLINE];

    if (!strstr(uri, "cgi-bin")) {  /* Static content */
	    strcpy(cgiargs, "");
	    strcpy(filename, ".");
	    strcat(filename, uri);
	    if (uri[strlen(uri)-1] == '/')
	        strcat(filename, "home.html");
	    return 1;
    }
    else {  /* Dynamic content */

	ptr = index(uri, '?');

        if (ptr) {
            strcpy(cgiargs, ptr+1);

            *ptr = '\0';
	}
	else {
	    strcpy(cgiargs, "");
	}

        strcpy(filename, ".");
        strcat(filename, uri);

	sprintf(buf, "\r\nURI: %s\r\nCGIARGS: %s\r\nFILENAME: %s\r\n", uri, cgiargs, filename);
	write(fd, buf, strlen(buf));

        return 0;
    }
}

/*
 * serve_static - copy a file back to the client
 */

void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    write(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    write(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n", filesize);
    write(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
    write(fd, buf, strlen(buf));

    /* Send response body to client */
    srcfd = open(filename, O_RDONLY, 0);
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); //https://man7.org/linux/man-pages/man2/mmap.2.html
    close(srcfd);
    write(fd, srcp, filesize);
    munmap(srcp, filesize);
}

/*
 * get_filetype - derive file type from file name
 */

void get_filetype(char *filename, char *filetype)
{
    if(strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if(strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
    else if(strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
	strcpy(filetype, "text/plain");
}

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Send response headers to client */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    write(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    write(fd, buf, strlen(buf));

    /*create 2nd process*/
    if(fork() == 0) {

        /*store parameter in environment variable*/
        if (setenv("QUERY_STRING", cgiargs, 1) != 0) {
	    sprintf(buf, "\r\nQUERY_STRING not set!\r\n");
	    write(fd, buf, strlen(buf));
        }

        /*reroute output to connfd*/
        dup2(fd, STDOUT_FILENO);

        /*run adder*/
        execve(filename, emptylist, environ);
    }

    wait(NULL);

    return;
}


/*
 * clienterror - returns an error message to the client
 */

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE];

    //write response header
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    write(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    write(fd, buf, strlen(buf));

    //write response body
    sprintf(buf, "<html><title>Tiny Error</title>");
    write(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    write(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s \r\n", errnum, shortmsg);
    write(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    write(fd, buf, strlen(buf));

}

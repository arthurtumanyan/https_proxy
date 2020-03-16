#ifndef CLIENT_H
#define	CLIENT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>
 
    int sockfd, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    struct timeval tv;
#ifdef	__cplusplus
}
#endif

#endif	/* CLIENT_H */


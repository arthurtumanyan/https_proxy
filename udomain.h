#ifndef UDOMAIN_H
#define	UDOMAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include "externals.h"
    
#define RCVBUF_SZ 1024

    int fd;
    struct sockaddr_un addr;
    struct sockaddr_un from;
    char buff[RCVBUF_SZ];
    char request[512];
    int len;
    
#ifdef	__cplusplus
}
#endif

#endif	/* UDOMAIN_H */


#ifndef BUFFER_H
#define	BUFFER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include	<netinet/in.h>		/* for IPPROTO_TCP value */

#define BUFFER_SZ       65535
    static int flag_buffer_not_empty = 0;

    struct {
        unsigned int mss;
        int nrecv;
        int nsend;
        int len;
        char data[65535];
    } ClientData[1024];

    struct {
        int epfd; // epoll fd
        int sfd; // src fd
        int dfd; // dst fd
        int n; // connection number
        struct epoll_event *lev;
        struct epoll_event *rev;
    } s_close;
#ifdef	__cplusplus
}
#endif

#endif	/* BUFFER_H */


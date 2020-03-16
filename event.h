#ifndef HTTPS_PROXY_H
#define	HTTPS_PROXY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <stdbool.h>    
#include <time.h>
#include "connections.h"
#include "cache.h"

    static int connections_cnt = 0;
    struct epoll_event listen_ev, events[MAX_EVENTS];
    int listen_sock = 0;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    struct sockaddr in_addr;
    socklen_t in_len;

    int yes = 1;
    bool listen_stop_flag = false;
    char ip[16], port[6];

#ifdef	__cplusplus
}
#endif

#endif	/* HTTPS_PROXY_H */


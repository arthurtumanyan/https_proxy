#include "client.h"
#include "externals.h"
#include "config.h"

int bind_to_ip(int fd, char *ip, u_short port) {
    struct sockaddr_in S;

    memset(&S, '\0', sizeof (S));
    S.sin_family = AF_INET;
    S.sin_port = htons(port);
    S.sin_addr.s_addr = inet_addr(ip);

    if (bind(fd, (struct sockaddr *) &S, sizeof (S)) == 0) {
        return 0;
    }
    logger(1, "%s: Cannot bind socket FD %d to %s:%d: %s\n",
            __FUNCTION__,
            fd,
            ip,
            (int) port,
            strerror(errno));
    return -1;
}

int open_client_connection() {
    int optval = 1;
    socklen_t optlen = sizeof (optval);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        logger(1, "%s: Can not open socket: %d (%s)", __FUNCTION__, errno, strerror(errno));
        return sockfd;
    }
    tv.tv_sec = Config.socket_timeout;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof (struct timeval));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof (struct timeval));
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
    // Set buffer size
    if (0 >= Config.remote_snd_buff_sz) {
        setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &Config.remote_snd_buff_sz, sizeof (Config.remote_snd_buff_sz));
    }
    if (0 >= Config.remote_rcv_buff_sz) {
        setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &Config.remote_rcv_buff_sz, sizeof (Config.remote_rcv_buff_sz));
    }
    return sockfd;
}

int _CONNECT(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec) {
    int flags, n, error;
    socklen_t len;
    fd_set rset, wset;
    struct timeval tval;

    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    error = 0;
    if ((n = connect(sockfd, (struct sockaddr *) saptr, salen)) < 0)
        if (errno != EINPROGRESS)
            return (-1);

    /* Do whatever we want while the connect is taking place. */

    if (n == 0)
        goto done; /* connect completed immediately */

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;

    if ((n = select(sockfd + 1, &rset, &wset, NULL,
            nsec ? &tval : NULL)) == 0) {
        close(sockfd); /* timeout */
        errno = ETIMEDOUT;
        return (-1);
    }

    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        len = sizeof (error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            return -1;
    } else
        logger(1, "%s: Error: %s", __FUNCTION__, strerror(errno));

done:
    fcntl(sockfd, F_SETFL, flags); /* restore file status flags */

    if (error) {
        close(sockfd); /* just in case */
        errno = error;
        return -1;
    }
    return 0;
}

int CONNECT(int fd, const struct sockaddr *saptr, socklen_t salen, int nsec) {

    int epfd = epoll_create(1);
    if (epfd < 0) {
        close(fd);
        return -1;
    }

    struct epoll_event ev;
    memset(&ev, 0, sizeof (ev));
    ev.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR;
    ev.data.fd = fd;

    int r = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    if (r < 0) {
        close(fd);
        return -1;
    }

    r = connect(fd, (struct sockaddr *) saptr, salen);
    int err = errno;
    if (r < 0 && err != EAGAIN && err != EINPROGRESS) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
        close(fd);
        return -1;
    }

    int n = epoll_wait(epfd, &ev, 1, nsec * 1000);
    if (n > 0) {
        if (ev.events & (EPOLLHUP | EPOLLERR)) {
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
            close(fd);
            return -1;
        }

        if (ev.events & EPOLLOUT) {
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
            return fd;
        }
    }
    return -1;
}

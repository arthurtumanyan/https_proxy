#include "buffer.h"
#include "config.h"
#include "externals.h"
#include "connections.h"

int write_to_buffer(int src_fd, int n) {

    char buf[BUFFER_SZ] = {0};
    int nrecv = recv(src_fd, buf, BUFFER_SZ, MSG_PEEK);
    if (nrecv < 0) {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
            logger(0, "%s: FD: %d (%s)", __FUNCTION__, src_fd, strerror(errno));
            return -1;
        }
    }
    if (BUFFER_SZ < (nrecv + ClientData[n].len)) {
        return 0;
    }

    ClientData[n].nrecv = recv(src_fd, ClientData[n].data, nrecv, 0);
    if (ClientData[n].nrecv == -1) {
        if (errno == ECONNRESET) {
            logger(0, "%s: FD: %d (%s)", __FUNCTION__, src_fd, strerror(errno));
            return -1;
        }
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            return 0;
        } else {
            return -1;
        }
    } else if (ClientData[n].nrecv == 0) {
        return -1;
    }
    ClientData[n].len += ClientData[n].nrecv;
    if (src_fd == sock_pair[n].local_sock) {
        Statistics[n].upload += ClientData[n].nrecv;
    }
    return 0;
}

int read_from_buffer(int dst_fd, int n) {
    int nodelay;
    if (0 == ClientData[n].len) {
        return 0;
    }
    ClientData[n].mss = 0;
    socklen_t sz = sizeof (ClientData[n].mss);
    if (0 > getsockopt(dst_fd, IPPROTO_TCP, TCP_MAXSEG, &ClientData[n].mss, &sz)) {
        logger(0, "%s: FD: %d (%s)", __FUNCTION__, dst_fd, strerror(errno));
    }

    if (Config.disable_nagle_algo) {
        if ((unsigned) ClientData[n].len < ClientData[n].mss) {
            nodelay = 1;
        } else {
            nodelay = 0;
        }
        setsockopt(dst_fd, SOL_TCP, TCP_NODELAY, &nodelay, sizeof (nodelay));
    }

    ClientData[n].nsend = send(dst_fd, ClientData[n].data, ClientData[n].len, MSG_DONTWAIT);
    if (ClientData[n].nsend == -1) {
        if (errno == ECONNRESET) {
            logger(0, "%s: FD: %d (%s)", __FUNCTION__, dst_fd, strerror(errno));
            return -1;
        }
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            return 0;
        } else {
            return -1;
        }
    } else if (ClientData[n].nsend == 0) {
        return -1;
    }
    if (dst_fd == sock_pair[n].local_sock) {
        Statistics[n].download += ClientData[n].nsend;
    }
    bzero(ClientData[n].data, sizeof (ClientData[n].data));
    ClientData[n].len = 0;

    if (1 == flag_buffer_not_empty) {
        close_conn();
        flag_buffer_not_empty = 0;
    }
    return 0;
}


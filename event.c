#include "event.h"
#include "externals.h"

#include "buffer.h"
#include "config.h"
#include "connections.h"
#include "cache.h"

static int success_hits = 0;
static int missed_hits = 0;
static int session_skipped = 0;

void close_conn() {
    logger(1, "%s: <TEST>", __FUNCTION__);
    // Close connection
    sock_pair[s_close.n].used = 0;
    sock_pair[s_close.n].local_sock = 0;
    sock_pair[s_close.n].remote_sock = 0;
    epoll_ctl(s_close.epfd, EPOLL_CTL_DEL, s_close.sfd, s_close.lev);
    epoll_ctl(s_close.epfd, EPOLL_CTL_DEL, s_close.dfd, s_close.rev);
    close(s_close.sfd);
    close(s_close.dfd);
    //  Clear struct s_close
    s_close.epfd = 0;
    s_close.sfd = 0;
    s_close.dfd = 0;
    s_close.n = 0;
    s_close.lev = NULL;
    s_close.rev = NULL;
    if (connections_cnt > 0) {
        connections_cnt--;
    }
}

void print_status(int sock) {

    int i;
    char status_str[1024] = {0};
    snprintf(status_str, 1024, "%s", "HTTP/1.1 200 OK\r\nServer: https_proxy\r\nContent-Type: text/html; charset=koi8-r\r\n\r\n");
    send(sock, status_str, strlen(status_str), 0);
    bzero(status_str, sizeof (status_str));
    // TODO
    // Realize paging support
    snprintf(status_str, 1024, "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content='3'><title>Connections status</title></head>"
            "<body><div align='left'>Total connections: %d</div><br />"
            "<div align='left'>Success hits: %d</div><br />"
            "<div align='left'>Failed hits: %d</div><br />"
            "<div align='left'>Session skipped: %d</div><br />"
            "<table width='100%%'>"
            "<tr align='center' bgcolor='a0a0a0'>"
            "<th>No</th>"
            "<th>Client</th>"
            "<th>Destination</th>"
            "<th>Domain</th>"
            "<th>Timestamp</th>"
            "<th>Sent</th>"
            "<th>Received</th>"
            "</tr>",
            connections_cnt, success_hits, missed_hits, session_skipped);
    send(sock, status_str, strlen(status_str), 0);
    bzero(status_str, sizeof (status_str));

    for (i = 0; i < Config.maxcon; i++) {
        if (sock_pair[i].used != 0) {
            snprintf(status_str, 1024, "<tr align='center' bgcolor='e0e0e0'>"
                    "<td>%d</td>"
                    "<td>%s</td>"
                    "<td>%s</td>"
                    "<td><a href='https://%s'>%s</a></td>"
                    "<td>%.3f</td>"
                    "<td>%ld</td>"
                    "<td>%ld</td>"
                    "</tr>",
                    i + 1,
                    (0 == (strcmp("", Statistics[i].client_ip))) ? "--" : Statistics[i].client_ip,
                    (0 == (strcmp("", Statistics[i].dst_ip))) ? "--" : Statistics[i].dst_ip,
                    (0 == (strcmp("", Statistics[i].domain))) ? "--" : Statistics[i].domain,
                    (0 == (strcmp("", Statistics[i].domain))) ? "" : Statistics[i].domain,
                    Statistics[i].session_start_timestamp,
                    Statistics[i].upload,
                    Statistics[i].download
                    );
            send(sock, status_str, strlen(status_str), 0);
            bzero(status_str, sizeof (status_str));
        }
    }
    snprintf(status_str, 1024, "%s", "</table></body></html>");
    send(sock, status_str, strlen(status_str), 0);
    bzero(status_str, sizeof (status_str));
}

int handle_status(char *data) {
    char *s = "GET /status HTTP/1.1";
    int n = strlen(s);
    if (0 == strncmp(s, data, n)) {
        return 1;
    }
    return 0;
}

int create_and_bind(char *listen_ip, int listen_port) {

    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&local_addr, sizeof (local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(listen_ip);
    local_addr.sin_port = htons(listen_port);

    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes)) < 0) {
        logger(0, "SETSOCKOPT(): %s", strerror(errno));
        halt(0);
    }

    if (0 != bind(listen_sock, (struct sockaddr *) &local_addr, sizeof (local_addr))) {
        logger(0, "Can not bind on %s:%d - %s", listen_ip, listen_port, strerror(errno));
        halt(0);
    }
    return listen_sock;
}

int setnonblocking(int sockfd) {
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK | O_ASYNC);
    return 0;
}

int init() {
    int i, sz;
    char tls_data[BUFFER_SZ];
    char *domain_name = malloc(sizeof (char) * 255);
    char *cached_domain_name;
    char *bind_ip = NULL;
    char *sess_id = NULL;
    struct timeval tv;

    tv.tv_sec = Config.socket_timeout;
    tv.tv_usec = 0;

    int listen_sock = create_and_bind(Config.listen_addr, Config.listen_port);

    if (0 != setnonblocking(listen_sock)) {
        logger(0, "Can not make nonblocking socket: %s", strerror(errno));
    } else {
        logger(1, "%s: Listen socket (%d) is nonblocking", __FUNCTION__, listen_sock);
    }

    if (-1 == listen(listen_sock, Config.maxcon)) {
        logger(0, "Listen failed: %s", strerror(errno));
        halt(0);
    }

    int epollfd = epoll_create(Config.maxcon);
    if (-1 == epollfd) {
        logger(0, "Error with epoll initialization: - %s", strerror(errno));
        halt(0);
    }

    listen_ev.events = EPOLLIN | EPOLLERR;
    listen_ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &listen_ev) == -1) {
        logger(0, "Error with epoll_ctl: - %s", strerror(errno));
        halt(0);
    }
    for (i = 0; i < Config.maxcon; i++) {
        sock_pair[i].used = 0;
        sock_pair[i].local_sock = 0;
        sock_pair[i].remote_sock = 0;
    }

    int nfds = 0, n = 0;
    socklen_t sockaddr_len = sizeof (struct sockaddr_in);

    while (!listen_stop_flag) {
        nfds = epoll_wait(epollfd, events, Config.maxcon, 5000);
        if (-1 == nfds) {
            continue;
        }

        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                int conn_sock;
                int remote_sock;

                conn_sock = accept(listen_sock, (struct sockaddr *) &local_addr, &sockaddr_len);
                if (conn_sock == -1) {
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                        /* We have processed all incoming
                           connections. */
                        continue;
                    } else {
                        logger(0, "Can not accept: %s", strerror(errno));
                        continue;
                    }
                }
                //
                socklen_t optlen = sizeof(yes);
                getnameinfo((struct sockaddr *) &local_addr, sockaddr_len, ip, sizeof ip, port, sizeof port, NI_NUMERICHOST | NI_NUMERICSERV);
                // Setting timeout to Config.socket_timeout seconds
                setsockopt(conn_sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof (struct timeval));
                setsockopt(conn_sock, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof (struct timeval));
                //
                setsockopt(conn_sock, SOL_SOCKET, SO_KEEPALIVE, &yes, optlen);
                
                if (Config.disable_nagle_algo) {
                    setsockopt(conn_sock, SOL_TCP, TCP_NODELAY, &yes, sizeof (yes));
                }
                
                // Set buffer size
                if (0 >= Config.local_snd_buff_sz) {
                    setsockopt(conn_sock, SOL_SOCKET, SO_SNDBUF, &Config.local_snd_buff_sz, sizeof (Config.local_snd_buff_sz));
                }
                
                if (0 >= Config.local_rcv_buff_sz) {
                    setsockopt(conn_sock, SOL_SOCKET, SO_RCVBUF, &Config.local_rcv_buff_sz, sizeof (Config.local_rcv_buff_sz));
                }
                
                if (0 < recv(conn_sock, &tls_data[0], sizeof (tls_data), MSG_PEEK)) {
                    if (handle_status(&tls_data[0])) {
                        recv(conn_sock, &tls_data[0], sizeof (tls_data), 0);
                        print_status(conn_sock);
                        close(conn_sock);
                        continue;
                    }
                    logger(0, "Connection attempt from %s,port %d", ip, atoi(port));
                    if (Config.use_cache) {
                        // get session_id
                        sess_id = get_client_session_id(&tls_data[0], sizeof (tls_data));
                        bzero(domain_name, 255);
                        // if session_id exists
                        if (sess_id && 0 < strlen(sess_id)) {
                            logger(1, "%s: ClientHello session id found: %s", __FUNCTION__, sess_id);
                            cached_domain_name = get_cache_element(sess_id);
                            // check if session_id is in cache
                            if (cached_domain_name) {
                                success_hits++;
                                bind_ip = get_outgoing_ip(cached_domain_name);
                                logger(1, "%s: Found ClientHello [%s:%s] pair in cache", __FUNCTION__, sess_id, cached_domain_name);
                            } else {
                                missed_hits++;
                                // otherwise clear session_id from ClientHello
                                logger(1, "%s: SessionID [%s] not found in the cache", __FUNCTION__, sess_id);
                                clear_session_id(&tls_data[0], sizeof (tls_data));
                                logger(1, "%s: Removed session id [%s] from ClientHello", __FUNCTION__, sess_id);

                            }
                        } else {
                            int retval = tls_fetch_domain(&tls_data[0], sizeof (tls_data), domain_name);
                            if (retval > 0 || retval == -2) {
                                session_skipped++;
                                // Categorize the domain
                                if (NULL != domain_name && 0 < strlen(domain_name)) {
                                    bind_ip = get_outgoing_ip(domain_name);
                                    logger(0, "Domain is '%s' for client with ip '%s'", domain_name, ip);
                                }
                            } else {
                                close(conn_sock);
                                continue;
                            }
                        }
                    } else {
                        int retval = tls_fetch_domain(&tls_data[0], sizeof (tls_data), domain_name);
                        if (retval > 0 || retval == -2) {
                            session_skipped++;
                            // Categorize the domain
                            if (NULL != domain_name && 0 < strlen(domain_name)) {
                                bind_ip = get_outgoing_ip(domain_name);
                                logger(0, "Domain is '%s' for client with ip '%s'", domain_name, ip);
                            }
                        } else {
                            close(conn_sock);
                            continue;
                        }
                    }
                } else {
                    close(conn_sock);
                    continue;
                }
                // Obtaining original destination for client
                bzero((char *) &remote_addr, sizeof (remote_addr));
                remote_addr.sin_family = AF_INET;
                socklen_t raddr_sz = sizeof (remote_addr);

                if (0 > getsockopt(conn_sock, SOL_IP, SO_ORIGINAL_DST, &remote_addr, &raddr_sz)) {
                    logger(0, "Can not get original destination for client ip '%s': %s ", ip, strerror(errno));
                    close(conn_sock);
                    continue;
                }
                char *original_dst = inet_ntoa(remote_addr.sin_addr);
                logger(1, "%s: Original destination for client with ip '%s' is '%s'", __FUNCTION__, ip, original_dst);
                remote_sock = open_client_connection();
                if (0 > remote_sock) {
                    close(conn_sock);
                    continue;
                }
                setnonblocking(remote_sock);
                //
                if (is_valid_ip(bind_ip)) {
                    bind_to_ip(remote_sock, bind_ip, 0);
                    logger(1, "%s: Outgoing address is %s for client with ip %s [fd : %d]", __FUNCTION__, bind_ip, ip, conn_sock);
                } else {
                    logger(1, "%s: Outgoing address not defined for client with ip %s [fd : %d]", __FUNCTION__, ip, conn_sock);
                    logger(0, "WARNING: Unable to categorize domain");
                }
                //
                if (CONNECT(remote_sock, (struct sockaddr *) &remote_addr, sizeof (remote_addr), Config.select_timeout) < 0) {
                    close(conn_sock);
                    continue;
                } else {
                    logger(0, "Connected to remote host %s:%d", original_dst, 443);
                    logger(1, "%s: Client connection descriptor: %d, destination socket descriptor: %d",
                            __FUNCTION__, conn_sock, remote_sock);
                }

                int find = -1;
                int i = 0;
                for (i = 0; i < Config.maxcon; i++) {
                    if (sock_pair[i].used == 0) {
                        find = i;
                        break;
                    }
                }
                if (find == -1) {
                    logger(0, "You are about to reach maximum connections limit [%d]", Config.maxcon);
                    close(conn_sock);
                    close(remote_sock);
                    continue;
                }

                sock_pair[find].used = 1;
                sock_pair[find].local_sock = conn_sock;
                sock_pair[find].remote_sock = remote_sock;

                setnonblocking(conn_sock);

                struct epoll_event local_ev, remote_ev;
                local_ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
                local_ev.data.fd = conn_sock;
                //
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &local_ev) == -1) {
                    logger(1, "%s: Error with epoll_ctl (conn_sock): - %s", __FUNCTION__, strerror(errno));
                    close(conn_sock);
                    close(remote_sock);
                    // clean up sock_pair struct
                    sock_pair[find].used = 0;
                    sock_pair[find].local_sock = 0;
                    sock_pair[find].remote_sock = 0;

                    continue;
                }

                remote_ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
                remote_ev.data.fd = remote_sock;

                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, remote_sock, &remote_ev) == -1) {
                    logger(1, "%s: Error with epoll_ctl (remote_sock): - %s", __FUNCTION__, strerror(errno));
                    close(conn_sock);
                    close(remote_sock);
                    //
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, conn_sock, &local_ev);
                    // clean up sock_pair struct
                    sock_pair[find].used = 0;
                    sock_pair[find].local_sock = 0;
                    sock_pair[find].remote_sock = 0;

                    continue;
                }
                //
                bzero(Statistics[n].client_ip, sizeof (Statistics[n].client_ip));
                if (NULL != ip) {
                    sz = sizeof (Statistics[n].client_ip);
                    strncpy(Statistics[n].client_ip, ip, sz);
                }
                //
                bzero(Statistics[n].dst_ip, sizeof (Statistics[n].dst_ip));
                if (NULL != original_dst) {
                    sz = sizeof (Statistics[n].dst_ip);
                    strncpy(Statistics[n].dst_ip, original_dst, sz);
                }

                bzero(Statistics[n].domain, sizeof (Statistics[n].domain));
                sz = sizeof (Statistics[n].domain);
                if (NULL != domain_name) {
                    strncpy(Statistics[n].domain, domain_name, sz);
                } else {
                    strncpy(Statistics[n].domain, cached_domain_name, sz);
                }

                Statistics[n].download = 0;
                Statistics[n].upload = 0;
                Statistics[n].session_start_timestamp = get_time_value();

                connections_cnt++;
            } else {

                int src_fd = events[n].data.fd;
                int dst_fd = -1;
                int find = -1;
                int i = 0;

                for (i = 0; i < Config.maxcon; i++) {
                    if (sock_pair[i].used != 0) {
                        if (sock_pair[i].local_sock == src_fd) {
                            dst_fd = sock_pair[i].remote_sock;
                            find = i;
                            break;
                        } else if (sock_pair[i].remote_sock == src_fd) {
                            dst_fd = sock_pair[i].local_sock;
                            find = i;
                            break;
                        }
                    }
                }

                if (dst_fd == -1) {
                    continue;
                }
                struct epoll_event local_ev, remote_ev;
                local_ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
                local_ev.data.fd = src_fd;
                remote_ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
                remote_ev.data.fd = dst_fd;

                if (events[n].events & (EPOLLIN | EPOLLPRI)) {
                    if (Config.use_cache) {
                        char buf[BUFFER_SZ] = {0};
                        if (0 < recv(src_fd, buf, BUFFER_SZ, MSG_PEEK)) {
                            char* server_session_id = get_server_session_id(&buf[0], sizeof (buf));
                            if (NULL != server_session_id && 0 < strlen(server_session_id)) {
                                char *dn = (NULL == domain_name) ? cached_domain_name : domain_name;
                                if (dn && 0 < strlen(dn)) {
                                    logger(1, "%s: Adding [%s:%s] pair to cache", __FUNCTION__, server_session_id, dn);
                                    add_cache_element(server_session_id, dn);
                                }
                            }
                        }
                    }
                    if (-1 == write_to_buffer(src_fd, n)) {
                        writeToAccessLog(n);
                        sock_pair[find].used = 0;
                        sock_pair[find].local_sock = 0;
                        sock_pair[find].remote_sock = 0;

                        epoll_ctl(epollfd, EPOLL_CTL_DEL, src_fd, &local_ev);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, dst_fd, &remote_ev);
                        //
                        close(src_fd);
                        logger(1, "%s: Closed socket FD %d", __FUNCTION__, src_fd);
                        close(dst_fd);
                        logger(1, "%s: Closed socket FD %d", __FUNCTION__, dst_fd);
                        if (connections_cnt > 0) {
                            connections_cnt--;
                        }
                        continue;
                    }
                }

                if (events[n].events & EPOLLOUT) {
                    if (-1 == read_from_buffer(dst_fd, n)) {
                        writeToAccessLog(n);
                        sock_pair[find].used = 0;
                        sock_pair[find].local_sock = 0;
                        sock_pair[find].remote_sock = 0;
                        //
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, src_fd, &local_ev);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, dst_fd, &remote_ev);
                        //
                        close(src_fd);
                        logger(1, "%s: Closed socket FD %d", __FUNCTION__, src_fd);
                        close(dst_fd);
                        logger(1, "%s: Closed socket FD %d", __FUNCTION__, dst_fd);
                        if (connections_cnt > 0) {
                            connections_cnt--;
                        }
                        continue;
                    }
                }

                if (events[n].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                    if (0 == ClientData[n].len) {
                        writeToAccessLog(n);
                        sock_pair[find].used = 0;
                        sock_pair[find].local_sock = 0;
                        sock_pair[find].remote_sock = 0;
                        //
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, src_fd, &local_ev);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, dst_fd, &remote_ev);
                        //
                        close(src_fd);
                        logger(1, "%s: Closed socket FD %d", __FUNCTION__, src_fd);
                        close(dst_fd);
                        logger(1, "%s: Closed socket FD %d", __FUNCTION__, dst_fd);
                        if (connections_cnt > 0) {
                            connections_cnt--;
                        }
                    } else {
                        s_close.dfd = dst_fd;
                        s_close.sfd = src_fd;
                        s_close.epfd = epollfd;
                        s_close.n = find;
                        s_close.lev = &local_ev;
                        s_close.rev = &remote_ev;

                        flag_buffer_not_empty = 1;
                    }
                    continue;
                }
            }
        }

        usleep(1000);
    }
    if (NULL != domain_name) {
        free(domain_name);
    }
    return 0;
}
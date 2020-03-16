#include "udomain.h"
#include "config.h"

char** splitString(char *stringToSplit) {
    int i = 0;
    char **data = malloc(2 * sizeof *data);
    char *tmp;
    tmp = strtok(stringToSplit, "\t");
    while (tmp != NULL) {
        data[i] = tmp;
        tmp = strtok(NULL, "\t");
        i++;
    }
    return data;
}

char * get_outgoing_ip(char *domain) {

    char **p = NULL;
    char *def_domain = "0.0.0.0";

    if ((NULL != domain) && (file_exists(Config.domain_sock))) {

        snprintf(request, 512, "GET /cc HTTP/1.0\r\n"
                "Host: %s\r\n"
                "Connection: Close\r\n\r\n", domain);
        if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
            logger(0, "Can not open domain socket: %s", strerror(errno));
        } else {
            memset(&addr, 0, sizeof (addr));
            addr.sun_family = AF_UNIX;
            strcpy(addr.sun_path, Config.domain_sock);
            if (CONNECT(fd, (struct sockaddr *) &addr, sizeof (addr), Config.select_timeout) == -1) {
                logger(0, "Can not connect to domain socket: %s", strerror(errno));
                return def_domain;
            }
            if (send(fd, request, strlen(request) + 1, 0) == -1) {
                logger(0, "Can not send to domain socket: %s", strerror(errno));
                close(fd);
                return def_domain;
            }
            if ((len = recv(fd, buff, RCVBUF_SZ, 0)) < 0) {
                logger(0, "Can not receive from domain socket: %s", strerror(errno));
                close(fd);
                return def_domain;
            }
            p = splitString(buff);
            def_domain = p[1];
        }

        if (fd >= 0) {
            close(fd);
        }
    }
    return def_domain;
}


#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h> 

#define MAXBUF 1024
#define FILENAME_LEN 256
#define DELIM "="
#define DEFAULT_CONFIG "/etc/https_proxy.conf"

    static const struct option longOpts[] = {
        { "config", required_argument, NULL, 'c'},
        { NULL, no_argument, NULL, 0}
    };

    char line[MAXBUF];

    struct {
        char config_file[FILENAME_LEN];
        char pid_file[FILENAME_LEN];
        char access_log[FILENAME_LEN];
        char custom_log[FILENAME_LEN];
        char debug_log[FILENAME_LEN];
        char domain_sock[FILENAME_LEN];
        char listen_addr[16];
        int listen_port;
        int maxcon;
        int socket_timeout;
        int select_timeout;
        int remote_snd_buff_sz;
        int remote_rcv_buff_sz;
        int local_snd_buff_sz;
        int local_rcv_buff_sz;
        bool disable_nagle_algo; // i.e set TCP_NODELAY for both local and remote connections socket 
        bool use_syslog;
        bool work_in_background;
        bool use_cache;
    } Config;

#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */


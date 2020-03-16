#ifndef CONNNECTIONS_H
#define	CONNNECTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define MAX_EVENTS 65535

    struct {
        double session_start_timestamp;
        long int upload;
        long int download;
        char domain[256];
        char client_ip[16];
        char dst_ip[16];
    } Statistics[MAX_EVENTS];

    struct conn_info {
        int used;
        int local_sock;
        int remote_sock;
    };
    struct conn_info sock_pair[MAX_EVENTS];

#ifdef	__cplusplus
}
#endif

#endif	/* CONNNECTIONS_H */


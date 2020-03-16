#ifndef EXTERNALS_H
#define	EXTERNALS_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>

    extern FILE * openCustomLog();
    extern void closeCustomLog();
    extern void logger(int, const char *fmt, ...);
    extern FILE * openDebugLog();
    extern void closeDebugLog();
    extern void writeToDebugLog(char *);
    extern FILE * openAccessLog();
    extern void writeToAccessLog(int);
    extern void closeAccessLog();
    //
    extern char * get_outgoing_ip(char *);
    extern char** splitString(char *);
    //
    extern int tls_fetch_domain(const char *, size_t, char *);
    extern void * xmalloc(size_t size);
    extern void trim(char* line);
    extern int open_client_connection();
    extern void set_sig_handler();
    extern int is_valid_ip(char *);
    extern bool is_valid_port(const char * p);
    extern int init();
    extern void halt(bool);
    extern int bind_to_ip(int, char *, u_short);
    extern int parse_server_name_extension(const char *, size_t, char *);
    extern int parse_extensions(const char *, size_t, char *);
    extern int read_from_buffer(int, int);
    extern int write_to_buffer(int, int);
    extern bool file_exists(char *);
    extern void read_config(char *);
    extern void save_pid();
    extern void remove_pid();
    extern int handle_status(char *);
    extern void print_status(int);
    extern int setnonblocking(int);
    extern int CONNECT(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec);
    extern char *get_client_session_id(char *, size_t);
    extern void clear_session_id(char *, size_t);
    extern char * get_server_session_id(const char *, size_t);
    extern void add_cache_element(char *, char *);
    extern char *get_cache_element(char *);
    extern void iterate_cache_elements();
    extern double get_time_value();
    extern void close_conn();
    //
#ifdef	__cplusplus
}
#endif

#endif	/* EXTERNALS_H */


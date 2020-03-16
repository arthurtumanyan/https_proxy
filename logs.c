
#include "logs.h"
#include "config.h"
#include "connections.h"

static int openlogcnt = 0;

FILE * openCustomLog() {
    openlog(IDENT, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    if (NULL == (custom_fd = fopen(Config.custom_log, "a+"))) {
        syslog(LOG_INFO, "Cannot create log file '%s' - %s\n", Config.custom_log, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return custom_fd;
}

void closeCustomLog() {
    if (NULL != custom_fd) {
        fclose(custom_fd);
        custom_fd = NULL;
    }
    closelog();
}

void writeToCustomLog(const char *fmt, va_list args) {
    ticks = time(NULL);
    char timestr[25];
    snprintf(timestr, 25, "%.25s", ctime(&ticks));

    char lbuffer[1024];
    memset(lbuffer, '\0', 1024);
    vsnprintf(lbuffer, 1024, fmt, args);

    if (Config.use_syslog) {
        syslog(LOG_INFO, "%s", lbuffer);
        return;
    }

    if (NULL != custom_fd) {
        fprintf(custom_fd, "[%s][pid %d] %s\n", timestr, getpid(), lbuffer);
        fflush(custom_fd);
    } else {
        if (openlogcnt <= 3) {
            openlogcnt++;
            custom_fd = openCustomLog();
        }
        syslog(LOG_INFO, "%s", lbuffer);
    }
}

FILE * openDebugLog() {

    if (NULL == (debug_fd = fopen(Config.debug_log, "a+"))) {
        fprintf(stderr, "Cannot create log file '%s' - %s\n", Config.debug_log, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return debug_fd;
}

void closeDebugLog() {

    if (NULL != debug_fd) {
        fclose(debug_fd);
        debug_fd = NULL;
    }
}

void writeToDebugLog(const char *fmt, va_list args) {

    ticks = time(NULL);
    char timestr[25];
    snprintf(timestr, 25, "%.25s", ctime(&ticks));
    char lbuffer[1024];
    memset(lbuffer, '\0', 1024);
    vsnprintf(lbuffer, 1024, fmt, args);

    if (NULL != debug_fd) {
        fprintf(debug_fd, "[%s][pid %d] %s\n", timestr, getpid(), lbuffer);
        fflush(debug_fd);
    }
}

FILE * openAccessLog() {
    if (NULL == (access_fd = fopen(Config.access_log, "a+"))) {
        printf("Cannot create log file '%s' - %s\n", Config.access_log, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return access_fd;

}

void writeToAccessLog(int n) {

    if (NULL != access_fd) {
        fprintf(access_fd, "%-16s %-16s %s %14.3lf %6ld %6ld\n",
                (0 == strcmp("", Statistics[n].client_ip)) ? "--" : Statistics[n].client_ip,
                (0 == strcmp("", Statistics[n].dst_ip)) ? "--" : Statistics[n].dst_ip,
                (0 == strcmp("", Statistics[n].domain)) ? "--" : Statistics[n].domain,
                Statistics[n].session_start_timestamp,
                Statistics[n].upload,
                Statistics[n].download);
        fflush(access_fd);
    }
}

void closeAccessLog() {

    if (NULL != access_fd) {
        fclose(access_fd);
        access_fd = NULL;
    }
}

void logger(int debug, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (debug) {
        writeToDebugLog(fmt, args);
    } else {
        writeToCustomLog(fmt, args);
    }
}

double get_time_value() {
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (double) tv.tv_sec + (double) 1e-6 * tv.tv_usec; 
}
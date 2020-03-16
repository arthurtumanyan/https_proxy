#ifndef LOGS_H
#define	LOGS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
 
#define IDENT           "https_proxy"

    time_t ticks;
    FILE * access_fd = NULL;
    FILE * custom_fd = NULL;
    FILE * debug_fd = NULL;


#ifdef	__cplusplus
}
#endif

#endif	/* LOGS_H */


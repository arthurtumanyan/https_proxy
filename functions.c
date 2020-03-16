#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "externals.h"
#include "config.h"


bool file_exists(char *path) {
    struct stat buffer;
    int exist = stat(path, &buffer);
    if (exist == 0) {
        return 1;
    } else {
        return 0;
    }
}

bool is_valid_port(const char * p) {
    int port = 0, z = 0;
    if (p == NULL) {
        return false;
    }
    if (0 != (z = sscanf(p, "%d", &port))) {
        if (port > 0 && port <= 65535) {
            return true;
        }
    }
    return false;
}

int is_valid_ip(char *str) {
    int segs = 0; /* Segment count. */
    int chcnt = 0; /* Character count within segment. */
    int accum = 0; /* Accumulator for segment. */
    /* Catch NULL pointer. */
    if (str == NULL)
        return 0;
    /* Process every character in string. */
    while (*str != '\0') {
        /* Segment changeover. */
        if (*str == '.') {
            /* Must have some digits in segment. */
            if (chcnt == 0)
                return 0;
            /* Limit number of segments. */
            if (++segs == 4)
                return 0;
            /* Reset segment values and restart loop. */
            chcnt = accum = 0;
            str++;
            continue;
        }
        /* Check numeric. */
        if ((*str < '0') || (*str > '9'))
            return 0;
        /* Accumulate and check segment. */
        if ((accum = accum * 10 + *str - '0') > 255)
            return 0;
        /* Advance other segment specific stuff and continue loop. */
        chcnt++;
        str++;
    }
    /* Check enough segments and enough characters in last segment. */
    if (segs != 3)
        return 0;
    if (chcnt == 0)
        return 0;
    /* Address okay. */
    return 1;
}

void halt(bool success) {
    
    logger(0, "Shutting down");
    remove_pid();
    closeAccessLog();
    closeCustomLog();
    closeDebugLog();

    exit((success) ? EXIT_SUCCESS : EXIT_FAILURE);
}

void trim(char* line) {
    int l;

    l = strlen(line);
    while (line[l - 1] == '\n' || line[l - 1] == '\r')
        line[--l] = '\0';
}

void * xrealloc(void *ptr, size_t size) {
    void *new_mem = (void *) realloc(ptr, size);
    if (new_mem == NULL) {
        logger(1, "%s: Can not allocate memory", __FUNCTION__);
        halt(0);
    }
    return new_mem;
}

void * xmalloc(size_t size) {
    void * new_mem = (void *) malloc(size);
    if (new_mem == NULL) {
        logger(1, "%s: Can not allocate memory", __FUNCTION__);
        halt(0);
    }
    return new_mem;
}

void * xcalloc(size_t nmemb, size_t size) {
    void *new_mem = (void *) calloc(nmemb, size);
    if (new_mem == NULL) {
        logger(1, "%s: Can not allocate memory", __FUNCTION__);
        halt(0);
    }
    return new_mem;
}

void save_pid() {
    FILE *pid_fd;
    if ((pid_fd = fopen(Config.pid_file, "w")) == NULL) {
        logger(0, "Can not create PID file '%s': %s", Config.pid_file, strerror(errno));
    }
    fprintf(pid_fd, "%d", getpid());
    if (pid_fd) {
        fclose(pid_fd);
    }
}

void remove_pid() {
    if (0 > unlink(Config.pid_file)) {
        logger(0, "Can not unlink file '%s',%s", Config.pid_file, strerror(errno));
    }
}
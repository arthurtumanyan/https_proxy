#include "config.h"
#include "externals.h"

void remove_spaces(char* line) {
    char* i = line;
    char* j = line;

    while (*j != 0) {
        *i = *j++;
        if (*i != ' ')
            i++;
    }
    *i = 0;
}

bool skip_line(char *line) {

    return (line[0] == '\0' ||
            line[0] == '\n' ||
            line[0] == '\r' ||
            (line[0] == '\r' && line[1] == '\n') ||
            line[0] == '#' || // Shell style comment
            (line[0] == '/' && line[1] == '/') // C style comment
            );
}

void read_config(char *filename) {

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("%s: %s\n", __FUNCTION__, strerror(errno));
        halt(1);
    }
    bzero(Config.access_log, sizeof (Config.access_log));
    bzero(Config.custom_log, sizeof (Config.custom_log));
    bzero(Config.debug_log, sizeof (Config.debug_log));
    bzero(Config.domain_sock, sizeof (Config.domain_sock));
    bzero(Config.listen_addr, sizeof (Config.listen_addr));
    bzero(Config.pid_file, sizeof (Config.pid_file));
    Config.listen_port = -1;
    Config.maxcon = -1;
    Config.use_syslog = 0;
    Config.use_cache = 0;
    Config.work_in_background = 0;
    Config.select_timeout = 1;
    Config.socket_timeout = 3;
    Config.remote_rcv_buff_sz = 65535;
    Config.remote_snd_buff_sz = 65535;
    Config.local_rcv_buff_sz = 65535;
    Config.local_snd_buff_sz = 65535;
    Config.disable_nagle_algo = false;

    char *p = NULL;

    while (fgets(line, sizeof (line), file) != NULL) {
        if (skip_line(line)) {
            continue;
        }
        remove_spaces(line);
        p = strtok(line, DELIM);
        if (p) {
            // Reading pid_file value
            if (0 == strcmp("pid_file", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                strncpy(Config.pid_file, p, sizeof (Config.pid_file) - 1);
                continue;
            }
            // Reading access_log value
            if (0 == strcmp("access_log", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                strncpy(Config.access_log, p, sizeof (Config.access_log) - 1);
                continue;
            }
            // Reading custom_log value
            if (0 == strcmp("custom_log", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                strncpy(Config.custom_log, p, sizeof (Config.custom_log) - 1);
                continue;
            }
            // Reading debug_log value
            if (0 == strcmp("debug_log", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                strncpy(Config.debug_log, p, sizeof (Config.debug_log) - 1);
                continue;
            }
            // Reading listen_addr
            if (0 == strcmp("listen_addr", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                if (is_valid_ip(p)) {
                    strncpy(Config.listen_addr, p, sizeof (Config.listen_addr) - 1);
                } else {
                    printf("Invalid 'listen_addr' value\n");
                    halt(1);
                }
                continue;
            }
            // Reading domain_sock value
            if (0 == strcmp("domain_sock", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                strncpy(Config.domain_sock, p, sizeof (Config.domain_sock) - 1);
                continue;
            }
            // Reading listen_port
            if (0 == strcmp("listen_port", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                if (is_valid_port(p)) {
                    Config.listen_port = atoi(p);
                } else {
                    printf("Invalid 'listen_port' value\n");
                    halt(1);
                }
                continue;
            }
            // Reading maxcon count
            if (0 == strcmp("maxcon", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                Config.maxcon = atoi(p);
                continue;
            }
            // Reading select timeout
            if (0 == strcmp("select_timeout", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                Config.select_timeout = atoi(p);
                continue;
            }
            // Reading socket timeout
            if (0 == strcmp("socket_timeout", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                Config.socket_timeout = atoi(p);
                continue;
            }
            // Reading SO_SNDBUF value for remote host (using in open_client_connection())
            if (0 == strcmp("remote_snd_buff_sz", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                Config.remote_snd_buff_sz = atoi(p);
                continue;
            }
            // Reading SO_RCVBUF value for remote host (using in open_client_connection())
            if (0 == strcmp("remote_rcv_buff_sz", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                Config.remote_rcv_buff_sz = atoi(p);
                continue;
            }
            // Reading SO_SNDBUF value for local host (using in init())
            if (0 == strcmp("local_snd_buff_sz", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                Config.local_snd_buff_sz = atoi(p);
                continue;
            }
            // Reading SO_RCVBUF value for local host (using in init())
            if (0 == strcmp("local_rcv_buff_sz", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                Config.local_rcv_buff_sz = atoi(p);
                continue;
            }
            // Reading use_syslog value
            if (0 == strcmp("use_syslog", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                if (0 == strcmp("true", p)) {
                    Config.use_syslog = true;
                } else if (0 == strcmp("false", p)) {
                    Config.use_syslog = false;
                } else {
                    printf("Invalid 'use_syslog' value\n");
                    halt(1);
                }
                continue;
            }
            // Reading use_cache value
            if (0 == strcmp("use_cache", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                if (0 == strcmp("true", p)) {
                    Config.use_cache = true;
                } else if (0 == strcmp("false", p)) {
                    Config.use_cache = false;
                } else {
                    printf("Invalid 'use_cache' value\n");
                    halt(1);
                }
                continue;
            }
            // Reading use_cache value
            if (0 == strcmp("use_cache", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                if (0 == strcmp("true", p)) {
                    Config.use_cache = true;
                } else if (0 == strcmp("false", p)) {
                    Config.use_cache = false;
                } else {
                    printf("Invalid 'use_cache' value\n");
                    halt(1);
                }
                continue;
            }
            // Reading disable_nagle_algo value
            if (0 == strcmp("disable_nagle_algo", p)) {
                p = strtok(NULL, DELIM);
                trim(p);
                if (0 == strcmp("true", p)) {
                    Config.disable_nagle_algo = true;
                } else if (0 == strcmp("false", p)) {
                    Config.disable_nagle_algo = false;
                } else {
                    printf("Invalid 'disable_nagle_algo' value\n");
                    halt(1);
                }
                continue;
            }
        }
    }

    if (0 == strcmp("", Config.access_log)) {
        printf("%s: access_log not defined\n", __FUNCTION__);
        halt(1);
    }
    if (0 == strcmp("", Config.custom_log)) {
        printf("%s: custom_log not defined\n", __FUNCTION__);
        halt(1);
    }
    if (0 == strcmp("", Config.debug_log)) {
        printf("%s: debug_log not defined\n", __FUNCTION__);
        halt(1);
    }
    if (0 == strcmp("", Config.domain_sock)) {
        printf("%s: domain_sock not defined\n", __FUNCTION__);
        halt(1);
    }
    if (0 == strcmp("", Config.listen_addr)) {
        printf("%s: listen_addr not defined\n", __FUNCTION__);
        halt(1);
    }
    if (0 == strcmp("", Config.pid_file)) {
        printf("%s: pid_file not defined\n", __FUNCTION__);
        halt(1);
    }
    if (0 > Config.listen_port) {
        printf("%s: listen_port not defined\n", __FUNCTION__);
        halt(1);
    }
    if (0 > Config.maxcon) {
        printf("%s: maxcon not defined\n", __FUNCTION__);
        halt(1);
    }

    fclose(file);
}

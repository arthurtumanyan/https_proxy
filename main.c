#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "externals.h"
#include "config.h"

int opt = 0;
int longIndex = 0;

bool is_valid_opt(char *opts) {
    if (opts == NULL)return false;
    if (opts[0] == '-' || opts[0] == ':' || opts[0] == '?')return false;
    return true;
}

int main(int argc, char **argv) {

    static const char *optString = "c:?";
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);

    while (opt != -1) {
        switch (opt) {
            case 'c':
                if (!is_valid_opt(optarg)) {
                    printf("Usage: %s [-c <config file>]\n", argv[0]);
                    printf("Usage: %s [--config <config file>]\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                strncpy(Config.config_file, optarg, sizeof (Config.config_file) - 1);
                break;
            default:
                printf("Usage: %s [-c <config file>]\n", argv[0]);
                printf("Usage: %s [--config <config file>]\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    // Install signal handlers
    set_sig_handler();
    // Read configuration
    if (0 == strcmp("", Config.config_file)) {
        strncpy(Config.config_file, DEFAULT_CONFIG, FILENAME_LEN - 1);
    }
    read_config(Config.config_file);
    // Open debug log file
    openDebugLog();
    // Open custom log file
    openCustomLog();
    // Open accesslog file
    openAccessLog();
    logger(0, "Starting...");
    save_pid();
    // Daemonizing
    if (Config.work_in_background) {
        if (0 > daemon(1, 0)) {
            logger(0, "Daemonizing failed: %s", strerror(errno));
        } else {
            logger(0, "Daemonized");
        }
    }

    // Init listening and connection handling
    init();
    // Handle shutdown
    halt(1);
    return 0; // this will never happen
}


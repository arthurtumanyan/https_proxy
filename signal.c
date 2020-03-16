#include "signal.h"
#include "config.h"
#include "cache.h"

typedef void Sigfunc(int);

Sigfunc * signal(int signo, Sigfunc *func) {
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM) {
        act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */
    } else {
        act.sa_flags |= SA_RESTART; /* SVR4, 44BSD */
    }
    if (sigaction(signo, &act, &oact) < 0)
        return (SIG_ERR);
    return (oact.sa_handler);
}

/* end signal */
Sigfunc * Signal(int signo, Sigfunc *func) /* for our signal() function */ {
    Sigfunc *sigfunc;
    if ((sigfunc = signal(signo, func)) == SIG_ERR) {
        // TODO
    }
    return (sigfunc);
}

void signal_usr1(int sig) {
    logger(0, "Caught cache dump signal", sig);
    iterate_cache_elements();
}

void signal_usr2(int sig) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    logger(0, "Caught usage dump signal", sig);

    printf("%32s: %ld\n", "Max resident set size (kb)", usage.ru_maxrss);
    printf("%32s: %ld\n", "Sharing text segment memory (kb/s)", usage.ru_ixrss);
    printf("%32s: %ld\n", "Data segment memory (kb/s)", usage.ru_idrss);
    printf("%32s: %ld\n", "Stack memory used (kb/s)", usage.ru_isrss);
    printf("%32s: %ld\n", "Soft page faults", usage.ru_minflt);
    printf("%32s: %ld\n", "Hard page faults", usage.ru_majflt);
    printf("%32s: %ld\n", "Input operations", usage.ru_inblock);
    printf("%32s: %ld\n", "Output operations", usage.ru_oublock);
}

void signal_hup(int sig) {
    logger(0, "Caught reconfiguration signal", sig);
    read_config(Config.config_file);
}

void signal_term(int sig) {
    logger(0, "Caught termination signal: %d", sig);
    halt(1);
}

void print_trace(int sig) {
    logger(0, "Caught signal: %d", sig);
    void *array[10];
    size_t size;
    char **strings;
    size_t i;
    size = backtrace(array, 10);
    strings = backtrace_symbols(array, size);
    printf("Obtained %zd stack frames.\n", size);
    for (i = 0; i < size; i++)
        printf("%s\n", strings[i]);
    free(strings);
    halt(0);
}

void set_sig_handler() {

    Signal(SIGINT, signal_term);
    Signal(SIGHUP, signal_hup);
    Signal(SIGUSR1, signal_usr1);
    Signal(SIGUSR2, signal_usr2);
    Signal(SIGTRAP, SIG_IGN);
    Signal(SIGCHLD, SIG_IGN);
    Signal(SIGTSTP, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);
    Signal(SIGTTIN, SIG_IGN);
    //Signal(SIGIO, SIG_IGN);
    Signal(SIGABRT, print_trace);
    Signal(SIGPIPE, SIG_IGN);
    Signal(SIGSEGV, print_trace);
    Signal(SIGBUS, print_trace);
    Signal(SIGWINCH, SIG_IGN);
    Signal(SIGTERM, signal_term);
}
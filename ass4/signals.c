#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

// writes a ! to stderr, used as signal handler.
void handler(int signal) {
    write(STDERR_FILENO, "! ", 2);
}

// testing behaviour of sigwait
int main(int argc, char** argv) {
    printf("pid: %d\n", getpid());

    // ignore the SIGHUP signal
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sigaction(SIGHUP, &sa, NULL);

    // a signal set containing only SIGHUP
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGHUP);
    // block SIGHUP from being processed by a signal handler
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    while (1) {
        int sig;
        // block and wait for SIGHUP
        sigwait(&sigset, &sig);
        printf("sigwait got signal: %d %s\n", sig, strsignal(sig));
    }
}

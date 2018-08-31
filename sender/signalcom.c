#define _POSIX_SOURCE 
#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "signalcom.h"

#include "../shared/err.h"

volatile pid_t child = -1;
volatile pid_t caller_pid;

#define SIGNAL_ID SIGTERM

void signal_handler(int sig) {
    if (sig != SIGNAL_ID) return;

    if (child > 0) {
        waitpid(child, NULL, 0);
    }
    pid_t me = getpid();
    if (me != caller_pid) { 
        _exit(1);
    }
}

void reg_handler() {
    if (signal(SIGNAL_ID, signal_handler) == SIG_ERR) {
        syserr("signal register");
    }
}

void unreg_handler() {
    if (signal(SIGNAL_ID, SIG_IGN) == SIG_ERR) {
        syserr("signal unregister");
    }
}

void send_signal() {
    caller_pid = getpid();
    pid_t gpid = getpgid(caller_pid);
    kill(-gpid, SIGTERM);
}

// Just calls signal to quit every other process before killing 
void syserr_sig(const char *fmt) {
    send_signal();
    syserr(fmt);
}

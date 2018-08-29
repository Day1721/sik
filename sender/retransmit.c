#define _GNU_SOURCE

#include <stdbool.h>
#include <fcntl.h> 
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "../shared/err.h"

#include "retransmit.h"

unsigned int militime() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        syserr("clock_gettime");
    }

    return (ts.tv_nsec / 1000000) + ts.tv_sec * 1000;
}

void milisleep(unsigned int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}


void run_resend(params_t* params, int ctrl_pipe, int trans_pipe) {
    fcntl(ctrl_pipe, F_SETFL, O_NONBLOCK);

    unsigned int sleep_milis = params->resend_time;
    while (true) {
        milisleep(sleep_milis);
        time_t start_time = militime();

        uint64_t value;
        while (read(ctrl_pipe, &value, sizeof(uint64_t)) > 0) {
            if (write(trans_pipe, &value, sizeof(uint64_t)) < 0) {
                syserr("write to trans_pipe");
            }
        }

        if (errno != EAGAIN) {
            syserr("read from ctrl_pipe");
        }

        time_t end_time = militime();
        sleep_milis = params->resend_time - (end_time - start_time);
    }
}
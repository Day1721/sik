#define _GNU_SOURCE

#include <time.h>

#include "err.h"
#include "time.h"

int militime() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        return -1;
    }

    return (ts.tv_nsec / 1000000) + ts.tv_sec * 1000;
}

void milisleep(unsigned int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
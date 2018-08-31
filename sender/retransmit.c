#include <stdbool.h>
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>

#include "signalcom.h"
#include "../shared/time.h"

#include "retransmit.h"

#define syserr(message) syserr_sig(message)

void run_resend(params_t* params, int ctrl_pipe, int trans_pipe) {
    fcntl(ctrl_pipe, F_SETFL, O_NONBLOCK);

    unsigned int sleep_milis = params->resend_time;
    int cnt = 0;
    while (true) {
        milisleep(sleep_milis);
        time_t start_time = militime();

        uint64_t value;
        while (read(ctrl_pipe, &value, sizeof(uint64_t)) > 0) {
            if (write(trans_pipe, &value, sizeof(uint64_t)) < 0) {
                syserr("write to trans_pipe");
            }
        }

        cnt++;
        if (cnt > 1000) {
            syserr("TESTING");
        }

        if (errno != EAGAIN) {
            syserr("read from ctrl_pipe");
        }

        time_t end_time = militime();
        sleep_milis = params->resend_time - (end_time - start_time);
    }
}
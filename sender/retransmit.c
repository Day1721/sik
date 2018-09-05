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
    while (true) {
        int tmp;
        milisleep(sleep_milis);
        if ((tmp = militime()) < 0) {
            syserr("militime");
        }
        time_t start_time = tmp; 

        uint64_t value;
        while (read(ctrl_pipe, &value, sizeof(uint64_t)) > 0) {
            if (write(trans_pipe, &value, sizeof(uint64_t)) < 0) {
                syserr("write to trans_pipe");
            }
        }

        if (errno != EAGAIN) {
            syserr("read from ctrl_pipe");
        }

        if ((tmp = militime()) < 0) {
            syserr("militime");
        }
        time_t end_time = tmp;
        sleep_milis = params->resend_time - (end_time - start_time);
    }
}
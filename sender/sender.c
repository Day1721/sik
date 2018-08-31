#define _POSIX_SOURCE 
#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

#include "../shared/time.h"

#include "signalcom.h"
#include "data.h"
#include "control.h"
#include "retransmit.h"
#include "transmit.h"
#include "sender.h"

#define BUFFER_SIZE 100
#define END_TIMEOUT_MS 1

#define syserr(message) syserr_sig(message)

void run_sender(params_t* params) {
    reg_handler();

    int pipe_io[2];
    if (pipe(pipe_io) < 0) {
        syserr("pipe D->T");
    }

    switch (child = fork()) {
        case -1:
            syserr("fork 1");
            break;

        case 0:
            close(pipe_io[1]);
            int pipe_out = pipe_io[0];//dp_pipe_out

            if (pipe(pipe_io) < 0) {
                syserr("pipe R->T");
            }

            switch (child = fork()) {
                case -1:
                    syserr("fork 2");
                    break;
                
                case 0:
                    close(pipe_out);
                    close(pipe_io[0]);
                    pipe_out = pipe_io[1];

                    if (pipe(pipe_io) < 0) {
                        syserr("pipe C->R");
                    }

                    switch(child = fork()) {
                        case -1:
                            syserr("fork 3");
                            break;
                        
                        case 0:
                            close(pipe_out);
                            close(pipe_io[0]);

                            run_ctrl(params, pipe_io[1]);
                            break;

                        default:
                            close(pipe_io[1]);

                            run_resend(params, pipe_io[0], pipe_out);
                            break;
                    }
                    break;

                default:
                    close(pipe_io[1]);

                    run_transmit(params, pipe_out, pipe_io[0]);
                    break;
            }
            break;

        default:
            close(pipe_io[0]);
            run_data(params, pipe_io[1]);

            send_signal();

            int status;
            waitpid(child, &status, 0);
            break;
    }
}

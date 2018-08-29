#ifndef RETRANSMIT_H
#define RETRANSMIT_H

#include "structs.h"

void run_resend(params_t* params, int ctrl_pipe, int trans_pipe);

#endif //RETRANSMIT_H
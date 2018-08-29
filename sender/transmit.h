#ifndef TRANSMIT_H
#define TRANSMIT_H

#include "structs.h"

#define MULTCAST_TTL_VALUE 4

void run_transmit(params_t* params, int pack_pipe, int resend_pipe);

#endif //TRANSMIT_H
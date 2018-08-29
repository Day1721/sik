#ifndef PARAMS_H
#define PARAMS_H

#include <netinet/in.h>

typedef struct params {
    in_addr_t multicast_ip;
    char* multicast_ip_str;
    in_port_t data_port;
    in_port_t ctrl_port;
    size_t pack_size;
    size_t fifo_size;
    unsigned int resend_time;
    char* name;
} params_t;

#endif //PARAMS_H
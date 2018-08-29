#include <stdbool.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../shared/err.h"
#include "../shared/parse.h"
#include "../shared/addr.h"

#include "sender.h"

#define DATA_PORT 21359
#define CTRL_PORT 31359
#define PSIZE 512 /*(B)*/
#define FSIZE 128*1024 /*(B)*/
#define NAME "Nienazwany Nadajnik"
#define RTIME 250

char* exec_name = NULL;

void exit_print_usage(char param, char* message) {
    printf("Parameter '%c': %s\n", param, message);
    printf("Usage: %s -a MCAST_ADDR [-P DATA_PORT] [-C CTRL_PORT] [-p PSIZE] [-f FSIZE] [-n NAZWA]\n",
            exec_name);
    exit(1);
}

int main(int argc, char* argv[]) {
    exec_name = argv[0];

    in_addr_t mcast_addr = 0;
    char* mcast_addr_str = NULL;
    int data_port = -1;
    int ctrl_port = -1;
    int pack_size = -1;
    int fifo_size = -1;
    int resend_time = -1;
    char* name = NULL;

    int opt_char;
    while ((opt_char = getopt(argc, argv, "a:P:C:p:f:R:n:")) != -1) {
        switch (opt_char) {
            case 'a':
                if (mcast_addr != 0) {
                    exit_print_usage(opt_char, "parameter's duplicate");
                }
                mcast_addr_str = optarg;
                mcast_addr = parse_address(optarg);
                if (mcast_addr == 0) {
                    exit_print_usage(opt_char, "a parameter should be a multicast ip address");
                }
                break;

            case 'P':
                if (data_port != -1) {
                    exit_print_usage(opt_char, "parameter's duplicate");
                } else {
                    data_port = checked_strtol(optarg);
                    if (data_port == -1) {
                        exit_print_usage(opt_char, "invalid format");
                    }
                }
                break;

            case 'C':
                if (ctrl_port != -1) {
                    exit_print_usage(opt_char, "parameter's duplicate");
                } else {
                    ctrl_port = checked_strtol(optarg);
                    if (ctrl_port == -1) {
                        exit_print_usage(opt_char, "invalid format");
                    }
                }
                break;

            case 'p':
                if (pack_size != -1) {
                    exit_print_usage(opt_char, "parameter's duplicate");
                } else {
                    pack_size = checked_strtol(optarg);
                }
                break;

            case 'f':
                if (fifo_size != -1) {
                    exit_print_usage(opt_char, "parameter's duplicate");
                } else {
                    fifo_size = checked_strtol(optarg);
                    if (fifo_size == -1) {
                        exit_print_usage(opt_char, "invalid format");
                    }
                }
                break;

            case 'R':
                if (resend_time != -1) {
                    exit_print_usage(opt_char, "parameter's duplicate");
                } else {
                    resend_time = checked_strtol(optarg);
                    if (fifo_size == -1) {
                        exit_print_usage(opt_char, "invalid format");
                    }
                }
                break;

            case 'n':
                if (name != NULL) {
                    exit_print_usage(opt_char, "parameter duplicate");
                } else if (strlen(optarg) > 64) {
                    exit_print_usage(opt_char, "too long name");
                } else {
                    name = optarg;
                }
                break;

            default:
                break;
        }
    }

    if (mcast_addr == 0) {
        exit_print_usage('a', "parameter is required");
    }

    params_t params = {
        .multicast_ip = mcast_addr,
        .multicast_ip_str = mcast_addr_str,
        .data_port = data_port != -1 ? data_port : DATA_PORT,
        .ctrl_port = ctrl_port != -1 ? ctrl_port : CTRL_PORT,
        .pack_size = pack_size != -1 ? pack_size : PSIZE,
        .fifo_size = fifo_size != -1 ? fifo_size : FSIZE,
        .resend_time = resend_time != -1 ? resend_time : RTIME,
        .name = name != NULL ? name : NAME,
    };


    run_sender(&params);

    return 0;
}

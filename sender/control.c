#define _GNU_SOURCE

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 

#include "../shared/parse.h"

#include "signalcom.h"
#include "proto.h"
#include "control.h"

#define syserr(message) syserr_sig(message)

static int get_ctrl_sock(params_t* params) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        syserr("socket in run_ctrl");
    }

    struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(params->ctrl_port);

	if (bind(sock, (struct sockaddr *) &server_address, (socklen_t) sizeof(server_address)) < 0) {
		syserr("bind in run_ctrl");
    }


    return sock;
}


void parse_rexmit_body(char* buffer, int pipe_fd) {
    for (char* p = strtok(buffer, ","); p != NULL; p = strtok(NULL, ",")) {
        int64_t val = str_to_i64(p);
        if (val == -1) {
            // ignore invalid values in retransmit package (long-staying server)
        } else {
            bool is_end = false;
            // little weird convension for last retransmit package id
            if (val < 0) {
                is_end = true;
                val = -val;
            }
            if (write(pipe_fd, &val, sizeof(int64_t)) != sizeof(int64_t)) {
                syserr("write ctrl_pipe");
            }
            if (is_end) break;
        }
    }
}

void parse_ctrl(params_t* params, int sock, char* buffer, int pipe_fd, struct sockaddr* client, socklen_t client_size) {
    if (strcmp(buffer, CTRL_LOOKUP) == 0) {
        //LOOKUP
        int len = sprintf(buffer, "%s %s %d %.64s\n", CTRL_REPLY, params->multicast_ip_str, params->data_port, params->name);
        if(len < 0) {
            syserr("sprintf");
        }
        
        if (sendto(sock, buffer, len, 0, client, client_size) < 0) {
            syserr("write ctrl_sock");
        }
    } else if (strncmp(buffer, CTRL_REXMIT, strlen(CTRL_REXMIT)) == 0) {
        //REXMIT
        buffer += strlen(CTRL_REXMIT) + 1;
        parse_rexmit_body(buffer, pipe_fd);
    }
}




void run_ctrl(params_t* params, int pipe_fd) {
    fcntl(pipe_fd, F_SETFL, O_NONBLOCK);
    int sock = get_ctrl_sock(params);
    
    struct ip_mreq mreq;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    mreq.imr_multiaddr.s_addr = params->multicast_ip;
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq)) < 0) {
        syserr("setsockopt ADD_MEMBERSHIP");
    }
    // int resuseaddr_param = 1;
    // if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&resuseaddr_param, sizeof(resuseaddr_param)) < 0) {
    //     syserr("setsockopt SO_REUSEADDR");
    // }
    // in_addr_t any_addr = INADDR_ANY;
    // if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (void*)&any_addr, sizeof(any_addr)) < 0) {
    //     syserr("setsockopt IP_MULTICAST_IF");
    // }

    char buffer[MAX_CTRL_BUFFER_SIZE];

    while(true) {
        struct sockaddr_in client;
        socklen_t client_size = sizeof(client);
        ssize_t len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&client, &client_size);
        if (len < 0) {
            syserr("recvfrom in run_ctrl");
        }
        if (len == 0) continue;

        buffer[len] = 0; // just in case

        parse_ctrl(params, sock, buffer, pipe_fd, (struct sockaddr*)&client, client_size);
    }
    
    // if (setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq)) < 0) {
    //     syserr("setsockopt DROP_MEMBERSHIP");
    // }
    close(sock);
}
#include <stdbool.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>

#include "../shared/addr.h"
#include "../shared/err.h"
#include "../shared/list.h"

#include "structs.h"
#include "transmit.h"

size_t packs_len = 0;
list2_t* packages = NULL;

package_t* get_curr_pack(list2_t* curr) {
    return (package_t*)curr->val;
}

package_t* find_package(uint64_t id) {
    if (packages == NULL) return NULL;
    if (get_curr_pack(packages)->first_byte_num == id) { 
        return get_curr_pack(packages); 
    }

    for (list2_t* curr = packages->next; curr != packages; curr = curr->next) {
        if (get_curr_pack(curr)->first_byte_num == id) { 
            return get_curr_pack(curr); 
        }
    }
    return NULL;
}

void update_packages(params_t* params, char* serialized_pack) {
    size_t max_len = params->fifo_size / pack_size(params->pack_size);
    if (max_len <= packs_len) {
        package_t* pack = get_curr_pack(packages->prev);
        deserialize_to(serialized_pack, params->pack_size, pack);

        packages = packages->prev;
    } else {
        package_t* pack = (package_t*)malloc(sizeof(package_t));
        deserialize_to(serialized_pack, params->pack_size, pack);

        packages = insert_before(packages, pack);
        max_len++;
    }
}

static int make_data_sock() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        syserr("socket in run_data");
    }

    int optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
        syserr("setsockopt broadcast in run_data");
    }

    // optval = MULTCAST_TTL_VALUE;
    // if (setsockopt(sock, SOL_SOCKET, IP_MULTICAST_TTL, &optval, sizeof(optval)) < 0) {
    //     syserr("setsockopt ttl in run_data");
    // }

    // struct sockaddr_in local_address;
    // local_address.sin_family = AF_INET;
    // local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    // local_address.sin_port = htons(params->data_port);
    // if (bind(sock, (struct sockaddr *)&local_address, sizeof local_address) < 0) {
    //     syserr("bind local in run_data");
    // }

    return sock;
}

void read_package(params_t* params, int sock, struct sockaddr_in remote, int pipe_fd) {
    size_t size = pack_size(params->pack_size);
    char buffer[size];
    
    ssize_t rw_res;
    if ((rw_res = read(pipe_fd, buffer, size)) < 0 || ((size_t)rw_res) != size) {
        syserr("read data pipe");
    }

    update_packages(params, buffer);
    if ((rw_res = sendto(sock, buffer, size, 0, (struct sockaddr*) &remote, sizeof(remote))) < 0 || ((size_t)rw_res) != size) {
        syserr("sendto data_sock new");
    }
    // if (write(sock, buffer, size) != size) {
    //     syserr("write data_sock");
    // }
}

void read_resend(params_t* params, int sock, struct sockaddr_in remote, int pipe_fd) {
    uint64_t pack_id;
    if (read(pipe_fd, &pack_id, sizeof(uint64_t)) != sizeof(uint64_t)) {
        syserr("read resend pipe");
    }

    package_t* to_send = find_package(pack_id);
    if (to_send == NULL) {
        return; // ignore request if package not found
    }
    
    size_t size = pack_size(params->pack_size);
    char buffer[size];
    serialize_to(to_send, buffer, params->pack_size);

    ssize_t rw_res;
    if ((rw_res = sendto(sock, buffer, size, 0, (struct sockaddr*) &remote, sizeof(remote))) < 0 || ((size_t)rw_res) != size) {
        syserr("sendto data_sock resend");
    }
    
    // if (write(sock, buffer, size) != size) {
    //     syserr("write data_sock");
    // }
}

void run_transmit(params_t* params, int pack_pipe, int resend_pipe) {
    int sock = make_data_sock();

    struct sockaddr_in remote_address;
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(params->data_port);
    remote_address.sin_addr.s_addr = params->multicast_ip;

    struct pollfd fds[2];
    fds[0].fd = pack_pipe;
    fds[1].fd = resend_pipe;
    fds[0].events = fds[1].events = POLLIN;

    while (true) {
        fds[0].revents = fds[1].revents = 0;
        int res = poll(fds, 2, 1000);
        if (res < 0) {
            syserr("poll");
        }
        if (fds[0].revents & POLLIN) {
            read_package(params, sock, remote_address, pack_pipe);
        }
        if (fds[1].revents & POLLIN) {
            read_resend(params, sock, remote_address, pack_pipe);
        }
    }
}
#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "../shared/addr.h"
#include "signalcom.h"

#include "data.h"

#define syserr(message) syserr_sig(message)

package_t* prepare_package(params_t* params, char* buff) {
    static uint64_t first_byte = 0;
    package_t* res = new_package(first_byte, buff, params->pack_size);
    first_byte += params->pack_size;
    return res;
}

// yes, I really wrote it, cause I don't really know 
// how read EXACTLY n bytes from pipe until EOF (read don't work as expected here)
size_t read_from_stdin(char* buffer, size_t buffsize) {
    int c;
    for (size_t i = 0; i < buffsize; i++) {
        switch(c = getchar()) {
            case EOF:
                return i;
            default:
                buffer[i] = c;
        }
    }
    return buffsize;
}

void run_data(params_t* params, int pipe_fd) {
    char buffer[params->pack_size];
    char send_buffer[pack_size(params->pack_size)];
    while (true) {
        size_t len = read_from_stdin(buffer, params->pack_size);
        if (len != params->pack_size) break;

        package_t* package = prepare_package(params, buffer);
        serialize_to(package, send_buffer, params->pack_size);
        free_package(package);

        ssize_t w_res = write(pipe_fd, send_buffer, pack_size(params->pack_size));
        if (w_res < 0 || ((size_t) w_res) < pack_size(params->pack_size)) {
            syserr("pipe write");
        } 
    }
}

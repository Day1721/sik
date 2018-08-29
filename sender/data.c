#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "../shared/addr.h"
#include "../shared/err.h"

#include "data.h"

package_t* prepare_package(params_t* params, char* buff) {
    static uint64_t first_byte = 0;
    package_t* res = new_package(first_byte, buff, params->pack_size);
    first_byte += params->pack_size;
    return res;
}

void run_data(params_t* params, int pipe_fd) {
    char buffer[params->pack_size];
    char send_buffer[pack_size(params->pack_size)];
    while (true) {
        ssize_t slen = read(STDIN_FILENO, buffer, params->pack_size);
        if (slen < 0) syserr("read stdin in run_data");
        size_t len = (size_t)slen;
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

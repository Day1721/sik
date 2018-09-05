#ifndef SHARED_ADDR_H
#define SHARED_ADDR_H

#include <netinet/in.h>
#include <stdbool.h>

typedef struct package {
    uint64_t session_id;
    uint64_t first_byte_num;
    char* audio_data;
} package_t;

package_t* new_package(uint64_t first_byte_num, char* audio, size_t audio_len);
void free_package(package_t* package);

void serialize_to(package_t* package, char* buffer, size_t audio_len);
void deserialize_to(char* buffer, size_t audio_len, package_t* package);
size_t pack_size(size_t audio_size);

// bool addr_eq(struct sockaddr_in left, struct sockaddr_in right);

in_addr_t parse_address(char* addr);

uint64_t htonll(uint64_t n);
uint64_t ntohll(uint64_t n);

bool port_valid(int port_nr);

#endif //SHARED_ADDR_H

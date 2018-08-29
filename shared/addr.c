#define _DEFAULT_SOURCE

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "addr.h"
#include "../shared/err.h"

package_t* new_package(uint64_t first_byte_num, char* audio, size_t len) {
    static uint64_t session_id = 0;
    if (session_id == 0) session_id = time(NULL);

    package_t* res = (package_t*)malloc(sizeof(package_t));
    res->first_byte_num = first_byte_num;
    res->session_id = session_id;
    res->audio_data = (char*)malloc(sizeof(char)*len);
    memcpy(res->audio_data, audio, len);
    return res;
}

void free_package(package_t* package) {
    free(package->audio_data);
    free(package);
}

void serialize_to(package_t* package, char* buffer, size_t len) {
    uint64_t* cbuff = (uint64_t*)buffer;
    cbuff[0] = htonll(package->session_id);
    cbuff[1] = htonll(package->first_byte_num);

    memcpy(buffer + 2*sizeof(uint64_t), package->audio_data, len);
}

void deserialize_to(char* buffer, size_t len, package_t* package) {
    uint64_t* cbuff = (uint64_t*)buffer;
    package->session_id = ntohll(cbuff[0]);
    package->first_byte_num = ntohll(cbuff[1]);
    
    if (package->audio_data != NULL) { 
        free(package->audio_data);
    }

    package->audio_data = (char*)malloc(sizeof(char)*len);
    memcpy(package->audio_data, buffer + 2*sizeof(uint64_t), len);
}

size_t pack_size(size_t audio_size) {
    return audio_size + 2*sizeof(uint64_t);
}

// bool addr_eq(struct sockaddr_in left, struct sockaddr_in right) {
//     if (left.sin_addr.s_addr != right.sin_addr.s_addr) return false;
//     if (left.sin_port != right.sin_port) return false;
//     return true;
// }

static bool is_multicast(in_addr_t addr) {
    uint32_t address = ntohl(addr);
    return (address & 0xF0000000) == 0xE0000000;
}

in_addr_t parse_address(char* addr) {
    struct in_addr res;
    if (inet_aton(addr, &res) == 0)
        syserr("inet_aton");
    
    return is_multicast(res.s_addr) ? res.s_addr : 0;
}

uint64_t htonll(uint64_t n) {
#if __BYTE_ORDER == __BIG_ENDIAN
    return n; 
#else
    return (((uint64_t)htonl(n)) << 32) + htonl(n >> 32);
#endif
}

uint64_t ntohll(uint64_t n) {
    return htonll(n);
}
#ifndef SHARED_PROTO_H
#define SHARED_PROTO_H

const char* CTRL_LOOKUP = "ZERO_SEVEN_COME_IN";
const char* CTRL_REPLY = "BOREWICZ_HERE";
const char* CTRL_REXMIT = "LOUDER_PLEASE";

const int MAX_NAME_LEN = 64;


const int MAX_CTRL_BUFFER_SIZE = 10000; // in case of even larger packages use 2^16

#endif //SHARED_PROTO_H

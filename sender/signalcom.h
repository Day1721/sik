#ifndef SIGNALCOM_H
#define SIGNALCOM_H

#include <sys/wait.h>

extern volatile pid_t child;

void reg_handler();
void unreg_handler();
void send_signal();

void syserr_sig(const char *fmt);

#endif //SIGNALCOM_H
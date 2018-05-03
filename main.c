#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "err.h"

#define QUEUE_LENGTH 10
#define BUFFER_SIZE 10

#define bool int
#define TRUE 1
#define FALSE 0

#define TELNET_SGA 3

#define TELNET_SUPDUP 21

#define TELNET_LINEMODE 34
#define TELNET_LINEMODE_FM 2

#define TELNET_SE 0xf0
#define TELNET_NOP 0xf1
#define TELNET_DATA_MARK 0xf2
#define TELNET_BREAK 0xf3
#define TELNET_INTERRUPT 0xf4
#define TELNET_ABORT_OUTPUT 0xf5
#define TELNET_ARE_YOU_THERE 0xf6
#define TELNET_ERASE_CHAR 0xf7
#define TELNET_ERASE_LINE 0xf8
#define TELNET_GO_AHEAD 0xf9
#define TELNET_SB 0xfa
#define TELNET_WILL 0xfb
#define TELNET_WONT 0xfc
#define TELNET_DO 0xfd
#define TELNET_DONT 0xfe
#define TELNET_IAC 0xff

// ARROW = {27,91,x}
// UP_ARROW = ARROW[x->65]
// DOWN_ARROW = ARROW[x->66]
// RIGHT_ARROW = ARROW[x->67]
// LEFT_ARROW = ARROW[x->68]
// ENTER = {13,0}
// PAGE = {27,91,x,126}
// PG_UP = PAGE[x->53]
// PG_DOWN = PAGE[x->54]

typedef ssize_t (*io_func_t)(int, void*, size_t);

int checked(io_func_t handler, int sock, void* buff, size_t cnt) {
	int res;
	if ((res = handler(sock, buff, cnt)) < 0) {
		syserr("write");
	}
	return res;
	
}

int checked_write(int sock, const void* buff, size_t cnt) {
	return checked((io_func_t) write, sock,(char*) buff, cnt);
} 

int checked_read(int sock, void* buff, size_t cnt) {
	return checked(read, sock, buff, cnt);
}

bool is_number(const char *str) {
    while(*str != '\0')
    {
        if(*str < '0' || *str > '9')
            return FALSE;
        str++;
    }
    return TRUE;
}

int strncmp_unsigned(unsigned char* p1, unsigned char *p2, int len) {
	int i;
	for (i = 0; i < len; i++) {
		if ((*p1) && (*p1 == *p2)) {
			p1++;
			p2++;
		} else break;
	}
	return i == len ? 0 : (*p1 - *p2);
}

//LINEMODE(34)->FORWARDMASK(2)->everything
int init_telnet_opts(int sock) {
	unsigned char sga_neg[3] = {TELNET_IAC, TELNET_WILL, TELNET_SGA};
	unsigned char sga_neg_exp[3] = {TELNET_IAC, TELNET_DO, TELNET_SGA};
	checked_write(sock, sga_neg, 3);
	checked_read(sock, sga_neg, 3);
	if (strncmp_unsigned(sga_neg, sga_neg_exp, 3)) {
		printf("can't negotiate sga");
		return 1;
	}
	
	unsigned char linemode_neg[3] = { TELNET_IAC, TELNET_DO, TELNET_LINEMODE };
	unsigned char linemode_neg_exp[3] = {TELNET_IAC, TELNET_WILL, TELNET_LINEMODE };
	checked_write(sock, linemode_neg, 3);
	checked_read(sock, linemode_neg, 3);
	if(strncmp_unsigned(linemode_neg, linemode_neg_exp, 3)) {
		printf("can't negotiate linemode, got %x %x %x\n", linemode_neg[0], linemode_neg[1], linemode_neg[2]);
		return 1;
	}

	unsigned int forwardmask_neg_size = 64+7;
	unsigned char forwardmask_neg[forwardmask_neg_size];
	//unsigned char forwardmask_neg_exp[7] = {TELNET_IAC, TELNET_SB, TELNET_LINEMODE, TELNET_WILL, TELNET_LINEMODE_FM, TELNET_IAC, TELNET_SE};
	forwardmask_neg[0] = TELNET_IAC;
	forwardmask_neg[1] = TELNET_SB;
	forwardmask_neg[2] = TELNET_LINEMODE;
	forwardmask_neg[3] = TELNET_DO;
	forwardmask_neg[4] = TELNET_LINEMODE_FM;
	for(unsigned int i = 5; i < forwardmask_neg_size - 2; i++) {
		forwardmask_neg[i] = TELNET_IAC;
	}
	forwardmask_neg[forwardmask_neg_size - 2] = TELNET_IAC;
	forwardmask_neg[forwardmask_neg_size - 1] = TELNET_SE;
	checked_write(sock, forwardmask_neg, forwardmask_neg_size);
	int len = checked_read(sock, forwardmask_neg, forwardmask_neg_size);
	for (int i = 0; i < len; i++) printf("%d ", forwardmask_neg[i]);
	printf("\n");
	len = checked_read(sock, forwardmask_neg, forwardmask_neg_size);
	for (int i = 0; i < len; i++) printf("%d ", forwardmask_neg[i]);
	printf("\n");
	//if(strncmp_unsigned(forwardmask_neg, forwardmask_neg_exp, 7)) {
	//	printf("can't negotiate forwardmask, got (%d) %x %x %x %x %x\n", len, forwardmask_neg[0], forwardmask_neg[1], forwardmask_neg[2], forwardmask_neg[3], forwardmask_neg[4]);
	//	return 1;
	//}
	
	unsigned char supdup_neg[10] = {TELNET_IAC, TELNET_DO, TELNET_SUPDUP };
	unsigned char supdup_neg_exp[3] = {TELNET_IAC, TELNET_WILL, TELNET_SUPDUP };
	checked_write(sock, supdup_neg, 3);
	len = checked_read(sock, supdup_neg, 10);
	for (int i = 0; i < len; i++) printf("%d ", supdup_neg[i]);
	printf("\n");

	unsigned char supdup_neg2[10] = {TELNET_IAC, TELNET_WILL, TELNET_SUPDUP };
	unsigned char supdup_neg_exp2[3] = {TELNET_IAC, TELNET_DO, TELNET_SUPDUP };
	checked_write(sock, supdup_neg2, 3);
	len = checked_read(sock, supdup_neg2, 10);
	for (int i = 0; i < len; i++) printf("%d ", supdup_neg2[i]);
	printf("\n");
	
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("usage: %s port\n", argv[0]);
		return 1;
	}
	if (!is_number(argv[1])) {
		printf("expected uint parameter: port number, but got %s\n", argv[1]);
		return 1;
	}
	int port = atoi(argv[1]);
	
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) syserr("socket");

	struct sockaddr_in server_address;
	
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		syserr("bind");

	if (listen(sock, QUEUE_LENGTH) < 0) syserr("listen");

	char buffer[BUFFER_SIZE];
	while(1) {
		struct sockaddr_in client_address;
		socklen_t client_address_len = sizeof(client_address); 
    	int msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);

		if(init_telnet_opts(msg_sock)) {
			exit(1);
			if (close(msg_sock) < 0)
				syserr("close");
			continue;
		}

		ssize_t len;
		do {
			len = read(msg_sock, buffer, sizeof(buffer));
			if (len < 0)
				syserr("reading from client socket");
			else {
				printf("read from socket: %zd bytes: ", len);
				for (int i = 0; i < len; i++) printf("%d ", buffer[i]);
				printf("\n");
				int snd_len = write(msg_sock, buffer, len);
				if (snd_len != len)
					syserr("writing to client socket");
			}
		} while (len > 0);
		printf("ending connection\n");
		if (close(msg_sock) < 0)
			syserr("close");
	}

	return 0;
}

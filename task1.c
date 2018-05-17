#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "err.h"

#define QUEUE_LENGTH 10
#define BUFFER_SIZE 10

#define bool int
#define TRUE 1
#define FALSE 0

#define TELNET_ECHO 1
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

#define min(a,b) a > b ? b : a
#define max(a,b) a > b ? a : b

char* CLEAN_SCREEN = "\033[2J\033[H";
unsigned char UP_ARROW[] = {27,91,65};
unsigned char DOWN_ARROW[] = {27,91,66};
unsigned char RIGHT_ARROW[] = {27,91,67};
unsigned char LEFT_ARROW[] = {27,91,68};
unsigned char ENTER[] = {13,0};
unsigned char PAGE_UP[] = {27,91,53,126};
unsigned char PAGE_DOWN[] = {27,91,54,126};

typedef ssize_t (*io_func_t)(int, void*, size_t);

int checked(io_func_t handler, int sock, void* buff, size_t cnt, char* err_message) {
	int res;
	if ((res = handler(sock, buff, cnt)) < 0) {
		syserr(err_message);
	}
	return res;
	
}

int checked_write(int sock, const void* buff, size_t cnt) {
	return checked((io_func_t) write, sock,(char*) buff, cnt, "write");
} 

int checked_read(int sock, void* buff, size_t cnt) {
	return checked(read, sock, buff, cnt, "read");
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

bool is_number(const char *str) {
    while(*str != '\0')
    {
        if(*str < '0' || *str > '9')
            return FALSE;
        str++;
    }
    return TRUE;
}

int mod(int a, int b) {
   int ret = a % b;
   if(ret < 0)
     ret += b;
   return ret;
}

typedef struct state {
	int level;
	int position;
	char bottom[3];
} state_t;

bool update_state(state_t* state, unsigned char* message, int len) {
	if (!strncmp_unsigned(message, UP_ARROW, len)) {
		state->position = mod(state->position - 1, 3); 
		state->bottom[0] = 0;
	} else if (!strncmp_unsigned(message, DOWN_ARROW, len)) {
		state->position = mod(state->position + 1, 3); 
		state->bottom[0] = 0;
	} else if (!strncmp_unsigned(message, ENTER, len)) {
		if (state->position == 2) {
			bool res = state->level == 0;
			state->level = max(state->level - 1, 0);
			return res;
		}
		switch(state->level) {
			case 0:
				if (state->position == 0) {
					state->bottom[0] = 'A';
					state->bottom[1] = 0;
				} else {
					state->bottom[0] = 0;
					state->level++;
					state->position = 0;
				}
				break;

			case 1:
				state->bottom[0] = 'B';
				state->bottom[1] = state->position + 1 + '0';
				state->bottom[2] = 0;
				break;

			default:
			fprintf(stderr, "error in get_menu, level out of range: %d", state->level);
			exit(2);

		}
	} else if (!strncmp_unsigned(message, PAGE_UP, len)) {
		state->level = state->level - 1 > 0 ? 0 : state->level - 1;
	}
	
	return FALSE;
}

char* get_menu(state_t state) {
	char* options[3];
	int len = 3;
	switch(state.level) {
		case 0:
			options[0] = "OPCJA A";
			options[1] = "OPCJA B";
			options[2] = "ZAKOŃCZ";
			break;

		case 1:
			options[0] = "OPCJA B1";
			options[1] = "OPCJA B2";
			options[2] = "POWRÓT";
			break;

		default:
			fprintf(stderr, "error in get_menu, level out of range: %d", state.level);
			exit(2);
	}

	char* res = (char*)malloc(sizeof(char)*1000);
	memset(res, 0, 1000);
	for (int i = 0; i < len; i++) {
		if (state.position == i) {
			strcat(res, "\033[7m");
			strcat(res, options[i]);
			strcat(res, "\033[m");
		} else strcat(res, options[i]);
		strcat(res, "\r\n");
	}

	return res;
}


int init_telnet_opts(int sock) {
	unsigned char sga_neg[] = {TELNET_IAC, TELNET_WILL, TELNET_SGA};
	unsigned char sga_neg_exp[] = {TELNET_IAC, TELNET_DO, TELNET_SGA};
	checked_write(sock, sga_neg, 3);
	checked_read(sock, sga_neg, 3);
	if (strncmp_unsigned(sga_neg, sga_neg_exp, 3)) {
		printf("can't negotiate sga");
		return 1;
	}
	
	unsigned char echo_neg[] = {TELNET_IAC, TELNET_WILL, TELNET_ECHO};
	unsigned char echo_neg_exp[] = {TELNET_IAC, TELNET_DO, TELNET_ECHO};
	checked_write(sock, echo_neg, 3);
	checked_read(sock, echo_neg, 3);
	if (strncmp_unsigned(echo_neg, echo_neg_exp, 3)) {
		printf("can't negotiate echo");
		return 1;		
	}

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

	errno = 0;
	char* endptr;
	long port = strtol(argv[1], &endptr, 10);
	if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN)) || (errno != 0 && port == 0)) {
		syserr("strtol");
	}

	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) syserr("socket");

	struct sockaddr_in server_address;
	
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		syserr("bind");

	if (listen(sock, QUEUE_LENGTH) < 0) syserr("listen");

	unsigned char buffer[BUFFER_SIZE];
	while(TRUE) {
		struct sockaddr_in client_address;
		socklen_t client_address_len = sizeof(client_address); 
    	int msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);

		if(init_telnet_opts(msg_sock)) {
			if (close(msg_sock) < 0)
				syserr("close");
			continue;
		}

		state_t state;
		state.level = state.position = 0;
		char* menu = get_menu(state);
		checked_write(msg_sock, CLEAN_SCREEN, strlen(CLEAN_SCREEN));
		checked_write(msg_sock, menu, strlen(menu));
		free(menu);

		ssize_t len;
		do {
			len = checked_read(msg_sock, buffer, sizeof(buffer));
			
			if (len == 0) break;

			fprintf(stderr, "read from socket: %zd bytes: ", len);
			for (int i = 0; i < len; i++) fprintf(stderr, "%d ", buffer[i]);
			fprintf(stderr, "\n");

			bool end = update_state(&state, buffer, len);
			if (end) {
				break;
			} else {
				menu = get_menu(state);
				checked_write(msg_sock, CLEAN_SCREEN, strlen(CLEAN_SCREEN));
				checked_write(msg_sock, menu, strlen(menu));
				free(menu);
				if (state.bottom[0] != 0) {
					checked_write(msg_sock, state.bottom, strlen(state.bottom));
					checked_write(msg_sock, "\r\n", 2);
				}
			}
		} while (len > 0);
		printf("ending connection\n");
		if (close(msg_sock) < 0)
			syserr("close");
	}

	return 0;
}

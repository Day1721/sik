CFLAGS=-Wall -Wextra -O2 -std=c11 
CC=gcc

all: main

err.o: err.c err.h
	$(CC) $(CFLAGS) -c -o err.o err.c

main: main.c err.o
	$(CC) $(CFLAGS) -o main main.c err.o

main-debug: main.c err.o
	$(CC) $(CFLAGS) -g -o main main.c err.o

clean:
	rm main err.o

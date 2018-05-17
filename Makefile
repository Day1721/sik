CFLAGS=-Wall -Wextra -O2 -std=c11 
CC=gcc

all: main

err.o: err.c err.h
	$(CC) $(CFLAGS) -c -o err.o err.c

task1: task1.c err.o
	$(CC) $(CFLAGS) -o task1 task1.c err.o

task1-debug: task1.c err.o
	$(CC) $(CFLAGS) -g -o task1 task1.c err.o

clean:
	rm task1 *.o

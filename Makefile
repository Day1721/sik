CFLAGS=-Wall -Wextra -O2 -std=c11
CC=gcc

all: task1exe receiver sender

# BEGIN zadanie 1
task1exe: task1/task1.c err.o
	$(CC) $(CFLAGS) -o task1exe task1/task1.c err.o

task1exe-debug: task1.c err.o
	$(CC) $(CFLAGS) -g -o task1exe task1/task1.c err.o
# END zadanie 2

# BEGIN shared
err.o: shared/err.c shared/err.h
	$(CC) $(CFLAGS) -c -o err.o shared/err.c

func_types.o: shared/func_types.h shared/func_types.c
	$(CC) $(CFLAGS) -c -o func_types.o shared/func_types.c

addr.o: shared/addr.c shared/addr.h
	$(CC) $(CFLAGS) -c -o addr.o shared/addr.c

list.o: shared/list.h shared/list.c func_types.o
	$(CC) $(CFLAGS) -c -o list.o shared/list.c 

parse.o: shared/parse.h shared/parse.c
	$(CC) $(CFLAGS) -c -o parse.o shared/parse.c

shared: err.o func_types.o addr.o list.o parse.o
# END shared


# BEGIN sender
data.o: sender/data.c sender/data.h shared
	$(CC) $(CFLAGS) -c -o data.o sender/data.c

control.o: sender/control.c sender/control.h shared
	$(CC) $(CFLAGS) -c -o control.o sender/control.c

retransmit.o: sender/retransmit.c sender/retransmit.h shared
	$(CC) $(CFLAGS) -c -o retransmit.o sender/retransmit.c

transmit.o: sender/transmit.c sender/transmit.h shared
	$(CC) $(CFLAGS) -c -o transmit.o sender/transmit.c

sender.o: sender/sender.c sender/sender.h shared data.o control.o retransmit.o transmit.o
	$(CC) $(CFLAGS) -c -o sender.o sender/sender.c 

sender: sender/main.c sender.o
	$(CC) $(CFLAGS) -o sikradio-sender sender/main.c err.o parse.o sender.o addr.o list.o func_types.o data.o control.o retransmit.o transmit.o
# END sender


# BEGIN receiver
receiver: receiver/main.c err.o parse.o
	$(CC) $(CFLAGS) -o sikradio-receiver receiver/main.c err.o parse.o
# END receiver

clean:
	rm -f task1exe *.o sikradio-receiver sikradio-sender

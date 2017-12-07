CC=aarch64-linux-android-gcc
CFLAGS=-c -Wall -fPIE -pie -O2 -I ../include
LDFLAGS=-lm -pie -fPIE

all: rtt_client rtt_server rtt_server_x86
	    
rtt_server: rtt_server.o
	$(CC) $(LDFLAGS) rtt_server.o -o $@

rtt_server_x86: rtt_server_x86.o
	gcc $(LDFLAGS) rtt_server_x86.o -o $@

rtt_client: rtt_client.o
	$(CC) $(LDFLAGS) rtt_client.o -o $@

rtt_server_x86.o: rtt_server.c
	gcc $(CFLAGS) $< -o $@

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o rtt_server rtt_server_x86 rtt_client

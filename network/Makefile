CC=aarch64-linux-android-gcc
CFLAGS=-c -Wall -fPIE -pie -O2 -I ../include
LDFLAGS=-lm -pie -fPIE

all: rtt_client rtt_server rtt_server_x86 bw_client bw_server bw_server_x86 teardown_client teardown_server teardown_server_x86
	    
rtt_server: rtt_server.o
	$(CC) $(LDFLAGS) rtt_server.o -o $@

rtt_server_x86: rtt_server_x86.o
	gcc $(LDFLAGS) rtt_server_x86.o -o $@

rtt_client: rtt_client.o
	$(CC) $(LDFLAGS) rtt_client.o -o $@

bw_server: bw_server.o
	$(CC) $(LDFLAGS) bw_server.o -o $@

bw_server_x86: bw_server_x86.o
	gcc $(LDFLAGS) bw_server_x86.o -o $@

bw_client: bw_client.o
	$(CC) $(LDFLAGS) bw_client.o -o $@

teardown_server: teardown_server.o
	$(CC) $(LDFLAGS) teardown_server.o -o $@

teardown_server_x86: teardown_server_x86.o
	gcc $(LDFLAGS) teardown_server_x86.o -o $@

teardown_client: teardown_client.o
	$(CC) $(LDFLAGS) teardown_client.o -o $@

rtt_server_x86.o: rtt_server.c
	gcc $(CFLAGS) $< -o $@

bw_server_x86.o: bw_server.c
	gcc $(CFLAGS) $< -o $@

teardown_server_x86.o: teardown_server.c
	gcc $(CFLAGS) $< -o $@

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o rtt_server rtt_server_x86 rtt_client bw_server bw_client bw_server_x86 teardown_client teardown_server teardown_server_x86

CC=aarch64-linux-android-gcc
CFLAGS=-c -Wall -fPIE -pie -O2 -I ../include
LDFLAGS=-lm -pie -fPIE

all: file_cache read_time contention remote_read
	    
file_cache: file_cache.o
	$(CC) $(LDFLAGS) file_cache.o -o $@

read_time: read_time.o
	$(CC) $(LDFLAGS) read_time.o -o $@

remote_read: remote_read.o
	$(CC) $(LDFLAGS) remote_read.o -o $@

contention: contention.o
	$(CC) $(LDFLAGS) contention.o -o $@
.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o file_cache read_time contention remote_read

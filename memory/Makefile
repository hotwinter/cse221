CC=aarch64-linux-android-gcc
CFLAGS=-c -Wall -fPIE -pie -O2 -I ../include
LDFLAGS=-lm -pie -fPIE

all: memory
	    
memory: memory.o
	$(CC) $(LDFLAGS) memory.o -o $@

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o memory

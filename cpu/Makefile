CC=aarch64-linux-android-gcc
CFLAGS=-c -Wall -fPIE -pie -O0 -I ../include
LDFLAGS=-lm -pie -fPIE

all: cpu
	    
cpu: cpu.o
	$(CC) $(LDFLAGS) cpu.o -o $@

cpu.o: cpu.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o cpu

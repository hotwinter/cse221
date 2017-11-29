CC=aarch64-linux-android-gcc
CFLAGS=-c -Wall -fPIE -pie -O2
CPUFLAGS=-c -Wall -fPIE -pie -O0
LDFLAGS=-lm -pie -fPIE

all: cpu memory
	    
cpu: cpu.o
	$(CC) $(LDFLAGS) cpu.o -o $@

memory: memory.o
	$(CC) $(LDFLAGS) memory.o -o $@

cpu.o: cpu.c
	$(CC) $(CPUFLAGS) $< -o $@

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o cpu memory

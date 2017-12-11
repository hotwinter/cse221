CC=aarch64-linux-android-gcc
CFLAGS=-c -Wall -fPIE -pie -O2 -I include
LDFLAGS=-lm -pie -fPIE

all: su
	    
su: su.o
	$(CC) $(LDFLAGS) su.o -o $@

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o su

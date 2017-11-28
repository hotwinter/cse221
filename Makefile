CC=aarch64-linux-android-gcc
CFLAGS=-c -Wall -fPIE -pie -I ./include -O0
LDFLAGS=-lm -pie -fPIE
SOURCES=cpu.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cpu

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(OBJECTS) 
	    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

.o:
	    $(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o $(EXECUTABLE)

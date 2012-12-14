CFLAGS=-g -Wall
LDFLAGS=
CC=gcc
SOURCES=emulator.c packet.c trace.c
EXECUTABLES=emulator trace
.PHONY=all
.DEFAULT=all

all: $(EXECUTABLES)

$(EXECUTABLES): $(SOURCES)
	$(CC) $(CFLAGS) $@.c -o $@

clean:
	rm -rf *.o $(EXECUTABLES)

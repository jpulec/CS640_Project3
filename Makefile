CFLAGS=-g -Wall
LDFLAGS=
CC=gcc
SOURCES=sender.c requester.c
EXECUTABLES=sender requester
.PHONY=all
.DEFAULT=all

all: $(EXECUTABLES)

$(EXECUTABLES): $(SOURCES)
	$(CC) $(CFLAGS) $@.c -o $@

clean:
	rm -rf *.o $(EXECUTABLES)

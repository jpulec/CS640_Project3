CFLAGS=-g -Wall -std=gnu99 -ggdb3
LDFLAGS=
CC=gcc
SHARED_SOURCES=utilities.c packet.c
EXECUTABLES=emulator trace
.PHONY=all
.DEFAULT=all

all: $(EXECUTABLES)

$(EXECUTABLES): $(SHARED_SOURCES)
	@echo " [Compiling dependencies: " $^ "...]";
	@echo " [Building " $@ "... ]";
	$(CC) $(CFLAGS) $^ $@.c -o $@
	@echo " [Complete]";
clean:
	rm -rf *.o $(EXECUTABLES)

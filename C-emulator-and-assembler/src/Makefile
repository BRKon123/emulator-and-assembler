CC = gcc
CFLAGS = -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic
EFILES = $(wildcard ./Emulator/*.c)
AFILES = $(wildcard ./Assembler/*.c)
XFILES = $(wildcard ./Extension/*.c)

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: $(AFILES)
	$(CC) $(CFLAGS) $(AFILES) -o assemble


assembleTest: $(AFILES)
	$(CC) -g $(AFILES) -o assemble

emulate: $(EFILES)
	$(CC) $(CFLAGS) $(EFILES) -o emulate -lm

extensionTest: $(XFILES)
	$(CC) -g $(XFILES) -o noticeBoard -l sqlite3 -lsodium

extension: $(XFILES)
	$(CC) $(CFLAGS) $(XFILES) -o noticeBoard -l sqlite3 -lsodium

clean:
	$(RM) *.o assemble emulate extension

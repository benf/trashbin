CFLAGS=-Wall -pedantic -std=c99 `pkg-config --cflags gnt` -D USE_CURSES
LDFLAGS=`pkg-config --libs gnt`
CC=gcc

all: readkey

%: %.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $<

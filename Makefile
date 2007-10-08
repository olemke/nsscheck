CC = gcc
TARGETS = nsscheck
CPPFLAGS = -Wall -pedantic -D_GNU_SOURCE -std=c99
CFLAGS = -O2

all: nsscheck

nsscheck: nssswath.h nssswath.c nsscheck.c

clean:
	rm -f $(wildcard *.o) $(TARGETS)


CC = gcc
TARGETS = nsscheck
CPPFLAGS = -Wall -pedantic -std=c99

all: nsscheck

nsscheck: nssswath.h nssswath.c nsscheck.c

clean:
	rm -f $(wildcard *.o) $(TARGETS)


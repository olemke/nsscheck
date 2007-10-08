CC = gcc
TARGETS = nsscheck
CPPFLAGS = -Wall -pedantic

all: nsscheck

nsscheck: nssswath.h nssswath.c nsscheck.c

clean:
	rm -f $(wildcard *.o) $(TARGETS)


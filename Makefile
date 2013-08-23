#CC = gcc
TARGETS = nsscheck
CPPFLAGS = -Wall -pedantic -D_GNU_SOURCE -std=c99
CFLAGS = -O2

all: nsscheck

nsscheck: \
	nsscheck.o \
	nssdups.o \
	nssgaps.o \
	nssswath.o

clean:
	rm -f $(wildcard *.o) $(TARGETS)


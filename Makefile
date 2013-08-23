#CC = gcc
TARGETS = nsscheck
CFLAGS = -Wall -pedantic -std=c99 -O2

all: nsscheck

nsscheck: \
	nsscheck.o \
	nssdups.o \
	nssgaps.o \
	nssswath.o

clean:
	rm -f $(wildcard *.o) $(TARGETS)


#CC = gcc
TARGETS = nsscheck
CFLAGS = -Wall -pedantic -std=c11 -D_POSIX_C_SOURCE=200809L -O2

all: nsscheck

nsscheck: \
	nsscheck.o \
	nssdups.o \
	nssgaps.o \
	nssswath.o

clean:
	rm -f $(wildcard *.o) $(TARGETS)


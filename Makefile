CC = gcc
TARGETS = nsscheck
CPPFLAGS = -Wall -pedantic -D_GNU_SOURCE -std=c99
CFLAGS = -O2

all: nsscheck

nsscheck: \
	nsscheck.c \
	nssdups.c \
	nssdups.h \
	nssgaps.c \
	nssgaps.h \
	nssswath.c \
	nssswath.h

clean:
	rm -f $(wildcard *.o) $(TARGETS)


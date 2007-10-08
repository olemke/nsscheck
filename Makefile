CC = gcc
TARGETS = nsscheck
CPPFLAGS = -Wall -pedantic -std=c99

all: $(TARGETS)

clean:
	rm -f $(wildcard *.o) $(TARGETS)


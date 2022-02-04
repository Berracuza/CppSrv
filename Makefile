CXXFLAGS=-std=c++2a -O2 -Wall -Wextra -pthread
LDLIBS=-lrt
LDFLAGS=-pthread
LINK.o=$(LINK.cc)

.PHONY: all clean

all: simpleCWebserver

clean:
	rm -f *.o
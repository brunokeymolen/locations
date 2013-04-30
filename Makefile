UNAME := $(shell uname -s)
CXX=g++
CC=gcc
OPTFLAGS=-g3 -ggdb -O0
CXXFLAGS=-Wall -I. -I../../src/ -I/usr/local/include $(OPTFLAGS) 
CFLAGS=-Wall -fPIC $(OPTFLAGS)
LDFLAGS= -L/usr/local/lib -lpthread  $(OPTFLAGS)


SRC = 	main.o \
		ipdb.o \
		/usr/local/lib/libfcgi.a
	

all: locations 

locations: $(SRC) $(MODULES)
	$(CXX) $(LDFLAGS) $(MODULES) $(SRC) -o locations

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f *.o ../src/*.o  locations

PREFIX ?= /usr

install: all
	install -d $(PREFIX)/bin
	install locations  $(PREFIX)/bin

.PHONY: clean all locations install

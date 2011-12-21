CC = gcc
CXX = g++
CFLAGS = -O2 -std=gnu99 -Wall -Wno-unused-result 
CXXFLAGS = -O2 -Wall -Wno-unused-result

all: bfi bfcc

bfi: bfi.c bfi.h

bfcc: bfcc.c bfcc.h bfop.o list.o error_handling.o

bfop.o: bfop.c bfop.h error_handling.o

error_handling.o: error_handling.c error_handling.h

tuple.o: tuple.h tuple.c

list.o: list.c list.h error_handling.o

unittest: unittest.c unittest.h list.o tuple.o bfop.o error_handling.o

clean :
	rm bfi bfcc *.o *.h.gch 

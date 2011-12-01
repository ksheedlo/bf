CC = gcc
CXX = g++
CFLAGS = -O2 -std=gnu99 -Wall -Wno-unused-result 
CXXFLAGS = -O2 -Wall -Wno-unused-result

all: bfi lexer.o bfc

bfi: bfi.c bfi.h

bfcc: bfcc.c bfcc.h

clean :
	rm bfi bfcc

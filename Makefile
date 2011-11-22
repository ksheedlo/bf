CC = gcc
CXX = g++
CFLAGS = -O2 -std=gnu99 -Wall -Wno-unused-result
CXXFLAGS = -O2 -Wall -Wno-unused-result

all: bfi lexer.o bfc

bfi: bfi.c bfi.h

lexer.o: lexer.c lexer.h

bfc: bfc.cc bfc.h lexer.o 
	$(CXX) $(CXXFLAGS) `llvm-config --cppflags --ldflags --libs core native` -o bfc bfc.cc bfc.h 

clean :
	rm bfi lexer.o lexer.h.gch bfc

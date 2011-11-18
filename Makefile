CC = gcc
CFLAGS = -g -std=gnu99 -Wall -Wno-unused-result

bfi: bfi.c bfi.h

clean :
	rm bfi

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#define MEM_SIZE 30000
#define MAX_PROGBUF 1048576
#define FILE_INPUT 1

uint8_t *bf_interpret(uint8_t *mem);

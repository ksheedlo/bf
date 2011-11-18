#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>

#define MEM_SIZE 30000
#define INPUT_USR_SPEC 1

uint8_t *bf_interpret(uint8_t *mem, FILE *stream);


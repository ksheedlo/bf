//  __       _    __  _____   _     _        ____     ____     ____
//  \ \     | |  / / |  ___| | \   | |      / __ \   / __ \   / __ \
//   \ \    | | / /  | |__   |  \  | |     | |  | | | |  | | | |  | |
//    \ \   | |/ /   |  __|  |   \ | |     | |  | | | |  | | | |  | |
//    / /   | |\ \   | |     | |\ \| |     | |  | | | |  | | | |  | |
//   / /    | | \ \  | |___  | | \   |     | |__| | | |__| | | |__| |
//  /_/     |_|  \_\ |_____| |_|  \__|      \____/   \____/   \____/
//                                      /

/* (there's no way that can be right) */

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<unistd.h>

#define MEM_SIZE 30000
#define MAX_PROGBUF 1048576
#define FILE_INPUT 1

/*Error definitions. */
#define ERR_BOUNDS 1
#define ERR_MEM 2

typedef struct {
    char *pc;
    uint8_t *base;
    size_t mem_size;
} bfstate_t;

uint8_t *bf_interpret(uint8_t *mem, bfstate_t *state);

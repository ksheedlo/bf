/*Ken Sheedlo
 * Brainfuck Compiler
 * Brainfuck Operation definitions and headers.*/

#include<stdlib.h>
#include<stdint.h>
#include "error_handling.h"

/* Command constants */
#define INC 0
#define INCV 1
#define DEC 2
#define DECV 3
#define ADD 4
#define ADDV 5
#define SUB 6
#define SUBV 7
#define PUT 8
#define GET 9
#define LABEL 10
#define JNZ 11
#define JZ 12
#define ZERO 13

/* Command type constants */
#define T_PTR 0
#define T_ARITH 1
#define T_IO 2
#define T_BRANCH 3
#define T_ZERO 4

/* Application-specific data structures. */
typedef struct {
    int32_t arg;    /* May or may not be req. depending on opcode */
    int32_t opcode;
} bfop_t;

bfop_t *bfop_new(int32_t opcode, int32_t arg);

int32_t bfop_type(int32_t opcode);

void bfop_print(FILE *output, const bfop_t *op);


/* Ken Sheedlo
 * Brainfuck Compiler
 * Brainfuck operation implementation */

#include "bfop.h"

bfop_t *bfop_new(int32_t opcode, int32_t arg){
    //Creates a new bfop_t object on the heap.
    bfop_t *op = malloc(sizeof(bfop_t));
    if(op == NULL){
        CriticalError("Failed to allocate memory");
    }

    op->opcode = opcode;
    op->arg = arg;

    return op;
}

int32_t bfop_type(int32_t opcode){
    switch(opcode){
        case INC:
        case INCV:
        case DEC:
        case DECV:
            return T_PTR;
        case ADD:
        case ADDV:
        case SUB:
        case SUBV:
            return T_ARITH;
        case PUT:
        case GET:
            return T_IO;
        case LABEL:
        case JNZ:
        case JZ:
            return T_BRANCH;
        case ZERO:
            return T_ZERO;
    }
    return -1; /*Something bad happened */
}

void bfop_print(FILE *output, const bfop_t *op){
    char *code = "undefined";
    switch(op->opcode){
        case INC:code = "INC";break;
        case INCV:code = "INCV";break;
        case DEC:code = "DEC";break;
        case DECV:code = "DECV";break;
        case ADD:code = "ADD";break;
        case ADDV:code = "ADDV";break;
        case SUB:code = "SUB";break;
        case SUBV:code = "SUBV";break;
        case PUT:code = "PUT";break;
        case GET:code = "GET";break;
        case LABEL:code = "LABEL";break;
        case JNZ:code = "JNZ";break;
        case JZ:code = "JZ";break;
        case ZERO:code = "ZERO";break;

    }
    fprintf(output, "{ opcode: %s, arg: %d }", code, op->arg);
}

void generic_bfop_print(FILE *output, const void *op){
    bfop_print(output, (const bfop_t *)op);
}


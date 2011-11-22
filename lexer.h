/* Ken Sheedlo
 * Brainfuck Lexer - includes and definitions. */

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#define TOKEN_EOF -1

int32_t gettok(FILE *stream);


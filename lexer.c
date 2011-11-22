/*Ken Sheedlo
 * Brainfuck Lexer */

#include "lexer.h"

int32_t gettok(FILE *stream){
    char c;
    while((c = fgetc(stream)) != EOF){
        switch(c){
            case '+':
            case '-':
            case '<':
            case '>':
            case '[':
            case ']':
            case '.':
            case ',':
                return c;
                break;
        }
    }
    return TOKEN_EOF;
}


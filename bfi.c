/* Ken Sheedlo
 * Brainfuck Interpreter */

#include "bfi.h"

uint8_t *mem_floor;
uint8_t *mem_ceil;

uint8_t  *bf_interpret(uint8_t *mem, FILE *stream){
    char input;
    int64_t pc;
    int32_t level;
    while((input = (char)fgetc(stream)) != EOF){
        switch(input){
            case '>': ++mem;break;
            case '<': --mem;break;
            case '+': ++*mem;break;
            case '-': --*mem;break;
            case '.': fputc(*mem, stdout);break;
            case ',': *mem = fgetc(stdin);break;
            case '[':
                pc = ftell(stream);
                while(*mem){
                    mem = bf_interpret(mem, stream);
                    fseek(stream, pc, SEEK_SET);
                }
                level = 1;
                do{
                    input = (char)fgetc(stream);
                    if(input == '['){
                        level++;
                    }else if(input == ']'){
                        level--;
                    }
                }while(input != EOF && level);
                break;
            case ']':
                return mem;
                break;
        }
    }
    return mem;
}

int main(int argc, char **argv){
    char c;
    FILE *input = stdin;
    int32_t st_flags = 0;
    while((c = getopt(argc, argv, "Vh")) != -1){
        switch(c){
            case 'V':
                printf("bfi 1.0 - a tiny brainfuck interpreter\n");
                printf("Author: Ken Sheedlo\n");
                return 0;
            case 'h':
                printf("Usage: ./bfi FILE to read from FILE\n");
                printf("       ./bfi      to read from stdin\n");
                return 0;
        }
    }

    if(optind<argc){
        input = fopen(argv[optind], "r");
        if(!input){
            fprintf(stderr, "Could not open file: %s\n", argv[optind]);
            return 1;
        }
        st_flags |= INPUT_USR_SPEC;
    }

    uint8_t *membuf = calloc(MEM_SIZE, sizeof(uint8_t));
    if(!membuf){
        fprintf(stderr, "Memory allocation failure.\n");
        return 1;
    }

    bf_interpret(membuf, input);

    free(membuf);
    return 0;
}


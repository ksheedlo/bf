/* Ken Sheedlo
 * Brainfuck Interpreter */

#include "bfi.h"

char *progc;

uint8_t *bf_interpret(uint8_t *mem){
    char input;
    int32_t level;

    while((input = *progc++) != EOF){
        switch(input){
            case '>': ++mem;break;
            case '<': --mem;break;
            case '+': ++*mem;break;
            case '-': --*mem;break;
            case '.': fputc(*mem, stdout);break;
            case ',': *mem = fgetc(stdin);break;
            case '[':
                if(*mem){
                    char *save_ptr = progc;
                    do{
                        progc = save_ptr;
                        mem = bf_interpret(mem);
                    }while(*mem);
                }else{
                    level = 1;
                    do{
                        input = *progc++;
                        if(input == '['){
                            level++;
                        }else if(input == ']'){
                            level--;
                        }
                    }while(input != EOF && level);
                }
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
    char *program;
    int32_t st_flags = 0;
    while((c = getopt(argc, argv, "Vh")) != -1){
        switch(c){
            case 'V':
                printf("bfi 1.0 - a tiny brainfuck interpreter\n");
                printf("Author: Ken Sheedlo\n");
                return 0;
            case 'h':
                printf("Usage: ./bfi FILE to read from FILE\n");
                return 0;
        }
    }

    if(optind<argc){
        input = fopen(argv[optind], "r");
        if(!input){
            fprintf(stderr, "Could not open file: %s\n", argv[optind]);
            return 1;
        }
        //XXX
        fseek(input, 0L, SEEK_END);
        int32_t f_len = ftell(input) + 1;
        fseek(input, 0L, SEEK_SET);

        program = malloc(f_len*sizeof(char));
        if(!program){
            fprintf(stderr, "%s\n", "Memory allocation failure");
            return 1;
        }
        fread(program, sizeof(char), f_len, input);
        program[f_len-1] = EOF;
        fclose(input);

        st_flags |= FILE_INPUT;
    }else{
        int32_t program_size = MAX_PROGBUF;
        int i = 0;

        program = malloc(program_size*sizeof(char));
        if(!program){
            fprintf(stderr, "%s\n", "Memory allocation failure");
            return 1;
        }

        do{
            do{
                c = getc(stdin);
                program[i++] = c;
            }while(i < program_size && c != EOF);

            if(c != EOF){
                //Reallocate the program buffer
                program_size *= 2;
                char *newprog = malloc(program_size * sizeof(char));
                if(!newprog){
                    fprintf(stderr, "%s\n", "Memory allocation failure");
                    return 1;
                }
                memcpy(newprog, program, (program_size >> 1));
                free(program);
                program = newprog;

            }
        }while(c != EOF);
    }

    uint8_t *membuf = calloc(MEM_SIZE, sizeof(uint8_t));
    if(!membuf){
        fprintf(stderr, "Memory allocation failure.\n");
        return 1;
    }
    progc = program;

    bf_interpret(membuf);

    free(membuf);
    free(program);
    return 0;
}


/* Ken Sheedlo
 * Brainfuck Interpreter */

#include "bfi.h"

uint8_t *bf_interpret(uint8_t *mem, bfstate_t *state){
    char input;
    int32_t level;
    char *program = state->pc;

    while((input = *program++) != EOF){
        switch(input){
            case '>':
                if(++mem >= (state->base + state->mem_size)){
                    //Allocate moar memory plz
                    int32_t new_size = 2*state->mem_size;
                    uint8_t *new_base = calloc(new_size, sizeof(uint8_t));
                    if(!new_base){
                        fprintf(stderr, "%s: %s\n", "Program failed due to errors", "ERR_MEM");
                        exit(ERR_MEM);
                    }
                    memcpy(new_base, state->base, state->mem_size);
                    free(state->base);
                    mem = new_base + state->mem_size;
                    state->base = new_base;
                    state->mem_size = new_size;
                }
                break;
            case '<': 
                if(--mem < state->base){
                    fprintf(stderr, "%s: %s\n", "Program failed due to errors", "ERR_BOUNDS");
                    exit(ERR_BOUNDS);
                }
                break;
            case '+': ++*mem;break;
            case '-': --*mem;break;
            case '.': fputc(*mem, stdout);break;
            case ',': *mem = fgetc(stdin);break;
            case '[':
                if(*mem){
                    do{
                        state->pc = program;
                        mem = bf_interpret(mem, state);
                    }while(*mem);
                    program = state->pc;
                }else{
                    level = 1;
                    do{
                        input = *program++;
                        if(input == '['){
                            level++;
                        }else if(input == ']'){
                            level--;
                        }
                    }while(input != EOF && level);
                }
                break;
            case ']':
                state->pc = program;
                return mem;
                break;

        }
    }
    state->pc = program;
    return mem;
}

int main(int argc, char **argv){
    char c;
    FILE *input = stdin;
    char *program;
    int32_t st_flags = 0;
    int32_t verbose = 0;
    struct timeval t1, t2;

    while((c = getopt(argc, argv, "Vvh")) != -1){
        switch(c){
            case 'V':
                printf("bfi 1.0 - a tiny brainfuck interpreter\n");
                printf("Author: Ken Sheedlo\n");
                return 0;
            case 'v':
                verbose = 1;
                break;
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

    bfstate_t state;
    state.pc = program;
    state.base = membuf;
    state.mem_size = MEM_SIZE;

    gettimeofday(&t1, NULL);
    bf_interpret(membuf, &state);
    gettimeofday(&t2, NULL);

    if(verbose){
        int64_t elapsed = (t2.tv_sec - t1.tv_sec) * 1000000;
        elapsed += (t2.tv_usec - t1.tv_usec);
        fprintf(stderr, "Time elapsed: %ld us\n", elapsed);
    }

    free(program);
    free(state.base);
    return 0;
}


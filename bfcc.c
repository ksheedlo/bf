/* Ken Sheedlo
 * bfcc, a tiny optimizing brainfuck compiler in C */

#include "bfcc.h"

list_t *bfcc_parse(char *program, list_t *parse_lst){
    /*Produces the initial parse list for the program. Does not allocate
     * memory for a new list because it may be used recursively. */
    char input;
    intptr_t label_ct = 0;
    list_t loop_stack;
    list_init(&loop_stack);

    while((input = *program++) != EOF){
        switch(input){
            case '>':
                list_addlast(parse_lst, bfop_new(INC, 0));
                break;
            case '<':
                list_addlast(parse_lst, bfop_new(DEC, 0));
                break;
            case '+':
                list_addlast(parse_lst, bfop_new(ADD, 0));
                break;
            case '-':
                list_addlast(parse_lst, bfop_new(SUB, 0));
                break;
            case '.':
                list_addlast(parse_lst, bfop_new(PUT, 0));
                break;
            case ',':
                list_addlast(parse_lst, bfop_new(GET, 0));
                break;
            case '[':
                //First add the jz and the label for the top of the loop
                list_addlast(parse_lst, bfop_new(JZ, label_ct + 1));
                list_addlast(parse_lst, bfop_new(LABEL, label_ct));
                
                /*Now push the label numbers onto the stack. DO NOT use the
                 * label counts as actual pointers */
                list_addfirst(&loop_stack, (void *)(label_ct + 1));
                list_addfirst(&loop_stack, (void *)label_ct);

                label_ct += 2;
                break;
            case ']':
                //Try to pop from the stack. If it's empty, you dun goofed
                if(loop_stack.length <= 1){
                    fprintf(stderr, "Error: mismatched []\n");
                    exit(1);
                }

                intptr_t top_label = (intptr_t)list_remove(loop_stack.head->next);
                intptr_t bot_label = (intptr_t)list_remove(loop_stack.head->next);

                list_addlast(parse_lst, bfop_new(JNZ, top_label));
                list_addlast(parse_lst, bfop_new(LABEL, bot_label));
                break;
        }
    }

    return parse_lst;
}

void bfcc_codegen(FILE *output, list_t *parse_lst){
    //Generate portable brainfuck bytecode to output.
    node_t *node = parse_lst->head->next;
    while(node != parse_lst->head){
        bfop_t *op = node->data;
        switch(op->opcode){
            case INC:
                fprintf(output, "%s\n", "inc");
                break;
            case INCV:
                fprintf(output, "%s %d\n", "incv", op->arg);
                break;
            case DEC:
                fprintf(output, "%s\n", "dec");
                break;
            case DECV:
                fprintf(output, "%s %d\n", "decv", op->arg);
                break;
            case ADD:
                fprintf(output, "%s\n", "add");
                break;
            case ADDV:
                fprintf(output, "%s %d\n", "addv", op->arg);
                break;
            case SUB:
                fprintf(output, "%s\n", "sub");
                break;
            case SUBV:
                fprintf(output, "%s %d\n", "subv", op->arg);
                break;
            case PUT:
                fprintf(output, "%s\n", "put");
                break;
            case GET:
                fprintf(output, "%s\n", "get");
                break;
            case LABEL:
                fprintf(output, "L%d:\n", op->arg);
                break;
            case JNZ:
                fprintf(output, "%s L%d\n", "jnz", op->arg);
                break;
            case JZ:
                fprintf(output, "%s L%d\n", "jz", op->arg);
                break;
            case ZERO:
                fprintf(output, "%s\n", "zero");
                break;
        }
        node = node->next;
    }
}

void bfcc_gen32(FILE *output, list_t *parse_lst){
    //XXX

}

void bfcc_gen64(FILE *output, list_t *parse_lst){
    //XXX

}

list_t *bfopt_combine_arith(list_t *parse_lst){
    //Combine repeated arithmetic and ptr ops.
    if(parse_lst->length < 2){
        return parse_lst;
    }
    int32_t curr_type, next_type;
    bfop_t *curr_op, *next_op;

    node_t *node = parse_lst->head->next;
    next_op = (bfop_t *)node->data;
    next_type = bfop_type(next_op->opcode);

    while(node != parse_lst->head && node->next != parse_lst->head){
        curr_op = (bfop_t *)node->data;
        next_op = (bfop_t *)node->next->data;

        curr_type = bfop_type(curr_op->opcode);
        next_type = bfop_type(next_op->opcode);

        if(curr_type != next_type || (curr_type != T_PTR && curr_type != T_ARITH)){
            node = node->next;
            continue;
        }

        int32_t op1 = curr_op->opcode;
        int32_t op2 = next_op->opcode;
        bfop_t *r1 = NULL, *r2 = NULL;
        switch(op1){
            case INC:
                switch(op2){
                    case INC:
                        curr_op->opcode = INCV;
                        curr_op->arg = 2;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                    case INCV:
                        curr_op->opcode = INCV;
                        curr_op->arg = 1 + next_op->arg;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                    case DEC:
                        node = node->next->next;
                        r1 = (bfop_t *)list_remove(node->next);
                        r2 = (bfop_t *)list_remove(node);
                        break;
                    case DECV:
                        curr_op->opcode = DECV;
                        curr_op->arg = next_op->arg - 1;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                }
                break;
            case DEC:
                switch(op2){
                     case INC:
                        node = node->next->next;
                        r1 = (bfop_t *)list_remove(node->next);
                        r2 = (bfop_t *)list_remove(node);
                        break;
                    case INCV:
                        curr_op->opcode = INCV;
                        curr_op->arg = next_op->arg - 1;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                    case DEC:
                        curr_op->opcode = DECV;
                        curr_op->arg = 2;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                    case DECV:
                        curr_op->opcode = DECV;
                        curr_op->arg = next_op->arg + 1;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                }
                break;
            case INCV:
                switch(op2){
                    case INC:
                        curr_op->arg = curr_op->arg + 1;
                        break;
                    case INCV:
                        curr_op->arg = curr_op->arg + next_op->arg;
                        break;
                    case DEC:
                        curr_op->arg = curr_op->arg - 1;
                        break;
                    case DECV:
                        curr_op->arg = curr_op->arg - next_op->arg;
                        break;
                }
                r1 = (bfop_t *)list_remove(node->next);
                break;
            case DECV:
                switch(op2){
                    case INC:
                        curr_op->arg = curr_op->arg - 1;
                        break;
                    case INCV:
                        curr_op->arg = curr_op->arg - next_op->arg;
                        break;
                    case DEC:
                        curr_op->arg = curr_op->arg + 1;
                        break;
                    case DECV:
                        curr_op->arg = curr_op->arg + next_op->arg;
                        break;
                }
                r1 = (bfop_t *)list_remove(node->next);
                break;
            case ADD:
                switch(op2){
                    case ADD:
                        curr_op->opcode = ADDV;
                        curr_op->arg = 2;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                    case ADDV:
                        curr_op->opcode = ADDV;
                        curr_op->arg = 1 + next_op->arg;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                    case SUB:
                        node = node->next->next;
                        r1 = (bfop_t *)list_remove(node->next);
                        r2 = (bfop_t *)list_remove(node);
                        break;
                    case SUBV:
                        curr_op->opcode = SUBV;
                        curr_op->arg = next_op->arg - 1;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                }
                break;
            case ADDV:
                switch(op2){
                    case ADD:
                        curr_op->arg = 1 + curr_op->arg;
                        break;
                    case ADDV:
                        curr_op->arg = next_op->arg + curr_op->arg;
                        break;
                    case SUB:
                        curr_op->arg = curr_op->arg - 1;
                        break;
                    case SUBV:
                        curr_op->arg = curr_op->arg - next_op->arg;
                        break;

                }
                r1 = (bfop_t *)list_remove(node->next);
                break;
            case SUB:
                switch(op2){
                    case ADD:
                        node = node->next->next;
                        r1 = (bfop_t *)list_remove(node->next);
                        r2 = (bfop_t *)list_remove(node);
                        break;
                    case ADDV:
                        curr_op->opcode = ADDV;
                        curr_op->arg = next_op->arg - 1;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                    case SUB:
                        curr_op->opcode = SUBV;
                        curr_op->arg = 2;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                    case SUBV:
                        curr_op->opcode = SUBV;
                        curr_op->arg = 1 + next_op->arg;
                        r1 = (bfop_t *)list_remove(node->next);
                        break;
                }
                break;
            case SUBV:
                switch(op2){
                    case ADD:
                        curr_op->arg = curr_op->arg - 1;
                        break;
                    case ADDV:
                        curr_op->arg = curr_op->arg - next_op->arg;
                        break;
                    case SUB:
                        curr_op->arg = 1 + curr_op->arg;
                        break;
                    case SUBV:
                        curr_op->arg = next_op->arg + curr_op->arg;
                        break;
                }
                r1 = (bfop_t *)list_remove(node->next);
                break;
        }
        /* In case there are bfop_t's that we removed, free them */
        if(r1 != NULL){
            free(r1);
        }
        if(r2 != NULL){
            free(r2);
        }
    }

    /*Need to do a quick sanity check to finish up here */
    node = parse_lst->head->next;
    while(node != parse_lst->head){
        curr_op = (bfop_t *)node->data;
        switch(curr_op->opcode){
            case ADDV:
                if(curr_op->arg < 0){
                    curr_op->opcode = curr_op->arg == -1 ? SUB : SUBV;
                    curr_op->arg = -(curr_op->arg);
                }else if(curr_op->arg == 0){
                    node = node->next;
                    free(list_remove(node));
                    break;
                }else if(curr_op->arg == 1){
                    curr_op->opcode = ADD;
                }
                node = node->next;
                break;
            case SUBV:
                if(curr_op->arg < 0){
                    curr_op->opcode = curr_op->arg == -1 ? ADD : ADDV;
                    curr_op->arg = -(curr_op->arg);
                }else if(curr_op->arg == 0){
                    node = node->next;
                    free(list_remove(node));
                    break;
                }else if(curr_op->arg == 1){
                    curr_op->opcode = SUB;
                }
                node = node->next;
                break;
            case INCV:
                if(curr_op->arg < 0){
                    curr_op->opcode = curr_op->arg == -1 ? DEC : DECV;
                    curr_op->arg = -(curr_op->arg);
                }else if(curr_op->arg == 0){
                    node = node->next;
                    free(list_remove(node));
                    break;
                }else if(curr_op->arg == 1){
                    curr_op->opcode = INC;
                }
                node = node->next;
                break;
            case DECV:
                if(curr_op->arg < 0){
                    curr_op->opcode = curr_op->arg == -1 ? INC : INCV;
                    curr_op->arg = -(curr_op->arg);
                }else if(curr_op->arg == 0){
                    node = node->next;
                    free(list_remove(node));
                    break;
                }else if(curr_op->arg == 1){
                    curr_op->opcode = DEC;
                }
                node = node->next;
                break;

            default:
                node = node->next;
                break;
        }
    }

    return parse_lst;
}

list_t *bfopt_make_zeros(list_t *parse_lst){
    /*Searches the parse list for the following series: "[-]" and replaces with
     * *ptr = 0. */
    if(parse_lst->length < 5){
        return parse_lst;
    }

    node_t *node = parse_lst->head->next;
    while(node->next->next->next->next != parse_lst->head){
        bfop_t *curr_op = (bfop_t *)node->data;
        bfop_t *test_op = curr_op;
        int32_t top_label, bot_label;

        if(test_op->opcode != JZ){
            node = node->next;
            continue;
        }else{
            bot_label = test_op->arg;
            test_op = (bfop_t *)node->next->data;
        }

        if(test_op->opcode != LABEL){
            node = node->next;
            continue;
        }else{
            top_label = test_op->arg;
            test_op = (bfop_t *)node->next->next->data;
        }

        if(test_op->opcode != SUB){
            node = node->next;
            continue;
        }else{
            test_op = (bfop_t *)node->next->next->next->data;
        }

        if(test_op->opcode != JNZ || test_op->arg != top_label){
            node = node->next;
            continue;
        }else{
            test_op = (bfop_t *)node->next->next->next->next->data;
        }

        if(test_op->opcode != LABEL || test_op->arg != bot_label){
            node = node->next;
            continue;
        }

        /* If we get here we know for sure this is a [-] */ 
        curr_op->opcode = ZERO;
        curr_op->arg = 0;

        free(list_remove(node->next));
        free(list_remove(node->next));
        free(list_remove(node->next));
        free(list_remove(node->next));

        node_t *head = parse_lst->head;
        node = node->next;
        node_t *test = node;
        int32_t st = 0;

        for(int i = 0; i<4; i++){
            if(test == head){
                st = 1;
                break;
            }
            test = test->next;
        }
        if(st)
            break;
    }

    return parse_lst;
}

int main(int argc, char **argv){
    if(argc < 2){
        CriticalError("Must provide a .b source file to compile");
    }

    FILE *input = fopen(argv[1], "r");
    if(!input){
        CriticalError("Could not open file");
    }

    fseek(input, 0L, SEEK_END);
    int32_t f_len = ftell(input) + 1;
    fseek(input, 0L, SEEK_SET);

    char *program = malloc(f_len * sizeof(char));
    if(!program){
        CriticalError("Failed to allocate memory");
    }

    fread(program, sizeof(char), f_len, input);
    program[f_len-1] = EOF;
    fclose(input);

    list_t list;
    list_init(&list);

    bfcc_parse(program, &list);
    bfopt_combine_arith(&list);
    bfopt_make_zeros(&list);
    
    FILE *output = fopen("a.bc", "w");
    if(!output){
        CriticalError("Could not open file");
    }

    bfcc_codegen(output, &list);
    fclose(output);
    list_clear(&list, 1);

    return 0;
}


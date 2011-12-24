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

void bfcc_codegen(FILE *output, list_t *parse_lst, char *filename){
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

int32_t gen32_find_bestp(int32_t curr, int32_t diff, int32_t *ptrs, int32_t *refresh){
    /* Given a set of pointer locations and valid flags, return the best 
     * value to put the next pointer. Hash into a location that's already
     * loaded if possible, else take the first invalid slot. If that fails,
     * overwrite the next location.
     *
     * Assumptions: Arrays are of length 3.
     *              Pointer locations are relative to the zeroth entry.
     * Sets the ptrs and refresh values to the desired result, and returns
     * the new curr pointer. */
    
    /* Best possibility: We already have the pointer we needed */
    curr = curr % 3;
    int32_t srch = ptrs[curr] + diff;
    for(int32_t i = 0; i < 3; i++){
        if(ptrs[i] == srch && (!refresh[i])){
            return i;
        }
    }

    /* If we get here, we're going to have to do a memory access. Try to do it
     * without overwriting someone else's registers */
    for(int32_t i = 0, c = ((curr + 1) % 3); i < 3; i++, c = ((c + 1) % 3)){
        if(refresh[c]){
            return c;
        }
    }

    /* This is the sucky case */
    int32_t ret = (curr + 1) % 3;
    refresh[ret] = 1;
    return ret;
}

void bfcc_gen32(FILE *output, list_t *parse_lst, char *filename){
    //Generate 32-bit x86 code.
    fprintf(output, "\t .file\t\"%s\"\n", filename);
    fprintf(output, "\t .text\n.globl bf_prog\n\t .type\t bf_prog, @function\n");
    fprintf(output, "bf_prog:\n");
    fprintf(output, "\t pushl\t%%ebp\n");
    fprintf(output, "\t movl\t%%esp, %%ebp\n");
    fprintf(output, "\t pushl\t%%ebx\n");
    fprintf(output, "\t pushl\t%%esi\n");
    fprintf(output, "\t pushl\t%%edi\n");
    fprintf(output, "\t movl\t8(%%ebp), %%ebx\n");
    fprintf(output, "\t subl\t$8, %%esp\n");
    fprintf(output, "\t movl\tstdout, %%eax\n");
    fprintf(output, "\t movl\t%%eax, 4(%%esp)\n");

    node_t *node = parse_lst->head->next;

    /* Try to track 3 different ptr/value combinations with 6 available
     * registers (ebx/eax, esi/ecx, edi/edx) */
    int32_t refresh_vals = {1, 1, 1};
    int32_t refresh_ptrs = {1, 1, 1};

    int32_t ptr_locs[] = {0, 0, 0};
    int32_t curr_ptr = 2;  /* uninitialized */

    /* We use callee-save registers for the pointers because it's harder to 
     * get them back if we lose them. The values are just a memory access */
    char *ptr_regs[] = {"ebx", "esi", "edi"};
    char *val_regs[] = {"eax", "ecx", "edx"};
    char *val_bregs[] = {"al", "cl", "dl"};

    while(node != parse_lst->head){
        bfop_t *op = node->data;
        switch(op->opcode){
            case INC:
                int32_t old_ptr = curr_ptr;
                curr_ptr = gen32_find_bestp(curr_ptr, 1, ptr_locs, refresh_ptrs);
                if(refresh_ptrs[curr_ptr]){
                    fprintf(output, "\t leal\t1(%%%s), %%%s\n", 
                        ptr_regs[old_ptr], ptr_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 1;
                }
                break;
            case INCV:
                int32_t old_ptr = curr_ptr;
                curr_ptr = gen32_find_bestp(curr_ptr, op->arg, ptr_locs, refresh_ptrs);
                if(refresh_ptrs[curr_ptr]){
                    fprintf(output, "\t leal\t%d(%%%s), %%%s\n", 
                        op->arg, ptr_regs[old_ptr], ptr_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 1;
                }
                break;
            case DEC:
                int32_t old_ptr = curr_ptr;
                curr_ptr = gen32_find_bestp(curr_ptr, -1, ptr_locs, refresh_ptrs);
                if(refresh_ptrs[curr_ptr]){
                    fprintf(output, "\t leal\t-1(%%%s), %%%s\n", 
                        ptr_regs[old_ptr], ptr_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 1;
                }
                break;
            case DECV:
                int32_t old_ptr = curr_ptr;
                curr_ptr = gen32_find_bestp(curr_ptr, -(op->arg), 
                                ptr_locs, refresh_ptrs);
                if(refresh_ptrs[curr_ptr]){
                    fprintf(output, "\t leal\t%d(%%%s), %%%s\n", 
                        -(op->arg), ptr_regs[old_ptr], ptr_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 1;
                }
                break;
            case ADD:
                if(refresh_vals[curr_ptr]){
                    fprintf(output, "\t movzbl\t(%%%s), %%%s\n",
                        ptr_regs[curr_ptr], val_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 0;
                }
                fprintf(output, "\t inc\t%%%s\n", val_regs[curr_ptr]);
                fprintf(output, "\t movb\t%%%s, (%%%s)\n", val_bregs[curr_ptr],
                    ptr_regs[curr_ptr]);
                break;
            case ADDV:
                if(refresh_vals[curr_ptr]){
                    fprintf(output, "\t movzbl\t(%%%s), %%%s\n",
                        ptr_regs[curr_ptr], val_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 0;
                }
                fprintf(output, "\t addl\t$%d, %%%s\n", op->arg, val_regs[curr_ptr]);
                fprintf(output, "\t movb\t%%%s, (%%%s)\n", val_bregs[curr_ptr],
                    ptr_regs[curr_ptr]);
                break;
            case SUB:
                if(refresh_vals[curr_ptr]){
                    fprintf(output, "\t movzbl\t(%%%s), %%%s\n",
                        ptr_regs[curr_ptr], val_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 0;
                }
                fprintf(output, "\t dec\t%%%s\n", val_regs[curr_ptr]);
                fprintf(output, "\t movb\t%%%s, (%%%s)\n", val_bregs[curr_ptr],
                    ptr_regs[curr_ptr]);
                break;
            case SUBV:
                if(refresh_vals[curr_ptr]){
                    fprintf(output, "\t movzbl\t(%%%s), %%%s\n",
                        ptr_regs[curr_ptr], val_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 0;
                }
                fprintf(output, "\t subl\t$%d, %%%s\n", op->arg, val_regs[curr_ptr]);
                fprintf(output, "\t movb\t%%%s, (%%%s)\n", val_bregs[curr_ptr],
                    ptr_regs[curr_ptr]);
                break;
            case ZERO:
                fprintf(output, "\t xorl\t%%%s, %%%s\n", val_regs[curr_ptr],
                    val_regs[curr_ptr]);
                fprintf(output, "\t movb\t%%%s, (%%%s)\n", val_bregs[curr_ptr],
                    ptr_regs[curr_ptr]);
                refresh_vals[curr_ptr] = 0;
                break;
            case LABEL:
                /* TODO: Figure out how we're going to handle curr_ptr assumptions
                 *  and refresh requirements around loops. A stack might be a 
                 *  good idea */
                fprintf(output, ".L%d:\n", op->arg);
                for(int i = 0; i < 3; i++){
                    refresh_ptrs[i] = refresh_vals[i] = 1;
                }
                break;
            case JNZ:
                if(refresh_vals[curr_ptr]){
                    fprintf(output, "\t movzbl\t(%%%s), %%%s\n", 
                        ptr_regs[curr_ptr], val_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 0;
                }
                fprintf(output, "\t test\t%%%s, %%%s\n", val_regs[curr_ptr], 
                    val_regs[curr_ptr]);
                fprintf(output, "\t jnz\t.L%d\n", op->arg);
                break;
            case JZ:
                if(refresh_vals[curr_ptr]){
                    fprintf(output, "\t movzbl\t(%%%s), %%%s\n", 
                        ptr_regs[curr_ptr], val_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 0;
                }
                fprintf(output, "\t test\t%%%s, %%%s\n", val_regs[curr_ptr], 
                    val_regs[curr_ptr]);
                fprintf(output, "\t jz\t.L%d\n", op->arg);
                break;
            case PUT:
                if(refresh_vals[curr_ptr]){
                    fprintf(output, "\t movzbl\t(%%%s), %%%s\n", 
                        ptr_regs[curr_ptr], val_regs[curr_ptr]);
                    refresh_vals[curr_ptr] = 0;
                }
                fprintf(output, "\t movzbl\t%%%s, %%%s\n", val_bregs[curr_ptr],
                    val_regs[curr_ptr]);
                fprintf(output, "\t movl\t%%%s, (%%esp)\n", val_regs[curr_ptr]);
                fprintf(output, "\t call\tfputc\n");
                break;
            case GET:
                fprintf(output, "\t movl\tstdin, %%%s\n", val_regs[curr_ptr]);
                fprintf(output, "\t movl\t%%%s, (%%esp)\n", val_regs[curr_ptr]);
                fprintf(output, "\t call\tfgetc\n");
                fprintf(output, "\t movb\t%%%s, (%%%s)\n", val_bregs[curr_ptr],
                    ptr_regs[curr_ptr]);
                break;
        }
        node = node->next;
    }

    /* Finish up the bf program function and fill in main */
#if 0
    fprintf(output, "\t popl\t%%edi\n");
    fprintf(output, "\t popl\t%%esi\n");
#endif
    fprintf(output, "\t addl\t$8, %%esp\n");
    fprintf(output, "\t popl\t%%edi\n");
    fprintf(output, "\t popl\t%%esi\n");
    fprintf(output, "\t popl\t%%ebx\n");
    fprintf(output, "\t popl\t%%ebp\n");
    fprintf(output, "\t ret\n\t .size\tbf_prog, .-bf_prog\n");
    fprintf(output, ".globl main\n\t .type\tmain, @function\nmain:\n");
    fprintf(output, "\t pushl\t%%ebp\n");
    fprintf(output, "\t movl\t%%esp, %%ebp\n");
    fprintf(output, "\t andl\t$-16, %%esp\n");
    fprintf(output, "\t subl\t$32, %%esp\n");
    fprintf(output, "\t movl\t$1, 4(%%esp)\n");
    fprintf(output, "\t movl\t$30000, (%%esp)\n");
    fprintf(output, "\t call\tcalloc\n");
    fprintf(output, "\t movl\t%%eax, 28(%%esp)\n");
    fprintf(output, "\t movl\t28(%%esp), %%eax\n");
    fprintf(output, "\t movl\t%%eax, (%%esp)\n");
    fprintf(output, "\t call\tbf_prog\n");
    fprintf(output, "\t movl\t$0, %%eax\n");
    fprintf(output, "\t leave\n");
    fprintf(output, "\t ret\n");
    fprintf(output, "\t .size\tmain, .-main\n");
    fprintf(output, "\t .ident\t\"bfcc 1.0.0\"\n");
    fprintf(output, "\t .section\t.note.GNU-stack,\"\",@progbits\n");
    
}

void bfcc_gen64(FILE *output, list_t *parse_lst, char *filename){
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

int32_t bfop_structural_eq(const void *op1, const void *op2){
    bfop_t *lhs = (bfop_t *)op1;
    bfop_t *rhs = (bfop_t *)op2;

    if(lhs->opcode == rhs->opcode && lhs->arg == rhs->arg){
        return 1;
    }

    switch(lhs->opcode){
        case JZ:
        case JNZ:
        case LABEL:
        case ZERO:
        case PUT:
        case GET:
            return lhs->opcode == rhs->opcode;
        case ADD:
            return rhs->opcode == ADD || (rhs->opcode == ADDV && rhs->arg == 1)
                    || (rhs->opcode == SUBV && rhs->arg == -1);
        case SUB:
            return rhs->opcode == SUB || (rhs->opcode == SUBV && rhs->arg == 1)
                || (rhs->opcode == ADDV && rhs->arg == -1);
        case INC:
            return rhs->opcode == INC || (rhs->opcode == INCV && rhs->arg == 1)
                || (rhs->opcode == DECV && rhs->arg == -1);
        case DEC:
            return rhs->opcode == DEC || (rhs->opcode == DECV && rhs->arg == 1)
                || (rhs->opcode == INCV && rhs->arg == -1);
        case ADDV:
            if(rhs->opcode == ADD)
                return lhs->arg == 1;
            if(rhs->opcode == SUB)
                return lhs->arg == -1;
            if(rhs->opcode == SUBV)
                return lhs->arg == -(rhs->arg);
            break;
        case SUBV:
            if(rhs->opcode == SUB)
                return lhs->arg == 1;
            if(rhs->opcode == ADD)
                return lhs->arg == -1;
            if(rhs->opcode == ADDV)
                return lhs->arg == -(rhs->arg);
            break;
        case INCV:
            if(rhs->opcode == INC)
                return lhs->arg == 1;
            if(rhs->opcode == DEC)
                return lhs->arg == -1;
            if(rhs->opcode == DECV)
                return lhs->arg == -(rhs->arg);
            break;
        case DECV:
            if(rhs->opcode == DEC)
                return lhs->arg == 1;
            if(rhs->opcode == INC)
                return lhs->arg == -1;
            if(rhs->opcode == INCV)
                return lhs->arg == -(rhs->arg);
            break;

    }
    return 0;
}

list_t *bfopt_apply_filter(list_t *parse_lst, list_t *pattern, list_t *replace){
    
    /* Applies the filter described in pattern to the parse list. */
    int32_t label = -1, replace_labels = 0;
    node_t *node = parse_lst->head->next;
    while(node != parse_lst->head){
        /*Find the highest loop label so as to avoid labelling conflicts */
        bfop_t *op = (bfop_t *)node->data;
        if(op->opcode == LABEL && op->arg > label){
            label = op->arg;
        }
        node = node->next;
    }
    label++;

    /*Determine how many labels there are in the replace list. */
    node = replace->head->next;
    while(node != replace->head){
        bfop_t *op = (bfop_t *)node->data;
        if(op->opcode == LABEL)
            replace_labels++;

        node = node->next;
    }

    node = parse_lst->head->next;
    node_t *leader = node;
    for(int i = 1; i < pattern->length; i++){
        leader = leader->next;
        if(leader == parse_lst->head){
            break;
        }
    }
    while(leader != parse_lst->head){
        int32_t match = list_match(parse_lst, node, pattern, pattern->head->next,
                            bfop_structural_eq, pattern->length);

        if(match){
            node_t *last = node->prev;
            for(int j = 0; j < pattern->length; j++){
                free(list_remove(last->next));
            }
            last = last->next;
            node_t *foo = replace->head->next;
            while(foo != replace->head){
                bfop_t *fop = (bfop_t *)foo->data;
                int32_t farg = fop->arg;
                if(bfop_type(fop->opcode) == T_BRANCH){
                    farg += label;
                }

                list_insertbefore(last, bfop_new(fop->opcode, farg));
                foo = foo->next;
            }
            label += replace_labels;
            node = last;
            leader = node;
            for(int i = 1; i < pattern->length; i++){
                leader = leader->next;
                if(leader == parse_lst->head){
                    break;
                }
            }
        }else{
            node = node->next;
            leader = leader->next;
        }
    }
    return parse_lst;
}

list_t *bfopt_make_zeros(list_t *parse_lst){
    list_t pattern, replace;
    list_init(&pattern);
    list_init(&replace);

    list_addlast(&pattern, bfop_new(JZ, 0));
    list_addlast(&pattern, bfop_new(LABEL, 1));
    list_addlast(&pattern, bfop_new(SUB, 0));
    list_addlast(&pattern, bfop_new(JNZ, 1));
    list_addlast(&pattern, bfop_new(LABEL, 0));

    list_addfirst(&replace, bfop_new(ZERO, 0));

    bfopt_apply_filter(parse_lst, &pattern, &replace);

    list_clear(&pattern, 1);
    list_clear(&replace, 1);
    return parse_lst;
}

void load_filter(FILE *input, list_t *pattern, list_t *replace){
    char buf[24];
    char *commands[] = {
        "inc",
        "incv", 
        "dec", 
        "decv", 
        "add",
        "addv",
        "sub",
        "subv",
        "put",
        "get",
        "jnz",
        "jz",
        "zero"
    };
    int32_t opcodes[] = {INC, INCV, DEC, DECV, ADD, ADDV, SUB, SUBV, PUT, GET,
                            JNZ, JZ, ZERO};
    int32_t length = sizeof(opcodes) / sizeof(opcodes[0]);
    int32_t cmd, arg;

    /* Burn the first input line; should be "pattern" */
    fscanf(input, "%s", buf);
    
    while(fscanf(input, "%s", buf) != EOF){
        if(!strcmp(buf, "replace")){
            break;
        }
        cmd = LABEL;
        for(int i = 0; i < length; i++){
            if(!strcmp(buf, commands[i])){
                cmd = opcodes[i];
                arg = 0;
                break;
            }
        }
        switch(cmd){
            case INCV:
            case DECV:
            case ADDV:
            case SUBV:
                fscanf(input, "%d", &arg);
                break;
            case JNZ:
            case JZ:
                fscanf(input, " L%d", &arg);
                break;
            case LABEL:
                sscanf(buf, "L%d:", &arg);
                break;
        }

        list_addlast(pattern, bfop_new(cmd, arg));
    }
    while(fscanf(input, "%s", buf) != EOF){
        cmd = LABEL;
        for(int i = 0; i< length; i++){
            if(!strcmp(buf, commands[i])){
                cmd = opcodes[i];
                arg = 0;
                break;
            }
        }
        switch(cmd){
            case INCV:
            case DECV:
            case ADDV:
            case SUBV:
                fscanf(input, " %d", &arg);
                break;
            case JNZ:
            case JZ:
                fscanf(input, " L%d", &arg);
                break;
            case LABEL:
                sscanf(buf, "L%d:", &arg);
                break;
           
        }
        list_addlast(replace, bfop_new(cmd, arg));
    }
}

void apply_filter_file(char *filename, list_t *parse_lst){
    FILE *input = fopen(filename, "r");
    if(!input){
        CriticalError("Could not open file");
    }

    list_t pattern, replace;
    list_init(&pattern);
    list_init(&replace);

    load_filter(input, &pattern, &replace);
    bfopt_apply_filter(parse_lst, &pattern, &replace);

    list_clear(&replace, 1);
    list_clear(&pattern, 1);
}

void exec_and_block(const char *filename, const char *argv[], const char *envp[]){
    int i = 0;
    const char *arg;
    while((arg = argv[i++]) != NULL){
        fprintf(stderr, "%s ", arg);
    }
    fprintf(stderr, "\n");

    /* Block SIGCHLD */

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL);

    pid_t pid = fork();
    if(pid == -1){
        fprintf(stderr, "fork failed\n");
    }
    if(pid == 0){
        /* We're the child process, so go ahead and execve */
        setpgid(0, 0);
        sigprocmask(SIG_UNBLOCK, &set, NULL); /* Unblock SIGCHLD */
        execve(filename, (char * const*)argv, (char * const *)envp);

        /*Control should never reach this line. If we did, you dun goofed */
        fprintf(stderr, "Unknown command: %s\n", filename);
        exit(1);
    }else{
        int status = 0;
        sigprocmask(SIG_UNBLOCK, &set, NULL);
        do{
            waitpid(pid, &status, 0);
        }while(!(WIFEXITED(status) || WIFSIGNALED(status)));
    }
}

int main(int argc, char **argv){
    char *filters[] = {
        "filters/zero.flt"
    };
    size_t filter_length = sizeof(filters) / sizeof(filters[0]);
    char *output_fname, *output_fdup;
    int32_t verbose = 0;
    int32_t opt_level;
    int32_t output_mode = -1;
    void (*codegen)(FILE *, list_t *, char *);

    /* If we compiled the compiler 64-bit, we probably want to compile brainfuck
     * to 64-bit also, and likewise for 32-bit */
#ifdef __LP64__
    codegen = bfcc_gen64;
#else
    codegen = bfcc_gen32;
#endif
#if 1
    /* XXX Get rid of this when bfcc_gen64 is finished */
    codegen = bfcc_gen32;
#endif

    struct option long_options[] = {
        {"verbose", no_argument, NULL, 'v'},
        {"m32", no_argument, NULL, 'l'},
        {"m64", no_argument, NULL, 'q'},
        {"bytecode", no_argument, NULL, 'b'}
    };
    int option_index = 0;

    char c;
    while((c = getopt_long_only(argc, argv, "vbO:", long_options, &option_index)) != -1){
        switch(c){
            case 'v':
                verbose = 1;
                break;
            case 'l':
                /* l option forces codegen = m32 */
                codegen = bfcc_gen32;
                output_mode = BFCCOUT_32BIT;
                break;
            case 'q':
                codegen = bfcc_gen64;
                output_mode = BFCCOUT_64BIT;
                break;
            case 'b':
                /* Platform-independent bytecode */
                codegen = bfcc_codegen;
                output_mode = BFCCOUT_BYTECODE;
                break;
            case 'O':
                opt_level = (int32_t)atoi(optarg);
                break;
        }
    }

    if(optind >= argc){
        CriticalError("Must provide a .b source file to compile");
    }

    FILE *input = fopen(argv[optind], "r");
    if(!input){
        CriticalError("Could not open file");
    }
    size_t ilen = strlen(argv[optind]) + 4;
    output_fname = (char *)alloca(ilen * sizeof(char));
    strcpy(output_fname, argv[optind]);
    char *p = strstr(output_fname, ".b");
    intptr_t pi = (intptr_t)(p - output_fname);
    if(output_mode == BFCCOUT_BYTECODE){
        strcpy(output_fname + pi, ".bc");
    }else{
        strcpy(output_fname + pi, ".s");
    }

    /* output_fdup - output name, without extension */
    output_fdup = (char *)alloca(ilen * sizeof(char));
    strcpy(output_fdup, output_fname);
    output_fdup[pi] = '\0';

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

    for(int i = 0; i<filter_length; i++){
        apply_filter_file(filters[i], &list);
    }

    FILE *output = fopen(output_fname, "w");
    if(!output){
        CriticalError("Could not open file");
    }

    codegen(output, &list, output_fname);
    fclose(output);
    list_clear(&list, 1);

    const char *gcc_args[] = {
        "/usr/bin/gcc",
        "-o", 
        output_fdup,
        output_fname,
        "-m32",         /* XXX Get rid of this when bfcc_gen64 is up */
        NULL
    };
    if(output_mode == BFCCOUT_32BIT){
        gcc_args[4] = "-m32";
    }else if(output_mode == BFCCOUT_64BIT){
        gcc_args[4]=  "-m64";
    }else if(output_mode == BFCCOUT_BYTECODE){
        return 0;
    }
    exec_and_block(gcc_args[0], gcc_args, (const char **)environ);

    const char *rm_args[] = {
        "/bin/rm",
        output_fname,
        NULL
    };
    exec_and_block(rm_args[0], rm_args, (const char **)environ);

    return 0;
}


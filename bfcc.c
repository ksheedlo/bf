/* Ken Sheedlo
 * bfcc, a tiny optimizing brainfuck compiler in C */

#include "bfcc.h"

void CriticalError(char *str){
    fprintf(stderr, "%s\n", str);
    exit(1);
}

void list_init(list_t *list){
    //Initialize the list to empty (one sentinel node).
    node_t *new_node = malloc(sizeof(node_t));
    if(new_node == NULL){
        CriticalError("Failed to allocate memory");
    }
    new_node->data = NULL;
    new_node->next = new_node;
    new_node->prev = new_node;

    list->head = new_node;
    list->length = 0;
}

void list_addfirst(list_t *list, void *data){
    //Add data to a new node at the head of the list.
    node_t *new_node = malloc(sizeof(node_t));
    if(new_node == NULL){
        CriticalError("Failed to allocate memory");
    }

    new_node->data = data;
    new_node->next = list->head->next;
    new_node->prev = list->head;
    new_node->list = list;

    new_node->next->prev = new_node;
    list->head->next = new_node;

    list->length = list->length + 1;
}

void list_addlast(list_t *list, void *data){
    //Add data to a new node at the tail of the list.
    node_t *new_node = malloc(sizeof(node_t));
    if(new_node == NULL){
        CriticalError("Failed to allocate memory");
    }

    new_node->data = data;
    new_node->list = list;
    new_node->next = list->head;
    new_node->prev = list->head->prev;

    new_node->prev->next = new_node;
    list->head->prev = new_node;

    list->length = list->length + 1;
}

void *list_remove(node_t *node){
    /*Remove the specified node from it's list and return a ptr to its data
     * entry. */
    void *ret = node->data;

    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->list->length = node->list->length - 1;

    free(node);
    return ret;
}

void list_clear(list_t *list, int32_t free_data){
    //Clear the list, freeing data if specified.
    while(list->head->next != list->head){
        void *data = list_remove(list->head->next);
        if(free_data){
            free(data);
        }
    }
    free(list->head);
}

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

list_t *bfcc_parse(char *program, list_t *parse_lst){
    /*Produces the initial parse list for the program. Does not allocate
     * memory for a new list because it may be used recursively. */
    char input;
    int32_t label_ct = 0;
    list_t loop_stack;
    list_init(loop_stack);

    while((input = *program++) != '\0'){
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
                list_addfirst(loop_stack, (void *)(label_ct + 1));
                list_addfirst(loop_stack, (void *)label_ct);

                label_ct += 2;
                break;
            case ']':
                //Try to pop from the stack. If it's empty, you dun goofed
                if(loop_stack->length <= 1){
                    fprintf(stderr, "Error: mismatched []\n");
                    exit(1);
                }

                int32_t top_label = (int32_t)list_remove(list->head->next);
                int32_t bot_label = (int32_t)list_remove(list->head->next);

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
    }
}

void bfcc_gen32(FILE *output, list_t *parse_lst){
    //XXX

}

void bfcc_gen64(FILE *output, list_t *parse_lst){
    //XXX

}

list_t *bfopt_combine_arith(list_t *parse_lst){
    //XXX

    return parse_lst;
}

list_t *bfopt_make_zeros(list_t *parse_lst){
    //XXX

    return parse_lst;
}

int main(int argc, char **argv){
    
    return 0;
}


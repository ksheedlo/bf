/* Ken Sheedlo
 * bfcc, a tiny optimizing brainfuck compiler in C */

#include "bfcc.h"

void list_init(list_t *list){
    //XXX

}

void list_addfirst(list_t *list, void *data){
    //XXX

}

void list_addlast(list_t *list, void *data){
    //XXX

}

node_t *list_remove(node_t *node){
    //XXX

    return NULL;
}

void list_clear(list_t *list){
    //XXX

}

bfop_t *bfop_new(int32_t opcode, int32_t arg){
    //XXX

    return NULL;
}

list_t *bfcc_parse(char *program){
    //XXX

    return NULL;
}

void bfcc_codegen(FILE *output, list_t *parse_lst){
    //XXX

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


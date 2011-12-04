/*Ken Sheedlo
 *brainfuck compiler includes and defs */

#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

#include "error_handling.h"
#include "list.h"
#include "bfop.h"

/*Function definitions. */
list_t *bfcc_parse(char *program, list_t *parse_lst);

void bfcc_codegen(FILE *output, list_t *parse_lst);

void bfcc_gen32(FILE *output, list_t *parse_lst);

void bfcc_gen64(FILE *output, list_t *parse_lst);

list_t *bfopt_combine_arith(list_t *parse_lst);

list_t *bfopt_make_zeros(list_t *parse_lst);


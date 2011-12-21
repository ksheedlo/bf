/*Ken Sheedlo
 *brainfuck compiler includes and defs */

#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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

list_t *bfopt_apply_filter(list_t *parse_lst, list_t *pattern, list_t *replace);

/*Compares op1, op2 for structural equality.*/
int32_t bfop_structural_eq(const void *op1, const void *op2);

void load_filter(FILE *input, list_t *pattern, list_t *replace);

void apply_filter_file(char *filename, list_t *parse_lst);


/*Ken Sheedlo's brainfuck compiler
 * Unit test program. */

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#include "bfcc.h"

int32_t bfop_equal(const void *lhs, const void *rhs);

int32_t list_equal(list_t *lhs, list_t *rhs, int32_t (*eq)(const void *, const void *));

void generic_bfop_print(FILE *output, const void *op);

void list_print(FILE *output, list_t *lst, void (*disp)(FILE *, const void *));

int32_t assert_bfop_lstcontents(list_t *lst, int32_t *ops, int32_t *args, int32_t len);

int32_t test_list_addfirst();

int32_t test_list_addlast();

int32_t test_list_remove();

int32_t test_list_match();


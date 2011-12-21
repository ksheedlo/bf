/*Ken Sheedlo's brainfuck compiler
 * Unit test program. */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#include "bfcc.h"

int32_t bfop_equal(const void *lhs, const void *rhs);

int32_t list_equal(list_t *lhs, list_t *rhs, int32_t (*eq)(const void *, const void *));

int32_t assert_bfop_lstcontents(list_t *lst, int32_t *ops, int32_t *args, int32_t len);

int32_t test_list_addfirst();

int32_t test_list_addlast();

int32_t test_list_remove();

int32_t test_list_match();


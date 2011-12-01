/*Ken Sheedlo
 *brainfuck compiler includes and defs */

#include<stdint.h>
#include<stdio.h>

/* Command constants */
#define INC 0
#define INCV 1
#define DEC 2
#define DECV 3
#define ADD 4
#define ADDV 5
#define SUB 6
#define SUBV 7
#define PUT 8
#define GET 9
#define LABEL 10
#define JNZ 11
#define ZERO 12

/* Command type constants */
#define T_PTR 0
#define T_ARITH 1
#define T_IO 2
#define T_BRANCH 3

/* General data structures. */
struct _ll_t;   /*Forward declaration*/

typedef struct _ll_node {
    void *data;
    struct _ll_t *list;
    struct _ll_node *next;
} node_t;

typedef struct _ll_t {
    node_t *head;
    node_t *tail;
    size_t length;
} list_t;

/* Application-specific data structures. */
typedef struct {
    int32_t arg;    /* May or may not be req. depending on opcode */
    int32_t opcode;
} bfop_t;

/*Function definitions. */
void list_init(list_t *list);

void list_addfirst(list_t *list, void *data);

void list_addlast(list_t *list, void *data);

node_t *list_remove(node_t *node);

void list_clear(list_t *list);

bfop_t *bfop_new(int32_t opcode, int32_t arg);

list_t *bfcc_parse(char *program);

void bfcc_codegen(FILE *output, list_t *parse_lst);

void bfcc_gen32(FILE *output, list_t *parse_lst);

void bfcc_gen64(FILE *output, list_t *parse_lst);

list_t *bfopt_combine_arith(list_t *parse_lst);

list_t *bfopt_make_zeros(list_t *parse_lst);


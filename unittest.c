/*Ken Sheedlo's brainfuck compiler
 * Unit tester */

#include "unittest.h"

int32_t bfop_equal(const void *lhs, const void *rhs){
    bfop_t *lop = (bfop_t *)lhs;
    bfop_t *rop = (bfop_t *)rhs;
    return (lop->opcode == rop->opcode) && (lop->arg == rop->arg);
}

int32_t list_equal(list_t *lhs, list_t *rhs, int32_t (*eq)(const void *, const void *)){
    if(lhs->length != rhs->length){
        return 0;
    }

    node_t *foo = lhs->head->next;
    node_t *bar = rhs->head->next;

    while(foo != lhs->head){
        if(!eq(foo->data, bar->data)){
            return 0;
        }
        foo = foo->next;
        bar = bar->next;
    }
    return 1;
}

void generic_bfop_print(FILE *output, const void *op){
    bfop_print(output, (const bfop_t *)op);
}

void list_print(FILE *output, list_t *lst, void (*disp)(FILE *, const void *)){
    fprintf(output, "[");
node_t *foo = lst->head->next; while(foo != lst->head){
        disp(output, foo->data);
        if(foo->next != lst->head){
            fprintf(output, ", ");
        }
        foo = foo->next;
    }

    fprintf(output, "]\n");
}

int32_t assert_bfop_lstcontents(list_t *lst, int32_t *ops, int32_t *args, int32_t len){
    if(lst->length != len){
        fprintf(stderr, "Wrong list length, expected %d, actual is %d\n", 
            len, (int32_t)lst->length);
        list_print(stderr, lst, generic_bfop_print);
        return 0;
    }

    node_t *node = lst->head->next;
    for(int i = 0; i < len; i++){
        bfop_t expected;
        expected.opcode = ops[i];
        expected.arg = args[i];
        if(!bfop_equal(&expected, node->data)){
            fprintf(stderr, "Error: lists not equal\n");
            list_print(stderr, lst, generic_bfop_print);
            fprintf(stderr, "Expected ith value (i=%d): ", i);
            bfop_print(stderr, &expected);
            fprintf(stderr, "\n");
            return 0;
        }
        node = node->next;
    }
    return 1;
}

int32_t test_list_addfirst(){
    list_t foo;
    list_init(&foo);

    list_addfirst(&foo, bfop_new(ADDV, 5));
    list_addfirst(&foo, bfop_new(INC, 1));
    list_addfirst(&foo, bfop_new(JNZ, 42));

    int32_t op_expected[] = {JNZ, INC, ADDV};
    int32_t arg_expected[] = {42, 1, 5};

    int32_t ret = assert_bfop_lstcontents(&foo, op_expected, arg_expected, 3);

    list_clear(&foo, 1);
    return ret;
}

int32_t test_list_addlast(){
    list_t list;
    list_init(&list);

    list_addlast(&list, bfop_new(LABEL, 0xF00));
    list_addlast(&list, bfop_new(ZERO, 0));
    list_addlast(&list, bfop_new(INCV, 42));

    int32_t op_expected[] = {LABEL, ZERO, INCV};
    int32_t arg_expected[] = {0xF00, 0, 42};

    int32_t ret = assert_bfop_lstcontents(&list, op_expected, arg_expected, 3);

    list_clear(&list, 1);
    return ret;
}

int32_t test_list_remove(){
    list_t list;
    list_init(&list);

    list_addlast(&list, bfop_new(SUB, 0));
    list_addlast(&list, bfop_new(INCV, 42));
    list_addlast(&list, bfop_new(JZ, 0x20));
    list_addlast(&list, bfop_new(LABEL, 0xF00));
    list_addlast(&list, bfop_new(ADDV, 2));

    int32_t ox[] = {SUB, INCV, JZ, LABEL, ADDV};
    int32_t ax[] = {0, 42, 0x20, 0xF00, 2};

    int32_t st = assert_bfop_lstcontents(&list, ox, ax, 5);

    if(!st){
        list_clear(&list, 1);
        return 0;
    }

    free(list_remove(list.head->next->next));
    free(list_remove(list.head->prev));

    int32_t ox2[] = {SUB, JZ, LABEL};
    int32_t ax2[] = {0, 0x20, 0xF00};

    st = assert_bfop_lstcontents(&list, ox2, ax2, 3);
    list_clear(&list, 1);
    return st;
}

int32_t test_list_match(){
    /*Pass in a match and a non-match to make sure match works properly*/
    list_t lst1, lst2;
    list_init(&lst1);
    list_init(&lst2);

    list_addlast(&lst1, bfop_new(SUB, 0));
    list_addlast(&lst1, bfop_new(INCV, 42));
    list_addlast(&lst1, bfop_new(JZ, 0x20));
    list_addlast(&lst1, bfop_new(LABEL, 0xF00));
    list_addlast(&lst1, bfop_new(ADDV, 2));
    
    list_addlast(&lst2, bfop_new(INCV, 42));
    list_addlast(&lst2, bfop_new(JZ, 0x20));
    list_addlast(&lst2, bfop_new(LABEL, 0xF00));

    int32_t st = list_match(&lst1, lst1.head->next->next, &lst2, lst2.head->next, 
                            bfop_equal, 3);

    if(!st){
        list_clear(&lst1, 1);
        list_clear(&lst2, 1);
        return 0;
    }

    st = list_match(&lst1, lst1.head->next, &lst2, lst2.head->next, bfop_equal, 3);
    list_clear(&lst1, 1);
    list_clear(&lst2, 1);

    return !st;
}

int main(int argc, char **argv){
    int32_t (*TESTS[])() = {
        test_list_addfirst,
        test_list_addlast,
        test_list_remove,
        test_list_match
    };

    const int32_t TEST_LENGTH = sizeof(TESTS) / sizeof(TESTS[0]);

    for(int i = 0; i<TEST_LENGTH; i++){
        int32_t (*test)() = TESTS[i];
        if(!test()){
            fprintf(stderr, "Test %d FAILED\n", i+1);
            return 1;
        }
        fprintf(stderr, "Test %d PASSED\n", i+1);
    }
    fprintf(stderr, "All tests passed\n");
    return 0;
}

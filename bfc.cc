/* Ken Sheedlo
 * Optimizing brainfuck Compiler
 * Generates LLVM IR which we can compile to whatever */

#include "bfc.h"

void *Error(const char *str){
    fprintf(stderr, "%s\n", str);
    return NULL;
}

Value *ErrorV(const char *str){
    Error(str);
    return NULL;
}

CharExprAST::CharExprAST(char val){
    expr = val;
}

Value *CharExprAST::Codegen(){
    //Generate according to the particular non-branching operator.
    switch(expr){
        case '+':
            // *ptr++

            break;
        case '-':
            // *ptr--

            break;
        case '<':
            // ptr--

            break;
        case '>':
            // ptr++

            break;
        case '.':
            // fputc(*ptr, stdout)

            break;
        case ',':
            // *ptr = fgetc(stdin)

            break;
        //We should ABSOLUTELY NOT get any looping constructs in here.
    }

    return NULL;
}

LoopExprAST::LoopExprAST(ExprAST *foo){
    body = foo;
}

Value *LoopExprAST::Codegen(){
    //TODO: implement this

    return NULL;
}


int main(int argc, char **argv){
    

    return 0;
}

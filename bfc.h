/* Ken Sheedlo
 * Optimizing brainfuck Compiler */

#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/IRBuilder.h"
#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>

#include<map>
#include<vector>

#include "lexer.h"

using namespace llvm;

void *Error(const char *str);

Value *ErrorV(const char *str);

/* This expression tree shit is probably overkill for brainfuck. 
 * Make this more elegant once we figure out what's going on. */
class ExprAST {
public:
    virtual ~ExprAST() {}
    virtual Value *Codegen() = 0;
};

/* Create a general expression tree for character expressions,
 * i.e., everything that can't be expressed as a loop */
class CharExprAST : public ExprAST {
    char expr;
public:
    CharExprAST(char val);
    virtual Value *Codegen();
};

/* Create an expression tree for loop constructs. Since all loops work the same
 * way in brainfuck, we don't need to have an explicit loop condition */
class LoopExprAST : public ExprAST {
    ExprAST *body;
public:
    LoopExprAST(ExprAST *foo);
    virtual Value *Codegen();
};



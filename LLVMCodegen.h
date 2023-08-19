#ifndef CODEGEN_H
#define CODEGEN_H

#include <cjson/cJSON.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Initialization.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/Target.h>

#define i32 LLVMInt32Type()

#define getType(node) cJSON_GetObjectItem(node,"type")->valuestring
#define getIdent(node) cJSON_GetObjectItem(node,"ident")->valuestring
#define getVal(node) cJSON_GetObjectItem(node,"value")->valueint
#define getExpr(node) cJSON_GetObjectItem(node,"expr")
#define getStmt(node) cJSON_GetObjectItem(node,"stmt")
#define getOp(node) cJSON_GetObjectItem(node,"op")->valueint
#define getRelOp(node) cJSON_GetObjectItem(node,"rel-op")->valueint
#define getLeft(node) cJSON_GetObjectItem(node,"left")
#define getRight(node) cJSON_GetObjectItem(node,"right")

typedef struct Symbol {
    char* name;
    LLVMValueRef value;
    struct Symbol* next; // Used for collision handling in hash table
} Symbol;

#define SYMBOL_TABLE_SIZE 211  // Choose a prime number for better distribution

typedef struct SymbolTable {
    Symbol* table[SYMBOL_TABLE_SIZE];
} SymbolTable;

SymbolTable* createSymbolTable();
unsigned int hash(char* name);
void insertSymbol(SymbolTable* st, char* name, LLVMValueRef value);
LLVMValueRef lookupSymbol(SymbolTable* st, char* name);
void freeSymbolTable(SymbolTable* st);

void generateProgram(LLVMBuilderRef builder, cJSON *ast);
void generateBlock(LLVMBuilderRef builder, cJSON *block);
void generateDecs(LLVMBuilderRef builder, cJSON *decs);
void generateDeclaration(LLVMBuilderRef builder, cJSON *dec);
void generateMain(LLVMBuilderRef builder, cJSON *block);
void generateStatement(LLVMBuilderRef builder, cJSON *stmt);
LLVMValueRef generateCondition(LLVMBuilderRef builder, cJSON *condition);
LLVMValueRef generateExpr(LLVMBuilderRef builder, cJSON *expr);
void generatePrint(LLVMBuilderRef builder, cJSON *expr);


extern cJSON* ast;

int codegen();

#endif // CODEGEN_H
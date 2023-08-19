#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Initialization.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/Target.h>

#include "LLVMCodegen.h"
#include "weaklang.tab.h"

SymbolTable* table;

LLVMContextRef context;
LLVMModuleRef module;
LLVMValueRef formatStr, printfFunc;
LLVMValueRef mainFunc;

int codegen() {
    
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();

    module = LLVMModuleCreateWithName("my_module");
    LLVMBuilderRef builder = LLVMCreateBuilder();

    table = createSymbolTable();
    
    generateProgram(builder, ast);

    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmPrinters();

    char *error = NULL;
    LLVMTargetRef targetRef;
    if (LLVMGetTargetFromTriple(LLVMGetDefaultTargetTriple(), &targetRef, &error)) {
        // Handle error
        printf("1");
        return;
    }

    LLVMTargetMachineRef targetMachine = LLVMCreateTargetMachine(targetRef, 
    LLVMGetDefaultTargetTriple(), "generic", "", 
    LLVMCodeGenLevelDefault, LLVMRelocPIC, LLVMCodeModelDefault);

    char *errMsg = NULL;
    if (LLVMVerifyModule(module, LLVMAbortProcessAction, &errMsg)) {
        printf("Error validating IR: %s\n", errMsg);
        LLVMDisposeMessage(errMsg); // Remember to dispose error message
        exit(EXIT_FAILURE);
    }
    
    if (LLVMTargetMachineEmitToFile(targetMachine, module, "output.o", LLVMObjectFile, &error)) {
        printf("Error: %s\n", error);
        LLVMDisposeMessage(error); // Don't forget to dispose of the error message once done.
        return;
    }
    
    LLVMDumpModule(module);

    // Cleanup
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);

    return 0;
}

// int main(int argc, char const *argv[]) {

//     LLVMInitializeNativeTarget();
//     LLVMInitializeNativeAsmPrinter();

//     module = LLVMModuleCreateWithName("my_module");
//     LLVMBuilderRef builder = LLVMCreateBuilder();

//     table = createSymbolTable();

//     // Assuming parsed AST from JSON
//     FILE *cin;
//     cin = fopen("ir.json","rb");

//     if (cin == NULL) {
//         perror("Failed to open file \"ir.json\"");
//         return 1;
//     }

//     // Move the file pointer to the end of the file and determine its size
//     fseek(cin, 0, SEEK_END);
//     long filesize = ftell(cin);
//     fseek(cin, 0, SEEK_SET);

//     // Allocate memory to read the file
//     char *content = (char *)malloc(filesize + 1);
//     if (content == NULL) {
//         perror("Failed to allocate content buffer");
//         fclose(cin);
//         return 1;
//     }

//     fread(content, 1, filesize, cin);
//     content[filesize] = '\0';  // Null-terminate the content string

//     fclose(cin);

//     ast = cJSON_Parse(content);
    
//     generateProgram(builder, ast);

    

//     LLVMInitializeAllTargetInfos();
//     LLVMInitializeAllTargets();
//     LLVMInitializeAllTargetMCs();
//     LLVMInitializeAllAsmPrinters();

//     char *error = NULL;
//     LLVMTargetRef targetRef;
//     if (LLVMGetTargetFromTriple(LLVMGetDefaultTargetTriple(), &targetRef, &error)) {
//         // Handle error
//         printf("1");
//         return;
//     }

//     LLVMTargetMachineRef targetMachine = LLVMCreateTargetMachine(targetRef, 
//     LLVMGetDefaultTargetTriple(), "generic", "", 
//     LLVMCodeGenLevelDefault, LLVMRelocPIC, LLVMCodeModelDefault);

//     char *errMsg = NULL;
//     if (LLVMVerifyModule(module, LLVMAbortProcessAction, &errMsg)) {
//         printf("Error validating IR: %s\n", errMsg);
//         LLVMDisposeMessage(errMsg); // Remember to dispose error message
//         exit(EXIT_FAILURE);
//     }
    
//     if (LLVMTargetMachineEmitToFile(targetMachine, module, "output.o", LLVMObjectFile, &error)) {
//         printf("Error: %s\n", error);
//         LLVMDisposeMessage(error); // Don't forget to dispose of the error message once done.
//         return;
//     }



    
//     LLVMDumpModule(module);

//     // Cleanup
//     LLVMDisposeBuilder(builder);
//     LLVMDisposeModule(module);

//     return 0;
// }

void generateProgram(LLVMBuilderRef builder, cJSON *ast) {
    if(!ast) return;

    char* type = getType(ast);
    if(strcmp(type, "PROGRAM") != 0){
        // TODO: Handle Error
        return;
    }
    generateBlock(builder, cJSON_GetObjectItem(ast,"body"));
}

void generateBlock(LLVMBuilderRef builder, cJSON *block) {
    cJSON* const_dec = cJSON_GetArrayItem(block,0);
    cJSON* var_dec = cJSON_GetArrayItem(block,1);
    cJSON* main = cJSON_GetArrayItem(block, 2);

    generateDecs(builder, cJSON_GetObjectItem(const_dec, "dec"));
    generateDecs(builder, cJSON_GetObjectItem(var_dec, "dec"));

    // Declare the printf function
    LLVMTypeRef printfArgs[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMTypeRef printfType = LLVMFunctionType(i32, printfArgs, 1, 1);
    printfFunc = LLVMAddFunction(module, "printf", printfType);
    LLVMSetFunctionCallConv(printfFunc, LLVMCCallConv);

    // Add the format string
    formatStr = LLVMAddGlobal(module, LLVMArrayType(LLVMInt8Type(), 20), "formatString");
    LLVMSetInitializer(formatStr, LLVMConstString("%d\n", 20, 1));
    LLVMSetGlobalConstant(formatStr, 0);
    LLVMSetLinkage(formatStr, LLVMPrivateLinkage);
    insertSymbol(table,"formatString",formatStr);

    generateMain(builder, cJSON_GetObjectItem(main, "block"));
}

void generateDecs(LLVMBuilderRef builder, cJSON *decs) {
    cJSON* child;
    cJSON_ArrayForEach(child, decs) {
        generateDeclaration(builder, child);
    }
}

void generateDeclaration(LLVMBuilderRef builder, cJSON *dec) {
    if (!dec) return;

    char* type = getType(dec);

    if (strcmp(type, "CONST") == 0) {
        // Here, we are treating constants as immutable memory locations.
        LLVMValueRef constant = LLVMAddGlobal(module, i32, getIdent(dec));
        LLVMValueRef initVal = LLVMConstInt(i32, getVal(dec), 0);
        LLVMSetInitializer(constant, initVal);
        LLVMSetGlobalConstant(constant, 1);
        // Add the constant to a symbol table if needed for future references
        insertSymbol(table, getIdent(dec), constant);
    } else if (strcmp(type, "VAR") == 0) {
        LLVMValueRef variable = LLVMAddGlobal(module, i32, getIdent(dec));
        LLVMValueRef initVal = LLVMConstInt(i32, 0, 0);
        LLVMSetInitializer(variable, initVal);
        // Add the variable to a symbol table if needed for future references
        insertSymbol(table, getIdent(dec), variable);
    }
}

void generateMain(LLVMBuilderRef builder, cJSON *block) {
    LLVMTypeRef returnType = LLVMVoidType();
    LLVMTypeRef funcType = LLVMFunctionType(returnType, NULL, 0, 0);
    mainFunc = LLVMAddFunction(module, "main", funcType);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(mainFunc, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    cJSON* child;
    cJSON_ArrayForEach(child, block) {
        generateStatement(builder, child);
    }
    LLVMBuildRetVoid(builder);
}

void generateStatement(LLVMBuilderRef builder, cJSON *stmt) {
    if (!stmt) return;

    char* type = getType(stmt);

    if (strcmp(type, "ASSIGN") == 0) {
        LLVMValueRef target = lookupSymbol(table,getIdent(stmt));
        if(!target){
            // TODO: Handle non existant variable
            exit(1);
        }
        LLVMValueRef result = generateExpr(builder, getExpr(stmt));
        LLVMBuildStore(builder, result, target);
    } else if(strcmp(type, "IF") == 0) {
        LLVMBasicBlockRef thenBlock, elseBlock, mergeBlock;
        cJSON* condition = cJSON_GetObjectItem(stmt, "condition");
        cJSON* then = cJSON_GetObjectItem(stmt, "stmt");
        cJSON* notThen = cJSON_GetObjectItem(stmt, "else");
        LLVMValueRef result_condition = generateCondition(builder, condition);
        thenBlock = LLVMAppendBasicBlock( mainFunc, "then");
        if(notThen != NULL)
            elseBlock = LLVMAppendBasicBlock( mainFunc, "else"); // if there's an else branch
        mergeBlock = LLVMAppendBasicBlock( mainFunc, "merge");
        LLVMBuildCondBr(builder, result_condition, thenBlock, mergeBlock);
        // Populate 'then'
        LLVMPositionBuilderAtEnd(builder, thenBlock);
        generateStatement(builder,then);
        LLVMBuildBr(builder, mergeBlock);
        LLVMPositionBuilderAtEnd(builder, mergeBlock);
    } else if(strcmp(type, "PRINT") == 0) {
        generatePrint(builder,getExpr(stmt));
    }
}

LLVMValueRef generateCondition(LLVMBuilderRef builder, cJSON *condition) {
    if (!condition) return NULL;
    char* type = getType(condition);
    LLVMValueRef left = generateExpr(builder, getLeft(condition));
    LLVMValueRef right = generateExpr(builder, getRight(condition));

    if (getRelOp(condition) == EQU) {
        return LLVMBuildICmp(builder, LLVMIntEQ, left, right, "ifcond");
    } else if (getRelOp(condition) == NEQU) {
        return LLVMBuildICmp(builder, LLVMIntNE, left, right, "ifcond");
    }
    return NULL;
}

LLVMValueRef generateExpr(LLVMBuilderRef builder, cJSON *expr) {
    if (!expr) return NULL;

    char* type = getType(expr);

    if (strcmp(type, "INT") == 0) {
        return LLVMConstInt(LLVMInt32Type(), getVal(expr), 0);
    } else if (strcmp(type, "VAR") == 0) {
        // Assuming variables are stored in some table or map
        char name[10];
        sprintf(name, "%s_val", getIdent(expr));
        return LLVMBuildLoad2(builder, LLVMInt32Type(),lookupSymbol(table,getIdent(expr)), name);
        //return NULL; // Placeholder
    } else if (strcmp(type, "BIN_EXPR") == 0) {
        LLVMValueRef left = generateExpr(builder, getLeft(expr));
        LLVMValueRef right = generateExpr(builder, getRight(expr));
        if (getOp(expr) == PLUS) {
            return LLVMBuildAdd(builder, left, right, "addtmp");
        } else if (getOp(expr) == MULT) {
            return LLVMBuildMul(builder, left, right, "multmp");
        }
    }
    return NULL;
}



void generatePrint(LLVMBuilderRef builder, cJSON *expr) {
    if (!expr) return;

    // Load
    LLVMValueRef valueToPrint = generateExpr(builder,expr);
    
    // Call printf
    LLVMValueRef indices[] = { LLVMConstInt(i32, 0, 0), LLVMConstInt(i32, 0, 0) };
    LLVMTypeRef i8PtrType = LLVMPointerType(LLVMInt8Type(), 0);
    LLVMValueRef formatStrPtr = LLVMBuildGEP(builder, formatStr, indices, 2, "formatStringPtr");
    LLVMValueRef printfArgsVals[] = { formatStrPtr, valueToPrint };
    LLVMBuildCall(builder, printfFunc, printfArgsVals, 2, "");

}

SymbolTable* createSymbolTable() {
    SymbolTable* st = malloc(sizeof(SymbolTable));
    for(int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        st->table[i] = NULL;
    }
    return st;
}

unsigned int hash(char* name) {
    unsigned int hash = 0;
    while(*name) {
        hash = (hash << 5) + *name++;
    }
    return hash % SYMBOL_TABLE_SIZE;
}

void insertSymbol(SymbolTable* st, char* name, LLVMValueRef value) {
    unsigned int index = hash(name);

    // Allocate the new symbol
    Symbol* newSymbol = malloc(sizeof(Symbol));
    newSymbol->name = strdup(name);
    newSymbol->value = value;
    newSymbol->next = NULL;

    // Insert in the table
    if (!st->table[index]) {
        st->table[index] = newSymbol;
    } else {
        // Handle collision
        Symbol* current = st->table[index];
        while (current->next) {
            current = current->next;
        }
        current->next = newSymbol;
    }
}

LLVMValueRef lookupSymbol(SymbolTable* st, char* name) {
    unsigned int index = hash(name);
    Symbol* current = st->table[index];
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->value;
        }
        current = current->next;
    }
    
    return NULL;  // Not found
}

void freeSymbolTable(SymbolTable* st) {
    for(int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        Symbol* current = st->table[i];
        while (current) {
            Symbol* temp = current;
            current = current->next;
            free(temp->name);
            free(temp);
        }
    }
    free(st);
}


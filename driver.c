#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "weaklang.tab.h"
#include "LLVMCodegen.h"

extern FILE* yyin;

cJSON* ast;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    FILE* cout = fopen("ir.json","w");

    yyin = file;

    ast = cJSON_CreateObject();

    // Parse input
    if (yyparse() != 0) {
        fprintf(stderr, "Parsing failed!\n");
        return 1;
    }

    char* jsonStr = cJSON_Print(ast);
    fprintf(cout,"%s", jsonStr);
    fclose(cout);

    codegen();

    cJSON_Delete(ast);

    fclose(file);

    printf("\n");

    return 0;
}


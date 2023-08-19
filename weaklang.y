%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "LLVMCodegen.h"

cJSON* jsonRoot;



// Declaration of external functions and variables
extern int yylex();
extern int yylineno;
extern FILE* yyin;
void yyerror(const char *s);
// void parse(char* file_path);


%}

// Tokens declaration
%token CONST VAR ASSIGN EQU NEQU LES LEQ GRT GRQ PLUS MINUS MULT DIV LP RP LB RB IF THEN WHILE DO COMMA SEMICOLON READ PRINT DEF MAIN

// This allows error recovery (more on this later if desired)
%define parse.error verbose

%union {
    int intValue;
    char* strValue;
    struct cJSON* nodeValue;
    // ... other types ...
}

%token<strValue> IDENT
%token<intValue> INT 
%type<nodeValue> program block const_dec const_list const var_dec var_list var main_dec condition statements statement expression factor term
%type<intValue> rel-op

%%

program: 
    block 
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "PROGRAM");
        cJSON_AddItemToObject(curr_node, "body", $1);
        ast = curr_node; 
    }
    ;

block: 
    const_dec var_dec main_dec
    {
        cJSON* list_node;
        list_node = cJSON_CreateArray();
        cJSON_AddItemToArray(list_node, $1);
        cJSON_AddItemToArray(list_node, $2);
        cJSON_AddItemToArray(list_node, $3);
        $$ = list_node;
    } 
    ;

const_dec: 
    CONST const_list SEMICOLON
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "CONST_DEC");
        cJSON_AddItemToObject(curr_node, "dec", $2);
        $$ = curr_node;
    }
    | /* Empty */
    {
        $$ = NULL;
    }
    ;

const_list: 
    const
    {
        cJSON* list_node;
        list_node = cJSON_CreateArray();
        cJSON_AddItemToArray(list_node, $1);
        $$ = list_node;
    }
    | const_list COMMA const
    {
        cJSON* list_node;
        if($1) {
            list_node = $1;
            cJSON_AddItemToArray(list_node, $3);
        } else {
            list_node = cJSON_CreateArray();
            cJSON_AddItemToArray(list_node, $3);
        }
        $$ = list_node;
    }
    ;

const:
    IDENT ASSIGN INT
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "CONST");
        cJSON_AddStringToObject(curr_node, "ident", $1);
        cJSON_AddNumberToObject(curr_node, "value", $3);
        $$ = curr_node;
    }
    ;

var_dec: 
    VAR var_list SEMICOLON
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "VAR_DEC");
        cJSON_AddItemToObject(curr_node, "dec", $2);
        $$ = curr_node;
    }
    | /* Empty */
    {
        $$ = NULL;
    }
    ;

var_list:
    var
    {
        cJSON* list_node;
        list_node = cJSON_CreateArray();
        cJSON_AddItemToArray(list_node, $1);
        $$ = list_node;
    }
    | var_list COMMA var
    {
        cJSON* list_node;
        if($1) {
            list_node = $1;
            cJSON_AddItemToArray(list_node, $3);
        } else {
            list_node = cJSON_CreateArray();
            cJSON_AddItemToArray(list_node, $3);
        }
        $$ = list_node;
    }
    ;

var:
    IDENT
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "VAR");
        cJSON_AddStringToObject(curr_node, "ident", $1);
        $$ = curr_node;
    }
    ;

main_dec:
    DEF MAIN LB statements RB
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "MAIN");
        cJSON_AddItemToObject(curr_node, "block", $4);
        $$ = curr_node;
    }
    | /* empty */
    {
        $$ = NULL;
    }
    ;

statements:
    statement
    {
        cJSON* list_node;
        list_node = cJSON_CreateArray();
        cJSON_AddItemToArray(list_node, $1);
        $$ = list_node;
    }
    | statements statement
    {
        cJSON* list_node;
        if($1) {
            list_node = $1;
            cJSON_AddItemToArray(list_node, $2);
        } else {
            list_node = cJSON_CreateArray();
            cJSON_AddItemToArray(list_node, $2);
        }
        $$ = list_node;
    }
    ;

statement: 
    IDENT ASSIGN expression SEMICOLON
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "ASSIGN");
        cJSON_AddStringToObject(curr_node, "ident", $1);
        cJSON_AddItemToObject(curr_node, "expr", $3);
        $$ = curr_node;
    }
    | LB statements RB
    {
        $$ = $2;
    }
    | IF condition THEN statement
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "IF");
        cJSON_AddItemToObject(curr_node, "condition", $2);
        cJSON_AddItemToObject(curr_node, "stmt", $4);
        $$ = curr_node;
    }
    | WHILE condition DO statement
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "WHILE");
        cJSON_AddItemToObject(curr_node, "condition", $2);
        cJSON_AddItemToObject(curr_node, "stmt", $4);
        $$ = curr_node;
    }
    | READ LP IDENT RP SEMICOLON
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "READ");
        cJSON_AddStringToObject(curr_node, "ident", $3);
        $$ = curr_node;
    }
    | PRINT LP expression RP SEMICOLON
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "PRINT");
        cJSON_AddItemToObject(curr_node, "expr", $3);
        $$ = curr_node;
    }
    | error SEMICOLON   // Skip erroneous statements until a semicolon
    {
        $$ = NULL; // or appropriate value indicating an error
    }
    ;

condition:
    expression rel-op expression
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "CONDITION");
        cJSON_AddNumberToObject(curr_node, "rel-op", $2);
        cJSON_AddItemToObject(curr_node, "left", $1);
        cJSON_AddItemToObject(curr_node, "right", $3);
        $$ = curr_node;
    }
    ;

rel-op:
    EQU
    {
        $$ = EQU;
    }
    | NEQU
    {
        $$ = NEQU;
    }
    | LES
    {
        $$ = LES;
    }
    | LEQ
    {
        $$ = LEQ;
    }
    | GRT
    {
        $$ = GRT;
    }
    | GRQ
    {
        $$ = GRQ;
    }
    ;

expression:
    term
    {
        $$ = $1;
    }
    | expression PLUS term
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "BIN_EXPR");
        cJSON_AddNumberToObject(curr_node, "op", PLUS);
        //cJSON_AddStringToObject(curr_node, "op", "PLUS");
        cJSON_AddItemToObject(curr_node, "left", $1);
        cJSON_AddItemToObject(curr_node, "right", $3);
        $$ = curr_node;
    }
    | expression MINUS term
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "BIN_EXPR");
        cJSON_AddNumberToObject(curr_node, "op", MINUS);
        //cJSON_AddStringToObject(curr_node, "op", "MINUS");
        cJSON_AddItemToObject(curr_node, "left", $1);
        cJSON_AddItemToObject(curr_node, "right", $3);
        $$ = curr_node;
    }
    ;

term:
    factor
    {
        $$ = $1;
    }
    | term MULT factor
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "BIN_EXPR");
        cJSON_AddNumberToObject(curr_node, "op", MULT);
        //cJSON_AddStringToObject(curr_node, "op", "MULT");
        cJSON_AddItemToObject(curr_node, "left", $1);
        cJSON_AddItemToObject(curr_node, "right", $3);
        $$ = curr_node;
    }
    | term DIV factor
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "BIN_EXPR");
        cJSON_AddNumberToObject(curr_node, "op", DIV);
        //cJSON_AddStringToObject(curr_node, "op", "MULT");
        cJSON_AddItemToObject(curr_node, "left", $1);
        cJSON_AddItemToObject(curr_node, "right", $3);
        $$ = curr_node;
    }
    ;

factor:
    IDENT
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "VAR");
        cJSON_AddStringToObject(curr_node, "ident", $1);
        $$ = curr_node;
    }
    | INT
    {
        cJSON* curr_node = cJSON_CreateObject();
        cJSON_AddStringToObject(curr_node, "type", "INT");
        cJSON_AddNumberToObject(curr_node, "value", $1);
        $$ = curr_node;
    }
    | LP expression RP
    {
        $$ = $2;
    }
    ;


/* procedure_dec:
    DEF IDENT LB block RB 
    | 
    ; */



%%

void yyerror(const char *s) {
    fprintf(stderr, "Error at line %d: %s\n", yylineno,s);
}


/* void parse(char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    cout = fopen("ir.json","w");

    yyin = file;

    ast = cJSON_CreateObject();

    // Parse input
    if (yyparse() != 0) {
        fprintf(stderr, "Parsing failed!\n");
        return 1;
    }

    char* jsonStr = cJSON_Print(jsonRoot);
    fprintf(cout,"%s", jsonStr);

    fclose(cout);

    fclose(file);

    printf("\n");
} */
/* 
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

    cout = fopen("ir.json","w");

    yyin = file;

    jsonRoot = cJSON_CreateObject();


    // Parse input
    if (yyparse() != 0) {
        fprintf(stderr, "Parsing failed!\n");
        return 1;
    }


    char* jsonStr = cJSON_Print(jsonRoot);
    fprintf(cout,"%s", jsonStr);

    cJSON_Delete(jsonRoot);

    fclose(cout);

    fclose(file);

    printf("\n");

    return 0;
} */




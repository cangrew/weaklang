%{
/* C declarations */
#include <stdio.h>
#include <string.h>
#include "weaklang.tab.h"
void showError();
%}

%option yylineno

/* Regular expression patterns */
DIGIT       [0-9]
LETTER      [a-zA-Z]
INTEGER     {DIGIT}+
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
WS      [ \t\n\r]

%%

"def"                   {
    printf("DEF = %s ", yytext);
    return DEF;}
"main"                  {
    printf("MAIN = %s ", yytext);
    return MAIN;}
"const"                 {
    printf("CONST = %s ", yytext);
    return CONST;}
"var"                   {
    printf("VAR = %s ", yytext);
    return VAR;}
"read"                  {
    printf("READ = %s ", yytext);
    return READ;}
"print"                 {
    printf("PRINT = %s ", yytext);
    return PRINT;}
"if"                    {
    printf("IF = %s ", yytext);
    return IF;}
"then"                   {
    printf("THEN = %s ", yytext);
    return THEN;}
"while"                 {
    printf("WHILE = %s ", yytext);
    return WHILE;}
"do"                    {
    printf("DO = %s ", yytext);
    return DO;}


"=="                    {
    printf("EQU = %s ", yytext);
    return EQU;}
"!="                    {
    printf("NEQU = %s ", yytext);
    return NEQU;}
"<="                    {
    printf("LEQ = %s ", yytext);
    return LEQ;}
"<"                    {
    printf("LES = %s ", yytext);
    return LES;}
">="                    {
    printf("GRQ = %s ", yytext);
    return GRQ;}
">"                     {
    printf("GRT = %s ", yytext);
    return GRT;}
"="                     {
    printf("ASSIGN = %s ", yytext);
    return ASSIGN;}

","                     {
    printf("COMMA = %s ", yytext);
    return COMMA;}
";"                     {
    printf("SEMICOLON = %s ", yytext);
    return SEMICOLON;}
"("                     {
    printf("LP = %s ", yytext);
    return LP;}
")"                     {
    printf("RP = %s ", yytext);
    return RP;}
"{"                     {
    printf("LB = %s ", yytext);
    return LB;}
"}"                     {
    printf("RB = %s ", yytext);
    return RB;}
"+"                     {
    printf("PLUS = %s ", yytext);
    return PLUS;}
"-"                     {
    printf("MINUS = %s ", yytext);
    return MINUS;}
"*"                     {
    printf("%s ", yytext);
    return MULT;}
"/"                     {
    printf("%s ", yytext);
    return DIV;}

{INTEGER}       {
    yylval.intValue = atoi(yytext);
    printf("INT = %s ", yytext);
    return INT; 
}

{IDENTIFIER}    {
    yylval.strValue = strdup(yytext);
    printf("IDENT = %s ", yytext);
    return IDENT;
}

{WS}+            /* Ignoring white spaces */

.                { fprintf(stderr, "Error: unrecognized character %s\n", yytext); exit(1); }

%%

void showError(){
    
}




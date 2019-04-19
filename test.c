#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "ast.h"
#include "evaluator.h"

int main(int argc, char **argv, char **envp) {
    char expr[100];
    int err = 0;

    Lex *lex;
    AST *ast;

    fgets(expr, 100, stdin);

    printf("\n[+] Tokenizing expression... ");

    lex = tokenize(expr, strlen(expr), &err);

    if (err < 0) {
        printf("\n[!] Error: %d\n", err);
        return -err;
    }

    printf("DONE.\n");
    printf("\n[i] Tokenized expression:\n\n");

    dump_lex(lex);

    printf("\n[+] Building AST... ");

    ast = build_ast(lex);

    printf("DONE.\n");

    printf("\n[i] Abstract Syntax Tree:\n");

    dump_ast(ast);

    printf("\n[+] Evaluating AST... ");

    ast = derive_ast(ast);

    printf("DONE.\n");
    printf("\n[i] Derived AST:\n");

    dump_ast(ast);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "ast.h"
#include "evaluator.h"
#include "strbuf.h"

int main(int argc, char **argv, char **envp) {
    /*char expr[100];*/
    int err = 0;

    Lex *lex, *der_lex;
    AST *ast;

    StringBuffer *input_buffer = create_str_buffer();

    /*fgets(expr, 100, stdin);*/
    get_string_input(input_buffer);

    printf("\n[+] Tokenizing expression... ");

    /*lex = tokenize(expr, strlen(expr), &err);*/
    lex = tokenize(input_buffer->str, input_buffer->last_char_indx, &err);

    if (err < 0) {
        printf("\n[!] Error: %d\n", err);
        return -err;
    }

    printf("DONE.\n");
    printf("\n[i] Tokenized expression:\n\n");

    dump_lex(lex);

    printf("\n[+] Building AST... ");

    ast = build_ast(lex);
    simplify_node(ast->root, 0);

    printf("DONE.\n");

    printf("\n[i] Abstract Syntax Tree:\n");

    dump_ast(ast);

    printf("\n[+] Evaluating AST... ");

    ast = derive_ast(ast);

    printf("DONE.\n");
    printf("\n[i] Derived AST:\n");

    dump_ast(ast);

    simplify_node(ast->root, 0);

    printf("\nAfter evaluation:\n");

    dump_ast(ast);

    der_lex = decompose_ast(ast);

    printf("\nDecomposed AST:\n\n");

    dump_lex(der_lex);

    printf("\nDetokenized expression: %s\n", detokenize(der_lex)->str);

    return 0;
}
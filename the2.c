#include <stdio.h>

#include "ast.h"
#include "lexer.h"
#include "evaluator.h"
#include "strbuf.h"

int main(int argc, char **argv, char **envp) {
    int err = 0;

    Lex *lex;
    AST *ast;
    StringBuffer *buf = create_str_buffer();

    get_string_input(buf);

    lex = tokenize(buf->str, buf->last_char_indx, &err);

    if (err < 0)
        return 1;

    ast = build_ast(lex);
    simplify_node(ast->root, 0);

    ast = derive_ast(ast);
    simplify_node(ast->root, 0);

    lex = decompose_ast(ast);
    buf = detokenize(lex);

    printf("%s\n", buf->str);

    return 0;
}
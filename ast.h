#if !defined(AST_H)
#define AST_H

#include "lexer.h"

typedef int aststacksize;

typedef struct _ASTNode {
    TOKEN tok;
    TOKEN_TYPE ttype;
    tnodevalue value;
    struct _ASTNode *left;
    struct _ASTNode *right;
} ASTNode;

typedef struct _AST {
    tnodesize node_count;
    tnodesize tree_depth;
    ASTNode *root;
} AST;

typedef struct _ASTStackNode {
    ASTNode *node;
    struct _ASTStackNode *next;
} ASTStackNode;

typedef struct _ASTStack {
    ASTStackNode *root;
    ASTStackNode *__last_node;
    aststacksize size;
} ASTStack;

ASTNode *create_ast_node(TOKEN tok, tnodevalue val);
ASTNode *duplicate_ast_node(ASTNode *node);
ASTStack *create_ast_stack(void);
aststacksize push_ast_stack(ASTStack *stack, ASTNode *node);
ASTNode *pop_ast_stack(ASTStack *stack);
ASTNode *peek_ast_stack(ASTStack *stack);
aststacksize add_stack_node(ASTStack *stack, ASTNode *node);
AST *initiate_ast_tree(void);
ASTNode *parse_lex(Lex *lex);
AST *build_ast(Lex *lex);
void _dump_ast_node(ASTNode *node, tnodesize depth);
void dump_ast(AST *ast);

#endif

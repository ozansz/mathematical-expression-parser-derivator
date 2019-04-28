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

int is_zero_node(ASTNode *node);
int is_one_node(ASTNode *node);

void simplify_node(ASTNode *node, int depth);
void eval_node_numeric(ASTNode *node);

ASTNode *create_ast_node(TOKEN tok, tnodevalue val);
ASTNode *duplicate_ast_node(ASTNode *node);
void convert_ast_node(ASTNode *node, TOKEN new_tok, tnodevalue value, ASTNode *left, ASTNode *right);
void convert_ast_node_from_node(ASTNode *to, ASTNode *from);
ASTStack *create_ast_stack(void);
aststacksize push_ast_stack(ASTStack *stack, ASTNode *node);
ASTNode *pop_ast_stack(ASTStack *stack);
ASTNode *peek_ast_stack(ASTStack *stack);
aststacksize add_stack_node(ASTStack *stack, ASTNode *node);
AST *initiate_ast_tree(void);
ASTNode *parse_lex(Lex *lex);
AST *build_ast(Lex *lex);

void decompose_ast_node(ASTNode *node, Lex *lex);
Lex *decompose_ast(AST *ast);

void _dump_ast_node(ASTNode *node, tnodesize depth);
void dump_ast(AST *ast);

void free_ast_node(ASTNode *node);

void __ddebug(char *str, int depth);

#endif

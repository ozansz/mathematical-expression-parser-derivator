#if !defined(EVALUATOR_H)
#define EVALUATOR_H

#include "ast.h"

AST *derive_ast(AST *ast);
int derive_node(ASTNode *node);
int derive_func(ASTNode *node);
int derive_op(ASTNode *node);
int _derive_tangent(ASTNode *node);
/*int derive_cosine(ASTNode *node);*/
int _derive_log(ASTNode *node);
int _derive_op_mul(ASTNode *node);
int _derive_op_div(ASTNode *node);
int _derive_op_pow(ASTNode *node);

#endif
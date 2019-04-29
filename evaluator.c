#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "ast.h"
#include "evaluator.h"

AST *derive_ast(AST *ast) {
    AST *ret = initiate_ast_tree();

    if (ret == NULL)
        return NULL; 

    ret->node_count = -1;
    ret->tree_depth = -1;
    ret->root = duplicate_ast_node(ast->root);

    derive_node(ret->root);
    
    return ret;
}

int derive_node(ASTNode *node) {
    int err_ret = 0;

    switch (node->ttype) {
        case TT_LITERAL:
            convert_ast_node(node, NUM, 0, NULL, NULL);
            break;
        case TT_VARIABLE:
            convert_ast_node(node, NUM, 1, NULL, NULL);
            break;
        case TT_FUNCTION_NOARG:
            err_ret = derive_func(node);
            break;
        case TT_OPERATOR:
            err_ret = derive_op(node);
            break;
        default:
            break;
    }

    return err_ret;
}

int derive_func(ASTNode *node) {
    int err_ret = 0;

    switch(node->tok) {
        case F_SIN:
            node->tok = F_COS;
            break;
        case F_SH:
            node->tok = F_CH;
            break;
        case F_CH:
            node->tok = F_SH;
            break;
        /*case F_COS:
            derive_cosine(node);
            break;*/
        case F_TAN:
            err_ret = _derive_tangent(node);
            break;
        case F_LN:
            err_ret = _derive_log(node);
            break;
        default:
            break;
    }

    return err_ret;
}

int _derive_tangent(ASTNode *node) {
    int err_ret = 0;    
    ASTNode *tannode, *pownode, *twonode, *onenode;

    tannode = duplicate_ast_node(node);
    pownode = create_ast_node(OP_POW, 0);
    twonode = create_ast_node(NUM, 2);
    onenode = create_ast_node(NUM, 1);

    pownode->right = twonode;
    pownode->left = tannode;

    convert_ast_node(node, OP_ADD, 0, pownode, onenode);

    return err_ret;
}

/*int derive_cosine(ASTNode *node) {

}*/

int _derive_log(ASTNode *node) {
    int err_ret = 0;
    ASTNode *onenode, *varnode;

    onenode = create_ast_node(NUM, 1);
    varnode = create_ast_node(VAR_X, 0);

    convert_ast_node(node, OP_DIV, 0, onenode, varnode);

    return err_ret;
}

int derive_op(ASTNode *node) {
    int err_ret = 0;

    switch(node->tok) {
        case OP_ADD:
        case OP_SUB:
            derive_node(node->right);
            derive_node(node->left);
            break;
        case OP_MUL:
            _derive_op_mul(node);
            break;
        case OP_DIV:
            _derive_op_div(node);
            break;
        case OP_POW:
            _derive_op_pow(node);
            break;
        default:
            break;
    }

    return err_ret;
}

int _derive_op_mul(ASTNode *node) {
    int err_ret = 0;
    ASTNode *mulnode_left, *mulnode_right, *rdup, *ldup, *rder, *lder;

    mulnode_right = create_ast_node(OP_MUL, 0);
    mulnode_left = create_ast_node(OP_MUL, 0);

    rdup = duplicate_ast_node(node->right);
    ldup = duplicate_ast_node(node->left);
    rder = duplicate_ast_node(node->right);
    lder = duplicate_ast_node(node->left);

    derive_node(rder);
    derive_node(lder);

    mulnode_left->left = lder;
    mulnode_left->right = rdup;
    mulnode_right->left = ldup;
    mulnode_right->right = rder;

    convert_ast_node(node, OP_ADD, 0, mulnode_left, mulnode_right);

    return err_ret;
}

int _derive_op_div(ASTNode *node) {
    int err_ret = 0;
    ASTNode *subnode, *pownode, *mulnode_left, *mulnode_right, *twonode, *ldup, *rdup2, *rdup, *lder, *rder;

    subnode = create_ast_node(OP_SUB, 0);
    pownode = create_ast_node(OP_POW, 0);
    mulnode_left = create_ast_node(OP_MUL, 0);
    mulnode_right = create_ast_node(OP_MUL, 0);
    twonode = create_ast_node(NUM, 2);

    rdup = duplicate_ast_node(node->right);
    rdup2= duplicate_ast_node(node->right);
    ldup = duplicate_ast_node(node->left);
    rder = duplicate_ast_node(node->right);
    lder = duplicate_ast_node(node->left);

    derive_node(rder);
    derive_node(lder);

    mulnode_left->left = lder;
    mulnode_left->right = rdup;
    mulnode_right->left = ldup;
    mulnode_right->right = rder;

    subnode->left = mulnode_left;
    subnode->right = mulnode_right;

    pownode->left = rdup2;
    pownode->right = twonode;

    convert_ast_node(node, OP_DIV, 0, subnode, pownode);

    return err_ret;
}

int _derive_op_pow(ASTNode *node) {
    int err_ret = 0;
    ASTNode *mulnode, *fder, *pownode, *fdup;

    if (node->right->tok != NUM)
        return 1;

    mulnode = create_ast_node(OP_MUL, 0);
    pownode = create_ast_node(OP_POW, 0);
    fdup = duplicate_ast_node(node->left);
    fder = duplicate_ast_node(node->left);

    derive_node(fder);

    pownode->left = fdup;
    pownode->right = create_ast_node(NUM, node->right->value - 1);
    mulnode->left = create_ast_node(NUM, node->right->value);
    mulnode->right = pownode;
    
    convert_ast_node(node, OP_MUL, 0, mulnode, fder);

    return err_ret;
}
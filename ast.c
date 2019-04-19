#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "lexer.h"

ASTNode *create_ast_node(TOKEN tok, tnodevalue val) {
    ASTNode *node = (ASTNode *) malloc(sizeof(ASTNode));

    if (node == NULL)
        return NULL;

    node->tok = tok;
    node->ttype = get_token_type(tok);
    node->value = val;
    node->left = NULL;
    node->right = NULL;

    return node;
}

ASTNode *duplicate_ast_node(ASTNode *node) {
    ASTNode *dup = create_ast_node(node->tok, node->value);

    dup->ttype = node->ttype;
    dup->right = node->right;
    dup->left = node->left;
    
    return dup;
}

ASTStack *create_ast_stack(void) {
    ASTStack *stack = (ASTStack *) malloc(sizeof(ASTStack));

    if (stack == NULL)
        return NULL;

    stack->root = NULL;
    stack->__last_node = NULL;
    stack->size = 0;

    return stack;
}

aststacksize push_ast_stack(ASTStack *stack, ASTNode *node) {
    ASTStackNode *stnode = (ASTStackNode *) malloc(sizeof(ASTStackNode));

    if (stnode == NULL)
        return -1;

    stnode->node = node;
    stnode->next = NULL;

    if (stack->size == 0) {
        stack->root = stnode;
        stack->__last_node = stnode;
    } else {
        stack->__last_node->next = stnode;
        stack->__last_node = stnode;
    }

    stack->size++;

    return stack->size;
}

ASTNode *pop_ast_stack(ASTStack *stack) {
    ASTStackNode *ret, *rem;

    if (stack->size == 0)
        return NULL;

    if (stack->size == 1) {
        ret = stack->__last_node;

        stack->root = NULL;
        stack->__last_node = NULL;
        stack->size = 0;
        
        return ret->node;
    }

    ret = stack->__last_node;
    rem = stack->root;

    while (rem->next != stack->__last_node)
        rem = rem->next;

    rem->next = NULL;
    stack->__last_node = rem;
    stack->size--;

    return ret->node;
}

ASTNode *peek_ast_stack(ASTStack *stack) {
    if (stack->size == 0)
        return NULL;

    return stack->__last_node->node;
}

aststacksize add_stack_node(ASTStack *stack, ASTNode *node) {
    ASTNode *right = pop_ast_stack(stack);
    ASTNode *left = pop_ast_stack(stack);

    node->right = right;
    node->left = left;

    return push_ast_stack(stack, node);
}

AST *initiate_ast_tree(void) {
    AST *ast = (AST *) malloc(sizeof(AST));
    ASTNode *root_node = create_ast_node(__INIT, 0);

    if (ast == NULL || root_node == NULL)
        return NULL;

    ast->root = root_node;
    ast->node_count = 1;
    ast->tree_depth = 0;

    return ast;
}

ASTNode *parse_lex(Lex *lex) {
    ASTStack *opStack = create_ast_stack();
    ASTStack *outStack = create_ast_stack();
    ASTNode *_peek;
    TokenNode *curr_node;

    while ((curr_node = lex_iternode(lex)) != NULL) {
        if (curr_node->tok == __INIT)
            continue;
        else if (curr_node->ttype == TT_LITERAL || curr_node->ttype == TT_VARIABLE || curr_node->ttype == TT_FUNCTION_NOARG)
            push_ast_stack(outStack, create_ast_node(curr_node->tok, curr_node->value));
        else if (curr_node->ttype == TT_OPERATOR) {
            while ((_peek = peek_ast_stack(opStack)) != NULL && (_peek->ttype == TT_OPERATOR) 
				&& ((get_operator_assoc(curr_node->tok) == OP_ASSOC_LEFT && get_operator_prec(curr_node->tok) <= get_operator_prec(_peek->tok))
					|| (get_operator_assoc(curr_node->tok) == OP_ASSOC_RIGHT && get_operator_prec(curr_node->tok) < get_operator_prec(_peek->tok))))
                        add_stack_node(outStack, pop_ast_stack(opStack));

            push_ast_stack(opStack, create_ast_node(curr_node->tok, curr_node->value));

        } else if (curr_node->ttype == TT_PAREN) {
            if (curr_node->tok == LPAREN) {
                push_ast_stack(opStack, create_ast_node(curr_node->tok, curr_node->value));
            } else if (curr_node->tok == RPAREN) {
                while ((_peek = peek_ast_stack(opStack)) != NULL && _peek->tok != LPAREN)
                    add_stack_node(outStack, pop_ast_stack(opStack));

                pop_ast_stack(opStack);
            }
        }
    }

    while ((_peek = peek_ast_stack(opStack)) != NULL)
        add_stack_node(outStack, pop_ast_stack(opStack));

    return pop_ast_stack(outStack);
}

AST *build_ast(Lex *lex) {
    AST *ast = initiate_ast_tree();

    ast->root = parse_lex(lex);
    ast->node_count = -1;
    ast->tree_depth = -1;

    return ast;
}

void _dump_ast_node(ASTNode *node, tnodesize depth) {
    int i;

    for (i = 0; i < depth; i++)
        printf("    ");

    printf("%s\n", get_token_repr((TokenNode *)node));

    if (node->left != NULL)
        _dump_ast_node(node->left, depth+1);
    if (node->right != NULL)
        _dump_ast_node(node->right, depth+1);
}

void dump_ast(AST *ast) {
    printf("\n");
    _dump_ast_node(ast->root, 0);
}
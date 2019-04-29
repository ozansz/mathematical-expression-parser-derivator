#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ast.h"
#include "lexer.h"

void __ddebug(char *str, int depth) {
#ifdef  USE_DDEBUG
    printf("\n");

    while (depth--)
        printf("    ");
    
    printf(str);
#else
    return;
#endif
}

int is_zero_node(ASTNode *node) {
    if (node == NULL)
        return -1;
    else if (node->tok == NUM && node->value == 0)
        return 1;
    else
        return 0;
}

int is_one_node(ASTNode *node) {
    if (node == NULL)
        return -1;
    else if (node->tok == NUM && node->value == 1)
        return 1;
    else
        return 0;
}

void eval_node_numeric(ASTNode *node) {
    int zero_left, zero_right, one_left, one_right;

    if (node == NULL || node->ttype != TT_OPERATOR)
        return;

    if (node->left->tok != NUM || node->right->tok != NUM)
        return;

    switch (node->tok) {
        case OP_ADD:
            convert_ast_node(node, NUM, node->left->value + node->right->value, NULL, NULL);
            break;
        case OP_SUB:
            if (node->left->value >= node->right->value)
                convert_ast_node(node, NUM, node->left->value - node->right->value, NULL, NULL);
            break;
        case OP_MUL:
            convert_ast_node(node, NUM, node->left->value * node->right->value, NULL, NULL);
            break;
        case OP_DIV:
            if (node->left->value % node->right->value == 0)
                convert_ast_node(node, NUM, node->left->value / node->right->value, NULL, NULL);
            break;
        case OP_POW:
            zero_left = is_zero_node(node->left);
            zero_right = is_zero_node(node->right);
            one_left = is_one_node(node->left);
            one_right = is_one_node(node->right);

            if (zero_left && !zero_right)
                convert_ast_node(node, NUM, 0, NULL, NULL);
            else if ((!zero_left && zero_right) || one_left)
                convert_ast_node(node, NUM, 1, NULL, NULL);
            else if (one_right)
                convert_ast_node_from_node(node, node->left);
            else
                convert_ast_node(node, NUM, pow(node->left->value, node->right->value), NULL, NULL);
            break;
        default:
            break;
    }
}

void simplify_node(ASTNode *node, int depth) {
    char buf[80];
    int zero_left, zero_right, one_left, one_right;

    snprintf(buf, 80, "CALL simplify_node at %p <TOKEN: %d, VALUE: %d>", (void *)node, node->tok, node->value);
    __ddebug(buf, depth);

    if (node == NULL) {
        __ddebug("node is NULL\n", depth);
        return;
    }

    if (node->ttype != TT_OPERATOR) {
        __ddebug("node is not type of TT_OPERATOR\n", depth);
        return;
    }

    __ddebug("making recursive calls\n", depth);
    simplify_node(node->left, depth+1);
    simplify_node(node->right, depth+1);

    zero_left = is_zero_node(node->left);
    zero_right = is_zero_node(node->right);
    one_left = is_one_node(node->left);
    one_right = is_one_node(node->right);

    snprintf(buf, 80, "zl: %d, zr: %d, ol: %d, or: %d", zero_left, zero_right, one_left, one_right);
    __ddebug(buf, depth);

    if (node->left->tok == NUM && node->right->tok == NUM) {
        __ddebug("eval node numeric is calling", depth);
        snprintf(buf, 80, "RET simplify_node at %p <TOKEN: %d, VALUE: %d>\n", (void *)node, node->tok, node->value);
        __ddebug(buf, depth);
        eval_node_numeric(node);
        return;
    }

    switch (node->tok) {
        case OP_ADD:
            __ddebug("op type <+>", depth);
            if (zero_left)
                convert_ast_node_from_node(node, node->right);
            else if (zero_right)
                convert_ast_node_from_node(node, node->left);
            break;
        case OP_SUB:
            __ddebug("op type <->", depth); 
            if (zero_right)
                convert_ast_node_from_node(node, node->left);
            break;
        case OP_MUL:
            __ddebug("op type <*>", depth);
            if (zero_left || zero_right)
                convert_ast_node(node, NUM, 0, NULL, NULL);
            else if (one_left)
                convert_ast_node_from_node(node, node->right);
            else if (one_right)
                convert_ast_node_from_node(node, node->left);
            break;
        case OP_DIV:
            __ddebug("op type </>", depth);
            if (one_right)
                convert_ast_node_from_node(node, node->left);
            else if (zero_left)
                convert_ast_node(node, NUM, 0, NULL, NULL);
            break;
        case OP_POW:
            __ddebug("op type <^>", depth);
            if (zero_left && !zero_right)
                convert_ast_node(node, NUM, 0, NULL, NULL);
            else if ((!zero_left && zero_right) || one_left)
                convert_ast_node(node, NUM, 1, NULL, NULL);
            else if (one_right)
                convert_ast_node_from_node(node, node->left);
            break;
        default:
            __ddebug("[ERROR] Invalid op type catched!", depth);
            snprintf(buf, 80, "[ERROR] node->tok = %d", node->tok);
            __ddebug(buf, depth);
            break;
    }

    snprintf(buf, 80, "RET simplify_node at %p <TOKEN: %d, VALUE: %d>\n", (void *)node, node->tok, node->value);
    __ddebug(buf, depth);
}

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

    if (node->left != NULL)
        dup->right = duplicate_ast_node(node->right);
    
    if (node->right != NULL)
        dup->left = duplicate_ast_node(node->left);
    
    return dup;
}

void convert_ast_node(ASTNode *node, TOKEN new_tok, tnodevalue value, ASTNode *left, ASTNode *right) {
    node->tok = new_tok;
    node->value = value;
    node->ttype = get_token_type(node->tok);
    node->left = left;
    node->right = right;
}

void convert_ast_node_from_node(ASTNode *to, ASTNode *from) {
    convert_ast_node(to, from->tok, from->value, from->left, from->right);
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

void decompose_ast_node(ASTNode *node, Lex *lex) {
    if (node == NULL)
        return;

    if (node->left == NULL || node->right == NULL) {
        add_token_node(lex, node->tok, node->value);
        return;
    }
        
    if (node->left->ttype != TT_OPERATOR && node->right->ttype != TT_OPERATOR) {
        add_token_node(lex, node->left->tok, node->left->value);
        add_token_node(lex, node->tok, node->value);
        add_token_node(lex, node->right->tok, node->right->value);
    } else {
        if (node->left->ttype == TT_OPERATOR) {
            if (get_operator_prec(node->tok) > get_operator_prec(node->left->tok)) {
                add_token_node(lex, LPAREN, 0);
                decompose_ast_node(node->left, lex);
                add_token_node(lex, RPAREN, 0);
            } else
                decompose_ast_node(node->left, lex);
            
        } else
            add_token_node(lex, node->left->tok, node->left->value);

        add_token_node(lex, node->tok, node->value);

        if (node->right->ttype == TT_OPERATOR) {
            if ((get_operator_prec(node->tok) > get_operator_prec(node->right->tok)) || (node->tok == OP_SUB && (node->right->tok == OP_ADD || node->right->tok == OP_SUB))) {
                add_token_node(lex, LPAREN, 0);
                decompose_ast_node(node->right, lex);
                add_token_node(lex, RPAREN, 0);
            } else
                decompose_ast_node(node->right, lex);
            
        } else
            add_token_node(lex, node->right->tok, node->right->value);
    }
}

Lex *decompose_ast(AST *ast) {
    Lex *lex = initiate_token_list(__INIT);

    decompose_ast_node(ast->root, lex);

    return lex;
}

void _dump_ast_node(ASTNode *node, tnodesize depth) {
    int i;

    for (i = 0; i < depth; i++)
        printf("      ");
    
    printf("%s<%d> %p\n", get_token_repr((TokenNode *)node), node->tok, (void *)node);

    if (node->left != NULL)
        _dump_ast_node(node->left, depth+1);
    if (node->right != NULL)
        _dump_ast_node(node->right, depth+1);
}

void dump_ast(AST *ast) {
    printf("\n");
    _dump_ast_node(ast->root, 0);
}

void free_ast_node(ASTNode *node) {
    free(node);
}
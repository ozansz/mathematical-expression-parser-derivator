#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define OP_ASSOC_LEFT   1
#define OP_ASSOC_RIGHT  2

#define DEF_STRBUF_SIZE_CHAR    128
#define STRBUF_EXTEND_SIZE  16

typedef enum {
    NOERR = 0,
    E_INVALID_TOKEN
} LEX_ERRNO;

typedef enum {
    __INIT = 0,
    NUM,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_POW,
    VAR_X,
    F_SIN,
    F_COS,
    F_TAN,
    F_SH,
    F_CH,
    F_LN,
    LPAREN,
    RPAREN
} TOKEN;

typedef int operatorprec;
typedef int operatorassoc;

typedef enum {
    TT_INVALID_TOKEN = -1,
    TT_INIT_MAGIC = 0,
    TT_LITERAL,
    TT_VARIABLE,
    TT_OPERATOR,
    TT_FUNCTION_NOARG,
    TT_PAREN
} TOKEN_TYPE;

typedef int tnodesize;
typedef int tnodevalue;

typedef struct _TokenNode{
    TOKEN tok;
    TOKEN_TYPE ttype;
    tnodevalue value;
    struct _TokenNode *next;
} TokenNode;

typedef struct _Lex {
    TokenNode *token_list;
    tnodesize lex_size;
    TokenNode *__last_node;
} Lex;

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

typedef struct _StringBuffer {
    char *str;
    int size;
    int last_char_indx;
} StringBuffer;

TOKEN_TYPE get_token_type(TOKEN tok);
operatorprec get_operator_prec(TOKEN tok);
operatorassoc get_operator_assoc(TOKEN tok);
Lex *initiate_token_list(TOKEN tok);
tnodesize add_token_node(Lex *lex, TOKEN new_tok, int value);
Lex *tokenize(char *str, tnodesize len, int *errn);
StringBuffer *detokenize(Lex *lex);
TokenNode *lex_iternode(Lex *lex);
char *get_token_repr(TokenNode *node);
void dump_lex(Lex *lex);
void free_lex(Lex *lex);

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

StringBuffer *create_str_buffer(void);
void zero_str_buffer(StringBuffer *buf);
int push_char_to_buffer(StringBuffer *buf, char c);
int push_str_to_buffer(StringBuffer *buf, char *str, int ssize);
int extend_str_buffer(StringBuffer *buf);

int get_string_input(StringBuffer *buf);

int is_number(char ch) {
    if ('0' <= ch && ch <= '9')
        return 1;

    return 0;
}

int is_space(char ch) {
    if (ch == ' ' || ch == '\t' || ch == '\n')
        return 1;

    return 0;
}

int _ch_to_int(char ch) {
    return ch - '0';
}

char _int_to_ch(int i) {
    return i + '0';
}

TOKEN_TYPE get_token_type(TOKEN tok) {
    if (tok == NUM)
        return TT_LITERAL;
    else if (tok == OP_ADD || tok == OP_DIV || tok == OP_MUL || tok == OP_POW || tok == OP_SUB)
        return TT_OPERATOR;
    else if (tok == VAR_X)
        return TT_VARIABLE;
    else if (tok == F_SIN || tok == F_COS || tok == F_TAN || tok == F_SH || tok == F_CH || tok == F_LN)
        return TT_FUNCTION_NOARG;
    else if (tok == LPAREN || tok == RPAREN)
        return TT_PAREN;
    else if (tok == __INIT)
        return TT_INIT_MAGIC;
    else
        return TT_INVALID_TOKEN;
}

operatorprec get_operator_prec(TOKEN tok) {
    if (tok == OP_ADD || tok == OP_SUB)
        return 1;
    else if (tok == OP_MUL || tok == OP_DIV)
        return 2;
    else if (tok == OP_POW)
        return 3;
    else
        return 0;
}

operatorassoc get_operator_assoc(TOKEN tok) {
    if (tok == OP_ADD || tok == OP_SUB)
        return OP_ASSOC_LEFT;
    else if (tok == OP_MUL || tok == OP_DIV)
        return OP_ASSOC_LEFT;
    else if (tok == OP_POW)
        return OP_ASSOC_RIGHT;
    else
        return -1;
}

Lex *initiate_token_list(TOKEN tok) {
    Lex *lex = (Lex *) malloc(sizeof(Lex));
    TokenNode *tnode = (TokenNode *) malloc(sizeof(TokenNode));

    if (lex == NULL || tnode == NULL)
        return NULL;

    tnode->tok = tok;
    tnode->ttype = get_token_type(tok);
    tnode->value = 0;
    tnode->next = NULL;
    lex->token_list = tnode;
    lex->lex_size = 1;
    lex->__last_node = tnode;
    return lex;
}

tnodesize add_token_node(Lex *lex, TOKEN new_tok, int value) {
    TokenNode *tnode = (TokenNode *) malloc(sizeof(TokenNode));

    if (tnode == NULL)
        return -1;

    tnode->tok = new_tok;
    tnode->ttype = get_token_type(new_tok);
    tnode->value = value;
    tnode->next = NULL;
    lex->__last_node->next = tnode;
    lex->__last_node = tnode;
    lex->lex_size++;

    return lex->lex_size;
}

Lex *tokenize(char *str, tnodesize len, int *errn) {
    Lex *_lex = initiate_token_list(__INIT);
    tnodesize pos;
    char curr_char;
    int open_paren_count = 0;

    *errn = NOERR;

    for (pos = 0; pos < len; pos++) {
        curr_char = *(str+pos);

        if (is_space(curr_char))
            continue;

        if (is_number(curr_char)) {
            if (_lex->__last_node->tok == NUM) {
                _lex->__last_node->value = _lex->__last_node->value * 10 + _ch_to_int(curr_char);
            } else {
                add_token_node(_lex, NUM, _ch_to_int(curr_char));
            }
            continue;
        }

        switch(curr_char) {
            case '+':
                add_token_node(_lex, OP_ADD, 1);
                break;

            case '-':
                add_token_node(_lex, OP_SUB, 1);
                break;
                
            case '*':
                add_token_node(_lex, OP_MUL, 1);
                break;
                
            case '/':
                add_token_node(_lex, OP_DIV, 1);
                break;
                
            case '^':
                add_token_node(_lex, OP_POW, 1);
                break;

            case 'X':
                add_token_node(_lex, VAR_X, 1);
                break;

            case '(':
                add_token_node(_lex, LPAREN, ++open_paren_count);
                break;

            case ')':
                add_token_node(_lex, RPAREN, open_paren_count--);
                break;

            case 's':
                if (*(str+pos+1) == 'h') {
                    add_token_node(_lex, F_SH, 1);
                    ++pos;
                } else if (*(str+pos+1) == 'i' && *(str+pos+2) == 'n') {
                    add_token_node(_lex, F_SIN, 1);
                    ++pos; ++pos;
                } else {
                    *errn = -E_INVALID_TOKEN;
                    return NULL;
                }
                break;

            case 'c':
                if (*(str+pos+1) == 'h') {
                    add_token_node(_lex, F_CH, 1);
                    ++pos;
                } else if (*(str+pos+1) == 'o' && *(str+pos+2) == 's') {
                    add_token_node(_lex, F_COS, 1);
                    ++pos; ++pos;
                } else {
                    *errn = -E_INVALID_TOKEN;
                    return NULL;
                }
                break;

            case 't':
                if (*(str+pos+1) == 'a' && *(str+pos+2) == 'n') {
                    add_token_node(_lex, F_TAN, 1);
                    ++pos; ++pos;
                } else {
                    *errn = -E_INVALID_TOKEN;
                    return NULL;
                }
                break;

            case 'l':
                if (*(str+pos+1) == 'n') {
                    add_token_node(_lex, F_LN, 1);
                    ++pos;
                } else {
                    *errn = -E_INVALID_TOKEN;
                    return NULL;
                }
                break;

            default:
                *errn = -E_INVALID_TOKEN;
                return NULL;
        }
        
    }

    return _lex;
}

StringBuffer *detokenize(Lex *lex) {
    tnodevalue tmp_num, base, sc;

    StringBuffer *detok = create_str_buffer();    
    TokenNode *curr_token;

    while ((curr_token = lex_iternode(lex)) != NULL) {
        switch (curr_token->tok) {
            case NUM:
                if (curr_token->value == 0) {
                    push_char_to_buffer(detok, '0');
                    break;
                }

                tmp_num = curr_token->value;

                for (sc = 0; tmp_num > 0; sc++)
                    tmp_num /= 10;

                tmp_num = curr_token->value;

                while (sc--) {
                    base = tmp_num / pow(10, sc);
                    tmp_num -= base * pow(10, sc);
                    push_char_to_buffer(detok, _int_to_ch(base));
                }
                break;
            case OP_ADD:
                push_char_to_buffer(detok, '+');
                break;
            case OP_SUB:
                push_char_to_buffer(detok, '-');
                break;
            case OP_MUL:
                push_char_to_buffer(detok, '*');
                break;
            case OP_DIV:
                push_char_to_buffer(detok, '/');
                break;
            case OP_POW:
                push_char_to_buffer(detok, '^');
                break;
            case VAR_X:
                push_char_to_buffer(detok, 'X');
                break;
            case F_SIN:
                push_str_to_buffer(detok, "sin", 3);
                break;
            case F_COS:
                push_str_to_buffer(detok, "cos", 3);
                break;
            case F_TAN:
                push_str_to_buffer(detok, "tan", 3);
                break;
            case F_SH:
                push_str_to_buffer(detok, "sh", 2);
                break;
            case F_CH:
                push_str_to_buffer(detok, "ch", 2);
                break;
            case F_LN:
                push_str_to_buffer(detok, "ln", 2);
                break;
            case LPAREN:
                push_char_to_buffer(detok, '(');
                break;
            case RPAREN:
                push_char_to_buffer(detok, ')');
                break;
            default:
                break;
        }
    }

    return detok;
}

TokenNode *lex_iternode(Lex *lex) {
    TokenNode *tnode = lex->token_list;

    if (lex->token_list == NULL)
        return NULL;

    lex->token_list = lex->token_list->next;
    lex->lex_size--;

    return tnode;
}

char *get_token_repr(TokenNode *node) {
    char *buf = (char *) malloc(sizeof(char) * 80);

    switch (node->tok) {
        case __INIT:
            sprintf(buf, "__INIT");
            break;
        case NUM:
            sprintf(buf, "NUM(%d)", node->value);
            break;
        case OP_ADD:
            sprintf(buf, "OP(+)");
            break;
        case OP_SUB:
            sprintf(buf, "OP(-)");
            break;
        case OP_MUL:
            sprintf(buf, "OP(*)");
            break;
        case OP_DIV:
            sprintf(buf, "OP(/)");
            break;
        case OP_POW:
            sprintf(buf, "OP(^)");
            break;
        case VAR_X:
            sprintf(buf, "VAR(X)");
            break;
        case F_SIN:
            sprintf(buf, "SIN(X)");
            break;
        case F_COS:
            sprintf(buf, "COS(X)");
            break;
        case F_TAN:
            sprintf(buf, "TAN(X)");
            break;
        case F_SH:
            sprintf(buf, "SINH(X)");
            break;
        case F_CH:
            sprintf(buf, "COSH(X)");
            break;
        case F_LN:
            sprintf(buf, "LN(X)");
            break;
        case LPAREN:
            sprintf(buf, "LPAREN");
            break;
        case RPAREN:
            sprintf(buf, "RPAREN");
            break;
    }

    return buf;
}

void dump_lex(Lex *lex) {
    int ctr = 0;
    TokenNode *node;
    
    if (lex == NULL) {
        printf("Lex is null. An error was occured probably.\n");
        return;
    }

    node = lex->token_list;

    while(node != NULL) {
        printf("#%d\t%s\t(%d)\n", ctr++, get_token_repr(node), node->ttype);
        node = node->next;
    }
}

void free_lex(Lex *lex) {
    TokenNode *next_node, *node = lex->token_list;

    while (node != NULL) {
        if (node->next == NULL) {
            free(node);
            break;
        }

        next_node = node->next;
        free(node);
        node = next_node;
    }

    free(lex);
}

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
            node->value = 0;
            break;
        case TT_VARIABLE:
            node->tok = NUM;
            node->ttype = TT_LITERAL;
            node->value = 1;
            node->right = NULL;
            node->left = NULL;
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
    node->right = onenode;
    node->left = pownode;
    node->tok = OP_ADD;
    node->ttype = get_token_type(node->tok);

    return err_ret;
}

/*int derive_cosine(ASTNode *node) {

}*/

int _derive_log(ASTNode *node) {
    int err_ret = 0;
    ASTNode *onenode, *varnode;

    onenode = create_ast_node(NUM, 1);
    varnode = create_ast_node(VAR_X, 0);

    node->left = onenode;
    node->right = varnode;
    node->tok = OP_DIV;
    node->ttype = get_token_type(node->tok);

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

    node->tok = OP_ADD;
    node->ttype = get_token_type(node->tok);
    node->left = mulnode_left;
    node->right = mulnode_right;

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

    node->tok = OP_DIV;
    node->ttype = get_token_type(node->tok);
    node->left = subnode;
    node->right = pownode;

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
    
    node->tok = OP_MUL;
    node->ttype = get_token_type(node->tok);
    node->left = mulnode;
    node->right = fder;

    return err_ret;
}

StringBuffer *create_str_buffer(void) {
    StringBuffer *buf = (StringBuffer *) malloc(sizeof(StringBuffer));

    buf->str = (char *) malloc(DEF_STRBUF_SIZE_CHAR * sizeof(char));
    buf->size = DEF_STRBUF_SIZE_CHAR;
    buf->last_char_indx = 0;

    zero_str_buffer(buf);

    return buf;
}

void zero_str_buffer(StringBuffer *buf) {
    int i;

    for (i = 0; i < buf->size; i++)
        buf->str[i] = 0;
}

int push_char_to_buffer(StringBuffer *buf, char c) {
    if (buf->size <= buf->last_char_indx - 1)
        extend_str_buffer(buf);

    buf->str[buf->last_char_indx++] = c;

    return buf->last_char_indx;
}

int push_str_to_buffer(StringBuffer *buf, char *str, int ssize) {
    int i;

    for (i = 0; i < ssize; i++)
        push_char_to_buffer(buf, str[i]);

    return buf->last_char_indx;
}

int extend_str_buffer(StringBuffer *buf) {
    buf->size += STRBUF_EXTEND_SIZE;
    buf->str = (char *) realloc(buf->str, buf->size);

    return buf->size;
}

int get_string_input(StringBuffer *buf) {
    char ch;

    while ((ch = getchar()) != EOF)
        push_char_to_buffer(buf, ch);

    return buf->last_char_indx;
}

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
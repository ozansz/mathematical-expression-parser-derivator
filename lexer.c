#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lexer.h"
#include "strbuf.h"

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


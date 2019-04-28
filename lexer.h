#if !defined(LEXER_H)
#define LEXER_H

#include "strbuf.h"

#define OP_ASSOC_LEFT   1
#define OP_ASSOC_RIGHT  2

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

int is_number(char ch);
int is_space(char ch);
int _ch_to_int(char ch);
char _int_to_ch(int i);

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

#endif
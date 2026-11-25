#ifndef TOKENS_H
#define TOKENS_H

typedef enum {
    TOKEN_FN,
    TOKEN_LET,
    TOKEN_MUT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_TRUE,
    TOKEN_FALSE,

    TOKEN_I32,
    TOKEN_F64,
    TOKEN_BOOL,
    TOKEN_STR,

    TOKEN_IDENT,
    TOKEN_INT_LIT,
    TOKEN_FLOAT_LIT,
    TOKEN_STR_LIT,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,

    TOKEN_EQ,

    TOKEN_EQEQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,

    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    TOKEN_ARROW,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COMMA,
    TOKEN_SEMI,
    TOKEN_COLON,

    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType tipo;
    char lexema[64];
    int linha;
    int coluna;
} Token;

const char *validarToken(TokenType t);

#endif

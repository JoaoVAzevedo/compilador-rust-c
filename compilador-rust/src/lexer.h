#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"

typedef struct {
    const char *src;
    int pos;
    int linha;
    int coluna;
} Lexer;

void        lexerInit(Lexer *lx, const char *src);
Token       proximoToken(Lexer *lx);
const char *mensagemErroLexico(Token t);

#endif

#ifndef AST_H
#define AST_H

#include "tokens.h"
#include "symtab.h"

typedef enum {
    N_PROGRAMA, N_FUNCAO, N_PARAM, N_BLOCO,
    N_LET, N_ATRIB, N_IF, N_WHILE, N_RETURN, N_EXPR,
    N_BINARIO, N_UNARIO, N_INT, N_FLOAT, N_BOOL, N_STR, N_IDENT, N_CHAMADA
} NoTipo;

typedef struct No {
    NoTipo tipo;
    int linha;
    int coluna;
    char nome[64];
    TokenType op;
    Tipo tipoDado;
    int mutavel;
    struct No *a;
    struct No *b;
    struct No *c;
    struct No *prox;
} No;

No  *noCriar(NoTipo tipo, int linha, int coluna);
No  *listaAnexar(No *lista, No *novo);
void astImprimir(const No *raiz);
void astLiberar(No *n);

#endif

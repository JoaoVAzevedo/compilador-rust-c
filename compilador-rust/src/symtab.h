#ifndef SYMTAB_H
#define SYMTAB_H

#define TAM_HASH 64
#define MAX_PARAMS 8

typedef enum { T_INT, T_FLOAT, T_BOOL, T_STR, T_VOID, T_NENHUM, T_ERRO } Tipo;
typedef enum { CAT_IDENT, CAT_VAR, CAT_PARAM, CAT_FUNC } Categoria;

typedef struct Simbolo {
    char nome[64];
    Categoria categoria;
    Tipo tipo;
    int linha;
    int ocorrencias;
    int mutavel;
    int nparams;
    Tipo params[MAX_PARAMS];
    struct Simbolo *proxHash;
    struct Simbolo *proxOrdem;
} Simbolo;

typedef struct {
    Simbolo *balde[TAM_HASH];
    Simbolo *primeiro;
    Simbolo *ultimo;
} Tabela;

Tabela  *tabelaCriar(void);
void     tabelaLiberar(Tabela *t);
Simbolo *tabelaBuscar(Tabela *t, const char *nome);
Simbolo *tabelaInserir(Tabela *t, const char *nome);

const char *nomeTipo(Tipo t);
const char *nomeCategoria(Categoria c);

#endif

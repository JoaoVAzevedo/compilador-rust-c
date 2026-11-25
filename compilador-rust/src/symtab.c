#include <stdlib.h>
#include <string.h>
#include "symtab.h"

static unsigned int hash(const char *s) {
    unsigned int h = 5381;
    while (*s) h = h * 33 + (unsigned char)*s++;
    return h % TAM_HASH;
}

Tabela *tabelaCriar(void) {
    return calloc(1, sizeof(Tabela));
}

void tabelaLiberar(Tabela *t) {
    Simbolo *s = t->primeiro;
    while (s) {
        Simbolo *prox = s->proxOrdem;
        free(s);
        s = prox;
    }
    free(t);
}

Simbolo *tabelaBuscar(Tabela *t, const char *nome) {
    for (Simbolo *s = t->balde[hash(nome)]; s; s = s->proxHash) {
        if (strcmp(s->nome, nome) == 0) return s;
    }
    return NULL;
}

Simbolo *tabelaInserir(Tabela *t, const char *nome) {
    Simbolo *s = calloc(1, sizeof(Simbolo));
    strncpy(s->nome, nome, sizeof(s->nome) - 1);

    unsigned int h = hash(nome);
    s->proxHash = t->balde[h];
    t->balde[h] = s;

    if (t->ultimo) t->ultimo->proxOrdem = s;
    else t->primeiro = s;
    t->ultimo = s;
    return s;
}

const char *nomeTipo(Tipo t) {
    switch (t) {
        case T_INT:    return "i32";
        case T_FLOAT:  return "f64";
        case T_BOOL:   return "bool";
        case T_STR:    return "str";
        case T_VOID:   return "void";
        case T_NENHUM: return "?";
        default:       return "erro";
    }
}

const char *nomeCategoria(Categoria c) {
    switch (c) {
        case CAT_VAR:   return "variavel";
        case CAT_PARAM: return "parametro";
        case CAT_FUNC:  return "funcao";
        default:        return "identificador";
    }
}

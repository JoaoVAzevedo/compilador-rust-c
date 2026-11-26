#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

No *noCriar(NoTipo tipo, int linha, int coluna) {
    No *n = calloc(1, sizeof(No));
    n->tipo = tipo;
    n->linha = linha;
    n->coluna = coluna;
    n->tipoDado = T_NENHUM;
    return n;
}

No *listaAnexar(No *lista, No *novo) {
    if (!novo) return lista;
    if (!lista) return novo;
    No *p = lista;
    while (p->prox) p = p->prox;
    p->prox = novo;
    return lista;
}

static void descrever(const No *n, char *buf, size_t tam) {
    switch (n->tipo) {
        case N_PROGRAMA: snprintf(buf, tam, "Programa"); break;
        case N_FUNCAO:   snprintf(buf, tam, "Funcao %s -> %s", n->nome, nomeTipo(n->tipoDado)); break;
        case N_PARAM:    snprintf(buf, tam, "Param %s : %s", n->nome, nomeTipo(n->tipoDado)); break;
        case N_BLOCO:    snprintf(buf, tam, "Bloco"); break;
        case N_LET:      snprintf(buf, tam, "Let%s %s : %s", n->mutavel ? " mut" : "", n->nome, nomeTipo(n->tipoDado)); break;
        case N_ATRIB:    snprintf(buf, tam, "Atribuicao %s", n->nome); break;
        case N_IF:       snprintf(buf, tam, "If"); break;
        case N_WHILE:    snprintf(buf, tam, "While"); break;
        case N_RETURN:   snprintf(buf, tam, "Return"); break;
        case N_EXPR:     snprintf(buf, tam, "ExprStmt"); break;
        case N_BINARIO:  snprintf(buf, tam, "Binario '%s' : %s", n->nome, nomeTipo(n->tipoDado)); break;
        case N_UNARIO:   snprintf(buf, tam, "Unario '%s' : %s", n->nome, nomeTipo(n->tipoDado)); break;
        case N_INT:      snprintf(buf, tam, "Int %s", n->nome); break;
        case N_FLOAT:    snprintf(buf, tam, "Float %s", n->nome); break;
        case N_BOOL:     snprintf(buf, tam, "Bool %s", n->nome); break;
        case N_STR:      snprintf(buf, tam, "Str %s", n->nome); break;
        case N_IDENT:    snprintf(buf, tam, "Ident %s : %s", n->nome, nomeTipo(n->tipoDado)); break;
        case N_CHAMADA:  snprintf(buf, tam, "Chamada %s : %s", n->nome, nomeTipo(n->tipoDado)); break;
    }
}

static void imprimirNo(const No *n, int nivel);

static void imprimirLista(const No *n, int nivel) {
    for (; n; n = n->prox) imprimirNo(n, nivel);
}

static void imprimirNo(const No *n, int nivel) {
    char desc[160];
    descrever(n, desc, sizeof(desc));
    printf("%*s%s\n", nivel * 2, "", desc);
    imprimirLista(n->a, nivel + 1);
    imprimirLista(n->b, nivel + 1);
    imprimirLista(n->c, nivel + 1);
}

void astImprimir(const No *raiz) {
    printf("\n=== ARVORE SINTATICA ABSTRATA (AST) ===\n");
    if (raiz) imprimirNo(raiz, 0);
}

void astLiberar(No *n) {
    while (n) {
        No *prox = n->prox;
        astLiberar(n->a);
        astLiberar(n->b);
        astLiberar(n->c);
        free(n);
        n = prox;
    }
}

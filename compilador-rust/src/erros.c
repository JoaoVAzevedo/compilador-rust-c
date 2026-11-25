#include <stdio.h>
#include <stdarg.h>
#include "erros.h"

#define MAX_ERROS 200

typedef struct {
    Fase fase;
    int linha;
    int coluna;
    char msg[160];
} Erro;

static Erro lista[MAX_ERROS];
static int total = 0;
static int descartados = 0;

void erroRegistrar(Fase fase, int linha, int coluna, const char *fmt, ...) {
    if (total >= MAX_ERROS) {
        descartados++;
        return;
    }
    Erro *e = &lista[total++];
    e->fase = fase;
    e->linha = linha;
    e->coluna = coluna;

    va_list args;
    va_start(args, fmt);
    vsnprintf(e->msg, sizeof(e->msg), fmt, args);
    va_end(args);
}

int errosTotal(void) { return total + descartados; }

static const char *nomeFase(Fase f) {
    switch (f) {
        case FASE_LEXICA:    return "Lexico";
        case FASE_SINTATICA: return "Sintatico";
        default:             return "Semantico";
    }
}

void errosImprimir(void) {
    printf("\n=== TABELA DE ERROS ===\n");
    if (errosTotal() == 0) {
        printf("(nenhum erro)\n");
        return;
    }
    printf("%-10s %-8s %s\n", "FASE", "LIN:COL", "DESCRICAO");
    for (int i = 0; i < total; i++) {
        char pos[24];
        snprintf(pos, sizeof(pos), "%d:%d", lista[i].linha, lista[i].coluna);
        printf("%-10s %-8s %s\n", nomeFase(lista[i].fase), pos, lista[i].msg);
    }
    if (descartados > 0) printf("... mais %d erro(s) omitido(s)\n", descartados);
    printf("Total: %d erro(s)\n", errosTotal());
}

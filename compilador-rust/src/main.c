#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "erros.h"
#include "symtab.h"
#include "ast.h"

static char *lerArquivo(const char *caminho) {
    FILE *f = fopen(caminho, "rb");
    if (!f) {
        perror(caminho);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    rewind(f);

    char *src = malloc(tam + 1);
    if (!src) {
        fclose(f);
        return NULL;
    }
    size_t lidos = fread(src, 1, tam, f);
    src[lidos] = '\0';
    fclose(f);
    return src;
}

static void imprimirTabelaLexica(const Tabela *t) {
    printf("\n=== TABELA DE SIMBOLOS INICIAL (LEXICA) ===\n");
    printf("%-16s %-12s %s\n", "IDENTIFICADOR", "1A LINHA", "OCORRENCIAS");
    for (const Simbolo *s = t->primeiro; s; s = s->proxOrdem) {
        printf("%-16s %-12d %d\n", s->nome, s->linha, s->ocorrencias);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: %s arquivo.rs\n", argv[0]);
        return 1;
    }

    char *src = lerArquivo(argv[1]);
    if (!src) return 1;

    Lexer lx;
    lexerInit(&lx, src);
    Tabela *lexica = tabelaCriar();

    int capacidade = 256, quantidade = 0;
    Token *tokens = malloc(capacidade * sizeof(Token));

    printf("=== ANALISE LEXICA: TOKENS ===\n");
    Token t;
    do {
        t = proximoToken(&lx);
        if (quantidade == capacidade) {
            capacidade *= 2;
            tokens = realloc(tokens, capacidade * sizeof(Token));
        }
        tokens[quantidade++] = t;
        printf("%3d:%-3d %-16s '%s'\n", t.linha, t.coluna, validarToken(t.tipo), t.lexema);

        if (t.tipo == TOKEN_IDENT) {
            Simbolo *s = tabelaBuscar(lexica, t.lexema);
            if (!s) {
                s = tabelaInserir(lexica, t.lexema);
                s->categoria = CAT_IDENT;
                s->linha = t.linha;
            }
            s->ocorrencias++;
        } else if (t.tipo == TOKEN_ERROR) {
            erroRegistrar(FASE_LEXICA, t.linha, t.coluna, "%s", mensagemErroLexico(t));
        }
    } while (t.tipo != TOKEN_EOF);

    imprimirTabelaLexica(lexica);

    int status = 0;
    if (errosTotal() > 0) {
        printf("\nAnalise interrompida: o arquivo possui erros lexicos.\n");
        errosImprimir();
        status = 1;
    } else {
        No *programa = parse(tokens, quantidade);
        if (errosTotal() > 0) {
            printf("\nAnalise interrompida: o arquivo possui erros sintaticos.\n");
            errosImprimir();
            status = 1;
        } else {
            printf("\nAnalise sintatica: o programa pertence a gramatica.\n");
            analisar(programa);
            astImprimir(programa);
            semanticaImprimirTabela();
            errosImprimir();
            status = errosTotal() > 0;
        }
        astLiberar(programa);
    }

    tabelaLiberar(lexica);
    free(tokens);
    free(src);
    return status;
}

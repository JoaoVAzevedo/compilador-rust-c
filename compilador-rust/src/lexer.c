#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"

void lexerInit(Lexer *lx, const char *src) {
    lx->src = src;
    lx->pos = 0;
    lx->linha = 1;
    lx->coluna = 1;
}

static char atual(Lexer *lx) { return lx->src[lx->pos]; }
static char proximo(Lexer *lx) { return atual(lx) == '\0' ? '\0' : lx->src[lx->pos + 1]; }

static char avancar(Lexer *lx) {
    char c = lx->src[lx->pos++];
    if (c == '\n') {
        lx->linha++;
        lx->coluna = 1;
    } else {
        lx->coluna++;
    }
    return c;
}

static int casar(Lexer *lx, char esperado) {
    if (atual(lx) == esperado) { avancar(lx); return 1; }
    return 0;
}

static Token criarToken(TokenType tipo, const char *lex, int len, int linha, int coluna) {
    Token t;
    if (len > 63) len = 63;
    memcpy(t.lexema, lex, len);
    t.lexema[len] = '\0';
    t.tipo = tipo;
    t.linha = linha;
    t.coluna = coluna;
    return t;
}

static int pularIgnorados(Lexer *lx, int *linhaErro, int *colunaErro) {
    for (;;) {
        char c = atual(lx);

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            avancar(lx);
        }
        else if (c == '/' && proximo(lx) == '/') {
            while (atual(lx) != '\n' && atual(lx) != '\0') avancar(lx);
        }
        else if (c == '/' && proximo(lx) == '*') {
            *linhaErro = lx->linha;
            *colunaErro = lx->coluna;
            avancar(lx);
            avancar(lx);
            for (;;) {
                if (atual(lx) == '\0') return 1;
                if (atual(lx) == '*' && proximo(lx) == '/') {
                    avancar(lx);
                    avancar(lx);
                    break;
                }
                avancar(lx);
            }
        }
        else {
            return 0;
        }
    }
}

typedef struct {
    const char *palavra;
    TokenType tipo;
} Reservada;

static const Reservada reservadas[] = {
    {"fn", TOKEN_FN},         {"let", TOKEN_LET},       {"mut", TOKEN_MUT},
    {"if", TOKEN_IF},         {"else", TOKEN_ELSE},     {"while", TOKEN_WHILE},
    {"return", TOKEN_RETURN}, {"true", TOKEN_TRUE},     {"false", TOKEN_FALSE},
    {"i32", TOKEN_I32},       {"f64", TOKEN_F64},       {"bool", TOKEN_BOOL},
    {"str", TOKEN_STR}
};

static Token lerIdentificador(Lexer *lx) {
    int linha = lx->linha;
    int coluna = lx->coluna;
    int inicio = lx->pos;

    while (isalnum((unsigned char)atual(lx)) || atual(lx) == '_') avancar(lx);

    int len = lx->pos - inicio;
    const char *lex = lx->src + inicio;

    for (size_t i = 0; i < sizeof(reservadas) / sizeof(reservadas[0]); i++) {
        if ((int)strlen(reservadas[i].palavra) == len &&
            strncmp(lex, reservadas[i].palavra, len) == 0) {
            return criarToken(reservadas[i].tipo, lex, len, linha, coluna);
        }
    }
    return criarToken(TOKEN_IDENT, lex, len, linha, coluna);
}

static Token lerNumero(Lexer *lx) {
    int linha = lx->linha;
    int coluna = lx->coluna;
    int inicio = lx->pos;

    while (isdigit((unsigned char)atual(lx))) avancar(lx);

    if (atual(lx) == '.') {
        avancar(lx);
        if (!isdigit((unsigned char)atual(lx))) {
            return criarToken(TOKEN_ERROR, lx->src + inicio, lx->pos - inicio, linha, coluna);
        }
        while (isdigit((unsigned char)atual(lx))) avancar(lx);
        return criarToken(TOKEN_FLOAT_LIT, lx->src + inicio, lx->pos - inicio, linha, coluna);
    }
    return criarToken(TOKEN_INT_LIT, lx->src + inicio, lx->pos - inicio, linha, coluna);
}

static Token lerString(Lexer *lx) {
    int linha = lx->linha;
    int coluna = lx->coluna;
    int inicio = lx->pos;

    avancar(lx);
    while (atual(lx) != '"' && atual(lx) != '\n' && atual(lx) != '\0') {
        if (atual(lx) == '\\' && proximo(lx) != '\0') avancar(lx);
        avancar(lx);
    }
    if (atual(lx) != '"') {
        return criarToken(TOKEN_ERROR, lx->src + inicio, lx->pos - inicio, linha, coluna);
    }
    avancar(lx);
    return criarToken(TOKEN_STR_LIT, lx->src + inicio, lx->pos - inicio, linha, coluna);
}

static Token lerSimbolo(Lexer *lx) {
    int linha = lx->linha;
    int coluna = lx->coluna;
    int inicio = lx->pos;
    char c = avancar(lx);
    TokenType tipo;

    switch (c) {
        case '+': tipo = TOKEN_PLUS;    break;
        case '*': tipo = TOKEN_STAR;    break;
        case '/': tipo = TOKEN_SLASH;   break;
        case '%': tipo = TOKEN_PERCENT; break;
        case '(': tipo = TOKEN_LPAREN;  break;
        case ')': tipo = TOKEN_RPAREN;  break;
        case '{': tipo = TOKEN_LBRACE;  break;
        case '}': tipo = TOKEN_RBRACE;  break;
        case ',': tipo = TOKEN_COMMA;   break;
        case ';': tipo = TOKEN_SEMI;    break;
        case ':': tipo = TOKEN_COLON;   break;

        case '-': tipo = casar(lx, '>') ? TOKEN_ARROW : TOKEN_MINUS; break;
        case '=': tipo = casar(lx, '=') ? TOKEN_EQEQ  : TOKEN_EQ;    break;
        case '!': tipo = casar(lx, '=') ? TOKEN_NEQ   : TOKEN_NOT;   break;
        case '<': tipo = casar(lx, '=') ? TOKEN_LE    : TOKEN_LT;    break;
        case '>': tipo = casar(lx, '=') ? TOKEN_GE    : TOKEN_GT;    break;

        case '&': tipo = casar(lx, '&') ? TOKEN_AND : TOKEN_ERROR; break;
        case '|': tipo = casar(lx, '|') ? TOKEN_OR  : TOKEN_ERROR; break;

        default:  tipo = TOKEN_ERROR; break;
    }
    return criarToken(tipo, lx->src + inicio, lx->pos - inicio, linha, coluna);
}

Token proximoToken(Lexer *lx) {
    int linhaErro, colunaErro;
    if (pularIgnorados(lx, &linhaErro, &colunaErro)) {
        return criarToken(TOKEN_ERROR, "/*", 2, linhaErro, colunaErro);
    }

    char c = atual(lx);

    if (c == '\0')                              return criarToken(TOKEN_EOF, "", 0, lx->linha, lx->coluna);
    if (isalpha((unsigned char)c) || c == '_')  return lerIdentificador(lx);
    if (isdigit((unsigned char)c))              return lerNumero(lx);
    if (c == '"')                               return lerString(lx);
    return lerSimbolo(lx);
}

const char *mensagemErroLexico(Token t) {
    static char msg[160];
    char c = t.lexema[0];

    if (strcmp(t.lexema, "/*") == 0)
        return "comentario de bloco nao fechado";
    if (c == '"')
        return "literal de texto nao fechado";
    if (isdigit((unsigned char)c))
        snprintf(msg, sizeof(msg), "numero decimal mal formado '%s' (faltam digitos apos o ponto)", t.lexema);
    else if (c == '&' || c == '|')
        snprintf(msg, sizeof(msg), "operador incompleto '%s' (esperado '%c%c')", t.lexema, c, c);
    else
        snprintf(msg, sizeof(msg), "caractere invalido '%s'", t.lexema);
    return msg;
}

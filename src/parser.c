#include <string.h>
#include "parser.h"
#include "erros.h"

typedef struct {
    const Token *toks;
    int n;
    int pos;
    int panico;
} Parser;

static const Token *atual(Parser *p) { return &p->toks[p->pos]; }

static const Token *olhar(Parser *p, int d) {
    int i = p->pos + d;
    if (i >= p->n) i = p->n - 1;
    return &p->toks[i];
}

static int checar(Parser *p, TokenType t) { return atual(p)->tipo == t; }

static const Token *avancar(Parser *p) {
    const Token *t = atual(p);
    if (t->tipo != TOKEN_EOF) p->pos++;
    return t;
}

static int casar(Parser *p, TokenType t) {
    if (checar(p, t)) { avancar(p); return 1; }
    return 0;
}

static void copiar(char *dest, const char *src) {
    strncpy(dest, src, 63);
    dest[63] = '\0';
}

static void erroSintatico(Parser *p, const char *esperado) {
    if (p->panico) return;
    p->panico = 1;
    const Token *t = atual(p);
    if (t->tipo == TOKEN_EOF)
        erroRegistrar(FASE_SINTATICA, t->linha, t->coluna, "esperado %s, mas o arquivo terminou", esperado);
    else
        erroRegistrar(FASE_SINTATICA, t->linha, t->coluna, "esperado %s, encontrado '%s'", esperado, t->lexema);
}

static int esperar(Parser *p, TokenType t, const char *descricao) {
    if (casar(p, t)) {
        if (t == TOKEN_SEMI) p->panico = 0;
        return 1;
    }
    erroSintatico(p, descricao);
    return 0;
}

static void sincronizar(Parser *p) {
    p->panico = 0;
    while (!checar(p, TOKEN_EOF)) {
        switch (atual(p)->tipo) {
            case TOKEN_SEMI:
                avancar(p);
                return;
            case TOKEN_RBRACE: case TOKEN_LET: case TOKEN_IF:
            case TOKEN_WHILE: case TOKEN_RETURN: case TOKEN_FN:
                return;
            default:
                avancar(p);
        }
    }
}

static No *expressao(Parser *p);
static No *bloco(Parser *p);

static No *primario(Parser *p) {
    const Token *t = atual(p);
    No *n;

    switch (t->tipo) {
        case TOKEN_INT_LIT:   n = noCriar(N_INT, t->linha, t->coluna);   break;
        case TOKEN_FLOAT_LIT: n = noCriar(N_FLOAT, t->linha, t->coluna); break;
        case TOKEN_STR_LIT:   n = noCriar(N_STR, t->linha, t->coluna);   break;
        case TOKEN_TRUE:
        case TOKEN_FALSE:     n = noCriar(N_BOOL, t->linha, t->coluna);  break;

        case TOKEN_IDENT:
            avancar(p);
            if (casar(p, TOKEN_LPAREN)) {
                n = noCriar(N_CHAMADA, t->linha, t->coluna);
                copiar(n->nome, t->lexema);
                if (!checar(p, TOKEN_RPAREN)) {
                    do {
                        n->a = listaAnexar(n->a, expressao(p));
                    } while (casar(p, TOKEN_COMMA));
                }
                esperar(p, TOKEN_RPAREN, "')'");
                return n;
            }
            n = noCriar(N_IDENT, t->linha, t->coluna);
            copiar(n->nome, t->lexema);
            return n;

        case TOKEN_LPAREN:
            avancar(p);
            n = expressao(p);
            esperar(p, TOKEN_RPAREN, "')'");
            return n;

        default:
            erroSintatico(p, "uma expressao");
            return NULL;
    }
    copiar(n->nome, t->lexema);
    avancar(p);
    return n;
}

static No *unario(Parser *p) {
    if (checar(p, TOKEN_MINUS) || checar(p, TOKEN_NOT)) {
        const Token *op = avancar(p);
        No *n = noCriar(N_UNARIO, op->linha, op->coluna);
        n->op = op->tipo;
        copiar(n->nome, op->lexema);
        n->a = unario(p);
        return n;
    }
    return primario(p);
}

static int precedencia(TokenType t) {
    switch (t) {
        case TOKEN_OR:  return 1;
        case TOKEN_AND: return 2;
        case TOKEN_EQEQ: case TOKEN_NEQ: return 3;
        case TOKEN_LT: case TOKEN_GT: case TOKEN_LE: case TOKEN_GE: return 4;
        case TOKEN_PLUS: case TOKEN_MINUS: return 5;
        case TOKEN_STAR: case TOKEN_SLASH: case TOKEN_PERCENT: return 6;
        default: return 0;
    }
}

static No *binario(Parser *p, int precMinima) {
    No *esq = unario(p);
    int prec;
    while ((prec = precedencia(atual(p)->tipo)) >= precMinima) {
        const Token *op = avancar(p);
        No *dir = binario(p, prec + 1);
        No *n = noCriar(N_BINARIO, op->linha, op->coluna);
        n->op = op->tipo;
        copiar(n->nome, op->lexema);
        n->a = esq;
        n->b = dir;
        esq = n;
    }
    return esq;
}

static No *expressao(Parser *p) { return binario(p, 1); }

static Tipo tipoDeclarado(Parser *p) {
    switch (atual(p)->tipo) {
        case TOKEN_I32:  avancar(p); return T_INT;
        case TOKEN_F64:  avancar(p); return T_FLOAT;
        case TOKEN_BOOL: avancar(p); return T_BOOL;
        case TOKEN_STR:  avancar(p); return T_STR;
        default:
            erroSintatico(p, "um tipo (i32, f64, bool ou str)");
            return T_ERRO;
    }
}

static No *comandoLet(Parser *p) {
    const Token *t = avancar(p);
    No *n = noCriar(N_LET, t->linha, t->coluna);

    if (casar(p, TOKEN_MUT)) n->mutavel = 1;

    const Token *id = atual(p);
    if (esperar(p, TOKEN_IDENT, "um identificador")) {
        copiar(n->nome, id->lexema);
        n->linha = id->linha;
        n->coluna = id->coluna;
    }
    if (casar(p, TOKEN_COLON)) n->tipoDado = tipoDeclarado(p);
    esperar(p, TOKEN_EQ, "'='");
    n->a = expressao(p);
    esperar(p, TOKEN_SEMI, "';'");
    return n;
}

static No *comandoIf(Parser *p) {
    const Token *t = avancar(p);
    No *n = noCriar(N_IF, t->linha, t->coluna);
    n->a = expressao(p);
    n->b = bloco(p);
    if (casar(p, TOKEN_ELSE)) {
        n->c = checar(p, TOKEN_IF) ? comandoIf(p) : bloco(p);
    }
    return n;
}

static No *comandoWhile(Parser *p) {
    const Token *t = avancar(p);
    No *n = noCriar(N_WHILE, t->linha, t->coluna);
    n->a = expressao(p);
    n->b = bloco(p);
    return n;
}

static No *comandoReturn(Parser *p) {
    const Token *t = avancar(p);
    No *n = noCriar(N_RETURN, t->linha, t->coluna);
    if (!checar(p, TOKEN_SEMI)) n->a = expressao(p);
    esperar(p, TOKEN_SEMI, "';'");
    return n;
}

static No *comandoAtribuicao(Parser *p) {
    const Token *id = avancar(p);
    avancar(p);
    No *n = noCriar(N_ATRIB, id->linha, id->coluna);
    copiar(n->nome, id->lexema);
    n->a = expressao(p);
    esperar(p, TOKEN_SEMI, "';'");
    return n;
}

static No *comandoExpressao(Parser *p) {
    const Token *t = atual(p);
    No *n = noCriar(N_EXPR, t->linha, t->coluna);
    n->a = expressao(p);
    esperar(p, TOKEN_SEMI, "';'");
    return n;
}

static No *comando(Parser *p) {
    switch (atual(p)->tipo) {
        case TOKEN_LET:    return comandoLet(p);
        case TOKEN_IF:     return comandoIf(p);
        case TOKEN_WHILE:  return comandoWhile(p);
        case TOKEN_RETURN: return comandoReturn(p);
        case TOKEN_LBRACE: return bloco(p);
        case TOKEN_IDENT:
            if (olhar(p, 1)->tipo == TOKEN_EQ) return comandoAtribuicao(p);
            return comandoExpressao(p);
        default:
            return comandoExpressao(p);
    }
}

static No *bloco(Parser *p) {
    const Token *t = atual(p);
    No *n = noCriar(N_BLOCO, t->linha, t->coluna);

    esperar(p, TOKEN_LBRACE, "'{'");
    while (!checar(p, TOKEN_RBRACE) && !checar(p, TOKEN_EOF) && !checar(p, TOKEN_FN)) {
        int antes = p->pos;
        n->a = listaAnexar(n->a, comando(p));
        if (p->panico) sincronizar(p);
        if (p->pos == antes) avancar(p);
    }
    if (esperar(p, TOKEN_RBRACE, "'}'")) p->panico = 0;
    return n;
}

static No *funcao(Parser *p) {
    const Token *t = avancar(p);
    No *n = noCriar(N_FUNCAO, t->linha, t->coluna);
    n->tipoDado = T_VOID;

    const Token *id = atual(p);
    if (esperar(p, TOKEN_IDENT, "o nome da funcao")) {
        copiar(n->nome, id->lexema);
        n->linha = id->linha;
        n->coluna = id->coluna;
    }

    esperar(p, TOKEN_LPAREN, "'('");
    if (!checar(p, TOKEN_RPAREN) && !checar(p, TOKEN_EOF)) {
        do {
            const Token *pid = atual(p);
            No *par = noCriar(N_PARAM, pid->linha, pid->coluna);
            if (esperar(p, TOKEN_IDENT, "o nome do parametro")) copiar(par->nome, pid->lexema);
            esperar(p, TOKEN_COLON, "':'");
            par->tipoDado = tipoDeclarado(p);
            n->a = listaAnexar(n->a, par);
        } while (casar(p, TOKEN_COMMA));
    }
    esperar(p, TOKEN_RPAREN, "')'");
    if (casar(p, TOKEN_ARROW)) n->tipoDado = tipoDeclarado(p);

    if (p->panico) {
        while (!checar(p, TOKEN_LBRACE) && !checar(p, TOKEN_FN) && !checar(p, TOKEN_EOF)) avancar(p);
        p->panico = 0;
    }
    n->b = bloco(p);
    return n;
}

No *parse(const Token *tokens, int quantidade) {
    Parser p = { tokens, quantidade, 0, 0 };
    No *programa = noCriar(N_PROGRAMA, 1, 1);

    while (!checar(&p, TOKEN_EOF)) {
        if (checar(&p, TOKEN_FN)) {
            programa->a = listaAnexar(programa->a, funcao(&p));
        } else {
            erroSintatico(&p, "'fn' (declaracao de funcao)");
            while (!checar(&p, TOKEN_EOF) && !checar(&p, TOKEN_FN)) avancar(&p);
        }
        p.panico = 0;
    }
    return programa;
}

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "semantic.h"
#include "erros.h"

typedef struct Escopo {
    Tabela *tabela;
    struct Escopo *pai;
    int nivel;
} Escopo;

typedef struct {
    char nome[64];
    Categoria categoria;
    Tipo tipo;
    int nivel;
    int linha;
} Registro;

#define MAX_REGISTROS 512

static Escopo *escopo = NULL;
static Tipo retornoAtual = T_VOID;
static Registro registros[MAX_REGISTROS];
static int totalRegistros = 0;

static void erroSem(const No *n, const char *fmt, ...) {
    char msg[160];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    erroRegistrar(FASE_SEMANTICA, n->linha, n->coluna, "%s", msg);
}

static void abrirEscopo(void) {
    Escopo *e = malloc(sizeof(Escopo));
    e->tabela = tabelaCriar();
    e->pai = escopo;
    e->nivel = escopo ? escopo->nivel + 1 : 0;
    escopo = e;
}

static void fecharEscopo(void) {
    Escopo *e = escopo;
    escopo = e->pai;
    tabelaLiberar(e->tabela);
    free(e);
}

static Simbolo *buscar(const char *nome) {
    for (Escopo *e = escopo; e; e = e->pai) {
        Simbolo *s = tabelaBuscar(e->tabela, nome);
        if (s) return s;
    }
    return NULL;
}

static Simbolo *declarar(const No *n, Categoria cat, Tipo tipo, int mutavel) {
    if (tabelaBuscar(escopo->tabela, n->nome)) {
        erroSem(n, "'%s' ja foi declarado neste escopo", n->nome);
        return NULL;
    }
    Simbolo *s = tabelaInserir(escopo->tabela, n->nome);
    s->categoria = cat;
    s->tipo = tipo;
    s->linha = n->linha;
    s->mutavel = mutavel;

    if (totalRegistros < MAX_REGISTROS) {
        Registro *r = &registros[totalRegistros++];
        snprintf(r->nome, sizeof(r->nome), "%s", n->nome);
        r->categoria = cat;
        r->tipo = tipo;
        r->nivel = escopo->nivel;
        r->linha = n->linha;
    }
    return s;
}

static int compativel(Tipo a, Tipo b) {
    return a == T_ERRO || b == T_ERRO || a == b;
}

static int numerico(Tipo t) { return t == T_INT || t == T_FLOAT; }

static Tipo expressao(No *n);

static Tipo valor(No *n) {
    Tipo t = expressao(n);
    if (t == T_VOID) {
        erroSem(n, "a expressao nao produz valor (funcao sem retorno)");
        n->tipoDado = T_ERRO;
        return T_ERRO;
    }
    return t;
}

static Tipo tipoIdentificador(No *n) {
    Simbolo *s = buscar(n->nome);
    if (!s) {
        erroSem(n, "variavel '%s' nao declarada", n->nome);
        return T_ERRO;
    }
    if (s->categoria == CAT_FUNC) {
        erroSem(n, "'%s' e uma funcao e nao pode ser usada como valor", n->nome);
        return T_ERRO;
    }
    return s->tipo;
}

static Tipo tipoUnario(No *n) {
    Tipo t = expressao(n->a);
    if (t == T_ERRO) return T_ERRO;

    if (n->op == TOKEN_MINUS && !numerico(t)) {
        erroSem(n, "operador '-' exige i32 ou f64, encontrado %s", nomeTipo(t));
        return T_ERRO;
    }
    if (n->op == TOKEN_NOT && t != T_BOOL) {
        erroSem(n, "operador '!' exige bool, encontrado %s", nomeTipo(t));
        return T_ERRO;
    }
    return t;
}

static Tipo tipoBinario(No *n) {
    Tipo e = expressao(n->a);
    Tipo d = expressao(n->b);
    if (e == T_ERRO || d == T_ERRO) return T_ERRO;

    int ok;
    Tipo resultado;

    switch (n->op) {
        case TOKEN_PLUS: case TOKEN_MINUS: case TOKEN_STAR:
        case TOKEN_SLASH: case TOKEN_PERCENT:
            ok = numerico(e) && e == d;
            resultado = e;
            break;
        case TOKEN_LT: case TOKEN_GT: case TOKEN_LE: case TOKEN_GE:
            ok = numerico(e) && e == d;
            resultado = T_BOOL;
            break;
        case TOKEN_EQEQ: case TOKEN_NEQ:
            ok = (e == d) && e != T_VOID;
            resultado = T_BOOL;
            break;
        default:
            ok = (e == T_BOOL && d == T_BOOL);
            resultado = T_BOOL;
            break;
    }
    if (!ok) {
        erroSem(n, "operador '%s' nao pode ser aplicado aos tipos %s e %s",
                n->nome, nomeTipo(e), nomeTipo(d));
        return T_ERRO;
    }
    return resultado;
}

static Tipo tipoChamada(No *n) {
    int quantidade = 0;
    for (No *arg = n->a; arg; arg = arg->prox) {
        valor(arg);
        quantidade++;
    }

    Simbolo *s = buscar(n->nome);
    if (!s) {
        erroSem(n, "funcao '%s' nao declarada", n->nome);
        return T_ERRO;
    }
    if (s->categoria != CAT_FUNC) {
        erroSem(n, "'%s' nao e uma funcao", n->nome);
        return T_ERRO;
    }
    if (quantidade != s->nparams) {
        erroSem(n, "funcao '%s' espera %d argumento(s), mas recebeu %d",
                n->nome, s->nparams, quantidade);
        return s->tipo;
    }

    int i = 0;
    for (No *arg = n->a; arg; arg = arg->prox, i++) {
        if (!compativel(s->params[i], arg->tipoDado)) {
            erroSem(arg, "argumento %d de '%s': esperado %s, encontrado %s",
                    i + 1, n->nome, nomeTipo(s->params[i]), nomeTipo(arg->tipoDado));
        }
    }
    return s->tipo;
}

static Tipo expressao(No *n) {
    Tipo t = T_ERRO;
    switch (n->tipo) {
        case N_INT:     t = T_INT;   break;
        case N_FLOAT:   t = T_FLOAT; break;
        case N_BOOL:    t = T_BOOL;  break;
        case N_STR:     t = T_STR;   break;
        case N_IDENT:   t = tipoIdentificador(n); break;
        case N_UNARIO:  t = tipoUnario(n);        break;
        case N_BINARIO: t = tipoBinario(n);       break;
        case N_CHAMADA: t = tipoChamada(n);       break;
        default: break;
    }
    n->tipoDado = t;
    return t;
}

static void comando(No *n);

static void bloco(No *n, int novoEscopo) {
    if (novoEscopo) abrirEscopo();
    for (No *c = n->a; c; c = c->prox) comando(c);
    if (novoEscopo) fecharEscopo();
}

static void condicao(No *n, const char *nome) {
    Tipo t = expressao(n->a);
    if (t != T_BOOL && t != T_ERRO) {
        erroSem(n->a, "a condicao do %s deve ser bool, encontrado %s", nome, nomeTipo(t));
    }
}

static void comandoLet(No *n) {
    Tipo inicial = valor(n->a);
    Tipo t = n->tipoDado;

    if (t == T_NENHUM || t == T_ERRO) {
        t = inicial;
    } else if (!compativel(t, inicial)) {
        erroSem(n, "variavel '%s' declarada como %s, mas recebe valor do tipo %s",
                n->nome, nomeTipo(t), nomeTipo(inicial));
    }
    n->tipoDado = t;
    declarar(n, CAT_VAR, t, n->mutavel);
}

static void comandoAtribuicao(No *n) {
    Tipo t = valor(n->a);
    Simbolo *s = buscar(n->nome);

    if (!s) {
        erroSem(n, "variavel '%s' nao declarada", n->nome);
        return;
    }
    if (s->categoria == CAT_FUNC) {
        erroSem(n, "'%s' e uma funcao e nao pode receber valor", n->nome);
        return;
    }
    n->tipoDado = s->tipo;
    if (!s->mutavel) {
        erroSem(n, "nao e possivel atribuir a '%s' pois e imutavel (use 'let mut')", n->nome);
    }
    if (!compativel(s->tipo, t)) {
        erroSem(n, "atribuicao incompativel: '%s' e %s, mas o valor e %s",
                n->nome, nomeTipo(s->tipo), nomeTipo(t));
    }
}

static void comandoReturn(No *n) {
    if (n->a) {
        Tipo t = valor(n->a);
        if (retornoAtual == T_VOID) {
            erroSem(n, "a funcao nao declara tipo de retorno, mas 'return' devolve um valor");
        } else if (!compativel(retornoAtual, t)) {
            erroSem(n, "retorno esperado %s, encontrado %s", nomeTipo(retornoAtual), nomeTipo(t));
        }
    } else if (retornoAtual != T_VOID) {
        erroSem(n, "'return' sem valor em funcao que retorna %s", nomeTipo(retornoAtual));
    }
}

static void comando(No *n) {
    switch (n->tipo) {
        case N_LET:   comandoLet(n); break;
        case N_ATRIB: comandoAtribuicao(n); break;
        case N_IF:
            condicao(n, "if");
            bloco(n->b, 1);
            if (n->c) {
                if (n->c->tipo == N_IF) comando(n->c);
                else bloco(n->c, 1);
            }
            break;
        case N_WHILE:
            condicao(n, "while");
            bloco(n->b, 1);
            break;
        case N_RETURN: comandoReturn(n); break;
        case N_EXPR:   expressao(n->a); break;
        case N_BLOCO:  bloco(n, 1); break;
        default: break;
    }
}

static int sempreRetorna(const No *n) {
    if (!n) return 0;
    switch (n->tipo) {
        case N_RETURN:
            return 1;
        case N_BLOCO:
            for (const No *c = n->a; c; c = c->prox) {
                if (sempreRetorna(c)) return 1;
            }
            return 0;
        case N_IF:
            return n->c && sempreRetorna(n->b) && sempreRetorna(n->c);
        default:
            return 0;
    }
}

static void funcao(No *f) {
    abrirEscopo();
    retornoAtual = f->tipoDado;

    for (No *p = f->a; p; p = p->prox) declarar(p, CAT_PARAM, p->tipoDado, 0);
    bloco(f->b, 0);

    if (f->tipoDado != T_VOID && !sempreRetorna(f->b)) {
        erroSem(f, "a funcao '%s' deve retornar %s em todos os caminhos",
                f->nome, nomeTipo(f->tipoDado));
    }
    fecharEscopo();
}

void analisar(No *programa) {
    totalRegistros = 0;
    abrirEscopo();

    for (No *f = programa->a; f; f = f->prox) {
        Simbolo *s = declarar(f, CAT_FUNC, f->tipoDado, 0);
        if (!s) continue;
        for (No *p = f->a; p; p = p->prox) {
            if (s->nparams < MAX_PARAMS) s->params[s->nparams++] = p->tipoDado;
            else erroSem(p, "a funcao '%s' excede o maximo de %d parametros", f->nome, MAX_PARAMS);
        }
    }

    for (No *f = programa->a; f; f = f->prox) funcao(f);
    fecharEscopo();
}

void semanticaImprimirTabela(void) {
    printf("\n=== TABELA DE SIMBOLOS (SEMANTICA) ===\n");
    printf("%-7s %-14s %-16s %-6s %s\n", "ESCOPO", "CATEGORIA", "NOME", "TIPO", "LINHA");
    for (int i = 0; i < totalRegistros; i++) {
        Registro *r = &registros[i];
        printf("%-7d %-14s %-16s %-6s %d\n",
               r->nivel, nomeCategoria(r->categoria), r->nome, nomeTipo(r->tipo), r->linha);
    }
}

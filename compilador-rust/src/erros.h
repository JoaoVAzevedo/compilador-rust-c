#ifndef ERROS_H
#define ERROS_H

typedef enum { FASE_LEXICA, FASE_SINTATICA, FASE_SEMANTICA } Fase;

void erroRegistrar(Fase fase, int linha, int coluna, const char *fmt, ...);
int  errosTotal(void);
void errosImprimir(void);

#endif

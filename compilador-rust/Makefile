CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Isrc
ALVO    = CompiladorRust
FONTES  = src/main.c src/tokens.c src/lexer.c src/symtab.c src/erros.c \
          src/ast.c src/parser.c src/semantic.c
OBJETOS = $(FONTES:.c=.o)

all: $(ALVO)

$(ALVO): $(OBJETOS)
	$(CC) $(CFLAGS) -o $@ $(OBJETOS)

%.o: %.c $(wildcard src/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

teste: $(ALVO)
	./$(ALVO) tests/valido.rs
	./$(ALVO) tests/erro_lexico.rs
	./$(ALVO) tests/erro_sintatico.rs
	./$(ALVO) tests/erro_semantico.rs

clean:
	rm -f $(OBJETOS) $(ALVO) $(ALVO).exe

.PHONY: all teste clean

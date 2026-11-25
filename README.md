# Compilador Rust (subconjunto) - Analises Lexica, Sintatica e Semantica

Atividade Autoinstrucional - Compiladores (Profa. Amanda Danielle Lima de Oliveira)

**Integrantes:** João Vitor de Azevedo, Miguel Angelo e Moisés Felipe Gonçalves

## O que o programa faz

Recebe um arquivo `.rs` com um subconjunto de Rust e imprime:

1. Lista de tokens (com linha e coluna) e tabela de simbolos inicial (identificadores);
2. Confirmacao de que o programa pertence a gramatica (ou os erros sintaticos);
3. AST identada, ja anotada com os tipos, e tabela de simbolos semantica (por escopo);
4. Tabela de erros (lexicos, sintaticos e semanticos) com linha e coluna.

Se houver erro lexico, a analise para depois da etapa lexica. Se houver erro sintatico, para depois do parser.

## Compilar e executar

```
make
./CompiladorRust tests/valido.rs
make teste      # roda os quatro arquivos de teste
make clean
```

Sem `make` (ex.: Windows/MinGW):

```
gcc -std=c99 -Wall -Wextra -Isrc src/*.c -o CompiladorRust
CompiladorRust tests/valido.rs
```

## Estrutura

| Arquivo | Funcao |
|---|---|
| `src/tokens.h/.c` | tipos de token e nomes para impressao |
| `src/lexer.h/.c` | scanner manual (sem Lex/Flex) |
| `src/symtab.h/.c` | tabela hash de simbolos |
| `src/erros.h/.c` | tabela de erros com fase, linha e coluna |
| `src/ast.h/.c` | nos, listas e impressao identada da AST |
| `src/parser.h/.c` | parser descendente recursivo |
| `src/semantic.h/.c` | escopos, tipos e verificacoes semanticas |
| `src/main.c` | orquestra as fases |
| `tests/` | casos de teste da demonstracao |
| `docs/gramatica.md` | gramatica em EBNF |

## Subconjunto de Rust aceito

- Funcoes: `fn nome(a: i32, b: f64) -> i32 { ... }`
- Tipos: `i32`, `f64`, `bool`, `str` (simplificacao de `&str`)
- Comandos: `let [mut] x [: tipo] = expr;`, atribuicao, `if / else if / else`, `while`, `return`, blocos aninhados, chamada de funcao como comando
- Expressoes: aritmeticas, relacionais, logicas (`&&`, `||`, `!`), literais inteiros, decimais, texto e booleanos
- Comentarios `//` e `/* */`

## Verificacoes semanticas

Declaracao antes do uso, redeclaracao no mesmo escopo, escopos aninhados, compatibilidade de tipos (atribuicoes, operadores, condicoes, retorno), aridade e tipos dos argumentos, atribuicao a variavel imutavel e retorno em todos os caminhos.

## Limitacoes

Sem expressao final sem `;` (so `return`), sem `let` sem valor inicial, sem `println!`/macros, sem conversao implicita entre `i32` e `f64`, sem arrays, structs ou referencias.

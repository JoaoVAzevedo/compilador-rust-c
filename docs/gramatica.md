# Gramatica (EBNF)

`{ x }` = zero ou mais; `[ x ]` = opcional; `|` = alternativa. Terminais em maiusculas ou entre aspas.

```
programa     = { funcao } EOF ;

funcao       = "fn" IDENT "(" [ parametros ] ")" [ "->" tipo ] bloco ;
parametros   = parametro { "," parametro } ;
parametro    = IDENT ":" tipo ;
tipo         = "i32" | "f64" | "bool" | "str" ;

bloco        = "{" { comando } "}" ;
comando      = declaracao | atribuicao | condicional | repeticao
             | retorno | bloco | expr_cmd ;

declaracao   = "let" [ "mut" ] IDENT [ ":" tipo ] "=" expressao ";" ;
atribuicao   = IDENT "=" expressao ";" ;
condicional  = "if" expressao bloco [ "else" ( condicional | bloco ) ] ;
repeticao    = "while" expressao bloco ;
retorno      = "return" [ expressao ] ";" ;
expr_cmd     = expressao ";" ;

expressao    = ou ;
ou           = e { "||" e } ;
e            = igualdade { "&&" igualdade } ;
igualdade    = relacional { ( "==" | "!=" ) relacional } ;
relacional   = soma { ( "<" | ">" | "<=" | ">=" ) soma } ;
soma         = produto { ( "+" | "-" ) produto } ;
produto      = unaria { ( "*" | "/" | "%" ) unaria } ;
unaria       = ( "-" | "!" ) unaria | primaria ;
primaria     = INT_LIT | FLOAT_LIT | STR_LIT | "true" | "false"
             | IDENT [ "(" [ argumentos ] ")" ]
             | "(" expressao ")" ;
argumentos   = expressao { "," expressao } ;
```

Observacoes:

- A gramatica e LL(1), exceto em `comando`: um `IDENT` pode iniciar `atribuicao` ou `expr_cmd`; o parser olha o token seguinte (`=`) para decidir (LL(2)).
- A precedencia (do menor para o maior) e: `||`, `&&`, `== !=`, `< > <= >=`, `+ -`, `* / %`, unarios. Todos os binarios associam a esquerda.
- Comentarios e espacos sao descartados pelo analisador lexico.

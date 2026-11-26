// programa valido: funcoes, escopos, if/else, while e chamadas
fn soma(a: i32, b: i32) -> i32 {
    return a + b;
}

fn fatorial(n: i32) -> i32 {
    let mut resultado: i32 = 1;
    let mut i = 2;
    while i <= n {
        resultado = resultado * i;
        i = i + 1;
    }
    return resultado;
}

fn maior(a: i32, b: i32) -> i32 {
    if a > b {
        return a;
    } else {
        return b;
    }
}

fn imprimir(msg: str) {
    let copia = msg;
    return;
}

fn principal() {
    let x: i32 = soma(2, 3);
    let mut y = 3.14;
    let ativo = true && !false;
    let titulo = "ola";
    y = y * 2.0;
    if x >= 5 && ativo {
        let f = fatorial(x);
        let m = maior(f, 10);
    } else if x == 0 {
        imprimir(titulo);
    }
    imprimir("fim");
}

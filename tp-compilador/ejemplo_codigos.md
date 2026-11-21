# EJEMPLO CODIGO

## PROGRAMA CORRECTO:

```
    inicio
        leer(a,b,c);
        d:=a+b-c;
    fin
```

## ERRORES LEXICO:
### Caracter no reconocido

```
    inicio
        c:= a + b @;
    fin
```

### Identificador largo
```
    inicio
        leer(a23456789012345678901234567890123);
    fin
```

## ERRORES SINTACTICO:
### Falta ';'
```
    inicio
        leer(a,b,f);
        c:= a + b
        escribir(f);
    fin
```


### Falta 'fin'

```
    inicio
        leer(ba);
    
```

### Falta ')'

```
    inicio
        leer(z);
        leer(b;
        z:=b+1;
    fin
```
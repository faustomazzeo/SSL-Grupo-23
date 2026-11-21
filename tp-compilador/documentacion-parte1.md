# TP3 – Flex y Bison – Parte 1: Compilador del lenguaje Micro

## CARATULA


### Titulo: TP Compilador (3)
### Integrantes: Marcos Ortiz Olivero, Tomas Jack Yedid, Fausto Mazzeo
### Materia: Sintaxis y Semántica de los Lenguajes
### Comision: K2002
### Año: 2026
### Fecha de entrega: 20-11-2026


## 1. Objetivo

Construir con flex y bison un compilador del lenguaje Micro que realice:

- Análisis léxico (flex): reconoce los tokens del lenguaje.
- Análisis sintáctico (bison): verifica que el programa respete la gramática de Micro.
- Análisis semántico (acciones de bison): mantiene una tabla de símbolos y genera código para una máquina virtual.

Además se personalizaron los mensajes de error léxicos, sintácticos y semánticos.

Se utilizo como base el archivo compilador de micro en lenguaje C provisto en el aula virtual, traduciendolo al formato de flex y bison.


## 2. Análisis léxico

### 2.1 Tokens

| Token |  Reconoce | Alias |
|---|---|---|
| `INICIO`, `FIN`, `LEER`, `ESCRIBIR` | `inicio`, `fin`, `leer`, `escribir` | idem |
| `ID` | letra seguida de letras o dígitos | `identificador` |
| `CONSTANTE` | uno o más dígitos | `constante` |
| `ASIGNACION` | `:=` | idem |
| `SUMA`, `RESTA` | `+`, `-` | idem |
| `PARENIZQ`, `PARENDER` | `(`, `)` | idem |
| `COMA`, `PUNTOCOMA` | `,`, `;` | idem |
| `ERRORLEXICO` | cualquier carácter no reconocido, o un identificador/constante de más de 32 caracteres | - |
| `FIN_ARCHIVO` (token 0) | fin de la entrada | `fin de archivo` |

### 2.2 Decisiones de diseño

- Las palabras reservadas se escriben antes que la regla de `ID`. Cuando dos reglas reconocen el mismo texto con igual longitud, flex elige la que aparece primero. Así `inicio` se devuelve como `INICIO` y no como un identificador.
- Se distinguen mayúsculas de minúsculas. `INICIO` o `Fin` se tratan como identificadores, no como palabras reservadas.
- Los blancos y saltos de línea se ignoran. La opción `yylineno` permite informar el número de línea en cada error.
- No hay token explícito de fin de archivo en el scanner. Cuando se acaba la entrada, `yylex()` devuelve `0`, y bison lo interpreta como el token `FIN_ARCHIVO`.
- Los identificadores y constantes de más de 32 caracteres se informan como error léxico, como el ejemplo en C.
- Para `ID` y `CONSTANTE`, el scanner guarda una copia del texto en `yylval.str`. Las constantes no se convierten a número, porque el compilador no evalúa las expresiones, sino que solo imprime su texto en las instrucciones.

## 3. Análisis sintáctico

### 3.1 Gramática

La gramática se tomó de los comentarios del compilador original. Las repeticiones se tradujeron a reglas recursivas.

```
programa             → INICIO listaSentencias FIN
listaSentencias      → sentencia | listaSentencias sentencia
sentencia            → ID ASIGNACION expresion PUNTOCOMA
                     | LEER PARENIZQ listaIdentificadores PARENDER PUNTOCOMA
                     | ESCRIBIR PARENIZQ listaExpresiones PARENDER PUNTOCOMA
listaIdentificadores → ID | listaIdentificadores COMA ID
listaExpresiones     → expresion | listaExpresiones COMA expresion
expresion            → primaria
                     | expresion SUMA primaria
                     | expresion RESTA primaria
primaria             → ID | CONSTANTE | PARENIZQ expresion PARENDER
```

### 3.2 Por qué recursión a izquierda

- En las expresiones, la recursión por la izquierda hace que la resta se agrupe de izquierda a derecha: `a - b - c` se interpreta como `(a - b) - c`. Con recursión por la derecha se interpretaría como `a - (b - c)` y el resultado sería incorrecto.
- En las listas, hace que las acciones semánticas se ejecuten en el mismo orden en que están escritos los elementos. Es decir `leer(a, b, c)` genera los `Read` de `a`, `b` y `c` en ese orden.


## 4. Análisis semántico

### 4.1 Valores semánticos

La unión de bison tiene un único campo: `char *str`. Los tokens `ID` y `CONSTANTE`, y los símbolos `expresion` y `primaria`, llevan como valor una cadena con el nombre del operando. Ese nombre puede ser:

- el texto de una variable,
- el texto de una constante,
- o el nombre de un temporal (`Temp&1`, `Temp&2`, etc) donde quedó el resultado de una operación anterior.

El valor que sube por el árbol no es el resultado numérico, sino la etiqueta del lugar donde la máquina virtual guardará ese resultado.

### 4.2 Tabla de símbolos

Es un arreglo de cadenas (`tabla`, hasta 1000 entradas de 40 caracteres) con un contador. Guarda las variables del programa y también los temporales que se generan. Se manejan con:

- `buscar(nombre)`: devuelve 1 si el nombre ya está en la tabla y 0 si no.
- `chequear(nombre)`: si el nombre no está en la tabla, lo agrega y genera la instrucción `Declara`.

### 4.3 Rutinas semánticas implementadas

Rutina 1 – Declaración automática de variables (`chequear`). Se llama cada vez que aparece un identificador: en `leer`, en el lado izquierdo de una asignación y dentro de las expresiones. La primera vez que se ve el nombre se declara. Las apariciones siguientes no repiten esto.

Rutina 2 – Generación de operaciones infijas (`gen_infijo`). Se ejecuta una vez por cada `+` o `-`. Hace cuatro cosas:

1. Crea un nombre nuevo de temporal con un contador estático (por ej `Temp&1`).
2. Lo declara en la tabla con `chequear`.
3. Genera la instrucción (`Sumar` o `Restar`) con los dos operandos y el temporal como destino.
4. Devuelve el nombre del temporal, que se convierte en el valor de la expresión y sirve de operando en la operación siguiente.

Generacion de instrucciones (`generar()`):

| Instrucción | Cuándo se genera |
|---|---|
| `Declara x,Entera,` | primera aparición de un identificador o temporal |
| `Read x,Entera,` | por cada identificador de `leer(...)` |
| `Write x,Entera,` | por cada expresión de `escribir(...)` |
| `Almacena origen,destino,` | al terminar una asignación |
| `Detiene ,,` | al reconocer el `fin` del programa |

### 4.4 Acción en medio de la regla de la asignación

En la regla `ID ASIGNACION expresion PUNTOCOMA` hay una acción intermedia, justo después de reconocer el `ID`, que llama a `chequear`. Así la variable de la izquierda se declara antes de generar el código de la expresión, igual que en el compilador original. La acción final genera el `Almacena`.

### 4.5 Memoria

Cada cadena que sube por el árbol se reserva dinámicamente (`strdup` en el scanner, `copiar` en el parser) y se libera con `free` en la regla que la consume.

## 5. Manejo de errores

### 5.1 Errores léxicos

Los informa el scanner, con un mensaje propio, la línea y el carácter involucrado:

```
ERROR LEXICO - Linea 2: caracter no reconocido '@'.
ERROR LEXICO - Linea 3: identificador demasiado largo.
ERROR LEXICO - Linea 3: constante demasiado larga.
```

Después de informar, el scanner devuelve el token `ERRORLEXICO`. Ninguna regla de la gramática lo acepta, así que el análisis termina con error.

- El parser también detectaría el token inesperado y agregaría su propio error. Para que solo se refleje un mensaje por problema, el scanner marca una variable global compartida (`error_lexico_detectado`) y `yyerror` no imprime nada si esa marca está activa.

### 5.2 Errores sintácticos

Los detecta bison y los muestra `yyerror` adaptado (personalizacion), con el número de línea:

```
ERROR SINTACTICO - LINEA 3 :  no se esperaba fin, se esperaba ; o + o -
```

La personalización tiene dos partes:

- Alias de tokens. En el `.y` cada token tiene un nombre legible entre comillas (por ejemplo `%token PUNTOCOMA ";"`). Los mensajes muestran `;` en lugar de `PUNTOCOMA`.
- Traducción de las palabras fijas. `yyerror` copia el mensaje de bison a un buffer temporal y reemplaza palabras/fragmentos (como `syntax error,` o `expecting`) en español. La función auxiliar `reemplazar_palabra` controla que el resultado no desborde el buffer.

El mensaje indica el token encontrado y los tokens que habrían sido válidos en dicho contexto.

La línea indicada es la del token que el parser leyó al detectar el error, que puede ser posterior a la línea donde el programador omitió algo (como suele pasar en C)
### 5.3 Errores semánticos

El único error semántico que detecta el compilador es el desborde de la tabla de símbolos:

```
ERROR SEMANTICO - Linea N : tabla de simbolos llena.
```

## 6. Limitaciones y comportamientos a tener en cuenta

- El análisis se detiene en el primer error léxico o sintáctico.
- Como las instrucciones se generan mientras se analiza, las que corresponden a lo ya reconocido se imprimen antes de que aparezca el error (aparece un codigo parcial erroneo).
- Las variables se declaran al primer uso, de modo que un programa como `c := c + 1;` es aceptado aunque `c` no haya recibido un valor antes.
- Identificadores que empiezan con dígito (`1a`) se leen como una constante seguida de un identificador. El error aparece en el parser y no en el scanner.
- Solo enteros, y solo suma y resta.
- Los errores no salen todos por el mismo canal: los sintácticos se escriben por la salida de error (`stderr`) y los léxicos y semánticos por la salida estándar (`stdout`).
- Los identificadores y constantes permitidos son de hasta 32 caracteres. La tabla de símbolos admite 1000 entradas, que incluyen los temporales.

## 7. Manual de usuario

### 7.1 Requisitos

- `flex`
- `bison`
- un compilador de C (`gcc`)

En Windows se pueden usar WSL, MSYS2 o MinGW. En esos entornos el ejecutable puede generarse como `micro.exe`.

### 7.2 Compilación

Ejecutar los comandos en este orden, en la carpeta con los dos archivos fuente:

```
bison -d parser-micro.y
flex scanner-micro.l
gcc lex.yy.c parser-micro.tab.c -o micro
```

- `bison -d` genera `parser-micro.tab.c` y `parser-micro.tab.h`. Tiene que ejecutarse primero ya que el scanner incluye ese encabezado.
- `flex` genera `lex.yy.c`.
- `gcc` compila ambos y produce el ejecutable `micro`.

### 7.3 Ejecución

El programa lee el código Micro utilizando:

```
./micro < programa.m
```

### 7.4 Salida

*Programa correcto*. Se imprimen las instrucciones generadas, una por línea, y al final un mensaje de confirmación. Utilizando como ejemplo el siguiente codigo:
```
inicio
    leer(a, b);
    c := a + b - 1;
    escribir(c, a + 2);
fin
```
se obtiene
```
Declara a,Entera,
Read a,Entera,
Declara b,Entera,
Read b,Entera,
Declara c,Entera,
Declara Temp&1,Entera,
Sumar a,b,Temp&1
Declara Temp&2,Entera,
Restar Temp&1,1,Temp&2
Almacena Temp&2,c,
Write c,Entera,
Declara Temp&3,Entera,
Sumar a,2,Temp&3
Write Temp&3,Entera,
Detiene ,,

Analisis finalizado correctamente.
```

*Programa con errores*. Se muestra el mensaje de error correspondiente y no aparece el mensaje de confirmación (info. en sección 5).

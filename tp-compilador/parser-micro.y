%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(const char *mensaje);
extern int yylineno;

int error_lexico_detectado = 0;
int error_semantico_detectado = 0;

/* Tabla de simbolos */
#define MAX_SIMBOLOS 1000
char tabla[MAX_SIMBOLOS][40];
int cantidad_simbolos = 0;

/* Rutinas semanticas */
int buscar(const char *nombre);
void chequear(const char *nombre);
void generar(const char *instruccion,
             const char *a, const char *b, const char *c);
char *copiar(const char *texto);
char *gen_infijo(const char *a, const char *b, int operador);


/* Errores */
#define MAX_ERR_LEN 512

void reemplazar_palabra(char *buffer, const char *buscar, const char *reemplazo);

%}

%define parse.error verbose

%union {
    char *str;
}

%token <str> ID "identificador"
%token <str> CONSTANTE "constante"
%token INICIO "inicio"
%token FIN "fin"
%token LEER "leer"
%token ESCRIBIR "escribir"
%token PARENIZQ "("
%token PARENDER ")"
%token PUNTOCOMA ";"
%token COMA ","
%token ASIGNACION ":="
%token SUMA "+"
%token RESTA "-"
%token ERRORLEXICO
%token FIN_ARCHIVO 0 "fin de archivo"

%type <str> expresion primaria

%start programa

%%

/* Estructura general del programa */

programa:
    INICIO listaSentencias FIN
    {
        generar("Detiene", "", "", "");
    }
    ;

/* Una o mas sentencias */

listaSentencias:
    sentencia
    | listaSentencias sentencia
    ;

/* Tipos de sentencias de Micro */

sentencia:
    ID
    {
        chequear($1);
    }
    ASIGNACION expresion PUNTOCOMA
    {
        generar("Almacena", $4, $1, "");
        free($1);
        free($4);
    }

    | LEER PARENIZQ listaIdentificadores PARENDER PUNTOCOMA

    | ESCRIBIR PARENIZQ listaExpresiones PARENDER PUNTOCOMA
    ;

/* Lista de identificadores para leer */

listaIdentificadores:
    ID
    {
        chequear($1);
        generar("Read", $1, "Entera", "");
        free($1);
    }

    | listaIdentificadores COMA ID
    {
        chequear($3);
        generar("Read", $3, "Entera", "");
        free($3);
    }
    ;

/* Lista de expresiones para escribir */

listaExpresiones:
    expresion
    {
        generar("Write", $1, "Entera", "");
        free($1);
    }

    | listaExpresiones COMA expresion
    {
        generar("Write", $3, "Entera", "");
        free($3);
    }
    ;

/* Expresiones aritmeticas */

expresion:
    primaria
    {
        $$ = $1;
    }

    | expresion SUMA primaria
    {
        $$ = gen_infijo($1, $3, SUMA);
        free($1);
        free($3);
    }

    | expresion RESTA primaria
    {
        $$ = gen_infijo($1, $3, RESTA);
        free($1);
        free($3);
    }
    ;

/* Elementos que pueden formar una expresion */

primaria:
    ID
    {
        chequear($1);
        $$ = copiar($1);
        free($1);
    }

    | CONSTANTE
    {
        $$ = copiar($1);
        free($1);
    }

    | PARENIZQ expresion PARENDER
    {
        $$ = $2;
    }
    ;

%%

/* Muestra los errores sintacticos */

void yyerror(const char *mensaje)
{
    if (!error_lexico_detectado) {
        char mi_buffer[MAX_ERR_LEN];

        strncpy(mi_buffer, mensaje, MAX_ERR_LEN - 1);
        mi_buffer[MAX_ERR_LEN - 1] = '\0';

        reemplazar_palabra(mi_buffer, "syntax error,", "");
        reemplazar_palabra(mi_buffer, "unexpected", "no se esperaba");
        reemplazar_palabra(mi_buffer, "expecting", "se esperaba");
        reemplazar_palabra(mi_buffer, " or ", " o ");

        fprintf(stderr, "ERROR SINTACTICO - LINEA %d : %s\n", yylineno, mi_buffer);
    }
}



/* Funcion auxiliar errores */

void reemplazar_palabra(char *buffer, const char *fragmento_buscado, const char *reemplazo) {
    char temp[MAX_ERR_LEN];
    char *pos;
    int len_buscar = strlen(fragmento_buscado);
    int len_reemplazo = strlen(reemplazo);

    while ((pos = strstr(buffer, fragmento_buscado)) != NULL) {
        if (strlen(buffer) - len_buscar + len_reemplazo >= MAX_ERR_LEN) {
            break;
        }
        int prefijo_len = pos - buffer;
        strncpy(temp, buffer, prefijo_len);
        temp[prefijo_len] = '\0';

        strcat(temp, reemplazo);
        strcat(temp, pos + len_buscar);

        strcpy(buffer, temp);
    }
}


/* Busca un identificador en la tabla */

int buscar(const char *nombre)
{
    int i;

    for (i = 0; i < cantidad_simbolos; i++) {
        if (strcmp(tabla[i], nombre) == 0)
            return 1;
    }

    return 0;
}

/* Declara un identificador si todavia no existe */

void chequear(const char *nombre)
{
    if (!buscar(nombre)) {

        if (cantidad_simbolos >= MAX_SIMBOLOS) {
            printf("ERROR SEMANTICO - Linea %d : "
                   "tabla de simbolos llena.\n", yylineno);

            error_semantico_detectado = 1;
            return;
        }

        strcpy(tabla[cantidad_simbolos], nombre);
        cantidad_simbolos++;

        generar("Declara", nombre, "Entera", "");
    }
}

/* Genera una instruccion de codigo intermedio */

void generar(const char *instruccion,
             const char *a, const char *b, const char *c)
{
    printf("%s %s,%s,%s\n", instruccion, a, b, c);
}

/* Crea una copia de una cadena */

char *copiar(const char *texto)
{
    char *copia = malloc(strlen(texto) + 1);

    if (copia == NULL) {
        fprintf(stderr, "Error: no se pudo reservar memoria.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(copia, texto);
    return copia;
}

/* Genera una operacion de suma o resta */

char *gen_infijo(const char *a, const char *b, int operador)
{
    static int contador = 1;

    char temporal[40];
    char *instruccion;

    if (operador == SUMA)
        instruccion = "Sumar";
    else
        instruccion = "Restar";

    sprintf(temporal, "Temp&%d", contador++);

    chequear(temporal);

    generar(instruccion, a, b, temporal);

    return copiar(temporal);
}

/* Punto de entrada del programa */

int main(void)
{
    int resultado = yyparse();

    if (resultado != 0 ||
        error_lexico_detectado ||
        error_semantico_detectado) {
        return EXIT_FAILURE;
    }

    printf("\nAnalisis finalizado correctamente.\n");

    return EXIT_SUCCESS;
}
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser-micro.tab.h"

    /* Variable compartida con Bison para evitar errores duplicados. */
extern int error_lexico_detectado;    

%}

    /* Opciones del analizador léxico */
%option noyywrap
%option yylineno
%option nounput
%option noinput

    /* Expresiones regulares */
DIGITO          [0-9]
LETRA           [a-zA-Z]
IDENTIFICADOR   {LETRA}({LETRA}|{DIGITO})*
CONSTANTE       {DIGITO}+

%%

    /* Palabras reservadas */

"inicio"        { return INICIO; }
"fin"           { return FIN; }
"leer"          { return LEER; }
"escribir"      { return ESCRIBIR; }

    /* Identificadores */

{IDENTIFICADOR} {
    if (yyleng > 32) {
        printf("ERROR LEXICO - Linea %d: identificador demasiado largo.\n",
               yylineno);
        error_lexico_detectado = 1;
        return ERRORLEXICO;
    }

    yylval.str = strdup(yytext);

    if (yylval.str == NULL) {
        fprintf(stderr, "ERROR: No se pudo reservar memoria.\n");
        exit(EXIT_FAILURE);
    }

    return ID;
}

    /* Constantes enteras */

{CONSTANTE} {
    if (yyleng > 32) {
        printf("ERROR LEXICO - Linea %d: constante demasiado larga.\n",
               yylineno);
        error_lexico_detectado = 1;
        return ERRORLEXICO;
    }

    yylval.str = strdup(yytext);

    if (yylval.str == NULL) {
        fprintf(stderr, "ERROR: No se pudo reservar memoria.\n");
        exit(EXIT_FAILURE);
    }

    return CONSTANTE;
}

    /* Operadores */

":="            { return ASIGNACION; }
"+"             { return SUMA; }
"-"             { return RESTA; }

    /* Parentesis */

"("             { return PARENIZQ; }
")"             { return PARENDER; }

    /* Separadores */

","             { return COMA; }
";"             { return PUNTOCOMA; }

    /* Espacios en blanco y saltos de linea */

[ \t\r\n]+      { /* Se ignoran */ }

    /* Caracteres no reconocidos */

. {
    printf("ERROR LEXICO - Linea %d: caracter no reconocido '%s'.\n",
           yylineno, yytext);

    error_lexico_detectado = 1;
    return ERRORLEXICO;
}

%%

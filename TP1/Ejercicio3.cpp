#include <stdio.h>
#include <iostream>
#include "Ejercicio2.cpp"
#include <stack>
using namespace std;


typedef enum {
    ESTADOINICIAL,
    ESTADOSIGNOINICIAL,
    ESTADONUMERO,
    ESTADOOPERADORLEIDO,
    ESTADORECHAZO,
    ESTADOTERMINADO
} Estados;

typedef enum {
    CARACTERDIGITO,
    CARACTERSUMARESTA,
    CARACTERMULTIPLICACION,
    CARACTERFINCADENA,
    CARACTERINVALIDO
} ColumnaCaracteres;

typedef enum {
    OPERADORSUMA,
    OPERADORRESTA,
    OPERADORMULTIPLICACION,
    OPERADORFIN
} Operadores;

typedef enum {
    PRIORIDADALTA, // prioridad de la multiplicacion
    PRIORIDADBAJA, // prioridad de la suma y resta
    PRIORIDADFIN, // prioridad del ultimo termino (no tiene asociada ninguna operacion)
} Prioridad;

typedef struct {
    int numero;
    int operador;
    int prioridad;
} Termino;

bool esDigito(char c){
	return (c >= '0' && c <= '9');
}
bool esSumaResta(char c){
	return (c == '+' || c == '-');
}
bool esMultiplicacion(char c){
	return (c == '*');
}
bool esFinCadena(char c){
	return (c == '\0');
}


int transicion(char caracter){
    if (esDigito(caracter)) return CARACTERDIGITO;
    if (esSumaResta(caracter)) return CARACTERSUMARESTA;
    if (esMultiplicacion(caracter)) return CARACTERMULTIPLICACION;
    if (esFinCadena(caracter)) return CARACTERFINCADENA;
    else return CARACTERINVALIDO;
}

bool verificarError(int estadoActual, int estadoProximo){
    // hay error si el caracter leido lleva a un estado de rechazo, o si se lee el caracter
    // de fin de cadena ('\0') cuando se estaba en un estado "intermedio" (no se estaba leyendo un numero).
    // Ejemplo: "+30-" se lee el '0' (estado numerico), se pasa al '-' (estado no numerico o intermedio) y se termina en '\0'
    return (estadoProximo == ESTADORECHAZO ||
        (estadoProximo == ESTADOTERMINADO && estadoActual != ESTADONUMERO));
}

int elevadoA(int exponente, int base){
    if (exponente == 0) return 1;
    return (base * elevadoA(exponente-1, base));
}

int obtenerNumero(char cadena[], int& offset){ //int offsetInicial = offset; // necesario para la segunda pasada de lectura de la cadena
    int len = 0; // tamanio del numero
    int numero = 0; // entero que representa la cadena del numero
    int signo = 0; // 0 es positivo, 1 es negativo (solo es relevante en caso de que la cadena arranque con "-30..." o "+20...")
    if (offset == 0 && esSumaResta(cadena[offset])){
        if (cadena[offset] == '-') signo = 1;
        offset++; // adelantamos el offset para ir directo al numero
    }
    int offsetInicial = offset; // necesario para la segunda lectura de la cadena
    while (esDigito(cadena[offset])){ // va a leer el tamanio del numero (importante para las unidades) y va a ir adelantando el offset
        // (importante para el while principal)
        len++;
        offset++;
    }
    while (len>0){
        int digitoConUnidad = funcion2(cadena[offsetInicial]) * (elevadoA(len-1,10)); // la funcion2 pasa de caracter a numero entero,
            // y se lo multiplicamos por la unidad que representa ese digito
        numero += digitoConUnidad; // le sumamos lo que equivale, va acumulando las distintas unidades
        len--; // disminuimos el len
        offsetInicial++; // aumentamos el offset
    }

    if (signo == 1) numero *= (-1); // chequea que el numero haya sido inicialmente negativo
    return numero;
}

int obtenerOperador(char op){
    if (op == '+') return OPERADORSUMA;
    if (op == '-') return OPERADORRESTA;
    if (op == '*') return OPERADORMULTIPLICACION;
    if (op == '\0') return OPERADORFIN;
    cout << "\n Error Validacion operador \n"; return -1; // no deberia pasar nunca si la cadena ya fue validada por el automata
}

int obtenerPrioridad(int op){
    if (op == OPERADORMULTIPLICACION) return PRIORIDADALTA;
    if (op == OPERADORFIN) return PRIORIDADFIN;
    else return PRIORIDADBAJA;
}

int operar(int numPila, int numAct, int operador) {
    int resultado = 0;
    if (operador == OPERADORSUMA) resultado = numPila + numAct;
    if (operador == OPERADORRESTA) resultado = numPila - numAct;
    if (operador == OPERADORMULTIPLICACION) resultado = numPila * numAct;
    return resultado;
}


int main(){
    int i=0;
    int j=0;
    int offset=0;
    bool huboError=false;
    char cadena[100];

    int tabla[6][5] = {
        {2, 1, 4, 5, 4},
        {2, 4, 4, 5, 4},
        {2, 3, 3, 5, 4},
        {2, 4, 4, 5, 4},
        {4, 4, 4, 5, 4},
        {5, 5, 5, 5, 5}
    };

    cout << "Ingrese una cuenta: " << endl;
	fscanf(stdin, "%99s", cadena);

    while (!huboError && i != ESTADOTERMINADO) {
        char caracter = cadena[offset]; // obtenemos el caracter de la cadena
        j = transicion(caracter); // nos fijamos a que columna pertenece para despues hacer la transicion al proximo estado
        int ii = tabla[i][j]; // obtenemos el proximo estado segun la tabla, el estado en el que estamos y el caracter que se registro

        if (verificarError(i,ii)){
            huboError = true;
        }

        offset++;
        i = ii;
    }

    cout << endl << "Cadena analizada: " << cadena;
    if (huboError) {
        cout << "    - con error/es. Cadena Invalida" << endl;
        return -1;
    } else {
        cout << "    - sin errores. Cadena Valida" << endl;
    }


    offset = 0;
    Termino terminoActual;
    bool obtuveResultado = false;
    stack<Termino> pila;
    while (!obtuveResultado){
        terminoActual.numero = obtenerNumero(cadena, offset); // esta funcion devuelve el numero entero y dentro manipula el offset
        terminoActual.operador = obtenerOperador(cadena[offset]); // obtenemos el operador asociado actual
        terminoActual.prioridad = obtenerPrioridad(terminoActual.operador); // determinamos la prioridad del termino/operacion

        while(!pila.empty() && pila.top().prioridad<=terminoActual.prioridad) { // mientras la pila no este vacia y el termino que este
            // arriba de todo de la pila tenga prioridad mayor o igual que el termino actual
            Termino temp = pila.top(); // toma el termino apilado que tiene mayor prioridad
            pila.pop();
            int resultadoTemporal = operar(temp.numero, terminoActual.numero, temp.operador); // opera el numero apilado con el numero
            // del termino actual usando la operacion del termino apilado
            terminoActual.numero = resultadoTemporal; // registra el resultado en el termino actual
        }

        pila.push(terminoActual);

        obtuveResultado = (terminoActual.operador == OPERADORFIN);  // si no tiene operador asociado (el "operador es '\0'"), quiere
        // decir que se llego al final de la cadena y ademas el bucle interior termino de resolver todas las cuentas

        offset++;
    }

    cout << "Resultado: " << pila.top().numero << endl;

    return 0;
}
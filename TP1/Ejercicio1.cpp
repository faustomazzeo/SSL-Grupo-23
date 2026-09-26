#include <stdio.h>
#include <iostream>
using namespace std;

typedef enum {
	ESTADOINICIAL,
	ESTADODECISIONCERO,
	ESTADODECIMALES,
	ESTADODECISIONSIGNADO,
	ESTADOOCTAL,
	ESTADOHEXATERMINADO,
	ESTADOHEXA,
	ESTADORECHAZO,
	ESTADOTERMINADO
} Estados;

typedef enum {
	CARACTERCERO,
	CARACTEROCTAL,
	CARACTERDECIMAL,
	CARACTERHEXA,
	CARACTERSIMBOLODECIMAL,
	CARACTERSIMBOLOHEXA,
	CARACTERSIMBOLOSEPARADOR,
	CARACTERSIMBOLOFINCADENA,
	CARACTERINVALIDO
} ColumnaCaracteres;


bool esCero(char c){
	return (c == '0');
}

bool esDigitoOctal(char c){
	return (c >= '1' && c <= '7');
}

bool esDigitoDecimal(char c){
	return (c == '8' || c == '9');
}

bool esDigitoHexa(char c){
	return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool esSumaResta(char c){
	return (c == '+' || c == '-');
}

bool esSimboloHexa(char c){
	return (c == 'x' || c == 'X');
}

bool esSimboloSeparador(char c){
	return (c == '@');
}

bool esFinCadena(char c){
	return (c == '\0');
}




int transicion(char caracter){
	if (esCero(caracter)) return CARACTERCERO;
	if (esDigitoOctal(caracter)) return CARACTEROCTAL;
	if (esDigitoDecimal(caracter)) return CARACTERDECIMAL;
	if (esDigitoHexa(caracter)) return CARACTERHEXA;
	if (esSumaResta(caracter)) return CARACTERSIMBOLODECIMAL;
	if (esSimboloHexa(caracter)) return CARACTERSIMBOLOHEXA;
	if (esSimboloSeparador(caracter)) return CARACTERSIMBOLOSEPARADOR;
	if (esFinCadena(caracter)) return CARACTERSIMBOLOFINCADENA;
	else return CARACTERINVALIDO;
}



void registrarNumero(int ultimoEstado, int cantidadNumeros[], int &totalNumeros){
	int cantidad = 0;
	if (ultimoEstado == ESTADOOCTAL) {
		totalNumeros++;
		cantidad = cantidadNumeros[0];
		cantidad++;
		cantidadNumeros[0] = cantidad;
		cout << totalNumeros << ". Octal registrado. Cantidad: " << cantidad << endl;
	} else if (ultimoEstado == ESTADODECIMALES) {
		totalNumeros++;
		cantidad = cantidadNumeros[1];
		cantidad++;
		cantidadNumeros[1] = cantidad;
		cout << totalNumeros << ". Decimal registrado. Cantidad: " << cantidad << endl;
	} else if (ultimoEstado == ESTADOHEXA) {
		totalNumeros++;
		cantidad = cantidadNumeros[2];
		cantidad++;	
		cantidadNumeros[2] = cantidad;
		cout << totalNumeros << ". Hexa registrado. Cantidad: " << cantidad << endl;
	} else {
		cout << "ERROR: me mandaron a registrar un numero que su estado anterior no coincide con los tipos de numeros" << endl;
	}
}

bool verificarError(int estadoActual, int estadoProximo){
	return (estadoProximo == ESTADORECHAZO || // si el caracter lleva a un estado invalido
		(estadoActual == ESTADODECISIONCERO && (estadoProximo == ESTADOINICIAL || estadoProximo == ESTADOTERMINADO)) || // si se registra "0@" o "0" (con fin de cadena '\0')
		(estadoActual == ESTADODECISIONSIGNADO && (estadoProximo == ESTADOINICIAL || estadoProximo == ESTADOTERMINADO)) || // si se registra "-@", "+@", "-" o "+"
		(estadoActual == ESTADOHEXATERMINADO && (estadoProximo == ESTADOINICIAL || estadoProximo == ESTADOTERMINADO)) || // si se registra "0x@", "0X@", "0x" o "0X"
		(estadoActual == ESTADOINICIAL && estadoProximo == ESTADOINICIAL) || // si se registra "...@@" o "...@@..."
		(estadoActual == ESTADOINICIAL && estadoProximo == ESTADOTERMINADO));  // si se registra "...@"
}



int main(){
	bool huboError = false;
	int cantErr = 0;
	int cantidadNumeros[3] = {0, 0, 0}; // octales, decimales, hexa
	int totalNumeros = 0;
	
	int i=0;
	int j=0;
	int offset=0;
	int tabla[9][9]={
		{1, 2, 2, 7, 3, 7, 0, 8, 7},
		{7, 4, 7, 7, 7, 5, 0, 8, 7},
		{2, 2, 2, 7, 7, 7, 0, 8, 7},
		{7, 2, 2, 7, 7, 7, 0, 8, 7},
		{4, 4, 7, 7, 7, 7, 0, 8, 7},
		{6, 6, 6, 6, 7, 7, 0, 8, 7},
		{6, 6, 6, 6, 7, 7, 0, 8, 7},
		{7, 7, 7, 7, 7, 7, 0, 8, 7},
		{8, 8, 8, 8, 8, 8, 8, 8, 8}
	};
	
	char cadena[100];
	
	cout << "Ingrese una cadena: " << endl;
	fscanf(stdin, "%99s", cadena);
	
	while (i!=8){
		char caracter = cadena[offset]; // obtiene un caracter de la cadena
		j=transicion(caracter); // calcula a que columna pertenece el caracter
		int ii=tabla[i][j]; // obtiene el proximo estado
		
		if (verificarError(i,ii)) { // verifica si hubo error y setea el flag
			huboError=true;
		}
		
		if ((ii == ESTADOTERMINADO || ii == ESTADOINICIAL) && !huboError) { // en caso de que el proximo estado sea de finalizacion o se haya registrado un caracter separador (en ambos casos sin haber error), registra el numero
			registrarNumero(i, cantidadNumeros, totalNumeros);
		} else if ((ii == ESTADOTERMINADO || ii == ESTADOINICIAL) && huboError) { // se "cerro" un numero pero se encontro error en el camino
			cantErr++;
			cout << "Error lexico detectado (Numero invalido). Cantidad de errores: " << cantErr << endl;
		}
		
		if (j == 6) { // si se registra el simbolo separador, hay que reiniciar el flag
			huboError=false;
		}
		
		i = ii; // el proximo estado pasa a ser el estado actual
		offset++; // avanza al proximo caracter de la cadena
	}
	
	
	cout << endl << "Cadena analizada: " << cadena << endl;
	cout << "Octales: " << cantidadNumeros[0] << " - Decimales: " << cantidadNumeros[1] << " - Hexadecimales: "
		<< cantidadNumeros[2] << endl << "Errores: " << cantErr << endl;
	
	return 0;
}




#include <iostream>
using namespace std;

int funcion2(char numero){
    int numeroCaracter = numero; // C++ trabaja con los caracteres usando codigo ASCII, por lo que numeroCaracter es el valor en la tabla ASCII del caracter numerico ingresado
    if (!(numero>=48 && numero<=57)){
        cout << "\n <ERROR> El caracter ingresado no es numerico. \n" << endl;
        return -1; // Nos aseguramos de que el caracter ingresado sea realmente un caracter numerico, devolviendo error en caso contrario
    }

    numeroCaracter -= 48; // Sabiendo que el caracter '0' es el numero 48 en ASCII, simplemente calculamos la diferencia y retornamos dicho valor
    return numeroCaracter;
}

// int main(){
//     char caracter;
//     cout << "Ingrese un numero: ";
//     cin >> caracter;

//     int numeroCaracter = funcion2(caracter);

//     printf("%d \n", numeroCaracter);
// }

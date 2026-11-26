#include <stdio.h>
#include "Ejercicio2.h"

int funcion2(char numero){
    // verificamos que el caracter sea un dígito entre '0' y '9'
    if (numero < '0' || numero > '9'){
        printf("\n <ERROR> El caracter ingresado no es numerico. \n\n");
        return -1; // retornamos -1 en caso de que el caracter ingresado no sea numerico
    }

    // C hace automáticamente la conversión entre caracteres y sus valores ASCII, por lo que no es necesario usar directamente los valores numericos de ASCII
    // Sabiendo que el caracter '0' es el numero 48 en ASCII, calculamos la diferencia y retornamos dicho valor.
    return numero - '0';
}
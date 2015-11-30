#include <inttypes.h>
#include <stdio.h>

// Tamaños de la cuadricula
// Se utilizan 16 columnas para facilitar la visualización
enum {
    NUM_FILAS = 9, NUM_COLUMNAS = 16, PADDING = 7
};

// Definiciones para valores muy utilizados
enum {
    FALSE = 0, TRUE = 1
};
typedef uint16_t CELDA;
// La información de cada celda está codificada en 16 bits
// con el siguiente formato (empezando en el bit más significativo):
// 4 MSB VALOR
// 1 bit PISTA
// 1 bit ERROR
// 1 bit no usado
// 9 LSB CANDIDATOS

inline void celda_poner_valor(CELDA *celdaptr, uint8_t val) {
    *celdaptr = (*celdaptr & 0x0FFF) | ((val & 0x000F) << 12);
}

inline uint8_t celda_leer_valor(CELDA celda) {
    return (celda >> 12);
}

/**
 * devuelve 1 si el valor [val] no esta entre los candidatos de la celda [celda]
 * o 0 en caso contrario
 */
inline uint8_t hay_error(CELDA *celda, uint8_t val) {

    uint16_t mascara_valor = (0x1 << (val - 1));
    uint16_t candidatos = (*celda & 0x1FF);

    return mascara_valor != (mascara_valor & candidatos);
}

inline void celda_poner_error(CELDA *celdaptr) {
    *celdaptr = (*celdaptr | 0x0400);
}

inline void celda_quitar_error(CELDA *celdaptr) {
    *celdaptr = (*celdaptr & ~(0x0400));
}

// funcion en ARM
extern int
sudoku_recalcular_arm(CELDA*, uint16_t);

// funcion en ARM
extern int
sudoku_candidatos_arm(CELDA*, uint8_t, uint8_t);

// funcion en Thumb
extern int
sudoku_candidatos_thumb(CELDA*, uint8_t, uint8_t);

int sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], int, char*);

////////////////////////////////////////////////////////////////////////////////
// Dada una determinada celda de la cuadricula encuentra los posibles valores
// candidatos. Devuelve 0 si celda vacia, -1 si celda llena con error,
// y 1 si celda llena sin error
int sudoku_candidatos_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila,
        uint8_t columna) {

    uint8_t i, j;

    // borrar bit de error
    celda_quitar_error(&cuadricula[fila][columna]);

    // iniciar candidatos
    cuadricula[fila][columna] |= 0x01FF;

    //precalcular el numero de columnas, evitando el padding
    uint8_t limiteColumna = NUM_COLUMNAS - PADDING;

    // recorrer fila recalculando candidatos
    for (i = 0; i < limiteColumna; i++) {

        //evita comprobar la celda que estamos tratando
        if (i != columna) {

            //eliminar valores de candidatos
            //TODO: comprobar si hay error
            uint8_t valor = celda_leer_valor(cuadricula[fila][i]);
            if (valor != 0) {

                //pone a cero el bit referente al candidato encontrado
                cuadricula[fila][columna] &= ~(0x0001 << (valor - 1));
            }
        }

    }

    // recorrer columna recalculando candidatos
    for (i = 0; i < NUM_FILAS; i++) {

        //evita comprobar la celda que estamos tratando
        if (i != fila) {

            //eliminar valores de candidatos
            //TODO: comprobar si hay error
            uint8_t valor = celda_leer_valor(cuadricula[i][columna]);
            if (valor != 0) {

                //pone a cero el bit referente al candidato encontrado
                cuadricula[fila][columna] &= ~(0x0001 << (valor - 1));
            }
        }

    }

    //TODO: Optimizar la obtencion del punto inicial de la region (tabla)

    // recorrer region recalculando candidatos
    // obtener region
    uint8_t regionFila = fila / 3;
    uint8_t regionColumna = columna / 3;

    // calcular punto inicial de la region
    regionFila = regionFila * 3;
    regionColumna = regionColumna * 3;

    // recorrer region
    for (i = regionFila; i < regionFila + 3; i++) {
        for (j = regionColumna; j < regionColumna + 3; j++) {

            //evita comprobar la celda que estamos tratando
            if (i != fila || j != columna) {

                //eliminar valores de candidatos
                //TODO: comprobar si hay error
                uint8_t valor = celda_leer_valor(cuadricula[i][j]);

                if (valor != 0) {

                    //pone a cero el bit referente al candidato encontrado
                    cuadricula[fila][columna] &= ~(0x0001 << (valor - 1));
                }
            }
        }
    }

    // retornar indicando si la celda tiene un valor (valido o erroneo) o esta vacia
    uint8_t valor_celda = celda_leer_valor(cuadricula[fila][columna]);

    if (valor_celda != 0) {

        if (hay_error(&cuadricula[fila][columna], valor_celda) == 1) {
            celda_poner_error(&cuadricula[fila][columna]);
            return -1;
        } else {
            return 1;
        }

    } else {
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Dada una cuadricula determina los candidatos de cada una de las celdas del
// tablero (9x9) y devuelve el numero de celdas vacias (sin valor asignado).
// El parametro [opcion] determina la funcion a utilizar
//      1 -> sudoku_candidatos_c
//      2 -> sudoku_candidatos_arm
//      3 -> sudoku_candidatos_thumb
int sudoku_recalcular_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], int opcion) {

    uint8_t error = 0;
    uint8_t contador = 0;
    uint8_t i, j;
    uint8_t limiteColumna = NUM_COLUMNAS - PADDING;
    uint16_t resultado;

    //para cada fila
    for (i = 0; i < NUM_FILAS; i++) {

        //para cada columna
        for (j = 0; j < limiteColumna; j++) {

            //determinar candidatos

            if (opcion == 1) {
                resultado = sudoku_candidatos_c(cuadricula, i, j);

            } else if (opcion == 2) {
                resultado = sudoku_candidatos_arm((CELDA*) cuadricula, i, j);

            } else if (opcion == 3) {
                resultado = sudoku_candidatos_thumb((CELDA*) cuadricula, i, j);
            }

            if (resultado == 0) {
                //actualizar contador de celdas vacias
                contador++;
            } else if (resultado < 0) {
                error = 1;
            }
        }
    }

    //devolver el numero de celdas vacias (valor absoluto), y si hay error (valor negativo)
    if (error == 1) {
        return -contador;
    } else {
        return contador;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Comprueba que los seis tableros (resultantes de las diferentes combinaciones)
// son iguales, y que la cantidad de celdas vacias en todos los casos es la
// misma. Almacena los resultados de la comprobacion en memoria
void comprobar_resultados(CELDA cuadricula1[NUM_FILAS][NUM_COLUMNAS],
        CELDA cuadricula2[NUM_FILAS][NUM_COLUMNAS],
        CELDA cuadricula3[NUM_FILAS][NUM_COLUMNAS],
        CELDA cuadricula4[NUM_FILAS][NUM_COLUMNAS],
        CELDA cuadricula5[NUM_FILAS][NUM_COLUMNAS],
        CELDA cuadricula6[NUM_FILAS][NUM_COLUMNAS], uint16_t resultados[8]) {

    uint8_t i, j;
    uint16_t diferencias_tablero = 0;
    uint8_t diferencias_resultados = 0;

    // comprueba tableros
    for (i = 0; i < NUM_FILAS; i++) {
        for (j = 0; j < NUM_COLUMNAS - PADDING; j++) {

            // comprueba diferencias entre los 6 tableros
            if (cuadricula1[i][j] != cuadricula2[i][j]
                    || cuadricula1[i][j] != cuadricula3[i][j]
                    || cuadricula1[i][j] != cuadricula4[i][j]
                    || cuadricula1[i][j] != cuadricula5[i][j]
                    || cuadricula1[i][j] != cuadricula6[i][j]) {

                diferencias_tablero++;
            }
        }
    }

    // comprueba resultados
    for (i = 1; i < 6; i++) {
        if (resultados[0] != resultados[i]) {
            diferencias_resultados++;
        }
    }

    // almacena resultados de comprobacion en memoria
    resultados[6] = diferencias_tablero;
    resultados[7] = diferencias_resultados;

}

////////////////////////////////////////////////////////////////////////////////
// Proceso principal del juego que recibe el tablero, la opcion de
// funcionamiento (C-C, C-ARM..), y la señal de ready que indica que se han
// actualizado fila y columna.
// Devuelve el numero de celdas vacias (sin valor asignado).
// Opcion:  1 -> C-C        4 -> ARM-C
//          2 -> C-ARM      5 -> ARM-ARM
//          3 -> C-THUMB    6 -> ARM-THUMB
int sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], int opcion,
        char *ready) {
    int celdas_vacias;

    // C + (C , ARM, THUMB)
    if (opcion >= 1 && opcion <= 3) {

        celdas_vacias = sudoku_recalcular_c(cuadricula, opcion);

    }

    // ARM + (C , ARM, THUMB)
    else if (opcion >= 4 && opcion <= 6) {

        celdas_vacias = sudoku_recalcular_arm((CELDA*) cuadricula, opcion - 3);
    }

    return celdas_vacias;
}


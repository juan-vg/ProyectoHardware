#include <inttypes.h>

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

// funcion a implementar en ARM
extern int
sudoku_candidatos_arm(CELDA* , uint8_t, uint8_t);

// funcion a implementar en Thumb
extern int
sudoku_candidatos_thumb(CELDA* , uint8_t, uint8_t);

////////////////////////////////////////////////////////////////////////////////
// dada una determinada celda encuentra los posibles valores candidatos
// y guarda el mapa de bits en la celda
// retorna false si la celda esta vacia, true si contiene un valor
int sudoku_candidatos_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila,
		uint8_t columna) {

	uint8_t i, j;

	//si NO es uno de los numeros fijados inicialmente (pista = 0)
	if ((cuadricula[fila][columna] & 0x0800) == 0) {

		//si tiene un valor erroneo, o no tiene valor
		if ((cuadricula[fila][columna] & 0x0400) == 0
				|| (cuadricula[fila][columna] & 0xF000) != 0) {

			//TODO: Tratar error, borrando bit de error y el valor de la celda

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

			// recorrer region recalculando candidatos
			// obtener region
			uint8_t regionFila = fila / 3;
			uint8_t regionColumna = columna / 3;

			// calcular punto inicial de la region
			regionFila = regionFila * 3;
			regionColumna = regionColumna * 3;

			// recorrer region
			for (i = regionFila; i < regionColumna + 3; i++) {
				for (j = regionColumna; j < regionFila + 3; j++) {

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
		}
	}

// retornar indicando si la celda tiene un valor o esta vacia
	if ((cuadricula[fila][columna] & 0xF000) != 0)
		return TRUE;
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// recalcula todo el tablero (9x9)
// retorna el numero de celdas vacias
int sudoku_recalcular_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]) {

	uint8_t contador = 0;
	uint8_t i, j;
	uint8_t limiteColumna = NUM_COLUMNAS - PADDING;

//para cada fila
	for (i = 0; i < NUM_FILAS; i++) {

		//para cada columna
		for (j = 0; j < limiteColumna; j++) {

			//determinar candidatos
			if (sudoku_candidatos_c(cuadricula, i, j) == FALSE) {

				//actualizar contador de celdas vacias
				contador++;
			}
		}
	}

//retornar el numero de celdas vacias
	return contador;
}

////////////////////////////////////////////////////////////////////////////////
// proceso principal del juego que recibe el tablero,
// y la señal de ready que indica que se han actualizado fila y columna
void sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], char *ready) {
	int celdas_vacias;     //numero de celdas aun vacias

	//__asm__("mov r0,#0");

	celdas_vacias = sudoku_candidatos_arm(cuadricula,4,0);
	//celdas_vacias = sudoku_recalcular_c(cuadricula);

}


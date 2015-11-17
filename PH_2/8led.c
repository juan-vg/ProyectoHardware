/*********************************************************************************************
 * Fichero:	8led.c
 * Autor:
 * Descrip:	Funciones de control del display 8-segmentos
 * Version:
 *********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"

/*--- definicion de macros ---*/
/* Mapa de bits de cada segmento
 (valor que se debe escribir en el display para que se ilumine el segmento) */

enum {
	cero = ~0xED,
	uno = ~0x60,
	dos = ~0xCE,
	tres = ~0xEA,
	cuatro = ~0x63,
	cinco = ~0xAB,
	seis = ~0x2F,
	siete = ~0xE0,
	ocho = ~0xEF,
	nueve = ~0xE3,
	A = ~0xE7,
	B = ~0x2F,
	C = ~0x8D,
	D = ~0x6E,
	E = ~0x8F,
	F = ~0x87,
	blank = ~0x00,
	size_8led = 17
};

/*--- variables globales ---*/
/* tabla de segmentos */
int Symbol[size_8led] = { cero, uno, dos, tres, cuatro, cinco, seis, siete,
		ocho, nueve, A, B, C, D, E, F, blank };
int8_t valor_actual;

/*--- declaración de funciones ---*/
void D8Led_init(void);
void D8Led_symbol(int value);
void D8Led_sequence(int8_t);
void D8Led_sequence_1_9(int8_t);


/*--- código de las funciones ---*/
void D8Led_init(void) {
	/* Estado inicial del display con todos los segmentos iluminados
	 (buscar en los ficheros de cabera la direccion implicada) */
	LED8ADDR = cero;
	valor_actual = 0;
}

void D8Led_symbol(int value) {
	/* muestra el Symbol[value] en el display (analogo al caso anterior) */
	if ((value >= 0) && (value < size_8led)) {
		LED8ADDR = Symbol[value];
		valor_actual = value;
	}
}

/**
 * Avanza una unidad el numero mostrado en el 8led (de 0 a F)
 * Se avanza hacia la derecha si [sentido] > 0
 * Se avanza hacia la izquierda si [sentido] < 0
 */
void D8Led_sequence(int8_t sentido) {
	if (sentido > 0) {
		valor_actual++;
	} else if (sentido < 0) {
		valor_actual--;
	}

	if (valor_actual < 0) {
		valor_actual = 0xf;
	} else if (valor_actual > 0xf) {
		valor_actual = 0;
	}

	LED8ADDR = Symbol[valor_actual];
}

/**
 * Avanza una unidad el numero mostrado en el 8led (de 1 a 9)
 * Se avanza hacia la derecha si [sentido] > 0
 * Se avanza hacia la izquierda si [sentido] < 0
 */
void D8Led_sequence_1_9(int8_t sentido) {
	if (sentido > 0) {
		valor_actual++;
	} else if (sentido < 0) {
		valor_actual--;
	}

	if (valor_actual < 1) {
		valor_actual = 9;
	} else if (valor_actual > 9) {
		valor_actual = 1;
	}

	LED8ADDR = Symbol[valor_actual];
}

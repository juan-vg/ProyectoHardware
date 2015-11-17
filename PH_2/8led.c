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

uint8_t valor_actual;
uint8_t minimo = 0;
uint8_t maximo = 9;

/*--- declaración de funciones ---*/
void D8Led_init(void);
void D8Led_symbol(int value);
void D8Led_sequence(int8_t);
void D8Led_siguiente();
void D8Led_define_rango(uint8_t min, uint8_t max);

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
 * Decrementa / Incrementa una unidad el numero mostrado en el 8led (de 0 a F)
 * Se avanza hacia la derecha si [sentido] > 0
 * Se avanza hacia la izquierda si [sentido] < 0
 */
void D8Led_sequence(int8_t sentido) {

    if (valor_actual == 0 && sentido < 0) {
        valor_actual = 0xf;
    } else if (valor_actual == 0xf && sentido > 0) {
        valor_actual = 0;
    } else {
        valor_actual += sentido;
    }

    LED8ADDR = Symbol[valor_actual];
}

/**
 * Define el rango de valores numericos que puede utilizar el 8led
 */
void D8Led_define_rango(uint8_t min, uint8_t max) {
    if (min >= 0) {
        minimo = min;
    }

    if (max <= 9) {
        maximo = max;
    }

}

/**
 * Avanza una unidad el numero mostrado en el 8led,
 * teniendo en cuenta el rango definido
 */
void D8Led_siguiente() {
    valor_actual++;

    if (valor_actual > maximo) {
        valor_actual = minimo;
    }

    LED8ADDR = Symbol[valor_actual];
}

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
    cero      = ~0xED,
    uno	      = ~0x60,
    dos       = ~0xCE,
    tres      = ~0xEA,
    cuatro    = ~0x63,
    cinco     = ~0xAB,
    seis      = ~0x2F,
    siete     = ~0xE0,
    ocho      = ~0xEF,
    nueve     = ~0xE3,
    A         = ~0xE7,
    B         = ~0x2F,
    C         = ~0x8D,
    D         = ~0x6E,
    E         = ~0x8F,
    F         = ~0x87,
    blank     = ~0x00,
    size_8led = 17 };

/*--- variables globales ---*/
/* tabla de segmentos */
int Symbol[size_8led] = { cero, uno, dos, tres, cuatro, cinco, seis, siete, ocho, nueve, A, B, C, D, E, F, blank};

/*--- declaración de funciones ---*/
void D8Led_init(void);
void D8Led_symbol(int value);

/*--- código de las funciones ---*/
void D8Led_init(void)
{
	/* Estado inicial del display con todos los segmentos iluminados
	   (buscar en los ficheros de cabera la direccion implicada) */
	LED8ADDR = cero;
}

void D8Led_symbol(int value)
{
	/* muestra el Symbol[value] en el display (analogo al caso anterior) */
	if ((value >= 0) && (value < size_8led)) {
		LED8ADDR = Symbol[value];
	}
}

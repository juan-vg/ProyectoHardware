/*********************************************************************************************
 * Fichero:	main.c
 * Autor:
 * Descrip:	punto de entrada de C
 * Version:  <P4-ARM.timer-leds>
 *********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "stdio.h"
#include <inttypes.h>

/*--- variables globales ---*/
extern uint8_t switch_timer;
extern int8_t valor_actual;

/*--- funciones externas ---*/
extern void leds_off();
extern void led1_on();
extern void led2_on();
extern void leds_switch();
extern void timer_init();
extern void Eint4567_init();
extern void D8Led_init();
extern void Timer2_Inicializar();
extern void Timer2_Empezar();
extern int Timer2_Leer();
extern void DelayMs(int);

void DoUndef(void);
void DoDabort(void);

/*--- declaracion de funciones ---*/
void Main(void);

/*--- codigo de funciones ---*/
void Main(void) {

	/* Inicializa controladores */
	sys_init();        // Inicializacion de la placa, interrupciones y puertos
	timer_init();	   // Inicializacion del temporizador
	Eint4567_init();// inicializamos los pulsadores. Cada vez que se pulse se verá reflejado en el 8led
	D8Led_init(); // inicializamos el 8led

	int t1, t2, t3;

	t1 = t2 = t3 = 1;

	Timer2_Inicializar();

	// mide 1 ms
	Timer2_Empezar();
	DelayMs(1);
	t1 = Timer2_Leer();

	// mide 10 ms
	//Timer2_Empezar();
	DelayMs(10);
	t2 = Timer2_Leer();

	// mide 1 s
	//Timer2_Empezar();
	DelayMs(1000);
	t3 = Timer2_Leer();

	//while(1);

	/* Valor inicial de los leds */
	leds_off();
	led1_on();
	led2_on();
	leds_off();

	DoDabort();

	// TODO: Esperar pulsacion de boton para empezar

	// SUDOKU

	int8_t fila, columna;

	// Mostrar la letra F (de FILA) en el 8led
	D8Led_symbol(0xF);

	// TODO: Esperar boton izquierdo

	// Mostrar un 1
	D8Led_symbol(0x1);

	// TODO: permitir modificar el num en el 8led (boton izq)

	// TODO: esperar boton derecho (confirma y sigue)
	fila = valor_actual;

	// Mostrar la letra C (de COLUMNA) en el 8led
	D8Led_symbol(0xC);

	// TODO: Esperar boton izquierdo

	// Mostrar un 1
	D8Led_symbol(0x1);

	// TODO: permitir modificar el num en el 8led (boton izq)

	// TODO: esperar boton derecho (confirma y sigue)
	columna = valor_actual;

	// TODO: mostrar valor de celda en 8led
	CELDA celda = cuadricula([fila][columna]);
	uint4_t valor = celda >> 12;
	D8Led_symbol(valor);

	// TODO: esperar a que introduzca nuevo valor


}

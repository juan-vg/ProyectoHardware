/*********************************************************************************************
 * Fichero:	main.c
 * Autor: Marta Frias y Juan Vela
 * Descrip:	punto de entrada de C
 * Version:  <P4-ARM.timer-leds>
 *********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "stdio.h"
#include <inttypes.h>

typedef uint16_t CELDA;
// La información de cada celda está codificada en 16 bits
// con el siguiente formato (empezando en el bit más significativo):
// 4 MSB VALOR
// 1 bit PISTA
// 1 bit ERROR
// 1 bit no usado
// 9 LSB CANDIDATOS

// Tamaños de la cuadricula
// Se utilizan 16 columnas para facilitar la visualización
enum {
	NUM_FILAS = 9, NUM_COLUMNAS = 16, PADDING = 7
};

/*--- variables globales ---*/
extern uint8_t switch_timer;
extern int8_t valor_actual;
extern uint8_t pulsado;

/*--- funciones externas ---*/
extern void leds_off();
extern void led1_on();
extern void led2_on();
extern void leds_switch();
extern void timer_init();
extern void Eint4567_init();

extern void Timer2_Inicializar();
extern void Timer2_Empezar();
extern unsigned int Timer2_Leer();
extern void DelayMs(int);
extern void Delay(int);

extern void D8Led_init();
extern void D8Led_symbol();
extern void D8Led_define_rango(uint8_t min, uint8_t max);
extern void D8Led_activar_avance(void);
extern void D8Led_desactivar_avance(void);

extern void celda_poner_valor(CELDA*, uint8_t);
extern int sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], int, char*);

void DoUndef(void);
void DoDabort(void);

/*--- declaracion de funciones ---*/
void Main(void);


/* variables */
	
// cuadricula SUDOKU. Definida en espacio de memoria de la aplicacion. Alineada
static CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS] __attribute__((aligned(32))) = { {
		0x9800, 0x6800, 0x0000, 0x0000, 0x0000, 0x0000, 0x7800, 0x0000,
		0x8800, 0, 0, 0, 0, 0, 0, 0 }, { 0x8800, 0x0000, 0x0000, 0x0000,
		0x0000, 0x4800, 0x3800, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, {
		0x1800, 0x0000, 0x0000, 0x5800, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0, 0, 0, 0, 0, 0, 0 }, { 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x1800, 0x7800, 0x6800, 0, 0, 0, 0, 0, 0, 0 }, {
		0x2800, 0x0000, 0x0000, 0x0000, 0x9800, 0x3800, 0x0000, 0x0000,
		0x5800, 0, 0, 0, 0, 0, 0, 0 }, { 0x7800, 0x0000, 0x8800, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, {
		0x0000, 0x0000, 0x7800, 0x0000, 0x3800, 0x2800, 0x0000, 0x4800,
		0x0000, 0, 0, 0, 0, 0, 0, 0 }, { 0x3800, 0x8800, 0x2800, 0x1800,
		0x0000, 0x5800, 0x6800, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, {
		0x0000, 0x4800, 0x1800, 0x0000, 0x0000, 0x9800, 0x5800, 0x2800,
		0x0000, 0, 0, 0, 0, 0, 0, 0 } };


/*--- codigo de funciones ---*/

/**
 * Devuelve el valor del bit 11 de la celda (inidica si es pista). Si el valor
 * es igual a 1 la celda tiene un valor fijado inicialmente
 */
inline uint8_t celda_comprueba_pista(CELDA *celdaptr) {
	return ((*celdaptr & 0x800) >> 11);
}

/**
 * Realiza una espera activa hasta que se pulsa y se suelta el boton [boton]
 *
 * boton = 0 -> cualquier boton
 * boton = 1 -> boton izquierdo
 * boton = 2 -> boton derecho
 */
void esperarPulsacion(uint8_t boton) {

	// espera mientras :
	// - no se pulsa nada
	// - no se pulsa el boton esperado
	while (pulsado == 0 || (boton > 0 && pulsado != boton))
		;

	// espera mientras no se suelta el boton esperado
	while (pulsado != 0)
		;
}

void Main() {

	/* Inicializa controladores */
	sys_init();         // inicializar la placa, interrupciones y puertos
	timer_init();	    // inicializar el temporizador
	Eint4567_init(); 	// inicializar los pulsadores
	D8Led_init();       // inicializar el 8led

	// configurar e iniciar el Timer2 
	Timer2_Inicializar();
	Timer2_Empezar();

	// provocar excepciones
	//DoUndef();

	/* variables para medicion de tiempos */
	int t1, t2;
	t1 = t2 = 0;

	// desactivar avance de 8Led (para boton izquierdo)
	D8Led_desactivar_avance();

	// esperar pulsacion de un boton para empezar
	esperarPulsacion(0);

	/*********************  SUDOKU  *********************/

	uint8_t fila, columna;
	uint8_t fin = 0;

	while (fin != 1) {

		// calcular candidatos del tablero
		// (opcion 1 -> C + C)
		t1 = Timer2_Leer();
		sudoku9x9(cuadricula, 5, 0);
		//Delay(100);
		t2 = Timer2_Leer() - t1;

		// para filas el rango de valores posibles es [1,9]
		// pero se permite el 0 para finalizar la partida
		D8Led_define_rango(0, 9);

		// mostrar la letra F (de FILA) en el 8led
		D8Led_symbol(0xF);

		// esperar boton izquierdo
		esperarPulsacion(1);

		// mostrar posicion inicial (un uno)
		D8Led_symbol(0x1);

		// Activar avance de 8Led (para boton izquierdo)
		D8Led_activar_avance();

		// esperar boton derecho (confirma)
		esperarPulsacion(2);

		// Desactivar avance de 8Led (para boton izquierdo)
		D8Led_desactivar_avance();

		// obtener numero de fila a partir del valor del 8led
		fila = valor_actual;

		// si se introduce un 0 -> terminar
		if (fila == 0) {
			fin = 1;
		}

		// si no -> continuar
		else {
			// para columnas el rango de valores posibles es [1,9]
			D8Led_define_rango(1, 9);

			// mostrar la letra C (de COLUMNA) en el 8led
			D8Led_symbol(0xC);

			// esperar boton izquierdo
			esperarPulsacion(1);

			// mostrar posicion inicial (un uno)
			D8Led_symbol(0x1);

			// activar avance de 8Led (para boton izquierdo)
			D8Led_activar_avance();

			// esperar boton derecho (confirma)
			esperarPulsacion(2);

			// desactivar avance de 8Led (para boton izquierdo)
			D8Led_desactivar_avance();

			// obtener numero de columna
			columna = valor_actual;

			// para contenido de celdas el rango de valores posibles es [0,9]
			D8Led_define_rango(0, 9);

			// mostrar valor inicial (un uno)
			D8Led_symbol(0x1);

			// activar avance de 8Led (para boton izquierdo)
			D8Led_activar_avance();

			// esperar boton derecho (confirma)
			esperarPulsacion(2);

			// desactivar avance de 8Led (para boton izquierdo)
			D8Led_desactivar_avance();

			// almacenar el valor introducido para la celda (a partir del 8led)
			// si y solo si NO es una pista inicial
			if (celda_comprueba_pista(&cuadricula[fila - 1][columna - 1]) == 0) {
				celda_poner_valor(&cuadricula[fila - 1][columna - 1],
						valor_actual);
			}
		}
	}
}

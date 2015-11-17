/*********************************************************************************************
 * Fichero:		timer.c
 * Autor:
 * Descrip:		funciones de control del timer0 del s3c44b0x
 * Version:
 *********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"
#include <inttypes.h>



/*--- variables globales ---*/
uint8_t switch_timer = 0;
unsigned int timer2_num_int = 0;

uint8_t tiempo_botones = 0;

/*--- elementos externos ---*/
extern uint8_t estado_botones;
extern unsigned int int_count;
extern uint8_t sentido;
extern uint8_t pulsado;

extern void D8Led_symbol(int);
extern void push_debug(int, int);

/*--- declaracion de funciones ---*/
void timer_ISR(void) __attribute__((interrupt("IRQ")));
void timer2_ISR(void) __attribute__((interrupt("IRQ")));
void timer_init(void);
void Timer2_Inicializar(void);
void Timer2_Empezar(void);
int Timer2_Leer(void);
void programar_alarma(unsigned int);

uint16_t tiempo_interr;
unsigned int alarma = 0;

/*--- codigo de las funciones ---*/
void timer_ISR(void) {
	switch_timer = 1;

	/* borrar bit en I_ISPC para desactivar la solicitud de interrupción*/
	rI_ISPC |= BIT_TIMER0; // BIT_TIMER0 está definido en 44b.h y pone un uno en el bit 13 que correponde al Timer0
}

void timer2_ISR(void) {

	timer2_num_int++;

	// momento actual = momento alarma
	if (alarma == timer2_num_int) {

		uint32_t auxData = 0;

		auxData |= estado_botones << 16;
		auxData |= pulsado << 8;
		auxData |= tiempo_botones;

		// ID = CACA1
		push_debug(830625, auxData);

		if(estado_botones == 1){
			programar_alarma(10);
			tiempo_botones++;
			estado_botones = 2;
		}

		// despues de 300 ms (100 + 200) como maximo
		else if(estado_botones == 2 && pulsado == 1 && tiempo_botones < 40){

			//leer bits 6 y 7 (pulsadores)
			//cuando NO estan pulsados valen 1
			if((rPDATG & 0xC0) == 0xC0){
				pulsado = 0;
			}

			programar_alarma(10);
			tiempo_botones++;
		}

		// despues de mas de 200 ms (mas los 100ms iniciales) -> siguiente
		/*else if(estado_botones == 2 && pulsado == 1 && tiempo_botones >= 40){

			// para comparar con 300ms solo cuando se mantenga pulsado
			tiempo_botones = 10;

			// avanza o retrocede una posicion
			//int_count = int_count + sentido;
			//D8Led_symbol(int_count&0x000f);
			D8Led_sequence(sentido);
		}*/

		else if(estado_botones == 2 && pulsado == 0){
			// trd = 100ms
			programar_alarma(100);
			estado_botones = 3;
		}

		else if(estado_botones == 3){
			// vuelve a activar las IRQs de botones
			rINTMSK    &= ~(BIT_EINT4567);
			estado_botones = 0;
			tiempo_botones = 0;
		}
	}



	/* borrar bit en I_ISPC para desactivar la solicitud de interrupción*/
	rI_ISPC |= BIT_TIMER2; // BIT_TIMER2 está definido en 44b.h y pone un uno en el bit 11 que correponde al Timer2
}

void timer_init(void) {
	/* Configuraion controlador de interrupciones */
	rINTMOD = 0x0; // Configura las linas como de tipo IRQ
	rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK &= ~(BIT_TIMER0); // Desenmascara Timer0 (BIT_TIMER0 están definido en 44b.h)

	// Establece la rutina de servicio para TIMER0
	pISR_TIMER0 = (unsigned) timer_ISR;

	// Configura el Timer0
	rTCFG0 = 255; // ajusta el preescalado
	rTCFG1 = 0x0; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCNTB0 = 65535; // valor inicial de cuenta (la cuenta es descendente)
	rTCMPB0 = 12800; // valor de comparación

	// establecer update=manual (bit 1) + inverter=on (¿? será inverter off un cero en el bit 2 pone el inverter en off)
	rTCON =	0x2;

	/* iniciar timer (bit 0) con auto-reload (bit 3)*/
	rTCON = 0x09;
}

/**
 * configura el timer 2 para que trabaje a la
 * máxima precisión posible.
 */
void Timer2_Inicializar(void) {
	/* Configuraion controlador de interrupciones */
	rINTMOD = 0x0; // Configura las linas como de tipo IRQ
	rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK &= ~(BIT_TIMER2); // Desenmascara Timer2 (BIT_TIMER2 están definido en 44b.h)

	// Establece la rutina de servicio para TIMER2
	pISR_TIMER2 = (unsigned) timer2_ISR;

	// Configura el Timer2
	rTCFG0 &= ~0xFF00; // ajusta el preescalado
	rTCFG1 &= ~0x0F00; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.

	// CNT = 32000 -> provoca interrupciones cada 1ms
	rTCNTB2 = 32000; // valor inicial de cuenta (la cuenta es descendente)
	rTCMPB2 = 0; // valor de comparación

	/* precalcula el tiempo por cada ciclo */
	tiempo_interr = (rTCNTB2 / 32);

}

/**
 * reinicia la cuenta de tiempo (contador y la variable),
 * y comienza a medir.
 */
void Timer2_Empezar(void) {
	/* desactivar timer2, limpiando bits 12..15)*/
	rTCON &= ~0xF000;

	timer2_num_int = 0;

	// establecer update manual (bit 13) [+ inverter=off]
	rTCON |= 0x2000;

	/* iniciar timer (bit 12) con auto-reload (bit 15), y desactivar update manual (bit 13)*/
	rTCON ^= 0xB000;
}

/**
 * Lee la cuenta actual del temporizador y el número de
 * interrupciones generadas, y devuelve el tiempo transcurrido en
 * microsegundos.
 *
 * Cada tick = 0.03125 us (64Mhz / 2)
 * Cada interrupcion = (32000 * 0.03125) us = (32000 / 32) us
 *
 */
int Timer2_Leer(void) {
	int tiempo_ticks = ((rTCNTB2 - rTCNTO2 ) / 32);
	return timer2_num_int * tiempo_interr + tiempo_ticks;
}

/**
 * Establece una alarma asincrona con precision de 1ms
 */
void programar_alarma(unsigned int ms) {
	//alarma = timer2_num_int + ms;
	alarma = timer2_num_int + ms*3;
}


/*********************************************************************************************
 * Fichero:		timer.c
 * Autor:
 * Descrip:		funciones de control del timer0 del s3c44b0x
 * Version:
 *********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"
#include <ctypes.h>

/*--- variables globales ---*/
uint8_t switch_leds = 0;
uint32_t timer2_num_int = 0;

/*--- declaracion de funciones ---*/
void timer_ISR(void) __attribute__((interrupt("IRQ")));
void timer2_ISR(void) __attribute__((interrupt("IRQ")));
void timer_init(void);
void Timer2_Inicializar(void);
void Timer2_Empezar(void);
int Timer2_Leer(void);
void programar_alarma(int);

float tiempo_interr;
int alarma = -1;

/*--- codigo de las funciones ---*/
void timer_ISR(void) {
	switch_leds = 1;

	/* borrar bit en I_ISPC para desactivar la solicitud de interrupción*/
	rI_ISPC |= BIT_TIMER0; // BIT_TIMER0 está definido en 44b.h y pone un uno en el bit 13 que correponde al Timer0
}

void timer2_ISR(void) {
	timer2_num_int++;

	// cuando pase el tiempo la alarma se desactiva, informando al mismo tiempo
	// que ya ha pasdo el tiempo
	if (alarma == timer2_num_int) {
		alarma = -1;
	}

	/* borrar bit en I_ISPC para desactivar la solicitud de interrupción*/
	rI_ISPC |= BIT_TIMER2; // BIT_TIMER2 está definido en 44b.h y pone un uno en el bit 11 que correponde al Timer2
}

void timer_init(void) {
	/* Configuraion controlador de interrupciones */
	rINTMOD = 0x0; // Configura las linas como de tipo IRQ
	rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK &= ~(BIT_TIMER0); // Desenmascara Timer0 (BIT_TIMER0 están definido en 44b.h)

	/* Establece la rutina de servicio para TIMER0 */pISR_TIMER0 =
			(unsigned) timer_ISR;

	/* Configura el Timer0 */rTCFG0 = 255; // ajusta el preescalado
	rTCFG1 = 0x0; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCNTB0 = 65535; // valor inicial de cuenta (la cuenta es descendente)
	rTCMPB0 = 12800; // valor de comparación

	/* establecer update=manual (bit 1) + inverter=on (¿? será inverter off un cero en el bit 2 pone el inverter en off)*/rTCON =
			0x2;
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

	/* Establece la rutina de servicio para TIMER2 */pISR_TIMER2 =
			(unsigned) timer2_ISR;

	/* Configura el Timer2 */rTCFG0 &= ~0xFF00; // ajusta el preescalado
	rTCFG1 &= ~0x0F00; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCNTB2 = 65535; // valor inicial de cuenta (la cuenta es descendente)
	rTCMPB2 = 0; // valor de comparación

	/* precalcula el tiempo por cada ciclo */
	tiempo_interr = (rTCNTB2 * 0.03125);

}

/**
 * reinicia la cuenta de tiempo (contador y la variable),
 * y comienza a medir.
 */
void Timer2_Empezar(void) {
	/* desactivar timer2, limpiando bits 12..15)*/
	rTCON &= ~0xF000;

	timer2_num_int = 0;
	//rTCNTB2 = 65535;// valor inicial de cuenta (la cuenta es descendente)

	/* establecer update manual (bit 13) [+ inverter=off] */rTCON |= 0x2000;

	/* iniciar timer (bit 12) con auto-reload (bit 15), y desactivar update manual (bit 13)*/
	rTCON ^= 0xB000;
}

/**
 * Lee la cuenta actual del temporizador y el número de
 * interrupciones generadas, y devuelve el tiempo transcurrido en
 * microsegundos.
 *
 * Cada tick = 0.03125 us (64Mhz / 2)
 * Cada interrupcion = (65535 * 0.03125) us
 *
 */
int Timer2_Leer(void) {
	float tiempo_ticks = ((rTCNTB2 - rTCNTO2 )* 0.03125);
	return timer2_num_int * tiempo_interr + tiempo_ticks;
}

// timer2_num_int == 1 => 2.047 ms
void programar_alarma(int ms) {
	alarma = timer2_num_int + ms / 2;
}


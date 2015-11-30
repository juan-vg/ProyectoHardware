/*********************************************************************************************
 * Fichero:	timer.c
 * Autor: Marta Frias y Juan Vela
 * Descrip:	funciones de control del timer0 y el timer2 del s3c44b0x
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
extern uint8_t trd_esperado;

extern void D8Led_symbol(int);
extern void D8Led_siguiente();
extern void push_debug(int, int);

/*--- declaracion de funciones ---*/
void timer_ISR(void) __attribute__((interrupt("IRQ")));
void timer2_ISR(void) __attribute__((interrupt("IRQ")));
void timer_init(void);
void Timer2_Inicializar(void);
void Timer2_Empezar(void);
unsigned int Timer2_Leer(void);
void programar_alarma(unsigned int);

uint16_t num_ticks_interr;
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
    if (alarma > 0 && alarma <= timer2_num_int) {

		// almacenar interrupcion en pila debug
        uint32_t auxData = 0;
        auxData |= estado_botones << 16;
        auxData |= pulsado << 8;
        auxData |= tiempo_botones;
        push_debug(0xDEB1, auxData); // ID = DEB1

        // comprobar estado del boton hasta 500 ms como maximo
		// (500 = 100 (trp) + 400 (= 40 * 10))
        if (estado_botones == 1 && pulsado > 0 && tiempo_botones <= 40) {

            //leer bits 6 y 7 (pulsadores)
            //cuando NO estan pulsados hay un 1 en rPDATG[7:6]
            if ((rPDATG & 0xC0) == 0xC0) {
                pulsado = 0;
            }

			// comprobar cada 10 ms
            programar_alarma(10);
            tiempo_botones++;
        }

        // despues de mas de 400 ms (mas los 100ms iniciales) -> avanzar 8Led
        else if (estado_botones == 1 && pulsado > 0 && tiempo_botones > 40) {

            // comparar con 300ms ((40-10) * 10) solo cuando se mantenga pulsado
            tiempo_botones = 10;
			
			// comprobar cada 10 ms
            programar_alarma(10);

            // avanza el 8Led una posicion
            D8Led_siguiente();
        }

		// cuando se detecta que se ha soltado el boton
        else if (estado_botones == 1 && pulsado == 0 && trd_esperado == 0) {
			
            // trd = 100ms
            programar_alarma(100);
            trd_esperado = 1;
        }

		// 
        else if (estado_botones == 1 && trd_esperado == 1) {

        	// Por precaucion, volver a borrar los bits de INTPND y EXTINTPND
        	rEXTINTPND = 0xf;
        	rI_ISPC |= (BIT_EINT4567);

            // volver a activar las IRQs de botones
            rINTMSK &= ~(BIT_EINT4567);
			
			// reinicializar estado
            estado_botones = 0;
            tiempo_botones = 0;
			trd_esperado = 0;

            // desactivar alarma
            alarma = 0;
        }
    }

    /* borrar bit en I_ISPC para desactivar la solicitud de interrupción*/
    rI_ISPC |= BIT_TIMER2; // pone un uno en el bit 11 que correponde al Timer2
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
    rTCFG1 = 0x0; // entrada del mux correspondiente a un divisor de 1/2.
    rTCNTB0 = 65535; // valor inicial de cuenta (la cuenta es descendente)
    rTCMPB0 = 12800; // valor de comparación

    // establecer update=manual (bit 1) + inverter=off
    rTCON = 0x2;

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
    rINTMSK &= ~(BIT_TIMER2); // Desenmascara Timer2

    // Establece la rutina de servicio para TIMER2
    pISR_TIMER2 = (unsigned) timer2_ISR;

    // Configura el Timer2
    rTCFG0 &= ~0xFF00; // ajusta el preescalado
    rTCFG1 &= ~0x0F00; // entrada del mux correspondiente a un divisor de 1/2.

    // CNT = 32000 -> provoca interrupciones cada 1ms
    rTCNTB2 = 33000; // valor inicial de cuenta (la cuenta es descendente)
    rTCMPB2 = 0; // valor de comparación

    /* precalcula el tiempo por cada ciclo */
    num_ticks_interr = rTCNTB2;
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
 unsigned int Timer2_Leer(void) {
    uint16_t num_ticks_actuales = (rTCNTB2 - rTCNTO2);
    unsigned int result = (timer2_num_int * num_ticks_interr + num_ticks_actuales) / 33;
    return result;
}

/**
 * Establece una alarma asincrona con precision de 1ms
 */
void programar_alarma(unsigned int ms) {
    alarma = timer2_num_int + ms;
}


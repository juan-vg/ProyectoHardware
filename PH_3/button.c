/*********************************************************************************************
 * Fichero:	button.c
 * Autor: Marta Frias y Juan Vela
 * Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
 * Version:
 *********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "def.h"
#include "inttypes.h"

/*--- variables globales ---*/
uint8_t estado_botones = 0;
uint8_t pulsado = 0;
uint8_t trd_esperado = 0;

uint8_t btn_izq_pnd = 0;
uint8_t btn_dch_pnd = 0;

/*--- declaracion de funciones ---*/
void Eint4567_ISR(void) __attribute__((interrupt("IRQ")));
void Eint4567_init(void);

// declaramos las funciones que escriben en el 8led
extern void D8Led_sequence(int8_t);
extern void D8Led_siguiente();

extern void programar_alarma(int);
extern void push_debug(int, int);

/*--- codigo de funciones ---*/
void Eint4567_init(void) {
	
    /* Configuracion del controlador de interrupciones. Registros definidos en 44b.h */
    rI_ISPC = 0x3ffffff;	// Borra INTPND escribiendo 1s en I_ISPC
    rEXTINTPND = 0xf;    // Borra EXTINTPND escribiendo 1s en el propio registro
    rINTMOD = 0x0;		// Configura las linas como de tipo IRQ
    rINTCON = 0x1;	    // Habilita int. vectorizadas y la linea IRQ (FIQ no)
    rINTMSK &= ~(BIT_EINT4567); // Desenmascara la linea de interrupcion eint4567

    // Establecer la rutina de servicio para Eint4567
    pISR_EINT4567 = (int) Eint4567_ISR;

    // Configurar el puerto G
    rPCONG = 0xffff; // Establece la funcion de los pines (EINT0-7)
    rPUPG = 0x0; // Habilita el "pull up" del puerto
    rEXTINT = rEXTINT | 0x22222222; // Configura las lineas de int. como de flanco de bajada

    // Por precaucion, se vuelven a borrar los bits de INTPND y EXTINTPND
    rI_ISPC |= (BIT_EINT4567);
    rEXTINTPND = 0xf;
}

void Eint4567_ISR(void) {
	
	// almacenar interrupcion en pila debug
    uint32_t auxData = 0;
    auxData |= estado_botones << 8;
    auxData |= pulsado;
    push_debug(0xDEB0, auxData); // ID = DEB0

    /* Identificar la interrupcion (hay dos pulsadores)*/
    int which_int = rEXTINTPND;
    switch (which_int) {
    case 4:
        pulsado = 1;    // boton izquierdo (1) pulsado
        btn_izq_pnd = btn_izq_pnd + 1;
		// avanzar el 8Led una posicion
		D8Led_siguiente();
        break;
    case 8:
        pulsado = 2;    // boton derecho (2) pulsado
        btn_dch_pnd = btn_dch_pnd + 1;
        break;
    default:
        break;
    }

    // comprobar por si acaso
    // solo deberia haber interrupciones en el estado 0
    if (estado_botones == 0) {
		
        // desactivar IRQ boton
        rINTMSK |= BIT_EINT4567;
		
		// cambiar de estado
		estado_botones = 1;
		
        // trp = 100 ms
        programar_alarma(100);
    }

    // Finalizar ISR
    rEXTINTPND = 0xf;		    // borra los bits en EXTINTPND
    rI_ISPC |= BIT_EINT4567;    // borra el bit pendiente en INTPND
}


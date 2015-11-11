/*********************************************************************************************
 * Fichero:	button.c
 * Autor:
 * Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
 * Version:
 *********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "def.h"
#include <inttypes.h>

/*--- variables globales ---*/
/* int_count la utilizamos para sacar un número por el 8led.
  Cuando se pulsa un botón sumamos y con el otro restamos. ¡A veces hay rebotes! */
unsigned int int_count = 0;
uint8_t sentido = 0;
uint8_t estado_botones = 0;
uint8_t pulsado = 0;

/*--- declaracion de funciones ---*/
void Eint4567_ISR(void) __attribute__((interrupt("IRQ")));
void Eint4567_init(void);
extern void D8Led_symbol(int value); // declaramos la función que escribe en el 8led
extern void programar_alarma(int ms);

/*--- codigo de funciones ---*/
void Eint4567_init(void)
{
    /* Configuracion del controlador de interrupciones. Estos registros están definidos en 44b.h */
    rI_ISPC    = 0x3ffffff;	// Borra INTPND escribiendo 1s en I_ISPC
    rEXTINTPND = 0xf;       // Borra EXTINTPND escribiendo 1s en el propio registro
    rINTMOD    = 0x0;		// Configura las linas como de tipo IRQ
    rINTCON    = 0x1;	    // Habilita int. vectorizadas y la linea IRQ (FIQ no)
    rINTMSK    &= ~(BIT_EINT4567); // Enmascara todas las lineas excepto eint4567, el bit global y el timer0

    /* Establece la rutina de servicio para Eint4567 */
    pISR_EINT4567 = (int)Eint4567_ISR;

    /* Configuracion del puerto G */
    rPCONG  = 0xffff;        		// Establece la funcion de los pines (EINT0-7)
    rPUPG   = 0x0;                  // Habilita el "pull up" del puerto
    rEXTINT = rEXTINT | 0x22222222;   // Configura las lineas de int. como de flanco de bajada

    /* Por precaucion, se vuelven a borrar los bits de INTPND y EXTINTPND */
    rI_ISPC    |= (BIT_EINT4567);
    rEXTINTPND = 0xf;
}

void Eint4567_ISR(void)
{
    uint32_t auxData = 0;

    auxData |= estado_botones << 8;
    auxData |= pulsado;

    push_debug(0, auxData);

    /* Identificar la interrupcion (hay dos pulsadores)*/
    int which_int = rEXTINTPND;
    switch (which_int)
    {
    case 4:
        int_count++; // incrementamos el contador
        sentido = 1;
        break;
    case 8:
        int_count--; // decrementamos el contador
        sentido = -1;
        break;
    default:
        break;
    }


    if(estado_botones == 0){
        // desactivar IRQ boton
        rINTMSK |= BIT_EINT4567;

        pulsado = 1;

        // trp = 100 ms
        programar_alarma(100);
        estado_botones = 1;
    }

    D8Led_symbol(int_count&0x000f); // sacamos el valor por pantalla (módulo 16)

    /* Finalizar ISR */
    rEXTINTPND = 0xf;				// borra los bits en EXTINTPND
    rI_ISPC   |= BIT_EINT4567;		// borra el bit pendiente en INTPND

}


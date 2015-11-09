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

/*--- variables globales ---*/
extern uint8_t switch_leds;

/*--- funciones externas ---*/
extern void leds_off();
extern void led1_on();
extern void leds_switch();
extern void timer_init();
extern void Eint4567_init();
extern void D8Led_init();
extern void Timer2_Inicializar();
extern void Timer2_Empezar();
extern float Timer2_Leer();

/*--- declaracion de funciones ---*/
void Main(void);

/*--- codigo de funciones ---*/
void Main(void)
{

	/* Inicializa controladores */
	sys_init();        // Inicializacion de la placa, interrupciones y puertos
	timer_init();	   // Inicializacion del temporizador
	Eint4567_init();	// inicializamos los pulsadores. Cada vez que se pulse se verá reflejado en el 8led
	D8Led_init(); // inicializamos el 8led

	float t1,t2,t3;

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

	while (1)
	{
		/* Cambia los leds con cada interrupcion del temporizador */
		if (switch_leds == 1)
		{
			leds_switch();
			switch_leds = 0;
		}
	}
}

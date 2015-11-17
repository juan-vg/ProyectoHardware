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

typedef uint16_t CELDA;
// La información de cada celda está codificada en 16 bits
// con el siguiente formato (empezando en el bit más significativo):
// 4 MSB VALOR
// 1 bit PISTA
// 1 bit ERROR
// 1 bit no usado
// 9 LSB CANDIDATOS

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
extern unsigned int Timer2_Leer();
extern void DelayMs(int);
extern void D8Led_define_rango(uint8_t min, uint8_t max);

void DoUndef(void);
void DoDabort(void);

/*--- declaracion de funciones ---*/
void Main(void);

/*--- codigo de funciones ---*/

inline void celda_poner_valor(CELDA *celdaptr, uint8_t val) {
    *celdaptr = (*celdaptr & 0x0FFF) | ((val & 0x000F) << 12);
}

inline uint8_t celda_comprueba_pista(CELDA *celdaptr) {
    return ((*celdaptr & 0x800) >> 11);
}

/**
 * boton = 0 -> cualquier boton
 * boton = 1 -> boton izquierdo
 * boton = 2 -> boton derecho
 */
void esperarPulsacion(uint8_t boton) {

    uint8_t valor;

    // espera mientras :
    // - no se pulsa nada
    // - no se pulsa el boton esperado
    while (pulsado == 0 || (boton > 0 && pulsado == (3 ^ boton)))
        ;

    // espera mientras no se suelta el boton esperado
    while (pulsado != 0)
        ;
}

void Main(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]) {

    /* Inicializa controladores */
    sys_init();         // Inicializacion de la placa, interrupciones y puertos
    timer_init();	    // Inicializacion del temporizador
    Eint4567_init(); // inicializamos los pulsadores. Cada vez que se pulse se verá reflejado en el 8led
    D8Led_init();       // inicializamos el 8led

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

    //DoDabort();

    // Esperar pulsacion de boton para empezar
    esperarPulsacion(0);

    // TODO : calcular candidatos de todo el tablero

    // SUDOKU

    int8_t fila, columna;

    // para filas y columnas el rango de valores posibles es [1,9]
    D8Led_define_rango(1, 9);

    // mostrar la letra F (de FILA) en el 8led
    D8Led_symbol(0xF);

    // esperar boton izquierdo
    esperarPulsacion(1);

    // mostrar posicion inicial (un uno)
    D8Led_symbol(0x1);

    // esperar boton derecho (confirma)
    esperarPulsacion(2);

    // obtener numero de fila a partir del valor del 8led
    fila = valor_actual;

    // mostrar la letra C (de COLUMNA) en el 8led
    D8Led_symbol(0xC);

    // esperar boton izquierdo
    esperarPulsacion(1);

    // mostrar posicion inicial (un uno)
    D8Led_symbol(0x1);

    // esperar boton derecho (confirma)
    esperarPulsacion(2);

    // obtener numero de columna
    columna = valor_actual;

    // para contenido de celdas el rango de valores posibles es [0,9]
    D8Led_define_rango(0, 9);

    // mostrar valor inicial (un uno)
    D8Led_symbol(0x1);

    // esperar boton derecho (confirma)
    esperarPulsacion(2);

    // almacena el valor introducido para la celda (a partir del 8led)
    // si y solo si NO es una pista inicial
    if (celda_comprueba_pista(cuadricula[fila - 1][columna - 1]) == 0) {
        celda_poner_valor(cuadricula[fila - 1][columna - 1], valor_actual);
    }

    // sudoku (opcion 1 -> C + C)
    sudoku9x9(cuadricula, 1, 0);
}

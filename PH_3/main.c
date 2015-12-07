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
#include "inttypes.h"
#include "def.h"

// Tamaños de la cuadricula
// Se utilizan 16 columnas para facilitar la visualización
enum {
    NUM_FILAS = 9, NUM_COLUMNAS = 16, PADDING = 7
};

/*--- variables globales ---*/
extern uint8_t switch_timer;
extern int8_t valor_actual;
extern uint8_t pulsado;
extern uint8_t btn_izq_pnd;
extern uint8_t btn_dch_pnd;
extern uint8_t alarma_1s;

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
extern void D8Led_define_rango(uint8_t, uint8_t);
extern void D8Led_activar_avance(void);
extern void D8Led_desactivar_avance(void);

// LCD
extern void Lcd_Init(void);
extern void Lcd_pantalla_presentacion(INT8);
extern void Lcd_pantalla_inicial(void);
extern void Lcd_pantalla_final(INT8, INT8U, INT8U);
extern void Lcd_cuadricula_sudoku(void);
extern void Lcd_rellenar_celda(INT8U, INT8U, CELDA);
extern void Lcd_actualizar_tiempo_calculo(int);
extern void Lcd_actualizar_tiempo_total(int);
extern void Lcd_limpiar_celda(INT8U, INT8U);
extern void Lcd_marcar_celda(INT8U, INT8U);
extern void Lcd_desmarcar_celda(INT8U, INT8U);

extern void celda_poner_valor(CELDA*, uint8_t);
extern int sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], int, char*);

/*--- declaracion de funciones ---*/
void Main(void);
void DoUndef(void);
void DoDabort(void);

/* variables */

uint8_t estado_juego = 0;
uint8_t num_errores = 0;
uint8_t num_acciones = 0;
int8_t resultado_partida = 0;

// cuadricula SUDOKU. Definida en espacio de memoria de la aplicacion. Alineada
static CELDA cuadricula_ini[NUM_FILAS][NUM_COLUMNAS] __attribute__((aligned(32)))
= { { 0x9800, 0x6800, 0x0000, 0x0000, 0x0000, 0x0000, 0x7800, 0x0000, 0x8800, 0,
        0, 0, 0, 0, 0, 0 }, { 0x8800, 0x0000, 0x0000, 0x0000, 0x0000, 0x4800,
        0x3800, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, { 0x1800, 0x0000, 0x0000,
        0x5800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1800, 0x7800, 0x6800,
        0, 0, 0, 0, 0, 0, 0 }, { 0x2800, 0x0000, 0x0000, 0x0000, 0x9800, 0x3800,
        0x0000, 0x0000, 0x5800, 0, 0, 0, 0, 0, 0, 0 }, { 0x7800, 0x0000, 0x8800,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, {
        0x0000, 0x0000, 0x7800, 0x0000, 0x3800, 0x2800, 0x0000, 0x4800, 0x0000,
        0, 0, 0, 0, 0, 0, 0 }, { 0x3800, 0x8800, 0x2800, 0x1800, 0x0000, 0x5800,
        0x6800, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, { 0x0000, 0x4800, 0x1800,
        0x0000, 0x0000, 0x9800, 0x5800, 0x2800, 0x0000, 0, 0, 0, 0, 0, 0, 0 } };

// cuadricula SUDOKU. Definida en espacio de memoria de la aplicacion. Alineada
static CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS] __attribute__((aligned(32)))
= { { 0x9800, 0x6800, 0x0000, 0x0000, 0x0000, 0x0000, 0x7800, 0x0000, 0x8800, 0,
        0, 0, 0, 0, 0, 0 }, { 0x8800, 0x0000, 0x0000, 0x0000, 0x0000, 0x4800,
        0x3800, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, { 0x1800, 0x0000, 0x0000,
        0x5800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1800, 0x7800, 0x6800,
        0, 0, 0, 0, 0, 0, 0 }, { 0x2800, 0x0000, 0x0000, 0x0000, 0x9800, 0x3800,
        0x0000, 0x0000, 0x5800, 0, 0, 0, 0, 0, 0, 0 }, { 0x7800, 0x0000, 0x8800,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, {
        0x0000, 0x0000, 0x7800, 0x0000, 0x3800, 0x2800, 0x0000, 0x4800, 0x0000,
        0, 0, 0, 0, 0, 0, 0 }, { 0x3800, 0x8800, 0x2800, 0x1800, 0x0000, 0x5800,
        0x6800, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0 }, { 0x0000, 0x4800, 0x1800,
        0x0000, 0x0000, 0x9800, 0x5800, 0x2800, 0x0000, 0, 0, 0, 0, 0, 0, 0 } };

/*--- codigo de funciones ---*/

/**
 * Devuelve el valor del bit 11 de la celda (inidica si es pista). Si el valor
 * es igual a 1 la celda tiene un valor fijado inicialmente
 */
inline uint8_t celda_comprueba_pista(CELDA *celdaptr) {
    return ((*celdaptr & 0x800) >> 11);
}

/**
 * Devuelve 1 si se ha pulsado el boton [boton]
 *
 * boton = 0 -> cualquier boton
 * boton = 1 -> boton izquierdo
 * boton = 2 -> boton derecho
 */
uint8_t comprobarPulsacion(uint8_t boton) {

    uint8_t resp = 0;

    if (boton == 0) {

        if (btn_izq_pnd == 1 || btn_dch_pnd == 1) {
            resp = 1;
        }
    } else if (boton == 1) {

        if (btn_izq_pnd == 1) {
            resp = 1;
        }
    } else if (boton == 2) {

        if (btn_dch_pnd == 1) {
            resp = 1;
        }
    }

    btn_izq_pnd = 0;
    btn_dch_pnd = 0;

    return resp;
}

int preparar_movimiento() {

    unsigned int t1, t2;
    t1 = t2 = 0;

    // calcular candidatos del tablero
    // (opcion 1 -> C + C)
    t1 = Timer2_Leer();

    // TODO : investigar opcion 5 (no pone error)

    int result = sudoku9x9(cuadricula, 5, 0);

    t2 = Timer2_Leer() - t1;

    rellenar_cuadricula();

    num_acciones++;

    // si tablero NO terminado
    if (result != 0) {

        // si hay error
        if (result < 0) {
            num_errores++;
        }

        return t2;
    }
    // si tablero terminado
    else {
        return -t2;
    }

}

/**
 * Reinicia las variables del juego.
 */
void reiniciar_juego(void) {
    uint8_t i, j;

    // reiniciar estadisticas
    num_errores = 0;
    num_acciones = 0;
    resultado_partida = 0;

    // reiniciar cuadricula
    for (i = 0; i < NUM_FILAS; i++) {
        for (j = 0; j < NUM_COLUMNAS - PADDING; j++) {
            cuadricula[i][j] = cuadricula_ini[i][j];
        }
    }
}

/**
 * Rellena el tablero del juego con los valores de la cuadricula.
 */
void rellenar_cuadricula(void) {

    uint8_t i, j;
    i = j = 0;

    // borra celdas en el Lcd y las rellena con nuevos valores
    for (i = 0; i < NUM_FILAS; i++) {
        for (j = 0; j < NUM_COLUMNAS - PADDING; j++) {

            CELDA celda = cuadricula[i][j];
            Lcd_limpiar_celda(i, j);
            Lcd_rellenar_celda(i, j, celda);
        }
    }
}

void preparar_introducir_fila(){

    // para filas el rango de valores posibles es [1,9]
    // pero se permite el 0 para finalizar la partida
    D8Led_define_rango(1, 0xA);

    // mostrar la letra F (de FILA) en el 8led
    D8Led_symbol(0xF);

    // desactivar avance de 8Led (para boton izquierdo)
    D8Led_desactivar_avance();
}

void preparar_introducir_columna(){

    // para filas el rango de valores posibles es [1,9]
    // pero se permite el 0 para finalizar la partida
    D8Led_define_rango(1, 9);

    // mostrar la letra F (de FILA) en el 8led
    D8Led_symbol(0xC);

    // desactivar avance de 8Led (para boton izquierdo)
    D8Led_desactivar_avance();
}

void Main() {

    // Inicializa controladores
    sys_init();         // inicializar la placa, interrupciones y puertos
    Lcd_Init();
    timer_init();       // inicializar el temporizador
    Eint4567_init();    // inicializar los pulsadores
    D8Led_init();       // inicializar el 8led

    // configurar e iniciar el Timer2
    Timer2_Inicializar();
    Timer2_Empezar();

    // desactivar avance de 8Led (para boton izquierdo)
    D8Led_desactivar_avance();

    uint8_t fila, columna;
    uint8_t fin = 0;
    uint8_t ultima_modificada;
    uint8_t ultimo_marcado;

    unsigned int tiempo_total = 0;
    unsigned int tiempo_calculo = 0;
    unsigned int alarma_5s = 0;
    unsigned int ultimo_segundo = 0;
    uint8_t parpadeo = 1;

    // PRESENTACION
    Lcd_pantalla_presentacion(-4);
    Delay(20000);

    int8_t i;
    for (i = -2; i < 10; i++) {
        Lcd_pantalla_presentacion(i);
        Delay(400);
    }

    Delay(20000);

    // INSTRUCCIONES
    Lcd_pantalla_inicial();

    while (1) {

        // ALARMA CADA 1 SEGUNDO (actualiza tiempos mostrados)
        if (estado_juego > 0 && fin == 0 && alarma_1s == 1) {
            Lcd_actualizar_tiempo_total(tiempo_total++);
            alarma_1s = 0;
        }

        // INICIAR JUEGO
        if (estado_juego == 0) {

            if (comprobarPulsacion(0) == 1) {

                tiempo_calculo = 0;
                tiempo_total = 0;

                Lcd_cuadricula_sudoku();
                tiempo_calculo += preparar_movimiento();
                preparar_introducir_fila();

                estado_juego++;

                Lcd_actualizar_tiempo_calculo(tiempo_calculo);
            }

        }

        // EMPIEZA A INTRODUCIR FILA
        else if (estado_juego == 1) {

            if (comprobarPulsacion(1) == 1) {

                // mostrar posicion inicial (un uno)
                D8Led_symbol(0x1);

                // Activar avance de 8Led (para boton izquierdo)
                D8Led_activar_avance();

                estado_juego++;
            }

        }

        // INTRODUCE FILA
        else if (estado_juego == 2) {

            // si ha confirmado abortar
            if (fin == 1 && comprobarPulsacion(2) == 1) {
                resultado_partida = -1;
                Lcd_pantalla_final(resultado_partida, num_acciones,
                        num_errores);
                reiniciar_juego();
                estado_juego = 0;
                fin = 0;

            }

            // si decide continuar sin abortar
            else if (fin == 1 && comprobarPulsacion(1) == 1) {

                Lcd_cuadricula_sudoku();
                rellenar_cuadricula();
                Lcd_actualizar_tiempo_calculo(tiempo_calculo);

                preparar_introducir_fila();

                estado_juego = 1;
                fin = 0;

            }

            // leer valor de fila
            else if (comprobarPulsacion(2) == 1) {

                // Desactivar avance de 8Led (para boton izquierdo)
                D8Led_desactivar_avance();

                // obtener numero de fila a partir del valor del 8led
                fila = valor_actual;

                // si se introduce una A -> terminar (Abortar)
                if (fila == 0xA) {
                    Lcd_pantalla_confirmar();
                    fin = 1;
                } else {

                    preparar_introducir_columna();

                    estado_juego++;
                }
            }
        }

        // EMPIEZA A INTRODUCIR COLUMNA
        else if (estado_juego == 3) {

            if (comprobarPulsacion(1) == 1) {

                // mostrar posicion inicial (un uno)
                D8Led_symbol(0x1);

                // activar avance de 8Led (para boton izquierdo)
                D8Led_activar_avance();

                estado_juego++;
            }

        }

        // INTRODUCE COLUMNA
        else if (estado_juego == 4) {

            if (comprobarPulsacion(2) == 1) {

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

                estado_juego++;
            }
        }

        // INTRODUCE VALOR
        else if (estado_juego == 5) {

            if (comprobarPulsacion(2) == 1) {

                // desactivar avance de 8Led (para boton izquierdo)
                D8Led_desactivar_avance();

                uint8_t modificado = 0;

                // almacenar el valor introducido para la celda (a partir del 8led)
                // si y solo si NO es una pista inicial
                if (celda_comprueba_pista(&cuadricula[fila - 1][columna - 1])
                        == 0) {

                    ultima_modificada = celda_leer_valor(
                            cuadricula[fila - 1][columna - 1]);
                    celda_poner_valor(&cuadricula[fila - 1][columna - 1],
                            valor_actual);
                    ultimo_marcado = valor_actual;

                    modificado = 1;
                }

                int t = preparar_movimiento();

                if (t > 0) {
                    tiempo_calculo += t;

                    if (modificado == 1) {
                        alarma_5s = tiempo_total + 5;
                        ultimo_segundo = tiempo_total;
                        Lcd_marcar_celda(fila - 1, columna - 1);
                        parpadeo = 1;
                        estado_juego = 6;
                    } else {
                        estado_juego = 1;
                    }

                } else {
                    tiempo_calculo += (-t);
                    resultado_partida = 1;
                    Lcd_pantalla_final(resultado_partida, num_acciones,
                            num_errores);
                    reiniciar_juego();
                    estado_juego = 0;
                }

                Lcd_actualizar_tiempo_calculo(tiempo_calculo);
            }
        }

        // CONFIRMACION DE VALOR
        else if (estado_juego == 6) {

            // parpadeo de recuadro de la celda seleccionada
            if (ultimo_segundo != tiempo_total) {
                ultimo_segundo = tiempo_total;
                if (parpadeo == 1) {
                    Lcd_desmarcar_celda(fila - 1, columna - 1);
                    D8Led_symbol(0x10);
                    parpadeo = 0;
                } else {
                    Lcd_marcar_celda(fila - 1, columna - 1);
                    D8Led_symbol(ultimo_marcado);
                    parpadeo = 1;
                }
            }

            if (comprobarPulsacion(0) == 1) {

                //desactivar alarma
                alarma_5s = 0;

                //descartar valor, restaurar anterior
                celda_poner_valor(&cuadricula[fila - 1][columna - 1],
                        ultima_modificada);

                int t = preparar_movimiento();

                if (t > 0) {
                    tiempo_calculo += t;
                    estado_juego = 1;
                } else {
                    tiempo_calculo += (-t);
                    resultado_partida = 1;
                    Lcd_pantalla_final(resultado_partida, num_acciones,
                            num_errores);
                    reiniciar_juego();
                    estado_juego = 0;
                }

                Lcd_actualizar_tiempo_calculo(tiempo_calculo);
            }

            // al cabo de 5s confirmar el valor
            if (alarma_5s > 0 && tiempo_total >= alarma_5s) {

                /*int t = preparar_movimiento();

                 if (t > 0) {
                 tiempo_calculo += t;
                 estado_juego = 1;
                 } else {
                 tiempo_calculo += (-t);
                 resultado_partida = 1;
                 Lcd_pantalla_final(resultado_partida, num_acciones, num_errores);
                 reiniciar_juego();
                 estado_juego = 0;
                 }

                 Lcd_actualizar_tiempo_calculo(tiempo_calculo);*/

                estado_juego = 1;
                alarma_5s = 0;
            }
        }
    }
}

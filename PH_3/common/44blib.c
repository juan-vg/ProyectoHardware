#include "44b.h"
#include "44blib.h"
#include "def.h"
#include "option.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>

#define STACKSIZE    0xa00 //SVC satck size(do not use user stack)//
#define HEAPEND     (_ISR_STARTADDRESS-STACKSIZE-0x500) // = 0xc7ff000//
//SVC Stack Area:0xc(e)7ff000-0xc(e)7ffaff//

/* PILA DEBUG */

// tamaño identico al resto de pilas, en direccion alineada
#define PILA_DEBUG_SIZE 256

// al final del bloque heap, sin interferir en las otras pilas
#define PILA_DEBUG_INI (HEAPEND)
#define PILA_DEBUG_FIN (PILA_DEBUG_INI - PILA_DEBUG_SIZE)

// sp de la pila debug
uint32_t* pila_debug_pointer = (uint32_t*) PILA_DEBUG_INI;
void push_debug(unsigned int, unsigned int);

/*************/

extern uint8_t switch_timer;
extern void D8Led_symbol(int);
extern int Timer2_Leer(void);

extern char Image_RW_Limit[];
volatile unsigned char *downPt;
unsigned int fileSize;
void *mallocPt = Image_RW_Limit;

void (*restart)(void) = (void (*)(void))0x0;
void (*run)(void) = (void (*)(void)) DOWNLOAD_ADDRESS;

//--------------------------------SYSTEM---------------------------------//
static int delayLoopCount = 400;

void Delay(int time)
// time=0: adjust the Delay function by WatchDog timer
// time>0: the number of loop time
// 100us resolution
// Delay(10) -> 10*100us resolution
{
	int i, adjust = 0;
	if (time == 0) {
		time = 200;
		adjust = 1;
		delayLoopCount = 400;
		rWTCON = ((MCLK / 1000000 - 1) << 8) | (2 << 3); // 1M/64,Watch-dog,nRESET,interrupt disable//
		rWTDAT = 0xffff;
		rWTCNT = 0xffff;
		rWTCON = ((MCLK / 1000000 - 1) << 8) | (2 << 3) | (1 << 5); // 1M/64,Watch-dog enable,nRESET,interrupt disable //
	}
	for (; time > 0; time--)
		for (i = 0; i < delayLoopCount; i++)
			;
	if (adjust == 1) {
		rWTCON = ((MCLK / 1000000 - 1) << 8) | (2 << 3);
		i = 0xffff - rWTCNT;   //  1count/16us?????????//
		delayLoopCount = 8000000 / (i * 64);	//400*100/(i*64/200)   //
	}
}

void DelayMs(int ms_time)
{
	int i;

	for (i = 0; i < 1000*ms_time ; i++)
		;
}

/**
 * Identifica el error que se ha producido y lo muestra en el 8Led
 *
 */
void mostrar_error_8led(uint8_t error, uint32_t dir_causante) {

	push_debug((unsigned int)0xDEB5 + error, (unsigned int)dir_causante);

	uint8_t mostrar_error = 0;
	uint32_t codigo;

	// UNDEF
	if (error == 0) {
		codigo = 0x1;
	}
	// DABORT
	else if (error == 1) {
		codigo = 0x2;
	}
	// PABORT
	else {
		codigo = 0x3;
	}

	// mostrar error indefinidamente
	while (1) {
		
		// mostrar la 'E' (de error)
		if (mostrar_error == 0) {
			D8Led_symbol(0xE);
			mostrar_error = 1;
		}
		
		// mostrar el codigo del error
		else if (mostrar_error == 1) {
			D8Led_symbol(codigo);
			mostrar_error = 2;
		} 
		
		// apagar 8Led con el simbolo 'blank' (efecto parpadeo)
		else {
			D8Led_symbol(0x10);
			mostrar_error = 0;
		}
		Delay(10000);
	}
}

//------------------------PORTS------------------------------//
void Port_Init(void) {
	//CAUTION:Follow the configuration order for setting the ports.
	// 1) setting value
		// 2) setting control register
	// 3) configure pull-up resistor.

	//16bit data bus configuration

	// PORT A GROUP
	// BIT 	9	8	7	6	5	4	3	2	1	0
	// 		A24	A23	A22	A21	A20	A19	A18	A17	A16	A0
	//		0	1	1	1	1	1	1	1	1	1
	rPCONA = 0x1ff;

	// PORT B GROUP
	// BIT 	10		9		8		7		6		5		4		3		2		1		0
	//		/CS5	/CS4	/CS3	/CS2	/CS1	GPB5	GPB4	/SRAS	/SCAS	SCLK	SCKE
	//		EXT		NIC		USB		IDE		SMC		NC		NC		Sdram	Sdram	Sdram	Sdram
	//      1, 		1,   	1,   	1,    	1,    	0,       0,     1,    	1,    	1,   	1
	rPDATB = 0x7ff;
	rPCONB = 0x1cf;			// P9-LED1 P10-LED2

	// PORT C GROUP
	// BUSWIDTH=16
	//  PC15	14		13		12		11		10		9		8
	//	I		I		RXD1	TXD1	I		I		I		I
	//	NC		NC		Uart1	Uart1	NC		NC		NC		NC
	//	00		00		11		11		00		00		00		00

	//  PC7		6		5		4		3		2		1		0
	//   I		I		I		I		I		I		I		I
	//   NC		NC		NC		NC		IISCLK	IISDI	IISDO	IISLRCK
	//   00		00		00		00		11		11		11		11
	rPDATC = 0xff00;
	rPCONC = 0x0ff0ffff;
	rPUPC = 0x30ff;	//PULL UP RESISTOR should be enabled to I/O

	// PORT D GROUP
	// PORT D GROUP(I/O OR LCD)
	// BIT7		6		5		4		3		2		1		0
	//	VF		VM		VLINE	VCLK	VD3		VD2		VD1		VD0
	//	00		00		00		00		00		00		00		00
	rPDATD = 0xff;
	rPCOND = 0xaaaa;
	rPUPD = 0x0;
	// These pins must be set only after CPU's internal LCD controller is enable

	// PORT E GROUP
	// Bit	8		7		6		5		4		3		2		1		0
	//  	CODECLK	LED4	LED5	LED6	LED7	BEEP	RXD0	TXD0	LcdDisp
	//  	10		01		01		01		01		01		10		10		01
	rPDATE = 0x1ff;
	rPCONE = 0x25529;
	rPUPE = 0x6;

	// PORT F GROUP
	// Bit8		7		6		5		 4		3		2		1		0
	// IISCLK	IISDI	IISDO	IISLRCK	Input	Input	Input	IICSDA	IICSCL
	// 100		100		100		100		00		00		00		10		10
	rPDATF = 0x0;
	rPCONF = 0x252a;
	rPUPF = 0x0;

	// PORT G GROUP
	// BIT7		6		5		4		3		2		1		0
	// 	INT7	INT6	INT5	INT4	INT3	INT2	INT1	INT0
	//	S3		S4		S5		S6		NIC		EXT		IDE		USB
	//	11      11      11      11      11      11      11      11
	rPDATG = 0xff;
	rPCONG = 0xffff;
	rPUPG = 0x0;		//pull-up portG enabled
	rSPUCR = 0x7;  		//D15-D0 pull-up disable

	// Non Cache area
	rNCACHBE0 = ((Non_Cache_End >> 12) << 16) | (Non_Cache_Start >> 12);

	/* Low level default */
	rEXTINT = 0x0;
}

//--------------------------------HEAP---------------------------------//
void *malloc(unsigned nbyte)
/* Very simple; Use malloc() & free() like Stack */
// void *mallocPt=Image$$RW$$Limit;
{
	void *returnPt = mallocPt;

	mallocPt = (int *) mallocPt + nbyte / 4 + ((nbyte % 4) > 0); //to align 4byte

	// el fin del heap esta condicionado por el inicio de la pila de depuracion
	if ((int) mallocPt > PILA_DEBUG_FIN) {
		mallocPt = returnPt;
		return NULL ;
	}
	return returnPt;
}

void free(void *pt) {
	mallocPt = pt;
}

/*
 * Inicializa la pila de depuracion con ceros
 */
void pila_debug_init() {
	uint32_t* pila = (uint32_t*) PILA_DEBUG_FIN;
	int i;
	for (i = 0; i <= (PILA_DEBUG_SIZE / 4); i++) {
		pila[i] = 0;
	}

	pila_debug_pointer = (uint32_t*) PILA_DEBUG_INI;
}

/*
 * Almacena un evento (y datos asociados) en la pila de depuracion.
 * Se trata de una pila circular con capacidad para 255 elementos
 * (multiplo de 3, ya que se apilan en esa cantidad)
 */
void push_debug(unsigned int ID_evento, unsigned int auxData) {
	
	// comprobar limite de la pila (pila circular)
	if (pila_debug_pointer <= (uint32_t*) PILA_DEBUG_FIN) {
		pila_debug_pointer = (uint32_t*) PILA_DEBUG_INI;
	}

	// reservar espacio (Decrement Before)
	pila_debug_pointer = pila_debug_pointer - 3;

	// TODO: struct

	// almacenar datos
	pila_debug_pointer[2] = ID_evento;
	pila_debug_pointer[1] = Timer2_Leer();
	pila_debug_pointer[0] = auxData;
}

//--------------------------------INIT---------------------------------//
void sys_init() // Interrupt & Port
{
	/* Config inicial para interrupciones */
	rINTMOD = 0x0;
	rINTCON = 0x1;
	rI_ISPC = 0xffffffff; /* clear all interrupt pend	*/
	rEXTINTPND = 0xf;				// clear EXTINTPND reg
	Port_Init(); /* Initial 44B0X's I/O port */
	Delay(0); /* delay time	*/

	// inicializar la pila de depuracion
	pila_debug_init();
	
	// enmascarar todas las lineas excepto el bit global (bit 26, definido en 44b.h)
	rINTMSK = ~BIT_GLOBAL;
}


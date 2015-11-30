/*********************************************************************************************
* File: 	DEF.H
* Author:	embest
* Desc: 	data type define
* History:
*********************************************************************************************/


//typedef uint16_t CELDA;
// La información de cada celda está codificada en 16 bits
// con el siguiente formato (empezando en el bit más significativo):
// 4 MSB VALOR
// 1 bit PISTA
// 1 bit ERROR
// 1 bit no usado
// 9 LSB CANDIDATOS

#define CELDA	unsigned short int

#ifndef __DEF_H_
#define __DEF_H_

#ifndef ULONG
#define ULONG	unsigned long
#endif

#define UINT	unsigned int
#define USHORT	unsigned short
#define UCHAR	unsigned char
#define U32 	unsigned int
#define INT32U  unsigned int
#define INT32	int
#define U16 	unsigned short
#define INT16U  unsigned short
#define INT16	short int
#define S32 	int
#define S16 	short int
#define U8  	unsigned char
#define INT8U 	unsigned char
#define byte	unsigned char
#define INT8 	char
#define	S8  	char

#ifndef FALSE
#define FALSE		0
#endif
#ifndef TRUE
#define TRUE		1
#endif

#define OK		1
#define FAIL	0
#define FileEnd	1
#define	NotEnd	0

#endif /* __DEF_H_ */

/*********************************************************************************************
 * Fichero:	lcd.c
 * Autor:
 * Descrip:	funciones de visualizacion y control LCD
 * Version:	<P6-ARM>
 *********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "def.h"
#include "44b.h"
#include "44blib.h"
#include "lcd.h"
#include "bmp.h"

/*--- definicion de macros ---*/
#define DMA_Byte  (0)
#define DMA_HW    (1)
#define DMA_Word  (2)
#define DW 		  DMA_Byte		//configura  ZDMA0 como media palabras

/*--- variables externas ---*/
extern INT8U g_auc_Ascii8x16[];
extern STRU_BITMAP Stru_Bitmap_gbMouse;

/*--- variables globales ---*/
INT16 margen_H = 76;
INT16 margen_V = 32;

/*--- definicion de funciones ---*/

void acentuar_vocales(void);
void acentuar_ascii(INT8U ref_val_ascii, INT8U val_ascii);
void escribirAscii(INT8U val_ascii, INT8U representacion[16]);

/*--- codigo de las funciones ---*/
void Lcd_Init(void) {
    rDITHMODE = 0x1223a;
    rDP1_2 = 0x5a5a;
    rDP4_7 = 0x366cd9b;
    rDP3_5 = 0xda5a7;
    rDP2_3 = 0xad7;
    rDP5_7 = 0xfeda5b7;
    rDP3_4 = 0xebd7;
    rDP4_5 = 0xebfd7;
    rDP6_7 = 0x7efdfbf;

    rLCDCON1 = (0) | (1 << 5) | (MVAL_USED << 7) | (0x0 << 8) | (0x0 << 10)
            | (CLKVAL_GREY16 << 12);
    rLCDCON2 = (LINEVAL) | (HOZVAL << 10) | (10 << 21);
    rLCDSADDR1 = (0x2 << 27)
            | (((LCD_ACTIVE_BUFFER >> 22) << 21) | M5D(LCD_ACTIVE_BUFFER>>1));
    rLCDSADDR2 = M5D(((LCD_ACTIVE_BUFFER+(SCR_XSIZE*LCD_YSIZE/2))>>1))
            | (MVAL << 21);
    rLCDSADDR3 = (LCD_XSIZE / 4) | (((SCR_XSIZE - LCD_XSIZE)/4)<<9 );
    // enable,4B_SNGL_SCAN,WDLY=8clk,WLH=8clk,
    rLCDCON1 = (1) | (1 << 5) | (MVAL_USED << 7) | (0x3 << 8) | (0x3 << 10)
            | (CLKVAL_GREY16 << 12);
    rBLUELUT = 0xfa40;
    //Enable LCD Logic and EL back-light.
    rPDATE = rPDATE & 0x0e;

    //DMA ISR
    rINTMSK &= ~(BIT_GLOBAL | BIT_ZDMA0);
    pISR_ZDMA0 = (int) Zdma0Done;

    //crear simbolos nuevos para letras acentuadas
    acentuar_vocales();
}

void escribirAscii(INT8U val_ascii, INT8U representacion[16]) {
    unsigned int pos = val_ascii * 16;
    INT16U i;

    for (i = 0; i < 16; i++) {
        g_auc_Ascii8x16[pos + i] = representacion[i];
    }
}

void acentuar_ascii(INT8U ref_val_ascii, INT8U val_ascii) {
    INT8U representacion[16];

    INT16U pos = ref_val_ascii * 16;

    INT8U i;

    for (i = 0; i < 16; i++) {
        representacion[i] = g_auc_Ascii8x16[pos + i];
    }

    if (ref_val_ascii >= 'a' && ref_val_ascii <= 'z') {
        representacion[2] = 0x0c;
        representacion[3] = 0x18;
        representacion[4] = 0x30;
    } else {
        representacion[0] = 0x0c;
        representacion[1] = 0x18;
    }

    escribirAscii(val_ascii, representacion);
}

void acentuar_vocales(void) {
    acentuarAscii('a', 'á');
    acentuarAscii('e', 'é');
    acentuarAscii('i', 'í');
    acentuarAscii('o', 'ó');
    acentuarAscii('u', 'ú');

    acentuarAscii('A', 'Á');
    acentuarAscii('E', 'É');
    acentuarAscii('I', 'Í');
    acentuarAscii('O', 'Ó');
    acentuarAscii('U', 'Ú');
}

/*********************************************************************************************
 * name:		Lcd_Active_Clr()
 * func:		clear LCD screen
 * para:		none
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void Lcd_Active_Clr(void) {
    INT32U i;
    INT32U *pDisp = (INT32U *) LCD_ACTIVE_BUFFER;

    for (i = 0; i < (SCR_XSIZE * SCR_YSIZE / 2 / 4); i++) {
        *pDisp++ = WHITE;
    }
}

/*********************************************************************************************
 * name:		Lcd_GetPixel()
 * func:		Get appointed point's color value
 * para:		usX,usY -- pot's X-Y coordinate
 * ret:		pot's color value
 * modify:
 * comment:
 *********************************************************************************************/INT8U LCD_GetPixel(
        INT16U usX, INT16U usY) {
    INT8U ucColor;

    ucColor = *((INT8U*) (LCD_VIRTUAL_BUFFER + usY * SCR_XSIZE / 2 + usX / 8 * 4
            + 3 - (usX % 8) / 2));
    ucColor = (ucColor >> ((1 - (usX % 2)) * 4)) & 0x0f;
    return ucColor;
}

/*********************************************************************************************
 * name:		Lcd_Active_Clr()
 * func:		clear virtual screen
 * para:		none
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void Lcd_Clr(void) {
    INT32U i;
    INT32U *pDisp = (INT32U *) LCD_VIRTUAL_BUFFER;

    for (i = 0; i < (SCR_XSIZE * SCR_YSIZE / 2 / 4); i++) {
        *pDisp++ = WHITE;
    }
}

/*********************************************************************************************
 * name:		LcdClrRect()
 * func:		fill appointed area with appointed color
 * para:		usLeft,usTop,usRight,usBottom -- area's rectangle acme coordinate
 *			ucColor -- appointed color value
 * ret:		none
 * modify:
 * comment:	also as clear screen function
 *********************************************************************************************/
void LcdClrRect(INT16 usLeft, INT16 usTop, INT16 usRight, INT16 usBottom,
        INT8U ucColor) {
    INT16 i, k, l, m;

    INT32U ulColor = (ucColor << 28) | (ucColor << 24) | (ucColor << 20)
            | (ucColor << 16) | (ucColor << 12) | (ucColor << 8)
            | (ucColor << 4) | ucColor;

    i = k = l = m = 0;
    if ((usRight - usLeft) <= 8) {
        for (i = usTop; i <= usBottom; i++) {
            for (m = usLeft; m <= usRight; m++) {
                LCD_PutPixel(m, i, ucColor);
            }
        }
        return;
    }

    /* check borderline */
    if (0 == (usLeft % 8))
        k = usLeft;
    else {
        k = (usLeft / 8) * 8 + 8;
    }
    if (0 == (usRight % 8))
        l = usRight;
    else {
        l = (usRight / 8) * 8;
    }

    for (i = usTop; i <= usBottom; i++) {
        for (m = usLeft; m <= (k - 1); m++) {
            LCD_PutPixel(m, i, ucColor);
        }
        for (m = k; m < l; m += 8) {
            (*(INT32U*) (LCD_VIRTUAL_BUFFER + i * SCR_XSIZE / 2 + m / 2)) =
                    ulColor;
        }
        for (m = l; m <= usRight; m++) {
            LCD_PutPixel(m, i, ucColor);
        }
    }
}

/*********************************************************************************************
 * name:		Lcd_Draw_Box()
 * func:		Draw rectangle with appointed color
 * para:		usLeft,usTop,usRight,usBottom -- rectangle's acme coordinate
 *			ucColor -- appointed color value
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void Lcd_Draw_Box(INT16 usLeft, INT16 usTop, INT16 usRight, INT16 usBottom,
        INT8U ucColor) {
    Lcd_Draw_HLine(usLeft, usRight, usTop, ucColor, 1);
    Lcd_Draw_HLine(usLeft, usRight, usBottom, ucColor, 1);
    Lcd_Draw_VLine(usTop, usBottom, usLeft, ucColor, 1);
    Lcd_Draw_VLine(usTop, usBottom, usRight, ucColor, 1);
}

/*********************************************************************************************
 * name:		Lcd_Draw_Line()
 * func:		Draw line with appointed color
 * para:		usX0,usY0 -- line's start point coordinate
 *			usX1,usY1 -- line's end point coordinate
 *			ucColor -- appointed color value
 *			usWidth -- line's width
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void Lcd_Draw_Line(INT16 usX0, INT16 usY0, INT16 usX1, INT16 usY1,
        INT8U ucColor, INT16U usWidth) {
    INT16 usDx;
    INT16 usDy;
    INT16 y_sign;
    INT16 x_sign;
    INT16 decision;
    INT16 wCurx, wCury, wNextx, wNexty, wpy, wpx;

    if (usY0 == usY1) {
        Lcd_Draw_HLine(usX0, usX1, usY0, ucColor, usWidth);
        return;
    }
    if (usX0 == usX1) {
        Lcd_Draw_VLine(usY0, usY1, usX0, ucColor, usWidth);
        return;
    }
    usDx = abs(usX0 - usX1);
    usDy = abs(usY0 - usY1);
    if (((usDx >= usDy && (usX0 > usX1)) || ((usDy > usDx) && (usY0 > usY1)))) {
        GUISWAP(usX1, usX0);
        GUISWAP(usY1, usY0);
    }
    y_sign = (usY1 - usY0) / usDy;
    x_sign = (usX1 - usX0) / usDx;

    if (usDx >= usDy) {
        for (wCurx = usX0, wCury = usY0, wNextx = usX1, wNexty = usY1, decision =
                (usDx >> 1); wCurx <= wNextx; wCurx++, wNextx--, decision +=
                usDy) {
            if (decision >= usDx) {
                decision -= usDx;
                wCury += y_sign;
                wNexty -= y_sign;
            }
            for (wpy = wCury - usWidth / 2; wpy <= wCury + usWidth / 2; wpy++) {
                LCD_PutPixel(wCurx, wpy, ucColor);
            }

            for (wpy = wNexty - usWidth / 2; wpy <= wNexty + usWidth / 2;
                    wpy++) {
                LCD_PutPixel(wNextx, wpy, ucColor);
            }
        }
    } else {
        for (wCurx = usX0, wCury = usY0, wNextx = usX1, wNexty = usY1, decision =
                (usDy >> 1); wCury <= wNexty; wCury++, wNexty--, decision +=
                usDx) {
            if (decision >= usDy) {
                decision -= usDy;
                wCurx += x_sign;
                wNextx -= x_sign;
            }
            for (wpx = wCurx - usWidth / 2; wpx <= wCurx + usWidth / 2; wpx++) {
                LCD_PutPixel(wpx, wCury, ucColor);
            }

            for (wpx = wNextx - usWidth / 2; wpx <= wNextx + usWidth / 2;
                    wpx++) {
                LCD_PutPixel(wpx, wNexty, ucColor);
            }
        }
    }
}

/*********************************************************************************************
 * name:		Lcd_Draw_HLine()
 * func:		Draw horizontal line with appointed color
 * para:		usX0,usY0 -- line's start point coordinate
 *			usX1 -- line's end point X-coordinate
 *			ucColor -- appointed color value
 *			usWidth -- line's width
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void Lcd_Draw_HLine(INT16 usX0, INT16 usX1, INT16 usY0, INT8U ucColor,
        INT16U usWidth) {
    INT16 usLen;

    if (usX1 < usX0) {
        GUISWAP(usX1, usX0);
    }

    while ((usWidth--) > 0) {
        usLen = usX1 - usX0 + 1;
        while ((usLen--) > 0) {
            LCD_PutPixel(usX0 + usLen, usY0, ucColor);
        }
        usY0++;
    }
}

/*********************************************************************************************
 * name:		Lcd_Draw_VLine()
 * func:		Draw vertical line with appointed color
 * para:		usX0,usY0 -- line's start point coordinate
 *			usY1 -- line's end point Y-coordinate
 *			ucColor -- appointed color value
 *			usWidth -- line's width
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void Lcd_Draw_VLine(INT16 usY0, INT16 usY1, INT16 usX0, INT8U ucColor,
        INT16U usWidth) {
    INT16 usLen;

    if (usY1 < usY0) {
        GUISWAP(usY1, usY0);
    }

    while ((usWidth--) > 0) {
        usLen = usY1 - usY0 + 1;
        while ((usLen--) > 0) {
            LCD_PutPixel(usX0, usY0 + usLen, ucColor);
        }
        usX0++;
    }
}

/*********************************************************************************************
 * name:		Lcd_DspAscII8x16()
 * func:		display 8x16 ASCII character string
 * para:		usX0,usY0 -- ASCII character string's start point coordinate
 *			ForeColor -- appointed color value
 *			pucChar   -- ASCII character string
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void Lcd_DspAscII8x16(INT16U x0, INT16U y0, INT8U ForeColor, INT8U * s) {
    INT16 i, j, k, x, y, xx;
    INT8U qm;
    INT32U ulOffset;
    INT8 ywbuf[16], temp[2];

    for (i = 0; i < strlen((const char*) s); i++) {
        qm = *(s + i);

        // adaptacion para eñes
        if(qm == 'ñ'){
            qm = 28;
        }

        ulOffset = (INT32U) (qm) * 16;     //Here to be changed tomorrow
        for (j = 0; j < 16; j++) {
            ywbuf[j] = g_auc_Ascii8x16[ulOffset + j];
        }

        for (y = 0; y < 16; y++) {
            for (x = 0; x < 8; x++) {
                k = x % 8;
                if (ywbuf[y] & (0x80 >> k)) {
                    xx = x0 + x + i * 8;
                    LCD_PutPixel(xx, (y + y0), (INT8U)ForeColor);
                }
            }
        }
    }

    /*for (i = 0; i < strlen((const char*) s); i++) {
     if ((INT8U) *(s + i) >= 161) {
     temp[0] = *(s + i);
     temp[1] = '\0';
     return;
     } else {
     qm = *(s + i);
     ulOffset = (INT32U) (qm) * 16;		//Here to be changed tomorrow
     for (j = 0; j < 16; j++) {
     ywbuf[j] = g_auc_Ascii8x16[ulOffset + j];
     }

     for (y = 0; y < 16; y++) {
     for (x = 0; x < 8; x++) {
     k = x % 8;
     if (ywbuf[y] & (0x80 >> k)) {
     xx = x0 + x + i * 8;
     LCD_PutPixel(xx, (y + y0), (INT8U)ForeColor);
     }
     }
     }
     }
     }*/
}

/*********************************************************************************************
 * name:		ReverseLine()
 * func:		Reverse display some lines
 * para:		ulHeight -- line's height
 *			ulY -- line's Y-coordinate
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void ReverseLine(INT32U ulHeight, INT32U ulY) {
    INT32U i, j, temp;

    for (i = 0; i < ulHeight; i++) {
        for (j = 0; j < (SCR_XSIZE / 4 / 2); j++) {
            temp = *(INT32U*) (LCD_VIRTUAL_BUFFER + (ulY + i) * SCR_XSIZE / 2
                    + j * 4);
            temp ^= 0xFFFFFFFF;
            *(INT32U*) (LCD_VIRTUAL_BUFFER + (ulY + i) * SCR_XSIZE / 2 + j * 4) =
                    temp;
        }
    }
}

/*********************************************************************************************
 * name:		Zdma0Done()
 * func:		LCD dma interrupt handle function
 * para:		none
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
static INT8U ucZdma0Done = 1; //When DMA is finish,ucZdma0Done is cleared to Zero
void Zdma0Done(void) {
    rI_ISPC = BIT_ZDMA0;	    //clear pending
    ucZdma0Done = 0;
}

/*********************************************************************************************
 * name:		Lcd_Dma_Trans()
 * func:		dma transport virtual LCD screen to LCD actual screen
 * para:		none
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void Lcd_Dma_Trans(void) {
    INT8U err;

    ucZdma0Done = 1;
    //#define LCD_VIRTUAL_BUFFER	(0xc400000)
    //#define LCD_ACTIVE_BUFFER	(LCD_VIRTUAL_BUFFER+(SCR_XSIZE*SCR_YSIZE/2))	//DMA ON
    //#define LCD_ACTIVE_BUFFER	LCD_VIRTUAL_BUFFER								//DMA OFF
    //#define LCD_BUF_SIZE		(SCR_XSIZE*SCR_YSIZE/2)
    //So the Lcd Buffer Low area is from LCD_VIRTUAL_BUFFER to (LCD_ACTIVE_BUFFER+(SCR_XSIZE*SCR_YSIZE/2))
    rNCACHBE1 = (((unsigned) (LCD_ACTIVE_BUFFER) >> 12) << 16)
            | ((unsigned) (LCD_VIRTUAL_BUFFER) >> 12);
    rZDISRC0 = (DW << 30) | (1 << 28) | LCD_VIRTUAL_BUFFER; // inc
    rZDIDES0 = (2 << 30) | (1 << 28) | LCD_ACTIVE_BUFFER; // inc
    rZDICNT0 = (2 << 28) | (1 << 26) | (3 << 22) | (0 << 20) | (LCD_BUF_SIZE);
    //                      |            |            |             |            |---->0 = Disable DMA
    //                      |            |            |             |------------>Int. whenever transferred
    //                      |            |            |-------------------->Write time on the fly
    //                      |            |---------------------------->Block(4-word) transfer mode
    //                      |------------------------------------>whole service
    //reEnable ZDMA transfer
    rZDICNT0 |= (1 << 20);		//after ES3
    rZDCON0 = 0x1; // start!!!

    //Delay(500);
    while (ucZdma0Done)
        ;		//wait for DMA finish
}

/*********************************************************************************************
 * name:		Lcd_Test()
 * func:		LCD test function
 * para:		none
 * ret:		none
 * modify:
 * comment:
 *********************************************************************************************/
void Lcd_Test(void) {
    /* initial LCD controller */
    Lcd_Init();
    /* clear screen */
    Lcd_Clr();
    Lcd_Active_Clr();

    /* draw rectangle pattern */
#ifdef Eng_v // english version
    Lcd_DspAscII8x16(10,0,DARKGRAY,"Embest S3CEV40 ");
#else
//	Lcd_DspHz16(10,0,DARKGRAY,"Ó¢ÝíÌØÈýÐÇÊµÑéÆÀ¹À°å");
#endif
    Lcd_DspAscII8x16(10, 20, BLACK, "Codigo del puesto: ");
    Lcd_Draw_Box(10, 40, 310, 230, 14);
    Lcd_Draw_Box(20, 45, 300, 225, 13);
    Lcd_Draw_Box(30, 50, 290, 220, 12);
    Lcd_Draw_Box(40, 55, 280, 215, 11);
    Lcd_Draw_Box(50, 60, 270, 210, 10);
    Lcd_Draw_Box(60, 65, 260, 205, 9);
    Lcd_Draw_Box(70, 70, 250, 200, 8);
    Lcd_Draw_Box(80, 75, 240, 195, 7);
    Lcd_Draw_Box(90, 80, 230, 190, 6);
    Lcd_Draw_Box(100, 85, 220, 185, 5);
    Lcd_Draw_Box(110, 90, 210, 180, 4);
    Lcd_Draw_Box(120, 95, 200, 175, 3);
    Lcd_Draw_Box(130, 100, 190, 170, 2);
    BitmapView(10, 40, Stru_Bitmap_gbMouse);
    Lcd_Dma_Trans();

}

void Lcd_pantalla_presentacion(void) {

    /* clear screen */
    Lcd_Clr();
    Lcd_Active_Clr();

    // margenes
    INT16U mH_lcd = 10;
    INT16U mH_letra = 5;
    INT16U mV_letra = 10;

    // tamaños
    INT16U tH_letra = 50 - mH_letra - mH_letra;
    INT16U tV_letra = 80 - mV_letra - mV_letra;

    // offsets
    INT16U offsetH = mH_lcd + mH_letra;
    INT16U offsetV = 80 + mV_letra;

    // Letra S: H(80-160) W(10-60)
    Lcd_Draw_HLine(offsetH, offsetH + tH_letra, offsetV, BLACK, 10);
    Lcd_Draw_HLine(offsetH, offsetH + tH_letra, offsetV + 30, BLACK, 10);
    Lcd_Draw_HLine(offsetH, offsetH + tH_letra, offsetV + 60, BLACK, 10);
    Lcd_Draw_VLine(offsetV, offsetV + 30, offsetH, BLACK, 10);
    Lcd_Draw_VLine(offsetV + 30, offsetV + 60, offsetH + tH_letra, BLACK, 10);

    offsetH += tH_letra + mH_letra + mH_letra;

    // Letra U: H(80-160) W(60-110)
    Lcd_Draw_HLine(offsetH, offsetH + tH_letra, offsetV + 60, BLACK, 10);
    Lcd_Draw_VLine(offsetV, offsetV + 60, offsetH, BLACK, 10);
    Lcd_Draw_VLine(offsetV, offsetV + 60, offsetH + tH_letra, BLACK, 10);

    offsetH += tH_letra + mH_letra + mH_letra;

    // Letra D: H(80-160) W(110-160)
    Lcd_Draw_HLine(offsetH, offsetH + 30, offsetV, BLACK, 10);
    Lcd_Draw_HLine(offsetH, offsetH + 30, offsetV + 60, BLACK, 10);
    Lcd_Draw_VLine(offsetV, offsetV + 60, offsetH, BLACK, 10);
    // + 2 diagonales
    Lcd_Draw_Line(offsetH + 30, offsetV, offsetH + 40, offsetV + 30, BLACK, 10);
    Lcd_Draw_Line(offsetH + 40, offsetV + 30, offsetH + 30, offsetV + 60, BLACK,
            10);

    offsetH += tH_letra + mH_letra + mH_letra;

    // Letra O: H(80-160) W(160-210)
    Lcd_Draw_HLine(offsetH, offsetH + tH_letra, offsetV, BLACK, 10);
    Lcd_Draw_HLine(offsetH, offsetH + tH_letra, offsetV + 60, BLACK, 10);
    Lcd_Draw_VLine(offsetV, offsetV + 60, offsetH, BLACK, 10);
    Lcd_Draw_VLine(offsetV, offsetV + 60, offsetH + tH_letra, BLACK, 10);

    offsetH += tH_letra + mH_letra + mH_letra;

    // Letra K: H(80-160) W(210-260)
    Lcd_Draw_VLine(offsetV, offsetV + 60, offsetH, BLACK, 10);
    // + 2 diagonales
    Lcd_Draw_Line(offsetH + tH_letra, offsetV, offsetH, offsetV + 30, BLACK,
            10);
    Lcd_Draw_Line(offsetH, offsetV + 30, offsetH + tH_letra, offsetV + 60,
            BLACK, 10);

    offsetH += tH_letra + mH_letra + mH_letra;

    // Letra U: H(80-160) W(260-310)
    Lcd_Draw_HLine(offsetH, offsetH + tH_letra, offsetV + 60, BLACK, 10);
    Lcd_Draw_VLine(offsetV, offsetV + 60, offsetH, BLACK, 10);
    Lcd_Draw_VLine(offsetV, offsetV + 60, offsetH + tH_letra, BLACK, 10);

    offsetH += tH_letra + mH_letra + mH_letra;
}

/**
 * Muestra una pantalla con las instrucciones del Sudoku.
 */
void Lcd_pantalla_inicial(void) {

    /* clear screen */
    Lcd_Clr();
    Lcd_Active_Clr();

    Lcd_DspAscII8x16(10, 0, BLACK, "Instrucciones del juego: ");
    Lcd_DspAscII8x16(20, 16, BLACK, "1. Hay que completar las casillas");
    Lcd_DspAscII8x16(30, 32, BLACK, "vacias con un numero del 1 al 9.");

    Lcd_DspAscII8x16(20, 48, BLACK, "2. No puede haber numeros repetidos");
    Lcd_DspAscII8x16(30, 64, BLACK, "en la misma fila, columna o region.");

    Lcd_DspAscII8x16(20, 80, BLACK, "3. Las casillas negras son pistas.");
    Lcd_DspAscII8x16(30, 96, BLACK, "(Las pistas no se pueden modificar)");

    Lcd_DspAscII8x16(20, 112, BLACK, "4. Los candidatos para cada celda");
    Lcd_DspAscII8x16(30, 128, BLACK, "vacia se representan como puntos");
    Lcd_DspAscII8x16(30, 144, BLACK, "ordenados (primero arriba izq.)");

    Lcd_DspAscII8x16(20, 160, BLACK, "5. Los errores se muestran con una X.");
    Lcd_DspAscII8x16(30, 176, BLACK, "(Se marcan las casillas implicadas)");

    Lcd_DspAscII8x16(20, 192, BLACK, "6. Al introducir un valor, durante 5s,");
    Lcd_DspAscII8x16(30, 208, BLACK, "se puede cancelar pulsando un boton.");

    Lcd_DspAscII8x16(50, 224, BLACK, "Pulse un boton para jugar!");
    BitmapView(320, 240, Stru_Bitmap_gbMouse);
    Lcd_Dma_Trans();
}

/**
 * Muestra una pantalla con los resultados del juego.
 */
void Lcd_pantalla_final(INT8 result, INT8U num_acc, INT8U num_err) {

    LcdClrRect(0, 16, 320, 240, WHITE);

    Lcd_DspAscII8x16(100, 0, BLACK, "JUEGO TERMINADO!");

    Lcd_DspAscII8x16(20, 32, BLACK, "Resultado:");
    if (result > 0) {
        Lcd_DspAscII8x16(108, 32, BLACK, "Juego completado");
    } else {
        Lcd_DspAscII8x16(108, 32, BLACK, "Juego abortado");
    }

    INT8U ascii_val[4] = { 0, 0, 0, '\0' };
    INT8U i;

    for (i = 2; i >= 0; i--) {
        ascii_val[i] = (num_acc % 10) + 48;
        num_acc /= 10;
    }
    Lcd_DspAscII8x16(20, 48, BLACK, "Num. acciones realizadas:");
    Lcd_DspAscII8x16(228, 48, BLACK, ascii_val);

    for (i = 2; i >= 0; i--) {
        ascii_val[i] = (num_err % 10) + 48;
        num_err /= 10;
    }
    Lcd_DspAscII8x16(20, 64, BLACK, "Num. errores cometidos:");
    Lcd_DspAscII8x16(212, 64, BLACK, ascii_val);

    Lcd_DspAscII8x16(50, 224, BLACK, "Pulse un boton para jugar");
    BitmapView(320, 240, Stru_Bitmap_gbMouse);
    Lcd_Dma_Trans();
}

/**
 * Muestra una pantalla de confirmacion (si el usuario desea terminar
 * la partida).
 */
void Lcd_pantalla_confirmar(void) {

    LcdClrRect(0, 16, 320, 240, WHITE);

    Lcd_DspAscII8x16(100, 100, BLACK, "TERMINAR PARTIDA?");

    Lcd_DspAscII8x16(80, 132, BLACK, "SEGUIR:   Boton izquierdo");
    Lcd_DspAscII8x16(80, 148, BLACK, "TERMINAR: Boton derecho");
    BitmapView(320, 240, Stru_Bitmap_gbMouse);
    Lcd_Dma_Trans();
}

/**
 * Actualiza el valor del tiempo de calculo que se muestra en la pantalla
 * segun el parametro [tiempo].
 */
void Lcd_actualizar_tiempo_calculo(int tiempo) {
    INT8U t[8] = { 0, 0, 0, 0, 0, 0, 0, '\0' };
    LcdClrRect(82, 0, 138, 15, WHITE);

    int i;

    for (i = 6; i >= 0; i--) {
        t[i] = (tiempo % 10) + 48;
        tiempo /= 10;
    }

    Lcd_DspAscII8x16(82, 0, BLACK, t);
    //Lcd_Dma_Trans();
}

/**
 * Actualiza el valor del tiempo total que se muestra en la pantalla segun
 * el parametro [tiempo].
 */
void Lcd_actualizar_tiempo_total(int tiempo) {
    INT8U t[6] = { 0, 0, ':', 0, 0, '\0' };
    LcdClrRect(270, 0, 310, 15, WHITE);
    int minutos = tiempo / 60;
    int segundos = tiempo % 60;

    t[0] = (minutos / 10) + 48;
    t[1] = (minutos % 10) + 48;
    t[3] = (segundos / 10) + 48;
    t[4] = (segundos % 10) + 48;

    Lcd_DspAscII8x16(270, 0, BLACK, t);
    Lcd_Dma_Trans();
}

/**
 * Dibuja en la pantalla la cuadricula del Sudoku.
 */
void Lcd_cuadricula_sudoku(void) {
    /* clear screen */
    Lcd_Clr();
    Lcd_Active_Clr();

    Lcd_DspAscII8x16(10, 0, BLACK, "T. calc: ");

    Lcd_DspAscII8x16(139, 0, BLACK, "us");

    Lcd_DspAscII8x16(206, 0, BLACK, "T. tot: ");

    Lcd_Draw_HLine(0, 320, 16, BLACK, 1);

    INT16 offset_H, offset_V;

    offset_H = margen_V;
    offset_V = margen_H;

    INT16 long_cuadricula = 167 + 19 + 2;

    // pinta cuadricula
    int i;
    for (i = 0; i < 10; i++) {
        INT16U ancho_linea;

        if (i % 3 == 0) {
            ancho_linea = 3;
        } else {
            ancho_linea = 1;
        }
        /*Lcd_Draw_HLine(8, 320, 32 + i*23, BLACK, ancho_linea);
         Lcd_Draw_VLine(32, 240, 8 + i*34, BLACK, ancho_linea);*/

        /*Lcd_Draw_HLine(137, 320, 40 + i*20, BLACK, ancho_linea);
         Lcd_Draw_VLine(40, 222, 137 + i*20, BLACK, ancho_linea);*/

        Lcd_Draw_HLine(margen_H, margen_H + long_cuadricula, offset_H, BLACK,
                ancho_linea);
        Lcd_Draw_VLine(margen_V, margen_V + long_cuadricula, offset_V, BLACK,
                ancho_linea);

        offset_H += 19 + ancho_linea;
        offset_V += 19 + ancho_linea;
    }

    // pinta nums de fila y columna
    INT16 posicion = 8;
    INT8U num[2] = { 0, '\0' };
    for (i = 1; i < 10; i++) {

        // transforma a ascii
        num[0] = i + 48;
        Lcd_DspAscII8x16(margen_H + posicion, 16, BLACK, num);
        Lcd_DspAscII8x16(margen_H - 12, margen_V + posicion - 4, BLACK, num);
        posicion += 19;

        if (i % 3 == 0) {
            posicion += 3;
        } else {
            posicion += 1;
        }
    }

    Lcd_DspAscII8x16(0, 224, BLACK, "Introduzca Fila A para acabar la partida");
    BitmapView(320, 240, Stru_Bitmap_gbMouse);
    Lcd_Dma_Trans();
}

/**
 * Limpia el contenido de una celda de la cuadricula cuya posicion
 * viene indicada por la fila [fila] y columna [columna].
 */
void Lcd_limpiar_celda(INT8U fila, INT8U columna) {
    INT16U px_fila = fila * 19;
    INT16U px_columna = columna * 19;

    // sumar incremento (2=3-1) de lineas gruesas (3) con respecto a las finas (1)
    px_fila += ((fila / 3) + 1) * 2;
    px_columna += ((columna / 3) + 1) * 2;

    // sumar lineas finas
    px_fila += fila + 1;
    px_columna += columna + 1;

    // sumar margenes de la cuadricula
    px_fila += margen_V;
    px_columna += margen_H;

    LcdClrRect(px_columna, px_fila, px_columna + 19 - 1, px_fila + 19 - 1,
            WHITE);
}

/**
 * Marca con un rectangulo de color [color] la celda de la cuadricula cuya
 * posicion viene indicada por la fila [fila] y columna [columna].
 */
void Lcd_marcar_celda_color(INT8U fila, INT8U columna, INT8U color) {
    INT16U px_fila = fila * 19;
    INT16U px_columna = columna * 19;

    // sumar incremento (2=3-1) de lineas gruesas (3) con respecto a las finas (1)
    px_fila += ((fila / 3) + 1) * 2;
    px_columna += ((columna / 3) + 1) * 2;

    // sumar lineas finas
    px_fila += fila + 1;
    px_columna += columna + 1;

    // sumar margenes de la cuadricula
    px_fila += margen_V;
    px_columna += margen_H;

    Lcd_Draw_Box(px_columna, px_fila, px_columna + 19 - 1, px_fila + 19 - 1,
            color);
    Lcd_Draw_Box(px_columna + 1, px_fila + 1, px_columna + 19 - 2,
            px_fila + 19 - 2, color);
}

/**
 * Marca con un rectangulo la celda de la cuadricula cuya posicion viene
 * indicada por la fila [fila] y columna [columna].
 */
void Lcd_marcar_celda(INT8U fila, INT8U columna) {
    Lcd_marcar_celda_color(fila, columna, BLACK);
}

/**
 * Desmarca la celda de la cuadricula cuya posicion viene indicada por la fila
 * [fila] y columna [columna].
 */
void Lcd_desmarcar_celda(INT8U fila, INT8U columna) {
    Lcd_marcar_celda_color(fila, columna, WHITE);
}

/**
 * Dibuja los numeros candidatos [candidatos] en la celda de la cuadricula cuya
 * posicion viene indicada por la fila [px_fila] y columna [px_columna]. Los
 * candidatos se muestran ordenados de izquierda a derecha y de arriba a abajo.
 *
 */
void rellenar_candidatos(INT16U px_fila, INT16U px_columna, INT16U candidatos) {

    INT16U fila, columna;
    INT8U margen = 1;
    INT8U lado = 4;
    INT8U i = 0;

    px_fila += margen;
    px_columna += margen;

    fila = px_fila;
    columna = px_columna;

    for (i = 1; i <= 9; i++) {

        if ((candidatos & (0x1 << (i - 1))) > 0) {
            LcdClrRect(columna, fila, columna + lado, fila + lado, BLACK);
        }

        columna += lado + margen + 1;

        if (i % 3 == 0) {
            fila += lado + margen + 1;
            columna = px_columna;
        }
    }
}

/**
 * Rellena la celda de la cuadricula [celda] cuya posicion viene indicada por la
 * fila [fila] y columna [columna], dependiendo de si es pista o no, y de si
 * tiene algun error.
 *  Celda vacia         - muestra candidatos con marcas negras.
 *  Valor introducido   - fondo blanco, numero negro.
 *  Pista               - fondo negro, numero blanco.
 *  Error               - muestra numero tachado con X.
 */
void Lcd_rellenar_celda(INT8U fila, INT8U columna, CELDA celda) {
    INT16U px_fila = fila * 19;
    INT16U px_columna = columna * 19;

    INT8U valor[2] = { 0, '\0' };

    valor[0] = celda >> 12;
    if (valor[0] == 0) {
        valor[0] = ' ';
    } else {
        valor[0] += 48;
    }

    INT8U es_pista = (celda & 0x0800) >> 11;
    INT8U color = BLACK;
    INT8U tiene_error;

    // sumar incremento (2=3-1) de lineas gruesas (3) con respecto a las finas (1)
    px_fila += ((fila / 3) + 1) * 2;
    px_columna += ((columna / 3) + 1) * 2;

    // sumar lineas finas
    px_fila += fila + 1;
    px_columna += columna + 1;

    // sumar margenes de la cuadricula
    px_fila += margen_V;
    px_columna += margen_H;

    if (es_pista == 1) {
        //pintar celda de negro
        LcdClrRect(px_columna, px_fila, px_columna + 19, px_fila + 19, BLACK);
        color = WHITE;
    }

    tiene_error = (celda & 0x0400) >> 10;
    if (tiene_error == 1) {

        // linea diagonal: de izq a dcha, de arriba a abajo
        Lcd_Draw_Line(px_columna, px_fila, px_columna + 19, px_fila + 19, color,
                1);

        // linea diagonal: de dcha a izq, de arriba a abajo
        Lcd_Draw_Line(px_columna + 19, px_fila, px_columna, px_fila + 19, color,
                1);

    }

    if (valor[0] == ' ') {

        rellenar_candidatos(px_fila, px_columna, celda & 0x1FF);
    } else {

        // sumar margen interior de la celda
        px_fila += 2;
        px_columna += 6;
        Lcd_DspAscII8x16(px_columna, px_fila, color, valor);
    }

    //Lcd_Dma_Trans();
}

.text
#        ENTRY                  /*  mark the first instruction to call */
.global start
start:
.arm /*indicates that we are using the ARM instruction set */
#------standard initial code
# --- Setup interrupt / exception vectors
      B       Reset_Handler
/* In this version we do not use the following handlers */
###############################################################################
#-----------Undefined_Handler:
#      B       Undefined_Handler
#----------SWI_Handler:
#      B       SWI_Handler
#----------Prefetch_Handler:
#      B       Prefetch_Handler
#----------Abort_Handler:
#      B       Abort_Handler
#         NOP      /* Reserved vector */
#----------IRQ_Handler:
#      B       IRQ_Handler
#----------FIQ_Handler:
#      B       FIQ_Handler
###############################################################################
# Reset Handler:
# the processor starts executing this code after system reset
###############################################################################
Reset_Handler:
#
        MOV     sp, #0x4000      /*  set up stack pointer (r13) */
#
#  USING A .C FUNCTION
#
# FUNCTION CALL the parameters are stored in r0 and r1
# If there are 4 or less parameters when calling a C function the compiler assumes that they have
# been stored in r0-r3. If there are more parameters you have to store them in the data stack using the stack pointer
# function __c_copy is in copy.c
        LDR     r0, =cuadricula  /*  puntero a la cuadricula */

.extern     sudoku9x9
        ldr         r5, = sudoku9x9
        mov         lr, pc
        bx          r5

stop:
        B       stop        /*  end of program */

###############################################################################
.arm
.global sudoku_recalcular_arm
sudoku_recalcular_arm:
		# saves the working registers
        # se puede modificar r0, r1, r2 y r3 sin guardarlos previamente.
        STMFD   sp!, {r4-r11, lr}

        # contador = 0
        mov r3, #0

        # i = 0
        mov r1, #0

sudoku_recalcular_arm_recorre_columna:

        # j = 0
        mov r2, #0

sudoku_recalcular_arm_recorre_fila:

		# guarda registros locales en uso antes de llamada a funcion
        STMFD   sp!, {r0-r3}

		# los parametros se corresponden con los registros locales actuales
        bl sudoku_candidatos_arm

		# comprueba resultado (0 es celda vacia)
		cmp r0, #0

		# recupera registros locales
		LDMFD   sp!, {r0-r3}

		# contador++
		addeq r3, r3, #1

		# j++
		add r2, r2, #1
		cmp r2, #9

		# j < 9
		bne sudoku_recalcular_arm_recorre_fila

		# i++
		add r1, r1, #1
		cmp r1, #9

		# i < 9
		bne sudoku_recalcular_arm_recorre_columna




		# devuelve resultado
		mov r0, r3

        # SALIR

        # restore the original registers
        LDMFD   sp!, {r4-r11, lr}

        #return to the instruccion that called the rutine and to arm mode
        bx 	lr

###############################################################################
.arm
.global sudoku_candidatos_arm
sudoku_candidatos_arm:
		# saves the working registers
        # se puede modificar r0, r1, r2 y r3 sin guardarlos previamente.
        STMFD   sp!, {r4-r11}

        # obtener @ fila (de 32 en 32 bytes)
        # r3 (calc. fila) = r1 (param. fila) * 32
        mov r3, r1, lsl#5
        add r3, r0, r3

        # obtener @ celda (@ fila + columna) (de 2 en 2 bytes)
        # r4 (calc. posicion) = r3 (calc. fila) + r2 (param. columna) * 2
        add r4, r3, r2, lsl#1

		# obtener contenido de la celda
        ldrh r5, [r4]

		# si NO es uno de los numeros fijados inicialmente (pista = 0)
		mov r6, #1
		lsl r6, #11
        ands r6, r5, r6
        bne candidatos_arm_return

        # si ya tiene un valor, no comprobar candidatos (saltar al fin)
        movs r6, r5, lsr#12
        bne candidatos_arm_return

        # iniciar candidatos
        ldr r6, =0x1FF
		orr r5, r5, r6

# ############# recorrer FILA recalculando candidatos #########################

		# contador de columna
		mov r6, #9

candidatos_arm_recorre_fila:

		# evita comprobar la celda que estamos tratando
		cmp r3, r4
		addeq r3, r3, #2
		beq candidatos_arm_recorre_fila_siguiente

		# obtener contenido de una celda de la fila
		ldrh r7, [r3], #2

		# obtener valor (4b mas significativos) [guarda flags]
		movs r7, r7, lsr#12

		# obtener desplazamiento [si valor != 0]
		subne r7, r7, #1

		# obtiene mascara para descartar candidato [si valor != 0]
		movne r8, #1
		lslne r8, r7

		# descarta candidato (pone bit a cero) [si valor != 0]
		bicne r5, r5, r8

candidatos_arm_recorre_fila_siguiente:

		# resta una columna del contador
		subs r6, r6, #1

		bne candidatos_arm_recorre_fila

# ############# recorrer COLUMNA recalculando candidatos ######################

		# obtiene @ primera celda de la columna
		add r3, r0, r2, lsl#1

		# contador de fila
		mov r6, #9

candidatos_arm_recorre_columna:

		# evita comprobar la celda que estamos tratando
		cmp r3, r4
		addeq r3, r3, #32
		beq candidatos_arm_recorre_columna_siguiente

		# obtener contenido de una celda de la columna
		ldrh r7, [r3], #32

		# obtener valor (4b mas significativos) [guarda flags]
		movs r7, r7, lsr#12

		# obtener desplazamiento [si valor != 0]
		subne r7, r7, #1

		# obtiene mascara para descartar candidato [si valor != 0]
		# movne r8, #1
		# movne r8, r8, lsl r7
		movne r8, #1
		lslne r8, r7

		# descarta candidato (pone bit a cero) [si valor != 0]
		bicne r5, r5, r8

candidatos_arm_recorre_columna_siguiente:

		# resta una columna del contador
		subs r6, r6, #1

		bne candidatos_arm_recorre_columna

# ############# recorrer REGION recalculando candidatos #######################

		# # # obtener region ########################################

		# obtener fila inicial de la region (fila / 3)*3
		mov r3, #0
candidatos_arm_recorre_region_divide_fila:
		subs r1, r1, #3
		addge r3, r3, #1
		bgt candidatos_arm_recorre_region_divide_fila

		add r1, r3, r3, lsl#1

		# obtener columna inicial de la region (columna / 3)*3
		mov r3, #0
candidatos_arm_recorre_region_divide_columna:
		subs r2, r2, #3
		addge r3, r3, #1
		bgt candidatos_arm_recorre_region_divide_columna

		add r2, r3, r3, lsl#1


		# # # calcular @ inicial de la region #######################

		# obtener @ fila (de 32 en 32 bytes)
        mov r1, r1, lsl#5
        add r1, r0, r1

        # obtener @ celda inicial (@ fila + columna) (de 2 en 2 bytes)
        add r2, r1, r2, lsl#1


		# # # recorrer region #######################################

		# i = 0
		mov r1, #0

candidatos_arm_recorre_region_columna:

		# j = 0
		mov r3, #0

candidatos_arm_recorre_region_fila:

		# evita comprobar la celda que estamos tratando
		cmp r4, r2
		addeq r2, r2, #2
		beq candidatos_arm_recorre_region_fila_siguiente

		# obtener contenido de una celda de la region
		ldrh r6, [r2], #2

		# obtener valor (4b mas significativos) [guarda flags]
		movs r6, r6, lsr#12

		# obtener desplazamiento [si valor != 0]
		subne r6, r6, #1

		# obtiene mascara para descartar candidato [si valor != 0]
		movne r7, #1
		lslne r7, r6

		# descarta candidato (pone bit a cero) [si valor != 0]
		bicne r5, r5, r7

candidatos_arm_recorre_region_fila_siguiente:

		# j++
		add r3, r3, #1

		# sigue recorriendo fila si j < 3
		cmp r3, #3
		bne candidatos_arm_recorre_region_fila

		# salta a siguiente fila de la region, teniendo en cuenta el
		# desplazamiento con respecto al punto inicial de la misma
		add r2, r2, #26     // 32 - 6 (6 = 2 bytes * 3 posiciones)
		add r1, r1, #1
		cmp r1, #3
		bne candidatos_arm_recorre_region_columna



		# guarda cambios
		strh r5, [r4]


candidatos_arm_return:

		# # # guarda cambios y devuelve resultados  ###########################

		# obtener valor de la celda
		movs r5, r5, lsr#12
		moveq r0, #0
		movne r0, #1



		# SALIR

        # restore the original registers
        LDMFD   sp!, {r4-r11}

        #return to the instruccion that called the rutine and to arm mode
        bx 	r14

###############################################################################
.arm
.global sudoku_candidatos_thumb

sudoku_candidatos_thumb:
		# saves the working registers
        # se puede modificar r0, r1, r2 y r3 sin guardarlos previamente.
        STMFD   sp!, {r4-r11, lr}

		adr r3, sudoku_candidatos_thumb_thumb + 1
		adr lr, sudoku_candidatos_thumb_vuelta
		bx r3

sudoku_candidatos_thumb_vuelta:

		# SALIR

        # restore the original registers
        LDMFD   sp!, {r4-r11, lr}

        #return to the instruccion that called the rutine and to arm mode
        bx 	lr


.thumb
sudoku_candidatos_thumb_thumb:
		# saves the working registers
        # se puede modificar r0, r1, r2 y r3 sin guardarlos previamente.
        PUSH {r4-r7}

        # obtener @ fila (de 32 en 32 bytes)
        # r3 (calc. fila) = r1 (param. fila) * 32
        mov r3, r1
        lsl r3, #5
        add r3, r0

        # obtener @ celda (@ fila + columna) (de 2 en 2 bytes)
        # r4 (calc. posicion) = r3 (calc. fila) + r2 (param. columna) * 2
        mov r4, r2
        lsl r4, #1
        add r4, r3

		# obtener contenido de la celda
        ldrh r5, [r4]

		# si es uno de los numeros fijados inicialmente (pista = 1) (saltar al fin)
		mov r6, #1
		lsl r6, #11
        and r6, r5
        cmp r6, #0
        bne candidatos_thumb_return

        # si ya tiene un valor, no comprobar candidatos (saltar al fin)
        mov r6, r5
        lsr r6, #12
        cmp r6, #0
        bne candidatos_thumb_return

        # iniciar candidatos
        ldr r6, =0x1FF
		orr r5, r6

# ############# recorrer FILA recalculando candidatos #########################

		# contador de columna
		mov r6, #9

candidatos_thumb_recorre_fila:

		# evita comprobar la celda que estamos tratando
		cmp r3, r4
		bne candidatos_thumb_recorre_fila_celda_distinta

		# si celda actual = celda investigada, no trata la celda
		add r3, #2
		beq candidatos_thumb_recorre_fila_siguiente

candidatos_thumb_recorre_fila_celda_distinta:

		# obtener contenido de una celda de la fila
		ldrh r7, [r3]
		add r3, #2

		# obtener valor (4b mas significativos) [guarda flags]
		lsr r7, #12

		# si valor = 0, no trata la celda
		cmp r7, #0
		beq candidatos_thumb_recorre_fila_siguiente

		# obtener desplazamiento [si valor != 0]
		sub r7, #1

		# guardar r0 (en la pila) para despues
		PUSH {r0}

		# obtiene mascara para descartar candidato [si valor != 0]
		mov r0, #1
		lsl r0, r7

		# descarta candidato (pone bit a cero) [si valor != 0]
		bic r5, r0

		# recupera r0 (de la pila)
		POP {r0}

candidatos_thumb_recorre_fila_siguiente:

		# resta una columna del contador
		sub r6, #1

		# si contador > 0, siguiente fila
		cmp r6, #0
		bne candidatos_thumb_recorre_fila

# ############# recorrer COLUMNA recalculando candidatos ######################

		# obtiene @ primera celda de la columna
		mov r3, r2
		lsl r3, #1
		add r3, r0

		# contador de fila
		mov r6, #9

candidatos_thumb_recorre_columna:

		# evita comprobar la celda que estamos tratando
		cmp r3, r4
		bne candidatos_thumb_recorre_columna_celda_distinta

		add r3, #32
		beq candidatos_thumb_recorre_columna_siguiente

candidatos_thumb_recorre_columna_celda_distinta:

		# obtener contenido de una celda de la columna
		ldrh r7, [r3]
		add r3, #32

		# obtener valor (4b mas significativos) [guarda flags]
		lsr r7, #12
		cmp r7, #0
		beq candidatos_thumb_recorre_columna_siguiente

		# obtener desplazamiento [si valor != 0]
		sub r7, #1

		# guardar r0 (en la pila) para despues
		PUSH {r0}

		# obtiene mascara para descartar candidato [si valor != 0]
		# movne r0, #1
		# movne r0, r0, lsl r7
		mov r0, #1
		lsl r0, r7

		# descarta candidato (pone bit a cero) [si valor != 0]
		bic r5, r0

		# recupera r0 (de la pila)
		POP {r0}

candidatos_thumb_recorre_columna_siguiente:

		# resta una columna del contador
		sub r6, #1
		cmp r6, #0

		bne candidatos_thumb_recorre_columna

# ############# recorrer REGION recalculando candidatos #######################

		# # # obtener region ########################################

		# obtener fila inicial de la region (fila / 3)*3
		mov r3, #0
candidatos_thumb_recorre_region_divide_fila:
		sub r1, #3
		cmp r1, #0
		blt candidatos_thumb_recorre_region_divide_fila_evitar_suma

		add r3, #1

candidatos_thumb_recorre_region_divide_fila_evitar_suma:

		bgt candidatos_thumb_recorre_region_divide_fila

		lsl r3, #1
		add r1, r3

		# obtener columna inicial de la region (columna / 3)*3
		mov r3, #0
candidatos_thumb_recorre_region_divide_columna:
		sub r2, #3
		cmp r2, #0
		blt candidatos_thumb_recorre_region_divide_columna_evitar_suma

		add r3, #1

candidatos_thumb_recorre_region_divide_columna_evitar_suma:

		bgt candidatos_thumb_recorre_region_divide_columna

		lsl r3, #1
		add r2, r3


		# # # calcular @ inicial de la region #######################

		# obtener @ fila (de 32 en 32 bytes)
        lsl r1, #5
        add r1, r0

        # obtener @ celda inicial (@ fila + columna) (de 2 en 2 bytes)
        lsl r2, #1
        add r2, r1


		# # # recorrer region #######################################

		# i = 0
		mov r1, #0

candidatos_thumb_recorre_region_columna:

		# j = 0
		mov r3, #0

candidatos_thumb_recorre_region_fila:

		# evita comprobar la celda que estamos tratando
		cmp r4, r2
		bne candidatos_thumb_recorre_region_fila_celda_distinta
		add r2, #2
		b candidatos_thumb_recorre_region_fila_siguiente

candidatos_thumb_recorre_region_fila_celda_distinta:

		# obtener contenido de una celda de la region
		ldrh r6, [r2]
		add r2, #2

		# obtener valor (4b mas significativos) [guarda flags]
		lsr r6, #12
		cmp r6, #0
		beq candidatos_thumb_recorre_region_fila_siguiente

		# obtener desplazamiento [si valor != 0]
		sub r6, #1

		# obtiene mascara para descartar candidato [si valor != 0]
		mov r7, #1
		lsl r7, r6

		# descarta candidato (pone bit a cero) [si valor != 0]
		bic r5, r7

candidatos_thumb_recorre_region_fila_siguiente:

		# j++
		add r3, #1

		# sigue recorriendo fila si j < 3
		cmp r3, #3
		bne candidatos_thumb_recorre_region_fila

		# salta a siguiente fila de la region, teniendo en cuenta el
		# desplazamiento con respecto al punto inicial de la misma
		add r2, #26     // 32 - 6 (6 = 2 bytes * 3 posiciones)
		add r1, #1
		cmp r1, #3
		bne candidatos_thumb_recorre_region_columna



		# guarda cambios
		strh r5, [r4]


candidatos_thumb_return:

		# # # guarda cambios y devuelve resultados  ###########################

		# obtener valor de la celda
		lsr r5, #12
		cmp r5, #0

		bne candidatos_thumb_return_true

		mov r0, #0

		b candidatos_thumb_return_fin

candidatos_thumb_return_true:

		mov r0, #1

candidatos_thumb_return_fin:



		# SALIR

        # restore the original registers
        POP {r4-r7}

        #return to the instruccion that called the rutine and to arm mode
        bx 	lr

###############################################################################


.data
.ltorg /*guarantees the alignment*/
.align 5

# huecos para cuadrar
cuadricula:
     /* 9 filas de 16 entradas para facilitar la visualizacion y 16 bits por celda */
    .hword   0x9800,0x6800,0x0000,0x0000,0x0000,0x0000,0x7800,0x0000,0x8800,0,0,0,0,0,0,0
    .hword   0x8800,0x0000,0x0000,0x0000,0x0000,0x4800,0x3800,0x0000,0x0000,0,0,0,0,0,0,0
    .hword   0x1800,0x0000,0x0000,0x5800,0x0000,0x0000,0x0000,0x0000,0x0000,0,0,0,0,0,0,0
    .hword   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1800,0x7800,0x6800,0,0,0,0,0,0,0
    .hword   0x2800,0x0000,0x0000,0x0000,0x9800,0x3800,0x0000,0x0000,0x5800,0,0,0,0,0,0,0
    .hword   0x7800,0x0000,0x8800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0,0,0,0,0,0,0
    .hword   0x0000,0x0000,0x7800,0x0000,0x3800,0x2800,0x0000,0x4800,0x0000,0,0,0,0,0,0,0
    .hword   0x3800,0x8800,0x2800,0x1800,0x0000,0x5800,0x6800,0x0000,0x0000,0,0,0,0,0,0,0
    .hword   0x0000,0x4800,0x1800,0x0000,0x0000,0x9800,0x5800,0x2800,0x0000,0,0,0,0,0,0,0

.end
#        END

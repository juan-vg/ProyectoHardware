
.arm
.global sudoku_recalcular_arm

sudoku_recalcular_arm:
        # saves the working registers
        # se puede modificar r0, r1, r2 y r3 sin guardarlos previamente.
        mov ip, sp
        STMDB   sp!, {r4-r10, fp, ip, lr, pc}

        # reserva espacio para 4 variables locales
        sub sp, sp, #16

        # fp apuntando a @ pc (la pila es FullDesc. -> Decr. Before -> la @ es ip - 4 )
        sub fp, ip, #4

        # guardar parametro opcion
        mov r4, r1

        # contador = 0
        mov r3, #0

        # i = 0
        mov r1, #0

sudoku_recalcular_arm_recorre_columna:

        # j = 0
        mov r2, #0

sudoku_recalcular_arm_recorre_fila:

        # guarda variables locales en uso, en el marco de pila, antes de llamada a funcion
        sub r5, fp, #40
        STMDB r5, {r0-r3}

        # comprueba el parametro opcion
        cmp r4, #2

        # los parametros se corresponden con los registros locales actuales
        beq sudoku_recalcular_arm_arm
        bgt sudoku_recalcular_arm_thumb

        bl sudoku_candidatos_c
        b sudoku_recalcular_arm_recorre_fila_sigue

sudoku_recalcular_arm_arm:

        bl sudoku_candidatos_arm
        b sudoku_recalcular_arm_recorre_fila_sigue

sudoku_recalcular_arm_thumb:

        bl sudoku_candidatos_thumb



sudoku_recalcular_arm_recorre_fila_sigue:

        # comprueba resultado (0 es celda vacia)
        cmp r0, #0

        # recupera variables locales
        LDMDB r5, {r0-r3}

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
        LDMDB   fp, {r4-r10, fp, sp, lr}

        #return to the instruccion that called the rutine and to arm mode
        bx     lr

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

        # r1 = r3 * 3
        add r1, r3, r3, lsl#1

        # obtener columna inicial de la region (columna / 3)*3
        mov r3, #0
candidatos_arm_recorre_region_divide_columna:
        subs r2, r2, #3
        addge r3, r3, #1
        bgt candidatos_arm_recorre_region_divide_columna

        # r2 = r3 * 3
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
        bx     lr



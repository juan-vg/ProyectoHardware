.arm
.global sudoku_candidatos_thumb

# Dada una determinada celda de la cuadricula encuentra los posibles valores
# candidatos. Devuelve false (0) si la celda esta vacia o true (1) si contiene
# un valor
# La funcion comienza con los parametros en los registros:
#	r0 - @cuadricula
# 	r1 - fila
#	r2 - columna
#
# Para ejecutar la funcion thumb:
#	r3 - @ inicio funcion thumb
#
# Al comenzar la ejecucion se utilizan los registros de la siguiente manera:
#	r0 - @cuadricula
# 	r1 - fila
#	r2 - columna
# 	r3 - iterador @fila / @columna
#	r4 - @celda
#	r5 - contenido celda
#	r6 - comprobacion de pista, valor
#
# Ademas, tanto cuando se recorre la fila como la columna se utilizan:
#	r6 - contador de fila / columna
#	r7 - contenido de la celda investigada, valor
# 	r0 - mascara utilizada para descartar candidatos
#
# Debido a la falta de registros es necesario guardar r0 en la pila
# momentaneamente. Una vez realizada la operacion con la mascara, se recupera el
# registro.
#
# A la hora de recorrer la region correspondiente se modifican:
#	r1 - contador fila i
#	r2 - iterador @celda
#	r3 - contador columna j
#	r6 - contenido celda investigada, valor
#	r7 - mascara utilizada para descartar candidatos
#
# La funcion termina con los siguientes resultados
# 	r0 - resultado (0 si celda vacia)

sudoku_candidatos_thumb:
        # crea marco de pila
        # se puede modificar r0, r1, r2 y r3 sin guardarlos previamente.
        mov ip, sp
        STMFD   sp!, {r4-r10, fp, ip, lr, pc}

        # @ de pc (la pila es FullDesc. -> Decr. Before -> la @ es ip - 4 )
        sub fp, ip, #4

        adr r3, sudoku_candidatos_thumb_thumb + 1
        adr lr, sudoku_candidatos_thumb_vuelta
        bx r3

sudoku_candidatos_thumb_vuelta:

        # SALIR

        # recupera contexto anterior a partir del marco de pila
        LDMDB   fp, {r4-r10, fp, sp, lr}

        #return to the instruccion that called the rutine and to previous mode
        bx     lr


.thumb
sudoku_candidatos_thumb_thumb:
        # crea marco de pila
        # se puede modificar r0, r1, r2 y r3 sin guardarlos previamente.
        PUSH {r4-r7}

        // TODO: Optimizar la obtencion de la @ de la celda (MLA) ??????
        // -> contar ciclos y determinar si procede !!!!

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

        # borrar bit de error
        mov r6, #1
        lsl r6, #10
        bic r5, r6

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
        b candidatos_thumb_recorre_fila_siguiente

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
        b candidatos_thumb_recorre_columna_siguiente

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

        // TODO: Optimizar la obtencion del punto inicial de la region (tabla)

        # obtener fila inicial de la region (fila / 3)*3
        mov r3, #0
candidatos_thumb_recorre_region_divide_fila:
        sub r1, #3
        cmp r1, #0
        blt candidatos_thumb_recorre_region_divide_fila_evitar_suma

        add r3, #1

candidatos_thumb_recorre_region_divide_fila_evitar_suma:
        cmp r1, #0
        bgt candidatos_thumb_recorre_region_divide_fila

        # mul * 3
        mov r1, r3
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

        cmp r2, #0
        bgt candidatos_thumb_recorre_region_divide_columna

        # mul * 3
        mov r2, r3
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


candidatos_thumb_return:

        # # # guarda cambios y devuelve resultados  ###########################
        # celda vacia             -> r0 =  0
        # celda llena (SIN error) -> r0 =  1
        # celda llena (CON error) -> r0 = -1

        # obtener valor de la celda
        lsr r5, #12
        cmp r5, #0

		# si valor = 0
        # almacenar valor de resultado y terminar
        beq candidatos_thumb_return_vacia

        # obtener mascara del valor
        sub r1, #1
        mov r2, #1
        lsl r2, r1

        # comprobar si esta entre los candidatos
        # (si el valor no esta entre los candidatos -> flag Z=1)
        and r2, r5
        cmp r2, #0

        beq candidatos_thumb_return_llena_error
        bne candidatos_thumb_return_llena

candidatos_thumb_return_llena:

        mov r0, #1
        b candidatos_thumb_return_fin

candidatos_thumb_return_llena_error:

		# poner bit de error
        mov r6, #1
        lsl r6, #10
        orr r5, r6

        mov r0, #1
        mvn r0, r0
        b candidatos_thumb_return_fin

candidatos_thumb_return_vacia:

        mov r0, #0


candidatos_thumb_return_fin:

        # guarda cambios
        strh r5, [r4]

        # SALIR

        # recupera contexto anterior
        POP {r4-r7}

        #return to the instruccion that called the rutine and to previous mode
        bx     lr

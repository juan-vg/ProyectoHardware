
    .extern main
    .equ STACK, 0x0c7ff000
.text
.equ	num, 10  /* this is used to indicate the number of words to copy */
#        ENTRY            		/*  mark the first instruction to call */
.arm /*indicates that we are using the ARM instruction set */
#------standard initial code
# --- Setup interrupt / exception vectors
.global	start
start:
      B       Reset_Handler
/* In this version we do not use the following handlers */
#######################################################################################################
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
#######################################################################################################
# Reset Handler:
# the processor starts executing this code after system reset
#######################################################################################################
Reset_Handler:
#
        ldr		 sp,=STACK      /*  set up stack pointer (r13) */


#bucle 'link_arm' realizado para medir el tiempo de 1000 iteraciones
        mov r7, #1000

link_arm:

# colocamos los parámetros para la llamada a la primera función
        LDR     r0, =scr        /*  r0 = pointer to source block */
 # guardamos r0 en r4 porque al llamar a las funciones podemos perder su valor. Así lo podremos restaurar sin acceder a memoria
        MOV 	r4, r0
        LDR     r1, =dst1       /*  r1 = pointer to destination block 1 */

#
# PART 1: USING ARM CODE
#

       	BL		ARM_copy_10			/*FUNCTION CALL*/

       	subs r7, r7, #1
       	bne		link_arm


#bucle 'link_thumb' realizado para medir el tiempo de 1000 iteraciones
        mov r7, #1000

link_thumb:
#
# PART 2: USING THUMB CODE
#
# colocamos los parámetros para la llamada a la segunda función porque ARM_copy_10 los ha podido eliminar
# IMPORTANTE: si queremos tener un código modular debemos volver a poner los parámetros a pesar de que en teoría el 1 y el 2
        MOV     r0, r4       /*  r0 = pointer to source block */
        LDR     r1, =dst2       /*  r1 = pointer to destination block 1 */
     	ADR		r3, th_copy_10+1 /* the last address bit is not really used to specify the address but to select between ARM and Thumb code */
		adr		r14,return		/* we store the return address in r14*/
		BX		r3				/* FUNCTION CALL, we jump to th_mul. +1 indicates that we want to switch to thumb */

		subs r7, r7, #1
        bne     link_thumb


return:

#bucle 'link_c' realizado para medir el tiempo de 1000 iteraciones
        mov r7, #1000

link_c:

#
# PART 3: USING A .C FUNCTION
#
# ponemos los parámetros de nuevo
		MOV     r0, r4        /*  r0 = pointer to source block */
		LDR     r1, =dst3        /*  r1 = pointer to destination block 1 */
# function __c_copy is in copy.c
.extern     __c_copy_10
        ldr         r3, = __c_copy_10
        mov         lr, pc
# FUNCTION CALL the parameters are stored in r0 and r1
# If there are 4 or less parameters when calling a C function the compiler assumes that they have
# been stored in r0-r3. If there are more parameters you have to store them in the data stack using the stack pointer
		bx          r3

#no se sale hasta realizar las 1000 iteraciones
		subs r7, r7, #1
        bne     link_c


stop:
 		B     	stop    	/*  end of program */
#######################################################################################################
# Función ARM: copia 10 palabras de la dirección indicada en r0 a la indicada por r1
# debería crear un marco de pila con la misma estructura que en el resto de llamadas a funciones
# pero como es un nodo hoja (no llama a ninguna función) vamos a hacer un marco simplificado: sólo guardamos los
# registros que utiliza y que no tiene permiso para alterar.

#######################################################################################################
ARM_copy_10:
        #  saves the working registers
        # Recordad que puede modificar r0, r1, r2 y r3 sin guardarlos previamente.
        STMFD   sp!, {r4-r11}

		# Poned el código aquí: sólo hacen falta dos instrucciones
		LDMIA r0, {r2-r11}
		STMIA r1, {r2-r11}

		# restore the original registers
        LDMFD   sp!, {r4-r11}
        #return to the instruccion that called the rutine and to arm mode
        bx 	r14
#######################################################################################################
# Función thumb: copia 10 palabras de la dirección indicada en r0 a la indicada por r1
# De nuevo, al ser un nodo hoja hacemos un marco simplificado: sólo los registros que toca y debe restaurar
#######################################################################################################
/*indicates that we are using the thumb instruction set */
.thumb
th_copy_10:
		push	{r4-r7}

		# poned aquí el código, como no podemos leer y escribir 10 palabras de golpe lo haremos en dos veces.
		LDMIA r0!, {r2-r6}
		STMIA r1!, {r2-r6}
		# hay que hacer wb(!) posiblemente por no poder acceder a todos los registros,
		# impidiendo que se mantenga la @ virtualmente en alguno.
		LDMIA r0!, {r2-r6}
		STMIA r1!, {r2-r6}

		#restaurar
		pop		{r4-r7} /*restores the registers*/
	    bx		r14	/* this is the return instrucction*/

#######################################################################################################
#        AREA BlockData, DATA, READWRITE
#	scr: source block (10 words)
#	dst1: destination block 1 (10 words)
#	dst2: destination block 2 (10 words)
#	dst3: destination block 3 (10 words)
#######################################################################################################
.data
.ltorg /*guarantees the alignment*/
scr:
     .long     1,2,3,4,5,6,7,8,9,10
dst1:
     .long     0,0,0,0,0,0,0,0,0,0
dst2:
     .long     0,0,0,0,0,0,0,0,0,0
dst3:
	 .long	   0,0,0,0,0,0,0,0,0,0
.end
#        END



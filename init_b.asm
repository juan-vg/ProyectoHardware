.text
#        ENTRY                  /*  mark the first instruction to call */
.global start
start:
.arm /*indicates that we are using the ARM instruction set */
#------standard initial code
# --- Setup interrupt / exception vectors
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

#################################################################################################################
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

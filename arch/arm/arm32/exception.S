/*
 * Exception handling
 */

#include <xtf/asm_macros.h>

.macro exception_entry name
.align 8
        b       entry_\name
.endm

.globl vector_table
vector_table:
        exception_entry reset
        exception_entry undef_instr
        exception_entry svc
        exception_entry prefetch_abort
        exception_entry data_abort
        exception_entry unused
        exception_entry irq
        exception_entry fiq

#include <arch/arm32/processor.h>

.macro exception_handler sym val
ENTRY(entry_\sym)
        stmfd   sp!,{r0-r12,lr}

        mov     r12, #\val
        stmfd   sp!,{r12}
        ldr     r12, [sp, #8]

        bl      do_exception

        ldmfd   sp!,{r12}
        /* ^ qualifier: CPSR is restored from SPSR */
        ldmfd   sp!,{r0-r12,pc}^
ENDFUNC(entry_\sym)
.endm

exception_handler reset           ARM32_VECTOR_RESET
exception_handler undef_instr     ARM32_VECTOR_UNDEF_INSTR
exception_handler svc             ARM32_VECTOR_SVC
exception_handler prefetch_abort  ARM32_VECTOR_PREFETCH_ABORT
exception_handler data_abort      ARM32_VECTOR_DATA_ABORT
exception_handler unused          ARM32_VECTOR_UNUSED
exception_handler irq             ARM32_VECTOR_IRQ
exception_handler fiq             ARM32_VECTOR_FIQ

ENTRY(init_exception_vector)
        /* initialize exception handling: Vector Base Address Register */
        ldr     r8, =vector_table
        mcr     15, 0, r8, c12, c0
        mov     pc, lr
ENDFUNC(init_exception_vector)

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */

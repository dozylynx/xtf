/**
 * @file arch/arm/include/arch/arm32/processor.h
 *
 * arm32 CPU definitions
 */
#ifndef XTF_ARM32_PROCESSOR_H
#define XTF_ARM32_PROCESSOR_H

#define ARM32_VECTOR_RESET          0
#define ARM32_VECTOR_UNDEF_INSTR    1
#define ARM32_VECTOR_SVC            2
#define ARM32_VECTOR_PREFETCH_ABORT 3
#define ARM32_VECTOR_DATA_ABORT     4
#define ARM32_VECTOR_UNUSED         5
#define ARM32_VECTOR_IRQ            6
#define ARM32_VECTOR_FIQ            7

#endif /* XTF_ARM32_PROCESSOR_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */

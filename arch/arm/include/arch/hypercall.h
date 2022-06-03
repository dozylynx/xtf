/**
 * @file arch/arm/include/arch/hypercall.h
 *
 * Hypercall primitives for arm.
 */
#ifndef XTF_ARM_HYPERCALL_H
#define XTF_ARM_HYPERCALL_H

#include <xtf/lib.h>

#define _hypercall_1(type, hcall, A0)                   \
    ({                                                         \
    register unsigned long nr asm ("r12") = hcall;             \
    register unsigned long a0 asm ("r0") = (unsigned long) A0; \
                                                               \
    asm volatile ("hvc #0xea1"                                 \
                  : [nr] "+r" (nr), [a0] "+r" (a0)             \
                  :                                            \
                  : "memory");                                 \
        (type)a0;                                              \
    })

#define _hypercall_2(type, hcall, A0, A1)               \
    ({                                                         \
    register unsigned long nr asm ("r12") = hcall;             \
    register unsigned long a0 asm ("r0") = (unsigned long) A0; \
    register unsigned long a1 asm ("r1") = (unsigned long) A1; \
                                                               \
    asm volatile ("hvc #0xea1"                                 \
                  : [nr] "+r" (nr), [a0] "+r" (a0),            \
                    [a1] "+r" (a1)                             \
                  :                                            \
                  : "memory");                                 \
        (type)a0;                                              \
    })

#define _hypercall_3(type, hcall, A0, A1, A2)                  \
    ({                                                         \
    register unsigned long nr asm ("r12") = hcall;             \
    register unsigned long a0 asm ("r0") = (unsigned long) A0; \
    register unsigned long a1 asm ("r1") = (unsigned long) A1; \
    register unsigned long a2 asm ("r2") = (unsigned long) A2; \
                                                               \
    asm volatile ("hvc #0xea1"                                 \
                  : [nr] "+r" (nr), [a0] "+r" (a0),            \
                    [a1] "+r" (a1), [a2] "+r" (a2)             \
                  :                                            \
                  : "memory");                                 \
        (type)a0;                                              \
    })

#define _hypercall_5(type, hcall, A0, A1, A2, A3, A4)          \
    ({                                                         \
    register unsigned long nr asm ("r12") = hcall;             \
    register unsigned long a0 asm ("r0") = (unsigned long) A0; \
    register unsigned long a1 asm ("r1") = (unsigned long) A1; \
    register unsigned long a2 asm ("r2") = (unsigned long) A2; \
    register unsigned long a3 asm ("r3") = (unsigned long) A3; \
    register unsigned long a4 asm ("r4") = (unsigned long) A4; \
                                                               \
    asm volatile ("hvc #0xea1"                                 \
                  : [nr] "+r" (nr), [a0] "+r" (a0),            \
                    [a1] "+r" (a1), [a2] "+r" (a2),            \
                    [a3] "+r" (a3), [a4] "+r" (a4)             \
                  :                                            \
                  : "memory");                                 \
        (type)a0;                                              \
    })

#endif /* XTF_ARM_HYPERCALL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */

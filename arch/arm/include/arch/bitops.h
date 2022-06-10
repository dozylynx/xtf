/**
 * @file arch/arm/include/arch/bitops.h
 *
 * Low level bit operations.
 */
#ifndef XTF_ARM_BITOPS_H
#define XTF_ARM_BITOPS_H

#include <xtf/lib.h>

static inline bool test_bit(unsigned int bit, const void *addr)
{
    UNIMPLEMENTED();
    return false;
}

static inline bool test_and_set_bit(unsigned int bit, volatile void *addr)
{
    UNIMPLEMENTED();
    return false;
}

static inline bool test_and_change_bit(unsigned int bit, volatile void *addr)
{
    UNIMPLEMENTED();
    return false;
}

static inline bool test_and_clear_bit(unsigned int bit, volatile void *addr)
{
    register unsigned long val, oldval, ok;
    register unsigned long mask = (1 << bit);
    register unsigned long *p = (unsigned long *)addr;

    asm volatile (
        "1:\n"
        "dmb ish\n"
        "ldrex %1, [%4]\n"
        "bic %0, %1, %5\n"
        "strex %3, %0, [%4]\n"
        "cmp %3, #0\n"
        "bne 1b\n"
        "dmb ish\n"
        : "=&r" (val), "=&r" (oldval), "+m" (*p), "=&r" (ok)
        : "r" (p), "r" (mask)
        : "memory");

    return oldval & mask;
}

#endif /* XTF_ARM_BITOPS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
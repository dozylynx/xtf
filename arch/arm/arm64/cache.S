#include <xtf/asm_macros.h>

/*
 * flush_dcache_range(start, end)
 * - start   - start address of a region
 * - end     - end address of a region
 */
ENTRY(flush_dcache_range)
    /* Get the minimum D-cache line size */
    mrs    x3, ctr_el0
    ubfm   x3, x3, #16, #19
    mov    x2, #4
    lsl    x2, x2, x3
    sub    x3, x2, #1
    bic    x0, x0, x3
    /* Clean and invalidate D-cache line */
1:  dc     civac, x0
    add    x0, x0, x2
    cmp    x0, x1
    b.lo   1b
    dsb    sy
    ret
ENDFUNC(flush_dcache_range)

/*
 * Paging
 */
        .section ".text", "ax", %progbits
        .code 32

#include <xtf/asm_macros.h>

ENTRY(init_paging)
        /* TODO: just return here, as this is not fully implemented yet */
        mov     pc, lr

        /* TODO: populate page tables */
        /*-----------------------------------------------------------*/

        ldr r1, =page_table_base

        /*-----------------------------------------------------------*/
        /* TTBR0, Translation Table Base Register 0 */
        /* Manual: G8.2.166 */
        /* When TTBCR.EAE == 1, 64 bits active */

        /* bits  | val  | descr */
        /* 56-63 |  0   | Reserved                                    */
        /* 48-55 |  0   | ASID                                        */
        /* 32-1  | addr | Base address                                */
        /*    0  |  0   | Common not Private                          */

        ldr r1, =page_table_base
        /* Write r1 to TTBR0 */
        mcr p15, 0, r1, c2, c0, 0; 

        /*-----------------------------------------------------------*/
        /* Translation Table Base Control Register */
        /* Manual: G8.2.164 : TTBCR */
        /* Manual: G5.5 */

        /* bits   val  descr */
        /*    31 |  1 | Extended Address Enable: set to '1' for         */
        /*       |    | Long-descriptor format tables                   */
        /*    30 |  0 | Implementation defined                          */
        /* 28-29 | 00 | SH1: shareable for TTBR1                        */
        /* 26-27 | 00 | ORGN1:                                          */
        /* 24-25 | 00 | IGRN1:                                          */
        /*    23 |  1 | EPD1: Disable TTBR1 tables                      */
        /*    22 |  0 | A1: selects TTBR0/1 defines ASID - 0 for TTBR0  */
        /* 19-21 |  0 | Reserved                                        */
        /* 16-18 |  0 | T1SZ: TTBR1 memory region sizes                 */
        /* 14-15 |  0 | Reserved                                        */
        /* 12-13 | 11 | SH0: shareable for TTBR0: 11 == inner shareable */
        /* 10-11 | 01 | ORGN0: Outer Write-back, Allocate Cacheable     */
        /*  8-9  | 01 | IGRN0: Inner Write-back, Allocate Cacheable     */
        /*    7  |  0 | EPD0: Disable TTBR0 tables                      */
        /*    6  |  0 | T2E: Enable TTBCR2                              */
        /*  3-5  |  0 | Reserved                                        */
        /*  0-2  |  0 | T0SZ: TTBR0 memory region sizes                 */

        /* set EAE (bit 31) and EPD1 (bit 23) */
        mov r1, #0x80800000
        /* set SH0, ORGN0, IGRN0 */
        orr r1, r1, #0x3500

        /* Write r1 to TTBCR */
        mcr p15, 0, r1, c2, c0, 2; 

        /* Done */
        mov     pc, lr
ENDFUNC(init_paging)

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */

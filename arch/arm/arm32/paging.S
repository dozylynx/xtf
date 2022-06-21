/*
 * Paging
 */
        .section ".text", "ax", %progbits
        .code 32

#include <xtf/asm_macros.h>
#include <arch/page.h>
#include <arch/arm32/mm.h>

/* In Arm documentation terminology, the L1 page table is the       */
/* parent table that can contain entries describing L2 page tables, */
/* and entries in L2 either describe individual pages or L3 tables. */

/* TODO: describe registers clobbered here */
ENTRY(setup_identity_mapping)
        /* Manual: G5.4 */
        /* Short format supports mappings based on: */
        /* - Supersection 16MB blocks */
        /* - Section 1MB blocks */
        /* - Large pages, 64K */
        /* - Small pages, 4K */
        /* Since the identity map is only needed for the transition */
        /* while enabling paging, use a 1M block so that only one level */
        /* of page table is required. */

        ldr r0, =identmap
        add r0, r0, r9
        ldr r1, =.text
        add r1, r1, r9

        /* Using TTBR0 with TTBCR.N set to: 0 */
        /* In TTBR0, the base address field is bits[31:14-N].*
        /* PDE indexed by VA[31-N:20] */
        /* so 12 bits: [31:20] (4096 entries in the table) */

        /* 1M Section descriptor format: Manual G5.4.2              */
        /* [31:20] |     | Section base address, PA[31:20]          */
        /*   19    |  1  | NS:Non-Secure                            */
        /*   18    |  0  | Section descriptor                       */
        /*   17    |  0  | nG:not Global                            */
        /*   16    |  1  | S:'Shareable'                            */
        /*   15    |  0  | Access Permission[2]: 0 to allow writes  */
        /* [14:12] | 001 | 'Type EXtension': Memory region attrs    */
        /* [11:10] |  10 | Access Permission[1:0]                   */
        /*   9     |  0  | Implementation defined                   */
        /* [ 8:5 ] |  0  | Domain                                   */
        /*   4     |  0  | eXecute Never                            */
        /*   3     |  1  | C:'Cacheable'                            */
        /*   2     |  1  | B:'Bufferable'                           */
        /*   1     |  1  | Section descriptor                       */
        /*   0     |  0  | Privileged eXecute Never                 */

        /* Memory: choose: Normal, Outer+Inner Write back, allocate */

        /* To write the 1M entry in the page directory, determine   */
        /* the 1MB block containing the virtual addr of this code.  */
        /* 1MB => lowest 20 bits all zero [19:0]                    */
        ldr r2, =ARM32_L1_TABLE_INDEX_MASK
        and r2, r2, r1, lsr #ARM32_L1_TABLE_SHIFT
        /* now r2 is the index of the entry in the identity table   */

        /* create the identity pte:                                 */
        /* - clear the lower 20 bits of the phys addr               */
        /*      - use a logical rotate right and then left to do it */
        /* - populate the lower 20 bits with section flag bits      */
        lsr r4, r1, #ARM32_L1_TABLE_SHIFT
        ldr r3, =ARM32_L1_TABLE_BLOCK
        orr r4, r3, r4, lsl #ARM32_L1_TABLE_SHIFT

        /* write the identity pagetable entry into the map: */
        str r4, [r0, r2, lsl #2]

        mov pc, lr
ENDFUNC(setup_identity_mapping)

/*------------------------------------------*/
/* TODO: describe registers clobbered here */

ENTRY(setup_page_tables)
        /*------------------------------------------*/
        ldr r1, =l1_page_table
        ldr r2, =l2_page_table
        ldr r3, =_start
        ldr r4, =_end

        /*------------------------------------------*/
        /* Build the page directory entry:                       */

        /* set r6 to physical address of the L2 table, using the */
        /* virtual address in r2 and the physical offset in r9.  */
        add r6, r2, r9
        /* use shifts to mask bits. set flags. */
        lsr r6, r6, #ARM32_L1_TABLE_SHIFT
        ldr r7, =ARM32_L1_TABLE_ENTRY
        orr r6, r7, r6, lsl #ARM32_L1_TABLE_SHIFT

        /* Index of L1 entry for L2 is derived from vaddr of _start (r3) */
        /* Indexed by: VA[31:20] */
        ldr r5, =ARM32_L1_TABLE_INDEX_MASK
        and r5, r5, r3, lsr #ARM32_L1_TABLE_SHIFT

        /* Convert r1 to physical from virtual address */
        /* phys offset is in r9 */
        add r1, r1, r9

        /* Write the PDE: */
        /* r1: physical base address of the page directory table */
        /* r5: index into the PDE page table, calculated from L2 vaddr */
        /*     shift left 2 to multiply by 4, for 4 byte (short) PTEs */
        /* r6: Page Directory Entry to write */
        str r6, [r1, r5, lsl #2]

        /*------------------------------------------*/
        /* Convert r2 to physical from virtual address */
        add r2, r2, r9
1:
        /* Build a PTE: */
        /* r3 is the virtual address of the page to map. */
        /* set r6 to the physical address of the page to be mapped, */
        /* using the virtual address in r3 and the phys offset in r9. */
        add r6, r3, r9
        /* use shifts to mask off the bottom bits and populate flags */
        lsr r6, r6, #PAGE_SHIFT
        ldr r7, =ARM32_L2_TABLE_SMALL_ENTRY
        orr r6, r7, r6, lsl #PAGE_SHIFT

        /* Calculate the offset of the entry in the table from vaddr */
        /* Indexed by: VA[19:12] */
        ldr r5, =ARM32_L2_TABLE_INDEX_MASK
        and r5, r5, r3, lsr #PAGE_SHIFT

        /* Write the PTE: */
        /* r2: physical base address of the page table */
        /* r5: index into the page table, calculated from page vaddr */
        /*     shift left 2 to multiply by 4, for 4 byte (short) PTEs */
        /* r6: Page Table Entry to write */
        str r6, [r2, r5, lsl #2]

        add r3, r3, #PAGE_SIZE
        cmp r3, r4
        ble 1b

        mov     pc, lr
ENDFUNC(setup_page_tables)

/* TODO: describe registers clobbered here */
ENTRY(init_paging)

        /* TODO: just return here as this is not fully implemented */
        mov     pc, lr

        /*-----------------------------------------------------------*/
        # SCTLR:
        # When using the short format descriptors:

        # SCTLR.TRE governs bits in the descriptor.
        # => clear to zero
        # 0: remap disabled, bits in the descriptor govern access.
        # 1: remap enabled, bits are indexes to the remap registers.

        # SCTLR.AFE selects the access permissions option[8876]
        # => set to 1
        # 1: (non-deprecated): AP[0] as flag, AP[2:1] permissions
        #   - AP[2]: disable write (0 to enable writes)
        #   - AP[1]: enable unpriv access (1 to enable unpriv)
        # 0: (deprecated): AP[2:0] permissions

        # read SCTLR
        mrc p15, 0, r2, c1, c0, 0;
        # TODO: define a macro for bit 28 as TEX Remap Enable
        bic r2, r2, #28
        # TODO: define a macro for bit 29 as Access Flag Enable
        orr r2, r2, #29
        # write SCTLR
        mcr p15, 0, r2, c1, c0, 0;

        bl setup_identity_mapping
        bl setup_page_tables

        /*-----------------------------------------------------------*/
        /* TTBR0, Translation Table Base Register 0 */
        /* Manual: G8.2.166 */
        /* When TTBCR.EAE == 0, only 32 lower bits active */

        /* bits */
        /* 31-7 | val | Translation Table Base address 0 */
        /*    6 |  1  | Inner RGN[0] */
        /*    5 |  0  | Not Outer Shareable */
        /*  4-3 | 01  | (Outer) RGN */
        /*    2 |  0  | Implementation-defined feature bit */
        /*    1 |  1  | S: memory for translation tables is Shareable */
        /*    0 |  0  | IRGN[1] */

        /* IRGN: choose: Inner Write-Back Write-Allocate Cacheable */
        /* IRGN = 0b01, so (low) bit6 = 1, (high) bit0 = 0 */
        /* RGN: choose: Outer Write-Back Write-Allocate Cacheable */
        /* RGN = 0b01 */

        ldr r1, =identmap
/* TODO: #define these bits and remove numbers here: */
        bic r1, r1, #0x7f
        orr r1, r1, #0x4a
        /* Write r1 to TTBR0 */
        mcr p15, 0, r1, c2, c0, 0;

        /*-----------------------------------------------------------*/
        /* TTBR1, Translation Table Base Register 1 */
        /* Manual: G8.2.166 */
        /* When TTBCR.EAE == 0, only 32 lower bits active */

        /* bits */
        /* 31-7 | val | Translation Table Base address 0 */
        /*    6 |  0  | Inner RGN[1] */
        /*    5 |  0  | Not Outer Shareable */
        /*  4-3 | 01  | (Outer) RGN */
        /*    2 |  0  | Implementation-defined feature bit */
        /*    1 |  1  | S: memory for translation tables is Shareable */
        /*    0 |  1  | IRGN[0] */

        /* IRGN: choose: Inner Write-Back Write-Allocate Cacheable */
        /* IRGN = 0b01, so (high) bit6 = 0, (low) bit0 = 1 */
        /* RGN: choose: Outer Write-Back Write-Allocate Cacheable */
        /* RGN = 0b01 */

        ldr r1, =l2_page_table
/* TODO: #define these bits and remove numbers here: */
        bic r1, r1, #0x7f
        orr r1, r1, #0xb
        /* Write r1 to TTBR1 */
        mcr p15, 0, r1, c2, c0, 1;

        /*-----------------------------------------------------------*/
        /* Translation Table Base Control Register */
        /* Manual: G8.2.164 : TTBCR */
        /* Manual: G 5.4.4 */

        /* bits */
        /*    31| Extended Address Enable: set to '0' for  */
        /*      |   Short-descriptor format tables         */
        /*  6-30| reserved */
        /*    5 | PD1: translation table disable for TTBR1 */
        /*    4 | PD0: translation table disable for TTBR0 */
        /*    3 | reserved */
        /*  0-2 | N: width of base addr in TTBR0 */

        /* Read TTBCR into r1 */
        mrc p15, 0, r1, c2, c0, 2 ;

        /* clear EAE (bit 31) */
        bic r1, r1, #0x8000000
        /* enable table walk via TTBR0 (bit 4) and TTBR1 (bit 5) */
        orr r1, r1, #0x30
        /* N: width of the base address in TTBR0 (bits 0-2) */
        /* "If N == 0 then use TTBR0. Setting TTBCR.N to zero disables
         *  use of a second set of translation tables." */
        /* If N > 0 then:
         *  If bits[31:32-N] of the input VA are all zero, then use TTBR0.
         *  Otherwise use TTBR1.
         */
        bic r1, r1, #0x7

        /* Write modified r1 back to TTBCR */
        mcr p15, 0, r1, c2, c0, 2;

        /*-----------------------------------------------------------*/
        /* Domain Access Control Register */
        /* Manual G8.2.37 */
        /* Since we are using the short-format for page table entries,  */
        /* they contain the 'domain' value.                             */
        /* Set this register to define the access permission for each   */
        /* of the sixteen memory domains.                               */

        /* "Memory Domain 0": configure this as 'Client' (01)           */
        /* ie. accesses are checked against the bits in the translation */
        /* page tables.            */
        /* All other domains: configured for 'No access' (00)           */
        mov r1, #1
        mcr p15, 0, r1, c3, c0, 0;

        /* Done */
        mov     pc, lr
ENDFUNC(init_paging)

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */

/**
 * @file arch/arm/mm.c
 *
 * Memory management for arm.
 */

#include <stdint.h>
#include <xtf/compiler.h>

#if defined(__arch64__)
#define COND(_32, _64) _64
#else
#define COND(_32, _64) _32
#endif

typedef COND(uint32_t, uint64_t) l1_pmd_t;
typedef COND(uint32_t, uint64_t) l2_pte_t;

/* Page tables in data section, zeroed during load */
l1_pmd_t __page_aligned_data identmap[COND(1024, 512)];
l1_pmd_t __page_aligned_data l1_page_table[COND(1024, 512)];
l2_pte_t __page_aligned_data l2_page_table[COND(1024, 512)];

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */

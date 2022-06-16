#ifndef __ARCH_ARM32_MM_H__
#define __ARCH_ARM32_MM_H__

/* Short-format page tables have 32-bit descriptors */

/* L1: indexed by VA[31:20] */
#define ARM32_L1_TABLE_ENTRIES       1024
#define ARM32_L1_TABLE_INDEX_MASK    (ARM32_L1_TABLE_ENTRIES-1)
#define ARM32_L1_TABLE_SHIFT         20

/* L2: indexed by VA[19:12] */
#define ARM32_L2_TABLE_ENTRIES       256
#define ARM32_L2_TABLE_INDEX_MASK    (ARM32_L2_TABLE_ENTRIES-1)
#define ARM32_L2_TABLE_SHIFT         12

/* Short-format PDE page tables can contain section and block descriptors */

/* Short format page table descriptor flags for sections */
#define PDE_SECTION                 (1u <<  1)
#define PDE_SECTION_B               (1u <<  2)
#define PDE_SECTION_C               (1u <<  3)
#define PDE_SECTION_XN              (1u <<  4)
#define PDE_SECTION_DOMAIN         (15u <<  5)
#define PDE_SECTION_APlow           (3u << 10)
#define PDE_SECTION_APhigh          (1u << 15)
#define PDE_SECTION_TEX             (7u << 12)
#define PDE_SECTION_S               (1u << 16)
#define PDE_SECTION_nG              (1u << 17)
#define PDE_SECTION_SS              (1u << 18)
#define PDE_SECTION_NS              (1u << 19)

/* AP[1:0] = 10, AP[2] = 0 */
#define PDE_SECTION_ACCESS_RW (1u << 11)

#define ARM32_L1_TABLE_BLOCK PDE_SECTION | \
                             PDE_SECTION_B | \
                             PDE_SECTION_C | \
                             PDE_SECTION_ACCESS_RW | \
                             PDE_SECTION_S | \
                             PDE_SECTION_NS

/* translation table entry bits: */
/* 31-10 | val | Translation Table Base address */
/*    9  |  0  | Implementation defined */
/*  5-8  |  0  | Domain (Arm terminology, not Xen) */
/*    4  |  0  | Reserved */
/*    3  |  1  | Non-Secure */
/*    2  |  0  | Privileged eXecute Never */
/*    1  |  0  | 0: is a translation table entry */
/*    0  |  1  | valid bit - 1: valid, 0: invalid */

/* Short format page table descriptor flags for translation table entries */
#define PDE_ENTRY                   (1u <<  0)
#define PDE_ENTRY_PXN               (1u <<  2)
#define PDE_ENTRY_NS                (1u <<  3)
#define PDE_ENTRY_RESERVED          (1u <<  4)
#define PDE_ENTRY_DOMAIN           (15u <<  5)
#define PDE_ENTRY_IMPL_DEFINED      (1u <<  9)

#define ARM32_L1_TABLE_ENTRY PDE_ENTRY | PDE_ENTRY_NS

/* Leaf translation tables can contain entries for Small and Large pages */
#define PTE_LARGE_PAGE              (1u <<  0)
#define PTE_SMALL_PAGE              (1u <<  1)
#define PTE_B                       (1u <<  2)
#define PTE_C                       (1u <<  3)
#define PTE_APlow                   (3u <<  4)
#define PTE_APhigh                  (1u <<  9)
#define PTE_S                       (1u << 10)
#define PTE_nG                      (1u << 11)
#define PTE_SMALL_TEX               (7u <<  6)
#define PTE_LARGE_TEX               (7u << 12)
#define PTE_SMALL_XN                (1u << 0)
#define PTE_LARGE_XN                (1u << 15)

/* AP[1:0] = 10, AP[2] = 0 */
/* TODO: check this: */
#define PTE_ACCESS_RW (1u << 5)

#define ARM32_L2_TABLE_SMALL_ENTRY PTE_SMALL_PAGE | \
                                   PTE_B | \
                                   PTE_C | \
                                   PTE_ACCESS_RW | \
                                   PTE_S

#endif /* __ARCH_ARM32_MM_H__ */

#ifndef XTF_ARM_MM_H
#define XTF_ARM_MM_H

static inline void *pfn_to_virt(unsigned long pfn)
{
    return (void *)(pfn << PAGE_SHIFT);
}

#endif

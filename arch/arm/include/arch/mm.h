#ifndef XTF_ARM_MM_H
#define XTF_ARM_MM_H

#include <xtf/numbers.h>

static inline void *pfn_to_virt(unsigned long pfn)
{
    return (void *)(pfn << PAGE_SHIFT);
}

static inline unsigned long virt_to_pfn(const void *va)
{
    return _u(va) >> PAGE_SHIFT;
}

#endif

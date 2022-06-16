#include <xtf/asm_macros.h>

ENTRY(init_paging)
/* skip initialization */
mov pc, lr
ENDFUNC(init_paging)

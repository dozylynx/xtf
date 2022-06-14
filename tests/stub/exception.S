#include <xtf/asm_macros.h>

ENTRY(init_exception_vector)
/* skip initialization */
mov pc, lr
ENDFUNC(init_exception_vector)

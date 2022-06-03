/**
 * @file arch/arm/setup.c
 *
 * Early bringup code for arm.
 */
#include <xtf/console.h>
#include <xtf/lib.h>

const char environment_description[] = ENVIRONMENT_DESCRIPTION;

void arch_setup(void)
{
    register_console_callback(xen_console_write);
}

void test_setup(void)
{
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */

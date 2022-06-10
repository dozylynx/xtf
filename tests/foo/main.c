/**
 * @file tests/foo/main.c
 * @ref test-foo - Hello World testcase.
 *
 * @page test-foo Example
 *
 * A development test case
 *
 * @see tests/foo/main.c
 */
#include <xtf.h>
#include <xtf/lib.h>
#include <xtf/console.h>

const char test_title[] = "Development Test";

void test_main(void)
{
    int domid;

    printk("test main\n");

    domid = xtf_get_domid();

    if ( domid > 0 )
        printk("domid was returned\n");
    else
        printk("no domid\n");

    xtf_success(NULL);
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

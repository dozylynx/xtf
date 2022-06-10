/**
 * @file arch/arm/setup.c
 *
 * Early bringup code for arm.
 */
#include <xtf/compiler.h>
#include <xtf/console.h>
#include <xtf/hypercall.h>
#include <xtf/lib.h>
#include <xtf/xenbus.h>

#include <arch/mm.h>

char __attribute__((section(".bss.page_aligned"))) stack[4096];

const char environment_description[] = ENVIRONMENT_DESCRIPTION;

shared_info_t shared_info __page_aligned_bss;

static void map_shared_info(void)
{
    int rc;
    struct xen_add_to_physmap xatp =
        {
            .domid = DOMID_SELF,
            .space = XENMAPSPACE_shared_info,
            .idx = 0,
            .gfn = virt_to_pfn(&shared_info),
        };

    rc = hypercall_memory_op(XENMEM_add_to_physmap, &xatp);
    if ( rc )
        panic("Failed to map shared_info: %d\n", rc);
}

static void setup_pv_console(void)
{
    xencons_interface_t *cons_ring;
    evtchn_port_t cons_evtchn;
    uint64_t raw_pfn, raw_evtchn;

    if ( (hvm_get_param(HVM_PARAM_CONSOLE_PFN, &raw_pfn) != 0) ||
         (hvm_get_param(HVM_PARAM_CONSOLE_EVTCHN, &raw_evtchn) != 0) )
        return;

    cons_evtchn = raw_evtchn;
    cons_ring = pfn_to_virt(raw_pfn);

    init_pv_console(cons_ring, cons_evtchn);
}

static void setup_xenbus(void)
{
    xenbus_interface_t *xb_ring;
    evtchn_port_t xb_port;
    uint64_t raw_pfn, raw_evtchn;
    int rc;

    rc = hvm_get_param(HVM_PARAM_STORE_PFN, &raw_pfn);
    if ( rc )
        panic("Failed to get XenStore PFN: %d\n", rc);

    rc = hvm_get_param(HVM_PARAM_STORE_EVTCHN, &raw_evtchn);
    if ( rc )
        panic("Failed to get XenStore evtchn: %d\n", rc);

    xb_ring = pfn_to_virt(raw_pfn);
    xb_port = raw_evtchn;

    init_xenbus(xb_ring, xb_port);
}

void arch_setup(void)
{
    register_console_callback(xen_console_write);
    setup_pv_console();
    map_shared_info();
    setup_xenbus();
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
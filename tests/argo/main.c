/**
 * @file tests/argo/main.c
 * @ref test-argo
 *
 * @page test-argo argo
 *
 * @todo Docs for test-argo
 *
 * @see tests/argo/main.c
 */
#include <xtf.h>
#include <xtf/lib.h>
#include <xen/argo.h>

const char test_title[] = "Argo hypercall test";

/*
 * The current Linux Argo driver has a default ring size of 32 4k pages,
 * so follow that for the test ring size.
 */
#define TEST_RING_NPAGES 32
static uint8_t ring_buffer[PAGE_SIZE * TEST_RING_NPAGES] __page_aligned_bss;

static int
test_invalid_register_ring(void)
{
    int rc;

    /* attempt an Argo call to register a ring with invalid arguments */
    rc = hypercall_argo_op(XEN_ARGO_OP_register_ring, NULL, NULL,
                           XEN_ARGO_MAX_RING_SIZE + 1, /* never allowed */
                           0);
    switch (rc) {
        case -EINVAL: /* this is the response we are looking for */
            return 0;

        /* All below here are test exit conditions */
        case 0:
            xtf_failure("Fail: an invalid ring register op was not rejected\n");
            break;
        case -ENOSYS:
            xtf_error("Error: Argo support has not been enabled in this hypervisor\n");
            break;
        case -EOPNOTSUPP:
            xtf_error("Error: Argo is not enabled at runtime for this hypervisor\n");
            break;
        case -ENODEV:
            xtf_error("Error: Argo is not enabled for this domain\n");
            break;
        case -EPERM:
            xtf_failure("Fail: ring registration by this domain is not permitted\n");
            break;
        default:
            xtf_failure("Fail: unknown error %d for invalid ring registration\n", rc);
            break;
    }

    return -1;
}

/* notify: asks Xen for information about rings */
static int
test_notify_for_one_ring(domid_t query_domid, xen_argo_port_t query_port,
                         bool exists)
{
    int rc;
    /* buffer for populating a struct with a variable length array member */
    uint8_t raw_buf[sizeof(xen_argo_ring_data_t) + 1 * sizeof(xen_argo_ring_data_ent_t)];
    xen_argo_ring_data_t *p_ring_data;

    p_ring_data = (xen_argo_ring_data_t *)&raw_buf;
    p_ring_data->nent = 1;
    p_ring_data->pad = 0;

    p_ring_data->data[0].ring.domain_id = query_domid;
    p_ring_data->data[0].ring.aport = query_port;
    p_ring_data->data[0].ring.pad = 0;

    p_ring_data->data[0].flags = 0;
    p_ring_data->data[0].space_required = 0;
    p_ring_data->data[0].max_message_size = 0;
    p_ring_data->data[0].pad = 0;

    rc = hypercall_argo_op(XEN_ARGO_OP_notify, p_ring_data, NULL, 0, 0);

    switch (rc) {
        case 0:
            break;
        case -ENODEV:
            xtf_failure("Fail: Argo is not enabled for this domain\n");
            return -1;
        case -EFAULT:
            xtf_failure("Fail: Memory fault performing notify test\n");
            return -1;
        default:
            xtf_failure("Fail: Unexpected error code %d in notify test\n", rc);
            return -1;
    }

    if ( !exists )
    {
        /* No currently-defined flags should be set for a non-existent ring */
        if ( p_ring_data->data[0].flags )
        {
            xtf_failure("Fail: Non-existent ring reported as existing\n");
            return -1;
        }
    }
    else
    {
        if ( !(p_ring_data->data[0].flags & XEN_ARGO_RING_EXISTS) )
        {
            xtf_failure("Fail: Ring not reported as existing\n");
            return -1;
        }
    }

    return 0;
}

/* See the Argo Linux device driver for similar use of these macros */
#define XEN_ARGO_ROUNDUP(x) roundup((x), XEN_ARGO_MSG_SLOT_SIZE)
#define ARGO_RING_OVERHEAD 80
#define TEST_RING_SIZE(npages) \
        (XEN_ARGO_ROUNDUP((((PAGE_SIZE)*npages) - ARGO_RING_OVERHEAD)))

static int
test_register_ring(domid_t own_domid, xen_argo_port_t aport)
{
    int rc;
    unsigned int i;
    xen_argo_register_ring_t reg;
    xen_argo_gfn_t gfns[TEST_RING_NPAGES];

    reg.aport = aport;
    reg.partner_id = own_domid;
    reg.len = TEST_RING_SIZE(TEST_RING_NPAGES);
    reg.pad = 0;

    for ( i = 0; i < TEST_RING_NPAGES; i++ )
        gfns[i] = virt_to_gfn(ring_buffer + (i * PAGE_SIZE));

    rc = hypercall_argo_op(XEN_ARGO_OP_register_ring, &reg, &gfns,
                           TEST_RING_NPAGES, XEN_ARGO_REGISTER_FLAG_FAIL_EXIST);
    switch (rc) {
        case 0:
            return 0;

        case -ENODEV:
            xtf_failure("Fail: Argo is not enabled for this domain\n");
            break;
        case -EFAULT:
            xtf_failure("Fail: Memory fault performing register ring test\n");
            break;
        default:
            xtf_failure("Fail: Unexpected error code %d in register ring test\n", rc);
            break;
    }
    return -1;
}

static int
test_unregister_ring(domid_t partner_domid, xen_argo_port_t aport,
                     bool exists)
{
    int rc;
    xen_argo_register_ring_t unreg;

    unreg.aport = aport;
    unreg.partner_id = partner_domid;
    unreg.pad = 0;

    rc = hypercall_argo_op(XEN_ARGO_OP_unregister_ring, &unreg, NULL, 0, 0);

    switch (rc) {
        case 0:
            if ( exists )
                return 0;
            xtf_failure("Fail: unexpected success unregistering non-existent ring\n");
            return -1;

        case -ENOENT:
            if ( !exists )
                return 0;
            xtf_failure("Fail: unexpected ring not found when unregistering \n");
            return -1;

        default:
            xtf_failure("Fail: Unexpected error code %d in unregister ring test\n", rc);
            break;
    }
    return -1;
}

static int
test_sendv(domid_t src_domid, xen_argo_port_t src_aport,
           domid_t dst_domid, xen_argo_port_t dst_aport,
           const char *msg_text, int msg_len, unsigned int msg_type)
{
    int rc;
    xen_argo_send_addr_t send_addr;
    xen_argo_iov_t iovs[1];
    unsigned int niov;

    send_addr.src.domain_id = src_domid;
    send_addr.src.aport     = src_aport;
    send_addr.src.pad       = 0;
    send_addr.dst.domain_id = dst_domid;
    send_addr.dst.aport     = dst_aport;
    send_addr.dst.pad       = 0;

    iovs[0].iov_hnd = (unsigned long)msg_text;
    iovs[0].iov_len = msg_len;
    iovs[0].pad = 0;
    niov = 1;

    rc = hypercall_argo_op(XEN_ARGO_OP_sendv, &send_addr, &iovs, niov, msg_type);

    if ( rc != msg_len )
    {
        if ( rc < 0 )
        {
            xtf_failure("Fail: Unexpected error code %d in sendv test\n", rc);
            return -1;
        }
        xtf_failure("Fail: Unexpected message size %d written in sendv test\n", rc);
        return -1;
    }

    return 0;
}

static int
inspect_ring_after_first_single_sendv(domid_t src_domid,
                                      xen_argo_port_t src_aport,
                                      const char *sent_msg,
                                      unsigned int sent_msg_len,
                                      unsigned int sent_msg_type)
{
    int rc = 0;
    xen_argo_ring_t *ringp = (xen_argo_ring_t *)ring_buffer;
    struct xen_argo_ring_message_header *msg_hdr;
    unsigned int sent_length;

    if ( ringp->rx_ptr != 0 )
    {
        xtf_failure("Fail: receive pointer non-zero after sendv: %u\n",
                    ringp->rx_ptr);
        rc = -1;
    }

    if ( ringp->tx_ptr != XEN_ARGO_ROUNDUP(
                sizeof(struct xen_argo_ring_message_header) + sent_msg_len) )
    {
        xtf_failure("Fail: transmit pointer incorrect after sendv: %u\n",
                    ringp->rx_ptr);
        rc = -1;
    }

    msg_hdr = (struct xen_argo_ring_message_header *)&(ringp->ring);

    if ( msg_hdr->source.domain_id != src_domid )
    {
        xtf_failure("Fail: source domain id incorrect: %u, expected %u\n",
                    msg_hdr->source.domain_id, src_domid);
        rc = -1;
    }

    if ( msg_hdr->source.aport != src_aport )
    {
        xtf_failure("Fail: source domain port incorrect: %u, expected %u\n",
                    msg_hdr->source.domain_id, src_aport);
        rc = -1;
    }

    if ( msg_hdr->source.pad != 0 )
    {
        xtf_failure("Fail: source padding incorrect: %u, expected zero\n",
                    msg_hdr->source.pad);
        rc = -1;
    }

    if ( sent_msg_type != msg_hdr->message_type )
    {
        xtf_failure("Fail: message type incorrect: %u sent, %u received\n",
                    sent_msg_type, msg_hdr->message_type);
        rc = -1;
    }

    sent_length = sent_msg_len + sizeof(struct xen_argo_ring_message_header);
    if ( sent_length != msg_hdr->len )
    {
        xtf_failure("Fail: received message length incorrect: "
                    "%u sent is %u with header added, %u received\n",
                    sent_msg_len, sent_length, msg_hdr->len);
        rc = -1;
    }

    if ( strncmp((const char *)msg_hdr->data, sent_msg, sent_msg_len) )
    {
        xtf_failure("Fail: sent message got mangled\n");
        rc = -1;
    }

    return rc;
}

static int
clear_test_ring(void)
{
    memset(ring_buffer, 0, PAGE_SIZE * TEST_RING_NPAGES);
    return 0;
}

void test_main(void)
{
    int own_domid;
    xen_argo_port_t test_aport = 1;
    const char simple_text[] = "a simple thing to send\n";
    const unsigned int msg_type = 0x12345678;

    /* First test validates for Argo availability to gate further testing */
    if ( test_invalid_register_ring() )
        return;

    own_domid = xtf_get_domid();
    if ( own_domid < 0 )
        return xtf_error("Error: could not determine domid of the test domain\n");

    if ( test_notify_for_one_ring(own_domid, test_aport, false) ||
         test_unregister_ring(own_domid, test_aport, false) ||
         clear_test_ring() ||
         test_register_ring(own_domid, test_aport) ||
         test_notify_for_one_ring(own_domid, test_aport, true) ||
         test_unregister_ring(own_domid, test_aport, true) ||
         test_notify_for_one_ring(own_domid, test_aport, false) ||
         test_unregister_ring(own_domid, test_aport, false) ||
         clear_test_ring() ||
         test_register_ring(own_domid, test_aport) ||
         test_sendv(own_domid, test_aport, own_domid, test_aport,
                    simple_text, sizeof(simple_text), msg_type) ||
         inspect_ring_after_first_single_sendv(own_domid, test_aport,
                    simple_text, sizeof(simple_text), msg_type) ||
         test_notify_for_one_ring(own_domid, test_aport, true) ||
         test_unregister_ring(own_domid, test_aport, true) ||
         test_unregister_ring(own_domid, test_aport, false) )
        return;

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

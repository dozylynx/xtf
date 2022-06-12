#include <xtf/atomic.h>
#include <xtf/bitops.h>
#include <xtf/console.h>
#include <xtf/hypercall.h>
#include <xtf/lib.h>
#include <xtf/traps.h>
#include <xtf/xenbus.h>

static xenbus_interface_t *xb_ring;
static evtchn_port_t xb_port;
static char payload[XENSTORE_PAYLOAD_MAX + 1];

void init_xenbus(xenbus_interface_t *ring, evtchn_port_t port)
{
    if ( port >= (sizeof(shared_info.evtchn_pending) * CHAR_BIT) )
        panic("evtchn %u out of evtchn_pending[] range\n", port);

    xb_ring = ring;
    xb_port = port;
}

/*
 * Write some raw data into the xenbus ring.  Waits for sufficient space to
 * appear if necessary.
 */
static void xenbus_write(const void *data, size_t len)
{
    uint32_t part, done = 0;

    while ( len )
    {
        uint32_t prod = ACCESS_ONCE(xb_ring->req_prod);
        uint32_t cons = ACCESS_ONCE(xb_ring->req_cons);

        part = (XENBUS_RING_SIZE - 1) - mask_xenbus_idx(prod - cons);

        /* No space?  Kick xenstored and wait for it to consume some data. */
        if ( !part )
        {
            hypercall_evtchn_send(xb_port);

            if ( !test_and_clear_bit(xb_port, shared_info.evtchn_pending) )
                hypercall_poll(xb_port);

            continue;
        }

        /* Don't overrun the ring. */
        part = min(part, XENBUS_RING_SIZE - mask_xenbus_idx(prod));

        /* Don't write more than necessary. */
        part = min(part, (unsigned int)len);

        memcpy(xb_ring->req + mask_xenbus_idx(prod), data + done, part);

        /* Complete the data read before updating the new producer index. */
        smp_wmb();

        ACCESS_ONCE(xb_ring->req_prod) = prod + part;

        len -= part;
        done += part;
    }
}

/*
 * Read some raw data from the xenbus ring.  Waits for sufficient data to
 * appear if necessary.
 */
static void xenbus_read(void *data, size_t len)
{
    uint32_t part, done = 0;

    while ( len )
    {
        uint32_t prod = ACCESS_ONCE(xb_ring->rsp_prod);
        uint32_t cons = ACCESS_ONCE(xb_ring->rsp_cons);

        part = prod - cons;

        /* No data?  Kick xenstored and wait for it to produce some data. */
        if ( !part )
        {
            hypercall_evtchn_send(xb_port);
            printk("xenbus_read 1\n");

            if ( !test_and_clear_bit(xb_port, shared_info.evtchn_pending) )
            {
                printk("xenbus_read 2\n");
                hypercall_poll(xb_port);
            }

            printk("xenbus_read 3\n");
            continue;
        }

        /* Avoid overrunning the ring. */
        part = min(part, XENBUS_RING_SIZE - mask_xenbus_idx(cons));

        /* Don't read more than necessary. */
        part = min(part, (unsigned int)len);

        memcpy(data + done, xb_ring->rsp + mask_xenbus_idx(cons), part);

        /* Complete the data read before updating the new consumer index. */
        smp_mb();

        ACCESS_ONCE(xb_ring->rsp_cons) = cons + part;

        len -= part;
        done += part;

    }
}

int xenstore_init(void)
{
    /* Nothing to initialise.  Report the presence of the xenbus ring. */
    return xb_port ? 0 : -ENODEV;
}

const char *xenstore_read(const char *path)
{
    struct xenstore_msg_hdr hdr = {
        .type = XS_READ,
        .len = strlen(path) + 1, /* Must send the NUL terminator. */
    };
    printk("xenstore_read 1\n");

    /* Write the header and path to read. */
    xenbus_write(&hdr, sizeof(hdr));
    xenbus_write(path, hdr.len);

    printk("xenstore_read 2\n");

    /* Kick xenstored. */
    hypercall_evtchn_send(xb_port);

    printk("xenstore_read 3\n");

    /* Read the response header. */
    xenbus_read(&hdr, sizeof(hdr));

    printk("xenstore_read 4\n");

    if ( hdr.type != XS_READ )
        return NULL;

    printk("xenstore_read 5\n");

    if ( hdr.len > XENSTORE_PAYLOAD_MAX )
    {
        printk("xenstore_read 6\n");
        /*
         * Xenstored handed back too much data.  Drain it safely attempt to
         * prevent the protocol from stalling.
         */
        while ( hdr.len )
        {
            unsigned int part = min(hdr.len, XENSTORE_PAYLOAD_MAX + 0u);

            printk("xenstore_read 7\n");
            xenbus_read(payload, part);
            printk("xenstore_read 8\n");

            hdr.len -= part;
        }

        printk("xenstore_read 9\n");
        return NULL;
    }

    printk("xenstore_read 10\n");

    /* Read the response payload. */
    xenbus_read(payload, hdr.len);

    printk("xenstore_read 11\n");

    /* Safely terminate the reply, just in case xenstored didn't. */
    payload[hdr.len] = '\0';

    printk("xenstore_read 12\n");
    return payload;
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

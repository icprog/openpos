
			idle_timeout.hdr.type =
				cpu_to_le16(I2400M_TLV_CONFIG_IDLE_TIMEOUT);
			idle_timeout.hdr.length = cpu_to_le16(
				sizeof(idle_timeout) - sizeof(idle_timeout.hdr));
			idle_timeout.timeout = 0;
			args[argc++] = &idle_timeout.hdr;
		}
	}
	if (i2400m_ge_v1_4(i2400m)) {
		/* Enable extended RX data format? */
		df.hdr.type =
			cpu_to_le16(I2400M_TLV_CONFIG_D2H_DATA_FORMAT);
		df.hdr.length = cpu_to_le16(
			sizeof(df) - sizeof(df.hdr));
		df.format = 1;
		args[argc++] = &df.hdr;

		/* Enable RX data reordering?
		 * (switch flipped in rx.c:i2400m_rx_setup() after fw upload) */
		if (i2400m->rx_reorder) {
			dlhr.hdr.type =
				cpu_to_le16(I2400M_TLV_CONFIG_DL_HOST_REORDER);
			dlhr.hdr.length = cpu_to_le16(
				sizeof(dlhr) - sizeof(dlhr.hdr));
			dlhr.reorder = 1;
			args[argc++] = &dlhr.hdr;
		}
	}
	result = i2400m_set_init_config(i2400m, args, argc);
	if (result < 0)
		goto error;
	/*
	 * Update state: Here it just calls a get state; parsing the
	 * result (System State TLV and RF Status TLV [done in the rx
	 * path hooks]) will set the hardware and software RF-Kill
	 * status.
	 */
	result = i2400m_cmd_get_state(i2400m);
error:
	if (result < 0)
		dev_err(dev, "failed to initialize the device: %d\n", result);
	d_fnend(3, dev, "(i2400m %p) = %d\n", i2400m, result);
	return result;
}


/**
 * i2400m_dev_shutdown - Shutdown a running device
 *
 * @i2400m: device descriptor
 *
 * Release resources acquired during the running of the device; in
 * theory, should also tell the device to go to sleep, switch off the
 * radio, all that, but at this point, in most cases (driver
 * disconnection, reset handling) we can't even talk to the device.
 */
void i2400m_dev_shutdown(struct i2400m *i2400m)
{
	struct device *dev = i2400m_dev(i2400m);

	d_fnstart(3, dev, "(i2400m %p)\n", i2400m);
	d_fnend(3, dev, "(i2400m %p) = void\n", i2400m);
	return;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               AT91_UDP_TXVC_TXVDIS (1 << 8)	/* Transceiver Disable */
#define     AT91_UDP_TXVC_PUON   (1 << 9)	/* PullUp On [AT91SAM9260 only] */

/*-------------------------------------------------------------------------*/

/*
 * controller driver data structures
 */

#define	NUM_ENDPOINTS	6

/*
 * hardware won't disable bus reset, or resume while the controller
 * is suspended ... watching suspend helps keep the logic symmetric.
 */
#define	MINIMUS_INTERRUPTUS \
	(AT91_UDP_ENDBUSRES | AT91_UDP_RXRSM | AT91_UDP_RXSUSP)

struct at91_ep {
	struct usb_ep			ep;
	struct list_head		queue;
	struct at91_udc			*udc;
	void __iomem			*creg;

	unsigned			maxpacket:16;
	u8				int_mask;
	unsigned			is_pingpong:1;

	unsigned			stopped:1;
	unsigned			is_in:1;
	unsigned			is_iso:1;
	unsigned			fifo_bank:1;

	const struct usb_endpoint_descriptor
					*desc;
};

/*
 * driver is non-SMP, and just blocks IRQs whenever it needs
 * access protection for chip registers or driver state
 */
struct at91_udc {
	struct usb_gadget		gadget;
	struct at91_ep			ep[NUM_ENDPOINTS];
	struct usb_gadget_driver	*driver;
	unsigned			vbus:1;
	unsigned			enabled:1;
	unsigned			clocked:1;
	unsigned			suspended:1;
	unsigned			req_pending:1;
	unsigned			wait_for_addr_ack:1;
	unsigned			wait_for_config_ack:1;
	unsigned			selfpowered:1;
	unsigned			active_suspend:1;
	u8				addr;
	struct at91_udc_data		board;
	struct clk			*iclk, *fclk;
	struct platform_device		*pdev;
	struct proc_dir_entry		*pde;
	void __iomem			*udp_baseaddr;
	int				udp_irq;
};

static inline struct at91_udc *to_udc(struct usb_gadget *g)
{
	return container_of(g, struct at91_udc, gadget);
}

struct at91_request {
	struct usb_request		req;
	struct list_head		queue;
};

/*-------------------------------------------------------------------------*/

#ifdef VERBOSE_DEBUG
#    define VDBG		DBG
#else
#    define VDBG(stuff...)	do{}while(0)
#endif

#ifdef PACKET_TRACE
#    define PACKET		VDBG
#else
#    define PACKET(stuff...)	do{}while(0)
#endif

#define ERR(stuff...)		pr_err("udc: " stuff)
#define WARNING(stuff...)	pr_warning("udc: " stuff)
#define INFO(stuff...)		pr_info("udc: " stuff)
#define DBG(stuff...)		pr_debug("udc: " stuff)

#endif


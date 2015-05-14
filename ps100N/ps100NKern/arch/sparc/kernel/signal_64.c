end =		ohci_bus_suspend,
	.bus_resume =		ohci_bus_resume,
#endif
	.start_port_reset =	ohci_start_port_reset,
};

static int ohci_hcd_ppc_soc_drv_probe(struct platform_device *pdev)
{
	int ret;

	if (usb_disabled())
		return -ENODEV;

	ret = usb_hcd_ppc_soc_probe(&ohci_ppc_soc_hc_driver, pdev);
	return ret;
}

static int ohci_hcd_ppc_soc_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_hcd_ppc_soc_remove(hcd, pdev);
	return 0;
}

static struct platform_driver ohci_hcd_ppc_soc_driver = {
	.probe		= ohci_hcd_ppc_soc_drv_probe,
	.remove		= ohci_hcd_ppc_soc_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
#ifdef	CONFIG_PM
	/*.suspend	= ohci_hcd_ppc_soc_drv_suspend,*/
	/*.resume	= ohci_hcd_ppc_soc_drv_resume,*/
#endif
	.driver		= {
		.name	= "ppc-soc-ohci",
		.owner	= THIS_MODULE,
	},
};

MODULE_ALIAS("platform:ppc-soc-ohci");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         /*======================================================================

    A driver for PCMCIA serial devices

    serial_cs.c 1.134 2002/05/04 05:48:53

    The contents of this file are subject to the Mozilla Public
    License Version 1.1 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS
    IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
    implied. See the License for the specific language governing
    rights and limitations under the License.

    The initial developer of the original code is David A. Hinds
    <dahinds@users.sourceforge.net>.  Portions created by David A. Hinds
    are Copyright (C) 1999 David A. Hinds.  All Rights Reserved.

    Alternatively, the contents of this file may be used under the
    terms of the GNU General Public License version 2 (the "GPL"), in which
    case the provisions of the GPL are applicable instead of the
    above.  If you wish to allow the use of your version of this file
    only under the terms of the GPL and not to allow others to use
    your version of this file under the MPL, indicate your decision
    by deleting the provisions above and replace them with the notice
    and other provisions required by the GPL.  If you do not delete
    the provisions above, a recipient may use your version of this
    file under either the MPL or the GPL.
    
======================================================================*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/serial_core.h>
#include <linux/delay.h>
#include <linux/major.h>
#include <asm/io.h>
#include <asm/system.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ciscode.h>
#include <pcmcia/ds.h>
#include <pcmcia/cisreg.h>

#include "8250.h"

#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0644);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static char *version = "serial_cs.c 1.134 2002/05/04 05:48:53 (David Hinds)";
#else
#define DEBUG(n, args...)
#endif

/*====================================================================*/

/* Parameters that can be set with 'insmod' */

/* Enable the speaker? */
static int do_sound = 1;
/* Skip strict UART tests? */
static int buggy_uart;

module_param(do_sound, int, 0444);
module_param(buggy_uart, int, 0444);

/*====================================================================*/

/* Table of multi-port card ID's */

struct serial_quirk {
	unsigned int manfid;
	unsigned int prodid;
	int multi;		/* 1 = multifunction, > 1 = # ports */
	void (*config)(struct pcmcia_device *);
	void (*setup)(struct pcmcia_device *, struct uart_port *);
	void (*wakeup)(struct pcmcia_device *);
	int (*post)(struct pcmcia_device *);
};

struct serial_info {
	struct pcmcia_device	*p_dev;
	int			ndev;
	int			multi;
	int			slave;
	int			manfid;
	int			prodid;
	int			c950ctrl;
	dev_node_t		node[4];
	int			line[4];
	const struct serial_quirk *quirk;
};

struct serial_cfg_mem {
	tuple_t tuple;
	cisparse_t parse;
	u_char buf[256];
};

/*
 * vers_1 5.0, "Brain Boxes", "2-Port RS232 card", "r6"
 * manfid 0x0160, 0x0104
 * This card appears to have a 14.7456MHz clock.
 */
static void quirk_setup_brainboxes_0104(struct pcmcia_device *link, struct uart_port *port)
{
	port->uartclk = 14745600;
}

static int quirk_post_ibm(struct pcmcia_device *link)
{
	conf_reg_t reg = { 0, CS_READ, 0x800, 0 };
	int last_ret, last_fn;

	last_ret = pcmcia_access_configuration_register(link, &reg);
	if (last_ret) {
		last_fn = AccessConfigurationRegister;
		goto cs_failed;
	}
	reg.Action = CS_WRITE;
	reg.Value = reg.Value | 1;
	last_ret = pcmcia_access_configuration_register(link, &reg);
	if (last_ret) {
		last_fn = AccessConfigurationRegister;
		goto cs_failed;
	}
	return 0;

 cs_failed:
	cs_error(link, last_fn, last_ret);
	return -ENODEV;
}

/*
 * Nokia cards are not really multiport cards.  Shouldn't this
 * be handled by setting the quirk entry .multi = 0 | 1 ?
 */
static void quirk_config_nokia(struct pcmcia_device *link)
{
	struct serial_info *info = link->priv;

	if (info->multi > 1)
		info->multi = 1;
}

static void quirk_wakeup_oxsemi(struct pcmcia_device *link)
{
	struct serial_info *info = link->priv;

	outb(12, info->c950ctrl + 1);
}

/* request_region? oxsemi branch does no request_region too... */
/*
 * This sequence is needed to properly initialize MC45 attached to OXCF950.
 * I tried decreasing these msleep()s, but it worked properly (survived
 * 1000 stop/start operations) with these timeouts (or bigger).
 */
static void quirk_wakeup_possio_gcc(struct pcmcia_device *link)
{
	struct serial_info *info = link->priv;
	unsigned int ctrl = info->c950ctrl;

	outb(0xA, ctrl + 1);
	msleep(100);
	outb(0xE, ctrl + 1);
	msleep(300);
	outb(0xC, ctrl + 1);
	msleep(100);
	outb(0xE, ctrl + 1);
	msleep(200);
	outb(0xF, ctrl + 1);
	msleep(100);
	outb(0xE, ctrl + 1);
	msleep(100);
	outb(0xC, ctrl + 1);
}

/*
 * Socket Dual IO: this enables irq's for second port
 */
static void quirk_config_socket(struct pcmcia_device *link)
{
	struct serial_info *info = link->priv;

	if (info->multi) {
		link->conf.Present |= PRESENT_EXT_STATUS;
		link->conf.ExtStatus = ESR_REQ_ATTN_ENA;
	}
}

static const struct serial_quirk quirks[] = {
	{
		.manfid	= 0x0160,
		.prodid	= 0x0104,
		.multi	= -1,
		.setup	= quirk_setup_brainboxes_0104,
	}, {
		.manfid	= MANFID_IBM,
		.prodid	= ~0,
		.multi	= -1,
		.post	= quirk_post_ibm,
	}, {
		.manfid	= MANFID_INTEL,
		.prodid	= PRODID_INTEL_DUAL_RS232,
		.multi	= 2,
	}, {
		.manfid	= MANFID_NATINST,
		.prodid	= PRODID_NATINST_QUAD_RS232,
		.multi	= 4,
	}, {
		.manfid	= MANFID_NOKIA,
		.prodid	= ~0,
		.multi	= -1,
		.config	= quirk_config_nokia,
	}, {
		.manfid	= MANFID_OMEGA,
		.prodid	= PRODID_OMEGA_QSP_100,
		.multi	= 4,
	}, {
		.manfid	= MANFID_OXSEMI,
		.prodid	= ~0,
		.multi	= -1,
		.wakeup	= quirk_wakeup_oxsemi,
	}, {
		.manfid	= MANFID_POSSIO,
		.prodid	= PRODID_POSSIO_GCC,
		.multi	= -1,
		.wakeup	= quirk_wakeup_possio_gcc,
	}, {
		.manfid	= MANFID_QUATECH,
		.prodid	= PRODID_QUATECH_DUAL_RS232,
		.multi	= 2,
	}, {
		.manfid	= MANFID_QUATECH,
		.prodid	= PRODID_QUATECH_DUAL_RS232_D1,
		.multi	= 2,
	}, {
		.manfid	= MANFID_QUATECH,
		.prodid	= PRODID_QUATECH_DUAL_RS232_G,
		.multi	= 2,
	}, {
		.manfid	= MANFID_QUATECH,
		.prodid	= PRODID_QUATECH_QUAD_RS232,
		.multi	= 4,
	}, {
		.manfid	= MANFID_SOCKET,
		.prodid	= PRODID_SOCKET_DUAL_RS232,
		.multi	= 2,
		.config	= quirk_config_socket,
	}, {
		.manfid	= MANFID_SOCKET,
		.prodid	= ~0,
		.multi	= -1,
		.config	= quirk_config_socket,
	}
};


static int serial_config(struct pcmcia_device * link);


/*======================================================================

    After a card is removed, serial_remove() will unregister
    the serial device(s), and release the PCMCIA configuration.
    
======================================================================*/

static void serial_remove(struct pcmcia_device *link)
{
	struct serial_info *info = link->priv;
	int i;

	DEBUG(0, "serial_release(0x%p)\n", link);

	/*
	 * Recheck to see if the device is still configured.
	 */
	for (i = 0; i < info->ndev; i++)
		serial8250_unregister_port(info->line[i]);

	info->p_dev->dev_node = NULL;

	if (!info->slave)
		pcmcia_disable_device(link);
}

static int serial_suspend(struct pcmcia_device *link)
{
	struct serial_info *info = link->priv;
	int i;

	for (i = 0; i < info->ndev; i++)
		serial8250_suspend_port(info->line[i]);

	return 0;
}

static int serial_resume(struct pcmcia_device *link)
{
	struct serial_info *info = link->priv;
	int i;

	for (i = 0; i < info->ndev; i++)
		serial8250_resume_port(info->line[i]);

	if (info->quirk && info->quirk->wakeup)
		info->quirk->wakeup(link);

	return 0;
}

/*======================================================================

    serial_attach() creates an "instance" of the driver, allocating
    local data structures for one device.  The device is registered
    with Card Services.

======================================================================*/

static int serial_probe(struct pcmcia_device *link)
{
	struct serial_info *info;

	DEBUG(0, "serial_attach()\n");

	/* Create new serial device */
	info = kzalloc(sizeof (*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	info->p_dev = link;
	link->priv = info;

	link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
	link->io.NumPorts1 = 8;
	link->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING;
	link->irq.IRQInfo1 = IRQ_LEVEL_ID;
	link->conf.Attributes = CONF_ENABLE_IRQ;
	if (do_sound) {
		link->conf.Attributes |= CONF_ENABLE_SPKR;
		link->conf.Status = CCSR_AUDIO_ENA;
	}
	link->conf.IntType = INT_MEMORY_AND_IO;

	return serial_config(link);
}

/*======================================================================

    This deletes a driver "instance".  The device is de-registered
    with Card Services.  If it has been released, all local data
    structures are freed.  Otherwise, the structures will be freed
    when the device is released.

======================================================================*/

static void serial_detach(struct pcmcia_device *link)
{
	struct serial_info *info = link->priv;

	DEBUG(0, "serial_detach(0x%p)\n", link);

	/*
	 * Ensure any outstanding scheduled tasks are completed.
	 */
	flush_scheduled_work();

	/*
	 * Ensure that the ports have been released.
	 */
	serial_remove(link);

	/* free bits */
	kfree(info);
}

/*====================================================================*/

static int setup_serial(struct pcmcia_device *handle, struct serial_info * info,
			unsigned int iobase, int irq)
{
	struct uart_port port;
	int line;

	memset(&port, 0, sizeof (struct uart_port));
	port.iobase = iobase;
	port.irq = irq;
	port.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_SHARE_IRQ;
	port.uartclk = 1843200;
	port.dev = &handle_to_dev(handle);
	if (buggy_uart)
		port.flags |= UPF_BUGGY_UART;

	if (info->quirk && info->quirk->setup)
		info->quirk->setup(handle, &port);

	line = serial8250_register_port(&port);
	if (line < 0) {
		printk(KERN_NOTICE "serial_cs: serial8250_register_port() at "
		       "0x%04lx, irq %d failed\n", (u_long)iobase, irq);
		return -EINVAL;
	}

	info->line[info->ndev] = line;
	sprintf(info->node[info->ndev].dev_name, "ttyS%d", line);
	info->node[info->ndev].major = TTY_MAJOR;
	info->node[info->ndev].minor = 0x40 + line;
	if (info->ndev > 0)
		info->node[info->ndev - 1].next = &info->node[info->ndev];
	info->ndev++;

	return 0;
}

/*====================================================================*/

static int
first_tuple(struct pcmcia_device *handle, tuple_t * tuple, cisparse_t * parse)
{
	int i;
	i = pcmcia_get_first_tuple(handle, tuple);
	if (i != 0)
		return i;
	i = pcmcia_get_tuple_data(handle, tuple);
	if (i != 0)
		return i;
	return pcmcia_parse_tuple(tuple, parse);
}

/*====================================================================*/

static int simple_config_check(struct pcmcia_device *p_dev,
			       cistpl_cftable_entry_t *cf,
			       cistpl_cftable_entry_t *dflt,
			       unsigned int vcc,
			       void *priv_data)
{
	static const int size_table[2] = { 8, 16 };
	int *try = priv_data;

	if (cf->vpp1.present & (1 << CISTPL_POWER_VNOM))
		p_dev->conf.Vpp =
			cf->vpp1.param[CISTPL_POWER_VNOM] / 10000;

	if ((cf->io.nwin > 0) && (cf->io.win[0].len == size_table[(*try >> 1)])
	    && (cf->io.win[0].base != 0)) {
		p_dev->io.BasePort1 = cf->io.win[0].base;
		p_dev->io.IOAddrLines = ((*try & 0x1) == 0) ?
			16 : cf->io.flags & CISTPL_IO_LINES_MASK;
		if (!pcmcia_request_io(p_dev, &p_dev->io))
			return 0;
	}
	return -EINVAL;
}

static int simple_config_check_notpicky(struct pcmcia_device *p_dev,
					cistpl_cftable_entry_t *cf,
					cistpl_cftable_entry_t *dflt,
					unsigned int vcc,
					void *priv_data)
{
	static const unsigned int base[5] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8, 0x0 };
	int j;

	if ((cf->io.nwin > 0) && ((cf->io.flags & CISTPL_IO_LINES_MASK) <= 3)) {
		for (j = 0; j < 5; j++) {
			p_dev->io.BasePort1 = base[j];
			p_dev->io.IOAddrLines = base[j] ? 16 : 3;
			if (!pcmcia_request_io(p_dev, &p_dev->io))
				return 0;
		}
	}
	return -ENODEV;
}

static int simple_config(struct pcmcia_device *link)
{
	struct serial_info *info = link->priv;
	int i = -ENODEV, try;

	/* If the card is already configured, look up the port and irq */
	if (link->function_config) {
		unsigned int port = 0;
		if ((link->io.BasePort2 != 0) &&
		    (link->io.NumPorts2 == 8)) {
			port = link->io.BasePort2;
			info->slave = 1;
		} else if ((info->manfid == MANFID_OSITECH) &&
			   (link->io.NumPorts1 == 0x40)) {
			port = link->io.BasePort1 + 0x28;
			info->slave = 1;
		}
		if (info->slave) {
			return setup_serial(link, info, port,
					    link->irq.AssignedIRQ);
		}
	}

	/* First pass: look for a config entry that looks normal.
	 * Two tries: without IO aliases, then with aliases */
	for (try = 0; try < 4; try++)
		if (!pcmcia_loop_config(link, simple_config_check, &try))
			goto found_port;

	/* Second pass: try to find an entry that isn't picky about
	   its base address, then try to grab any standard serial port
	   address, and finally try to get any free port. */
	if (!pcmcia_loop_config(link, simple_config_check_notpicky, NULL))
		goto found_port;

	printk(KERN_NOTICE
	       "serial_cs: no usable port range found, giving up\n");
	cs_error(link, RequestIO, i);
	return -1;

found_port:
	i = pcmcia_request_irq(link, &link->irq);
	if (i != 0) {
		cs_error(link, RequestIRQ, i);
		link->irq.AssignedIRQ = 0;
	}
	if (info->multi && (info->manfid == MANFID_3COM))
		link->conf.ConfigIndex &= ~(0x08);

	/*
	 * Apply any configuration quirks.
	 */
	if (info->quirk && info->quirk->config)
		info->quirk->config(link);

	i = pcmcia_request_configuration(link, &link->conf);
	if (i != 0) {
		cs_error(link, RequestConfiguration, i);
		return -1;
	}
	return setup_serial(link, info, link->io.BasePort1, link->irq.AssignedIRQ);
}

static int multi_config_check(struct pcmcia_device *p_dev,
			      cistpl_cftable_entry_t *cf,
			      cistpl_cftable_entry_t *dflt,
			      unsigned int vcc,
			      void *priv_data)
{
	int *base2 = priv_data;

	/* The quad port cards have bad CIS's, so just look for a
	   window larger than 8 ports and assume
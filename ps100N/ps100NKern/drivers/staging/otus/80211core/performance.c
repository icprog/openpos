shiba tc6393xb revision %d at 0x%08lx, irq %d\n",
			tmio_ioread8(tc6393xb->scr + SCR_REVID),
			(unsigned long) iomem->start, tc6393xb->irq);

	tc6393xb->gpio.base = -1;

	if (tcpd->gpio_base >= 0) {
		ret = tc6393xb_register_gpio(tc6393xb, tcpd->gpio_base);
		if (ret)
			goto err_gpio_add;
	}

	tc6393xb_attach_irq(dev);

	if (tcpd->setup) {
		ret = tcpd->setup(dev);
		if (ret)
			goto err_setup;
	}

	tc6393xb_cells[TC6393XB_CELL_NAND].driver_data = tcpd->nand_data;
	tc6393xb_cells[TC6393XB_CELL_NAND].platform_data =
		&tc6393xb_cells[TC6393XB_CELL_NAND];
	tc6393xb_cells[TC6393XB_CELL_NAND].data_size =
		sizeof(tc6393xb_cells[TC6393XB_CELL_NAND]);

	tc6393xb_cells[TC6393XB_CELL_MMC].platform_data =
		&tc6393xb_cells[TC6393XB_CELL_MMC];
	tc6393xb_cells[TC6393XB_CELL_MMC].data_size =
		sizeof(tc6393xb_cells[TC6393XB_CELL_MMC]);

	tc6393xb_cells[TC6393XB_CELL_OHCI].platform_data =
		&tc6393xb_cells[TC6393XB_CELL_OHCI];
	tc6393xb_cells[TC6393XB_CELL_OHCI].data_size =
		sizeof(tc6393xb_cells[TC6393XB_CELL_OHCI]);

	tc6393xb_cells[TC6393XB_CELL_FB].driver_data = tcpd->fb_data;
	tc6393xb_cells[TC6393XB_CELL_FB].platform_data =
		&tc6393xb_cells[TC6393XB_CELL_FB];
	tc6393xb_cells[TC6393XB_CELL_FB].data_size =
		sizeof(tc6393xb_cells[TC6393XB_CELL_FB]);

	ret = mfd_add_devices(&dev->dev, dev->id,
			tc6393xb_cells, ARRAY_SIZE(tc6393xb_cells),
			iomem, tcpd->irq_base);

	if (!ret)
		return 0;

	if (tcpd->teardown)
		tcpd->teardown(dev);

err_setup:
	tc6393xb_detach_irq(dev);

err_gpio_add:
	if (tc6393xb->gpio.base != -1)
		temp = gpiochip_remove(&tc6393xb->gpio);
	tcpd->disable(dev);
err_clk_enable:
	clk_disable(tc6393xb->clk);
err_enable:
	iounmap(tc6393xb->scr);
err_ioremap:
	release_resource(&tc6393xb->rscr);
err_request_scr:
	clk_put(tc6393xb->clk);
err_noirq:
err_clk_get:
	kfree(tc6393xb);
err_kzalloc:
	return ret;
}

static int __devexit tc6393xb_remove(struct platform_device *dev)
{
	struct tc6393xb_platform_data *tcpd = dev->dev.platform_data;
	struct tc6393xb *tc6393xb = platform_get_drvdata(dev);
	int ret;

	mfd_remove_devices(&dev->dev);

	if (tcpd->teardown)
		tcpd->teardown(dev);

	tc6393xb_detach_irq(dev);

	if (tc6393xb->gpio.base != -1) {
		ret = gpiochip_remove(&tc6393xb->gpio);
		if (ret) {
			dev_err(&dev->dev, "Can't remove gpio chip: %d\n", ret);
			return ret;
		}
	}

	ret = tcpd->disable(dev);
	clk_disable(tc6393xb->clk);
	iounmap(tc6393xb->scr);
	release_resource(&tc6393xb->rscr);
	platform_set_drvdata(dev, NULL);
	clk_put(tc6393xb->clk);
	kfree(tc6393xb);

	return ret;
}

#ifdef CONFIG_PM
static int tc6393xb_suspend(struct platform_device *dev, pm_message_t state)
{
	struct tc6393xb_platform_data *tcpd = dev->dev.platform_data;
	struct tc6393xb *tc6393xb = platform_get_drvdata(dev);
	int i, ret;

	tc6393xb->suspend_state.ccr = ioread16(tc6393xb->scr + SCR_CCR);
	tc6393xb->suspend_state.fer = ioread8(tc6393xb->scr + SCR_FER);

	for (i = 0; i < 3; i++) {
		tc6393xb->suspend_state.gpo_dsr[i] =
			ioread8(tc6393xb->scr + SCR_GPO_DSR(i));
		tc6393xb->suspend_state.gpo_doecr[i] =
			ioread8(tc6393xb->scr + SCR_GPO_DOECR(i));
		tc6393xb->suspend_state.gpi_bcr[i] =
			ioread8(tc6393xb->scr + SCR_GPI_BCR(i));
	}
	ret = tcpd->suspend(dev);
	clk_disable(tc6393xb->clk);

	return ret;
}

static int tc6393xb_resume(struct platform_device *dev)
{
	struct tc6393xb_platform_data *tcpd = dev->dev.platform_data;
	struct tc6393xb *tc6393xb = platform_get_drvdata(dev);
	int ret;
	int i;

	clk_enable(tc6393xb->clk);

	ret = tcpd->resume(dev);
	if (ret)
		return ret;

	if (!tcpd->resume_restore)
		return 0;

	iowrite8(tc6393xb->suspend_state.fer,	tc6393xb->scr + SCR_FER);
	iowrite16(tcpd->scr_pll2cr,		tc6393xb->scr + SCR_PLL2CR);
	iowrite16(tc6393xb->suspend_state.ccr,	tc6393xb->scr + SCR_CCR);
	iowrite16(SCR_MCR_RDY_OPENDRAIN | SCR_MCR_RDY_UNK | SCR_MCR_RDY_EN |
		  SCR_MCR_INT_OPENDRAIN | SCR_MCR_INT_UNK | SCR_MCR_INT_EN |
		  BIT(15),			tc6393xb->scr + SCR_MCR);
	iowrite16(tcpd->scr_gper,		tc6393xb->scr + SCR_GPER);
	iowrite8(0,				tc6393xb->scr + SCR_IRR);
	iowrite8(0xbf,				tc6393xb->scr + SCR_IMR);

	for (i = 0; i < 3; i++) {
		iowrite8(tc6393xb->suspend_state.gpo_dsr[i],
					tc6393xb->scr + SCR_GPO_DSR(i));
		iowrite8(tc6393xb->suspend_state.gpo_doecr[i],
					tc6393xb->scr + SCR_GPO_DOECR(i));
		iowrite8(tc6393xb->suspend_state.gpi_bcr[i],
					tc6393xb->scr + SCR_GPI_BCR(i));
	}

	return 0;
}
#else
#define tc6393xb_suspend NULL
#define tc6393xb_resume NULL
#endif

static struct platform_driver tc6393xb_driver = {
	.probe = tc6393xb_probe,
	.remove = __devexit_p(tc6393xb_remove),
	.suspend = tc6393xb_suspend,
	.resume = tc6393xb_resume,

	.driver = {
		.name = "tc6393xb",
		.owner = THIS_MODULE,
	},
};

static int __init tc6393xb_init(void)
{
	return platform_driver_register(&tc6393xb_driver);
}

static void __exit tc6393xb_exit(void)
{
	platform_driver_unregister(&tc6393xb_driver);
}

subsys_initcall(tc6393xb_init);
module_exit(tc6393xb_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Ian Molton, Dmitry Baryshkov and Dirk Opfer");
MODULE_DESCRIPTION("tc6393xb Toshiba Mobile IO Controller");
MODULE_ALIAS("platform:tc6393xb");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           /*
 * Copyright (C) 2004 Hollis Blanchard <hollisb@us.ibm.com>, IBM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/* Host Virtual Serial Interface (HVSI) is a protocol between the hosted OS
 * and the service processor on IBM pSeries servers. On these servers, there
 * are no serial ports under the OS's control, and sometimes there is no other
 * console available either. However, the service processor has two standard
 * serial ports, so this over-complicated protocol allows the OS to control
 * those ports by proxy.
 *
 * Besides data, the procotol supports the reading/writing of the serial
 * port's DTR line, and the reading of the CD line. This is to allow the OS to
 * control a modem attached to the service processor's serial port. Note that
 * the OS cannot change the speed of the port through this protocol.
 */

#undef DEBUG

#include <linux/console.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/major.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/sysrq.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <asm/hvcall.h>
#include <asm/hvconsole.h>
#include <asm/prom.h>
#include <asm/uaccess.h>
#include <asm/vio.h>
#include <asm/param.h>

#define HVSI_MAJOR	229
#define HVSI_MINOR	128
#define MAX_NR_HVSI_CONSOLES 4

#define HVSI_TIMEOUT (5*HZ)
#define HVSI_VERSION 1
#define HVSI_MAX_PACKET 256
#define HVSI_MAX_READ 16
#define HVSI_MAX_OUTGOING_DATA 12
#define N_OUTBUF 12

/*
 * we pass data via two 8-byte registers, so we would like our char arrays
 * properly aligned for those loads.
 */
#define __ALIGNED__	__attribute__((__aligned__(sizeof(long))))

struct hvsi_struct {
	struct delayed_work writer;
	struct work_struct handshaker;
	wait_queue_head_t emptyq; /* woken when outbuf is emptied */
	wait_queue_head_t stateq; /* woken when HVSI state changes */
	spinlock_t lock;
	int index;
	struct tty_struct *tty;
	int count;
	uint8_t throttle_buf[128];
	uint8_t outbuf[N_OUTBUF]; /* to implement write_room and chars_in_buffer */
	/* inbuf is for packet reassembly. leave a little room for leftovers. */
	uint8_t inbuf[HVSI_MAX_PACKET + HVSI_MAX_READ];
	uint8_t *inbuf_end;
	int n_throttle;
	int n_outbuf;
	uint32_t vtermno;
	uint32_t virq;
	atomic_t seqno; /* HVSI packet sequence number */
	uint16_t mctrl;
	uint8_t state;  /* HVSI protocol state */
	uint8_t flags;
#ifdef CONFIG_MAGIC_SYSRQ
	uint8_t sysrq;
#endif /* CONFIG_MAGIC_SYSRQ */
};
static struct hvsi_struct hvsi_ports[MAX_NR_HVSI_CONSOLES];

static struct tty_driver *hvsi_driver;
static int hvsi_count;
static int (*hvsi_wait)(struct hvsi_struct *hp, int state);

enum HVSI_PROTOCOL_STATE {
	HVSI_CLOSED,
	HVSI_WAIT_FOR_VER_RESPONSE,
	HVSI_WAIT_FOR_VER_QUERY,
	HVSI_OPEN,
	HVSI_WAIT_FOR_MCTRL_RESPONSE,
	HVSI_FSP_DIED,
};
#define HVSI_CONSOLE 0x1

#define VS_DATA_PACKET_HEADER           0xff
#define VS_CONTROL_PACKET_HEADER        0xfe
#define VS_QUERY_PACKET_HEADER          0xfd
#define VS_QUERY_RESPONSE_PACKET_HEADER 0xfc

/* control verbs */
#define VSV_SET_MODEM_CTL    1 /* to service processor only */
#define VSV_MODEM_CTL_UPDATE 2 /* from service processor only */
#define VSV_CLOSE_PROTOCOL   3

/* query verbs */
#define VSV_SEND_VERSION_NUMBER 1
#define VSV_SEND_MODEM_CTL_STATUS 2

/* yes, these masks are not consecutive. */
#define HVSI_TSDTR 0x01
#define HVSI_TSCD  0x20

struct hvsi_header {
	uint8_t  type;
	uint8_t  len;
	uint16_t seqno;
} __attribute__((packed));

struct hvsi_data {
	uint8_t  type;
	uint8_t  len;
	uint16_t seqno;
	uint8_t  data[HVSI_MAX_OUTGOING_DATA];
} __attribute__((packed));

struct hvsi_control {
	uint8_t  type;
	uint8_t  len;
	uint16_t seqno;
	uint16_t verb;
	/* optional depending on verb: */
	uint32_t word;
	uint32_t mask;
} __attribute__((packed));

struct hvsi_query {
	uint8_t  type;
	uint8_t  len;
	uint16_t seqno;
	uint16_t verb;
} __attribute__((packed));

struct hvsi_query_response {
	uint8_t  type;
	uint8_t  len;
	uint16_t seqno;
	uint16_t verb;
	uint16_t query_seqno;
	union {
		uint8_t  version;
		uint32_t mctrl_word;
	} u;
} __attribute__((packed));



static inline int is_console(struct hvsi_struct *hp)
{
	return hp->flags & HVSI_CONSOLE;
}

static inline int is_open(struct hvsi_struct *hp)
{
	/* if we're waiting for an mctrl then we're already open */
	return (hp->state == HVSI_OPEN)
			|| (hp->state == HVSI_WA
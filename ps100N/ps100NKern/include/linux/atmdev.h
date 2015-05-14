ables registered with
 * uwb_est_register(). For variable sized events, it will look further
 * ahead into their length field to see how much data should be read.
 *
 * Note this size is *not* final--the neh (Notification/Event Handle)
 * might specificy an extra size to add or replace.
 */
ssize_t uwb_est_find_size(struct uwb_rc *rc, const struct uwb_rceb *rceb,
			  size_t rceb_size)
{
	/* FIXME: add vendor/product data */
	ssize_t size;
	struct device *dev = &rc->uwb_dev.dev;
	unsigned long flags;
	unsigned itr;
	u16 type_event_high, event;
	u8 *ptr = (u8 *) rceb;

	read_lock_irqsave(&uwb_est_lock, flags);
	size = -ENOSPC;
	if (rceb_size < sizeof(*rceb))
		goto out;
	event = le16_to_cpu(rceb->wEvent);
	type_event_high = rceb->bEventType << 8 | (event & 0xff00) >> 8;
	for (itr = 0; itr < uwb_est_used; itr++) {
		if (uwb_est[itr].type_event_high != type_event_high)
			continue;
		size = uwb_est_get_size(rc, &uwb_est[itr],
					event & 0x00ff, rceb, rceb_size);
		/* try more tables that might handle the same type */
		if (size != -ENOENT)
			goto out;
	}
	dev_dbg(dev, "event 0x%02x/%04x/%02x: no handlers available; "
		"RCEB %02x %02x %02x %02x\n",
		(unsigned) rceb->bEventType,
		(unsigned) le16_to_cpu(rceb->wEvent),
		(unsigned) rceb->bEventContext,
		ptr[0], ptr[1], ptr[2], ptr[3]);
	size = -ENOENT;
out:
	read_unlock_irqrestore(&uwb_est_lock, flags);
	return size;
}
EXPORT_SYMBOL_GPL(uwb_est_find_size);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          #include <scsi/scsi.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <linux/usb.h>

#include "usb.h"
#include "transport.h"
#include "protocol.h"
#include "scsiglue.h"
#include "sierra_ms.h"
#include "debug.h"

#define SWIMS_USB_REQUEST_SetSwocMode	0x0B
#define SWIMS_USB_REQUEST_GetSwocInfo	0x0A
#define SWIMS_USB_INDEX_SetMode		0x0000
#define SWIMS_SET_MODE_Modem		0x0001

#define TRU_NORMAL 			0x01
#define TRU_FORCE_MS 			0x02
#define TRU_FORCE_MODEM 		0x03

static unsigned int swi_tru_install = 1;
module_param(swi_tru_install, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(swi_tru_install, "TRU-Install mode (1=Full Logic (def),"
		 " 2=Force CD-Rom, 3=Force Modem)");

struct swoc_info {
	__u8 rev;
	__u8 reserved[8];
	__u16 LinuxSKU;
	__u16 LinuxVer;
	__u8 reserved2[47];
} __attribute__((__packed__));

static bool containsFullLinuxPackage(struct swoc_info *swocInfo)
{
	if ((swocInfo->LinuxSKU >= 0x2100 && swocInfo->LinuxSKU <= 0x2FFF) ||
	   (swocInfo->LinuxSKU >= 0x7100 && swocInfo->LinuxSKU <= 0x7FFF))
		return true;
	else
		return false;
}

static int sierra_set_ms_mode(struct usb_device *udev, __u16 eSWocMode)
{
	int result;
	US_DEBUGP("SWIMS: %s", "DEVICE MODE SWITCH\n");
	result = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			SWIMS_USB_REQUEST_SetSwocMode,	/* __u8 request      */
			USB_TYPE_VENDOR | USB_DIR_OUT,	/* __u8 request type */
			eSWocMode,			/* __u16 value       */
			0x0000,				/* __u16 index       */
			NULL,				/* void *data        */
			0,				/* __u16 size 	     */
			USB_CTRL_SET_TIMEOUT);		/* int timeout       */
	return result;
}


static int sierra_get_swoc_info(struct usb_device *udev,
				struct swoc_info *swocInfo)
{
	int result;

	US_DEBUGP("SWIMS: Attempting to get TRU-Install info.\n");

	result = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			SWIMS_USB_REQUEST_GetSwocInfo,	/* __u8 request      */
			USB_TYPE_VENDOR | USB_DIR_IN,	/* __u8 request type */
			0,				/* __u16 value       */
			0,				/* __u16 index       */
			(void *) swocInfo,		/* void *data        */
			sizeof(struct swoc_info),	/* __u16 size 	     */
			USB_CTRL_SET_TIMEOUT);		/* int timeout 	     */

	swocInfo->LinuxSKU = le16_to_cpu(swocInfo->LinuxSKU);
	swocInfo->LinuxVer = le16_to_cpu(swocInfo->LinuxVer);
	return result;
}

static void debug_swoc(struct swoc_info *swocInfo)
{
	US_DEBUGP("SWIMS: SWoC Rev: %02d \n", swocInfo->rev);
	US_DEBUGP("SWIMS: Linux SKU: %04X \n", swocInfo->LinuxSKU);
	US_DEBUGP("SWIMS: Linux Version: %04X \n", swocInfo->LinuxVer);
}


static ssize_t show_truinst(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct swoc_info *swocInfo;
	struct usb_interface *intf = to_usb_interface(dev);
	struct usb_device *udev = interface_to_usbdev(intf);
	int result;
	if (swi_tru_install == TRU_FORCE_MS) {
		result = snprintf(buf, PAGE_SIZE, "Forced Mass Storage\n");
	} else {
		swocInfo = kmalloc(sizeof(struct swoc_info), GFP_KERNEL);
		if (!swocInfo) {
			US_DEBUGP("SWIMS: Allocation failure\n");
			snprintf(buf, PAGE_SIZE, "Error\n");
			return -ENOMEM;
		}
		result = sierra_get_swoc_info(udev, swocInfo);
		if (result < 0) {
			US_DEBUGP("SWIMS: failed SWoC query\n");
			kfree(swocInfo);
			snprintf(buf, PAGE_SIZE, "Error\n");
			return -EIO;
		}
		debug_swoc(swocInfo);
		result = snprintf(buf, PAGE_SIZE,
			"REV=%02d SKU=%04X VER=%04X\n",
			swocInfo->rev,
			swocInfo->LinuxSKU,
			swocInfo->LinuxVer);
		kfree(swocInfo);
	}
	return result;
}
static DEVICE_ATTR(truinst, S_IWUGO | S_IRUGO, show_truinst, NULL);

int sierra_ms_init(struct us_data *us)
{
	int result, retries;
	signed long delay_t;
	struct swoc_info *swocInfo;
	struct usb_device *udev;
	struct Scsi_Host *sh;
	struct scsi_device *sd;

	delay_t = 2;
	retries = 3;
	result = 0;
	udev = us->pusb_dev;

	sh = us_to_host(us);
	sd = scsi_get_host_dev(sh);

	US_DEBUGP("SWIMS: sierra_ms_init called\n");

	/* Force Modem mode */
	if (swi_tru_install == TRU_FORCE_MODEM) {
		US_DEBUGP("SWIMS: %s", "Forcing Modem Mode\n");
		result = sierra_set_ms_mode(udev, SWIMS_SET_MODE_Modem);
		if (result < 0)
			US_DEBUGP("SWIMS: Failed to switch to modem mode.\n");
		return -EIO;
	}
	/* Force Mass Storage mode (keep CD-Rom) */
	else if (swi_tru_install == TRU_FORCE_MS) {
		US_DEBUGP("SWIMS: %s", "Forcing Mass Storage Mode\n");
		goto complete;
	}
	/* Normal TRU-Install Logic */
	else {
		US_DEBUGP("SWIMS: %s", "Normal SWoC Logic\n");

		swocInfo = kmalloc(sizeof(struct swoc_info),
				GFP_KERNEL);
		if (!swocInfo) {
			US_DEBUGP("SWIMS: %s", "Allocation failure\n");
			return -ENOMEM;
		}

		retries = 3;
		do {
			retries--;
			result = sierra_get_swoc_info(udev, swocInfo);
			if (result < 0) {
				US_DEBUGP("SWIMS: %s", "Failed SWoC query\n");
				schedule_timeout_uninterruptible(2*HZ);
			}
		} while (retries && result < 0);

		if (result < 0) {
			US_DEBUGP("SWIMS: %s",
				  "Completely failed SWoC query\n");
			kfree(swocInfo);
			return -EIO;
		}

		debug_swoc(swocInfo);

		/* If there is not Linux software on the TRU-Install device
		 * then switch to modem mode
		 */
		if (!containsFullLinuxPackage(swocInfo)) {
			US_DEBUGP("SWIMS: %s",
				"Switching to Modem Mode\n");
			result = sierra_set_ms_mode(udev,
				SWIMS_SET_MODE_Modem);
			if (result < 0)
				US_DEBUGP("SWIMS: Failed to switch modem\n");
			kfree(swocInfo);
			return -EIO;
		}
		kfree(swocInfo);
	}
complete:
	result = device_create_file(&us->pusb_intf->dev, &dev_attr_truinst);

	return 0;
}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 /*
 * timbuart.c timberdale FPGA UART driver
 * Copyright (c) 2009 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Supports:
 * Timberdale FPGA UART
 */

#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/serial_core.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>

#include "timbuart.h"

struct timbuart_port {
	struct uart_port	port;
	struct tasklet_struct	tasklet;
	int			usedma;
	u32			last_ier;
	struct platform_device  *dev;
};

static int baudrates[] = {9600, 19200, 38400, 57600, 115200, 230400, 460800,
	921600, 1843200, 3250000};

static void timbuart_mctrl_check(struct uart_port *port, u32 isr, u32 *ier);

static irqreturn_t timbuart_handleinterrupt(int irq, void *devid);

static void timbuart_stop_rx(struct uart_port *port)
{
	/* spin lock held by upper layer, disable all RX interrupts */
	u32 ier = ioread32(port->membase + TIMBUART_IER) & ~RXFLAGS;
	iowrite32(ier, port->membase + TIMBUART_IER);
}

static void timbuart_stop_tx(struct uart_port *port)
{
	/* spinlock held by upper layer, disable TX interrupt */
	u32 ier = ioread32(port->membase + TIMBUART_IER) & ~TXBAE;
	iowrite32(ier, port->membase + TIMBUART_IER);
}

static void timbuart_start_tx(struct uart_port *port)
{
	struct timbuart_port *uart =
		container_of(port, struct timbuart_port, port);

	/* do not transfer anything here -> fire off the tasklet */
	tasklet_schedule(&uart->tasklet);
}

static void timbuart_flush_buffer(struct uart_port *port)
{
	u8 ctl = ioread8(port->membase + TIMBUART_CTRL) | TIMBUART_CTRL_FLSHTX;

	iowrite8(ctl, port->membase + TIMBUART_CTRL);
	iowrite32(TXBF, port->membase + TIMBUART_ISR);
}

static void timbuart_rx_chars(struct uart_port *port)
{
	struct tty_struct *tty = port->info->port.tty;

	while (ioread32(port->membase + TIMBUART_ISR) & RXDP) {
		u8 ch = ioread8(port->membase + TIMBUART_RXFIFO);
		port->icount.rx++;
		tty_insert_flip_char(tty, ch, TTY_NORMAL);
	}

	spin_unlock(&port->lock);
	tty_flip_buffer_push(port->info->port.tty);
	spin_lock(&port->lock);

	dev_dbg(port->dev, "%s - total read %d bytes\n",
		__func__, port->icount.rx);
}

static void timbuart_tx_chars(struct uart_port *port)
{
	struct circ_buf *xmit = &port->info->xmit;

	while (!(ioread32(port->membase + TIMBUART_ISR) & TXBF) &&
		!uart_circ_empty(xmit)) {
		iowrite8(xmit->buf[xmit->tail],
			port->membase + TIMBUART_TXFIFO);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
	}

	dev_dbg(port->dev,
		"%s - total written %d bytes, CTL: %x, RTS: %x, baud: %x\n",
		 __func__,
		port->icount.tx,
		ioread8(port->membase + TIMBUART_CTRL),
		port->mctrl & TIOCM_RTS,
		ioread8(port->membase + TIMBUART_BAUDRATE));
}

static void timbuart_handle_tx_port(struct uart_port *port, u32 isr, u32 *ier)
{
	struct timbuart_port *uart =
		container_of(port, struct timbuart_port, port);
	struct circ_buf *xmit = &port->info->xmit;

	if (uart_circ_empty(xmit) || uart_tx_stopped(port))
		return;

	if (port->x_char)
		return;

	if (isr & TXFLAGS) {
		timbuart_tx_chars(port);
		/* clear all TX interrupts */
		iowrite32(TXFLAGS, port->membase + TIMBUART_ISR);

		if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
			uart_write_wakeup(port);
	} else
		/* Re-enable any tx interrupt */
		*ier |= uart->last_ier & TXFLAGS;

	/* enable interrupts if there are chars in the transmit buffer,
	 * Or if we delivered some bytes and want the almost empty interrupt
	 * we wake up the upper layer later when we got the interrupt
	 * to give it some time to go out...
	 */
	if (!uart_circ_empty(xmit))
		*ier |= TXBAE;

	dev_dbg(port->dev, "%s - leaving\n", __func__);
}

void timbuart_handle_rx_port(struct uart_port *port, u32 isr, u32 *ier)
{
	if (isr & RXFLAGS) {
		/* Some RX status is set */
		if (isr & 
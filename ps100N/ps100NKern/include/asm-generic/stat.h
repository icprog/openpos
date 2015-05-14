/*
 *  PS3 OHCI Host Controller driver
 *
 *  Copyright (C) 2006 Sony Computer Entertainment Inc.
 *  Copyright 2006 Sony Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <asm/firmware.h>
#include <asm/ps3.h>

static int ps3_ohci_hc_reset(struct usb_hcd *hcd)
{
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);

	ohci->flags |= OHCI_QUIRK_BE_MMIO;
	ohci_hcd_init(ohci);
	return ohci_init(ohci);
}

static int __devinit ps3_ohci_hc_start(struct usb_hcd *hcd)
{
	int result;
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);

	/* Handle root hub init quirk in spider south bridge. */
	/* Also set PwrOn2PwrGood to 0x7f (254ms). */

	ohci_writel(ohci, 0x7f000000 | RH_A_PSM | RH_A_OCPM,
		&ohci->regs->roothub.a);
	ohci_writel(ohci, 0x00060000, &ohci->regs->roothub.b);

	result = ohci_run(ohci);

	if (result < 0) {
		err("can't start %s", hcd->self.bus_name);
		ohci_stop(hcd);
	}

	return result;
}

static const struct hc_driver ps3_ohci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "PS3 OHCI Host Controller",
	.hcd_priv_size		= sizeof(struct ohci_hcd),
	.irq			= ohci_irq,
	.flags			= HCD_MEMORY | HCD_USB11,
	.reset			= ps3_ohci_hc_reset,
	.start			= ps3_ohci_hc_start,
	.stop			= ohci_stop,
	.shutdown		= ohci_shutdown,
	.urb_enqueue		= ohci_urb_enqueue,
	.urb_dequeue		= ohci_urb_dequeue,
	.endpoint_disable	= ohci_endpoint_disable,
	.get_frame_number	= ohci_get_frame,
	.hub_status_data	= ohci_hub_status_data,
	.hub_control		= ohci_hub_control,
	.start_port_reset	= ohci_start_port_reset,
#if defined(CONFIG_PM)
	.bus_suspend 		= ohci_bus_suspend,
	.bus_resume 		= ohci_bus_resume,
#endif
};

static int __devinit ps3_ohci_probe(struct ps3_system_bus_device *dev)
{
	int result;
	struct usb_hcd *hcd;
	unsigned int virq;
	static u64 dummy_mask = DMA_BIT_MASK(32);

	if (usb_disabled()) {
		result = -ENODEV;
		goto fail_start;
	}

	result = ps3_open_hv_device(dev);

	if (resu
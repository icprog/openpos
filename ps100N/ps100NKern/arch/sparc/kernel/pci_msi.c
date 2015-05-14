/*
 * UWB PAL support.
 *
 * Copyright (C) 2008 Cambridge Silicon Radio Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/uwb.h>

#include "uwb-internal.h"

/**
 * uwb_pal_init - initialize a UWB PAL
 * @pal: the PAL to initialize
 */
void uwb_pal_init(struct uwb_pal *pal)
{
	INIT_LIST_HEAD(&pal->node);
}
EXPORT_SYMBOL_GPL(uwb_pal_init);

/**
 * uwb_pal_register - register a UWB PAL
 * @pal: the PAL
 *
 * The PAL must be initialized with uwb_pal_init().
 */
int uwb_pal_register(struct uwb_pal *pal)
{
	struct uwb_rc *rc = pal->rc;
	int ret;

	if (pal->device) {
		ret = sysfs_create_link(&pal->device->kobj,
					&rc->uwb_dev.dev.kobj, "uwb_rc");
		if (ret < 0)
			return ret;
		ret = sysfs_create_link(&rc->uwb_dev.dev.kobj,
					&pal->device->kobj, pal->name);
		if (ret < 0) {
			sysfs_remove_link(&pal->device->kobj, "uwb_rc");
			return ret;
		}
	}

	pal->debugfs_dir = uwb_dbg_create_pal_dir(pal);

	mutex_lock(&rc->uwb_dev.mutex);
	list_add(&pal->node, &rc->pals);
	mutex_unlock(&rc->uwb_dev.mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(uwb_pal_register);

/**
 * uwb_pal_register - unregister a UWB PAL
 * @pal: the PAL
 */
void uwb_pal_unregister(struct uwb_pal *pal)
{
	struct uwb_rc *rc = pal->rc;

	uwb_radio_stop(pal);

	mutex_lock(&rc->uwb_dev.mutex);
	list_del(&pal->node);
	mutex_unlock(&rc->uwb_dev.mutex);

	debugfs_remove(pal->debugfs_dir);

	if (pal->device) {
		sysfs_remove_link(&rc->uwb_dev.dev.kobj, pal->name);
		sysfs_remove_link(&pal->device->kobj, "uwb_rc");
	}
}
EXPORT_SYMBOL_GPL(uwb_pal_unregister);

/**
 * uwb_rc_pal_init - initialize the PAL related parts of a radio controller
 * @rc: the radio controller
 */
void uwb_rc_pal_init(struct uwb_rc *rc)
{
	INIT_LIST_HEAD(&rc->pals);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       /*
 * Copyright (c) 2001-2002 by David Brownell
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __LINUX_EHCI_HCD_H
#define __LINUX_EHCI_HCD_H

/* definitions used for the EHCI driver */

/*
 * __hc32 and __hc16 are "Host Controller" types, they may be equivalent to
 * __leXX (normally) or __beXX (given EHCI_BIG_ENDIAN_DESC), depending on
 * the host controller implementation.
 *
 * To facilitate the strongest possible byte-order checking from "sparse"
 * and so on, we use __leXX unless that's not practical.
 */
#ifdef CONFIG_USB_EHCI_BIG_ENDIAN_DESC
typedef __u32 __bitwise __hc32;
typedef __u16 __bitwise __hc16;
#else
#define __hc32	__le32
#define __hc16	__le16
#endif

/* statistics can be kept for for tuning/monitoring */
struct ehci_stats {
	/* irq usage */
	unsigned long		normal;
	unsigned long		error;
	unsigned long		reclaim;
	unsigned long		lost_iaa;

	/* termination of urbs from core */
	unsigned long		complete;
	unsigned long		unlink;
};

/* ehci_hcd->lock guards shared data against other CPUs:
 *   ehci_hcd:	async, reclaim, periodic (and shadow), ...
 *   usb_host_endpoint: hcpriv
 *   ehci_qh:	qh_next, qtd_list
 *   ehci_qtd:	qtd_list
 *
 * Also, hold this lock when talking to HC registers or
 * when updating hw_* fields in shared qh/qtd/... structures.
 */

#define	EHCI_MAX_ROOT_PORTS	15		/* see HCS_N_PORTS */

struct ehci_hcd {			/* one per controller */
	/* glue to PCI and HCD framework */
	struct ehci_caps __iomem *caps;
	struct ehci_regs __iomem *regs;
	struct ehci_dbg_port __iomem *debug;

	__u32			hcs_params;	/* cached register copy */
	spinlock_t		lock;

	/* async schedule support */
	struct ehci_qh		*async;
	struct ehci_qh		*reclaim;
	unsigned		scanning : 1;

	/* periodic schedule support */
#define	DEFAULT_I_TDPS		1024		/* some HCs can do less */
	unsigned		periodic_size;
	__hc32			*periodic;	/* hw periodic table */
	dma_addr_t		periodic_dma;
	unsigned		i_thresh;	/* uframes HC might cache */

	union ehci_shadow	*pshadow;	/* mirror hw periodic table */
	int			next_uframe;	/* scan periodic, start here */
	unsigned		periodic_sched;	/* periodic activity count */

	/* list of itds completed while clock_frame was still active */
	struct list_head	cached_itd_list;
	unsigned		clock_frame;

	/* per root hub port */
	unsigned long		reset_done [EHCI_MAX_ROOT_PORTS];

	/* bit vectors (one bit per port) */
	unsigned long		bus_suspended;		/* which ports were
			already suspended at the start of a bus suspend */
	unsigned long		companion_ports;	/* which ports are
			dedicated to the companion controller */
	unsigned long		owned_ports;		/* which ports are
			owned by the companion during a bus suspend */
	unsigned long		port_c_suspend;		/* which ports have
			the change-suspend feature turned on */
	unsigned long		suspended_ports;	/* which ports are
			suspended */

	/* per-HC memory pools (could be per-bus, but ...) */
	struct dma_pool		*qh_pool;	/* qh per active urb */
	struct dma_pool		*qtd_pool;	/* one or more per qh */
	struct dma_pool		*itd_pool;	/* itd per iso urb */
	struct dma_pool		*sitd_pool;	/* sitd per split iso urb */

	struct timer_list	iaa_watchdog;
	struct timer_list	watchdog;
	unsigned long		actions;
	unsigned		stamp;
	unsigned		random_frame;
	unsigned long		next_statechange;
	u32			command;

	/* SILICON QUIRKS */
	unsigned		no_selective_suspend:1;
	unsigned		has_fsl_port_bug:1; /* FreeScale */
	unsigned		big_endian_mmio:1;
	unsigned		big_endian_desc:1;
	unsigned		has_amcc_usb23:1;

	/* required for usb32 quirk */
	#define OHCI_CTRL_HCFS          (3 << 6)
	#define OHCI_USB_OPER           (2 << 6)
	#define OHCI_USB_SUSPEND        (3 << 6)

	#define OHCI_HCCTRL_OFFSET      0x4
	#define OHCI_HCCTRL_LEN         0x4
	__hc32			*ohci_hcctrl_reg;

	u8			sbrn;		/* packed release number */

	/*
	 * OTG controllers and transceivers need software interaction;
	 * other external transceivers should be software-transparent
	 */
	struct otg_transceiver   *transceiver;
#ifdef CONFIG_USB_STATIC_IRAM
	u32			iram_buffer[2];
	u32			iram_buffer_v[2];
	int  			iram_in_use[2];
	int			usb_address[2];
#endif

	/* irq statistics */
#ifdef EHCI_STATS
	struct ehci_stats	stats;
#	define COUNT(x) do { (x)++; } while (0)
#else
#	define COUNT(x) do {} while (0)
#endif

	/* debug files */
#ifdef DEBUG
	struct dentry		*debug_dir;
	struct dentry		*debug_async;
	struct dentry		*debug_periodic;
	struct dentry		*debug_registers;
#endif
};

/* convert between an HCD pointer and the corresponding EHCI_HCD */
static inline struct ehci_hcd *hcd_to_ehci (struct usb_hcd *hcd)
{
	return (struct ehci_hcd *) (hcd->hcd_priv);
}
static inline struct usb_hcd *ehci_to_hcd (struct ehci_hcd *ehci)
{
	return container_of ((void *) ehci, struct usb_hcd, hcd_priv);
}


static inline void
iaa_watchdog_start(struct ehci_hcd *ehci)
{
	WARN_ON(timer_pending(&ehci->iaa_watchdog));
	mod_timer(&ehci->iaa_watchdog,
			jiffies + msecs_to_jiffies(EHCI_IAA_MSECS));
}

static inline void iaa_watchdog_done(struct ehci_hcd *ehci)
{
	del_timer(&ehci->iaa_watchdog);
}

enum ehci_timer_action {
	TIMER_IO_WATCHDOG,
	TIMER_ASYNC_SHRINK,
	TIMER_ASYNC_OFF,
};

static inline void
timer_action_done (struct ehci_hcd *ehci, enum ehci_timer_
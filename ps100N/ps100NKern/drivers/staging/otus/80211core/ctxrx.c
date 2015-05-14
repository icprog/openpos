N | CSR0_INEA | CSR0_STRT;
}


/* This is needed for old RieblCards and possible for new RieblCards */

static int lance_set_mac_address( struct net_device *dev, void *addr )
{
	struct lance_private *lp = netdev_priv(dev);
	struct sockaddr *saddr = addr;
	int i;

	if (lp->cardtype != OLD_RIEBL && lp->cardtype != NEW_RIEBL)
		return( -EOPNOTSUPP );

	if (netif_running(dev)) {
		/* Only possible while card isn't started */
		DPRINTK( 1, ( "%s: hwaddr can be set only while card isn't open.\n",
					  dev->name ));
		return( -EIO );
	}

	memcpy( dev->dev_addr, saddr->sa_data, dev->addr_len );
	for( i = 0; i < 6; i++ )
		MEM->init.hwaddr[i] = dev->dev_addr[i^1]; /* <- 16 bit swap! */
	lp->memcpy_f( RIEBL_HWADDR_ADDR, dev->dev_addr, 6 );
	/* set also the magic for future sessions */
	*RIEBL_MAGIC_ADDR = RIEBL_MAGIC;

	return( 0 );
}


#ifdef MODULE
static struct net_device *atarilance_dev;

static int __init atarilance_module_init(void)
{
	atarilance_dev = atarilance_probe(-1);
	if (IS_ERR(atarilance_dev))
		return PTR_ERR(atarilance_dev);
	return 0;
}

static void __exit atarilance_module_exit(void)
{
	unregister_netdev(atarilance_dev);
	free_irq(atarilance_dev->irq, atarilance_dev);
	free_netdev(atarilance_dev);
}
module_init(atarilance_module_init);
module_exit(atarilance_module_exit);
#endif /* MODULE */


/*
 * Local variables:
 *  c-indent-level: 4
 *  tab-width: 4
 * End:
 */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        /*******************************************************************************
 *
 * Module Name: utmath - Integer math support routines
 *
 ******************************************************************************/

/*
 * Copyright (C) 2000 - 2008, Intel Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 */

#include <acpi/acpi.h>
#include "accommon.h"

#define _COMPONENT          ACPI_UTILITIES
ACPI_MODULE_NAME("utmath")

/*
 * Support for double-precision integer divide.  This code is included here
 * in order to support kernel environments where the double-precision math
 * library is not available.
 */
#ifndef ACPI_USE_NATIVE_DIVIDE
/*******************************************************************************
 *
 * FUNCTION:    acpi_ut_short_divide
 *
 * PARAMETERS:  Dividend            - 64-bit dividend
 *              Divisor             - 32-bit divisor
 *              out_quotient        - Pointer to where the quotient is returned
 *              out_remainder       - Pointer to where the remainder is returned
 *
 * RETURN:      Status (Checks for divide-by-zero)
 *
 * DESCRIPTION: Perform a short (maximum 64 bits divided by 32 bits)
 *              divide and modulo.  The result is a 64-bit quotient and a
 *              32-bit remainder.
 *
 ******************************************************************************/
acpi_status
acpi_ut_short_divide(acpi_integer dividend,
		     u32 divisor,
		     acpi_integer * out_quotient, u32 * out_remainder)
{
	union uint64_overlay dividend_ovl;
	union uint64_overlay quotient;
	u32 remainder32;

	ACPI_FUNCTION_TRACE(ut_short_divide);

	/* Always check for a zero divisor */

	if (divisor == 0) {
		ACPI_ERROR((AE_INFO, "Divide by zero"));
		return_ACPI_STATUS(AE_AML_DIVIDE_BY_ZERO);
	}

	dividend_ovl.full = dividend;

	/*
	 * The quotient is 64 bits, the remainder is always 32 bits,
	 * and is generated by the second divide.
	 */
	ACPI_DIV_64_BY_32(0, dividend_ovl.part.hi, divisor,
			  quotient.part.hi, remainder32);
	ACPI_DIV_64_BY_32(remainder32, dividend_ovl.part.lo, divisor,
			  quotient.part.lo, remainder32);

	/* Return only what was requested */

	if (out_quotient) {
		*out_quotient = quotient.full;
	}
	if (out_remainder) {
		*out_remainder = remainder32;
	}

	return_ACPI_STATUS(AE_OK);
}

/*******************************************************************************
 *
 * FUNCTION:    acpi_ut_divide
 *
 * PARAMETERS:  in_dividend         - Dividend
 *              in_divisor          - Divisor
 *              out_quotient        - Pointer to where the quotient is returned
 *              out_remainder       - Pointer to where the remainder is returned
 *
 * RETURN:      Status (Checks for divide-by-zero)
 *
 * DESCRIPTION: Perform a divide and modulo.
 *
 ******************************************************************************/

acpi_status
acpi_ut_divide(acpi_integer in_dividend,
	       acpi_integer in_divisor,
	       acpi_integer * out_quotient, acpi_integer * out_remainder)
{
	union uint64_overlay dividend;
	union uint64_overlay divisor;
	union uint64_overlay quotient;
	union uint64_overlay remainder;
	union uint64_overlay normalized_dividend;
	union uint64_overlay normalized_divisor;
	u32 partial1;
	union uint64_overlay partial2;
	union uint64_overlay partial3;

	ACPI_FUNCTION_TRACE(ut_divide);

	/* Always check for a zero divisor */

	if (in_divisor == 0) {
		ACPI_ERROR((AE_INFO, "Divide by zero"));
		return_ACPI_STATUS(AE_AML_DIVIDE_BY_ZERO);
	}

	divisor.full = in_divisor;
	dividend.full = in_dividend;
	if (divisor.part.hi == 0) {
		/*
		 * 1) Simplest case is where the divisor is 32 bits, we can
		 * just do two divides
		 */
		remainder.part.hi = 0;

		/*
		 * The quotient is 64 bits, the remainder is always 32 bits,
		 * and is generated by the second divide.
		 */
		ACPI_DIV_64_BY_32(0, dividend.part.hi, divisor.part.lo,
				  quotient.part.hi, partial1);
		ACPI_DIV_64_BY_32(partial1, dividend.part.lo, divisor.part.lo,
				  quotient.part.lo, remainder.part.lo);
	}

	else {
		/*
		 * 2) The general case where the divisor is a full 64 bits
		 * is more difficult
		 */
		quotient.part.hi = 0;
		normalized_dividend = dividend;
		normalized_divisor = divisor;

		/* Normalize the operands (shift until the divisor is < 32 bits) */

		do {
			ACPI_SHIFT_RIGHT_64(normalized_divisor.part.hi,
					    normalized_divisor.part.lo);
			ACPI_SHIFT_RIGHT_64(normalized_dividend.part.hi,
					    normalized_dividend.part.lo);

		} while (normalized_divisor.part.hi != 0);

		/* Partial divide */

		ACPI_DIV_64_BY_32(normalized_dividend.part.hi,
				  normalized_dividend.part.lo,
				  normalized_divisor.part.lo,
				  quotient.part.lo, partial1);

		/*
		 * The quotient is always 32 bits, and simply requires adjustment.
		 * The 64-bit remainder must be generated.
		 */
		partial1 = quotient.part.lo * divisor.part.hi;
		partial2.full =
		    (acpi_integer) quotient.part.lo * divisor.part.lo;
		partial3.full = (acpi_integer) partial2.part.hi + partial1;

		remainder.part.hi = partial3.part.lo;
		remainder.part.lo = partial2.part.lo;

		if (partial3.part.hi == 0) {
			if (partial3.part.lo >= dividend.part.hi) {
				if (partial3.part.lo == dividend.part.hi) {
					if (partial2.part.lo > dividend.part.lo) {
						quotient.part.lo--;
						remainder.full -= divisor.full;
					}
				} else {
					quotient.part.lo--;
					remainder.full -= divisor.full;
				}
			}

			remainder.full = remainder.full - dividend.full;
			remainder.part.hi = (u32) - ((s32) remainder.part.hi);
			remainder.part.lo = (u32) - ((s32) remainder.part.lo);

			if (remainder.part.lo) {
				remainder.part.hi--;
			}
		}
	}

	/* Return only what was requested */

	if (out_quotient) {
		*out_quotient = quotient.full;
	}
	if (out_remainder) {
		*out_remainder = remainder.full;
	}

	return_ACPI_STATUS(AE_OK);
}

#else
/*******************************************************************************
 *
 * FUNCTION:    acpi_ut_short_divide, acpi_ut_divide
 *
 * PARAMETERS:  See function headers above
 *
 * DESCRIPTION: Native versions of the ut_divide functions. Use these if either
 *              1) The target is a 64-bit platform and therefore 64-bit
 *                 integer math is supported directly by the machine.
 *              2) The target is a 32-bit or 16-bit platform, and the
 *                 double-precision integer math library is available to
 *                 perform the divide.
 *
 ******************************************************************************/
acpi_status
acpi_ut_short_divide(acpi_integer in_dividend,
		     u32 divisor,
		     acpi_integer * out_quotient, u32 * out_remainder)
{

	ACPI_FUNCTION_TRACE(ut_short_divide);

	/* Always check for a zero divisor */

	if (divisor == 0) {
		ACPI_ERROR((AE_INFO, "Divide by zero"));
		return_ACPI_STATUS(AE_AML_DIVIDE_BY_ZERO);
	}

	/* Return only what was requested */

	if (out_quotient) {
		*out_quotient = in_dividend / divisor;
	}
	if (out_remainder) {
		*out_remainder = (u32) (in_dividend % divisor);
	}

	return_ACPI_STATUS(AE_OK);
}

acpi_status
acpi_ut_divide(acpi_integer in_dividend,
	       acpi_integer in_divisor,
	       acpi_integer * out_quotient, acpi_integer * out_remainder)
{
	ACPI_FUNCTION_TRACE(ut_divide);

	/* Always check for a zero divisor */

	if (in_divisor == 0) {
		ACPI_ERROR((AE_INFO, "Divide by zero"));
		return_ACPI_STATUS(AE_AML_DIVIDE_BY_ZERO);
	}

	/* Return only what was requested */

	if (out_quotient) {
		*out_quotient = in_dividend / in_divisor;
	}
	if (out_remainder) {
		*out_remainder = in_dividend % in_divisor;
	}

	return_ACPI_STATUS(AE_OK);
}

#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             EXTRA_CFLAGS += -DIPATH_IDSTR='"QLogic kernel.org driver"' \
	-DIPATH_KERN_TYPE=0

obj-$(CONFIG_INFINIBAND_IPATH) += ib_ipath.o

ib_ipath-y := \
	ipath_cq.o \
	ipath_diag.o \
	ipath_dma.o \
	ipath_driver.o \
	ipath_eeprom.o \
	ipath_file_ops.o \
	ipath_fs.o \
	ipath_init_chip.o \
	ipath_intr.o \
	ipath_keys.o \
	ipath_mad.o \
	ipath_mmap.o \
	ipath_mr.o \
	ipath_qp.o \
	ipath_rc.o \
	ipath_ruc.o \
	ipath_sdma.o \
	ipath_srq.o \
	ipath_stats.o \
	ipath_sysfs.o \
	ipath_uc.o \
	ipath_ud.o \
	ipath_user_pages.o \
	ipath_user_sdma.o \
	ipath_verbs_mcast.o \
	ipath_verbs.o \
	ipath_iba7220.o \
	ipath_sd7220.o \
	ipath_sd7220_img.o

ib_ipath-$(CONFIG_HT_IRQ) += ipath_iba6110.o
ib_ipath-$(CONFIG_PCI_MSI) += ipath_iba6120.o

ib_ipath-$(CONFIG_X86_64) += ipath_wc_x86_64.o
ib_ipath-$(CONFIG_PPC64) += ipath_wc_ppc64.o
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             /*
 * Generic IEEE 1394 definitions
 */

#ifndef _IEEE1394_IEEE1394_H
#define _IEEE1394_IEEE1394_H

#define TCODE_WRITEQ		0x0
#define TCODE_WRITEB		0x1
#define TCODE_WRITE_RESPONSE	0x2
#define TCODE_READQ		0x4
#define TCODE_READB		0x5
#define TCODE_READQ_RESPONSE	0x6
#define TCODE_READB_RESPONSE	0x7
#define TCODE_CYCLE_START	0x8
#define TCODE_LOCK_REQUEST	0x9
#define TCODE_ISO_DATA		0xa
#define TCODE_STREAM_DATA	0xa
#define TCODE_LOCK_RESPONSE	0xb

#define RCODE_COMPLETE		0x0
#define RCODE_CONFLICT_ERROR	0x4
#define RCODE_DATA_ERROR	0x5
#define RCODE_TYPE_ERROR	0x6
#define RCODE_ADDRESS_ERROR	0x7

#define EXTCODE_MASK_SWAP	0x1
#define EXTCODE_COMPARE_SWAP	0x2
#define EXTCODE_FETCH_ADD	0x3
#define EXTCODE_LITTLE_ADD	0x4
#define EXTCODE_BOUNDED_ADD	0x5
#define EXTCODE_WRAP_ADD	0x6

#define ACK_COMPLETE		0x1
#define ACK_PENDING		0x2
#define ACK_BUSY_X		0x4
#define ACK_BUSY_A		0x5
#define ACK_BUSY_B		0x6
#define ACK_TARDY		0xb
#define ACK_CONFLICT_ERROR	0xc
#define ACK_DATA_ERROR		0xd
#define ACK_TYPE_ERROR		0xe
#define ACK_ADDRESS_ERROR	0xf

/* Non-standard "ACK codes" for internal use */
#define ACKX_NONE		(-1)
#define ACKX_SEND_ERROR		(-2)
#define ACKX_ABORTED		(-3)
#define ACKX_TIMEOUT		(-4)

#define IEEE1394_SPEED_100	0x00
#define IEEE1394_SPEED_200	0x01
#define IEEE1394_SPEED_400	0x02
#define IEEE1394_SPEED_800	0x03
#define IEEE1394_SPEED_1600	0x04
#define IEEE1394_SPEED_3200	0x05
#define IEEE1394_SPEED_MAX	IEEE1394_SPEED_3200

/* Maps speed values above to a string representation */
extern const char *hpsb_speedto_str[];

/* 1394a cable PHY packets */
#define SELFID_PWRCL_NO_POWER		0x0
#define SELFID_PWRCL_PROVIDE_15W	0x1
#define SELFID_PWRCL_PROVIDE_30W	0x2
#define SELFID_PWRCL_PROVIDE_45W	0x3
#define SELFID_PWRCL_USE_1W		0x4
#define SELFID_PWRCL_USE_3W		0x5
#define SELFID_PWRCL_USE_6W		0x6
#define SELFID_PWRCL_USE_10W		0x7

#define SELFID_PORT_CHILD		0x3
#define SELFID_PORT_PARENT		0x2
#define SELFID_PORT_NCONN		0x1
#define SELFID_PORT_NONE		0x0

#define SELFID_SPEED_UNKNOWN		0x3	/* 1394b PHY */

#define PHYPACKET_LINKON			0x40000000
#define PHYPACKET_PHYCONFIG_R			0x00800000
#define PHYPACKET_PHYCONFIG_T			0x00400000
#define EXTPHYPACKET_TYPE_PING			0x00000000
#define EXTPHYPACKET_TYPE_REMOTEACCESS_BASE	0x00040000
#define EXTPHYPACKET_TYPE_REMOTEACCESS_PAGED	0x00140000
#define EXTPHYPACKET_TYPE_REMOTEREPLY_BASE	0x000C0000
#define EXTPHYPACKET_TYPE_REMOTEREPLY_PAGED	0x001C0000
#define EXTPHYPACKET_TYPE_REMOTECOMMAND		0x00200000
#define EXTPHYPACKET_TYPE_REMOTECONFIRMATION	0x00280000
#define EXTPHYPACKET_TYPE_RESUME		0x003C0000

#define EXTPHYPACKET_TYPEMASK			0xC0FC0000

#define PHYPACKET_PORT_SHIFT		24
#define PHYPACKET_GAPCOUNT_SHIFT	16

/* 1394a PHY register map bitmasks */
#define PHY_00_PHYSICAL_ID	0xFC
#define PHY_00_R		0x02 /* Root */
#define PHY_00_PS		0x01 /* Power Status*/
#define PHY_01_RHB		0x80 /* Root Hold-Off */
#define PHY_01_IBR		0x80 /* Initiate Bus Reset */
#define PHY_01_GAP_COUNT	0x3F
#define PHY_02_EXTENDED		0xE0 /* 0x7 for 1394a-compliant PHY */
#define PHY_02_TOTAL_PORTS	0x1F
#define PHY_03_MAX_SPEED	0xE0
#define PHY_03_DELAY		0x0F
#define PHY_04_LCTRL		0x80 /* Link Active Report Control */
#define PHY_04_CONTENDER	0x40
#define PHY_04_JITTER		0x38
#define PHY_04_PWR_CLASS	0x07 /* Power Class */
#define PHY_05_WATCHDOG		0x80
#define PHY_05_ISBR		0x40 /* Initiate Short Bus Reset */
#define PHY_05_LOOP		0x20 /* Loop Detect */
#define PHY_05_PWR_FAIL		0x10 /* Cable Power Failure Detect */
#define PHY_05_TIMEOUT		0x08 /* Arbitration State Machine Timeout */
#define PHY_05_PORT_EVENT	0x04 /* Port Event Detect */
#define PHY_05_ENAB_ACCEL	0x02 /* Enable Arbitration Acceleration */
#define PHY_05_ENAB_MULTI	0x01 /* Ena. Multispeed Packet Concatenation */

#include <asm/byteorder.h>

/* '1' '3' '9' '4' in ASCII */
#define IEEE1394_BUSID_MAGIC	cpu_to_be32(0x31333934)

#ifdef __BIG_ENDIAN_BITFIELD

struct selfid {
	u32 packet_identifier:2; /* always binary 10 */
	u32 phy_id:6;
	/* byte */
	u32 extended:1; /* if true is struct ext_selfid */
	u32 link_active:1;
	u32 gap_count:6;
	/* byte */
	u32 speed:2;
	u32 phy_delay:2;
	u32 contender:1;
	u32 power_class:3;
	/* byte */
	u32 port0:2;
	u32 port1:2;
	u32 port2:2;
	u32 initiated_reset:1;
	u32 more_packets:1;
} __attribute__((packed));

struct ext_selfid {
	u32 packet_identifier:2; /* always binary 10 */
	u32 phy_id:6;
	/* byte */
	u32 extended:1; /* if false is struct selfid */
	u32 seq_nr:3;
	u32 reserved:2;
	u32 porta:2;
	/* byte */
	u32 portb:2;
	u32 portc:2;
	u32 portd:2;
	u32 porte:2;
	/* byte */
	u32 portf:2;
	u32 portg:2;
	u32 porth:2;
	u32 reserved2:1;
	u32 more_packets:1;
} __attribute__((packed));

#elif defined __LITTLE_ENDIAN_BITFIELD /* __BIG_ENDIAN_BITFIELD */

/*
 * Note: these mean to be bit fields of a big endian SelfID as seen on a little
 * endian machine.  Without swapping.
 */

struct selfid {
	u32 phy_id:6;
	u32 packet_identifier:2; /* always binary 10 */
	/* byte */
	u32 gap_count:6;
	u32 link_active:1;
	u32 extended:1; /* if true is struct ext_selfid */
	/* byte */
	u32 power_class:3;
	u32 contender:1;
	u32 phy_delay:2;
	u32 speed:2;
	/* byte */
	u32 more_packets:1;
	u32 initiated_reset:1;
	u32 port2:2;
	u32 port1:2;
	u32 port0:2;
} __attribute__((packed));

struct ext_selfid {
	u32 phy_id:6;
	u32 packet_identifier:2; /* always binary 10 */
	/* byte */
	u32 porta:2;
	u32 reserved:2;
	u32 seq_nr:3;
	u32 extended:1; /* if false is struct selfid */
	/* byte */
	u32 porte:2;
	u32 portd:2;
	u32 portc:2;
	u32 portb:2;
	/* byte */
	u32 more_packets:1;
	u32 reserved2:1;
	u32 porth:2;
	u32 portg:2;
	u32 portf:2;
} __attribute__((packed));

#else
#error What? PDP endian?
#endif /* __BIG_ENDIAN_BITFIELD */

#endif /* _IEEE1394_IEEE1394_H */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   /*
 *  indycam.c - Silicon Graphics IndyCam digital camera driver
 *
 *  Copyright (C) 2003 Ladislav Michl <ladis@linux-mips.org>
 *  Copyright (C) 2004,2005 Mikael Nousiainen <tmnousia@cc.hut.fi>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>

/* IndyCam decodes stream of photons into digital image representation ;-) */
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-i2c-drv.h>

#include "indycam.h"

#define INDYCAM_MODULE_VERSION "0.0.5"

MODULE_DESCRIPTION("SGI IndyCam driver");
MODULE_VERSION(INDYCAM_MODULE_VERSION);
MODULE_AUTHOR("Mikael Nousiainen <tmnousia@cc.hut.fi>");
MODULE_LICENSE("GPL");


// #define INDYCAM_DEBUG

#ifdef INDYCAM_DEBUG
#define dprintk(x...) printk("IndyCam: " x);
#define indycam_regdump(client) indycam_regdump_debug(client)
#else
#define dprintk(x...)
#define indycam_regdump(client)
#endif

struct indycam {
	struct v4l2_subdev sd;
	u8 version;
};

static inline struct indycam *to_indycam(struct v4l2_subdev *sd)
{
	return container_of(sd, struct indycam, sd);
}

static const u8 initseq[] = {
	INDYCAM_CONTROL_AGCENA,		/* INDYCAM_CONTROL */
	INDYCAM_SHUTTER_60,		/* INDYCAM_SHUTTER */
	INDYCAM_GAIN_DEFAULT,		/* INDYCAM_GAIN */
	0x00,				/* INDYCAM_BRIGHTNESS (read-only) */
	INDYCAM_RED_BALANCE_DEFAULT,	/* INDYCAM_RED_BALANCE */
	INDYCAM_BLUE_BALANCE_DEFAULT,	/* INDYCAM_BLUE_BALANCE */
	INDYCAM_RED_SATURATION_DEFAULT,	/* INDYCAM_RED_SATURATION */
	INDYCAM_BLUE_SATURATION_DEFAULT,/* INDYCAM_BLUE_SATURATION */
};

/* IndyCam register handling */

static int indycam_read_reg(struct v4l2_subdev *sd, u8 reg, u8 *value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	if (reg == INDYCAM_REG_RESET) {
		dprintk("indycam_read_reg(): "
			"skipping write-only register %d\n", reg);
		*value = 0;
		return 0;
	}

	ret = i2c_smbus_read_byte_data(client, reg);

	if (ret < 0) {
		printk(KERN_ERR "IndyCam: indycam_read_reg(): read failed, "
		       "register = 0x%02x\n", reg);
		return ret;
	}

	*value = (u8)ret;

	return 0;
}

static int indycam_write_reg(struct v4l2_subdev *sd, u8 reg, u8 value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err;

	if (reg == INDYCAM_REG_BRIGHTNESS || reg == INDYCAM_REG_VERSION) {
		dprintk("indycam_write_reg(): "
			"skipping read-only register %d\n", reg);
		return 0;
	}

	dprintk("Writing Reg %d = 0x%02x\n", reg, value);
	err = i2c_smbus_write_byte_data(client, reg, value);

	if (err) {
		printk(KERN_ERR "IndyCam: indycam_write_reg(): write failed, "
		       "register = 0x%02x, value = 0x%02x\n", reg, value);
	}
	return err;
}

static int indycam_write_block(struct v4l2_subdev *sd, u8 reg,
			       u8 length, u8 *data)
{
	int i, err;

	for (i = 0; i < length; i++) {
		err = indycam_write_reg(sd, reg + i, data[i]);
		if (err)
			return err;
	}

	return 0;
}

/* Helper functions */

#ifdef INDYCAM_DEBUG
static void indycam_regdump_debug(struct v4l2_subdev *sd)
{
	int i;
	u8 val;

	for (i = 0; i < 9; i++) {
		indycam_read_reg(sd, i, &val);
		dprintk("Reg %d = 0x%02x\n", i, val);
	}
}
#endif

static int indycam_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct indycam *camera = to_indycam(sd);
	u8 reg;
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_AUTOGAIN:
	case V4L2_CID_AUTO_WHITE_BALANCE:
		ret = indycam_read_reg(sd, INDYCAM_REG_CONTROL, &reg);
		if (ret)
			return -EIO;
		if (ctrl->id == V4L2_CID_AUTOGAIN)
			ctrl->value = (reg & INDYCAM_CONTROL_AGCENA)
				? 1 : 0;
		else
			ctrl->value = (reg & INDYCAM_CONTROL_AWBCTL)
				? 1 : 0;
		break;
	case V4L2_CID_EXPOSURE:
		ret = indycam_read_reg(sd, INDYCAM_REG_SHUTTER, &reg);
		if (ret)
			return -EIO;
		ctrl->value = ((s32)reg == 0x00) ? 0xff : ((s32)reg - 1);
		break;
	case V4L2_CID_GAIN:
		ret = indycam_read_reg(sd, INDYCAM_REG_GAIN, &reg);
		if (ret)
			return -EIO;
		ctrl->value = (s32)reg;
		break;
	case V4L2_CID_RED_BALANCE:
		ret = indycam_read_reg(sd, INDYCAM_REG_RED_BALANCE, &reg);
		if (ret)
			return -EIO;
		ctrl->value = (s32)reg;
		break;
	case V4L2_CID_BLUE_BALANCE:
		ret = indycam_read_reg(sd, INDYCAM_REG_BLUE_BALANCE, &reg);
		if (ret)
			return -EIO;
		ctrl->value = (s32)reg;
		break;
	case INDYCAM_CONTROL_RED_SATURATION:
		ret = indycam_read_reg(sd,
				       INDYCAM_REG_RED_SATURATION, &reg);
		if (ret)
			return -EIO;
		ctrl->value = (s32)reg;
		break;
	case INDYCAM_CONTROL_BLUE_SATURATION:
		ret = indycam_read_reg(sd,
				       INDYCAM_REG_BLUE_SATURATION, &reg);
		if (ret)
			return -EIO;
		ctrl->value = (s32)reg;
		break;
	case V4L2_CID_GAMMA:
		if (camera->version == CAMERA_VERSION_MOOSE) {
			ret = indycam_read_reg(sd,
					       INDYCAM_REG_GAMMA, &reg);
			if (ret)
				return -EIO;
			ctrl->value = (s32)reg;
		} else {
			ctrl->value = INDYCAM_GAMMA_DEFAULT;
		}
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static int indycam_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct indycam *camera = to_indycam(sd);
	u8 reg;
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_AUTOGAIN:
	case V4L2_CID_AUTO_WHITE_BALANCE:
		ret = indycam_read_reg(sd, INDYCAM_REG_CONTROL, &reg);
		if (ret)
			break;

		if (ctrl->id == V4L2_CID_AUTOGAIN) {
			if (ctrl->value)
				reg |= INDYCAM_CONTROL_AGCENA;
			else
				reg &= ~INDYCAM_CONTROL_AGCENA;
		} else {
			if (ctrl->value)
				reg |= INDYCAM_CONTROL_AWBCTL;
			else
				reg &= ~INDYCAM_CONTROL_AWBCTL;
		}

		ret = indycam_write_reg(sd, INDYCAM_REG_CONTROL, reg);
		break;
	case V4L2_CID_EXPOSURE:
		reg = (ctrl->value == 0xff) ? 0x00 : (ctrl->value + 1);
		ret = indycam_write_reg(sd, INDYCAM_REG_SHUTTER, reg);
		break;
	case V4L2_CID_GAIN:
		ret = indycam_write_reg(sd, INDYCAM_REG_GAIN, ctrl->value);
		break;
	case V4L2_CID_RED_BALANCE:
		ret = indycam_write_reg(sd, INDYCAM_REG_RED_BALANCE,
					ctrl->value);
		break;
	case V4L2_CID_BLUE_BALANCE:
		ret = indycam_write_reg(sd, INDYCAM_REG_BLUE_BALANCE,
					ctrl->value);
		break;
	case INDYCAM_CONTROL_RED_SATURATION:
		ret = indycam_write_reg(sd, INDYCAM_REG_RED_SATURATION,
					ctrl->value);
		break;
	case INDYCAM_CONTROL_BLUE_SATURATION:
		ret = indycam_write_reg(sd, INDYCAM_REG_BLUE_SATURATION,
					ctrl->value);
		break;
	case V4L2_CID_GAMMA:
		if (camera->version == CAMERA_VERSION_MOOSE) {
			ret = indycam_write_reg(sd, INDYCAM_REG_GAMMA,
						ctrl->value);
		}
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

/* I2C-interface */

static int indycam_g_chip_ident(struct v4l2_subdev *sd,
		struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct indycam *camera = to_indycam(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_INDYCAM,
		       camera->version);
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops indycam_core_ops = {
	.g_chip_ident = indycam_g_chip_ident,
	.g_ctrl = indycam_g_ctrl,
	.s_ctrl = indycam_s_ctrl,
};

static const struct v4l2_subdev_ops indycam_ops = {
	.core = &indycam_core_ops,
};

static int indycam_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	int err = 0;
	struct indycam *camera;
	struct v4l2_subdev *sd;

	v4l_info(client, "chip found @ 0x%x (%s)\n",
			client->addr << 1, client->adapter->name);

	camera = kzalloc(sizeof(struct indycam), GFP_KERNEL);
	if (!camera)
		return -ENOMEM;

	sd = &camera->sd;
	v4l2_i2c_subdev_init(sd, client, &indycam_ops);

	camera->version = i2c_smbus_read_byte_data(client,
						   INDYCAM_REG_VERSION);
	if (camera->version != CAMERA_VERSION_INDY &&
	    camera->version != CAMERA_VERSION_MOOSE) {
		kfree(camera);
		return -ENODEV;
	}

	printk(KERN_INFO "IndyCam v%d.%d detected\n",
	       INDYCAM_VERSION_MAJOR(camera->version),
	       INDYCAM_VERSION_MINOR(camera->version));

	indycam_regdump(sd);

	// initialize
	err = indycam_write_block(sd, 0, sizeof(initseq), (u8 *)&initseq);
	if (err) {
		printk(KERN_ERR "IndyCam initialization failed\n");
		kfree(camera);
		return -EIO;
	}

	indycam_regdump(sd);

	// white balance
	err = indycam_write_reg(sd, INDYCAM_REG_CONTROL,
			  INDYCAM_CONTROL_AGCENA | INDYCAM_CONTROL_AWBCTL);
	if (err) {
		printk(KERN_ERR "IndyCam: White balancing camera failed\n");
		kfree(camera);
		return -EIO;
	}

	indycam_regdump(sd);

	printk(KERN_INFO "IndyCam initialized\n");

	return 0;
}

static int indycam_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	kfree(to_indycam(sd));
	return 0;
}

static const struct i2c_device_id indycam_id[] = {
	{ "indycam", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, indycam_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "indycam",
	.probe = indycam_probe,
	.remove = indycam_remove,
	.id_table = indycam_id,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           /* saa711x - Philips SAA711x video decoder register specifications
 *
 * Copyright (c) 2006 Mauro Carvalho Chehab <mchehab@infradead.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define R_00_CHIP_VERSION                             0x00
/* Video Decoder */
	/* Video Decoder - Frontend part */
#define R_01_INC_DELAY                                0x01
#define R_02_INPUT_CNTL_1                             0x02
#define R_03_INPUT_CNTL_2                             0x03
#define R_04_INPUT_CNTL_3                             0x04
#define R_05_INPUT_CNTL_4                             0x05
	/* Video Decoder - Decoder part */
#define R_06_H_SYNC_START                             0x06
#define R_07_H_SYNC_STOP                              0x07
#define R_08_SYNC_CNTL                                0x08
#define R_09_LUMA_CNTL                                0x09
#define R_0A_LUMA_BRIGHT_CNTL                         0x0a
#define R_0B_LUMA_CONTRAST_CNTL                       0x0b
#define R_0C_CHROMA_SAT_CNTL                          0x0c
#define R_0D_CHROMA_HUE_CNTL                          0x0d
#define R_0E_CHROMA_CNTL_1                            0x0e
#define R_0F_CHROMA_GAIN_CNTL                         0x0f
#define R_10_CHROMA_CNTL_2                            0x10
#define R_11_MODE_DELAY_CNTL                          0x11
#define R_12_RT_SIGNAL_CNTL                           0x12
#define R_13_RT_X_PORT_OUT_CNTL                       0x13
#define R_14_ANAL_ADC_COMPAT_CNTL                     0x14
#define R_15_VGATE_START_FID_CHG                      0x15
#define R_16_VGATE_STOP                               0x16
#define R_17_MISC_VGATE_CONF_AND_MSB                  0x17
#define R_18_RAW_DATA_GAIN_CNTL                       0x18
#define R_19_RAW_DATA_OFF_CNTL                        0x19
#define R_1A_COLOR_KILL_LVL_CNTL                      0x1a
#define R_1B_MISC_TVVCRDET                            0x1b
#define R_1C_ENHAN_COMB_CTRL1                         0x1c
#define R_1D_ENHAN_COMB_CTRL2                         0x1d
#define R_1E_STATUS_BYTE_1_VD_DEC                     0x1e
#define R_1F_STATUS_BYTE_2_VD_DEC                     0x1f

/* Component processing and interrupt masking part */
#define R_23_INPUT_CNTL_5                             0x23
#define R_24_INPUT_CNTL_6                             0x24
#define R_25_INPUT_CNTL_7                             0x25
#define R_29_COMP_DELAY                               0x29
#define R_2A_COMP_BRIGHT_CNTL                         0x2a
#define R_2B_COMP_CONTRAST_CNTL                       0x2b
#define R_2C_COMP_SAT_CNTL                            0x2c
#define R_2D_INTERRUPT_MASK_1                         0x2d
#define R_2E_INTERRUPT_MASK_2                         0x2e
#define R_2F_INTERRUPT_MASK_3                         0x2f

/* Audio clock generator part */
#define R_30_AUD_MAST_CLK_CYCLES_PER_FIELD            0x30
#define R_34_AUD_MAST_CLK_NOMINAL_INC                 0x34
#define R_38_CLK_RATIO_AMXCLK_TO_ASCLK                0x38
#define R_39_CLK_RATIO_ASCLK_TO_ALRCLK                0x39
#define R_3A_AUD_CLK_GEN_BASIC_SETUP                  0x3a

/* General purpose VBI data slicer part */
#define R_40_SLICER_CNTL_1                            0x40
#define R_41_LCR_BASE                                 0x41
#define R_58_PROGRAM_FRAMING_CODE                     0x58
#define R_59_H_OFF_FOR_SLICER                         0x59
#define R_5A_V_OFF_FOR_SLICER                         0x5a
#define R_5B_FLD_OFF_AND_MSB_FOR_H_AND_V_OFF          0x5b
#define R_5D_DID                                      0x5d
#define R_5E_SDID                                     0x5e
#define R_60_SLICER_STATUS_BYTE_0                     0x60
#define R_61_SLICER_STATUS_BYTE_1                     0x61
#define R_62_SLICER_STATUS_BYTE_2                     0x62

/* X port, I port and the scaler part */
	/* Task independent global settings */
#define R_80_GLOBAL_CNTL_1                            0x80
#define R_81_V_SYNC_FLD_ID_SRC_SEL_AND_RETIMED_V_F    0x81
#define R_83_X_PORT_I_O_ENA_AND_OUT_CLK               0x83
#define R_84_I_PORT_SIGNAL_DEF                        0x84
#define R_85_I_PORT_SIGNAL_POLAR                      0x85
#define R_86_I_PORT_FIFO_FLAG_CNTL_AND_ARBIT          0x86
#define R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED         0x87
#define R_88_POWER_SAVE_ADC_PORT_CNTL                 0x88
#define R_8F_STATUS_INFO_SCALER                       0x8f
	/* Task A definition */
		/* Basic settings and acquisition window definition */
#define R_90_A_TASK_HANDLING_CNTL                     0x90
#define R_91_A_X_PORT_FORMATS_AND_CONF                0x91
#define R_92_A_X_PORT_INPUT_REFERENCE_SIGNAL          0x92
#define R_93_A_I_PORT_OUTPUT_FORMATS_AND_CONF         0x93
#define R_94_A_HORIZ_INPUT_WINDOW_START               0x94
#define R_95_A_HORIZ_INPUT_WINDOW_START_MSB           0x95
#define R_96_A_HORIZ_INPUT_WINDOW_LENGTH              0x96
#define R_97_A_HORIZ_INPUT_WINDOW_LENGTH_MSB          0x97
#define R_98_A_VERT_INPUT_WINDOW_START                0x98
#define R_99_A_VERT_INPUT_WINDOW_START_MSB            0x99
#define R_9A_A_VERT_INPUT_WINDOW_LENGTH               0x9a
#define R_9B_A_VERT_INPUT_WINDOW_LENGTH_MSB           0x9b
#define R_9C_A_HORIZ_OUTPUT_WINDOW_LENGTH             0x9c
#define R_9D_A_HORIZ_OUTPUT_WINDOW_LENGTH_MSB         0x9d
#define R_9E_A_VERT_OUTPUT_WINDOW_LENGTH              0x9e
#define R_9F_A_VERT_OUTPUT_WINDOW_LENGTH_MSB          0x9f
		/* FIR filtering and prescaling */
#define R_A0_A_HORIZ_PRESCALING                       0xa0
#define R_A1_A_ACCUMULATION_LENGTH                    0xa1
#define R_A2_A_PRESCALER_DC_GAIN_AND_FIR_PREFILTER    0xa2
#define R_A4_A_LUMA_BRIGHTNESS_CNTL                   0xa4
#define R_A5_A_LUMA_CONTRAST_CNTL                     0xa5
#define R_A6_A_CHROMA_SATURATION_CNTL                 0xa6
		/* Horizontal phase scaling */
#define R_A8_A_HORIZ_LUMA_SCALING_INC                 0xa8
#define R_A9_A_HORIZ_LUMA_SCALING_INC_MSB             0xa9
#define R_AA_A_HORIZ_LUMA_PHASE_OFF                   0xaa
#define R_AC_A_HORIZ_CHROMA_SCALING_INC               0xac
#define R_AD_A_HORIZ_CHROMA_SCALING_INC_MSB           0xad
#define R_AE_A_HORIZ_CHROMA_PHASE_OFF                 0xae
#define R_AF_A_HORIZ_CHROMA_PHASE_OFF_MSB             0xaf
		/* Vertical scaling */
#define R_B0_A_VERT_LUMA_SCALING_INC                  0xb0
#define R_B1_A_VERT_LUMA_SCALING_INC_MSB              0xb1
#define R_B2_A_VERT_CHROMA_SCALING_INC                0xb2
#define R_B3_A_VERT_CHROMA_SCALING_INC_MSB            0xb3
#define R_B4_A_VERT_SCALING_MODE_CNTL                 0xb4
#define R_B8_A_VERT_CHROMA_PHASE_OFF_00               0xb8
#define R_B9_A_VERT_CHROMA_PHASE_OFF_01               0xb9
#define R_BA_A_VERT_CHROMA_PHASE_OFF_10               0xba
#define R_BB_A_VERT_CHROMA_PHASE_OFF_11               0xbb
#define R_BC_A_VERT_LUMA_PHASE_OFF_00                 0xbc
#define R_BD_A_VERT_LUMA_PHASE_OFF_01                 0xbd
#define R_BE_A_VERT_LUMA_PHASE_OFF_10                 0xbe
#define R_BF_A_VERT_LUMA_PHASE_OFF_11                 0xbf
	/* Task B definition */
		/* Basic settings and acquisition window definition */
#define R_C0_B_TASK_HANDLING_CNTL                     0xc0
#define R_C1_B_X_PORT_FORMATS_AND_CONF                0xc1
#define R_C2_B_INPUT_REFERENCE_SIGNAL_DEFINITION      0xc2
#define R_C3_B_I_PORT_FORMATS_AND_CONF                0xc3
#define R_C4_B_HORIZ_INPUT_WINDOW_START               0xc4
#define R_C5_B_HORIZ_INPUT_WINDOW_START_MSB           0xc5
#define R_C6_B_HORIZ_INPUT_WINDOW_LENGTH              0xc6
#define R_C7_B_HORIZ_INPUT_WINDOW_LENGTH_MSB          0xc7
#define R_C8_B_VERT_INPUT_WINDOW_START                0xc8
#define R_C9_B_VERT_INPUT_WINDOW_START_MSB            0xc9
#define R_CA_B_VERT_INPUT_WINDOW_LENGTH               0xca
#define R_CB_B_VERT_INPUT_WINDOW_LENGTH_MSB           0xcb
#define R_CC_B_HORIZ_OUTPUT_WINDOW_LENGTH             0xcc
#define R_CD_B_HORIZ_OUTPUT_WINDOW_LENGTH_MSB         0xcd
#define R_CE_B_VERT_OUTPUT_WINDOW_LENGTH              0xce
#define R_CF_B_VERT_OUTPUT_WINDOW_LENGTH_MSB          0xcf
		/* FIR filtering and prescaling */
#define R_D0_B_HORIZ_PRESCALING                       0xd0
#define R_D1_B_ACCUMULATION_LENGTH                    0xd1
#define R_D2_B_PRESCALER_DC_GAIN_AND_FIR_PREFILTER    0xd2
#define R_D4_B_LUMA_BRIGHTNESS_CNTL                   0xd4
#define R_D5_B_LUMA_CONTRAST_CNTL                     0xd5
#define R_D6_B_CHROMA_SATURATION_CNTL                 0xd6
		/* Horizontal phase scaling */
#define R_D8_B_HORIZ_LUMA_SCALING_INC                 0xd8
#define R_D9_B_HORIZ_LUMA_SCALING_INC_MSB             0xd9
#define R_DA_B_HORIZ_LUMA_PHASE_OFF                   0xda
#define R_DC_B_HORIZ_CHROMA_SCALING                   0xdc
#define R_DD_B_HORIZ_CHROMA_SCALING_MSB               0xdd
#define R_DE_B_HORIZ_PHASE_OFFSET_CRHOMA              0xde
		/* Vertical scaling */
#define R_E0_B_VERT_LUMA_SCALING_INC                  0xe0
#define R_E1_B_VERT_LUMA_SCALING_INC_MSB              0xe1
#define R_E2_B_VERT_CHROMA_SCALING_INC                0xe2
#define R_E3_B_VERT_CHROMA_SCALING_INC_MSB            0xe3
#define R_E4_B_VERT_SCALING_MODE_CNTL                 0xe4
#define R_E8_B_VERT_CHROMA_PHASE_OFF_00               0xe8
#define R_E9_B_VERT_CHROMA_PHASE_OFF_01               0xe9
#define R_EA_B_VERT_CHROMA_PHASE_OFF_10               0xea
#define R_EB_B_VERT_CHROMA_PHASE_OFF_11               0xeb
#define R_EC_B_VERT_LUMA_PHASE_OFF_00                 0xec
#define R_ED_B_VERT_LUMA_PHASE_OFF_01                 0xed
#define R_EE_B_VERT_LUMA_PHASE_OFF_10                 0xee
#define R_EF_B_VERT_LUMA_PHASE_OFF_11                 0xef

/* second PLL (PLL2) and Pulsegenerator Programming */
#define R_F0_LFCO_PER_LINE                            0xf0
#define R_F1_P_I_PARAM_SELECT                         0xf1
#define R_F2_NOMINAL_PLL2_DTO                         0xf2
#define R_F3_PLL_INCREMENT                            0xf3
#define R_F4_PLL2_STATUS                              0xf4
#define R_F5_PULSGEN_LINE_LENGTH                      0xf5
#define R_F6_PULSE_A_POS_LSB_AND_PULSEGEN_CONFIG      0xf6
#define R_F7_PULSE_A_POS_MSB                          0xf7
#define R_F8_PULSE_B_POS                              0xf8
#define R_F9_PULSE_B_POS_MSB                          0xf9
#define R_FA_PULSE_C_POS                              0xfa
#define R_FB_PULSE_C_POS_MSB                          0xfb
#define R_FF_S_PLL_MAX_PHASE_ERR_THRESH_NUM_LINES     0xff

#if 0
/* Those structs will be used in the future for debug purposes */
struct saa711x_reg_descr {
	u8 reg;
	int count;
	char *name;
};

struct saa711x_reg_descr saa711x_regs[] = {
	/* REG COUNT NAME */
	{R_00_CHIP_VERSION,1,
	 "Chip version"},

	/* Video Decoder: R_01_INC_DELAY to R_1F_STATUS_BYTE_2_VD_DEC */

	/* Video Decoder - Frontend part: R_01_INC_DELAY to R_05_INPUT_CNTL_4 */
	{R_01_INC_DELAY,1,
	 "Increment delay"},
	{R_02_INPUT_CNTL_1,1,
	 "Analog input control 1"},
	{R_03_INPUT_CNTL_2,1,
	 "Analog input control 2"},
	{R_04_INPUT_CNTL_3,1,
	 "Analog input control 3"},
	{R_05_INPUT_CNTL_4,1,
	 "Analog input control 4"},

	/* Video Decoder - Decoder part: R_06_H_SYNC_START to R_1F_STATUS_BYTE_2_VD_DEC */
	{R_06_H_SYNC_START,1,
	 "Horizontal sync start"},
	{R_07_H_SYNC_STOP,1,
	 "Horizontal sync stop"},
	{R_08_SYNC_CNTL,1,
	 "Sync control"},
	{R_09_LUMA_CNTL,1,
	 "Luminance control"},
	{R_0A_LUMA_BRIGHT_CNTL,1,
	 "Luminance brightness control"},
	{R_0B_LUMA_CONTRAST_CNTL,1,
	 "Luminance contrast control"},
	{R_0C_CHROMA_SAT_CNTL,1,
	 "Chrominance saturation control"},
	{R_0D_CHROMA_HUE_CNTL,1,
	 "Chrominance hue control"},
	{R_0E_CHROMA_CNTL_1,1,
	 "Chrominance control 1"},
	{R_0F_CHROMA_GAIN_CNTL,1,
	 "Chrominance gain control"},
	{R_10_CHROMA_CNTL_2,1,
	 "Chrominance control 2"},
	{R_11_MODE_DELAY_CNTL,1,
	 "Mode/delay control"},
	{R_12_RT_SIGNAL_CNTL,1,
	 "RT signal control"},
	{R_13_RT_X_PORT_OUT_CNTL,1,
	 "RT/X port output control"},
	{R_14_ANAL_ADC_COMPAT_CNTL,1,
	 "Analog/ADC/compatibility control"},
	{R_15_VGATE_START_FID_CHG,  1,
	 "VGATE start FID change"},
	{R_16_VGATE_STOP,1,
	 "VGATE stop"},
	{R_17_MISC_VGATE_CONF_AND_MSB,  1,
	 "Miscellaneous VGATE configuration and MSBs"},
	{R_18_RAW_DATA_GAIN_CNTL,1,
	 "Raw data gain control",},
	{R_19_RAW_DATA_OFF_CNTL,1,
	 "Raw data offset control",},
	{R_1A_COLOR_KILL_LVL_CNTL,1,
	 "Color Killer Level Control"},
	{ R_1B_MISC_TVVCRDET, 1,
	  "MISC /TVVCRDET"},
	{ R_1C_ENHAN_COMB_CTRL1, 1,
	 "Enhanced comb ctrl1"},
	{ R_1D_ENHAN_COMB_CTRL2, 1,
	 "Enhanced comb ctrl1"},
	{R_1E_STATUS_BYTE_1_VD_DEC,1,
	 "Status byte 1 video decoder"},
	{R_1F_STATUS_BYTE_2_VD_DEC,1,
	 "Status byte 2 video decoder"},

	/* Component processing and interrupt masking part:  0x20h to R_2F_INTERRUPT_MASK_3 */
	/* 0x20 to 0x22 - Reserved */
	{R_23_INPUT_CNTL_5,1,
	 "Analog input control 5"},
	{R_24_INPUT_CNTL_6,1,
	 "Analog input control 6"},
	{R_25_INPUT_CNTL_7,1,
	 "Analog input control 7"},
	/* 0x26 to 0x28 - Reserved */
	{R_29_COMP_DELAY,1,
	 "Component delay"},
	{R_2A_COMP_BRIGHT_CNTL,1,
	 "Component brightness control"},
	{R_2B_COMP_CONTRAST_CNTL,1,
	 "Component contrast control"},
	{R_2C_COMP_SAT_CNTL,1,
	 "Component saturation control"},
	{R_2D_INTERRUPT_MASK_1,1,
	 "Interrupt mask 1"},
	{R_2E_INTERRUPT_MASK_2,1,
	 "Interrupt mask 2"},
	{R_2F_INTERRUPT_MASK_3,1,
	 "Interrupt mask 3"},

	/* Audio clock generator part: R_30_AUD_MAST_CLK_CYCLES_PER_FIELD to 0x3f */
	{R_30_AUD_MAST_CLK_CYCLES_PER_FIELD,3,
	 "Audio master clock cycles per field"},
	/* 0x33 - Reserved */
	{R_34_AUD_MAST_CLK_NOMINAL_INC,3,
	 "Audio master clock nominal increment"},
	/* 0x37 - Reserved */
	{R_38_CLK_RATIO_AMXCLK_TO_ASCLK,1,
	 "Clock ratio AMXCLK to ASCLK"},
	{R_39_CLK_RATIO_ASCLK_TO_ALRCLK,1,
	 "Clock ratio ASCLK to ALRCLK"},
	{R_3A_AUD_CLK_GEN_BASIC_SETUP,1,
	 "Audio clock generator basic setup"},
	/* 0x3b-0x3f - Reserved */

	/* General purpose VBI data slicer part: R_40_SLICER_CNTL_1 to 0x7f */
	{R_40_SLICER_CNTL_1,1,
	 "Slicer control 1"},
	{R_41_LCR,23,
	 "R_41_LCR"},
	{R_58_PROGRAM_FRAMING_CODE,1,
	 "Programmable framing code"},
	{R_59_H_OFF_FOR_SLICER,1,
	 "Horizontal offset for slicer"},
	{R_5A_V_OFF_FOR_SLICER,1,
	 "Vertical offset for slicer"},
	{R_5B_FLD_OFF_AND_MSB_FOR_H_AND_V_OFF,1,
	 "Field offset and MSBs for horizontal and vertical offset"},
	{R_5D_DID,1,
	 "Header and data identification (R_5D_DID)"},
	{R_5E_SDID,1,
	 "Sliced data identification (R_5E_SDID) code"},
	{R_60_SLICER_STATUS_BYTE_0,1,
	 "Slicer status byte 0"},
	{R_61_SLICER_STATUS_BYTE_1,1,
	 "Slicer status byte 1"},
	{R_62_SLICER_STATUS_BYTE_2,1,
	 "Slicer status byte 2"},
	/* 0x63-0x7f - Reserved */

	/* X port, I port and the scaler part: R_80_GLOBAL_CNTL_1 to R_EF_B_VERT_LUMA_PHASE_OFF_11 */
	/* Task independent global settings: R_80_GLOBAL_CNTL_1 to R_8F_STATUS_INFO_SCALER */
	{R_80_GLOBAL_CNTL_1,1,
	 "Global control 1"},
	{R_81_V_SYNC_FLD_ID_SRC_SEL_AND_RETIMED_V_F,1,
	 "Vertical sync and Field ID source selection, retimed V and F signals"},
	/* 0x82 - Reserved */
	{R_83_X_PORT_I_O_ENA_AND_OUT_CLK,1,
	 "X port I/O enable and output clock"},
	{R_84_I_PORT_SIGNAL_DEF,1,
	 "I port signal definitions"},
	{R_85_I_PORT_SIGNAL_POLAR,1,
	 "I port signal polarities"},
	{R_86_I_PORT_FIFO_FLAG_CNTL_AND_ARBIT,1,
	 "I port FIFO flag control and arbitration"},
	{R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED,  1,
	 "I port I/O enable output clock and gated"},
	{R_88_POWER_SAVE_ADC_PORT_CNTL,1,
	 "Power save/ADC port control"},
	/* 089-0x8e - Reserved */
	{R_8F_STATUS_INFO_SCALER,1,
	 "Status information scaler part"},

	/* Task A definition: R_90_A_TASK_HANDLING_CNTL to R_BF_A_VERT_LUMA_PHASE_OFF_11 */
	/* Task A: Basic settings and acquisition window definition */
	{R_90_A_TASK_HANDLING_CNTL,1,
	 "Task A: Task handling control"},
	{R_91_A_X_PORT_FORMATS_AND_CONF,1,
	 "Task A: X port formats and configuration"},
	{R_92_A_X_Pd);
                zm_debug_msg1("EAP-Packet Venodr Type = ", VendorType);
                zm_debug_msg1("EAP-Packet Op Code = ", Op_Code);
                zm_debug_msg1("EAP-Packet Flags = ", flags);
            }
        }
        else if( code == 3 )
        {
            zm_debug_msg0("EAP-Packet Success");

            /* EAP-Packet Identifier */
            identifier = zmw_rx_buf_readb(dev, buf, offset+5);
            /* EAP-Packet Length */
            length = (((u16_t) zmw_rx_buf_readb(dev, buf, offset+6)) << 8) +
                      zmw_rx_buf_readb(dev, buf, offset+7);

            zm_debug_msg1("EAP-Packet Identifier = ", identifier);
            zm_debug_msg1("EAP-Packet Length = ", length);
        }
        else if( code == 4 )
        {
            zm_debug_msg0("EAP-Packet Failure");

            /* EAP-Packet Identifier */
            identifier = zmw_rx_buf_readb(dev, buf, offset+5);
            /* EAP-Packet Length */
            length = (((u16_t) zmw_rx_buf_readb(dev, buf, offset+6)) << 8) +
                      zmw_rx_buf_readb(dev, buf, offset+7);

            zm_debug_msg1("EAP-Packet Identifier = ", identifier);
            zm_debug_msg1("EAP-Packet Length = ", length);
        }
    }
    else if( packetType == 1 )
    { // EAPOL-Start
        zm_debug_msg0("EAPOL-Start");
    }
    else if( packetType == 2 )
    { // EAPOL-Logoff
        zm_debug_msg0("EAPOL-Logoff");
    }
    else if( packetType == 3 )
    { // EAPOL-Key
        /* EAPOL-Key type */
        keyType = zmw_rx_buf_readb(dev, buf, offset+4);
        /* EAPOL-Key information */
        keyInfo = (((u16_t) zmw_rx_buf_readb(dev, buf, offset+5)) << 8) +
                  zmw_rx_buf_readb(dev, buf, offset+6);
        /* EAPOL-Key length */
        keyLen = (((u16_t) zmw_rx_buf_readb(dev, buf, offset+7)) << 8) +
                 zmw_rx_buf_readb(dev, buf, offset+8);
        /* EAPOL-Key replay counter (high double word) */
        replayCounterH = (((u32_t) zmw_rx_buf_readb(dev, buf, offset+9)) << 24) +
                         (((u32_t) zmw_rx_buf_readb(dev, buf, offset+10)) << 16) +
                         (((u32_t) zmw_rx_buf_readb(dev, buf, offset+11)) << 8) +
                         zmw_rx_buf_readb(dev, buf, offset+12);
        /* EAPOL-Key replay counter (low double word) */
        replayCounterL = (((u32_t) zmw_rx_buf_readb(dev, buf, offset+13)) << 24) +
                         (((u32_t) zmw_rx_buf_readb(dev, buf, offset+14)) << 16) +
                         (((u32_t) zmw_rx_buf_readb(dev, buf, offset+15)) << 8) +
                         zmw_rx_buf_readb(dev, buf, offset+16);
        /* EAPOL-Key data length */
        keyDataLen = (((u16_t) zmw_rx_buf_readb(dev, buf, offset+97)) << 8) +
                     zmw_rx_buf_readb(dev, buf, offset+98);

        zm_debug_msg0("EAPOL-Key");
        zm_debug_msg1("packet length = ", packetLen);

        if ( keyType == 254 )
        {
            zm_debug_msg0("key type = 254 (SSN key descriptor)");
        }
        else
        {
            zm_debug_msg2("key type = 0x", keyType);
        }

        zm_debug_msg2("replay counter(L) = ", replayCounterL);

        zm_debug_msg2("key information = ", keyInfo);

        if ( keyInfo & ZM_BIT_3 )
        {
            zm_debug_msg0("    - pairwise key");
        }
        else
        {
            zm_debug_msg0("    - group key");
        }

        if ( keyInfo & ZM_BIT_6 )
        {
            zm_debug_msg0("    - Tx key installed");
        }
        else
        {
            zm_debug_msg0("    - Tx key not set");
        }

        if ( keyInfo & ZM_BIT_7 )
        {
            zm_debug_msg0("    - Ack needed");
        }
        else
        {
            zm_debug_msg0("    - Ack not needed");
        }

        if ( keyInfo & ZM_BIT_8 )
        {
            zm_debug_msg0("    - MIC set");
        }
        else
        {
            zm_debug_msg0("    - MIC not set");
        }

        if ( keyInfo & ZM_BIT_9 )
        {
            zm_debug_msg0("    - packet encrypted");
        }
        else
        {
            zm_debug_msg0("    - packet not encrypted");
        }

        zm_debug_msg1("keyLen = ", keyLen);
        zm_debug_msg1("keyDataLen = ", keyDataLen);
    }
    else if( packetType == 4 )
    {
        zm_debug_msg0("EAPOL-Encapsulated-ASF-Alert");
    }
}

void zfShowTxEAPOL(zdev_t* dev, zbuf_t* buf, u16_t offset)
{
    u8_t   packetType, keyType, code, identifier, type, flags;
    u16_t  packetLen, keyInfo, keyLen, keyDataLen, length, Op_Code;
    u32_t  replayCounterH, replayCounterL, vendorId, VendorType;

    zmw_get_wlan_dev(dev);

    zm_debug_msg1("EAPOL Packet size = ", zfwBufGetSize(dev, buf));

    /* EAPOL packet type */
    // 0: EAP-Packet
    // 1: EAPOL-Start
    // 2: EAPOL-Logoff
    // 3: EAPOL-Key
    // 4: EAPOL-Encapsulated-ASF-Alert

    /* EAPOL frame format */
    /*  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15   */
    /* -----------------------------------------------   */
    /*            PAE Ethernet Type (0x888e)             */
    /* ----------------------------------------------- 2 */
    /*     Protocol Version    |         Type            */
    /* ----------------------------------------------- 4 */
    /*                       Length                      */
    /* ----------------------------------------------- 6 */
    /*                    Packet Body                    */
    /* ----------------------------------------------- N */

    packetType = zmw_tx_buf_readb(dev, buf, offset+1);
    /* EAPOL body length */
    packetLen = (((u16_t) zmw_tx_buf_readb(dev, buf, offset+2)) << 8) +
                zmw_tx_buf_readb(dev, buf, offset+3);

    if( packetType == 0 )
    { // EAP-Packet
        /* EAP-Packet Code */
        code = zmw_tx_buf_readb(dev, buf, offset+4); // 1 : Request
                                                     // 2 : Response
                                                     // 3 : Success
                                                     // 4 : Failure

        // An EAP packet of the type of Success and Failure has no Data field, and has a length of 4.

        /* EAP Packet format */
        /*  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15   */
        /* -----------------------------------------------   */
        /*           Code          |        Identifier       */
        /* ----------------------------------------------- 2 */
        /*                       Length                      */
        /* ----------------------------------------------- 4 */
        /*                        Data                       */
        /* ----------------------------------------------- N */

        zm_debug_msg0("EAP-Packet");
        zm_debug_msg1("Packet Length = ", packetLen);
        zm_debug_msg1("EAP-Packet Code = ", code);

        if( code == 1 )
        {
            zm_debug_msg0("EAP-Packet Request");

            /* EAP-Packet Identifier */
            identifier = zmw_tx_buf_readb(dev, buf, offset+5);
            /* EAP-Packet Length */
            length = (((u16_t) zmw_tx_buf_readb(dev, buf, offset+6)) << 8) +
                      zmw_tx_buf_readb(dev, buf, offset+7);
            /* EAP-Packet Type */
            type = zmw_tx_buf_readb(dev, buf, offset+8); // 1   : Identity
                                                         // 2   : Notification
                                                         // 3   : Nak (Response Only)
                                                         // 4   : MD5-Challenge
                                                         // 5   : One Time Password (OTP)
                                                         // 6   : Generic Token Card (GTC)
                                                         // 254 : (Expanded Types)Wi-Fi Protected Setup
                                                         // 255 : Experimental Use

            /* The data field in an EAP packet of the type of Request or Response is in the format shown bellowing */
            /*  0  1  2  3  4  5  6  7             N             */
            /* -----------------------------------------------   */
            /*           Type          |        Type Data        */
            /* -----------------------------------------------   */

            zm_debug_msg1("EAP-Packet Identifier = ", identifier);
            zm_debug_msg1("EAP-Packet Length = ", length);
            zm_debug_msg1("EAP-Packet Type = ", type);

            if( type == 1 )
            {
                zm_debug_msg0("EAP-Packet Request Identity");
            }
            else if( type == 2 )
            {
                zm_debug_msg0("EAP-Packet Request Notification");
            }
            else if( type == 4 )
            {
                zm_debug_msg0("EAP-Packet Request MD5-Challenge");
            }
            else if( type == 5 )
            {
                zm_debug_msg0("EAP-Packet Request One Time Password");
            }
            else if( type == 6 )
            {
                zm_debug_msg0("EAP-Packet Request Generic Token Card");
            }
            else if( type == 254 )
            {
                zm_debug_msg0("EAP-Packet Request Wi-Fi Protected Setup");

                /* 0                   1                   2                   3   */
                /* 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
                /*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
                /*|     Type      |               Vendor-Id                       |*/
                /*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
                /*|                          Vendor-Type                          |*/
                /*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
                /*|              Vendor data...                                    */
                /*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                        */

                /* EAP-Packet Vendor ID */
                vendorId = (((u32_t) zmw_tx_buf_readb(dev, buf, offset+9)) << 16) +
                           (((u32_t) zmw_tx_buf_readb(dev, buf, offset+10)) << 8) +
                           zmw_tx_buf_readb(dev, buf, offset+11);
                /* EAP-Packet Vendor Type */
                VendorType = (((u32_t) zmw_tx_buf_readb(dev, buf, offset+12)) << 24) +
                             (((u32_t) zmw_tx_buf_readb(dev, buf, offset+13)) << 16) +
                             (((u32_t) zmw_tx_buf_readb(dev, buf, offset+14)) << 8) +
                             zmw_tx_buf_readb(dev, buf, offset+15);
                /* EAP-Packet Op Code */
                Op_Code = (((u16_t) zmw_tx_buf_readb(dev, buf, offset+16)) << 8) +
                          zmw_tx_buf_readb(dev, buf, offset+17);
                /* EAP-Packet Flags */
                flags = zmw_tx_buf_readb(dev, buf, offset+18);

                zm_debug_msg1("EAP-Packet Vendor ID = ", vendorId);
                zm_debug_msg1("EAP-Packet Venodr Type = ", VendorType);
                zm_debug_msg1("EAP-Packet Op Code = ", Op_Code);
                zm_debug_msg1("EAP-Packet Flags = ", flags);
            }
        }
        else if( code == 2 )
        {
            zm_debug_msg0("EAP-Packet Response");

            /* EAP-Packet Identifier */
            identifier = zmw_tx_buf_readb(dev, buf, offset+5);
            /* EAP-Packet Length */
            length = (((u16_t) zmw_tx_buf_readb(dev, buf, offset+6)) << 8) +
                      zmw_tx_buf_readb(dev, buf, offset+7);
            /* EAP-Packet Type */
            type = zmw_tx_buf_readb(dev, buf, offset+8);

            zm_debug_msg1("EAP-Packet Identifier = ", identifier);
            zm_debug_msg1("EAP-Packet Length = ", length);
            zm_debug_msg1("EAP-Packet Type = ", type);

            if( type == 1 )
            {
                zm_debug_msg0("EAP-Packet Response Identity");
            }
            else if( type == 2 )
            {
                zm_debug_msg0("EAP-Packet Request Notification");
            }
            else if( type == 3 )
            {
                zm_debug_msg0("EAP-Packet Request Nak");
            }
                 |       |
   |          #                |                            #          |       |
   |          #                |                            #          |       |
   |          #                |yres                        #          |       |
   |          #                |                            #          |       |
   |          #                |                            #          |       |
   |          #                |                            #          |       |
   |          #                |                            #          |       |
   |          #                |                            #          |       |
   |          #                |                            #          |       |
   |          #                |                            #          |       |
   |          #                |                            #          |       |
   |          #                v                            #          |       |
   +----------###############################################----------+-------+
   |          |                ^                            |          |       |
   |          |                |lower_margin                |          |       |
   |          |                v                            |          |       |
   +----------+---------------------------------------------+----------+-------+
   |          |                ^                            |          |       |
   |          |                |vsync_len                   |          |       |
   |          |                v                            |          |       |
   +----------+---------------------------------------------+----------+-------+


   Amiga video timings
   -------------------

   The Amiga native chipsets uses another timing scheme:

      - hsstrt:   Start of horizontal synchronization pulse
      - hsstop:   End of horizontal synchronization pulse
      - htotal:   Last value on the line (i.e. line length = htotal+1)
      - vsstrt:   Start of vertical synchronization pulse
      - vsstop:   End of vertical synchronization pulse
      - vtotal:   Last line value (i.e. number of lines = vtotal+1)
      - hcenter:  Start of vertical retrace for interlace

   You can specify the blanking timings independently. Currently I just set
   them equal to the respective synchronization values:

      - hbstrt:   Start of horizontal blank
      - hbstop:   End of horizontal blank
      - vbstrt:   Start of vertical blank
      - vbstop:   End of vertical blank

   Horizontal values are in color clock cycles (280 ns), vertical values are in
   scanlines.

   (0, 0) is somewhere in the upper-left corner :-)


   Amiga visible window definitions
   --------------------------------

   Currently I only have values for AGA, SHRES (28 MHz dotclock). Feel free to
   make corrections and/or additions.

   Within the above synchronization specifications, the visible window is
   defined by the following parameters (actual register resolutions may be
   different; all horizontal values are normalized with respect to the pixel
   clock):

      - diwstrt_h:   Horizontal start of the visible window
      - diwstop_h:   Horizontal stop+1(*) of the visible window
      - diwstrt_v:   Vertical start of the visible window
      - diwstop_v:   Vertical stop of the visible window
      - ddfstrt:     Horizontal start of display DMA
      - ddfstop:     Horizontal stop of display DMA
      - hscroll:     Horizontal display output delay

   Sprite positioning:

      - sprstrt_h:   Horizontal start-4 of sprite
      - sprstrt_v:   Vertical start of sprite

   (*) Even Commodore did it wrong in the AGA monitor drivers by not adding 1.

   Horizontal values are in dotclock cycles (35 ns), vertical values are in
   scanlines.

   (0, 0) is somewhere in the upper-left corner :-)


   Dependencies (AGA, SHRES (35 ns dotclock))
   -------------------------------------------

   Since there are much more parameters for the Amiga display than for the
   frame buffer interface, there must be some dependencies among the Amiga
   display parameters. Here's what I found out:

      - ddfstrt and ddfstop are best aligned to 64 pixels.
      - the chipset needs 64+4 horizontal pixels after the DMA start before the
        first pixel is output, so diwstrt_h = ddfstrt+64+4 if you want to
        display the first pixel on the line too. Increase diwstrt_h for virtual
        screen panning.
      - the display DMA always fetches 64 pixels at a time (fmode = 3).
      - ddfstop is ddfstrt+#pixels-64.
      - diwstop_h = diwstrt_h+xres+1. Because of the additional 1 this can be 1
        more than htotal.
      - hscroll simply adds a delay to the display output. Smooth horizontal
        panning needs an extra 64 pixels on the left to prefetch the pixels that
        `fall off' on the left.
      - if ddfstrt < 192, the sprite DMA cycles are all stolen by the bitplane
        DMA, so it's best to make the DMA start as late as possible.
      - you really don't want to make ddfstrt < 128, since this will steal DMA
        cycles from the other DMA channels (audio, floppy and Chip RAM refresh).
      - I make diwstop_h and diwstop_v as large as possible.

   General dependencies
   --------------------

      - all values are SHRES pixel (35ns)

                  table 1:fetchstart  table 2:prefetch    table 3:fetchsize
                  ------------------  ----------------    -----------------
   Pixclock     # SHRES|HIRES|LORES # SHRES|HIRES|LORES # SHRES|HIRES|LORES
   -------------#------+-----+------#------+-----+------#------+-----+------
   Bus width 1x #   16 |  32 |  64  #   16 |  32 |  64  #   64 |  64 |  64
   Bus width 2x #   32 |  64 | 128  #   32 |  64 |  64  #   64 |  64 | 128
   Bus width 4x #   64 | 128 | 256  #   64 |  64 |  64  #   64 | 128 | 256

      - chipset needs 4 pixels before the first pixel is output
      - ddfstrt must be aligned to fetchstart (table 1)
      - chipset needs also prefetch (table 2) to get first pixel data, so
        ddfstrt = ((diwstrt_h-4) & -fetchstart) - prefetch
      - for horizontal panning decrease diwstrt_h
      - the length of a fetchline must be aligned to fetchsize (table 3)
      - if fetchstart is smaller than fetchsize, then ddfstrt can a little bit
        moved to optimize use of dma (useful for OCS/ECS overscan displays)
      - ddfstop is ddfstrt+ddfsize-fetchsize
      - If C= didn't change anything for AGA, then at following positions the
        dma bus is already used:
        ddfstrt <  48 -> memory refresh
                <  96 -> disk dma
                < 160 -> audio dma
                < 192 -> sprite 0 dma
                < 416 -> sprite dma (32 per sprite)
      - in accordance with the hardware reference manual a hardware stop is at
        192, but AGA (ECS?) can go below this.

   DMA priorities
   --------------

   Since there are limits on the earliest start value for display DMA and the
   display of sprites, I use the following policy on horizontal panning and
   the hardware cursor:

      - if you want to start display DMA too early, you lose the ability to
        do smooth horizontal panning (xpanstep 1 -> 64).
      - if you want to go even further, you lose the hardware cursor too.

   IMHO a hardware cursor is more important for X than horizontal scrolling,
   so that's my motivation.


   Implementation
   --------------

   ami_decode_var() converts the frame buffer values to the Amiga values. It's
   just a `straightforward' implementation of the above rules.


   Standard VGA timings
   --------------------

               xres  yres    left  right  upper  lower    hsync    vsync
               ----  ----    ----  -----  -----  -----    -----    -----
      80x25     720   400      27     45     35     12      108        2
      80x30     720   480      27     45     30      9      108        2

   These were taken from a XFree86 configuration file, recalculated for a 28 MHz
   dotclock (Amigas don't have a 25 MHz dotclock) and converted to frame buffer
   generic timings.

   As a comparison, graphics/monitor.h suggests the fo
    if (frameType == ZM_WLAN_DATA_FRAME)
    {
        wd->sta.TotalNumberOfReceivePackets++;
        wd->sta.TotalNumberOfReceiveBytes += zfwBufGetSize(dev, buf);
        //zm_debug_msg1("Receive packets     = ", wd->sta.TotalNumberOfReceivePackets);

        //zm_msg0_rx(ZM_LV_0, "Rx data");
        if (wd->wlanMode == ZM_MODE_AP)
        {
            if ((ret = zfApUpdatePsBit(dev, buf, &vap, &uapsdTrig)) != ZM_SUCCESS)
            {
                zfwBufFree(dev, buf, 0);
                return;
            }

            if (((uapsdTrig&0xf) != 0) && ((frameSubtype & 0x80) != 0))
            {
                u8_t ac = zcUpToAc[zmw_buf_readb(dev, buf, 24)&0x7];
                u8_t pktNum;
                u8_t mb;
                u16_t flag;
                u8_t src[6];

                //printk("QoS ctrl=%d\n", zmw_buf_readb(dev, buf, 24));
                //printk("UAPSD trigger, ac=%d\n", ac);

                if (((0x8>>ac) & uapsdTrig) != 0)
                {
                    pktNum = zcMaxspToPktNum[(uapsdTrig>>4) & 0x3];

                    for (ii=0; ii<6; ii++)
                    {
                        src[ii] = zmw_buf_readb(dev, buf, ZM_WLAN_HEADER_A2_OFFSET+ii);
                    }

                    for (ii=0; ii<pktNum; ii++)
                    {
                        //if ((psBuf = zfQueueGet(dev, wd->ap.uapsdQ)) != NULL)
                        if ((psBuf = zfQueueGetWithMac(dev, wd->ap.uapsdQ, src, &mb)) != NULL)
                        {
                            if ((ii+1) == pktNum)
                            {
                                //EOSP anyway
                                flag = 0x100 | (mb<<5);
                            }
                            else
                            {
                                if (mb != 0)
                                {
                                    //more data, not EOSP
                                    flag = 0x20;
                                }
                                else
                                {
                                    //no more data, EOSP
                                    flag = 0x100;
                                }
                            }
                            zfTxSendEth(dev, psBuf, 0, ZM_EXTERNAL_ALLOC_BUF, flag);
                        }

                        if ((psBuf == NULL) || (mb == 0))
                        {
                            if ((ii == 0) && (psBuf == NULL))
                            {
                                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_QOS_NULL, (u16_t*)src, 0, 0, 0);
                            }
                            break;
                        }
                    }
                }
            }

        }
        else if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
        {
            u16_t frameCtrlMSB;
    		u8_t   bssid[6];

            /* Check Is RIFS frame and decide to enable RIFS or not */
            if( wd->sta.EnableHT )
                zfCheckIsRIFSFrame(dev, buf, frameSubtype);

            if ( zfPowerSavingMgrIsSleeping(dev) || wd->sta.psMgr.tempWakeUp == 1)
            {
                frameCtrlMSB = zmw_rx_buf_readb(dev, buf, 1);

                /* check more data */
                if ( frameCtrlMSB & ZM_BIT_5 )
                {
                    //if rx frame's AC is not delivery-enabled
                    if ((wd->sta.qosInfo&0xf) != 0xf)
                    {
                        u8_t rxAc = 0;
                        if ((frameSubtype & 0x80) != 0)
                        {
                            rxAc = zcUpToAc[zmw_buf_readb(dev, buf, 24)&0x7];
                        }

                        if (((0x8>>rxAc) & wd->sta.qosInfo) == 0)
                        {
                            zfSendPSPoll(dev);
                            wd->sta.psMgr.tempWakeUp = 0;
                        }
                    }
                }
            }
			/*increase beacon count when receive vaild data frame from AP*/
        	ZM_MAC_WORD_TO_BYTE(wd->sta.bssid, bssid);

			if (zfStaIsConnected(dev)&&
				zfRxBufferEqualToStr(dev, buf, bssid, ZM_WLAN_HEADER_A2_OFFSET, 6))
			{
                wd->sta.rxBeaconCount++;
			}
        }

        zm_msg1_rx(ZM_LV_2, "Rx VAP=", vap);

        /* handle IV, EXT-IV, ICV, and EXT-ICV */
        zfGetRxIvIcvLength(dev, buf, vap, &offset, &tailLen, addInfo);

        zfStaIbssPSCheckState(dev, buf);
        //QoS data frame
        if ((frameSubtype & 0x80) == 0x80)
        {
            offset += 2;
        }

        len = zfwBufGetSize(dev, buf);
        /* remove ICV */
        if (tailLen > 0)
        {
            if (len > tailLen)
            {
                len -= tailLen;
                zfwBufSetSize(dev, buf, len);
            }
        }

        /* Filter NULL data */
        if (((frameSubtype&0x40) != 0) || ((len = zfwBufGetSize(dev, buf))<=24))
        {
            zm_msg1_rx(ZM_LV_1, "Free Rx NULL data, len=", len);
            zfwBufFree(dev, buf, 0);
            return;
        }

        /* check and handle defragmentation */
        if ( wd->sta.bSafeMode && (wd->sta.wepStatus == ZM_ENCRYPTION_AES) && wd->sta.SWEncryptEnable )
        {
            zm_msg0_rx(ZM_LV_1, "Bypass defragmentation packets in safe mode");
        }
        else
        {
            if ( (buf = zfDefragment(dev, buf, &bIsDefrag, addInfo)) == NULL )
            {
                /* In this case, the buffer has been freed in zfDefragment */
                return;
            }
        }

        ret = ZM_MIC_SUCCESS;

        /* If SW WEP/TKIP are not turned on */
        if ((wd->sta.SWEncryptEnable & ZM_SW_TKIP_DECRY_EN) == 0 &&
            (wd->sta.SWEncryptEnable & ZM_SW_WEP_DECRY_EN) == 0)
        {
            encryMode = zfGetEncryModeFromRxStatus(addInfo);

            /* check if TKIP */
            if ( encryMode == ZM_TKIP )
            {
                if ( bIsDefrag )
                {
                    ret = zfMicRxVerify(dev, buf);
                }
                else
                {
                    /* check MIC failure bit */
                    if ( ZM_RX_STATUS_IS_MIC_FAIL(addInfo) )
                    {
                        ret = ZM_MIC_FAILURE;
                    }
                }

                if ( ret == ZM_MIC_FAILURE )
                {
                    u8_t Unicast_Pkt = 0x0;

                    if ((zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET) & 0x1) == 0)
                    {
                        wd->commTally.swRxUnicastMicFailCount++;
                        Unicast_Pkt = 0x1;
                    }/*
                    else if (zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET) == 0xffff)
                    {
                        wd->commTally.swRxMulticastMicFailCount++;
                    }*/
                    else
                    {
                        wd->commTally.swRxMulticastMicFailCount++;
                    }
                    if ( wd->wlanMode == ZM_MODE_AP )
                    {
                        u16_t idx;
                        u8_t addr[6];

                        for (idx=0; idx<6; idx++)
                        {
                            addr[idx] = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_A2_OFFSET+idx);
                        }

                        if (wd->zfcbApMicFailureNotify != NULL)
                        {
                            wd->zfcbApMicFailureNotify(dev, addr, buf);
                        }
                    }
                    else
                    {
                        if(Unicast_Pkt)
                        {
                            zm_debug_msg0("Countermeasure : Unicast_Pkt ");
                        }
                        else
                        {
                            zm_debug_msg0("Countermeasure : Non-Unicast_Pkt ");
                        }

                        if((wd->TKIP_Group_KeyChanging == 0x0) || (Unicast_Pkt == 0x1))
                        {
                            zm_debug_msg0("Countermeasure : Do MIC Check ");
                            zfStaMicFailureHandling(dev, buf);
                        }
                        else
                        {
                            zm_debug_msg0("Countermeasure : SKIP MIC Check due to Group Keychanging ");
                        }
                    }
                    /* Discard MIC failed frame */
                    zfwBufFree(dev, buf, 0);
                    return;
                }
            }
        }
        else
        {
            u8_t IsEncryFrame;

            /* TODO: Check whether WEP bit is turned on in MAC header */
            encryMode = ZM_NO_WEP;

            IsEncryFrame = (zmw_rx_buf_readb(dev, buf, 1) & 0x40);

            if (IsEncryFrame)
            {
                /* Software decryption for TKIP */
                if (wd->sta.SWEncryptEnable & ZM_SW_TKIP_DECRY_EN)
                {
                    u16_t iv16;
                    u16_t iv32;
                    u8_t RC4Key[16];
                    u16_t IvOffset;
                    struct zsTkipSeed *rxSeed;

                    IvOffset = offset + ZM_SIZE_OF_WLAN_DATA_HEADER;

                    rxSeed = zfStaGetRxSeed(dev, buf);

                    if (rxSeed == NULL)
                    {
                        zm_debug_msg0("rxSeed is NULL");

                        /* Discard this frame */
                        zfwBufFree(dev, buf, 0);
                        return;
                    }

                    iv16 = (zmw_rx_buf_readb(dev, buf, IvOffset) << 8) + zmw_rx_buf_readb(dev, buf, IvOffset+2);
                    iv32 = zmw_rx_buf_readb(dev, buf, IvOffset+4) +
                           (zmw_rx_buf_readb(dev, buf, IvOffset+5) << 8) +
                           (zmw_rx_buf_readb(dev, buf, IvOffset+6) << 16) +
                           (zmw_rx_buf_readb(dev, buf, IvOffset+7) << 24);

                    /* TKIP Key Mixing */
                    zfTkipPhase1KeyMix(iv32, rxSeed);
                    zfTkipPhase2KeyMix(iv16, rxSeed);
                    zfTkipGetseeds(iv16, RC4Key, rxSeed);

                    /* Decrypt Data */
                    ret = zfTKIPDecrypt(dev, buf, IvOffset+ZM_SIZE_OF_IV+ZM_SIZE_OF_EXT_IV, 16, RC4Key);

                    if (ret == ZM_ICV_FAILURE)
                    {
                        zm_debug_msg0("TKIP ICV fail");

                        /* Discard ICV failed frame */
                        zfwBufFree(dev, buf, 0);
                        return;
                    }

                    /* Remove ICV from buffer */
                    zfwBufSetSize(dev, buf, len-4);

                    /* Check MIC */
                    ret = zfMicRxVerify(dev, buf);

                    if (ret == ZM_MIC_FAILURE)
                    {
                        if ((zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET) & 0x1) == 0)
                        {
                            wd->commTally.swRxUnicastMicFailCount++;
                        }
                        else if (zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET) == 0xffff)
                        {
                            wd->commTally.swRxMulticastMicFailCount++;
                        }
                        else
                        {
                            wd->commTally.swRxMulticastMicFailCount++;
                        }
                        if ( wd->wlanMode == ZM_MODE_AP )
                        {
                            u16_t idx;
                            u8_t addr[6];

                            for (idx=0; idx<6; idx++)
                            {
                                addr[idx] = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_A2_OFFSET+idx);
                            }

                            if (wd->zfcbApMicFailureNotify != NULL)
                            {
                                wd->zfcbApMicFailureNotify(dev, addr, buf);
                            }
                        }
                        else
                        {
                            zfStaMicFailureHandling(dev, buf);
                        }

                        zm_debug_msg0("MIC fail");
                        /* Discard MIC failed frame */
                        zfwBufFree(dev, buf, 0);
                        return;
                    }

                    encryMode = ZM_TKIP;
                    offset += ZM_SIZE_OF_IV + ZM_SIZE_OF_EXT_IV;
                }
                else if(wd->sta.SWEncryptEnable & ZM_SW_WEP_DECRY_EN)
                {
                    u16_t IvOffset;
                    u8_t keyLen = 5;
                    u8_t iv[3];
                    u8_t *wepKey;
                    u8_t keyIdx;

                    IvOffset = offset + ZM_SIZE_OF_WLAN_DATA_HEADER;

                    /* Retrieve IV */
                    iv[0] = zmw_rx_buf_readb(dev, buf, IvOffset);
                    iv[1] = zmw_rx_buf_readb(dev, buf, IvOffset+1);
                    iv[2] = zmw_rx_buf_readb(dev, buf, IvOffset+2);

                    keyIdx = ((zmw_rx_buf_readb(dev, buf, IvOffset+3) >> 6) & 0x03);

                    IvOffset += ZM_SIZE_OF_IV;

                    if (wd->sta.SWEncryMode[keyIdx] == ZM_WEP64)
                    {
                        keyLen = 5;
                    }
                    else if (wd->sta.SWEncryMode[keyIdx] == ZM_WEP128)
                    {
                        keyLen = 13;
                    }
                    else if (wd->sta.SWEncryMode[keyIdx] == ZM_WEP256)
                    {
                        keyLen = 29;
                    }

                    zfWEPDecrypt(dev, buf, IvOffset, keyLen, wd->sta.wepKey[keyIdx], iv);

                    if (ret == ZM_ICV_FAILURE)
                    {
                        zm_debug_msg0("WEP ICV fail");

                        /* Discard ICV failed frame */
                        zfwBufFree(dev, buf, 0);
                        return;
                    }

                    encryMode = wd->sta.SWEncryMode[keyIdx];

                    /* Remove ICV from buffer */
                    zfwBufSetSize(dev, buf, len-4);

                    offset += ZM_SIZE_OF_IV;
                }
            }
        }

#ifdef ZM_ENABLE_CENC
        //else if ( encryMode == ZM_CENC ) /* check if CENC */
        if ( encryMode == ZM_CENC )
        {
            u32_t rxIV[4];

            rxIV[0] = (zmw_rx_buf_readh(dev, buf, 28) << 16)
                     + zmw_rx_buf_readh(dev, buf, 26);
            rxIV[1] = (zmw_rx_buf_readh(dev, buf, 32) << 16)
                     + zmw_rx_buf_readh(dev, buf, 30);
            rxIV[2] = (zmw_rx_buf_readh(dev, buf, 36) << 16)
                     + zmw_rx_buf_readh(dev, buf, 34);
            rxIV[3] = (zmw_rx_buf_readh(dev, buf, 40) << 16)
                     + zmw_rx_buf_readh(dev, buf, 38);

            //zm_debug_msg2("rxIV[0] = 0x", rxIV[0]);
            //zm_debug_msg2("rxIV[1] = 0x", rxIV[1]);
            //zm_debug_msg2("rxIV[2] = 0x", rxIV[2]);
            //zm_debug_msg2("rxIV[3] = 0x", rxIV[3]);

            /* destination address*/
            da[0] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET);
            da[1] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET+2);
            da[2] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET+4);

            if ( wd->wlanMode == ZM_MODE_AP )
            {
            }
            else
            {
                if ((da[0] & 0x1))
                { //multicast frame
                    /* Accumlate the PN sequence */
                    wd->sta.rxivGK[0] ++;

                    if (wd->sta.rxivGK[0] == 0)
                    {
                        wd->sta.rxivGK[1]++;
                    }

                    if (wd->sta.rxivGK[1] == 0)
                    {
                        wd->sta.rxivGK[2]++;
                    }

                    if (wd->sta.rxivGK[2] == 0)
                    {
                        wd->sta.rxivGK[3]++;
                    }

                    if (wd->sta.rxivGK[3] == 0)
                    {
                        wd->sta.rxivGK[0] = 0;
                        wd->sta.rxivGK[1] = 0;
                        wd->sta.rxivGK[2] = 0;
                    }

                    //zm_debug_msg2("wd->sta.rxivGK[0] = 0x", wd->sta.rxivGK[0]);
                    //zm_debug_msg2("wd->sta.rxivGK[1] = 0x", wd->sta.rxivGK[1]);
                    //zm_debug_msg2("wd->sta.rxivGK[2] = 0x", wd->sta.rxivGK[2]);
                    //zm_debug_msg2("wd->sta.rxivGK[3] = 0x", wd->sta.rxivGK[3]);

                    if ( !((wd->sta.rxivGK[0] == rxIV[0])
                        && (wd->sta.rxivGK[1] == rxIV[1])
                        && (wd->sta.rxivGK[2] == rxIV[2])
                        && (wd->sta.rxivGK[3] == rxIV[3])))
                    {
                        u8_t PacketDiscard = 0;
                        /* Discard PN Code Error frame */
                        if (rxIV[0] < wd->sta.rxivGK[0])
                        {
                            PacketDiscard = 1;
                        }
                        if (wd->sta.rxivGK[0] > 0xfffffff0)
                        { //boundary case
                            if ((rxIV[0] < 0xfffffff0)
                                && (((0xffffffff - wd->sta.rxivGK[0]) + rxIV[0]) > 16))
                            {
                                PacketDiscard = 1;
                            }
                        }
                        else
                        { //normal case
                            if ((rxIV[0] - wd->sta.rxivGK[0]) > 16)
                            {
                                PacketDiscard = 1;
                            }
                        }
                        // sync sta pn code with ap because of losting some packets
                        wd->sta.rxivGK[0] = rxIV[0];
                        wd->sta.rxivGK[1] = rxIV[1];
                        wd->sta.rxivGK[2] = rxIV[2];
                        wd->sta.rxivGK[3] = rxIV[3];
                        if (PacketDiscard)
                        {
                            zm_debug_msg0("Discard PN Code lost too much multicast frame");
                            zfwBufFree(dev, buf, 0);
                            return;
                        }
                    }
                }
                else
                { //unicast frame
                    /* Accumlate the PN sequence */
                    wd->sta.rxiv[0] += 2;

                    if (wd->sta.rxiv[0] == 0 || wd->sta.rxiv[0] == 1)
                    {
                        wd->sta.rxiv[1]++;
                    }

                    if (wd->sta.rxiv[1] == 0)
                    {
                        wd->sta.rxiv[2]++;
                    }

                    if (wd->sta.rxiv[2] == 0)
                    {
                        wd->sta.rxiv[3]++;
                    }

                    if (wd->sta.rxiv[3] == 0)
                    {
                        wd->sta.rxiv[0] = 0;
                        wd->sta.rxiv[1] = 0;
                        wd->sta.rxiv[2] = 0;
                    }

                    //zm_debug_msg2("wd->sta.rxiv[0] = 0x", wd->sta.rxiv[0]);
                    //zm_debug_msg2("wd->sta.rxiv[1] = 0x", wd->sta.rxiv[1]);
                    //zm_debug_msg2("wd->sta.rxiv[2] = 0x", wd->sta.rxiv[2]);
                    //zm_debug_msg2("wd->sta.rxiv[3] = 0x", wd->sta.rxiv[3]);

                    if ( !((wd->sta.rxiv[0] == rxIV[0])
                        && (wd->sta.rxiv[1] == rxIV[1])
                        && (wd->sta.rxiv[2] == rxIV[2])
                        && (wd->sta.rxiv[3] == rxIV[3])))
                    {
                        zm_debug_msg0("PN Code mismatch, lost unicast frame, sync pn code to recv packet");
                        // sync sta pn code with ap because of losting some packets
                        wd->sta.rxiv[0] = rxIV[0];
                        wd->sta.rxiv[1] = rxIV[1];
                        wd->sta.rxiv[2] = rxIV[2];
                        wd->sta.rxiv[3] = rxIV[3];
                        /* Discard PN Code Error frame */
                        //zm_debug_msg0("Discard PN Code mismatch unicast frame");
                        //zfwBufFree(dev, buf, 0);
                        //return;
                    }
                }
            }
        }
#endif //ZM_ENABLE_CENC

        /* for tally */
        if ((zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET) & 0x1) == 0)
        {
            /* for ACU to display RxRate */
            zfWlanUpdateRxRate(dev, addInfo);

            wd->commTally.rxUnicastFrm++;
            wd->commTally.rxUnicastOctets += (len-24);
        }
        else if (zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET) == 0xffff)
        {
            wd->commTally.rxBroadcastFrm++;
            wd->commTally.rxBroadcastOctets += (len-24);
        }
        else
        {
            wd->commTally.rxMulticastFrm++;
            wd->commTally.rxMulticastOctets += (len-24);
        }
        wd->ledStruct.rxTraffic++;

        if ((frameSubtype & 0x80) == 0x80)
        {
            /* if QoS control bit-7 is 1 => A-MSDU frame */
            if ((zmw_rx_buf_readh(dev, buf, 24) & 0x80) != 0)
            {
                zfDeAmsdu(dev, buf, vap, encryMode);
                return;
            }
        }

        // Remove MIC of TKIP
        if ( encryMode == ZM_TKIP )
        {
            zfwBufSetSize(dev, buf, zfwBufGetSize(dev, buf) - 8);
        }

        /* Convert 802.11 and SNAP header to ethernet header */
        if ( (wd->wlanMode == ZM_MODE_INFRASTRUCTURE)||
             (wd->wlanMode == ZM_MODE_IBSS) )
        {
            /* destination address*/
            da[0] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET);
            da[1] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET+2);
            da[2] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET+4);

            /* check broadcast frame */
            if ( (da[0] == 0xffff) && (da[1] == 0xffff) && (da[2] == 0xffff) )
            {
                // Ap send broadcast frame to the DUT !
            }
            /* check multicast frame */
            /* TODO : Remove these code, hardware should be able to block */
            /*        multicast frame on the multicast address list       */
            /*        or bypass all multicast packet by flag bAllMulticast */
            else if ((da[0] & 0x01) && (wd->sta.bAllMulticast == 0))
            {
                for(ii=0; ii<wd->sta.multicastList.size; ii++)
                {
                    if ( zfMemoryIsEqual(wd->sta.multicastList.macAddr[ii].addr,
                                         (u8_t*) da, 6))
                    {
                        break;
                    }
                }

                if ( ii == wd->sta.multicastList.size )
                {   /* not found */
                    zm_debug_msg0("discard unknown multicast frame");

                    zfwBufFree(dev, buf, 0);
                    return;
                }
            }

#ifdef ZM_ENABLE_NATIVE_WIFI //Native Wifi : 1, Ethernet format : 0
            //To remove IV
            if (offset > 0)
            {
                for (i=12; i>0; i--)
                {
                    zmw_rx_buf_writeh(dev, buf, ((i-1)*2)+offset,
                            zmw_rx_buf_readh(dev, buf, (i-1)*2));
                }
                zfwBufRemoveHead(dev, buf, offset);
            }
#else

            if (zfRxBufferEqualToStr(dev, buf, zgSnapBridgeTunnel,
                                     24+offset, 6))
            {
                snapCase = 1;
            }
            else if ( zfRxBufferEqualToStr(dev, buf, zgSnap8021h,
                                           24+offset, 6) )
            {
                typeLengthField =
                    (((u16_t) zmw_rx_buf_readb(dev, buf, 30+offset)) << 8) +
                    zmw_rx_buf_readb(dev, buf, 31+offset);

                //zm_debug_msg2("tpyeLengthField = ", typeLengthField);

                //8137 : IPX, 80F3 : Appletalk
                if ( (typeLengthField != 0x8137)&&
                     (typeLengthField != 0x80F3) )
                {
                    snapCase = 2;
                }

                if ( typeLengthField == 0x888E )
                {
         _var(struct fb_var_screeninfo *var,
                          struct amifb_par *par);
static int ami_encode_var(struct fb_var_screeninfo *var,
                          struct amifb_par *par);
static void ami_pan_var(struct fb_var_screeninfo *var);
static int ami_update_par(void);
static void ami_update_display(void);
static void ami_init_display(void);
static void ami_do_blank(void);
static int ami_get_fix_cursorinfo(struct fb_fix_cursorinfo *fix);
static int ami_get_var_cursorinfo(struct fb_var_cursorinfo *var, u_char __user *data);
static int ami_set_var_cursorinfo(struct fb_var_cursorinfo *var, u_char __user *data);
static int ami_get_cursorstate(struct fb_cursorstate *state);
static int ami_set_cursorstate(struct fb_cursorstate *state);
static void ami_set_sprite(void);
static void ami_init_copper(void);
static void ami_reinit_copper(void);
static void ami_build_copper(void);
static void ami_rebuild_copper(void);


static struct fb_ops amifb_ops = {
	.owner		= THIS_MODULE,
	.fb_check_var	= amifb_check_var,
	.fb_set_par	= amifb_set_par,
	.fb_setcolreg	= amifb_setcolreg,
	.fb_blank	= amifb_blank,
	.fb_pan_display	= amifb_pan_display,
	.fb_fillrect	= amifb_fillrect,
	.fb_copyarea	= amifb_copyarea,
	.fb_imageblit	= amifb_imageblit,
	.fb_ioctl	= amifb_ioctl,
};

static void __init amifb_setup_mcap(char *spec)
{
	char *p;
	int vmin, vmax, hmin, hmax;

	/* Format for monitor capabilities is: <Vmin>;<Vmax>;<Hmin>;<Hmax>
	 * <V*> vertical freq. in Hz
	 * <H*> horizontal freq. in kHz
	 */

	if (!(p = strsep(&spec, ";")) || !*p)
		return;
	vmin = simple_strtoul(p, NULL, 10);
	if (vmin <= 0)
		return;
	if (!(p = strsep(&spec, ";")) || !*p)
		return;
	vmax = simple_strtoul(p, NULL, 10);
	if (vmax <= 0 || vmax <= vmin)
		return;
	if (!(p = strsep(&spec, ";")) || !*p)
		return;
	hmin = 1000 * simple_strtoul(p, NULL, 10);
	if (hmin <= 0)
		return;
	if (!(p = strsep(&spec, "")) || !*p)
		return;
	hmax = 1000 * simple_strtoul(p, NULL, 10);
	if (hmax <= 0 || hmax <= hmin)
		return;

	fb_info.monspecs.vfmin = vmin;
	fb_info.monspecs.vfmax = vmax;
	fb_info.monspecs.hfmin = hmin;
	fb_info.monspecs.hfmax = hmax;
}

int __init amifb_setup(char *options)
{
	char *this_opt;

	if (!options || !*options)
		return 0;

	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt)
			continue;
		if (!strcmp(this_opt, "inverse")) {
			amifb_inverse = 1;
			fb_invert_cmaps();
		} else if (!strcmp(this_opt, "ilbm"))
			amifb_ilbm = 1;
		else if (!strncmp(this_opt, "monitorcap:", 11))
			amifb_setup_mcap(this_opt+11);
		else if (!strncmp(this_opt, "fstart:", 7))
			min_fstrt = simple_strtoul(this_opt+7, NULL, 0);
		else
			mode_option = this_opt;
	}

	if (min_fstrt < 48)
		min_fstrt = 48;

	return 0;
}


static int amifb_check_var(struct fb_var_screeninfo *var,
			   struct fb_info *info)
{
	int err;
	struct amifb_par par;

	/* Validate wanted screen parameters */
	if ((err = ami_decode_var(var, &par)))
		return err;

	/* Encode (possibly rounded) screen parameters */
	ami_encode_var(var, &par);
	return 0;
}


static int amifb_set_par(struct fb_info *info)
{
	struct amifb_par *par = (struct amifb_par *)info->par;

	do_vmode_pan = 0;
	do_vmode_full = 0;

	/* Decode wanted screen parameters */
	ami_decode_var(&info->var, par);

	/* Set new videomode */
	ami_build_copper();

	/* Set VBlank trigger */
	do_vmode_full = 1;

	/* Update fix for new screen parameters */
	if (par->bpp == 1) {
		info->fix.type = FB_TYPE_PACKED_PIXELS;
		info->fix.type_aux = 0;
	} else if (amifb_ilbm) {
		info->fix.type = FB_TYPE_INTERLEAVED_PLANES;
		info->fix.type_aux = par->next_line;
	} else {
		info->fix.type = FB_TYPE_PLANES;
		info->fix.type_aux = 0;
	}
	info->fix.line_length = div8(upx(16<<maxfmode, par->vxres));

	if (par->vmode & FB_VMODE_YWRAP) {
		info->fix.ywrapstep = 1;
		info->fix.xpanstep = 0;
		info->fix.ypanstep = 0;
		info->flags = FBINFO_DEFAULT | FBINFO_HWACCEL_YWRAP |
		    FBINFO_READS_FAST; /* override SCROLL_REDRAW */
	} else {
		info->fix.ywrapstep = 0;
		if (par->vmode & FB_VMODE_SMOOTH_XPAN)
			info->fix.xpanstep = 1;
		else
			info->fix.xpa /* Free Rx buffer if intra-BSS unicast frame */
                    zm_msg0_rx(ZM_LV_2, "Free intra-BSS unicast frame");
                    zfwBufFree(dev, buf, 0);
                    return;
                }
                #endif
            }
            else
            /* WDS mode */
            {
                zm_msg0_rx(ZM_LV_2, "Rx WDS data");

                /* SA = Address 4 */
                zmw_rx_buf_writeh(dev, buf, 30+offset, zmw_rx_buf_readh(dev, buf,
                        ZM_WLAN_HEADER_A4_OFFSET));
                zmw_rx_buf_writeh(dev, buf, 32+offset, zmw_rx_buf_readh(dev, buf,
                        ZM_WLAN_HEADER_A4_OFFSET+2));
                zmw_rx_buf_writeh(dev, buf, 34+offset, zmw_rx_buf_readh(dev, buf,
                        ZM_WLAN_HEADER_A4_OFFSET+4));
                /* DA = Address 3 */
                /* Seq : Read 20 write 22, read 18 write 20, read 16 write 18 */
                /* sequence must not be inverted */
                zmw_rx_buf_writeh(dev, buf, 28+offset, zmw_rx_buf_readh(dev, buf,
                        ZM_WLAN_HEADER_A3_OFFSET+4));
                zmw_rx_buf_writeh(dev, buf, 26+offset, zmw_rx_buf_readh(dev, buf,
                        ZM_WLAN_HEADER_A3_OFFSET+2));
                zmw_rx_buf_writeh(dev, buf, 24+offset, zmw_rx_buf_readh(dev, buf,
                        ZM_WLAN_HEADER_A3_OFFSET));
                zfwBufRemoveHead(dev, buf, 24+offset);
            }
        }
        else if (wd->wlanMode == ZM_MODE_PSEUDO)
        {
			/* WDS test: remove add4 */
            if (wd->enableWDS)
            {
                offset += 6;
            }

            /* SA = Address 2 */
            zmw_rx_buf_writeh(dev, buf, 24+offset, zmw_rx_buf_readh(dev, buf,
                              ZM_WLAN_HEADER_A2_OFFSET));
            zmw_rx_buf_writeh(dev, buf, 26+offset, zmw_rx_buf_readh(dev, buf,
                              ZM_WLAN_HEADER_A2_OFFSET+2));
            zmw_rx_buf_writeh(dev, buf, 28+offset, zmw_rx_buf_readh(dev, buf,
                              ZM_WLAN_HEADER_A2_OFFSET+4));
            /* DA = Address 1 */
            zmw_rx_buf_writeh(dev, buf, 18+offset, zmw_rx_buf_readh(dev, buf,
                              ZM_WLAN_HEADER_A1_OFFSET));
            zmw_rx_buf_writeh(dev, buf, 20+offset, zmw_rx_buf_readh(dev, buf,
                              ZM_WLAN_HEADER_A1_OFFSET+2));
            zmw_rx_buf_writeh(dev, buf, 22+offset, zmw_rx_buf_readh(dev, buf,
                              ZM_WLAN_HEADER_A1_OFFSET+4));
            zfwBufRemoveHead(dev, buf, 18+offset);
        }
        else
        {
            zm_assert(0);
        }

        /* Call zfwRecvEth() to notify upper layer */
        //zm_msg2_rx(ZM_LV_2, "Call zfwRecvEth(), buf=", buf);
        //zfwDumpBuf(dev, buf);

        #if ZM_PROTOCOL_RESPONSE_SIMULATION == 1
        zfProtRspSim(dev, buf);
        #endif
        //zfwDumpBuf(dev, buf);

        /* tally */
    	wd->commTally.NotifyNDISRxFrmCnt++;

    	if (wd->zfcbRecvEth != NULL)
    	{
            wd->zfcbRecvEth(dev, buf, vap);
            ZM_PERFORMANCE_RX_MSDU(dev, wd->tick)
        }
    }
    /* if management frame */
    else if (frameType == ZM_WLAN_MANAGEMENT_FRAME)
    {
        zm_msg2_rx(ZM_LV_2, "Rx management,FC=", frameCtrl);
        /* Call zfProcessManagement() to handle management frame */
        zfProcessManagement(dev, buf, addInfo); //CWYang(m)
        zfwBufFree(dev, buf, 0);
    }
    /* PsPoll */
    else if ((wd->wlanMode == ZM_MODE_AP) && (frameCtrl == 0xa4))
    {
        zm_msg0_rx(ZM_LV_0, "Rx PsPoll");
        zfApProcessPsPoll(dev, buf);
        zfwBufFree(dev, buf, 0);
    }
    else
    {
        zm_msg0_rx(ZM_LV_1, "Rx discard!!");
        wd->commTally.DriverDiscardedFrm++;

        zfwBufFree(dev, buf, 0);
    }
    return;
}


/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfWlanRxValidate            */
/*      Validate Rx frame.                                              */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : received 802.11 frame buffer.                             */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      Error code                                                      */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen             ZyDAS Technology Corporation    2005.10     */
/*                                                                      */
/************************************************************************/
u16_t zfWlanRxValidate(zdev_t* dev, zbuf_t* buf)
{
    u16_t frameType;
    u16_t frameCtrl;
    u16_t frameLen;
    u16_t ret;
    u8_t  frameSubType;

    zmw_get_wlan_dev(dev);

    frameCtrl = zmw_rx_buf_readh(dev, buf, 0);
    frameType = frameCtrl & 0xC;
    frameSubType = (frameCtrl & 0xF0) >> 4;

    frameLen = zfwBufGetSize(dev, buf);

    /* Accept Data/Management frame with protocol version = 0 */
    if ((frameType == 0x8) || (frameType == 0x0))
    {

        /* TODO : check rx status => erro bit */

        /* Check Minimum Length with Wep */
        if ((frameCtrl & 0x4000) != 0)
        {
            /* Minimum Length =                                       */
            /*     PLCP(5)+Header(24)+IV(4)+ICV(4)+CRC(4)+RxStatus(8) */
            if (frameLen < 32)
            {
                return ZM_ERR_MIN_RX_ENCRYPT_FRAME_LENGTH;
            }
        }
        else if ( frameSubType == 0x5 || frameSubType == 0x8 )
        {
            /* Minimum Length = PLCP(5)+MACHeader(24)+Timestamp(8)+BeaconInterval(2)+Cap(2)+CRC(4)+RxStatus(8) */
            if (frameLen < 36)
            {
                return ZM_ERR_MIN_RX_FRAME_LENGTH;
            }
        }
        else
        {
            /* Minimum Length = PLCP(5)+MACHeader(24)+CRC(4)+RxStatus(8) */
            if (frameLen < 24)
            {
                return ZM_ERR_MIN_RX_FRAME_LENGTH;
            }
        }

        /* Check if frame Length > ZM_WLAN_MAX_RX_SIZE. */
        if (frameLen > ZM_WLAN_MAX_RX_SIZE)
        {
            return ZM_ERR_MAX_RX_FRAME_LENGTH;
        }
    }
    else if ((frameCtrl&0xff) == 0xa4)
    {
        /* PsPoll */
        //zm_msg0_rx(ZM_LV_0, "rx pspoll");
    }
    else if ((frameCtrl&0xff) == ZM_WLAN_FRAME_TYPE_BAR)
    {
        if (wd->sta.enableDrvBA == 1)
        {
            zfAggRecvBAR(dev, buf);
        }

        return ZM_ERR_RX_BAR_FRAME;
    }
    else
    {
        return ZM_ERR_RX_FRAME_TYPE;
    }

    if ( wd->wlanMode == ZM_MODE_AP )
    {
    }
    else if ( wd->wlanMode != ZM_MODE_PSEUDO )
    {
        if ( (ret=zfStaRxValidateFrame(dev, buf))!=ZM_SUCCESS )
        {
            //zm_debug_msg1("discard frame, code = ", ret);
            return ret;
        }
    }

    return ZM_SUCCESS;
}


/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfWlanRxFilter              */
/*      Filter duplicated frame.                                        */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : received 802.11 frame buffer.                             */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      Error code                                                      */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen             ZyDAS Technology Corporation    2005.10     */
/*                                                                      */
/************************************************************************/
u16_t zfWlanRxFilter(zdev_t* dev, zbuf_t* buf)
{
    u16_t src[3];
    u16_t dst0;
    u16_t frameType;
    u16_t seq;
    u16_t offset;
    u16_t index;
    u16_t col;
    u16_t i;
    u8_t up = 0; /* User priority */

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    ZM_BUFFER_TRACE(dev, buf)

    /* RX PREFIX */
    offset = 0;

    frameType = zmw_rx_buf_readh(dev, buf, offset);

    // Don't divide 2^4 because we don't want the fragementation pkt to be treated as
    // duplicated frames
    seq = zmw_rx_buf_readh(dev, buf, offset+22);
    dst0 = zmw_rx_buf_readh(dev, buf, offset+4);
    src[0] = zmw_rx_buf_readh(dev, buf, offset+10);
    src[1] = zmw_rx_buf_readh(dev, buf, offset+12);
    src[2] = zmw_rx_buf_readh(dev, buf, offset+14);

    /* QoS data frame */
    if ((frameType & 0x88) == 0x88)
    {
        up = zmw_rx_buf_readb(dev, buf, offset+24);
        up &= 0x7;
    }

    index = (src[2]+up) & (ZM_FILTER_TABLE_ROW-1);

    /* TBD : filter frame with source address == own MAC adress */
    if ((wd->macAddr[0] == src[0]) && (wd->macAddr[1] == src[1])
            && (wd->macAddr[2] == src[2]))
    {
        //zm_msg0_rx(ZM_LV_0, "Rx filter=>src is own MAC");
        wd->trafTally.rxSrcIsOwnMac++;
#if 0
        return ZM_ERR_RX_SRC_ADDR_IS_OWN_MAC;
#endif
    }

    zm_msg2_rx(ZM_LV_2, "Rx seq=", seq);

    /* Filter unicast frame only */
    if ((dst0 & 0x1) == 0)
    {
        zmw_enter_critical_section(dev);

        for(i=0; i<ZM_FILTER_TABLE_COL; i++)
        {
            if ((wd->rxFilterTbl[i][index].addr[0] == src[0])
                    && (wd->rxFilterTbl[i][index].addr[1] == src[1])
                    && (wd->rxFilterTbl[i][index].addr[2] == src[2])
                    && (wd->rxFilterTbl[i][index].up == up))
            {
                if (((frameType&0x800)==0x800)
                        &&(wd->rxFilterTbl[i][index].seq==seq))
                {
                    zmw_leave_critical_section(dev);
                    /* hit : duplicated frame */
                    zm_msg0_rx(ZM_LV_1, "Rx filter hit=>duplicated");
                    wd->trafTally.rxDuplicate++;
                    return ZM_ERR_RX_DUPLICATE;
                }
                else
                {
                    /* hit : not duplicated frame, update sequence number */
                    wd->rxFilterTbl[i][index].seq = seq;
                    zmw_leave_critical_section(dev);
                    zm_msg0_rx(ZM_LV_2, "Rx filter hit");
                    return ZM_SUCCESS;
                }
            }
        } /* for(i=0; i<ZM_FILTER_TABLE_COL; i++) */

        /* miss : add to table */
        zm_msg0_rx(ZM_LV_1, "Rx filter miss");
        /* TODO : Random select a column */
        col = (u16_t)(wd->tick & (ZM_FILTER_TABLE_COL-1));
        wd->rxFilterTbl[col][index].addr[0] = src[0];
        wd->rxFilterTbl[col][index].addr[1] = src[1];
        wd->rxFilterTbl[col][index].addr[2] = src[2];
        wd->rxFilterTbl[col][index].seq = seq;
        wd->rxFilterTbl[col][index].up = up;

        zmw_leave_critical_section(dev);
    } /* if ((dst0 & 0x1) == 0) */

    return ZM_SUCCESS;
}



u16_t zfTxGenWlanTail(zdev_t* dev, zbuf_t* buf, u16_t* snap, u16_t snaplen,
                      u16_t* mic)
{
    struct zsMicVar*  pMicKey;
    u16_t  i, length, payloadOffset;
    u8_t   bValue, qosType = 0;
    u8_t   snapByte[12];

    zmw_get_wlan_dev(dev);

    if ( wd->wlanMode == ZM_MODE_AP )
    {
        pMicKey = zfApGetTxMicKey(dev, buf, &qosType);

        if ( pMicKey == NULL )
        {
            return 0;
        }
    }
    else if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
    {
        pMicKey = zfStaGetTxMicKey(dev, buf);

        if ( pMicKey == NULL )
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    length = zfwBufGetSize(dev, buf);

    zfMicClear(pMicKey);

    /* append DA and SA */
#ifdef ZM_ENABLE_NATIVE_WIFI
    for(i=16; i<22; i++)
    { // VISTA DA
        bValue = zmw_tx_buf_readb(dev, buf, i);
        zfMicAppendByte(bValue, pMicKey);
    }
    for(i=10; i<16; i++)
    { // VISTA SA
        bValue = zmw_tx_buf_readb(dev, buf, i);
        zfMicAppendByte(bValue, pMicKey);
    }
#else
    for(i=0; i<12; i++)
    {
        bValue = zmw_tx_buf_readb(dev, buf, i);
        zfMicAppendByte(bValue, pMicKey);
    }
#endif

    /* append for alignment */
    if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
    {
        if (wd->sta.wmeConnected != 0)
            zfMicAppendByte(zmw_tx_buf_readb(dev, buf, ZM_80211_FRAME_IP_OFFSET + 1) >> 5, pMicKey);
        else
            zfMicAppendByte(0, pMicKey);
    }
    else if ( wd->wlanMode == ZM_MODE_AP )
    {
        if (qosType == 1)
            zfMicAppendByte(zmw_tx_buf_readb(dev, buf, ZM_80211_FRAME_IP_OFFSET + 1) >> 5, pMicKey);
        else
            zfMicAppendByte(0, pMicKey);
    }
    else
    {
        /* TODO : Qos Software MIC in IBSS Mode */
        zfMicAppendByte(0, pMicKey);
    }
    zfMicAppendByte(0, pMicKey);
    zfMicAppendByte(0, pMicKey);
    zfMicAppendByte(0, pMicKey);

    if ( snaplen == 0 )
    {
        payloadOffset = ZM_80211_FRAME_IP_OFFSET;
    }
    else
    {
        payloadOffset = ZM_80211_FRAME_TYPE_OFFSET;

        for(i=0; i<(snaplen>>1); i++)
        {
            snapByte[i*2] = (u8_t) (snap[i] & 0xff);
            snapByte[i*2+1] = (u8_t) ((snap[i] >> 8) & 0xff);
        }

        for(i=0; i<snaplen; i++)
        {
            zfMicAppendByte(snapByte[i], pMicKey);
        }
    }

    for(i=payloadOffset; i<length; i++)
    {
        bValue = zmw_tx_buf_readb(dev, buf, i);
        zfMicAppendByte(bValue, pMicKey);
    }

    zfMicGetMic( (u8_t*) mic, pMicKey);

    return ZM_SIZE_OF_MIC;
}


/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfTxGetIpTosAndFrag         */
/*      Get IP TOS and frag offset from Tx buffer                       */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : Tx buffer pointer                                         */
/*      up : pointer for returning user priority                        */
/*      fragOff : pointer for returning ip frag offset                  */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      None                                                            */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        ZyDAS Technology Corporation    2006.6      */
/*                                                                      */
/************************************************************************/
void zfTxGetIpTosAndFrag(zdev_t* dev, zbuf_t* buf, u8_t* up, u16_t* fragOff)
{
    u8_t ipv;
    u16_t len;
	u16_t etherType;
    u8_t tos;

    *up = 0;
    *fragOff = 0;

    len = zfwBufGetSize(dev, buf);

    if (len >= 34) //Minimum IPv4 packet size, 14(Ether header)+20(IPv4 header)
    {
        etherType = (((u16_t)zmw_tx_buf_readb(dev, buf, ZM_80211_FRAME_TYPE_OFFSET))<<8)
                    + zmw_tx_buf_readb(dev, buf, ZM_80211_FRAME_TYPE_OFFSET + 1);

        /* protocol type = IP */
        if (etherType == 0x0800)
        {
            ipv = zmw_tx_buf_readb(dev, buf, ZM_80211_FRAME_IP_OFFSET) >> 4;
            if (ipv == 0x4) //IPv4
            {
                tos = zmw_tx_buf_readb(dev, buf, ZM_80211_FRAME_IP_OFFSET + 1);
                *up = (tos >> 5);
                *fragOff = zmw_tx_buf_readh(dev, buf, ZM_80211_FRAME_IP_OFFSET + 6);
            }
            /* TODO : handle VLAN tag and IPv6 packet */
        }
    }
    return;
}

#ifdef ZM_ENABLE_NATIVE_WIFI
u16_t zfTxGenWlanSnap(zdev_t* dev, zbuf_t* buf, u16_t* snap, u16_t* snaplen)
{
    snap[0] = zmw_buf_readh(dev, buf, ZM_80211_FRAME_HEADER_LEN + 0);
    snap[1] = zmw_buf_readh(dev, buf, ZM_80211_FRAME_HEADER_LEN + 2);
    snap[2] = zmw_buf_readh(dev, buf, ZM_80211_FRAME_HEADER_LEN + 4);
    *snaplen = 6;

    return ZM_80211_FRAME_HEADER_LEN + *snaplen;
}
#else
u16_t zfTxGenWlanSnap(zdev_t* dev, zbuf_t* buf, u16_t* snap, u16_t* snaplen)
{
    u16_t removed;
	   u16_t etherType;
   	u16_t len;

	   len = zfwBufGetSize(dev, buf);
    if (len < 14) //Minimum Ethernet packet size, 14(Ether header)
    {
        /* TODO : Assert? */
        *snaplen = 0;
        return 0;
    }

    /* Generate RFC1042 header */
    etherType = (((u16_t)zmw_tx_buf_readb(dev, buf, 12))<<8)
                + zmw_tx_buf_readb(dev, buf, 13);

    //zm_debug_msg2("ethernet type or length = ", etherType);

    if (etherType > 1500)
    {
        /* ETHERNET format */
        removed = 12;
        snap[0] = 0xaaaa;
        snap[1] = 0x0003;
        if ((etherType ==0x8137) || (etherType == 0x80f3))
        {
            /* Bridge Tunnel */
            snap[2] = 0xF800;
        }
        else
        {
            /* RFC 1042 */
            snap[2] = 0x0000;
        }
        *snaplen = 6;

        if ( etherType == 0x888E )
        {
            zfShowTxEAPOL(dev, buf, 14);
        }
    }
    else
    {
        /* 802.3 format */
        removed = 14;
        *snaplen = 0;
    }

    return removed;
}
#endif

u8_t zfIsVtxqEmpty(zdev_t* dev)
{
    u8_t isEmpty = TRUE;
    u8_t i;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);

    if (wd->vmmqHead != wd->vmmqTail)
    {
        isEmpty = FALSE;
        goto check_done;
    }

    for(i=0; i < 4; i++)
    {
        if (wd->vtxqHead[i] != wd->vtxqTail[i])
        {
            isEmpty = FALSE;
            goto check_done;
        }
    }

check_done:
    zmw_leave_critical_section(dev);
    return isEmpty;
}

/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfPutVtxq                   */
/*      Put Tx buffer to virtual TxQ                                    */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : Tx buffer pointer                                         */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      ZM_SUCCESS or error code                                        */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        ZyDAS Technology Corporation    2006.6      */
/*                                                                      */
/************************************************************************/
u16_t zfPutVtxq(zdev_t* dev, zbuf_t* buf)
{
    u8_t ac;
    u8_t up;
    u16_t fragOff;
#ifdef ZM_AGG_TALLY
    struct aggTally *agg_tal;
#endif
#ifdef ZM_ENABLE_AGGREGATION
    #ifndef ZM_BYPASS_AGGR_SCHEDULING
	u16_t ret;
    u16_t tid;
    #endif
#endif

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zfTxGetIpTosAndFrag(dev, buf, &up, &fragOff);

    if ( wd->zfcbClassifyTxPacket != NULL )
    {
        ac = wd->zfcbClassifyTxPacket(dev, buf);
    }
    else
    {
        ac = zcUpToAc[up&0x7] & 0x3;
    }

    /*
     * add by honda
     * main A-MPDU aggregation function
     */
#ifdef ZM_AGG_TALLY
    agg_tal = &wd->agg_tal;
    agg_tal->got_packets_sum++;

#endif

#ifdef ZM_ENABLE_AGGREGATION
    #ifndef ZM_BYPASS_AGGR_SCHEDULING
    tid = up&0x7;
    if(wd->enableAggregation==0)
    {
        if( (wd->wlanMode == ZM_MODE_AP) ||
            (wd->wlanMode == ZM_MODE_INFRASTRUCTURE && wd->sta.EnableHT) ||
            (wd->wlanMode == ZM_MODE_PSEUDO) ) {
            // (infrastructure_mode && connect_to_11n_ap) || (ap_mode && is_11n_ap)
            //ret = zfAggPutVtxq(dev, buf);


            ret = zfAggTx(dev, buf, tid);
            if (ZM_SUCCESS == ret)
            {
                //zfwBufFree(dev, buf, ZM_SUCCESS);

                return ZM_SUCCESS;
            }
            if (ZM_ERR_EXCEED_PRIORITY_THRESHOLD == ret)
            {
                wd->commTally.txQosDropCount[ac]++;
                zfwBufFree(dev, buf, ZM_SUCCESS);

                zm_msg1_tx(ZM_LV_1, "Packet discarded, VTXQ full, ac=", ac);

                return ZM_ERR_EXCEED_PRIORITY_THRESHOLD;
            }
            if (ZM_ERR_TX_BUFFER_UNAVAILABLE == ret)
            {
                /*
                * do nothing
                * continue following procession, put into VTXQ
                * return ZM_SUCCESS;
                */
            }
        }
    }
    #endif
#endif
    /*
     * end of add by honda
     */

    /* First Ip frag */
    if ((fragOff & 0xff3f) == 0x0020)
    {
        /* Don't let ip frag in if VTXQ unable to hold */
        /* whole ip frag burst(assume 20 frag)         */
        zmw_enter_critical_section(dev);
        if (((wd->vtxqHead[ac] - wd->vtxqTail[ac])& ZM_VTXQ_SIZE_MASK)
                > (ZM_VTXQ_SIZE-20))
        {
            wd->qosDropIpFrag[ac] = 1;
        }
        else
        {
            wd->qosDropIpFrag[ac] = 0;
        }
        zmw_leave_critical_section(dev);

        if (wd->qosDropIpFrag[ac] == 1)
        {
            //zm_debug_msg2("vtQ full, drop buf = ", buf);
            wd->commTally.txQosDropCount[ac]++;
            zfwBufFree(dev, buf, ZM_SUCCESS);
            zm_msg1_tx(ZM_LV_1, "Packet discarded, first ip frag, ac=", ac);
            //VTXQ[] can not hold whold ip frag burst(assume 20 frags)
            return ZM_ERR_EXCEED_PRIORITY_THRESHOLD;
        }
    }
    else if ((fragOff & 0xff3f) == 0)
    {
        wd->qosDropIpFrag[ac] = 0;
    }

    if (((fragOff &= 0xff1f) != 0) && (wd->qosDropIpFrag[ac] == 1))
    {
        wd->commTally.txQosDropCount[ac]++;
        zfwBufFree(dev, buf, ZM_SUCCESS);
        zm_msg1_tx(ZM_LV_1, "Packet discarded, ip frag, ac=", ac);
        //Discard following ip frags
        return ZM_ERR_EXCEED_PRIORITY_THRESHOLD;
    }

    zmw_enter_critical_section(dev);
    if (((wd->vtxqHead[ac] + 1) & ZM_VTXQ_SIZE_MASK) != wd->vtxqTail[ac])
    {
        wd->vtxq[ac][wd->vtxqHead[ac]] = buf;
        wd->vtxqHead[ac] = ((wd->vtxqHead[ac] + 1) & ZM_VTXQ_SIZE_MASK);
        zmw_leave_critical_section(dev);
        return ZM_SUCCESS;
    }
    else
    {
        zmw_leave_critical_section(dev);

        wd->commTally.txQosDropCount[ac]++;
        zfwBufFree(dev, buf, ZM_SUCCESS);
        zm_msg1_tx(ZM_LV_1, "Packet discarded, VTXQ full, ac=", ac);
        return ZM_ERR_EXCEED_PRIORITY_THRESHOLD; //VTXQ[] Full
    }
}


/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfGetVtxq                   */
/*      Get Tx buffer from virtual TxQ                                  */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      Tx buffer pointer                                               */
/*                                                                      */
/*    AUTHOR       isplay.list[0][0], copins *, chipptr, COPLISTSIZE);
	assignchunk(copdisplay.list[0][1], copins *, chipptr, COPLISTSIZE);
	assignchunk(copdisplay.list[1][0], copins *, chipptr, COPLISTSIZE);
	assignchunk(copdisplay.list[1][1], copins *, chipptr, COPLISTSIZE);

	/*
	 * access the videomem with writethrough cache
	 */
	fb_info.fix.smem_start = (u_long)ZTWO_PADDR(videomemory);
	videomemory = (u_long)ioremap_writethrough(fb_info.fix.smem_start,
						   fb_info.fix.smem_len);
	if (!videomemory) {
		printk("amifb: WARNING! unable to map videomem cached writethrough\n");
		fb_info.screen_base = (char *)ZTWO_VADDR(fb_info.fix.smem_start);
	} else
		fb_info.screen_base = (char *)videomemory;

	memset(dummysprite, 0, DUMMYSPRITEMEMSIZE);

	/*
	 * Enable Display DMA
	 */

	custom.dmacon = DMAF_SETCLR | DMAF_MASTER | DMAF_RASTER | DMAF_COPPER |
	                DMAF_BLITTER | DMAF_SPRITE;

	/*
	 * Make sure the Copper has something to do
	 */

	ami_init_copper();

	if (request_irq(IRQ_AMIGA_COPPER, amifb_interrupt, 0,
	                "fb vertb handler", &currentpar)) {
		err = -EBUSY;
		goto amifb_error;
	}

	err = fb_alloc_cmap(&fb_info.cmap, 1<<fb_info.var.bits_per_pixel, 0);
	if (err)
		goto amifb_error;

	if (register_framebuffer(&fb_info) < 0) {
		err = -EINVAL;
		goto amifb_error;
	}

	printk("fb%d: %s frame buffer device, using %dK of video memory\n",
	       fb_info.node, fb_info.fix.id, fb_info.fix.smem_len>>10);

	return 0;

amifb_error:
	amifb_deinit();
	return err;
}

static void amifb_deinit(void)
{
	if (fb_info.cmap.len)
		fb_dealloc_cmap(&fb_info.cmap);
	chipfree();
	if (videomemory)
		iounmap((void*)videomemory);
	release_mem_region(CUSTOM_PHYSADDR+0xe0, 0x120);
	custom.dmacon = DMAF_ALL | DMAF_MASTER;
}


	/*
	 * Blank the display.
	 */

static int amifb_blank(int blank, struct fb_info *info)
{
	do_blank = blank ? blank : -1;

	return 0;
}

	/*
	 * Flash the cursor (called by VBlank interrupt)
	 */

static int flash_cursor(void)
{
	static int cursorcount = 1;

	if (cursormode == FB_CURSOR_FLASH) {
		if (!--cursorcount) {
			cursorstate = -cursorstate;
			cursorcount = cursorrate;
			if (!is_blanked)
				return 1;
		}
	}
	return 0;
}

	/*
	 * VBlank Display Interrupt
	 */

static irqreturn_t amifb_interrupt(int irq, void *dev_id)
{
	if (do_vmode_pan || do_vmode_full)
		ami_update_display();

	if (do_vmode_full)
		ami_init_display();

	if (do_vmode_pan) {
		flash_cursor();
		ami_rebuild_copper();
		do_cursor = do_vmode_pan = 0;
	} else if (do_cursor) {
		flash_cursor();
		ami_set_sprite();
		do_cursor = 0;
	} else {
		if (flash_cursor())
			ami_set_sprite();
	}

	if (do_blank) {
		ami_do_blank();
		do_blank = 0;
	}

	if (do_vmode_full) {
		ami_reinit_copper();
		do_vmode_full = 0;
	}
	return IRQ_HANDLED;
}

/* --------------------------- Hardware routines --------------------------- */

	/*
	 * Get the video params out of `var'. If a value doesn't fit, round
	 * it up, if it's too big, return -EINVAL.
	 */

static int ami_decode_var(struct fb_var_screeninfo *var,
                          struct amifb_par *par)
{
	u_short clk_shift, line_shift;
	u_long maxfetchstop, fstrt, fsize, fconst, xres_n, yres_n;
	u_int htotal, vtotal;

	/*
	 * Find a matching Pixel Clock
	 */

	for (clk_shift = TAG_SHRES; clk_shift <= TAG_LORES; clk_shift++)
		if (var->pixclock <= pixclock[clk_shift])
			break;
	if (clk_shift > TAG_LORES) {
		DPRINTK("pixclock too high\n");
		return -EINVAL;
	}
	par->clk_shift = clk_shift;

	/*
	 * Check the Geometry Values
	 */

	if ((par->xres = var->xres) < 64)
		par->xres = 64;
	if ((par->yres = var->yres) < 64)
		par->yres = 64;
	if ((par->vxres = var->xres_virtual) < par->xres)
		par->vxres = par->xres;
	if ((par->vyres = var->yres_virtual) < par->yres)
		par->vyres = par->yres;

	par->bpp = var->bits_per_pixel;
	if (!var->nonstd) {
		if (par->bpp < 1)
			par->bpp = 1;
		if (par->bpp > maxdepth[clk_shift]) {
			if (round_down_bpp && maxdepth[clk_shift])
				par->bpp = maxdepth[clk_shift];
			else {
				DPRINTK("invalid bpp\n");
				return -EINVAL;
			}
		}
	} else if (var->nonstd == FB_NONSTD_HAM) {
		iuf;
    }
    else
    {
        zmw_leave_critical_section(dev);
        return 0; //VTXQ[] empty
    }
}

/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfPushVtxq                  */
/*      Service Virtual TxQ (weighted round robin)                      */
/*      Get Tx buffer form virtual TxQ and put to hardware TxD queue    */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      None                                                            */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        ZyDAS Technology Corporation    2006.6      */
/*                                                                      */
/************************************************************************/
void zfPushVtxq(zdev_t* dev)
{
    zbuf_t* buf;
    u16_t i;
    u16_t txed;
    u32_t freeTxd;
    u16_t err;
    u16_t skipFlag = 0;
    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();



    //zm_debug_msg1("zfHpGetFreeTxdCount = ", zfHpGetFreeTxdCount(dev));

    if (wd->halState == ZM_HAL_STATE_INIT)
    {
        if (!wd->modeMDKEnable)
        {
            zm_debug_msg0("HAL is not ready for Tx");
        }
        return;
    }
    else if (wd->sta.DFSDisableTx)
    {
        zm_debug_msg0("return because 802.11h DFS Disable Tx");
        return;
    }
    else if (wd->sta.flagFreqChanging != 0)
    {
        //Hold until RF frequency changed
        return;
    }
    else if (( wd->sta.flagKeyChanging ) && ( wd->wlanMode != ZM_MODE_AP ))
    {
        return;
    }
#ifdef ZM_ENABLE_POWER_SAVE
    else if ( zfPowerSavingMgrIsSleeping(dev) )
    {
        //zm_debug_msg0("Packets queued since the MAC is in power-saving mode\n");
        return;
    }
#endif

    zmw_enter_critical_section(dev);
    if (wd->vtxqPushing != 0)
    {
        skipFlag = 1;
    }
    else
    {
        wd->vtxqPushing = 1;
    }
    zmw_leave_critical_section(dev);

    if (skipFlag == 1)
    {
        return;
    }

    while (1)
    {
        txed = 0;

        /* 2006.12.20, Serve Management queue */
        while( zfHpGetFreeTxdCount(dev) > 0 )
        {
            if ((buf = zfGetVmmq(dev)) != 0)
            {
                txed = 1;
                //zm_debug_msg2("send buf = ", buf);
                if ((err = zfHpSend(dev, NULL, 0, NULL, 0, NULL, 0, buf, 0,
                        ZM_INTERNAL_ALLOC_BUF, 0, 0xff)) != ZM_SUCCESS)
                {
                    zfwBufFree(dev, buf, 0);
                }
            }
            else
            {
                break;
            }
        }
        if ((wd->sta.bScheduleScan) || ((wd->sta.bChannelScan == TRUE) && (zfStaIsConnected(dev))))
        {
            //Hold until Scan Stop
            wd->vtxqPushing = 0;
            return;
        }

#ifdef ZM_ENABLE_AGGREGATION
    #ifndef ZM_BYPASS_AGGR_SCHEDULING
        if( (wd->wlanMode == ZM_MODE_AP) ||
            (wd->wlanMode == ZM_MODE_INFRASTRUCTURE && wd->sta.EnableHT) ||
            (wd->wlanMode == ZM_MODE_PSEUDO) ) {

            zfAggTxScheduler(dev, 0);

            if (txed == 0) {
                wd->vtxqPushing = 0;
                return;
            }
            else {
                continue;
            }
        }
    #endif
#endif

        /* Service VTxQ[3] */
        for (i=0; i<4; i++)
        {
            if ((freeTxd = zfHpGetFreeTxdCount(dev)) >= 3)
            {
                if ((buf = zfGetVtxq(dev, 3)) != 0)
                {
                    txed = 1;
                    //zm_debug>right_margin<<clk_shift;
		par->hsstop = (var->right_margin+var->hsync_len)<<clk_shift;
		par->diwstop_h = par->htotal - mod8(par->hsstrt) + 8 - (1 << clk_shift);
		if (!IS_AGA)
			par->diwstop_h = down4(par->diwstop_h) - 16;
		par->diwstrt_h = par->diwstop_h - xres_n;
		par->hbstop = par->diwstrt_h + 4;
		par->hbstrt = par->diwstop_h + 4;
		if (par->hbstrt >= par->htotal + 8)
			par->hbstrt -= par->htotal;
		par->hcenter = par->hsstrt + (par->htotal >> 1);
		par->vsstrt = var->lower_margin<<line_shift;
		par->vsstop = (var->lower_margin+var->vsync_len)<<line_shift;
		par->diwstop_v = par->vtotal;
		if ((par->vmode & FB_VMODE_MASK) == FB_VMODE_INTERLACED)
			par->diwstop_v -= 2;
		par->diwstrt_v = par->diwstop_v - yres_n;
		par->vbstop = par->diwstrt_v - 2;
		par->vbstrt = par->diwstop_v - 2;
		if (par->vtotal > 2048) {
			DPRINTK("vtotal too high\n");
			return -EINVAL;
		}
		if (par->htotal > 2048) {
			DPRINTK("htotal too high\n");
			return -EINVAL;
		}
		par->bplcon3 |= BPC3_EXTBLKEN;
		par->beamcon0 = BMC0_HARDDIS | BMC0_VARVBEN | BMC0_LOLDIS |
		                BMC0_VARVSYEN | BMC0_VARHSYEN | BMC0_VARBEAMEN |
		                BMC0_PAL | BMC0_VARCSYEN;
		if (var->sync & FB_SYNC_HOR_HIGH_ACT)
			par->beamcon0 |= BMC0_HSYTRUE;
		if (var->sync & FB_SYNC_VERT_HIGH_ACT)
			par->beamcon0 |= BMC0_VSYTRUE;
		if (var->sync & FB_SYNC_COMP_HIGH_ACT)
			par->beamcon0 |= BMC0_CSYTRUE;
		htotal = par->htotal>>clk_shift;
		vtotal = par->vtotal>>1;
	} else {
		DPRINTK("only broadcast modes possible for ocs\n");
		return -EINVAL;
	}

	/*
	 * Checking the DMA timing
	 */

	fconst = 16<<maxfmode<<clk_shift;

	/*
	 * smallest window start value without turn off other dma cycles
	 * than sprite1-7, unless you change min_fstrt
	 */


	fsize = ((maxfmode+clk_shift <= 1) ? fconst : 64);
	fstrt = downx(fconst, par->diwstrt_h-4) - fsize;
	if (fstrt < min_fstrt) {
		DPRINTK("fetch start too low\n");
		return -EINVAL;
	}

	/*
	 * smallest window start value where smooth scrolling is possible
	 */

	fstrt = downx(fconst, par->diwstrt_h-fconst+(1<<clk_shift)-4) - fsize;
	if (fstrt < min_fstrt)
		par->vmode &= ~FB_VMODE_SMOOTH_XPAN;

	maxfetchstop = down16(par->htotal - 80);

	fstrt = downx(fconst, par->diwstrt_h-4) - 64 - fconst;
	fsize = upx(fconst, xres_n + modx(fconst, downx(1<<clk_shift, par->diwstrt_h-4)));
	if (fstrt + fsize > maxfetchstop)
		par->vmode &= ~FB_VMODE_SMOOTH_XPAN;

	fsize = upx(fconst, xres_n);
	if (fstrt + fsize > maxfetchstop) {
		DPRINTK("fetch stop too high\n");
		return -EINVAL;
	}

	if (maxfmode + clk_shift <= 1) {
		fsize = up64(xres_n + fconst - 1);
		if (min_fstrt + fsize - 64 > maxfetchstop)
			par->vmode &= ~FB_VMODE_SMOOTH_XPAN;

		fsize = up64(xres_n);
		if (min_fstrt + fsize - 64 > maxfetchstop) {
			DPRINTK("fetch size too high\n");
			return -EINVAL;
		}

		fsize -= 64;
	} else
		fsize -= fconst;

	/*
	 * Check if there is enough time to update the bitplane pointers for ywrap
	 */

	if (par->htotal-fsize-64 < par->bpp*64)
		par->vmode &= ~FB_VMODE_YWRAP;

	/*
	 * Bitplane calculations and check the Memory Requirements
	 */

	if (amifb_ilbm) {
		par->next_plane = div8(upx(16<<maxfmode, par->vxres));
		par->next_line = par->bpp*par->next_plane;
		if (par->next_line * par->vyres > fb_info.fix.smem_len) {
			DPRINTK("too few video mem\n");
			return -EINVAL;
		}
	} else {
		par->next_line = div8(upx(16<<maxfmode, par->vxres));
		par->next_plane = par->vyres*par->next_line;
		if (par->next_plane * par->bpp > fb_info.fix.smem_len) {
			DPRINTK("too few video mem\n");
			return -EINVAL;
		}
	}

	/*
	 * Hardware Register Values
	 */

	par->bplcon0 = BPC0_COLOR | bplpixmode[clk_shift];
	if (!IS_OCS)
		par->bplcon0 |= BPC0_ECSENA;
	if (par->bpp == 8)
		par->bplcon0 |= BPC0_BPU3;
	else
		par->bplcon0 |= par->bpp<<12;
	if (var->nonstd == FB_NONSTD_HAM)
		par->bplcon0 |= BPC0_HAM;
	if (var->sync & FB_SYNC_EXT)
		par->bplcon0 |= BPC0_ERSY;

	if (IS_AGA)
		par->fmode = bplfetchmode[maxfmode];

	switch (par->vmode & FB_VMODE_MASK) {
		case FB_VMODE_INTERLACED:
			par->bplcon0 |= BPC0_LACE;
			break;
		case FB_VMODE_DOUBLEoffset, u16_t bufType,
                           u8_t ac, u8_t keyIdx)
{
    u16_t err;
    u16_t fragLen;

    zmw_get_wlan_dev(dev);

    fragLen = zfwBufGetSize(dev, buf);
    if ((da[0]&0x1) == 0)
    {
        wd->commTally.txUnicastFrm++;
        wd->commTally.txUnicastOctets += (fragLen+snapLen);
    }
    else if (da[0] == 0xffff)
    {
        wd->commTally.txBroadcastFrm++;
        wd->commTally.txBroadcastOctets += (fragLen+snapLen);
    }
    else
    {
        wd->commTally.txMulticastFrm++;
        wd->commTally.txMulticastOctets += (fragLen+snapLen);
    }
    wd->ledStruct.txTraffic++;

    if ((err = zfHpSend(dev, header, headerLen, snap, snapLen,
                        tail, tailLen, buf, offset,
                        bufType, ac, keyIdx)) != ZM_SUCCESS)
    {
        if (bufType == ZM_EXTERNAL_ALLOC_BUF)
        {
            zfwBufFree(dev, buf, err);
        }
        else if (bufType == ZM_INTERNAL_ALLOC_BUF)
        {
            zfwBufFree(dev, buf, 0);
        }
        else
        {
            zm_assert(0);
        }
    }
}

void zfCheckIsRIFSFrame(zdev_t* dev, zbuf_t* buf, u16_t frameSubtype)
{
    zmw_get_wlan_dev(dev);

    /* #2 Record the sequence number to determine whether the unicast frame is separated by RIFS or not */
    if (frameSubtype & 0x80)
    {   //QoS data frame
        u16_t sequenceNum;
        u16_t qosControlField;

        sequenceNum = ( zmw_buf_readh(dev, buf, 22) >> 4 ); // Discard fragment number !
        qosControlField = zmw_buf_readh(dev, buf, 24); // Don't consider WDS (Wireless Distribution System)
        //DbgPrint("The QoS Control Field                              : %d", qosControlField);
        //DbgPrint("The RIFS Count                                     : %d", wd->sta.rifsCount);

        if( qosControlField & ZM_BIT_5 )
        {// ACK policy is "No ACK"
            /* RIFS-Like frame */
            wd->sta.rifsLikeFrameSequence[wd->sta.rifsLikeFrameCnt]   = sequenceNum;

            if( wd->sta.rifsState == ZM_RIFS_STATE_DETECTING )
            {
                if( wd->sta.rifsLikeFrameSequence[2] != 0 )
                {// RIFS-like Pattern collected
                    if( ( wd->sta.rifsLikeFrameSequence[2] - wd->sta.rifsLikeFrameSequence[1] == 2 ) &&
                        ( wd->sta.rifsLikeFrameSequence[1] - wd->sta.rifsLikeFrameSequence[0] == 2 ) )
                    {
                        /* RIFS pattern matched */

                        /* #3 Enable RIFS function if the RIFS pattern matched  */
                        zfHpEnableRifs(dev, ((wd->sta.currentFrequency<3000)?1:0), wd->sta.EnableHT, wd->sta.HT2040);

                        // Set RIFS timer
                        wd->sta.rifsTimer = wd->tick;

                        wd->sta.rifsCount++;

                        // Set state to be Detected
                        wd->sta.rifsState = ZM_RIFS_STATE_DETECTED;
                    }
                }
            }
            else
            {// state = Detected
                // Reset RIFS timer
                if( (wd->tick - wd->sta.rifsTimer) < ZM_RIFS_TIMER_TIMEOUT )
                    wd->sta.rifsTimer = wd->tick;
            }

            //DbgPrint("SN1 = %d, SN2 = %d, SN3 = %d\n", wd->sta.rifsLikeFrameSequence[0],
            //                                           wd->sta.rifsLikeFrameSequence[1],
            //                                           wd->sta.rifsLikeFrameSequence[2]);

            // Update RIFS-like sequence number
            if( wd->sta.rifsLikeFrameSequence[2] != 0 )
            {
                wd->sta.rifsLikeFrameSequence[0] = wd->sta.rifsLikeFrameSequence[1];
                wd->sta.rifsLikeFrameSequence[1] = wd->sta.rifsLikeFrameSequence[2];
                wd->sta.rifsLikeFrameSequence[2] = 0;
            }

            // Only record three adjacent frame
            if( wd->sta.rifsLikeFrameCnt < 2 )
                wd->sta.rifsLikeFrameCnt++;
        }
    }

    /* #4 Disable RIFS function if the timer TIMEOUT  */
    if( wd->sta.rifsState == ZM_RIFS_STATE_DETECTED )
  <<maxfmode<<clk_shift;
	vshift = modx(16<<maxfmode, par->xoffset);
	fstrt = par->diwstrt_h - (vshift<<clk_shift) - 4;
	fsize = (par->xres+vshift)<<clk_shift;
	shift = modx(fconst, fstrt);
	move = downx(2<<maxfmode, div8(par->xoffset));
	if (maxfmode + clk_shift > 1) {
		fstrt = downx(fconst, fstrt) - 64;
		fsize = upx(fconst, fsize);
		fstop = fstrt + fsize - fconst;
	} else {
		mod = fstrt = downx(fconst, fstrt) - fconst;
		fstop = fstrt + upx(fconst, fsize) - 64;
		fsize = up64(fsize);
		fstrt = fstop - fsize + 
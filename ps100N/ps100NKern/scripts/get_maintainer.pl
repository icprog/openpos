#
# USB Core configuration
#
config USB_DEBUG
	bool "USB verbose debug messages"
	depends on USB
	help
	  Say Y here if you want the USB core & hub drivers to produce a bunch
	  of debug messages to the system log. Select this if you are having a
	  problem with USB support and want to see more of what is going on.

config USB_ANNOUNCE_NEW_DEVICES
	bool "USB announce new devices"
	depends on USB
	default N
	help
	  Say Y here if you want the USB core to always announce the
	  idVendor, idProduct, Manufacturer, Product, and SerialNumber
	  strings for every new USB device to the syslog.  This option is
	  usually used by distro vendors to help with debugging and to
	  let users know what specific device was added to the machine
	  in what location.

	  If you do not want this kind of information sent to the system
	  log, or have any doubts about this, say N here.

comment "Miscellaneous USB options"
	depends on USB

config USB_DEVICEFS
	bool "USB device filesystem (DEPRECATED)"
	depends on USB
	---help---
	  If you say Y here (and to "/proc file system support" in the "File
	  systems" section, above), you will get a file /proc/bus/usb/devices
	  which lists the devices currently connected to your USB bus or
	  busses, and for every connected device a file named
	  "/proc/bus/usb/xxx/yyy", where xxx is the bus number and yyy the
	  device number; the latter files can be used by user space programs
	  to talk directly to the device. These files are "virtual", meaning
	  they are generated on the fly and not stored on the hard drive.

	  You may need to mount the usbfs file system to see the files, use
	  mount -t usbfs none /proc/bus/usb

	  For the format of the various /proc/bus/usb/ files, please read
	  <file:Documentation/usb/proc_usb_info.txt>.

	  Modern Linux systems do not use this.

	  Usbfs entries are files and not character devices; usbfs can't
	  handle Access Control Lists (ACL) which are the default way to
	  grant access to USB devices for untrusted users of a desktop
	  system.

	  The usbfs functionality is replaced by real device-nodes managed by
	  udev.  These nodes lived in /dev/bus/usb and are used by libusb.

config USB_DEVICE_CLASS
	bool "USB device class-devices (DEPRECATED)"
	depends on USB
	default y
	---help---
	  Userspace access to USB devices is granted by device-nodes exported
	  directly from the usbdev in sysfs. Old versions of the driver
	  core and udev needed additional class devices to export device nodes.

	  These additional devices are difficult to handle in userspace, if
	  information about USB interfaces must be available. One device
	  contains the device node, the other device contains the interface
	  data. Both devices are at the same level in sysfs (siblings) and one
	  can't access the other. The device node created directly by the
	  usb device is the parent device of the interface and therefore
	  easily accessible from the interface event.

	  This option provides backward compatibility for libusb device
	  nodes (lsusb) when usbfs is not used, and the following udev rule
	  doesn't exist:
	    SUBSYSTEM=="usb", ACTION=="add", ENV{DEVTYPE}=="usb_device", \
	    NAME="bus/usb/$env{BUSNUM}/$env{DEVNUM}", MODE="0644"

config USB_DYNAMIC_MINORS
	bool "Dynamic USB minor allocation"
	depends on USB
	help
	  If you say Y here, the USB subsystem will use dynamic minor
	  allocation for any device that uses the USB major number.
	  This means that you can have more than 16 of a single type
	  of device (like USB printers).

	  If you are unsure about this, say N here.

config USB_SUSPEND
	bool "USB selective suspend/resume and wakeup"
	depends on USB && PM
	help
	  If you say Y here, you can use driver calls or the sysfs
	  "power/level" file to suspend or resume individual USB
	  peripherals and to enable or disable autosuspend (see
	  Documentation/usb/power-management.txt for more details).

	  Also, USB "remote wakeup" signaling is supported, whereby some
	  USB devices (like keyboards and network adapters) can wake up
	  their parent hub.  That wakeup cascades up the USB tree, and
	  could wake the system from states like suspend-to-RAM.

	  If you are unsure about this, say N here.

config USB_OTG
	bool
	depends on USB && EXPERIMENTAL
	select USB_SUSPEND
	default n


config USB_OTG_WHITELIST
	bool "Rely on OTG Targeted Peripherals List"
	depends on USB_OTG || EMBEDDED
	default y if USB_OTG
	default n if EMBEDDED
	help
	  If you say Y here, the "otg_whitelist.h" file will be used as a
	  product whitelist, so USB peripherals not listed there will be
	  rejected during enumeration.  This behavior is required by the
	  USB OTG specification for all devices not on your product's
	  "Targeted Peripherals List".  "Embedded Hosts" are likewise
	  allowed to support only a limited number of peripherals.

	  Otherwise, peripherals not listed there will only generate a
	  warning and enumeration will continue.  That's more like what
	  normal Linux-USB hosts do (other than the warning), and is
	  convenient for many stages of product development.

config USB_OTG_BLACKLIST_HUB
	bool "Disable external hubs"
	depends on USB_OTG || EMBEDDED
	help
	  If you say Y here, then Linux will refuse to enumerate
	  external hubs.  OTG hosts are allowed to reduce hardware
	  and software costs by not supporting external hubs.  So
	  are "Embedded Hosts" that don't offer OTG support.

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            /*
 * Copyright 2004-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
#ifndef __PMIC_H__
#define __PMIC_H__

 /*!
  * @file pmic.h
  * @brief This file contains prototypes of all the functions to be
  * defined for each PMIC chip. The implementation of these may differ
  * from PMIC chip to PMIC chip.
  *
  * @ingroup PMIC_CORE
  */

#include <linux/spi/spi.h>

#define MAX_ACTIVE_EVENTS		10

/*!
 * This structure is a way for the PMIC core driver to define their own
 * \b spi_device structure. This structure includes the core \b spi_device
 * structure that is provided by Linux SPI Framework/driver as an
 * element and may contain other elements that are required by core driver.
 */
struct mxc_pmic {
	/*!
	 * Master side proxy for an SPI slave device(PMIC)
	 */
	struct spi_device *spi;
};

/*!
 * This function is called to transfer data to PMIC on SPI.
 *
 * @param    spi	the SPI slave device(PMIC)
 * @param    buf   	the pointer to the data buffer
 * @param    len    	the length of the data to be transferred
 *
 * @return   Returns 0 on success -1 on failure.
 */
static inline int spi_rw(struct spi_device *spi, u8 * buf, size_t len)
{
	struct spi_transfer t = {
		.tx_buf = (const void *)buf,
		.rx_buf = buf,
		.len = len,
		.cs_change = 0,
		.delay_usecs = 0,
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	if (spi_sync(spi, &m) != 0 || m.status != 0)
		return PMIC_ERROR;
	return (len - m.actual_length);
}

/*!
 * This function returns the PMIC version in system.
 *
 * @param 	ver	pointer to the pmic_version_t structure
 *
 * @return       This function returns PMIC version.
 */
void pmic_get_revision(pmic_version_t * ver);

/*!
 * This function initializes the SPI device parameters for this PMIC.
 *
 * @param    spi	the SPI slave device(PMIC)
 *
 * @return   None
 */
int pmic_spi_setup(struct spi_device *spi);

/*!
 * This function initializes the PMIC registers.
 *
 * @return   None
 */
int pmic_init_registers(void);

/*!
 * This function reads the interrupt status registers of PMIC
 * and determine the current active events.
 *
 * @param 	active_events array pointer to be used to return active
 *		event numbers.
 *
 * @return       This function returns PMIC version.
 */
unsigned int pmic_get_active_events(unsigned int *active_events);

/*!
 * This function sets a bit in mask register of pmic to disable an event IT.
 *
 * @param	event 	the event to be masked
 *
 * @return     This function returns PMIC_SUCCESS on SUCCESS, error on FAILURE.
 */
int pmic_event_mask(type_event event);

/*!
 * This function unsets a bit in mask register of pmic to unmask an event IT.
 *
 * @param	event 	the event to be unmasked
 *
 * @return    This function returns PMIC_SUCCESS on SUCCESS, error on FAILURE.
 */
int pmic_event_unmask(type_event event);

#ifdef CONFIG_MXC_PMIC_FIXARB
extern PMIC_STATUS pmic_fix_arbitration(struct spi_device *spi);
#else
static inline PMIC_STATUS pmic_fix_arbitration(struct spi_device *spi)
{
	return PMIC_SUCCESS;
}
#endif

void *pmic_alloc_data(struct device *dev);

#endif				/* __PMIC_H__ */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     /*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * File: desc.h
 *
 * Purpose:The header file of descriptor
 *
 * Revision History:
 *
 * Author: Tevin Chen
 *
 * Date: May 21, 1996
 *
 */


#ifndef __DESC_H__
#define __DESC_H__

#include <linux/types.h>
#include <linux/mm.h>

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__TETHER_H__)
#include "tether.h"
#endif
// #ifdef PRIVATE_OBJ
//#if !defined(__DEVICE_MODULE_H)
//#include "device_module.h"
//#endif




/*---------------------  Export Definitions -------------------------*/

#define B_OWNED_BY_CHIP     1           //
#define B_OWNED_BY_HOST     0           //

//
// Bits in the RSR register
//
#define RSR_ADDRBROAD       0x80        // 1000 0000
#define RSR_ADDRMULTI       0x40        // 0100 0000
#define RSR_ADDRUNI         0x00        // 0000 0000
#define RSR_IVLDTYP         0x20        // 0010 0000 , invalid packet type
#define RSR_IVLDLEN         0x10        // 0001 0000 , invalid len (> 2312 byte)
#define RSR_BSSIDOK         0x08        // 0000 1000
#define RSR_CRCOK           0x04        // 0000 0100
#define RSR_BCNSSIDOK       0x02        // 0000 0010
#define RSR_ADDROK          0x01        // 0000 0001

//
// Bits in the new RSR register
//
#define NEWRSR_DECRYPTOK    0x10        // 0001 0000
#define NEWRSR_CFPIND       0x08        // 0000 1000
#define NEWRSR_HWUTSF       0x04        // 0000 0100
#define NEWRSR_BCNHITAID    0x02        // 0000 0010
#define NEWRSR_BCNHITAID0   0x01        // 0000 0001

//
// Bits in the TSR0 register
//
#define TSR0_PWRSTS1_2      0xC0        // 1100 0000
#define TSR0_PWRSTS7        0x20        // 0010 0000
#define TSR0_NCR            0x1F        // 0001 1111

//
// Bits in the TSR1 register
//
#define TSR1_TERR           0x80        // 1000 0000
#define TSR1_PWRSTS4_6      0x70        // 0111 0000
#define TSR1_RETRYTMO       0x08        // 0000 1000
#define TSR1_TMO            0x04        // 0000 0100
#define TSR1_PWRSTS3        0x02        // 0000 0010
#define ACK_DATA            0x01        // 0000 0000

//
// Bits in the TCR register
//
#define EDMSDU              0x04        // 0000 0100 end of sdu
#define TCR_EDP             0x02        // 0000 0010 end of packet
#define TCR_STP             0x01        // 0000 0001 start of packet

// max transmit or receive buffer size
#define CB_MAX_BUF_SIZE     2900U       // max buffer size
                                        // NOTE: must be multiple of 4
#define CB_MAX_TX_BUF_SIZE          CB_MAX_BUF_SIZE // max Tx buffer size
#define CB_MAX_RX_BUF_SIZE_NORMAL   CB_MAX_BUF_SIZE // max Rx buffer size when not use Multi-RD

#define CB_BEACON_BUF_SIZE  512U        // default beacon buffer size

#define CB_MAX_RX_DESC      128         // max # of descriptor
#define CB_MIN_RX_DESC      16          // min # of rx descriptor
#define CB_MAX_TX_DESC      64          // max # of descriptor
#define CB_MIN_TX_DESC      16          // min # of tx descriptor

#define CB_MAX_RECEIVED_PACKETS     16  // max # of received packets at one time
                                        // limit our receive routine to indicating
                                        // this many at a time for 2 reasons:
                                        // 1. driver flow control to protocol layer
                                        // 2. limit the time used in ISR routine

#define CB_EXTRA_RD_NUM     32          // default # of Extra RD
#define CB_RD_NUM           32          // default # of RD
#define CB_TD_NUM           32          // default # of TD


// max number of physical segments
// in a single NDIS packet. Above this threshold, the packet
// is copied into a single physically contiguous buffer
#define CB_MAX_SEGMENT      4

#defi
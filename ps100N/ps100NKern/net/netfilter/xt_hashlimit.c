SmsMsgHdr_ST) + (2 * sizeof(u32));

	buffer = kmalloc(totalLen + SMS_DMA_ALIGNMENT,
			GFP_KERNEL | GFP_DMA);
	if (!buffer)
		return -ENOMEM;

	pMsg = (struct SetGpioMsg *) SMS_ALIGN_ADDRESS(buffer);

	pMsg->xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	pMsg->xMsgHeader.msgDstId = HIF_TASK;
	pMsg->xMsgHeader.msgFlags = 0;
	pMsg->xMsgHeader.msgType = MSG_SMS_GPIO_GET_LEVEL_REQ;
	pMsg->xMsgHeader.msgLength = (u16) totalLen;
	pMsg->msgData[0] = PinNum;
	pMsg->msgData[1] = 0;

	/* Send message to SMS */
	smsendian_handle_tx_message((struct SmsMsgHdr_ST *)pMsg);
	rc = smscore_sendrequest_and_wait(coredev, pMsg, totalLen,
			&coredev->gpio_get_level_done);

	if (rc != 0) {
		if (rc == -ETIME)
			sms_err("smscore_gpio_get_level timeout");
		else
			sms_err("smscore_gpio_get_level error");
	}
	kfree(buffer);

	/* Its a race between other gpio_get_level() and the copy of the single
	 * global 'coredev->gpio_get_res' to  the function's variable 'level'
	 */
	*level = coredev->gpio_get_res;

	return rc;
}

static int __init smscore_module_init(void)
{
	int rc = 0;

	INIT_LIST_HEAD(&g_smscore_notifyees);
	INIT_LIST_HEAD(&g_smscore_devices);
	kmutex_init(&g_smscore_deviceslock);

	INIT_LIST_HEAD(&g_smscore_registry);
	kmutex_init(&g_smscore_registrylock);

	return rc;
}

static void __exit smscore_module_exit(void)
{
	kmutex_lock(&g_smscore_deviceslock);
	while (!list_empty(&g_smscore_notifyees)) {
		struct smscore_device_notifyee_t *notifyee =
			(struct smscore_device_notifyee_t *)
				g_smscore_notifyees.next;

		list_del(&notifyee->entry);
		kfree(notifyee);
	}
	kmutex_unlock(&g_smscore_deviceslock);

	kmutex_lock(&g_smscore_registrylock);
	while (!list_empty(&g_smscore_registry)) {
		struct smscore_registry_entry_t *entry =
			(struct smscore_registry_entry_t *)
				g_smscore_registry.next;

		list_del(&entry->entry);
		kfree(entry);
	}
	kmutex_unlock(&g_smscore_registrylock);

	sms_debug("");
}

module_init(smscore_module_init);
module_exit(smscore_module_exit);

MODULE_DESCRIPTION("Siano MDTV Core module");
MODULE_AUTHOR("Siano Mobile Silicon, Inc. (uris@siano-ms.com)");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 /*
 *  derived from "twidjoy.c"
 *
 *  Copyright (c) 2008 Martin Kebert
 *  Copyright (c) 2001 Arndt Schoenewald
 *  Copyright (c) 2000-2001 Vojtech Pavlik
 *  Copyright (c) 2000 Mark Fletcher
 *
 */

/*
 * Driver to use 4CH RC transmitter using Zhen Hua 5-byte protocol (Walkera Lama,
 * EasyCopter etc.) as a joystick under Linux.
 *
 * RC transmitters using Zhen Hua 5-byte protocol are cheap four channels
 * transmitters for control a RC planes or RC helicopters with possibility to
 * connect on a serial port.
 * Data coming from transmitter is in this order:
 * 1. byte = synchronisation byte
 * 2. byte = X axis
 * 3. byte = Y axis
 * 4. byte = RZ axis
 * 5. byte = Z axis
 * (and this is repeated)
 *
 * For questions or feedback regarding this driver module please contact:
 * Martin Kebert <gkmarty@gmail.com> - but I am not a C-programmer nor kernel
 * coder :-(
 */

/*
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include <linux/init.h>

#define DRIVER_DESC	"RC transmitter with 5-byte Zhen Hua protocol joystick driver"

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

/*
 * Constants.
 */

#define ZHENHUA_MAX_LENGTH 5

/*
 * Zhen Hua data.
 */

struct zhenhua {
	struct input_dev *dev;
	int idx;
	unsigned char data[ZHENHUA_MAX_LENGTH];
	char phys[32];
};


/* bits in all incoming bytes needs to be "reversed" */
static int zhenhua_bitreverse(int x)
{
	x = ((x & 0xaa) >> 1) | ((x & 0x55) << 1);
	x = ((x & 0xcc) >> 2) | ((x & 0x33) << 2);
	x = ((x & 0xf0) >> 4) | ((x & 0x0f) << 4);
	return x;
}

/*
 * zhenhua_process_packet() decodes packets the driver receives from the
 * RC transmitter. It updates the data accordingly.
 */

static void zhenhua_process_packet(struct zhenhua *zhenhua)
{
	struct input_dev *dev = zhenhua->dev;
	unsigned char *data = zhenhua->data;

	input_report_abs(dev, ABS_Y, data[1]);
	input_report_abs(dev, ABS_X, data[2]);
	input_report_abs(dev, ABS_RZ, data[3]);
	input_report_abs(dev, ABS_Z, data[4]);

	input_sync(dev);
}

/*
 * zhenhua_interrupt() is called by the low level driver when characters
 * are ready for us. We then buffer them for further processing, or call the
 * packet processing routine.
 */

static irqreturn_t zhenhua_interrupt(struct serio *serio, unsigned char data, unsigned int flags)
{
	struct zhenhua *zhenhua = serio_get_drvdata(serio);

	/* All Zhen Hua packets are 5 bytes. The fact that the first byte
	 * is allways 0xf7 and all others are in range 0x32 - 0xc8 (50-200)
	 * can be used to check and regain sync. */

	if (data == 0xef)
		zhenhua->idx = 0;	/* this byte starts a new packet */
	else if (zhenhua->idx == 0)
		return IRQ_HANDLED;	/* wrong MSB -- ignore this byte */

	if (zhenhua->idx < ZHENHUA_MAX_LENGTH)
		zhenhua->data[zhenhua->idx++] = zhenhua_bitreverse(data);

	if (zhenhua->idx == ZHENHUA_MAX_LENGTH) {
		zhenhua_process_packet(zhenhua);
		zhenhua->idx = 0;
	}

	return IRQ_HANDLED;
}

/*
 * zhenhua_disconnect() is the opposite of zhenhua_connect()
 */

static void zhenhua_disconnect(struct serio *serio)
{
	struct zhenhua *zhenhua = serio_get_drvdata(serio);

	serio_close(serio);
	serio_set_drvdata(serio, NULL);
	input_unregister_device(zhenhua->dev);
	kfree(zhenhua);
}

/*
 * zhenhua_connect() is the routine that is called when someone adds a
 * new serio device. It looks for the Twiddler, and if found, registers
 * it as an input device.
 */

static int zhenhua_connect(struct serio *serio, struct serio_driver *drv)
{
	struct zhenhua *zhenhua;
	struct input_dev *input_dev;
	int err = -ENOMEM;

	zhenhua = kzalloc(sizeof(struct zhenhua), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!zhenhua || !input_dev)
		goto fail1;

	zhenhua->dev = input_dev;
	snprintf(zhenhua->phys, sizeof(zhenhua->phys), "%s/input0", serio->phys);

	input_dev->name = "Zhen Hua 5-byte device";
	input_dev->phys = zhenhua->phys;
	input_dev->id.bustype = BUS_RS232;
	input_dev->id.vendor = SERIO_ZHENHUA;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0100;
	input_dev->dev.parent = &serio->dev;

	input_dev->evbit[0] = BIT(EV_ABS);
	input_set_abs_params(input_dev, ABS_X, 50, 200, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 50, 200, 0, 0);
	input_set_abs_params(input_dev, ABS_Z, 50, 200, 0, 0);
	input_set_abs_params(input_dev, ABS_RZ, 50, 200, 0, 0);

	serio_set_drvdata(serio, zhenhua);

	err = serio_open(serio, drv);
	if (err)
		goto fail2;

	err = input_register_device(zhenhua->dev);
	if (err)
		goto fail3;

	return 0;

 fail3:	serio_close(serio);
 fail2:	serio_set_drvdata(serio, NULL);
 fail1:	input_free_device(input_dev);
	kfree(zhenhua);
	return err;
}

/*
 * The serio driver structure.
 */

static struct serio_device_id zhenhua_serio_ids[] = {
	{
		.type	= SERIO_RS232,
		.proto	= SERIO_ZHENHUA,
		.id	= SERIO_ANY,
		.extra	= SERIO_ANY,
	},
	{ 0 }
};

MODULE_DEVICE_TABLE(serio, zhenhua_serio_ids);

static struct serio_driver zhenhua_drv = {
	.driver		= {
		.name	= "zhenhua",
	},
	.description	= DRIVER_DESC,
	.id_table	= zhenhua_serio_ids,
	.interrupt	= zhenhua_interrupt,
	.connect	= zhenhua_connect,
	.disconnect	= zhenhua_disconnect,
};

/*
 * The functions for inserting/removing us as a module.
 */

static int __init zhenhua_init(void)
{
	return serio_register_driver(&zhenhua_drv);
}

static void __exit zhenhua_exit(void)
{
	serio_unregister_driver(&zhenhua_drv);
}

module_init(zhenhua_init);
module_exit(zhenhua_exit);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     INDX( 	                 (   �   �       �                    B     X H     B     &�}<����j@����j@��dG�j@��                        t m p               B     X H     B     &�}<��=�Wh��� �}<���H!�>��                        t m p               X H     B     &�}<��=�Wh��� �}<���H!�>��                        t m p               B     X H     B     &�}<��=�Wh��� �}<���H!�>��                        t m p               X H     B     &�}<��=�Wh��� �}<� �H!�>��                        t m p               B     X H     B     &�}<��=�Wh��� �}<���H!�>��                        t m p                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        /* ------------------------------------------------------------
 * iSeries_vscsi.c
 * (C) Copyright IBM Corporation 1994, 2003
 * Authors: Colin DeVilbiss (devilbis@us.ibm.com)
 *          Santiago Leon (santil@us.ibm.com)
 *          Dave Boutcher (sleddog@us.ibm.com)
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 *
 * ------------------------------------------------------------
 * iSeries-specific functions of the SCSI host adapter for Virtual I/O devices
 *
 * This driver allows the Linux SCSI peripheral drivers to directly
 * access devices in the hosting partition, either on an iSeries
 * hypervisor system or a converged hypervisor system.
 */

#include <asm/iseries/vio.h>
#include <asm/iseries/hv_lp_event.h>
#include <asm/iseries/hv_types.h>
#include <asm/iseries/hv_lp_config.h>
#include <asm/vio.h>
#include <linux/device.h>
#include "ibmvscsi.h"

/* global variables */
static struct ibmvscsi_host_data *single_host_data;

/* ------------------------------------------------------------
 * Routines for direct interpartition interaction
 */
struct srp_lp_event {
	struct HvLpEvent lpevt;	/* 0x00-0x17          */
	u32 reserved1;		/* 0x18-0x1B; unused  */
	u16 version;		/* 0x1C-0x1D; unused  */
	u16 subtype_rc;		/* 0x1E-0x1F; unused  */
	struct viosrp_crq crq;	/* 0x20-0x3F          */
};

/** 
 * standard interface for handling logical partition events.
 */
static void iseriesvscsi_handle_event(struct HvLpEvent *lpevt)
{
	struct srp_lp_event *evt = (struct srp_lp_event *)lpevt;

	if (!evt) {
		printk(KERN_ERR "ibmvscsi: received null event\n");
		return;
	}

	if (single_host_data == NULL) {
		printk(KERN_ERR
		       "ibmvscsi: received event, no adapter present\n");
		return;
	}

	ibmvscsi_handle_crq(&evt->crq, single_host_data);
}

/* ------------------------------------------------------------
 * Routines for driver initialization
 */
static int iseriesvscsi_init_crq_queue(struct crq_queue *queue,
				       struct ibmvscsi_host_data *hostdata,
				       int max_requests)
{
	int rc;

	single_host_data = hostdata;
	rc = viopath_open(viopath_hostLp, viomajorsubtype_scsi, max_requests);
	if (rc < 0) {
		printk("viopath_open failed with rc %d in open_event_path\n",
		       rc);
		goto viopath_open_failed;
	}

	rc = vio_setHandler(viomajorsubtype_scsi, iseriesvscsi_handle_event);
	if (rc < 0) {
		printk("vio_setHandler failed with rc %d in open_event_path\n",
		       rc);
		goto vio_setHandler_failed;
	}
	return 0;

      vio_setHandler_failed:
	viopath_close(viopath_hostLp, viomajorsubtype_scsi, max_requests);
      viopath_open_failed:
	return -1;
}

static void iseriesvscsi_release_crq_queue(struct crq_queue *queue,
					   struct ibmvscsi_host_data *hostdata,
					   int max_requests)
{
	vio_clearHandler(viomajorsubtype_scsi);
	viopath_close(viopath_hostLp, viomajorsubtype_scsi, max_requests);
}

/**
 * reset_crq_queue: - resets a crq after a failure
 * @queue:	crq_queue to initialize and register
 * @hostdata:	ibmvscsi_host_data of host
 *
 * no-op for iSeries
 */
static int iseriesvscsi_reset_crq_queue(struct crq_queue *queue,
					struct ibmvscsi_host_data *hostdata)
{
	return 0;
}

/**
 * reenable_crq_queue: - reenables a crq after a failure
 * @queue:	crq_queue to initialize and register
 * @hostdata:	ibmvscsi_host_data of host
 *
 * no-op for iSeries
 */
static int iseriesvscsi_reenable_crq_queue(struct crq_queue *queue,
					   struct ibmvscsi_host_data *hostdata)
{
	return 0;
}

/**
 * iseriesvscsi_send_crq: - Send a CRQ
 * @hostdata:	the adapter
 * @word1:	the first 64 bits of the data
 * @word2:	the second 64 bits of the data
 */
static int iseriesvscsi_send_crq(struct ibmvscsi_host_data *hostdata,
				 u64 word1, u64 word2)
{
	single_host_data = hostdata;
	return HvCallEvent_signalLpEventFast(viopath_hostLp,
					     HvLpEvent_Type_VirtualIo,
					     viomajorsubtype_scsi,
					     HvLpEvent_AckInd_NoAck,
					     HvLpEvent_AckType_ImmediateAck,
					     viopath_sourceinst(viopath_hostLp),
					     viopath_targetinst(viopath_hostLp),
					     0,
					     VIOVERSION << 16, word1, word2, 0,
					     0);
}

struct ibmvscsi_ops iseriesvscsi_ops = {
	.init_crq_queue = iseriesvscsi_init_crq_queue,
	.release_crq_queue = iseriesvscsi_release_crq_queue,
	.reset_crq_queue = iseriesvscsi_reset_crq_queue,
	.reenable_crq_queue = iseriesvscsi_reenable_crq_queue,
	.send_crq = iseriesvscsi_send_crq,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       /*
 * Freescale QUICC Engine USB Host Controller Driver
 *
 * Copyright (c) Freescale Semicondutor, Inc. 2006.
 *               Shlomi Gridish <gridish@freescale.com>
 *               Jerry Huang <Chang-Ming.Huang@freescale.com>
 * Copyright (c) Logic Product Development, Inc. 2007
 *               Peter Barada <peterb@logicpd.com>
 * Copyright (c) MontaVista Software, Inc. 2008.
 *               Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __FHCI_H
#define __FHCI_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/kfifo.h>
#include <linux/io.h>
#include <linux/usb.h>
#include <asm/qe.h>
#include "../core/hcd.h"

#define USB_CLOCK	48000000

#define FHCI_PRAM_SIZE 0x100

#define MAX_EDS		32
#define MAX_TDS		32


/* CRC16 field size */
#define CRC_SIZE 2

/* USB protocol overhead for each frame transmitted from the host */
#define PROTOCOL_OVERHEAD 7

/* Packet structure, info field */
#define PKT_PID_DATA0		0x80000000 /* PID - Data toggle zero */
#define PKT_PID_DATA1		0x40000000 /* PID - Data toggle one  */
#define PKT_PID_SETUP		0x20000000 /* PID - Setup bit */
#define PKT_SETUP_STATUS	0x10000000 /* Setup status bit */
#define PKT_SETADDR_STATUS	0x08000000 /* Set address status bit */
#define PKT_SET_HOST_LAST	0x04000000 /* Last data packet */
#define PKT_HOST_DATA		0x02000000 /* Data packet */
#define PKT_FIRST_IN_FRAME	0x01000000 /* First packet in the frame */
#define PKT_TOKEN_FRAME		0x00800000 /* Token packet */
#define PKT_ZLP			0x00400000 /* Zero length packet */
#define PKT_IN_TOKEN_FRAME	0x00200000 /* IN token packet */
#define PKT_OUT_TOKEN_FRAME	0x00100000 /* OUT token packet */
#define PKT_SETUP_TOKEN_FRAME	0x00080000 /* SETUP token packet */
#define PKT_STALL_FRAME		0x00040000 /* STALL packet */
#define PKT_NACK_FRAME		0x00020000 /* NACK packet */
#define PKT_NO_PID		0x00010000 /* No PID */
#define PKT_NO_CRC		0x00008000 /* don't append CRC */
#define PKT_HOST_COMMAND	0x00004000 /* Host command packet */
#define PKT_DUMMY_PACKET	0x00002000 /* Dummy packet, used for mmm */
#define PKT_LOW_SPEED_PACKET	0x00001000 /* Low-Speed packet */

#define TRANS_OK		(0)
#define TRANS_INPROGRESS	(-1)
#define TRANS_DISCARD		(-2)
#define TRANS_FAIL		(-3)

#define PS_INT		0
#define PS_DISCONNECTED	1
#define PS_CONNECTED	2
#define PS_READY	3
#define PS_MISSING	4

/* Transfer Descriptor status field */
#define USB_TD_OK		0x00000000 /* TD transmited or received ok */
#define USB_TD_INPROGRESS	0x80000000 /* TD is being transmitted */
#define USB_TD_RX_ER_NONOCT	0x40000000 /* Tx Non Octet Aligned Packet */
#define USB_TD_RX_ER_BITSTUFF	0x20000000 /* Frame Aborted-Received pkt */
#define USB_TD_RX_ER_CRC	0x10000000 /* CRC error */
#define USB_TD_RX_ER_OVERUN	0x08000000 /* Over - run occured */
#define USB_TD_RX_ER_PID	0x04000000 /* wrong PID received */
#define USB_TD_RX_DATA_UNDERUN	0x02000000 /* shorter than expected */
#define USB_
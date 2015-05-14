/*
 * IPWireless 3G PCMCIA Network Driver
 *
 * Original code
 *   by Stephen Blackheath <stephen@blacksapphire.com>,
 *      Ben Martel <benm@symmetric.co.nz>
 *
 * Copyrighted as follows:
 *   Copyright (C) 2004 by Symmetric Systems Ltd (NZ)
 *
 * Various driver changes and rewrites, port to new kernels
 *   Copyright (C) 2006-2007 Jiri Kosina
 *
 * Misc code cleanups and updates
 *   Copyright (C) 2007 David Sterba
 */

#ifndef _IPWIRELESS_CS_TTY_H_
#define _IPWIRELESS_CS_TTY_H_

#include <linux/types.h>
#include <linux/sched.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ds.h>

struct ipw_tty;
struct ipw_network;
struct ipw_hardware;

int ipwireless_tty_init(void);
void ipwireless_tty_release(void);

struct ipw_tty *ipwireless_tty_create(struct ipw_hardware *hw,
				      struct ipw_network *net,
				      dev_node_t *nodes);
void ipwireless_tty_free(struct ipw_tty *tty);
void ipwireless_tty_received(struct ipw_tty *tty, unsigned char *data,
			     unsigned int length);
int ipwireless_tty_is_modem(struct ipw_tty *tty);
void ipwireless_tty_notify_control_line_change(struct ipw_tty *tty,
					       unsigned int channel_idx,
					       unsigned int control_lines,
					       unsigned int changed_mask);

#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         /*
 * sbp2.h - Defines and prototypes for sbp2.c
 *
 * Copyright (C) 2000 James Goodwin, Filanet Corporation (www.filanet.com)
 * jamesg@filanet.com
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SBP2_H
#define SBP2_H

#define SBP2_DEVICE_NAME		"sbp2"

/*
 * There is no transport protocol limit to the CDB length,  but we implement
 * a fixed length only.  16 bytes is enough for disks larger than 2 TB.
 */
#define SBP2_MAX_CDB_SIZE		16

/*
 * SBP-2 specific definitions
 */

#define ORB_DIRECTION_WRITE_TO_MEDIA	0x0
#define ORB_DIRECTION_READ_FROM_MEDIA	0x1
#define ORB_DIRECTION_NO_DATA_TRANSFER	0x2

#define ORB_SET_NULL_PTR(v)		(((v) & 0x1) << 31)
#define ORB_SET_NOTIFY(v)		(((v) & 0x1) << 31)
#define ORB_SET_RQ_FMT(v)		(((v) & 0x3) << 29)
#define ORB_SET_NODE_ID(v)		(((v) & 0xffff) << 16)
#define ORB_SET_STATUS_FIFO_HI(v, id)	((v) >> 32 | ORB_SET_NODE_ID(id))
#define ORB_SET_STATUS_FIFO_LO(v)	((v) & 0xffffffff)
#define ORB_SET_DATA_SIZE(v)		((v) & 0xffff)
#define ORB_SET_PAGE_SIZE(v)		(((v) & 0x7) << 16)
#define ORB_SET_PAGE_TABLE_PRESENT(v)	(((v) & 0x1) << 19)
#define ORB_SET_MAX_PAYLOAD(v)		(((v) & 0xf) << 20)
#define ORB_SET_SPEED(v)		(((v) & 0x7) << 24)
#define ORB_SET_DIRECTION(v)		(((v) & 0x1) << 27)

struct sbp2_command_orb {
	u32 next_ORB_hi;
	u32 next_ORB_lo;
	u32 data_descriptor_hi;
	u32 data_descriptor_lo;
	u32 misc;
	u8 cdb[SBP2_MAX_CDB_SIZE];
} __attribute__((packed));

#define SBP2_LOGIN_REQUEST		0x0
#define SBP2_QUERY_LOGINS_REQUEST	0x1
#define SBP2_RECONNECT_REQUEST		0x3
#define SBP2_SET_PASSWORD_REQUEST	0x4
#define SBP2_LOGOUT_REQUEST		0x7
#define SBP2_ABORT_TASK_REQUEST		0xb
#define SBP2_ABORT_TASK_SET		0xc
#define SBP2_LOGICAL_UNIT_RESET		0xe
#define SBP2_TARGET_RESET_REQUEST	0xf

#define ORB_SET_LUN(v)			((v) & 0xffff)
#define ORB_SET_FUNCTION(v)		(((v) & 0xf) << 16)
#define ORB_SET_RECONNECT(v)		(((v) & 0xf) << 20)
#define ORB_SET_EXCLUSIVE(v)		((v) ? 1 << 28 : 0)
#define ORB_SET_LOGIN_RESP_LENGTH(v)	((v) & 0xffff)
#define ORB_SET_PASSWD_LENGTH(v)	(((v) & 0xffff) << 16)

struct sbp2_login_orb {
	u32 password_hi;
	u32 password_lo;
	u32 login_response_hi;
	u32 login_response_lo;
	u32 lun_misc;
	u32 passwd_resp_lengths;
	u32 status_fifo_hi;
	u32 status_fifo_lo;
} __attribute__((packed));

#define RESPONSE_GET_LOGIN_ID(v)	((v) & 0xffff)
#define RESPONSE_GET_LENGTH(v)		(((v) >> 16) & 0xffff)
#define RESPONSE_GET_RECONNECT_HOLD(v)	((v) & 0xffff)

struct sbp2_login_response {
	u32 length_login_ID;
	u32 command_block_agent_hi;
	u32 command_block_agent_lo;
	u32 reconnect_hold;
} __attribute__((packed));

#define ORB_SET_LOGIN_ID(v)                 ((v) & 0xffff)
#define ORB_SET_QUERY_LOGINS_RESP_LENGTH(v) ((v) & 0xffff)

struct sbp2_query_logins_orb {
	u32 reserved1;
	u32 reserved2;
	u32 query_response_hi;
	u32 query_response_lo;
	u32 lun_misc;
	u32 reserved_resp_length;
	u32 status_fifo_hi;
	u32 status_fifo_lo;
} __attribute__((packed));

#define RESPONSE_GET_MAX_LOGINS(v)	((v) & 0xffff)
#define RESPONSE_GET_ACTIVE_LOGINS(v)	((RESPONSE_GET_LENGTH((v)) - 4) / 12)

struct sbp2_query_logins_response {
	u32 length_max_logins;
	u32 misc_IDs;
	u32 initiator_misc_hi;
	u32 initiator_misc_lo;
} __attribute__((packed));

struct sbp2_reconnect_orb {
	u32 reserved1;
	u32 reserved2;
	u32 reserved3;
	u32 reserved4;
	u32 login_ID_misc;
	u32 reserved5;
	u32 status_fifo_hi;
	u32 status_fifo_lo;
} __attribute__((packed));

struct sbp2_logout_orb {
	u32 reserved1;
	u32 reserved2;
	u32 reserved3;
	u32 reserved4;
	u32 login_ID_misc;
	u32 reserved5;
	u32 status_fifo_hi;
	u32 status_fifo_lo;
} __attribute__((packed));

struct sbp2_unrestricted_page_table {
	__be32 high;
	__be32 low;
};

#define RESP_STATUS_REQUEST_COMPLETE		0x0
#define RESP_STATUS_TRANSPORT_FAILURE		0x1
#define RESP_STATUS_ILLEGAL_REQUEST		0x2
#define RESP_STATUS_VENDOR_DEPENDENT		0x3

#define SBP2_STATUS_NO_ADDITIONAL_INFO		0x0
#define SBP2_STATUS_REQ_TYPE_NOT_SUPPORTED	0x1
#define SBP2_STATUS_SPEED_NOT_SUPPORTED		0x2
#define SBP2_STATUS_PAGE_SIZE_NOT_SUPPORTED	0x3
#define SBP2_STATUS_ACCESS_DENIED		0x4
#define SBP2_STATUS_LU_NOT_SUPPORTED		0x5
#define SBP2_STATUS_MAX_PAYLOAD_TOO_SMALL	0x6
#define SBP2_STATUS_RESOURCES_UNAVAILABLE	0x8
#define SBP2_STATUS_FUNCTION_REJECTED		0x9
#define SBP2_STATUS_LOGIN_ID_NOT_RECOGNIZED	0xa
#define SBP2_STATUS_DUMMY_ORB_COMPLETED		0xb
#define SBP2_STATUS_REQUEST_ABORTED		0xc
#define SBP2_STATUS_UNSPECIFIED_ERROR		0xff

#define SFMT_CURRENT_ERROR			0x0
#define SFMT_DEFERRED_ERROR			0x1
#define SFMT_VENDOR_DEPENDENT_STATUS		0x3

#define STATUS_GET_SRC(v)			(((v) >> 30) & 0x3)
#define STATUS_GET_RESP(v)			(((v) >> 28) & 0x3)
#define STATUS_GET_LEN(v)			(((v) >> 24) & 0x7)
#define STATUS_GET_SBP_STATUS(v)		(((v) >> 16) & 0xff)
#define STATUS_GET_ORB_OFFSET_HI(v)		((v) & 0x0000ffff)
#define STATUS_TEST_DEAD(v)			((v) & 0x08000000)
/* test 'resp' | 'dead' | 'sbp2_status' */
#define STATUS_TEST_RDS(v)			((v) & 0x38ff0000)

struct sbp2_status_block {
	u32 ORB_offset_hi_misc;
	u32 ORB_offset_lo;
	u8 command_set_dependent[24];
} __attribute__((packed));


/*
 * SBP2 related configuration ROM definitions
 */

#define SBP2_UNIT_DIRECTORY_OFFSET_KEY		0xd1
#define SBP2_CSR_OFFSET_KEY			0x54
#define SBP2_UNIT_SPEC_ID_KEY			0x12
#define SBP2_UNIT_SW_VERSION_KEY		0x13
#define SBP2_COMMAND_SET_SPEC_ID_KEY		0x38
#define SBP2_COMMAND_SET_KEY			0x39
#define SBP2_UNIT_CHARACTERISTICS_KEY		0x3a
#define SBP2_DEVICE_TYPE_AND_LUN_KEY		0x14
#define SBP2_FIRMWARE_REVISION_KEY		0x3c

#define SBP2_AGENT_STATE_OFFSET			0x00ULL
#define SBP2_AGENT_RESET_OFFSET			0x04ULL
#define SBP2_ORB_POINTER_OFFSET			0x08ULL
#define SBP2_DOORBELL_OFFSET			0x10ULL
#define SBP2_UNSOLICITED_STATUS_ENABLE_OFFSET	0x14ULL
#define SBP2_UNSOLICITED_STATUS_VALUE		0xf

#define SBP2_BUSY_TIMEOUT_ADDRESS		0xfffff0000210ULL
/* biggest possible value for Single Phase Retry count is 0xf */
#define SBP2_BUSY_TIMEOUT_VALUE			0xf

#define SBP2_AGENT_RESET_DATA			0xf

#define SBP2_UNIT_SPEC_ID_ENTRY			0x0000609e
#define SBP2_SW_VERSION_ENTRY			0x00010483

/*
 * The default maximum s/g segment size of a FireWire controller is
 * usually 0x10000, but SBP-2 only allows 0xffff. Since buffers have to
 * be quadlet-aligned, we set the length limit to 0xffff & ~3.
 */
#define SBP2_MAX_SEG_SIZE			0xfffc

/*
 * There is no real limitation of the queue depth (i.e. length of the linked
 * list of command ORBs) at the target. The chosen depth is merely an
 * implementation detail of the sbp2 driver.
 */
#define SBP2_MAX_CMDS				8

#define SBP2_SCSI_STATUS_GOOD			0x0
#define SBP2_SCSI_STATUS_CHECK_CONDITION	0x2
#define SBP2_SCSI_STATUS_CONDITION_MET		0x4
#define SBP2_SCSI_STATUS_BUSY			0x8
#define SBP2_SCSI_STATUS_RESERVATION_CONFLICT	0x18
#define SBP2_SCSI_STATUS_COMMAND_TERMINATED	0x22
#define SBP2_SCSI_STATUS_SELECTION_TIMEOUT	0xff


/*
 * Representations of commands and devices
 */

/* Per SCSI command */
struct sbp2_command_info {
	struct list_head list;
	struct sbp2_command_orb command_orb;
	dma_addr_t command_orb_dma;
	struct scsi_cmnd *Current_SCpnt;
	void (*Current_done)(struct scsi_cmnd *);

	/* Also need s/g structure for each sbp2 command */
	struct sbp2_unrestricted_page_table
		scatter_gather_element[SG_ALL] __attribute__((aligned(8)));
	dma_addr_t sge_dma;
};

/* Per FireWire host */
struct sbp2_fwhost_info {
	struct hpsb_host *host;
	struct list_head logical_units;
};

/* Per logical unit */
struct sbp2_lu {
	/* Operation request blocks */
	struct sbp2_command_orb *last_orb;
	dma_addr_t last_orb_dma;
	struct sbp2_login_orb *login_orb;
	dma_addr_t login_orb_dma;
	struct sbp2_login_response *login_response;
	dma_addr_t login_response_dma;
	struct sbp2_query_logins_orb *query_logins_orb;
	dma_addr_t query_logins_orb_dma;
	struct sbp2_query_logins_response *query_logins_response;
	dma_addr_t query_logins_response_dma;
	struct sbp2_reconnect_orb *reconnect_orb;
	dma_addr_t reconnect_orb_dma;
	struct sbp2_logout_orb *logout_orb;
	dma_addr_t logout_orb_dma;
	struct sbp2_status_block status_block;

	/* How to talk to the unit */
	u64 management_agent_addr;
	u64 command_block_agent_addr;
	u32 speed_code;
	u32 max_payload_size;
	u16 lun;

	/* Address for the unit to write status blocks to */
	u64 status_fifo_addr;

	/* Waitqueue flag for logins, reconnects, logouts, query logins */
	unsigned int access_complete:1;

	/* Pool of command ORBs for this logical unit */
	spinlock_t cmd_orb_lock;
	struct list_head cmd_orb_inuse;
	struct list_head cmd_orb_completed;

	/* Backlink to FireWire host; list of units attached to the host */
	struct sbp2_fwhost_info *hi;
	struct list_head lu_list;

	/* IEEE 1394 core's device representations */
	struct node_entry *ne;
	struct unit_directory *ud;

	/* SCSI core's device representations */
	struct scsi_device *sdev;
	struct Scsi_Host *shost;

	/* Device specific workarounds/brokeness */
	unsigned workarounds;

	/* Connection state */
	atomic_t state;

	/* For deferred requests to the fetch agent */
	struct work_struct protocol_work;
};

/* For use in sbp2_lu.state */
enum sbp2lu_state_types {
	SBP2LU_STATE_RUNNING,		/* all normal */
	SBP2LU_STATE_IN_RESET,		/* between bus reset and reconnect */
	SBP2LU_STATE_IN_SHUTDOWN	/* when sbp2_remove was called */
};

/* For use in sbp2_lu.workarounds and in the corresponding
 * module load parameter */
#define SBP2_WORKAROUND_128K_MAX_TRANS	0x1
#define SBP2_WORKAROUND_INQUIRY_36	0x2
#define SBP2_WORKAROUND_MODE_SENSE_8	0x4
#define SBP2_WORKAROUND_FIX_CAPACITY	0x8
#define SBP2_WORKAROUND_DELAY_INQUIRY	0x10
#define SBP2_INQUIRY_DELAY		12
#define SBP2_WORKAROUND_POWER_CONDITION	0x20
#define SBP2_WORKAROUND_OVERRIDE	0x100

#endif /* SBP2_H */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      /*
 *	Video for Linux Two
 *
 *	A generic video device interface for the LINUX operating system
 *	using a set of device structures/vectors for low level operations.
 *
 *	This file replaces the videodev.c file that comes with the
 *	regular kernel distribution.
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 * Author:	Bill Dirks <bill@thedirks.org>
 *		based on code by Alan Cox, <alan@cymru.net>
 *
 */

/*
 * Video capture interface for Linux
 *
 *	A generic video device interface for the LINUX operating system
 *	using a set of device structures/vectors for low level operations.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Author:	Alan Cox, <alan@lxorguk.ukuu.org.uk>
 *
 * Fixes:
 */

/*
 * Video4linux 1/2 integration by Justin Schoeman
 * <justin@suntiger.ee.up.ac.za>
 * 2.4 PROCFS support ported from 2.4 kernels by
 *  Iñaki García Etxebarria <garetxe@euskalnet.net>
 * Makefile fix by "W. Michael Petullo" <mike@flyn.org>
 * 2.4 devfs support ported from 2.4 kernels by
 *  Dan Merillat <dan@merillat.org>
 * Added Gerd Knorrs v4l1 enhancements (Justin Schoeman)
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/div64.h>
#define __OLD_VIDIOC_ /* To allow fixing old calls*/
#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>

#include <linux/videodev2.h>

MODULE_AUTHOR("Bill Dirks, Justin Schoeman, Gerd Knorr");
MODULE_DESCRIPTION("misc helper functions for v4l2 device drivers");
MODULE_LICENSE("GPL");

/*
 *
 *	V 4 L 2   D R I V E R   H E L P E R   A P I
 *
 */

/*
 *  Video Standard Operations (contributed by Michael Schimek)
 */


/* ----------------------------------------------------------------- */
/* priority handling                                                 */

#define V4L2_PRIO_VALID(val) (val == V4L2_PRIORITY_BACKGROUND   || \
			      val == V4L2_PRIORITY_INTERACTIVE  || \
			      val == V4L2_PRIORITY_RECORD)

int v4l2_prio_init(struct v4l2_prio_state *global)
{
	memset(global,0,sizeof(*global));
	return 0;
}
EXPORT_SYMBOL(v4l2_prio_init);

int v4l2_prio_change(struct v4l2_prio_state *global, enum v4l2_priority *local,
		     enum v4l2_priority new)
{
	if (!V4L2_PRIO_VALID(new))
		return -EINVAL;
	if (*local == new)
		return 0;

	atomic_inc(&global->prios[new]);
	if (V4L2_PRIO_VALID(*local))
		atomic_dec(&global->prios[*local]);
	*local = new;
	return 0;
}
EXPORT_SYMBOL(v4l2_prio_change);

int v4l2_prio_open(struct v4l2_prio_state *global, enum v4l2_priority *local)
{
	return v4l2_prio_change(global,local,V4L2_PRIORITY_DEFAULT);
}
EXPORT_SYMBOL(v4l2_prio_open);

int v4l2_prio_close(struct v4l2_prio_state *global, enum v4l2_priority *local)
{
	if (V4L2_PRIO_VALID(*local))
		atomic_dec(&global->prios[*local]);
	return 0;
}
EXPORT_SYMBOL(v4l2_prio_close);

enum v4l2_priority v4l2_prio_max(struct v4l2_prio_state *global)
{
	if (atomic_read(&global->prios[V4L2_PRIORITY_RECORD]) > 0)
		return V4L2_PRIORITY_RECORD;
	if (atomic_read(&global->prios[V4L2_PRIORITY_INTERACTIVE]) > 0)
		return V4L2_PRIORITY_INTERACTIVE;
	if (atomic_read(&global->prios[V4L2_PRIORITY_BACKGROUND]) > 0)
		return V4L2_PRIORITY_BACKGROUND;
	return V4L2_PRIORITY_UNSET;
}
EXPORT_SYMBOL(v4l2_prio_max);

int v4l2_prio_check(struct v4l2_prio_state *global, enum v4l2_priority *local)
{
	if (*local < v4l2_prio_max(global))
		return -EBUSY;
	return 0;
}
EXPORT_SYMBOL(v4l2_prio_check);

/* ----------------------------------------------------------	8, 9,
	/* Analog */
	10, 11,
	/* AO4S-192 and AI4S-192 extension boards */
	12, 13, 14, 15,
	/* others don't exist */
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1
};

static int snd_hammerfall_get_buffer(struct pci_dev *pci, struct snd_dma_buffer *dmab, size_t size)
{
	dmab->dev.type = SNDRV_DMA_TYPE_DEV;
	dmab->dev.dev = snd_dma_pci_data(pci);
	if (snd_dma_get_reserved_buf(dmab, snd_dma_pci_buf_id(pci))) {
		if (dmab->bytes >= size)
			return 0;
	}
	if (snd_dma_alloc_pages(SNDRV_DMA_TYPE_DEV, snd_dma_pci_data(pci),
				size, dmab) < 0)
		return -ENOMEM;
	return 0;
}

static void snd_hammerfall_free_buffer(struct snd_dma_buffer *dmab, struct pci_dev *pci)
{
	if (dmab->area) {
		dmab->dev.dev = NULL; /* make it anonymous */
		snd_dma_reserve_buf(dmab, snd_dma_pci_buf_id(pci));
	}
}


static struct pci_device_id snd_hdsp_ids[] = {
	{
		.vendor = PCI_VENDOR_ID_XILINX,
		.device = PCI_DEVICE_ID_XILINX_HAMMERFALL_DSP,
		.subvendor = PCI_ANY_ID,
		.subdevice = PCI_ANY_ID,
	}, /* RME Hammerfall-DSP */
	{ 0, },
};

MODULE_DEVICE_TABLE(pci, snd_hdsp_ids);

/* prototypes */
static int snd_hdsp_create_alsa_devices(struct snd_card *card, struct hdsp *hdsp);
static int snd_hdsp_create_pcm(struct snd_card *card, struct hdsp *hdsp);
static int snd_hdsp_enable_io (struct hdsp *hdsp);
static void snd_hdsp_initialize_midi_flush (struct hdsp *hdsp);
static void snd_hdsp_initialize_channels (struct hdsp *hdsp);
static int hdsp_fifo_wait(struct hdsp *hdsp, int count, int timeout);
static int hdsp_autosync_ref(struct hdsp *hdsp);
static int snd_hdsp_set_defaults(struct hdsp *hdsp);
static void snd_hdsp_9652_enable_mixer (struct hdsp *hdsp);

static int hdsp_playback_to_output_key (struct hdsp *hdsp, int in, int out)
{
	switch (hdsp->io_type) {
	case Multiface:
	case Digiface:
	default:
		if (hdsp->firmware_rev == 0xa)
			return (64 * out) + (32 + (in));
		else
			return (52 * out) + (26 + (in));
	case H9632:
		return (32 * out) + (16 + (in));
	case H9652:
		return (52 * out) + (26 + (in));
	}
}

static int hdsp_input_to_output_key (struct hdsp *hdsp, int in, int out)
{
	switch (hdsp->io_type) {
	case Multiface:
	case Digiface:
	default:
		if (hdsp->firmware_rev == 0xa)
			return (64 * out) + in;
		else
			return (52 * out) + in;
	case H9632:
		return (32 * out) + in;
	case H9652:
		return (52 * out) + in;
	}
}

static void hdsp_write(struct hdsp *hdsp, int reg, int val)
{
	writel(val, hdsp->iobase + reg);
}

static unsigned int hdsp_read(struct hdsp *hdsp, int reg)
{
	return readl (hdsp->iobase + reg);
}

static int hdsp_check_for_iobox (struct hdsp *hdsp)
{
	if (hdsp->io_type == H9652 || hdsp->io_type == H9632) return 0;
	if (hdsp_read (hdsp, HDSP_statusRegister) & HDSP_ConfigError) {
		snd_printk ("Hammerfall-DSP: no Digiface or Multiface connected!\n");
		hdsp->state &= ~HDSP_FirmwareLoaded;
		return -EIO;
	}
	return 0;
}

static int hdsp_wait_for_iobox(struct hdsp *hdsp, unsigned int loops,
			       unsigned int delay)
{
	unsigned int i;

	if (hdsp->io_type == H9652 || hdsp->io_type == H9632)
		return 0;

	for (i = 0; i != loops; ++i) {
		if (hdsp_read(hdsp, HDSP_statusRegister) & HDSP_ConfigError)
			msleep(delay);
		else {
			snd_printd("Hammerfall-DSP: iobox found after %ums!\n",
				   i * delay);
			return 0;
		}
	}

	snd_printk("Hammerfall-DSP: no Digiface or Multiface connected!\n");
	hdsp->state &= ~HDSP_FirmwareLoaded;
	return -EIO;
}

static int snd_hdsp_load_firmware_from_cache(struct hdsp *hdsp) {

	int i;
	unsigned long flags;

	if ((hdsp_read (hdsp, HDSP_statusRegister) & HDSP_DllError) != 0) {

		snd_printk ("Hammerfall-DSP: loading firmware\n");

		hdsp_write (hdsp, HDSP_control2Reg, HDSP_S_PROGRAM);
		hdsp_write (hdsp, HDSP_fifoData, 0);

		if (hdsp_fifo_wait (hdsp, 0, HDSP_LONG_WAIT)) {
			snd_printk ("Hammerfall-DSP: timeout waiting for download preparation\n");
			return -EIO;
		}

		hdsp_write (hdsp, HDSP_control2Reg, HDSP_S_LOAD);

		for (i = 0; i < 24413; ++i) {
			hdsp_write(hdsp, HDSP_fifoData, hdsp->firmware_cache[i]);
			if (hdsp_fifo_wait (hdsp, 127, HDSP_LONG_WAIT)) {
				snd_prig_audio_encoding;
		case V4L2_CID_MPEG_AUDIO_L1_BITRATE:
			return mpeg_audio_l1_bitrate;
		case V4L2_CID_MPEG_AUDIO_L2_BITRATE:
			return mpeg_audio_l2_bitrate;
		case V4L2_CID_MPEG_AUDIO_L3_BITRATE:
			return mpeg_audio_l3_bitrate;
		case V4L2_CID_MPEG_AUDIO_AC3_BITRATE:
			return mpeg_audio_ac3_bitrate;
		case V4L2_CID_MPEG_AUDIO_MODE:
			return mpeg_audio_mode;
		case V4L2_CID_MPEG_AUDIO_MODE_EXTENSION:
			return mpeg_audio_mode_extension;
		case V4L2_CID_MPEG_AUDIO_EMPHASIS:
			return mpeg_audio_emphasis;
		case V4L2_CID_MPEG_AUDIO_CRC:
			return mpeg_audio_crc;
		case V4L2_CID_MPEG_VIDEO_ENCODING:
			return mpeg_video_encoding;
		case V4L2_CID_MPEG_VIDEO_ASPECT:
			return mpeg_video_aspect;
		case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
			return mpeg_video_bitrate_mode;
		case V4L2_CID_MPEG_STREAM_TYPE:
			return mpeg_stream_type;
		case V4L2_CID_MPEG_STREAM_VBI_FMT:
			return mpeg_stream_vbi_fmt;
		case V4L2_CID_POWER_LINE_FREQUENCY:
			return camera_power_line_frequency;
		case V4L2_CID_EXPOSURE_AUTO:
			return camera_exposure_auto;
		case V4L2_CID_COLORFX:
			return colorfx;
		default:
			return NULL;
	}
}
EXPORT_SYMBOL(v4l2_ctrl_get_menu);

/* Return the control name. */
const char *v4l2_ctrl_get_name(u32 id)
{
	switch (id) {
	/* USER controls */
	case V4L2_CID_USER_CLASS: 		return "User Controls";
	case V4L2_CID_BRIGHTNESS: 		return "Brightness";
	case V4L2_CID_CONTRAST: 		return "Contrast";
	case V4L2_CID_SATURATION: 		return "Saturation";
	case V4L2_CID_HUE: 			return "Hue";
	case V4L2_CID_AUDIO_VOLUME: 		return "Volume";
	case V4L2_CID_AUDIO_BALANCE: 		return "Balance";
	case V4L2_CID_AUDIO_BASS: 		return "Bass";
	case V4L2_CID_AUDIO_TREBLE: 		return "Treble";
	case V4L2_CID_AUDIO_MUTE: 		return "Mute";
	case V4L2_CID_AUDIO_LOUDNESS: 		return "Loudness";
	case V4L2_CID_BLACK_LEVEL:		return "Black Level";
	case V4L2_CID_AUTO_WHITE_BALANCE:	return "White Balance, Automatic";
	case V4L2_CID_DO_WHITE_BALANCE:		return "Do White Balance";
	case V4L2_CID_RED_BALANCE:		return "Red Balance";
	case V4L2_CID_BLUE_BALANCE:		return "Blue Balance";
	case V4L2_CID_GAMMA:			return "Gamma";
	case V4L2_CID_EXPOSURE:			return "Exposure";
	case V4L2_CID_AUTOGAIN:			return "Gain, Automatic";
	case V4L2_CID_GAIN:			return "Gain";
	case V4L2_CID_HFLIP:			return "Horizontal Flip";
	case V4L2_CID_VFLIP:			return "Vertical Flip";
	case V4L2_CID_HCENTER:			return "Horizontal Center";
	case V4L2_CID_VCENTER:			return "Vertical Center";
	case V4L2_CID_POWER_LINE_FREQUENCY:	return "Power Line Frequency";
	case V4L2_CID_HUE_AUTO:			return "Hue, Automatic";
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE: return "White Balance Temperature";
	case V4L2_CID_SHARPNESS:		return "Sharpness";
	case V4L2_CID_BACKLIGHT_COMPENSATION:	return "Backlight Compensation";
	case V4L2_CID_CHROMA_AGC:		return "Chroma AGC";
	case V4L2_CID_COLOR_KILLER:		return "Color Killer";
	case V4L2_CID_COLORFX:			return "Color Effects";

	/* MPEG controls */
	case V4L2_CID_MPEG_CLASS: 		return "MPEG Encoder Controls";
	case V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ: return "Audio Sampling Frequency";
	case V4L2_CID_MPEG_AUDIO_ENCODING: 	return "Audio Encoding";
	case V4L2_CID_MPEG_AUDIO_L1_BITRATE: 	return "Audio Layer I Bitrate";
	case V4L2_CID_MPEG_AUDIO_L2_BITRATE: 	return "Audio Layer II Bitrate";
	case V4L2_CID_MPEG_AUDIO_L3_BITRATE: 	return "Audio Layer III Bitrate";
	case V4L2_CID_MPEG_AUDIO_AAC_BITRATE: 	return "Audio AAC Bitrate";
	case V4L2_CID_MPEG_AUDIO_AC3_BITRATE: 	return "Audio AC-3 Bitrate";
	case V4L2_CID_MPEG_AUDIO_MODE: 		return "Audio Stereo Mode";
	case V4L2_CID_MPEG_AUDIO_MODE_EXTENSION: return "Audio Stereo Mode Extension";
	case V4L2_CID_MPEG_AUDIO_EMPHASIS: 	return "Audio Emphasis";
	case V4L2_CID_MPEG_AUDIO_CRC: 		return "Audio CRC";
	case V4L2_CID_MPEG_AUDIO_MUTE: 		return "Audio Mute";
	case V4L2_CID_MPEG_VIDEO_ENCODING: 	return "Video Encoding";
	case V4L2_CID_MPEG_VIDEO_ASPECT: 	return "Video Aspect";
	case V4L2_CID_MPEG_VIDEO_B_FRAMES: 	return "Video B Frames";
	case V4L2_CID_MPEG_VIDEO_GOP_SIZE: 	return "Video GOP Size";
	case V4L2_CID_MPEG_VIDEO_GOP_CLOSURE: 	return "Video GOP Closure";
	case V4L2_CID_MPEG_VIDEO_PULLDOWN: 	return "Video Pulldown";
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE: 	return "Video Bitrate Mode";
	case V4L2_CID_MPEG_VIDEO_BITRATE: 	return "Video Bitrate";
	case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK: 	return "Video Peak Bitrate";
	case V4L2_CID_MPEG_VIDEO_TEMPORAL_DECIMATION: return "Video Temporal Decimation";
	case V4L2_CID_MPEG_VIDEO_MUTE: 		return "Video Mute";
	case V4L2_CID_MPEG_VIDEO_MUTE_YUV:	return "Video Mute YUV";
	case V4L2_CID_MPEG_STREAM_TYPE: 	return "Stream Type";
	case V4L2_CID_MPEG_STREAM_PID_PMT: 	return "Stream PMT Program ID";
	case V4L2_CID_MPEG_STREAM_PID_AUDIO: 	return "Stream Audio Program ID";
	case V4L2_CID_MPEG_STREAM_PID_VIDEO: 	return "Stream Video Program ID";
	case V4L2_CID_MPEG_STREAM_PID_PCR: 	return "Stream PCR Program ID";
	case V4L2_CID_MPEG_STREAM_PES_ID_AUDIO: return "Stream PES Audio ID";
	case V4L2_CID_MPEG_STREAM_PES_ID_VIDEO: return "Stream PES Video ID";
	case V4L2_CID_MPEG_STREAM_VBI_FMT:	return "Stream VBI Format";

	/* CAMERA controls */
	case V4L2_CID_CAMERA_CLASS:		return "Camera Controls";
	case V4L2_CID_EXPOSURE_AUTO:		return "Auto Exposure";
	case V4L2_CID_EXPOSURE_ABSOLUTE:	return "Exposure Time, Absolute";
	case V4L2_CID_EXPOSURE_AUTO_PRIORITY:	return "Exposure, Dynamic Framerate";
	case V4L2_CID_PAN_RELATIVE:		return "Pan, Relative";
	case V4L2_CID_TILT_RELATIVE:		return "Tilt, Relative";
	case V4L2_CID_PAN_RESET:		return "Pan, Reset";
	case V4L2_CID_TILT_RESET:		return "Tilt, Reset";
	case V4L2_CID_PAN_ABSOLUTE:		return "Pan, Absolute";
	case V4L2_CID_TILT_ABSOLUTE:		return "Tilt, Absolute";
	case V4L2_CID_FOCUS_ABSOLUTE:		return "Focus, Absolute";
	case V4L2_CID_FOCUS_RELATIVE:		return "Focus, Relative";
	case V4L2_CID_FOCUS_AUTO:		return "Focus, Automatic";
	case V4L2_CID_ZOOM_ABSOLUTE:		return "Zoom, Absolute";
	case V4L2_CID_ZOOM_RELATIVE:		return "Zoom, Relative";
	case V4L2_CID_ZOOM_CONTINUOUS:		return "Zoom, Continuous";
	case V4L2_CID_PRIVACY:			return "Privacy";

	default:
		return NULL;
	}
}
EXPORT_SYMBOL(v4l2_ctrl_get_name);

/* Fill in a struct v4l2_queryctrl */
int v4l2_ctrl_query_fill(struct v4l2_queryctrl *qctrl, s32 min, s32 max, s32 step, s32 def)
{
	const char *name = v4l2_ctrl_get_name(qctrl->id);

	qctrl->flags = 0;
	if (name == NULL)
		return -EINVAL;

	switch (qctrl->id) {
	case V4L2_CID_AUDIO_MUTE:
	case V4L2_CID_AUDIO_LOUDNESS:
	case V4L2_CID_AUTO_WHITE_BALANCE:
	case V4L2_CID_AUTOGAIN:
	case V4L2_CID_HFLIP:
	case V4L2_CID_VFLIP:
	case V4L2_CID_HUE_AUTO:
	case V4L2_CID_CHROMA_AGC:
	case V4L2_CID_COLOR_KILLER:
	case V4L2_CID_MPEG_AUDIO_MUTE:
	case V4L2_CID_MPEG_VIDEO_MUTE:
	case V4L2_CID_MPEG_VIDEO_GOP_CLOSURE:
	case V4L2_CID_MPEG_VIDEO_PULLDOWN:
	case V4L2_CID_EXPOSURE_AUTO_PRIORITY:
	case V4L2_CID_FOCUS_AUTO:
	case V4L2_CID_PRIVACY:
		qctrl->type = V4L2_CTRL_TYPE_BOOLEAN;
		min = 0;
		max = step = 1;
		break;
	case V4L2_CID_PAN_RESET:
	case V4L2_CID_TILT_RESET:
		qctrl->type = V4L2_CTRL_TYPE_BUTTON;
		qctrl->flags |= V4L2_CTRL_FLAG_WRITE_ONLY;
		min = max = step = def = 0;
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
	case V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ:
	case V4L2_CID_MPEG_AUDIO_ENCODING:
	case V4L2_CID_MPEG_AUDIO_L1_BITRATE:
	case V4L2_CID_MPEG_AUDIO_L2_BITRATE:
	case V4L2_CID_MPEG_AUDIO_L3_BITRATE:
	case V4L2_CID_MPEG_AUDIO_AC3_BITRATE:
	case V4L2_CID_MPEG_AUDIO_MODE:
	case V4L2_CID_MPEG_AUDIO_MODE_EXTENSION:
	case V4L2_CID_MPEG_AUDIO_EMPHASIS:
	case V4L2_CID_MPEG_AUDIO_CRC:
	case V4L2_CID_MPEG_VIDEO_ENCODING:
	case V4L2_CID_MPEG_VIDEO_ASPECT:
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
	case V4L2_CID_MPEG_STREAM_TYPE:
	case V4L2_CID_MPEG_STREAM_VBI_FMT:
	case V4L2_CID_EXPOSURE_AUTO:
	case V4L2_CID_COLORFX:
		qctrl->type = V4L2_CTRL_TYPE_MENU;
		step = 1;
		break;
	case V4L2_CID_USER_CLASS:
	case V4L2_CID_CAMERA_CLASS:
	case V4L2_CID_MPEG_CLASS:
		qctrl->type = V4L2_CTRL_TYPE_CTRL_CLASS;
		qctrl->flags |= V4L2_CTRL_FLAG_READ_ONLY;
		min = max = step = def = 0;
		break;
	default:
		qctrl->type = V4L2_CTRL_TYPE_INTEGER;
		break;
	}
	switch (qctrl->id) {
	case V4L2_CID_MPEG_AUDIO_ENCODING:
	case V4L2_CID_MPEG_AUDIO_MODE:
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
	case V4L2_CID_MPEG_VIDEO_B_FRAMES:
	case V4L2_CID_MPEG_STREAM_TYPE:
		qctrl->flags |= V4L2_CTRL_FLAG_UPDATE;
		break;
	case V4L2_CID_AUDIO_VOLUME:
	case V4L2_CID_AUDIO_BALANCE:
	case V4L2_CID_AUDIO_BASS:
	case V4L2_CID_AUDIO_TREBLE:
	case V4L2_CID_BRIGHTNESS:
	case V4L2_CID_CONTRAST:
	case V4L2_CID_SATURATION:
	case V4L2_CID_HUE:
	case V4L2_CID_RED_BALANCE:
	case V4L2_CID_BLUE_BALANCE:
	case V4L2_CID_GAMMA:
	case V4L2_CID_SHARPNESS:
		qctrl->flags |= V4L2_CTRL_FLAG_SLIDER;
		break;
	case V4L2_CID_PAN_RELATIVE:
	case V4L2_CID_TILT_RELATIVE:
	case V4L2_CID_FOCUS_RELATIVE:
	case V4L2_CID_ZOOM_RELATIVE:
		qctrl->flags |= V4L2_CTRL_FLAG_WRITE_ONLY;
		break;
	}
	qctrl->minimum = min;
	qctrl->maximum = max;
	qctrl->step = step;
	qctrl->default_value = def;
	qctrl->reserved[0] = qctrl->reserved[1] = 0;
	strlcpy(qctrl->name, name, sizeof(qctrl->name));
	return 0;
}
EXPORT_SYMBOL(v4l2_ctrl_query_fill);

/* Fill in a struct v4l2_querymenu based on the struct v4l2_queryctrl and
   the menu. The qctrl pointer may be NULL, in which case it is ignored.
   If menu_items is NULL, then the menu items are retrieved using
   v4l2_ctrl_get_menu. */
int v4l2_ctrl_query_menu(struct v4l2_querymenu *qmenu, struct v4l2_queryctrl *qctrl,
	       const char **menu_items)
{
	int i;

	qmenu->reserved = 0;
	if (menu_items == NULL)
		menu_items = v4l2_ctrl_get_menu(qmenu->id);
	if (menu_items == NULL ||
	    (qctrl && (qmenu->index < qctrl->minimum || qmenu->index > qctrl->maximum)))
		return -EINVAL;
	for (i = 0; i < qmenu->index && menu_items[i]; i++) ;
	if (menu_items[i] == NULL || menu_items[i][0] == '\0')
		return -EINVAL;
	strlcpy(qmenu->name, menu_items[qmenu->index], sizeof(qmenu->name));
	return 0;
}
EXPORT_SYMBOL(v4l2_ctrl_query_menu);

/* Fill in a struct v4l2_querymenu based on the specified array of valid
   menu items (terminated by V4L2_CTRL_MENU_IDS_END).
   Use this if there are 'holes' in the list of valid menu items. */
int v4l2_ctrl_query_menu_valid_items(struct v4l2_querymenu *qmenu, const u32 *ids)
{
	const char **menu_items = v4l2_ctrl_get_menu(qmenu->id);

	qmenu->reserved = 0;
	if (menu_items == NULL || ids == NULL)
		return -EINVAL;
	while (*ids != V4L2_CTRL_MENU_IDS_END) {
		if (*ids++ == qmenu->index) {
			strlcpy(qmenu->name, menu_items[qmenu->index],
					sizeof(qmenu->name));
			return 0;
		}
	}
	return -EINVAL;
}
EXPORT_SYMBOL(v4l2_ctrl_query_menu_valid_items);

/* ctrl_classes points to an array of u32 pointers, the last element is
   a NULL pointer. Each u32 array is a 0-terminated array of control IDs.
   Each array must be sorted low to high and belong to the same control
   class. The array of u32 pointers must also be sorted, from low class IDs
   to high class IDs.

   This function returns the first ID that follows after the given ID.
   When no more controls are available 0 is returned. */
u32 v4l2_ctrl_next(const u32 * const * ctrl_classes, u32 id)
{
	u32 ctrl_class = V4L2_CTRL_ID2CLASS(id);
	const u32 *pctrl;

	if (ctrl_classes == NULL)
		return 0;

	/* if no query is desired, then check if the ID is part of ctrl_classes */
	if ((id & V4L2_CTRL_FLAG_NEXT_CTRL) == 0) {
		/* find class */
		while (*ctrl_classes && V4L2_CTRL_ID2CLASS(**ctrl_classes) != ctrl_class)
			ctrl_classes++;
		if (*ctrl_classes == NULL)
			return 0;
		pctrl = *ctrl_classes;
		/* find control ID */
		while (*pctrl && *pctrl != id) pctrl++;
		return *pctrl ? id : 0;
	}
	id &= V4L2_CTRL_ID_MASK;
	id++;	/* select next control */
	/* find first class that matches (or is greater than) the class of
	   the ID */
	while (*ctrl_classes && V4L2_CTRL_ID2CLASS(**ctrl_classes) < ctrl_class)
		ctrl_classes++;
	/* no more classes */
	if (*ctrl_classes == NULL)
		return 0;
	pctrl = *ctrl_classes;
	/* find first ctrl within the class that is >= ID */
	while (*pctrl && *pctrl < id) pctrl++;
	if (*pctrl)
		return *pctrl;
	/* we are at the end of the controls of the current class. */
	/* continue with next class if available */
	ctrl_classes++;
	if (*ctrl_classes == NULL)
		return 0;
	return **ctrl_classes;
}
EXPORT_SYMBOL(v4l2_ctrl_next);

int v4l2_chip_match_host(const struct v4l2_dbg_match *match)
{
	switch (match->type) {
	case V4L2_CHIP_MATCH_HOST:
		return match->addr == 0;
	default:
		return 0;
	}
}
EXPORT_SYMBOL(v4l2_chip_match_host);

#if defined(CONFIG_I2C) || (defined(CONFIG_I2C_MODULE) && defined(MODULE))
int v4l2_chip_match_i2c_client(struct i2c_client *c, const struct v4l2_dbg_match *match)
{
	int len;

	if (c == NULL || match == NULL)
		return 0;

	switch (match->type) {
	case V4L2_CHIP_MATCH_I2C_DRIVER:
		if (c->driver == NULL || c->driver->driver.name == NULL)
			return 0;
		len = strlen(c->driver->driver.name);
		/* legacy drivers have a ' suffix, don't try to match that */
		if (len && c->driver->driver.name[len - 1] == '\'')
			len--;
		return len && !strncmp(c->driver->driver.name, match->name, len);
	case V4L2_CHIP_MATCH_I2C_ADDR:
		return c->addr == match->addr;
	default:
		return 0;
	}
}
EXPORT_SYMBOL(v4l2_chip_match_i2c_client);

int v4l2_chip_ident_i2c_client(struct i2c_client *c, struct v4l2_dbg_chip_ident *chip,
		u32 ident, u32 revision)
{
	if (!v4l2_chip_match_i2c_client(c, &chip->match))
		return 0;
	if (chip->ident == V4L2_IDENT_NONE) {
		chip->ident = ident;
		chip->revision = revision;
	}
	else {
		chip->ident = V4L2_IDENT_AMBIGUOUS;
		chip->revision = 0;
	}
	return 0;
}
EXPORT_SYMBOL(v4l2_chip_ident_i2c_client);

/* ----------------------------------------------------------------- */

/* I2C Helper functions */


void v4l2_i2c_subdev_init(struct v4l2_subdev *sd, struct i2c_client *client,
		const struct v4l2_subdev_ops *ops)
{
	v4l2_subdev_init(sd, ops);
	sd->flags |= V4L2_SUBDEV_FL_IS_I2C;
	/* the owner is the same as the i2c_client's driver owner */
	sd->owner = client->driver->driver.owner;
	/* i2c_client and v4l2_subdev point to one another */
	v4l2_set_subdevdata(sd, client);
	i2c_set_clientdata(client, sd);
	/* initialize name */
	snprintf(sd->name, sizeof(sd->name), "%s %d-%04x",
		client->driver->driver.name, i2c_adapter_id(client->adapter),
		client->addr);
}
EXPORT_SYMBOL_GPL(v4l2_i2c_subdev_init);



/* Load an i2c sub-device. */
struct v4l2_subdev *v4l2_i2c_new_subdev(struct v4l2_device *v4l2_dev,
		struct i2c_adapter *adapter,
		const char *module_name, const char *client_type, u8 addr)
{
	struct v4l2_subdev *sd = NULL;
	struct i2c_client *client;
	struct i2c_board_info info;

	BUG_ON(!v4l2_dev);

	if (module_name)
		request_module(module_name);

	/* Setup the i2c board info with the device type and
	   the device address. */
	memset(&info, 0, sizeof(info));
	strlcpy(info.type, client_type, sizeof(info.type));
	info.addr = addr;

	/* Create the i2c client */
	client = i2c_new_device(adapter, &info);
	/* Note: it is possible in the future that
	   c->driver is NULL if the driver is still being loaded.
	   We need better support from the kernel so that we
	   can easily wait for the load to finish. */
	if (client == NULL || client->driver == NULL)
		goto error;

	/* Lock the module so we can safely get the v4l2_subdev pointer */
	if (!try_module_get(client->driver->driver.owner))
		goto error;
	sd = i2c_get_clientdata(client);

	/* Register with the v4l2_device which increases the module's
	   use count as well. */
	if (v4l2_device_register_subdev(v4l2_dev, sd))
		sd = NULL;
	/* Decrease the module use count to match the first try_module_get. */
	module_put(client->driver->driver.owner);

	if (sd) {
		/* We return errors from v4l2_subdev_call only if we have the
		   callback as the .s_config is not mandatory */
		int err = v4l2_subdev_call(sd, core, s_config, 0, NULL);

		if (err && err != -ENOIOCTLCMD) {
			v4l2_device_unregister_subdev(sd);
			sd = NULL;
		}
	}

error:
	/* If we have a client but no subdev, then something went wrong and
	   we must unregister the client. */
	if (client && sd == NULL)
		i2c_unregister_device(client);
	return sd;
}
EXPORT_SYMBOL_GPL(v4l2_i2c_new_subdev);

/* Probe and load an i2c sub-device. */
struct v4l2_subdev *v4l2_i2c_new_probed_subdev(struct v4l2_device *v4l2_dev,
	struct i2c_adapter *adapter,
	const charnsigned long flags;
	u32 ie;

	hmidi = (struct hdsp_midi *) substream->rmidi->private_data;
	hdsp = hmidi->hdsp;
	ie = hmidi->id ? HDSP_Midi1InterruptEnable : HDSP_Midi0InterruptEnable;
	spin_lock_irqsave (&hdsp->lock, flags);
	if (up) {
		if (!(hdsp->control_register & ie)) {
			snd_hdsp_flush_midi_input (hdsp, hmidi->id);
			hdsp->control_register |= ie;
		}
	} else {
		hdsp->control_register &= ~ie;
		tasklet_kill(&hdsp->midi_tasklet);
	}

	hdsp_write(hdsp, HDSP_controlRegister, hdsp->control_register);
	spin_unlock_irqrestore (&hdsp->lock, flags);
}

static void snd_hdsp_midi_output_timer(unsigned long data)
{
	struct hdsp_midi *hmidi = (struct hdsp_midi *) data;
	unsigned long flags;

	snd_hdsp_midi_output_write(hmidi);
	spin_lock_irqsave (&hmidi->lock, flags);

	/* this does not bump hmidi->istimer, because the
	   kernel automatically removed the timer when it
	   expired, and we are now adding it back, thus
	   leaving istimer wherever it was set before.
	*/

	if (hmidi->istimer) {
		hmidi->timer.expires = 1 + jiffies;
		add_timer(&hmidi->timer);
	}

	spin_unlock_irqrestore (&hmidi->lock, flags);
}

static void snd_hdsp_midi_output_trigger(struct snd_rawmidi_substream *substream, int up)
{
	struct hdsp_midi *hmidi;
	unsigned long flags;

	hmidi = (struct hdsp_midi *) substream->rmidi->private_data;
	spin_lock_irqsave (&hmidi->lock, flags);
	if (up) {
		if (!hmidi->istimer) {
			init_timer(&hmidi->timer);
			hmidi->timer.function = snd_hdsp_midi_output_timer;
			hmidi->timer.data = (unsigned long) hmidi;
			hmidi->timer.expires = 1 + jiffies;
			add_timer(&hmidi->timer);
			hmidi->istimer++;
		}
	} else {
		if (hmidi->istimer && --hmidi->istimer <= 0)
			del_timer (&hmidi->timer);
	}
	spin_unlock_irqrestore (&hmidi->lock, flags);
	if (up)
		snd_hdsp_midi_output_write(hmidi);
}

static int snd_hdsp_midi_input_open(struct snd_rawmidi_substream *substream)
{
	struct hdsp_midi *hmidi;

	hmidi = (struct hdsp_midi *) substream->rmidi->private_data;
	spin_lock_irq (&hmidi->lock);
	snd_hdsp_flush_midi_input (hmidi->hdsp, hmidi->id);
	hmidi->input = substream;
	spin_unlock_irq (&hmidi->lock);

	return 0;
}

static int snd_hdsp_midi_output_open(struct snd_rawmidi_substream *substream)
{
	struct hdsp_midi *hmidi;

	hmidi = (struct hdsp_midi *) substream->rmidi->private_data;
	spin_lock_irq (&hmidi->lock);
	hmidi->output = substream;
	spin_unlock_irq (&hmidi->lock);

	return 0;
}

static int snd_hdsp_midi_input_close(struct snd_rawmidi_substream *substream)
{
	struct hdsp_midi *hmidi;

	snd_hdsp_midi_input_trigger (substream, 0);

	hmidi = (struct hdsp_midi *) substream->rmidi->private_data;
	spin_lock_irq (&hmidi->lock);
	hmidi->input = NULL;
	spin_unlock_irq (&hmidi->lock);

	return 0;
}

static int snd_hdsp_midi_output_close(struct snd_rawmidi_substream *substream)
{
	struct hdsp_midi *hmidi;

	snd_hdsp_midi_output_trigger (substream, 0);

	hmidi = (struct hdsp_midi *) substream->rmidi->private_data;
	spin_lock_irq (&hmidi->lock);
	hmidi->output = NULL;
	spin_unlock_irq (&hmidi->lock);

	return 0;
}

static struct snd_rawmidi_ops snd_hdsp_midi_output =
{
	.open =		snd_hdsp_midi_output_open,
	.close =	snd_hdsp_midi_output_close,
	.trigger =	snd_hdsp_midi_output_trigger,
};

static struct snd_rawmidi_ops snd_hdsp_midi_input =
{
	.open =		snd_hdsp_midi_input_open,
	.close =	snd_hdsp_midi_input_close,
	.trigger =	snd_hdsp_midi_input_trigger,
};

static int snd_hdsp_create_midi (struct snd_card *card, struct hdsp *hdsp, int id)
{
	char buf[32];

	hdsp->midi[id].id = id;
	hdsp->midi[id].rmidi = NULL;
	hdsp->midi[id].input = NULL;
	hdsp->midi[id].output = NULL;
	hdsp->midi[id].hdsp = hdsp;
	hdsp->midi[id].istimer = 0;
	hdsp->midi[id].pending = 0;
	spin_lock_init (&hdsp->midi[id].lock);

	sprintf (buf, "%s MIDI %d", card->shortname, id+1);
	if (snd_rawmidi_new (card, buf, id, 1, 1, &hdsp->midi[id].rmidi) < 0)
		return -1;

	sprintf(hdsp->midi[id].rmidi->name, "HDSP MIDI %d", id+1);
	hdsp->midi[id].rmidi->private_data = &hdsp->midi[id];

	snd_rawmidi_set_ops (hdsp->midi[id].rmidi, SNDRV_RAWMIDI_STREAM_OUTPUT, &snd_hdsp_midi_output)w_subdev_board);

struct v4l2_subdev *v4l2_i2c_new_subdev_cfg(struct v4l2_device *v4l2_dev,
		struct i2c_adapter *adapter,
		const char *module_name, const char *client_type,
		int irq, void *platform_data,
		u8 addr, const unsigned short *probe_addrs)
{
	struct i2c_board_info info;

	/* Setup the i2c board info with the device type and
	   the device address. */
	memset(&info, 0, sizeof(info));
	strlcpy(info.type, client_type, sizeof(info.type));
	info.addr = addr;
	info.irq = irq;
	info.platform_data = platform_data;

	return v4l2_i2c_new_subdev_board(v4l2_dev, adapter, module_name,
			&info, probe_addrs);
}
EXPORT_SYMBOL_GPL(v4l2_i2c_new_subdev_cfg);

/* Return i2c client address of v4l2_subdev. */
unsigned short v4l2_i2c_subdev_addr(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return client ? client->addr : I2C_CLIENT_END;
}
EXPORT_SYMBOL_GPL(v4l2_i2c_subdev_addr);

/* Return a list of I2C tuner addresses to probe. Use only if the tuner
   addresses are unknown. */
const unsigned short *v4l2_i2c_tuner_addrs(enum v4l2_i2c_tuner_type type)
{
	static const unsigned short radio_addrs[] = {
#if defined(CONFIG_MEDIA_TUNER_TEA5761) || defined(CONFIG_MEDIA_TUNER_TEA5761_MODULE)
		0x10,
#endif
		0x60,
		I2C_CLIENT_END
	};
	static const unsigned short demod_addrs[] = {
		0x42, 0x43, 0x4a, 0x4b,
		I2C_CLIENT_END
	};
	static const unsigned short tv_addrs[] = {
		0x42, 0x43, 0x4a, 0x4b,		/* tda8290 */
		0x60, 0x61, 0x62, 0x63, 0x64,
		I2C_CLIENT_END
	};

	switch (type) {
	case ADDRS_RADIO:
		return radio_addrs;
	case ADDRS_DEMOD:
		return demod_addrs;
	case ADDRS_TV:
		return tv_addrs;
	case ADDRS_TV_WITH_DEMOD:
		return tv_addrs + 4;
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(v4l2_i2c_tuner_addrs);

#endif /* defined(CONFIG_I2C) */

/* Clamp x to be between min and max, aligned to a multiple of 2^align.  min
 * and max don't have to be aligned, but there must be at least one valid
 * value.  E.g., min=17,max=31,align=4 is not allowed as there are no multiples
 * of 16 between 17 and 31.  */
static unsigned int clamp_align(unsigned int x, unsigned int min,
				unsigned int max, unsigned int align)
{
	/* Bits that must be zero to be aligned */
	unsigned int mask = ~((1 << align) - 1);

	/* Round to nearest aligned value */
	if (align)
		x = (x + (1 << (align - 1))) & mask;

	/* Clamp to aligned value of min and max */
	if (x < min)
		x = (min + ~mask) & mask;
	else if (x > max)
		x = max & mask;

	return x;
}

/* Bound an image to have a width between wmin and wmax, and height between
 * hmin and hmax, inclusive.  Additionally, the width will be a multiple of
 * 2^walign, the height will be a multiple of 2^halign, and the overall size
 * (width*height) will be a multiple of 2^salign.  The image may be shrunk
 * or enlarged to fit the alignment constraints.
 *
 * The width or height maximum must not be smaller than the corresponding
 * minimum.  The alignments must not be so high there are no possible image
 * sizes within the allowed bounds.  wmin and hmin must be at least 1
 * (don't use 0).  If you don't care about a certain alignment, specify 0,
 * as 2^0 is 1 and one byte alignment is equivalent to no alignment.  If
 * you only want to adjust downward, specify a maximum that's the same as
 * the initial value.
 */
void v4l_bound_align_image(u32 *w, unsigned int wmin, unsigned int wmax,
			   unsigned int walign,
			   u32 *h, unsigned int hmin, unsigned int hmax,
			   unsigned int halign, unsigned int salign)
{
	*w = clamp_align(*w, wmin, wmax, walign);
	*h = clamp_align(*h, hmin, hmax, halign);

	/* Usually we don't need to align the size and are done now. */
	if (!salign)
		return;

	/* How much alignment do we have? */
	walign = __ffs(*w);
	halign = __ffs(*h);
	/* Enough to satisfy the image alignment? */
	if (walign + halign < salign) {
		/* Max walign where there is still a valid width */
		unsigned int wmaxa = __fls(wmax ^ (wmin - 1));
		/* Max halign where there is still a valid height */
		unsigned int hmaxa = __fls(hmax ^ (hmin - 1));

		/* up the smaller alignment until we have enough */
		do {
			if (halign >= hmaxa ||
			    (walign <= halign && walign < wmaxa)) {
				*w = clamp_align(*w, wmin, wmax, walign + 1);
				walign = __ffs(*w);
			} else {
				*h = clamp_align(*h, hmin, hmax, halign + 1);
				halign = __ffs(*h);
			}
		} while (halign + walign < salign);
	}
}
EXPORT_SYMBOL_GPL(v4l_bound_align_image);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   /*
 * Copyright (c) 2000-2001 Adaptec Inc.
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
 *
 * String handling code courtesy of Gerard Roudier's <groudier@club-internet.fr>
 * sym driver.
 *
 * $Id: //depot/aic7xxx/linux/drivers/scsi/aic7xxx/aic79xx_proc.c#19 $
 */
#include "aic79xx_osm.h"
#include "aic79xx_inline.h"

static void	copy_mem_info(struct info_str *info, char *data, int len);
static int	copy_info(struct info_str *info, char *fmt, ...);
static void	ahd_dump_target_state(struct ahd_softc *ahd,
				      struct info_str *info,
				      u_int our_id, char channel,
				      u_int target_id);
static void	ahd_dump_device_state(struct info_str *info,
				      struct scsi_device *sdev);
static int	ahd_proc_write_seeprom(struct ahd_softc *ahd,
				       char *buffer, int length);

/*
 * Table of syncrates that don't follow the "divisible by 4"
 * rule. This table will be expanded in future SCSI specs.
 */
static const struct {
	u_int period_factor;
	u_int period;	/* in 100ths of ns */
} scsi_syncrates[] = {
	{ 0x08, 625 },	/* FAST-160 */
	{ 0x09, 1250 },	/* FAST-80 */
	{ 0x0a, 2500 },	/* FAST-40 40MHz */
	{ 0x0b, 3030 },	/* FAST-40 33MHz */
	{ 0x0c, 5000 }	/* FAST-20 */
};

/*
 * Return the frequency in kHz corresponding to the given
 * sync period factor.
 */
static u_int
ahd_calc_syncsrate(u_int period_factor)
{
	int i;

	/* See if the period is in the "exception" table */
	for (i = 0; i < ARRAY_SIZE(scsi_syncrates); i++) {

		if (period_factor == scsi_syncrates[i].period_factor) {
			/* Period in kHz */
			return (100000000 / scsi_syncrates[i].period);
		}
	}

	/*
	 * Wasn't in the table, so use the standard
	 * 4 times conversion.
	 */
	return (10000000 / (period_factor * 4 * 10));
}


static void
copy_mem_info(struct info_str *info, char *data, int len)
{
	if (info->pos + len > info->offset + info->length)
		len = info->offset + info->length - info->pos;

	if (info->pos + len < info->offset) {
		info->pos += len;
		return;
	}

	if (info->pos < info->offset) {
		off_t partial;

		partial = info->offset - info->pos;
		data += partial;
		info->pos += partial;
		len  -= partial;
	}

	if (len > 0) {
		memcpy(info->buffer, data, len);
		info->pos += len;
		info->buffer += len;
	}
}

static int
copy_info(struct info_str *info, char *fmt, ...)
{
	va_list args;
	char buf[256];
	int len;

	va_start(args, fmt);
	len = vsprintf(buf, fmt, args);
	va_end(args);

	copy_mem_info(info, buf, len);
	return (len);
}

static void
ahd_format_transinfo(struct info_str *info, struct ahd_transinfo *tinfo)
{
	u_int speed;
	u_int freq;
	u_int mb;

	if (tinfo->period == AHD_PERIOD_UNKNOWN) {
		copy_info(info, "Renegotiation Pending\n");
		return;
	}
        speed = 3300;
        freq = 0;
	if (tinfo->offset != 0) {
		freq = ahd_calc_syncsrate(tinfo->period);
		speed = freq;
	}
	speed *= (0x01 << tinfo->width);
        mb = speed / 1000;
        if (mb > 0)
		copy_info(info, "%d.%03dMB/s transfers", mb, speed % 1000);
        else
		copy_info(info, "%dKB/s transfers", speed);

	if (freq != 0) {
		int	printed_options;

		printed_options = 0;
		copy_info(info, " (%d.%03dMHz", freq / 1000, freq % 1000);
		if ((tinfo->ppr_options & MSG_EXT_PPR_RD_STRM) != 0) {
			copy_info(info, " RDSTRM");
			printed_options++;
		}
		if ((tinfo->ppr_options & MSG_EXT_PPR_DT_REQ) != 0) {
			copy_info(info, "%s", printed_options ? "|DT" : " DT");
			printed_options++;
		}
		if ((tinfo->ppr_options & MSG_EXT_PPR_IU_REQ) != 0) {
			copy_info(info, "%s", printed_options ? "|IU" : " IU");
			printed_options++;
		}
		if ((tinfo->ppr_options & MSG_EXT_PPR_RTI) != 0) {
			copy_info(info, "%s",
				  printed_options ? "|RTI" : " RTI");
			printed_options++;
		}
		if ((tinfo->ppr_options & MSG_EXT_PPR_QAS_REQ) != 0) {
			copy_info(info, "%s",
				  printed_options ? "|QAS" : " QAS");
			printed_options++;
		}
	}

	if (tinfo->width > 0) {
		if (freq != 0) {
			copy_info(info, ", ");
		} else {
			copy_info(info, " (");
		}
		copy_info(info, "%dbit)", 8 * (0x01 << tinfo->width));
	} else if (freq != 0) {
		copy_info(info, ")");
	}
	copy_info(info, "\n");
}

static void
ahd_dump_target_state(struct ahd_softc *ahd, struct info_str *info,
		      u_int our_id, char channel, u_int target_id)
{
	struct  scsi_target *starget;
	struct	ahd_initiator_tinfo *tinfo;
	struct	ahd_tmode_tstate *tstate;
	int	lun;

	tinfo = ahd_fetch_transinfo(ahd, channel, our_id,
				    target_id, &tstate);
	copy_info(info, "Target %d Negotiation Settings\n", target_id);
	copy_info(info, "\tUser: ");
	ahd_format_transinfo(info, &tinfo->user);
	starget = ahd->platform_data->starget[target_id];
	if (starget == NULL)
		return;

	copy_info(info, "\tGoal: ");
	ahd_format_transinfo(info, &tinfo->goal);
	copy_info(info, "\tCurr: ");
	ahd_format_transinfo(info, &tinfo->curr);

	for (lun = 0; lun < AHD_NUM_LUNS; lun++) {
		struct scsi_device *dev;

		dev = scsi_device_lookup_by_target(starget, lun);

		if (dev == NULL)
			continue;

		ahd_dump_device_state(info, dev);
	}
}

static void
ahd_dump_device_state(struct info_str *info, struct scsi_device *sdev)
{
	struct ahd_linux_device *dev = scsi_transport_device_data(sdev);

	copy_info(info, "\tChannel %c Target %d Lun %d Settings\n",
		  sdev->sdev_target->channel + 'A',
		  sdev->sdev_target->id, sdev->lun);

	copy_info(info, "\t\tCommands Queued %ld\n", dev->commands_issued);
	copy_info(info, "\t\tCommands Active %d\n", dev->active);
	copy_info(info, "\t\tCommand Openings %d\n", dev->openings);
	copy_info(info, "\t\tMax Tagged Openings %d\n", dev->maxtags);
	copy_info(info, "\t\tDevice Queue Frozen Count %d\n", dev->qfrozen);
}

static int
ahd_proc_write_seeprom(struct ahd_softc *ahd, char *buffer, int length)
{
	ahd_mode_state saved_modes;
	int have_seeprom;
	u_long s;
	int paused;
	int written;

	/* Default to failure. */
	written = -EINVAL;
	ahd_lock(ahd, &s);
	paused = ahd_is_paused(ahd);
	if (!paused)
		ahd_pause(ahd);

	saved_modes = ahd_save_modes(ahd);
	ahd_set_modes(ahd, AHD_MODE_SCSI, AHD_MODE_SCSI);
	if (length != sizeof(struct seeprom_config)) {
		printf("ahd_proc_write_seeprom: incorrect buffer size\n");
		goto done;
	}

	have_seeprom = ahd_verify_cksum((struct seeprom_config*)buffer);
	if (have_seeprom == 0) {
		printf("ahd_proc_write_seeprom: cksum verification failed\n");
		goto done;
	}

	have_seeprom = ahd_acquire_seeprom(ahd);
	if (!have_seeprom) {
		printf("ahd_proc_write_seeprom: No Serial EEPROM\n");
		sp *hdsp)
{
	if (hdsp->control_register & HDSP_ClockModeMaster) {
		switch (hdsp->system_sample_rate) {
		case 32000:
			return 1;
		case 44100:
			return 2;
		case 48000:
			return 3;
		case 64000:
			return 4;
		case 88200:
			return 5;
		case 96000:
			return 6;
		case 128000:
			return 7;
		case 176400:
			return 8;
		case 192000:
			return 9;
		default:
			return 3;
		}
	} else {
		return 0;
	}
}

static int hdsp_set_clock_source(struct hdsp *hdsp, int mode)
{
	int rate;
	switch (mode) {
	case HDSP_CLOCK_SOURCE_AUTOSYNC:
		if (hdsp_external_sample_rate(hdsp) != 0) {
		    if (!hdsp_set_rate(hdsp, hdsp_external_sample_rate(hdsp), 1)) {
			hdsp->control_register &= ~HDSP_ClockModeMaster;
			hdsp_write(hdsp, HDSP_controlRegister, hdsp->control_register);
			return 0;
		    }
		}
		return -1;
	case HDSP_CLOCK_SOURCE_INTERNAL_32KHZ:
		rate = 32000;
		break;
	case HDSP_CLOCK_SOURCE_INTERNAL_44_1KHZ:
		rate = 44100;
		break;
	case HDSP_CLOCK_SOURCE_INTERNAL_48KHZ:
		rate = 48000;
		break;
	case HDSP_CLOCK_SOURCE_INTERNAL_64KHZ:
		rate = 64000;
		break;
	case HDSP_CLOCK_SOURCE_INTERNAL_88_2KHZ:
		rate = 88200;
		break;
	case HDSP_CLOCK_SOURCE_INTERNAL_96KHZ:
		rate = 96000;
		break;
	case HDSP_CLOCK_SOURCE_INTERNAL_128KHZ:
		rate = 128000;
		break;
	case HDSP_CLOCK_SOURCE_INTERNAL_176_4KHZ:
		rate = 176400;
		break;
	case HDSP_CLOCK_SOURCE_INTERNAL_192KHZ:
		rate = 192000;
		break;
	default:
		rate = 48000;
	}
	hdsp->control_register |= HDSP_ClockModeMaster;
	hdsp_write(hdsp, HDSP_controlRegister, hdsp->control_register);
	hdsp_set_rate(hdsp, rate, 1);
	return 0;
}

static int snd_hdsp_info_clock_source(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	static char *texts[] = {"AutoSync", "Internal 32.0 kHz", "Internal 44.1 kHz", "Internal 48.0 kHz", "Internal 64.0 kHz", "Internal 88.2 kHz", "Internal 96.0 kHz", "Internal 128 kHz", "Internal 176.4 kHz", "Internal 192.0 KHz" };
	struct hdsp *hdsp = snd_kcontrol_chip(kcontrol);

	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = 1;
	if (hdsp->io_type == H9632)
	    uinfo->value.enumerated.items = 10;
	else
	    uinfo->value.enumerated.items = 7;
	if (uinfo->value.enumerated.item >= uinfo->value.enumerated.items)
		uinfo->value.enumerated.item = uinfo->value.enumerated.items - 1;
	strcpy(uinfo->value.enumerated.name, texts[uinfo->value.enumerated.item]);
	return 0;
}

static int snd_hdsp_get_clock_source(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct hdsp *hdsp = snd_kcontrol_chip(kcontrol);

	ucontrol->value.enumerated.item[0] = hdsp_clock_source(hdsp);
	return 0;
}

static int snd_hdsp_put_clock_source(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct hdsp *hdsp = snd_kcontrol_chip(kcontrol);
	int change;
	int val;

	if (!snd_hdsp_use_is_exclusive(hdsp))
		return -EBUSY;
	val = ucontrol->value.enumerated.item[0];
	if (val < 0) val = 0;
	if (hdsp->io_type == H9632) {
		if (val > 9)
			val = 9;
	} else {
		if (val > 6)
			val = 6;
	}
	spin_lock_irq(&hdsp->lock);
	if (val != hdsp_clock_source(hdsp))
		change = (hdsp_set_clock_source(hdsp, val) == 0) ? 1 : 0;
	else
		change = 0;
	spin_unlock_irq(&hdsp->lock);
	return change;
}

#define snd_hdsp_info_clock_source_lock		snd_ctl_boolean_mono_info

static int snd_hdsp_get_clock_source_lock(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct hdsp *hdsp = snd_kcontrol_chip(kcontrol);

	ucontrol->value.integer.value[0] = hdsp->clock_source_locked;
	return 0;
}

static int snd_hdsp_put_clock_source_lock(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct hdsp *hdsp = snd_kcontrol_chip(kcontrol);
	int change;

	change = (int)ucontrol->value.integer.value[0] != hdsp->clock_source_locked;
	if (change)
		hdsp->clock_source_locked = !!ucontrol->value.integer.value[0];
	return change;
}

#define HDSP_DA_GAIN(xname, xindex) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .name = xname, \
  .index = xindex, \
  .info = snd_hdsp_info_da_gain, \
  .get = snd_hdsp_get_da_gain, \
  .put = snd_hdsp_put_da_gain \
}

/*
 * R8A66597 HCD (Host Controller Driver)
 *
 * Copyright (C) 2006-2007 Renesas Solutions Corp.
 * Portions Copyright (C) 2004 Psion Teklogix (for NetBook PRO)
 * Portions Copyright (C) 2004-2005 David Brownell
 * Portions Copyright (C) 1999 Roman Weissgaerber
 *
 * Author : Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/usb.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/irq.h>

#include "../core/hcd.h"
#include "r8a66597.h"

MODULE_DESCRIPTION("R8A66597 USB Host Controller Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yoshihiro Shimoda");
MODULE_ALIAS("platform:r8a66597_hcd");

#define DRIVER_VERSION	"2009-05-26"

static const char hcd_name[] = "r8a66597_hcd";

static void packet_write(struct r8a66597 *r8a66597, u16 pipenum);
static int r8a66597_get_frame(struct usb_hcd *hcd);

/* this function must be called with interrupt disabled */
static void enable_pipe_irq(struct r8a66597 *r8a66597, u16 pipenum,
			    unsigned long reg)
{
	u16 tmp;

	tmp = r8a66597_read(r8a66597, INTENB0);
	r8a66597_bclr(r8a66597, BEMPE | NRDYE | BRDYE, INTENB0);
	r8a66597_bset(r8a66597, 1 << pipenum, reg);
	r8a66597_write(r8a66597, tmp, INTENB0);
}

/* this function must be called with interrupt disabled */
static void disable_pipe_irq(struct r8a66597 *r8a66597, u16 pipenum,
			     unsigned long reg)
{
	u16 tmp;

	tmp = r8a66597_read(r8a66597, INTENB0);
	r8a66597_bclr(r8a66597, BEMPE | NRDYE | BRDYE, INTENB0);
	r8a66597_bclr(r8a66597, 1 << pipenum, reg);
	r8a66597_write(r8a66597, tmp, INTENB0);
}

static void set_devadd_reg(struct r8a66597 *r8a66597, u8 r8a66597_address,
			   u16 usbspd, u8 upphub, u8 hubport, int port)
{
	u16 val;
	unsigned long devadd_reg = get_devadd_addr(r8a66597_address);

	val = (upphub << 11) | (hubport << 8) | (usbspd << 6) | (port & 0x0001);
	r8a66597_write(r8a66597, val, devadd_reg);
}

static int r8a66597_clock_enable(struct r8a66597 *r8a66597)
{
	u16 tmp;
	int i = 0;

#if defined(CONFIG_SUPERH_ON_CHIP_R8A66597)
#if defined(CONFIG_HAVE_CLK)
	clk_enable(r8a66597->clk);
#endif
	do {
		r8a66597_write(r8a66597, SCKE, SYSCFG0);
		tmp = r8a66597_read(r8a66597, SYSCFG0);
		if (i++ > 1000) {
			printk(KERN_ERR "r8a66597: register access fail.\n");
			return -ENXIO;
		}
	} while ((tmp & SCKE) != SCKE);
	r8a66597_write(r8a66597, 0x04, 0x02);
#else
	do {
		r8a66597_write(r8a66597, USBE, SYSCFG0);
		tmp = r8a66597_read(r8a66597, SYSCFG0);
		if (i++ > 1000) {
			printk(KERN_ERR "r8a66597: register access fail.\n");
			return -ENXIO;
		}
	} while ((tmp & USBE) != USBE);
	r8a66597_bclr(r8a66597, USBE, SYSCFG0);
	r8a66597_mdfy(r8a66597, get_xtal_from_pdata(r8a66597->pdata), XTAL,
			SYSCFG0);

	i = 0;
	r8a66597_bset(r8a66597, XCKE, SYSCFG0);
	do {
		msleep(1);
		tmp = r8a66597_read(r8a66597, SYSCFG0);
		if (i++ > 500) {
			printk(KERN_ERR "r8a66597: register access fail.\n");
			return -ENXIO;
		}
	} while ((tmp & SCKE) != SCKE);
#endif	/* #if defined(CONFIG_SUPERH_ON_CHIP_R8A66597) */

	return 0;
}

static void r8a66597_clock_disable(struct r8a66597 *r8a66597)
{
	r8a66597_bclr(r8a66597, SCKE, SYSCFG0);
	udelay(1);
#if defined(CONFIG_SUPERH_ON_CHIP_R8A66597)
#if defined(CONFIG_HAVE_CLK)
	clk_disable(r8a66597->clk);
#endif
#else
	r8a66597_bclr(r8a66597, PLLC, SYSCFG0);
	r8a66597_bclr(r8a66597, XCKE, SYSCFG0);
	r8a66597_bclr(r8a66597, USBE, SYSCFG0);
#endif
}

static void r8a66597_enable_port(struct r8a66597 *r8a66597, int port)
{
	u16 val;

	val = port ? DRPD : DCFM | DRPD;
	r8a66597_bset(r8a66597, val, get_syscfg_reg(port));
	r8a66597_bset(r8a66597, HSE, get_syscfg_reg(port));

	r8a66597_write(r8a66597, BURST | CPU_ADR_RD_WR, get_dmacfg_reg(port));
	r8a66597_bclr(r8a66597, DTCHE, get_intenb_reg(port));
	r8a66597_bset(r8a66597, ATTCHE, get_intenb_reg(port));
}

static void r8a66597_disable_port(struct r8a66597 *r8a66597, int port)
{
	u16 val, tmp;

	r8a66597_write(r8a66597, 0, get_intenb_reg(port));
	r8a66597_write(r8a66597, 0, get_intsts_reg(port));

	r8a66597_port_power(r8a66597, port, 0);

	do {
		tmp = r8a66597_read(r8a66597, SOFCFG) & EDGESTS;
		udelay(640);
	} while (tmp == EDGESTS);

	val = port ? DRPD : DCFM | DRPD;
	r8a66597_bclr(r8a66597, val, get_syscfg_reg(port));
	r8a66597_bclr(r8a66597, HSE, get_syscfg_reg(port));
}

static int enable_controller(struct r8a66597 *r8a66597)
{
	int ret, port;
	u16 vif = r8a66597->pdata->vif ? LDRV : 0;
	u16 irq_sense = r8a66597->irq_sense_low ? INTL : 0;
	u16 endian = r8a66597->pdata->endian ? BIGEND : 0;

	ret = r8a66597_clock_enable(r8a66597);
	if (ret < 0)
		return ret;

	r8a66597_bset(r8a66597, vif & LDRV, PINCFG);
	r8a66597_bset(r8a66597, USBE, SYSCFG0);

	r8a66597_bset(r8a66597, BEMPE | NRDYE | BRDYE, INTENB0);
	r8a66597_bset(r8a66597, irq_sense & INTL, SOFCFG);
	r8a66597_bset(r8a66597, BRDY0, BRDYENB);
	r8a66597_bset(r8a66597, BEMP0, BEMPENB);

	r8a66597_bset(r8a66597, endian & BIGEND, CFIFOSEL);
	r8a66597_bset(r8a66597, endian & BIGEND, D0FIFOSEL);
	r8a66597_bset(r8a66597, endian & BIGEND, D1FIFOSEL);
	r8a66597_bset(r8a66597, TRNENSEL, SOFCFG);

	r8a66597_bset(r8a66597, SIGNE | SACKE, INTENB1);

	for (port = 0; port < R8A66597_MAX_ROOT_HUB; port++)
		r8a66597_enable_port(r8a66597, port);

	return 0;
}

static void disable_controller(struct r8a66597 *r8a66597)
{
	int port;

	r8a66597_write(r8a66597, 0, INTENB0);
	r8a66597_write(r8a66597, 0, INTSTS0);

	for (port = 0; port < R8A66597_MAX_ROOT_HUB; port++)
		r8a66597_disable_port(r8a66597, port);

	r8a66597_clock_disable(r8a66597);
}

static int get_parent_r8a66597_address(struct r8a66597 *r8a66597,
				       struct usb_device *udev)
{
	struct r8a66597_device *dev;

	if (udev->parent && udev->parent->devnum != 1)
		udev = udev->parent;

	dev = dev_get_drvdata(&udev->dev);
	if (dev)
		return dev->address;
	else
		return 0;
}

static int is_child_device(char *devpath)
{
	return (devpath[2] ? 1 : 0);
}

static int is_hub_limit(char *devpath)
{
	return ((strlen(devpath) >= 4) ? 1 : 0);
}

static void get_port_number(char *devpath, u16 *root_port, u16 *hub_port)
{
	if (root_port) {
		*root_port = (devpath[0] & 0x0F) - 1;
		if (*root_port >= R8A66597_MAX_ROOT_HUB)
			printk(KERN_ERR "r8a66597: Illegal root port number.\n");
	}
	if (hub_port)
		*hub_port = devpath[2] & 0x0F;
}

static u16 get_r8a66597_usb_speed(enum usb_device_speed speed)
{
	u16 usbspd = 0;

	switch (speed) {
	case USB_SPEED_LOW:
		usbspd = LSMODE;
		break;
	case USB_SPEED_FULL:
		usbspd = FSMODE;
		break;
	case USB_SPEED_HIGH:
		usbspd = HSMODE;
		break;
	default:
		printk(KERN_ERR "r8a66597: unknown speed\n");
		break;
	}

	return usbspd;
}

static void set_child_connect_map(struct r8a66597 *r8a66597, int address)
{
	int idx;

	idx = address / 32;
	r8a66597->child_connect_map[idx] |= 1 << (address % 32);
}

static void put_child_connect_map(struct r8a66597 *r8a66597, int address)
{
	int idx;

	idx = address / 32;
	r8a66597->child_connect_map[idx] &= ~(1 << (address % 32));
}

static void set_pipe_reg_addr(struct r8a66597_pipe *pipe, u8 dma_ch)
{
	u16 pipenum = pipe->info.pipenum;
	const unsigned long fifoaddr[] = {D0FIFO, D1FIFO, CFIFO};
	const unsigned long fifosel[] = {D0FIFOSEL, D1FIFOSEL, CFIFOSEL};
	const unsigned long fifoctr[] = {D0FIFOCTR, D1FIFOCTR, CFIFOCTR};

	if (dma_ch > R8A66597_PIPE_NO_DMA)	/* dma fifo not use? */
		dma_ch = R8A66597_PIPE_NO_DMA;

	pipe->fifoaddr = fifoaddr[dma_ch];
	pipe->fifosel = fifosel[dma_ch];
	pipe->fifoctr = fifoctr[dma_ch];

	if (pipenum == 0)
		pipe->pipectr = DCPCTR;
	else
		pipe->pipectr = get_pipectr_addr(pipenum);

	if (check_bulk_or_isoc(pipenum)) {
		pipe->pipetre = get_pipetre_addr(pipenum);
		pipe->pipetrn = get_pipetrn_addr(pipenum);
	} else {
		pipe->pipetre = 0;
		pipe->pipetrn = 0;
	}
}

static struct r8a66597_device *
get_urb_to_r8a66597_dev(struct r8a66597 *r8a66597, struct urb *urb)
{
	if (usb_pipedevice(urb->pipe) == 0)
		return &r8a66597->device0;

	return dev_get_drvdata(&urb->dev->dev);
}

static int make_r8a66597_device(struct r8a66597 *r8a66597,
				struct urb *urb, u8 addr)
{
	struct r8a66597_device *dev;
	int usb_address = urb->setup_packet[2];	/* urb->pipe is address 0 */

	dev = kzalloc(sizeof(struct r8a66597_device), GFP_ATOMIC);
	if (dev == NULL)
		return -ENOMEM;

	dev_set_drvdata(&urb->dev->dev, dev);
	dev->udev = urb->dev;
	dev->address = addr;
	dev->usb_address = usb_address;
	dev->state = USB_STATE_ADDRESS;
	dev->ep_in_toggle = 0;
	dev->ep_out_toggle = 0;
	INIT_LIST_HEAD(&dev->device_list);
	list_add_tail(&dev->device_list, &r8a66597->child_device);

	get_port_number(urb->dev->devpath, &dev->root_port, &dev->hub_port);
	if (!is_child_device(urb->dev->devpath))
		r8a66597->root_hub[dev->root_port].dev = dev;

	set_devadd_reg(r8a66597, dev->address,
		       get_r8a66597_usb_speed(urb->dev->speed),
		       get_parent_r8a66597_address(r8a66597, urb->dev),
		       dev->hub_port, dev->root_port);

	return 0;
}

/* this function must be called with interrupt disabled */
static u8 alloc_usb_address(struct r8a66597 *r8a66597, struct urb *urb)
{
	u8 addr;	/* R8A66597's address */
	struct r8a66597_device *dev;

	if (is_hub_limit(urb->dev->devpath)) {
		dev_err(&urb->dev->dev, "External hub limit reached.\n");
		return 0;
	}

	dev = get_urb_to_r8a66597_dev(r8a66597, urb);
	if (dev && dev->state >= USB_STATE_ADDRESS)
		return dev->address;

	for (addr = 1; addr <= R8A66597_MAX_DEVICE; addr++) {
		if (r8a66597->address_map & (1 << addr))
			continue;

		dbg("alloc_address: r8a66597_addr=%d", addr);
		r8a66597->address_map |= 1 << addr;

		if (make_r8a66597_device(r8a66597, urb, addr) < 0)
			return 0;

		return addr;
	}

	dev_err(&urb->dev->dev,
		"cannot communicate with a USB device more than 10.(%x)\n",
		r8a66597->address_map);

	return 0;
}

/* this function must be called with interrupt disabled */
static void free_usb_address(struct r8a66597 *r8a66597,
			     struct r8a66597_device *dev)
{
	int port;

	if (!dev)
		return;

	dbg("free_addr: addr=%d", dev->address);

	dev->state = USB_STATE_DEFAULT;
	r8a66597->address_map &= ~(1 << dev->address);
	dev->address = 0;
	dev_set_drvdata(&dev->udev->dev, NULL);
	list_del(&dev->device_list);
	kfree(dev);

	for (port = 0; port < R8A66597_MAX_ROOT_HUB; port++) {
		if (r8a66597->root_hub[port].dev == dev) {
			r8a66597->root_hub[port].dev = NULL;
			break;
		}
	}
}

static void r8a66597_reg_wait(struct r8a66597 *r8a66597, unsigned long reg,
			      u16 mask, u16 loop)
{
	u16 tmp;
	int i = 0;

	do {
		tmp = r8a66597_read(r8a66597, reg);
		if (i++ > 1000000) {
			printk(KERN_ERR "r8a66597: register%lx, loop %x "
			       "is timeout\n", reg, loop);
			break;
		}
		ndelay(1);
	} while ((tmp & mask) != loop);
}

/* this function must be called with interrupt disabled */
static void pipe_start(struct r8a66597 *r8a66597, struct r8a66597_pipe *pipe)
{
	u16 tmp;

	tmp = r8a66597_read(r8a66597, pipe->pipectr) & PID;
	if ((pipe->info.pipenum != 0) & ((tmp & PID_STALL) != 0)) /* stall? */
		r8a66597_mdfy(r8a66597, PID_NAK, PID, pipe->pipectr);
	r8a66597_mdfy(r8a66597, PID_BUF, PID, pipe->pipectr);
}

/* this function must be called with interrupt disabled */
static void pipe_stop(struct r8a66597 *r8a66597, struct r8a66597_pipe *pipe)
{
	u16 tmp;

	tmp = r8a66597_read(r8a66597, pipe->pipectr) & PID;
	if ((tmp & PID_STALL11) != PID_STALL11)	/* force stall? */
		r8a66597_mdfy(r8a66597, PID_STALL, PID, pipe->pipectr);
	r8a66597_mdfy(r8a66597, PID_NAK, PID, pipe->pipectr);
	r8a66597_reg_wait(r8a66597, pipe->pipectr, PBUSY, 0);
}

/* this function must be called with interrupt disabled */
static void clear_all_buffer(struct r8a66597 *r8a66597,
			     struct r8a66597_pipe *pipe)
{
	u16 tmp;

	if (!pipe || pipe->info.pipenum == 0)
		return;

	pipe_stop(r8a66597, pipe);
	r8a66597_bset(r8a66597, ACLRM, pipe->pipectr);
	tmp = r8a66597_read(r8a66597, pipe->pipectr);
	tmp = r8a66597_read(r8a66597, pipe->pipectr);
	tmp = r8a66597_read(r8a66597, pipe->pipectr);
	r8a66597_bclr(r8a66597, ACLRM, pipe->pipectr);
}

/* this function must be called with interrupt disabled */
static void r8a66597_pipe_toggle(struct r8a66597 *r8a66597,
				 struct r8a66597_pipe *pipe, int toggle)
{
	if (toggle)
		r8a66597_bset(r8a66597, SQSET, pipe->pipectr);
	else
		r8a66597_bset(r8a66597, SQCLR, pipe->pipectr);
}

/* this function must be called with interrupt disabled */
static inline void cfifo_change(struct r8a66597 *r8a66597, u16 pipenum)
{
	r8a66597_mdfy(r8a66597, MBW | pipenum, MBW | CURPIPE, CFIFOSEL);
	r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, pipenum);
}

/* this function must be called with interrupt disabled */
static inline void fifo_change_from_pipe(struct r8a66597 *r8a66597,
					 struct r8a66597_pipe *pipe)
{
	cfifo_change(r8a66597, 0);
	r8a66597_mdfy(r8a66597, MBW | 0, MBW | CURPIPE, D0FIFOSEL);
	r8a66597_mdfy(r8a66597, MBW | 0, MBW | CURPIPE, D1FIFOSEL);

	r8a66597_mdfy(r8a66597, MBW | pipe->info.pipenum, MBW | CURPIPE,
		      pipe->fifosel);
	r8a66597_reg_wait(r8a66597, pipe->fifosel, CURPIPE, pipe->info.pipenum);
}

static u16 r8a66597_get_pipenum(struct urb *urb, struct usb_host_endpoint *hep)
{
	struct r8a66597_pipe *pipe = hep->hcpriv;

	if (usb_pipeendpoint(urb->pipe) == 0)
		return 0;
	else
		return pipe->info.pipenum;
}

static u16 get_urb_to_r8a66597_addr(struct r8a66597 *r8a66597, struct urb *urb)
{
	struct r8a66597_device *dev = get_urb_to_r8a66597_dev(r8a66597, urb);

	return (usb_pipedevice(urb->pipe) == 0) ? 0 : dev->address;
}

static unsigned short *get_toggle_pointer(struct r8a66597_device *dev,
					  int urb_pipe)
{
	if (!dev)
		return NULL;

	return usb_pipein(urb_pipe) ? &dev->ep_in_toggle : &dev->ep_out_toggle;
}

/* this function must be called with interrupt disabled */
static void pipe_toggle_set(struct r8a66597 *r8a66597,
			    struct r8a66597_pipe *pipe,
			    struct urb *urb, int set)
{
	struct r8a66597_device *dev = get_urb_to_r8a66597_dev(r8a66597, urb);
	unsigned char endpoint = usb_pipeendpoint(urb->pipe);
	unsigned short *toggle = get_toggle_pointer(dev, urb->pipe);

	if (!toggle)
		return;

	if (set)
		*toggle |= 1 << endpoint;
	else
		*toggle &= ~(1 << endpoint);
}

/* this function must be called with interrupt disabled */
static void pipe_toggle_save(struct r8a66597 *r8a66597,
			     struct r8a66597_pipe *pipe,
			     struct urb *urb)
{
	if (r8a66597_read(r8a66597, pipe->pipectr) & SQMON)
		pipe_toggle_set(r8a66597, pipe, urb, 1);
	else
		pipe_toggle_set(r8a66597, pipe, urb, 0);
}

/* this function must be called with interrupt disabled */
static void pipe_toggle_restore(struct r8a66597 *r8a66597,
				struct r8a66597_pipe *pipe,
				struct urb *urb)
{
	struct r8a66597_device *dev = get_urb_to_r8a66597_dev(r8a66597, urb);
	unsigned char endpoint = usb_pipeendpoint(urb->pipe);
	unsigned short *toggle = get_toggle_pointer(dev, urb->pipe);

	if (!toggle)
		return;

	r8a66597_pipe_toggle(r8a66597, pipe, *toggle & (1 << endpoint));
}

/* this function must be called with interrupt disabled */
static void pipe_buffer_setting(struct r8a66597 *r8a66597,
				struct r8a66597_pipe_info *info)
{
	u16 val = 0;

	if (info->pipenum == 0)
		return;

	r8a66597_bset(r8a66597, ACLRM, get_pipectr_addr(info->pipenum));
	r8a66597_bclr(r8a66597, ACLRM, get_pipectr_addr(info->pipenum));
	r8a66597_write(r8a66597, info->pipenum, PIPESEL);
	if (!info->dir_in)
		val |= R8A66597_DIR;
	if (info->type == R8A66597_BULK && info->dir_in)
		val |= R8A66597_DBLB | R8A66597_SHTNAK;
	val |= info->type | info->epnum;
	r8a66597_write(r8a66597, val, PIPECFG);

	r8a66597_write(r8a66597, (info->buf_bsize << 10) | (info->bufnum),
		       PIPEBUF);
	r8a66597_write(r8a66597, make_devsel(info->address) | info->maxpacket,
		       PIPEMAXP);
	r8a66597_write(r8a66597, info->interval, PIPEPERI);
}

/* this function must be called with interrupt disabled */
static void pipe_setting(struct r8a66597 *r8a66597, struct r8a66597_td *td)
{
	struct r8a66597_pipe_info *info;
	struct urb *urb = td->urb;

	if (td->pipenum > 0) {
		info = &td->pipe->info;
		cfifo_change(r8a66597, 0);
		pipe_buffer_setting(r8a66597, info);

		if (!usb_gettoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				   usb_pipeout(urb->pipe)) &&
		    !usb_pipecontrol(urb->pipe)) {
			r8a66597_pipe_toggle(r8a66597, td->pipe, 0);
			pipe_toggle_set(r8a66597, td->pipe, urb, 0);
			clear_all_buffer(r8a66597, td->pipe);
			usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				      usb_pipeout(urb->pipe), 1);
		}
		pipe_toggle_restore(r8a66597, td->pipe, urb);
	}
}

/* this function must be called with interrupt disabled */
static u16 get_empty_pipenum(struct r8a66597 *r8a66597,
			     struct usb_endpoint_descriptor *ep)
{
	u16 array[R8A66597_MAX_NUM_PIPE], i = 0, min;

	memset(array, 0, sizeof(array));
	switch (usb_endpoint_type(ep)) {
	case USB_ENDPOINT_XFER_BULK:
		if (usb_endpoint_dir_in(ep))
			array[i++] = 4;
		else {
			array[i++] = 3;
			array[i++] = 5;
		}
		break;
	case USB_ENDPOINT_XFER_INT:
		if (usb_endpoint_dir_in(ep)) {
			array[i++] = 6;
			array[i++] = 7;
			array[i++] = 8;
		} else
			array[i++] = 9;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		if (usb_endpoint_dir_in(ep))
			array[i++] = 2;
		else
			array[i++] = 1;
		break;
	default:
		printk(KERN_ERR "r8a66597: Illegal type\n");
		return 0;
	}

	i = 1;
	min = array[0];
	while (array[i] != 0) {
		if (r8a66597->pipe_cnt[min] > r8a66597->pipe_cnt[array[i]])
			min = array[i];
		i++;
	}

	return min;
}

static u16 get_r8a66597_type(__u8 type)
{
	u16 r8a66597_type;

	switch (type) {
	case USB_ENDPOINT_XFER_BULK:
		r8a66597_type = R8A66597_BULK;
		break;
	case USB_ENDPOINT_XFER_INT:
		r8a66597_type = R8A66597_INT;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		r8a66597_type = R8A66597_ISO;
		break;
	default:
		printk(KERN_ERR "r8a66597: Illegal type\n");
		r8a66597_type = 0x0000;
		break;
	}

	return r8a66597_type;
}

static u16 get_bufnum(u16 pipenum)
{
	u16 bufnum = 0;

	if (pipenum == 0)
		bufnum = 0;
	else if (check_bulk_or_isoc(pipenum))
		bufnum = 8 + (pipenum - 1) * R8A66597_BUF_BSIZE*2;
	else if (check_interrupt(pipenum))
		bufnum = 4 + (pipenum - 6);
	else
		printk(KERN_ERR "r8a66597: Illegal pipenum (%d)\n", pipenum);

	return bufnum;
}

static u16 get_buf_bsize(u16 pipenum)
{
	u16 buf_bsize = 0;

	if (pipenum == 0)
		buf_bsize = 3;
	else if (check_bulk_or_isoc(pipenum))
		buf_bsize = R8A66597_BUF_BSIZE - 1;
	else if (check_interrupt(pipenum))
		buf_bsize = 0;
	else
		printk(KERN_ERR "r8a66597: Illegal pipenum (%d)\n", pipenum);

	return buf_bsize;
}

/* this function must be called with interrupt disabled */
static void enable_r8a66597_pipe_dma(struct r8a66597 *r8a66597,
				     struct r8a66597_device *dev,
				     struct r8a66597_pipe *pipe,
				     struct urb *urb)
{
#if !defined(CONFIG_SUPERH_ON_CHIP_R8A66597)
	int i;
	struct r8a66597_pipe_info *info = &pipe->info;

	if ((pipe->info.pipenum != 0) && (info->type != R8A66597_INT)) {
		for (i = 0; i < R8A66597_MAX_DMA_CHANNEL; i++) {
			if ((r8a66597->dma_map & (1 << i)) != 0)
				continue;

			dev_info(&dev->udev->dev,
				 "address %d, EndpointAddress 0x%02x use "
				 "DMA FIFO\n", usb_pipedevice(urb->pipe),
				 info->dir_in ?
				 	USB_ENDPOINT_DIR_MASK + info->epnum
					: info->epnum);

			r8a66597->dma_map |= 1 << i;
			dev->dma_map |= 1 << i;
			set_pipe_reg_addr(pipe, i);

			cfifo_change(r8a66597, 0);
			r8a66597_mdfy(r8a66597, MBW | pipe->info.pipenum,
				      MBW | CURPIPE, pipe->fifosel);

			r8a66597_reg_wait(r8a66597, pipe->fifosel, CURPIPE,
					  pipe->info.pipenum);
			r8a66597_bset(r8a66597, BCLR, pipe->fifoctr);
			break;
		}
	}
#endif	/* #if defined(CONFIG_SUPERH_ON_CHIP_R8A66597) */
}

/* this function must be called with interrupt disabled */
static void enable_r8a66597_pipe(struct r8a66597 *r8a66597, struct urb *urb,
				 struct usb_host_endpoint *hep,
				 struct r8a66597_pipe_info *info)
{
	struct r8a66597_device *dev = get_urb_to_r8a66597_dev(r8a66597, urb);
	struct r8a66597_pipe *pipe = hep->hcpriv;

	dbg("enable_pipe:");

	pipe->info = *info;
	set_pipe_reg_addr(pipe, R8A66597_PIPE_NO_DMA);
	r8a66597->pipe_cnt[pipe->info.pipenum]++;
	dev->pipe_cnt[pipe->info.pipenum]++;

	enable_r8a66597_pipe_dma(r8a66597, dev, pipe, urb);
}

/* this function must be called with interrupt disabled */
static void force_dequeue(struct r8a66597 *r8a66597, u16 pipenum, u16 address)
{
	struct r8a66597_td *td, *next;
	struct urb *urb;
	struct list_head *list = &r8a66597->pipe_queue[pipenum];

	if (list_empty(list))
		return;

	list_for_each_entry_safe(td, next, list, queue) {
		if (!td)
			continue;
		if (td->address != address)
			continue;

		urb = td->urb;
		list_del(&td->queue);
		kfree(td);

		if (urb) {
			usb_hcd_unlink_urb_from_ep(r8a66597_to_hcd(r8a66597),
					urb);

			spin_unlock(&r8a66597->lock);
			usb_hcd_giveback_urb(r8a66597_to_hcd(r8a66597), urb,
					-ENODEV);
			spin_lock(&r8a66597->lock);
		}
		break;
	}
}

/* this function must be called with interrupt disabled */
static void disable_r8a66597_pipe_all(struct r8a66597 *r8a66597,
				      struct r8a66597_device *dev)
{
	int check_ep0 = 0;
	u16 pipenum;

	if (!dev)
		return;

	for (pipenum = 1; pipenum < R8A66597_MAX_NUM_PIPE; pipenum++) {
		if (!dev->pipe_cnt[pipenum])
			continue;

		if (!check_ep0) {
			check_ep0 = 1;
			force_dequeue(r8a66597, 0, dev->address);
		}

		r8a66597->pipe_cnt[pipenum] -= dev->pipe_cnt[pipenum];
		dev->pipe_cnt[pipenum] = 0;
		force_dequeue(r8a66597, pipenum, dev->address);
	}

	dbg("disable_pipe");

	r8a66597->dma_map &= ~(dev->dma_map);
	dev->dma_map = 0;
}

static u16 get_interval(struct urb *urb, __u8 interval)
{
	u16 time = 1;
	int i;

	if (urb->dev->speed == USB_SPEED_HIGH) {
		if (interval > IITV)
			time = IITV;
		else
			time = interval ? interval - 1 : 0;
	} else {
		if (interval > 128) {
			time = IITV;
		} else {
			/* calculate the nearest value for PIPEPERI */
			for (i = 0; i < 7; i++) {
				if ((1 << i) < interval &&
				    (1 << (i + 1) > interval))
					time = 1 << i;
			}
		}
	}

	return time;
}

static unsigned long get_timer_interval(struct urb *urb, __u8 interval)
{
	__u8 i;
	unsigned long time = 1;

	if (usb_pipeisoc(urb->pipe))
		return 0;

	if (get_r8a66597_usb_speed(urb->dev->speed) == HSMODE) {
		for (i = 0; i < (interval - 1); i++)
			time *= 2;
		time = time * 125 / 1000;	/* uSOF -> msec */
	} else {
		time = interval;
	}

	return time;
}

/* this function must be called with interrupt disabled */
static void init_pipe_info(struct r8a66597 *r8a66597, struct urb *urb,
			   struct usb_host_endpoint *hep,
			   struct usb_endpoint_descriptor *ep)
{
	struct r8a66597_pipe_info info;

	info.pipenum = get_empty_pipenum(r8a66597, ep);
	info.address = get_urb_to_r8a66597_addr(r8a66597, urb);
	info.epnum = usb_endpoint_num(ep);
	info.maxpacket = le16_to_cpu(ep->wMaxPacketSize);
	info.type = get_r8a66597_type(usb_endpoint_type(ep));
	info.bufnum = get_bufnum(info.pipenum);
	info.buf_bsize = get_buf_bsize(info.pipenum);
	if (info.type == R8A66597_BULK) {
		info.interval = 0;
		info.timer_interval = 0;
	} else {
		info.interval = get_interval(urb, ep->bInterval);
		info.timer_interval = get_timer_interval(urb, ep->bInterval);
	}
	if (usb_endpoint_dir_in(ep))
		info.dir_in = 1;
	else
		info.dir_in = 0;

	enable_r8a66597_pipe(r8a66597, urb, hep, &info);
}

static void init_pipe_config(struct r8a66597 *r8a66597, struct urb *urb)
{
	struct r8a66597_device *dev;

	dev = get_urb_to_r8a66597_dev(r8a66597, urb);
	dev->state = USB_STATE_CONFIGURED;
}

static void pipe_irq_enable(struct r8a66597 *r8a66597, struct urb *urb,
			    u16 pipenum)
{
	if (pipenum == 0 && usb_pipeout(urb->pipe))
		enable_irq_empty(r8a66597, pipenum);
	else
		enable_irq_ready(r8a66597, pipenum);

	if (!usb_pipeisoc(urb->pipe))
		enable_irq_nrdy(r8a66597, pipenum);
}

static void pipe_irq_disable(struct r8a66597 *r8a66597, u16 pipenum)
{
	disable_irq_ready(r8a66597, pipenum);
	disable_irq_nrdy(r8a66597, pipenum);
}

static void r8a66597_root_hub_start_polling(struct r8a66597 *r8a66597)
{
	mod_timer(&r8a66597->rh_timer,
			jiffies + msecs_to_jiffies(R8A66597_RH_POLL_TIME));
}

static void start_root_hub_sampling(struct r8a66597 *r8a66597, int port,
					int connect)
{
	struct r8a66597_root_hub *rh = &r8a66597->root_hub[port];

	rh->old_syssts = r8a66597_read(r8a66597, get_syssts_reg(port)) & LNST;
	rh->scount = R8A66597_MAX_SAMPLING;
	if (connect)
		rh->port |= 1 << USB_PORT_FEAT_CONNECTION;
	else
		rh->port &= ~(1 << USB_PORT_FEAT_CONNECTION);
	rh->port |= 1 << USB_PORT_FEAT_C_CONNECTION;

	r8a66597_root_hub_start_polling(r8a66597);
}

/* this function must be called with interrupt disabled */
static void r8a66597_check_syssts(struct r8a66597 *r8a66597, int port,
					u16 syssts)
{
	if (syssts == SE0) {
		r8a66597_write(r8a66597, ~ATTCH, get_intsts_reg(port));
		r8a66597_bset(r8a66597, ATTCHE, get_intenb_reg(port));
		return;
	}

	if (syssts == FS_JSTS)
		r8a66597_bset(r8a66597, HSE, get_syscfg_reg(port));
	else if (syssts == LS_JSTS)
		r8a66597_bclr(r8a66597, HSE, get_syscfg_reg(port));

	r8a66597_write(r8a66597, ~DTCH, get_intsts_reg(port));
	r8a66597_bset(r8a66597, DTCHE, get_intenb_reg(port));

	if (r8a66597->bus_suspended)
		usb_hcd_resume_root_hub(r8a66597_to_hcd(r8a66597));
}

/* this function must be called with interrupt disabled */
static void r8a66597_usb_connect(struct r8a66597 *r8a66597, int port)
{
	u16 speed = get_rh_usb_speed(r8a66597, port);
	struct r8a66597_root_hub *rh = &r8a66597->root_hub[port];

	if (speed == HSMODE)
		rh->port |= (1 << USB_PORT_FEAT_HIGHSPEED);
	else if (speed == LSMODE)
		rh->port |= (1 << USB_PORT_FEAT_LOWSPEED);

	rh->port &= ~(1 << USB_PORT_FEAT_RESET);
	rh->port |= 1 << USB_PORT_FEAT_ENABLE;
}

/* this function must be called with interrupt disabled */
static void r8a66597_usb_disconnect(struct r8a66597 *r8a66597, int port)
{
	struct r8a66597_device *dev = r8a66597->root_hub[port].dev;

	disable_r8a66597_pipe_all(r8a66597, dev);
	free_usb_address(r8a66597, dev);

	start_root_hub_sampling(r8a66597, port, 0);
}

/* this function must be called with interrupt disabled */
static void prepare_setup_packet(struct r8a66597 *r8a66597,
				 struct r8a66597_td *td)
{
	int i;
	__le16 *p = (__le16 *)td->urb->setup_packet;
	unsigned long setup_addr = USBREQ;

	r8a66597_write(r8a66597, make_devsel(td->address) | td->maxpacket,
		       DCPMAXP);
	r8a66597_write(r8a66597, ~(SIGN | SACK), INTSTS1);

	for (i = 0; i < 4; i++) {
		r8a66597_write(r8a66597, le16_to_cpu(p[i]), setup_addr);
		setup_addr += 2;
	}
	r8a66597_write(r8a66597, SUREQ, DCPCTR);
}

/* this function must be called with interrupt disabled */
static void prepare_packet_read(struct r8a66597 *r8a66597,
				struct r8a66597_td *td)
{
	struct urb *urb = td->urb;

	if (usb_pipecontrol(urb->pipe)) {
		r8a66597_bclr(r8a66597, R8A66597_DIR, DCPCFG);
		r8a66597_mdfy(r8a66597, 0, ISEL | CURPIPE, CFIFOSEL);
		r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, 0);
		if (urb->actual_length == 0) {
			r8a66597_pipe_toggle(r8a66597, td->pipe, 1);
			r8a66597_write(r8a66597, BCLR, CFIFOCTR);
		}
		pipe_irq_disable(r8a66597, td->pipenum);
		pipe_start(r8a66597, td->pipe);
		pipe_irq_enable(r8a66597, urb, td->pipenum);
	} else {
		if (urb->actual_length == 0) {
			pipe_irq_disable(r8a66597, td->pipenum);
			pipe_setting(r8a66597, td);
			pipe_stop(r8a66597, td->pipe);
			r8a66597_write(r8a66597, ~(1 << td->pipenum), BRDYSTS);

			if (td->pipe->pipetre) {
				r8a66597_write(r8a66597, TRCLR,
						td->pipe->pipetre);
				r8a66597_write(r8a66597,
						DIV_ROUND_UP
						  (urb->transfer_buffer_length,
						   td->maxpacket),
						td->pipe->pipetrn);
				r8a66597_bset(r8a66597, TRENB,
						td->pipe->pipetre);
			}

			pipe_start(r8a66597, td->pipe);
			pipe_irq_enable(r8a66597, urb, td->pipenum);
		}
	}
}

/* this function must be called with interrupt disabled */
static void prepare_packet_write(struct r8a66597 *r8a66597,
				 struct r8a66597_td *td)
{
	u16 tmp;
	struct urb *urb = td->urb;

	if (usb_pipecontrol(urb->pipe)) {
		pipe_stop(r8a66597, td->pipe);
		r8a66597_bset(r8a66597, R8A66597_DIR, DCPCFG);
		r8a66597_mdfy(r8a66597, ISEL, ISEL | CURPIPE, CFIFOSEL);
		r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, 0);
		if (urb->actual_length == 0) {
			r8a66597_pipe_toggle(r8a66597, td->pipe, 1);
			r8a66597_write(r8a66597, BCLR, CFIFOCTR);
		}
	} else {
		if (urb->actual_length == 0)
			pipe_setting(r8a66597, td);
		if (td->pipe->pipetre)
			r8a66597_bclr(r8a66597, TRENB, td->pipe->pipetre);
	}
	r8a66597_write(r8a66597, ~(1 << td->pipenum), BRDYSTS);

	fifo_change_from_pipe(r8a66597, td->pipe);
	tmp = r8a66597_read(r8a66597, td->pipe->fifoctr);
	if (unlikely((tmp & FRDY) == 0))
		pipe_irq_enable(r8a66597, urb, td->pipenum);
	else
		packet_write(r8a66597, td->pipenum);
	pipe_start(r8a66597, td->pipe);
}

/* this function must be called with interrupt disabled */
static void prepare_status_packet(struct r8a66597 *r8a66597,
				  struct r8a66597_td *td)
{
	struct urb *urb = td->urb;

	r8a66597_pipe_toggle(r8a66597, td->pipe, 1);
	pipe_stop(r8a66597, td->pipe);

	if (urb->setup_packet[0] & USB_ENDPOINT_DIR_MASK) {
		r8a66597_bset(r8a66597, R8A66597_DIR, DCPCFG);
		r8a66597_mdfy(r8a66597, ISEL, ISEL | CURPIPE, CFIFOSEL);
		r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, 0);
		r8a66597_write(r8a66597, ~BEMP0, BEMPSTS);
		r8a66597_write(r8a66597, BCLR | BVAL, CFIFOCTR);
		enable_irq_empty(r8a66597, 0);
	} else {
		r8a66597_bclr(r8a66597, R8A66597_DIR, DCPCFG);
		r8a66597_mdfy(r8a66597, 0, ISEL | CURPIPE, CFIFOSEL);
		r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, 0);
		r8a66597_write(r8a66597, BCLR, CFIFOCTR);
		enable_irq_ready(r8a66597, 0);
	}
	enable_irq_nrdy(r8a66597, 0);
	pipe_start(r8a66597, td->pipe);
}

static int is_set_address(unsigned char *setup_packet)
{
	if (((setup_packet[0] & USB_TYPE_MASK) == USB_TYPE_STANDARD) &&
			setup_packet[1] == USB_REQ_SET_ADDRESS)
		return 1;
	else
		return 0;
}

/* this function must be called with interrupt disabled */
static int start_transfer(struct r8a66597 *r8a66597, struct r8a66597_td *td)
{
	BUG_ON(!td);

	switch (td->type) {
	case USB_PID_SETUP:
		if (is_set_address(td->urb->setup_packet)) {
			td->set_address = 1;
			td->urb->setup_packet[2] = alloc_usb_address(r8a66597,
								     td->urb);
			if (td->urb->setup_packet[2] == 0)
				return -EPIPE;
		}
		prepare_setup_packet(r8a66597, td);
		break;
	case USB_PID_IN:
		prepare_packet_read(r8a66597, td);
		break;
	case USB_PID_OUT:
		prepare_packet_write(r8a66597, td);
		break;
	case USB_PID_ACK:
		prepare_status_packet(r8a66597, td);
		break;
	default:
		printk(KERN_ERR "r8a66597: invalid type.\n");
		break;
	}

	return 0;
}

static int check_transfer_finish(struct r8a66597_td *td, struct urb *urb)
{
	if (usb_pipeisoc(urb->pipe)) {
		if (urb->number_of_packets == td->iso_cnt)
			return 1;
	}

	/* control or bulk or interrupt */
	if ((urb->transfer_buffer_length <= urb->actual_length) ||
	    (td->short_packet) || (td->zero_packet))
		return 1;

	return 0;
}

/* this function must be called with interrupt disabled */
static void set_td_timer(struct r8a66597 *r8a66597, struct r8a66597_td *td)
{
	unsigned long time;

	BUG_ON(!td);

	if (!list_empty(&r8a66597->pipe_queue[td->pipenum]) &&
	    !usb_pipecontrol(td->urb->pipe) && usb_pipein(td->urb->pipe)) {
		r8a66597->timeout_map |= 1 << td->pipenum;
		switch (usb_pipetype(td->urb->pipe)) {
		case PIPE_INTERRUPT:
		case PIPE_ISOCHRONOUS:
			time = 30;
			break;
		default:
			time = 300;
			break;
		}

		mod_timer(&r8a66597->td_timer[td->pipenum],
			  jiffies + msecs_to_jiffies(time));
	}
}

/* this function must be called with interrupt disabled */
static void finish_request(struct r8a66597 *r8a66597, struct r8a66597_td *td,
		u16 pipenum, struct urb *urb, int status)
__releases(r8a66597->lock) __acquires(r8a66597->lock)
{
	int restart = 0;
	struct usb_hcd *hcd = r8a66597_to_hcd(r8a66597);

	r8a66597->timeout_map &= ~(1 << pipenum);

	if (likely(td)) {
		if (td->set_address && (status != 0 || urb->unlinked))
			r8a66597->address_map &= ~(1 << urb->setup_packet[2]);

		pipe_toggle_save(r8a66597, td->pipe, urb);
		list_del(&td->queue);
		kfree(td);
	}

	if (!list_empty(&r8a66597->pipe_queue[pipenum]))
		restart = 1;

	if (likely(urb)) {
		if (usb_pipeisoc(urb->pipe))
			urb->start_frame = r8a66597_get_frame(hcd);

		usb_hcd_unlink_urb_from_ep(r8a66597_to_hcd(r8a66597), urb);
		spin_unlock(&r8a66597->lock);
		usb_hcd_giveback_urb(hcd, urb, status);
		spin_lock(&r8a66597->lock);
	}

	if (restart) {
		td = r8a66597_get_td(r8a66597, pipenum);
		if (unlikely(!td))
			return;

		start_transfer(r8a66597, td);
		set_td_timer(r8a66597, td);
	}
}

static void packet_read(struct r8a66597 *r8a66597, u16 pipenum)
{
	u16 tmp;
	int rcv_len, bufsize, urb_len, size;
	u16 *buf;
	struct r8a66597_td *td = r8a66597_get_td(r8a66597, pipenum);
	struct urb *urb;
	int finish = 0;
	int status = 0;

	if (unlikely(!td))
		return;
	urb = td->urb;

	fifo_change_from_pipe(r8a66597, td->pipe);
	tmp = r8a66597_read(r8a66597, td->pipe->fifoctr);
	if (unlikely((tmp & FRDY) == 0)) {
		pipe_stop(r8a66597, td->pipe);
		pipe_irq_disable(r8a66597, pipenum);
		printk(KERN_ERR "r8a66597: in fifo not ready (%d)\n", pipenum);
		finish_request(r8a66597, td, pipenum, td->urb, -EPIPE);
		return;
	}

	/* prepare parameters */
	rcv_len = tmp & DTLN;
	if (usb_pipeisoc(urb->pipe)) {
		buf = (u16 *)(urb->transfer_buffer +
				urb->iso_frame_desc[td->iso_cnt].offset);
		urb_len = urb->iso_frame_desc[td->iso_cnt].length;
	} else {
		buf = (void *)urb->transfer_buffer + urb->actual_length;
		urb_len = urb->transfer_buffer_length - urb->actual_length;
	}
	bufsize = min(urb_len, (int) td->maxpacket);
	if (rcv_len <= bufsize) {
		size = rcv_len;
	} else {
		size = bufsize;
		status = -EOVERFLOW;
		finish = 1;
	}

	/* update parameters */
	urb->actual_length += size;
	if (rcv_len == 0)
		td->zero_packet = 1;
	if (rcv_len < bufsize) {
		td->short_packet = 1;
	}
	if (usb_pipeisoc(urb->pipe)) {
		urb->iso_frame_desc[td->iso_cnt].actual_length = size;
		urb->iso_frame_desc[td->iso_cnt].status = status;
		td->iso_cnt++;
		finish = 0;
	}

	/* check transfer finish */
	if (finish || check_transfer_finish(td, urb)) {
		pipe_stop(r8a66597, td->pipe);
		pipe_irq_disable(r8a66597, pipenum);
		finish = 1;
	}

	/* read fifo */
	if (urb->transfer_buffer) {
		if (size == 0)
			r8a66597_write(r8a66597, BCLR, td->pipe->fifoctr);
		else
			r8a66597_read_fifo(r8a66597, td->pipe->fifoaddr,
					   buf, size);
	}

	if (finish && pipenum != 0)
		finish_request(r8a66597, td, pipenum, urb, status);
}

static void packet_write(struct r8a66597 *r8a66597, u16 pipenum)
{
	u16 tmp;
	int bufsize, size;
	u16 *buf;
	struct r8a66597_td *td = r8a66597_get_td(r8a66597, pipenum);
	struct urb *urb;

	if (unlikely(!td))
		return;
	urb = td->urb;

	fifo_change_from_pipe(r8a66597, td->pipe);
	tmp = r8a66597_read(r8a66597, td->pipe->fifoctr);
	if (unlikely((tmp & FRDY) == 0)) {
		pipe_stop(r8a66597, td->pipe);
		pipe_irq_disable(r8a66597, pipenum);
		printk(KERN_ERR "r8a66597: out fifo not ready (%d)\n", pipenum);
		finish_request(r8a66597, td, pipenum, urb, -EPIPE);
		return;
	}

	/* prepare parameters */
	bufsize = td->maxpacket;
	if (usb_pipeisoc(urb->pipe)) {
		buf = (u16 *)(urb->transfer_buffer +
				urb->iso_frame_desc[td->iso_cnt].offset);
		size = min(bufsize,
			   (int)urb->iso_frame_desc[td->iso_cnt].length);
	} else {
		buf = (u16 *)(urb->transfer_buffer + urb->actual_length);
		size = min_t(u32, bufsize,
			   urb->transfer_buffer_length - urb->actual_length);
	}

	/* write fifo */
	if (pipenum > 0)
		r8a66597_write(r8a66597, ~(1 << pipenum), BEMPSTS);
	if (urb->transfer_buffer) {
		r8a66597_write_fifo(r8a66597, td->pipe->fifoaddr, buf, size);
		if (!usb_pipebulk(urb->pipe) || td->maxpacket != size)
			r8a66597_write(r8a66597, BVAL, td->pipe->fifoctr);
	}

	/* update parameters */
	urb->actual_length += size;
	if (usb_pipeisoc(urb->pipe)) {
		urb->iso_frame_desc[td->iso_cnt].actual_length = size;
		urb->iso_frame_desc[td->iso_cnt].status = 0;
		td->iso_cnt++;
	}

	/* check transfer finish */
	if (check_transfer_finish(td, urb)) {
		disable_irq_ready(r8a66597, pipenum);
		enable_irq_empty(r8a66597, pipenum);
		if (!usb_pipeisoc(urb->pipe))
			enable_irq_nrdy(r8a66597, pipenum);
	} else
		pipe_irq_enable(r8a66597, urb, pipenum);
}


static void check_next_phase(struct r8a66597 *r8a66597, int status)
{
	struct r8a66597_td *td = r8a66597_get_td(r8a66597, 0);
	struct urb *urb;
	u8 finish = 0;

	if (unlikely(!td))
		return;
	urb = td->urb;

	switch (td->type) {
	case USB_PID_IN:
	case USB_PID_OUT:
		if (check_transfer_finish(td, urb))
			td->type = USB_PID_ACK;
		break;
	case USB_PID_SETUP:
		if (urb->transfer_buffer_length == urb->actual_length)
			td->type = USB_PID_ACK;
		else if (usb_pipeout(urb->pipe))
			td->type = USB_PID_OUT;
		else
			td->type = USB_PID_IN;
		break;
	case USB_PID_ACK:
		finish = 1;
		break;
	}

	if (finish || status != 0 || urb->unlinked)
		finish_request(r8a66597, td, 0, urb, status);
	else
		start_transfer(r8a66597, td);
}

static int get_urb_error(struct r8a66597 *r8a66597, u16 pipenum)
{
	struct r8a66597_td *td = r8a66597_get_td(r8a66597, pipenum);

	if (td) {
		u16 pid = r8a66597_read(r8a66597, td->pipe->pipectr) & PID;

		if (pid == PID_NAK)
			return -ECONNRESET;
		else
			return -EPIPE;
	}
	return 0;
}

static void irq_pipe_ready(struct r8a66597 *r8a66597)
{
	u16 check;
	u16 pipenum;
	u16 mask;
	struct r8a66597_td *td;

	mask = r8a66597_read(r8a66597, BRDYSTS)
	       & r8a66597_read(r8a66597, BRDYENB);
	r8a66597_write(r8a66597, ~mask, BRDYSTS);
	if (mask & BRDY0) {
		td = r8a66597_get_td(r8a66597, 0);
		if (td && td->type == USB_PID_IN)
			packet_read(r8a66597, 0);
		else
			pipe_irq_disable(r8a66597, 0);
		check_next_phase(r8a66597, 0);
	}

	for (pipenum = 1; pipenum < R8A66597_MAX_NUM_PIPE; pipenum++) {
		check = 1 << pipenum;
		if (mask & check) {
			td = r8a66597_get_td(r8a66597, pipenum);
			if (unlikely(!td))
				continue;

			if (td->type == USB_PID_IN)
				packet_read(r8a66597, pipenum);
			else if (td->type == USB_PID_OUT)
				packet_write(r8a66597, pipenum);
		}
	}
}

static void irq_pipe_empty(struct r8a66597 *r8a66597)
{
	u16 tmp;
	u16 check;
	u16 pipenum;
	u16 mask;
	struct r8a66597_td *td;

	mask = r8a66597_read(r8a66597, BEMPSTS)
	       & r8a66597_read(r8a66597, BEMPENB);
	r8a66597_write(r8a66597, ~mask, BEMPSTS);
	if (mask & BEMP0) {
		cfifo_change(r8a66597, 0);
		td = r8a66597_get_td(r8a66597, 0);
		if (td && td->type != USB_PID_OUT)
			disable_irq_empty(r8a66597, 0);
		check_next_phase(r8a66597, 0);
	}

	for (pipenum = 1; pipenum < R8A66597_MAX_NUM_PIPE; pipenum++) {
		check = 1 << pipenum;
		if (mask &  check) {
			struct r8a66597_td *td;
			td = r8a66597_get_td(r8a66597, pipenum);
			if (unlikely(!td))
				continue;

			tmp = r8a66597_read(r8a66597, td->pipe->pipectr);
			if ((tmp & INBUFM) == 0) {
				disable_irq_empty(r8a66597, pipenum);
				pipe_irq_disable(r8a66597, pipenum);
				finish_request(r8a66597, td, pipenum, td->urb,
						0);
			}
		}
	}
}

static void irq_pipe_nrdy(struct r8a66597 *r8a66597)
{
	u16 check;
	u16 pipenum;
	u16 mask;
	int status;

	mask = r8a66597_read(r8a66597, NRDYSTS)
	       & r8a66597_read(r8a66597, NRDYENB);
	r8a66597_write(r8a66597, ~mask, NRDYSTS);
	if (mask & NRDY0) {
		cfifo_change(r8a66597, 0);
		status = get_urb_error(r8a66597, 0);
		pipe_irq_disable(r8a66597, 0);
		check_next_phase(r8a66597, status);
	}

	for (pipenum = 1; pipenum < R8A66597_MAX_NUM_PIPE; pipenum++) {
		check = 1 << pipenum;
		if (mask & check) {
			struct r8a66597_td *td;
			td = r8a66597_get_td(r8a66597, pipenum);
			if (unlikely(!td))
				continue;

			status = get_urb_error(r8a66597, pipenum);
			pipe_irq_disable(r8a66597, pipenum);
			pipe_stop(r8a66597, td->pipe);
			finish_request(r8a66597, td, pipenum, td->urb, status);
		}
	}
}

static irqreturn_t r8a66597_irq(struct usb_hcd *hcd)
{
	struct r8a66597 *r8a66597 = hcd_to_r8a66597(hcd);
	u16 intsts0, intsts1, intsts2;
	u16 intenb0, intenb1, intenb2;
	u16 mask0, mask1, mask2;
	int status;

	spin_lock(&r8a66597->lock);

	intsts0 = r8a66597_read(r8a66597, INTSTS0);
	intsts1 = r8a66597_read(r8a6659te (hdsp, HDSP_control2Reg, hdsp->control2_register);

	hdsp_reset_hw_pointer(hdsp);
	hdsp_compute_period_size(hdsp);

	/* silence everything */

	for (i = 0; i < HDSP_MATRIX_MIXER_SIZE; ++i)
		hdsp->mixer_matrix[i] = MINUS_INFINITY_GAIN;

	for (i = 0; i < ((hdsp->io_type == H9652 || hdsp->io_type == H9632) ? 1352 : HDSP_MATRIX_MIXER_SIZE); ++i) {
		if (hdsp_write_gain (hdsp, i, MINUS_INFINITY_GAIN))
			return -EIO;
	}

	/* H9632 specific defaults */
	if (hdsp->io_type == H9632) {
		hdsp->control_register |= (HDSP_DAGainPlus4dBu | HDSP_ADGainPlus4dBu | HDSP_PhoneGain0dB);
		hdsp_write(hdsp, HDSP_controlRegister, hdsp->control_register);
	}

	/* set a default rate so that the channel map is set up.
	 */

	hdsp_set_rate(hdsp, 48000, 1);

	return 0;
}

static void hdsp_midi_tasklet(unsigned long arg)
{
	struct hdsp *hdsp = (struct hdsp *)arg;

	if (hdsp->midi[0].pending)
		snd_hdsp_midi_input_read (&hdsp->midi[0]);
	if (hdsp->midi[1].pending)
		snd_hdsp_midi_input_read (&hdsp->midi[1]);
}

static irqreturn_t snd_hdsp_interrupt(int irq, void *dev_id)
{
	struct hdsp *hdsp = (struct hdsp *) dev_id;
	unsigned int status;
	int audio;
	int midi0;
	int midi1;
	unsigned int midi0status;
	unsigned int midi1status;
	int schedule = 0;

	status = hdsp_read(hdsp, HDSP_statusRegister);

	audio = status & HDSP_audioIRQPending;
	midi0 = status & HDSP_midi0IRQPending;
	midi1 = status & HDSP_midi1IRQPending;

	if (!audio && !midi0 && !midi1)
		return IRQ_NONE;

	hdsp_write(hdsp, HDSP_interruptConfirmation, 0);

	midi0status = hdsp_read (hdsp, HDSP_midiStatusIn0) & 0xff;
	midi1status = hdsp_read (hdsp, HDSP_midiStatusIn1) & 0xff;

	if (!(hdsp->state & HDSP_InitializationComplete))
		return IRQ_HANDLED;

	if (audio) {
		if (hdsp->capture_substream)
			snd_pcm_period_elapsed(hdsp->pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream);

		if (hdsp->playback_substream)
			snd_pcm_period_elapsed(hdsp->pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream);
	}

	if (midi0 && midi0status) {
		if (hdsp->use_midi_tasklet) {
			/* we disable interrupts for this input until processing is done */
			hdsp->control_register &= ~HDSP_Midi0InterruptEnable;
			hdsp_write(hdsp, HDSP_controlRegister, hdsp->control_register);
			hdsp->midi[0].pending = 1;
			schedule = 1;
		} else {
			snd_hdsp_midi_input_read (&hdsp->midi[0]);
		}
	}
	if (hdsp->io_type != Multiface && hdsp->io_type != H9632 && midi1 && midi1status) {
		if (hdsp->use_midi_tasklet) {
			/* we disable interrupts for this input until processing is done */
			hdsp->control_register &= ~HDSP_Midi1InterruptEnable;
			hdsp_write(hdsp, HDSP_controlRegister, hdsp->control_register);
			hdsp->midi[1].pending = 1;
			schedule = 1;
		} else {
			snd_hdsp_midi_input_read (&hdsp->midi[1]);
		}
	}
	if (hdsp->use_midi_tasklet && schedule)
		tasklet_schedule(&hdsp->midi_tasklet);
	return IRQ_HANDLED;
}

static snd_pcm_uframes_t snd_hdsp_hw_pointer(struct snd_pcm_substream *substream)
{
	struct hdsp *hdsp = snd_pcm_substream_chip(substream);
	return hdsp_hw_pointer(hdsp);
}

static char *hdsp_channel_buffer_location(struct hdsp *hdsp,
					     int stream,
					     int channel)

{
	int mapped_channel;

        if (snd_BUG_ON(channel < 0 || channel >= hdsp->max_channels))
		return NULL;

	if ((mapped_channel = hdsp->channel_map[channel]) < 0)
		return NULL;

	if (stream == SNDRV_PCM_STREAM_CAPTURE)
		return hdsp->capture_buffer + (mapped_channel * HDSP_CHANNEL_BUFFER_BYTES);
	else
		return hdsp->playback_buffer + (mapped_channel * HDSP_CHANNEL_BUFFER_BYTES);
}

static int snd_hdsp_playback_copy(struct snd_pcm_substream *substream, int channel,
				  snd_pcm_uframes_t pos, void __user *src, snd_pcm_uframes_t count)
{
	struct hdsp *hdsp = snd_pcm_substream_chip(substream);
	char *channel_buf;

	if (snd_BUG_ON(pos + count > HDSP_CHANNEL_BUFFER_BYTES / 4))
		return -EINVAL;

	channel_buf = hdsp_channel_buffer_location (hdsp, substream->pstr->stream, channel);
	if (snd_BUG_ON(!channel_buf))
		return -EIO;
	if (copy_from_user(channel_buf + pos * 4, src, count * 4))
		return -EFAULT;
	return count;
}

static int snd_hdsp_capture_copy(struct snd_pcm_substream *substream, int channel,
				 snd_pcm_uframes_t pos, void __user *dst, snd_pcm_uframes_t count)
{
	struct hdsp *hdsp = snd_pcm_substream_chip(substream);
	char *channel_buf;

	if (snd_BUG_ON(pos + count > HDSP_CHANNEL_BUFFER_BYTES / 4))
		return -EINVAL;

	channel_buf = hdsp_channel_buffer_location (hdsp, substream->pstr->stream, channel);
	if (snd_BUG_ON(!channel_buf))
		return -EIO;
	if (copy_to_user(dst, channel_buf + pos * 4, count * 4))
		return -EFAULT;
	return count;
}

static int snd_hdsp_hw_silence(struct snd_pcm_substream *substream, int channel,
				  snd_pcm_uframes_t pos, snd_pcm_uframes_t count)
{
	struct hdsp *hdsp = snd_pcm_substream_chip(substream);
	char *channel_buf;

	channel_buf = hdsp_channel_buffer_location (hdsp, substream->pstr->stream, channel);
	if (snd_BUG_ON(!channel_buf))
		return -EIO;
	memset(channel_buf + pos * 4, 0, count * 4);
	return count;
}

static int snd_hdsp_reset(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct hdsp *hdsp = snd_pcm_substream_chip(substream);
	struct snd_pcm_substream *other;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		other = hdsp->capture_substream;
	else
		other = hdsp->playback_substream;
	if (hdsp->running)
		runtime->status->hw_ptr = hdsp_hw_pointer(hdsp);
	else
		runtime->status->hw_ptr = 0;
	if (other) {
		struct snd_pcm_substream *s;
		struct snd_pcm_runtime *oruntime = other->runtime;
		snd_pcm_group_for_each_entry(s, substream) {
			if (s == other) {
				oruntime->status->hw_ptr = runtime->status->hw_ptr;
				break;
			}
		}
	}
	return 0;
}

static int snd_hdsp_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params)
{
	struct hdsp *hdsp = snd_pcm_substream_chip(substream);
	int err;
	pid_t this_pid;
	pid_t other_pid;

	if (hdsp_check_for_iobox (hdsp))
		return -EIO;

	if (hdsp_check_for_firmware(hdsp, 1))
		return -EIO;

	spin_lock_irq(&hdsp->lock);

	if (substream->pstr->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		hdsp->control_register &= ~(HDSP_SPDIFProfessional | HDSP_SPDIFNonAudio | HDSP_SPDIFEmphasis);
		hdsp_write(hdsp, HDSP_controlRegister, hdsp->control_register |= hdsp->creg_spdif_stream);
		this_pid = hdsp->playback_pid;
		other_pid = hdsp->capture_pid;
	} else {
		this_pid = hdsp->capture_pid;
		other_pid = hdsp->playback_pid;
	}

	if ((other_pid > 0) && (this_pid != other_pid)) {

		/* The other stream is open, and not by the same
		   task as this one. Make sure that the parameters
		   that matter are the same.
		 */

		if (params_rate(params) != hdsp->system_sample_rate) {
			spin_unlock_irq(&hdsp->lock);
			_snd_pcm_hw_param_setempty(params, SNDRV_PCM_HW_PARAM_RATE);
			return -EBUSY;
		}

		if (params_period_size(params) != hdsp->period_bytes / 4) {
			spin_unlock_irq(&hdsp->lock);
			_snd_pcm_hw_param_setempty(params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE);
			return -EBUSY;
		}

		/* We're fine. */

		spin_unlock_irq(&hdsp->lock);
 		return 0;

	} else {
		spin_unlock_irq(&hdsp->lock);
	}

	/* how to make sure that the rate matches an externally-set one ?
	 */

	spin_lock_irq(&hdsp->lock);
	if (! hdsp->clock_source_locked) {
		if ((err = hdsp_set_rate(hdsp, params_rate(params), 0)) < 0) {
			spin_unlock_irq(&hdsp->lock);
			_snd_pcm_hw_param_setempty(params, SNDRV_PCM_HW_PARAM_RATE);
			return err;
		}
	}
	spin_unlock_irq(&hdsp->lock);

	if ((err = hdsp_set_interrupt_interval(hdsp, params_period_size(params))) < 0) {
		_snd_pcm_hw_param_setempty(params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE);
		return err;
	}

	return 0;
}

static int snd_hdsp_channel_info(struct snd_pcm_substream *substream,
				    struct snd_pcm_channel_info *info)
{
	struct hdsp *hdsp = snd_pcm_substream_chip(substream);
	int mapped_channel;

	if (snd_BUG_ON(info->channel >= hdsp->max_channels))
		return -EINVAL;

	if ((mapped_channel = hdsp->channel_map[info->channel]) < 0)
		return -EINVAL;

	info->offset = mapped_channel * HDSP_CHANNEL_BUFFER_BYTES;
	info->first = 0;
	info->step = 32;
	return 0;
}

static int snd_hdsp_ioctl(struct snd_pcm_substream *subret = start_transfer(r8a66597, td);
			if (ret < 0) {
				list_del(&td->queue);
				kfree(td);
			}
		}
	} else
		set_td_timer(r8a66597, td);

error:
	if (ret)
		usb_hcd_unlink_urb_from_ep(hcd, urb);
error_not_linked:
	spin_unlock_irqrestore(&r8a66597->lock, flags);
	return ret;
}

static int r8a66597_urb_dequeue(struct usb_hcd *hcd, struct urb *urb,
		int status)
{
	struct r8a66597 *r8a66597 = hcd_to_r8a66597(hcd);
	struct r8a66597_td *td;
	unsigned long flags;
	int rc;

	spin_lock_irqsave(&r8a66597->lock, flags);
	rc = usb_hcd_check_unlink_urb(hcd, urb, status);
	if (rc)
		goto done;

	if (urb->hcpriv) {
		td = urb->hcpriv;
		pipe_stop(r8a66597, td->pipe);
		pipe_irq_disable(r8a66597, td->pipenum);
		disable_irq_empty(r8a66597, td->pipenum);
		finish_request(r8a66597, td, td->pipenum, urb, status);
	}
 done:
	spin_unlock_irqrestore(&r8a66597->lock, flags);
	return rc;
}

static void r8a66597_endpoint_disable(struct usb_hcd *hcd,
				      struct usb_host_endpoint *hep)
{
	struct r8a66597 *r8a66597 = hcd_to_r8a66597(hcd);
	struct r8a66597_pipe *pipe = (struct r8a66597_pipe *)hep->hcpriv;
	struct r8a66597_td *td;
	struct urb *urb = NULL;
	u16 pipenum;
	unsigned long flags;

	if (pipe == NULL)
		return;
	pipenum = pipe->info.pipenum;

	if (pipenum == 0) {
		kfree(hep->hcpriv);
		hep->hcpriv = NULL;
		return;
	}

	spin_lock_irqsave(&r8a66597->lock, flags);
	pipe_stop(r8a66597, pipe);
	pipe_irq_disable(r8a66597, pipenum);
	disable_irq_empty(r8a66597, pipenum);
	td = r8a66597_get_td(r8a66597, pipenum);
	if (td)
		urb = td->urb;
	finish_request(r8a66597, td, pipenum, urb, -ESHUTDOWN);
	kfree(hep->hcpriv);
	hep->hcpriv = NULL;
	spin_unlock_irqrestore(&r8a66597->lock, flags);
}

static int r8a66597_get_frame(struct usb_hcd *hcd)
{
	struct r8a66597 *r8a66597 = hcd_to_r8a66597(hcd);
	return r8a66597_read(r8a66597, FRMNUM) & 0x03FF;
}

static void collect_usb_address_map(struct usb_device *udev, unsigned long *map)
{
	int chix;

	if (udev->state == USB_STATE_CONFIGURED &&
	    udev->parent && udev->parent->devnum > 1 &&
	    udev->parent->descriptor.bDeviceClass == USB_CLASS_HUB)
		map[udev->devnum/32] |= (1 << (udev->devnum % 32));

	for (chix = 0; chix < udev->maxchild; chix++) {
		struct usb_device *childdev = udev->children[chix];

		if (childdev)
			collect_usb_address_map(childdev, map);
	}
}

/* this function must be called with interrupt disabled */
static struct r8a66597_device *get_r8a66597_device(struct r8a66597 *r8a66597,
						   int addr)
{
	struct r8a66597_device *dev;
	struct list_head *list = &r8a66597->child_device;

	list_for_each_entry(dev, list, device_list) {
		if (!dev)
			continue;
		if (dev->usb_address != addr)
			continue;

		return dev;
	}

	printk(KERN_ERR "r8a66597: get_r8a66597_device fail.(%d)\n", addr);
	return NULL;
}

static void update_usb_address_map(struct r8a66597 *r8a66597,
				   struct usb_device *root_hub,
				   unsigned long *map)
{
	int i, j, addr;
	unsigned long diff;
	unsigned long flags;

	for (i = 0; i < 4; i++) {
		diff = r8a66597->child_connect_map[i] ^ map[i];
		if (!diff)
			continue;

		for (j = 0; j < 32; j++) {
			if (!(diff & (1 << j)))
				continue;

			addr = i * 32 + j;
			if (map[i] & (1 << j))
				set_child_connect_map(r8a66597, addr);
			else {
				struct r8a66597_device *dev;

				spin_lock_irqsave(&r8a66597->lock, flags);
				dev = get_r8a66597_device(r8a66597, addr);
				disable_r8a66597_pipe_all(r8a66597, dev);
				free_usb_address(r8a66597, dev);
				put_child_connect_map(r8a66597, addr);
				spin_unlock_irqrestore(&r8a66597->lock, flags);
			}
		}
	}
}

static void r8a66597_check_detect_child(struct r8a66597 *r8a66597,
					struct usb_hcd *hcd)
{
	struct usb_bus *bus;
	unsigned long now_map[4];

	memset(now_map, 0, sizeof(now_map));

	list_for_each_entry(bus, &usb_bus_list, bus_list) {
		if (!bus->root_hub)
			continue;

		if (bus->busnum != hcd->self.busnum)
			continue;

		collect_usb_address_map(bus->root_hub, now_map);
		update_usb_address_map(r8a66597, bus->root_hub, now_map);
	}
}

static int r8a66597_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	struct r8a66597 *r8a66597 = hcd_to_r8a66597(hcd);
	unsigned long flags;
	int i;

	r8a66597_check_detect_child(r8a66597, hcd);

	spin_lock_irqsave(&r8a66597->lock, flags);

	*buf = 0;	/* initialize (no change) */

	for (i = 0; i < R8A66597_MAX_ROOT_HUB; i++) {
		if (r8a66597->root_hub[i].port & 0xffff0000)
			*buf |= 1 << (i + 1);
	}

	spin_unlock_irqrestore(&r8a66597->lock, flags);

	return (*buf != 0);
}

static void r8a66597_hub_descriptor(struct r8a66597 *r8a66597,
				    struct usb_hub_descriptor *desc)
{
	desc->bDescriptorType = 0x29;
	desc->bHubContrCurrent = 0;
	desc->bNbrPorts = R8A66597_MAX_ROOT_HUB;
	desc->bDescLength = 9;
	desc->bPwrOn2PwrGood = 0;
	desc->wHubCharacteristics = cpu_to_le16(0x0011);
	desc->bitmap[0] = ((1 << R8A66597_MAX_ROOT_HUB) - 1) << 1;
	desc->bitmap[1] = ~0;
}

static int r8a66597_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue,
				u16 wIndex, char *buf, u16 wLength)
{
	struct r8a66597 *r8a66597 = hcd_to_r8a66597(hcd);
	int ret;
	int port = (wIndex & 0x00FF) - 1;
	struct r8a66597_root_hub *rh = &r8a66597->root_hub[port];
	unsigned long flags;

	ret = 0;

	spin_lock_irqsave(&r8a66597->lock, flags);
	switch (typeReq) {
	case ClearHubFeature:
	case SetHubFeature:
		switch (wValue) {
		case C_HUB_OVER_CURRENT:
		case C_HUB_LOCAL_POWER:
			break;
		default:
			goto error;
		}
		break;
	case ClearPortFeature:
		if (wIndex > R8A66597_MAX_ROOT_HUB)
			goto error;
		if (wLength != 0)
			goto error;

		switch (wValue) {
		case USB_PORT_FEAT_ENABLE:
			rh->port &= ~(1 << USB_PORT_FEAT_POWER);
			break;
		case USB_PORT_FEAT_SUSPEND:
			break;
		case USB_PORT_FEAT_POWER:
			r8a66597_port_power(r8a66597, port, 0);
			break;
		case USB_PORT_FEAT_C_ENABLE:
		case USB_PORT_FEAT_C_SUSPEND:
		case USB_PORT_FEAT_C_CONNECTION:
		case USB_PORT_FEAT_C_OVER_CURRENT:
		case USB_PORT_FEAT_C_RESET:
			break;
		default:
			goto error;
		}
		rh->port &= ~(1 << wValue);
		break;
	case GetHubDescriptor:
		r8a66597_hub_descriptor(r8a66597,
					(struct usb_hub_descriptor *)buf);
		break;
	case GetHubStatus:
		*buf = 0x00;
		break;
	case GetPortStatus:
		if (wIndex > R8A66597_MAX_ROOT_HUB)
			goto error;
		*(__le32 *)buf = cpu_to_le32(rh->port);
		break;
	case SetPortFeature:
		if (wIndex > R8A66597_MAX_ROOT_HUB)
			goto error;
		if (wLength != 0)
			goto error;

		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			break;
		case USB_PORT_FEAT_POWER:
			r8a66597_port_power(r8a66597, port, 1);
			rh->port |= (1 << USB_PORT_FEAT_POWER);
			break;
		case USB_PORT_FEAT_RESET: {
			struct r8a66597_device *dev = rh->dev;

			rh->port |= (1 << USB_PORT_FEAT_RESET);

			disable_r8a66597_pipe_all(r8a66597, dev);
			free_usb_address(r8a66597, dev);

			r8a66597_mdfy(r8a66597, USBRST, USBRST | UACT,
				      get_dvstctr_reg(port));
			mod_timer(&r8a66597->rh_timer,
				  jiffies + msecs_to_jiffies(50));
			}
			break;
		default:
			goto error;
		}
		rh->port |= 1 << wValue;
		break;
	default:
error:
		ret = -EPIPE;
		break;
	}

	spin_unlock_irqrestore(&r8a66597->lock, flags);
	return ret;
}

#if defined(CONFIG_PM)
static int r8a66597_bus_suspend(struct usb_hcd *hcd)
{
	struct r8a66597 *r8a66597 = hcd_to_r8a66597(hcd);
	int port;

	dbg("%s", __func__);

	for (port = 0; port < R8A66597_MAX_ROOT_HUB; port++) {
		struct r8a66597_root_hub *rh = &r8a66597->root_hub[port];
		unsigned long dvstctr_reg = get_dvstctr_reg(port);

		if (!(rh->port & (1 << USB_PORT_FEAT_ENABLE)))
			continue;

		dbg("suspend port = %d", port);
		r8a66597_bclr(r8a66597, UACT, dvstctr_reg);	/* suspend */
		rh->port |= 1 << USB_PORT_FEAT_SUSPEND;

		if (rh->dev->udev->do_remote_wakeup) {
			msleep(3);	/* waiting last SOF */
			r8a66597_bset(r8a66597, RWUPE, dvstctr_reg);
			r8a66597_write(r8a66597, ~BCHG, get_intsts_reg(port));
			r8a66597_bset(r8a66597, BCHGE, get_intenb_reg(port));
		}
	}

	r8a66597->bus_suspended = 1;

	return 0;
}

static int r8a66597_bus_resume(struct usb_hcd *hcd)
{
	struct r8a66597 *r8a66597 = hcd_to_r8a66597(hcd);
	int port;

	dbg("%s", __func__);

	for (port = 0; port < R8A66597_MAX_ROOT_HUB; port++) {
		struct r8a66597_root_hub *rh = &r8a66597->root_hub[port];
		unsigned long dvstctr_reg = get_dvstctr_reg(port);

		if (!(rh->port & (1 << USB_PORT_FEAT_SUSPEND)))
			continue;

		dbg("resume port = %d", port);
		rh->port &= ~(1 << USB_PORT_FEAT_SUSPEND);
		rh->port |= 1 << USB_PORT_FEAT_C_SUSPEND;
		r8a66597_mdfy(r8a66597, RESUME, RESUME | UACT, dvstctr_reg);
		msleep(50);
		r8a66597_mdfy(r8a66597, UACT, RESUME | UACT, dvstctr_reg);
	}

	return 0;

}
#else
#define	r8a66597_bus_suspend	NULL
#define	r8a66597_bus_resume	NULL
#endif

static struct hc_driver r8a66597_hc_driver = {
	.description =		hcd_name,
	.hcd_priv_size =	sizeof(struct r8a66597),
	.irq =			r8a66597_irq,

	/*
	 * generic hardware linkage
	 */
	.flags =		HCD_USB2,

	.start =		r8a66597_start,
	.stop =			r8a66597_stop,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		r8a66597_urb_enqueue,
	.urb_dequeue =		r8a66597_urb_dequeue,
	.endpoint_disable =	r8a66597_endpoint_disable,

	/*
	 * periodic schedule support
	 */
	.get_frame_number =	r8a66597_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	r8a66597_hub_status_data,
	.hub_control =		r8a66597_hub_control,
	.bus_suspend =		r8a66597_bus_suspend,
	.bus_resume =		r8a66597_bus_resume,
};

#if defined(CONFIG_PM)
static int r8a66597_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct r8a66597		*r8a66597 = dev_get_drvdata(&pdev->dev);
	int port;

	dbg("%s", __func__);

	disable_controller(r8a66597);

	for (port = 0; port < R8A66597_MAX_ROOT_HUB; port++) {
		struct r8a66597_root_hub *rh = &r8a66597->root_hub[port];

		rh->port = 0x00000000;
	}

	return 0;
}

static int r8a66597_resume(struct platform_device *pdev)
{
	struct r8a66597		*r8a66597 = dev_get_drvdata(&pdev->dev);
	struct usb_hcd		*hcd = r8a66597_to_hcd(r8a66597);

	dbg("%s", __func__);

	enable_controller(r8a66597);
	usb_root_hub_lost_power(hcd->self.root_hub);

	return 0;
}
#else	/* if defined(CONFIG_PM) */
#define r8a66597_suspend	NULL
#define r8a66597_resume		NULL
#endif

static int __init_or_module r8a66597_remove(struct platform_device *pdev)
{
	struct r8a66597		*r8a66597 = dev_get_drvdata(&pdev->dev);
	struct usb_hcd		*hcd = r8a66597_to_hcd(r8a66597);

	del_timer_sync(&r8a66597->rh_timer);
	usb_remove_hcd(hcd);
	iounmap((void *)r8a66597->reg);
#if defined(CONFIG_SUPERH_ON_CHIP_R8A66597) && defined(CONFIG_HAVE_CLK)
	clk_put(r8a66597->clk);
#endif
	usb_put_hcd(hcd);
	return 0;
}

static int __devinit r8a66597_probe(struct platform_device *pdev)
{
#if defined(CONFIG_SUPERH_ON_CHIP_R8A66597) && defined(CONFIG_HAVE_CLK)
	char clk_name[8];
#endif
	struct resource *res = NULL, *ires;
	int irq = -1;
	void __iomem *reg = NULL;
	struct usb_hcd *hcd = NULL;
	struct r8a66597 *r8a66597;
	int ret = 0;
	int i;
	unsigned long irq_trigger;

	if (pdev->dev.dma_mask) {
		ret = -EINVAL;
		dev_err(&pdev->dev, "dma not supported\n");
		goto clean_up;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENODEV;
		dev_err(&pdev->dev, "platform_get_resource error.\n");
		goto clean_up;
	}

	ires = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!ires) {
		ret = -ENODEV;
		dev_err(&pdev->dev,
			"platform_get_resource IORESOURCE_IRQ error.\n");
		goto clean_up;
	}

	irq = ires->start;
	irq_trigger = ires->flags & IRQF_TRIGGER_MASK;

	reg = ioremap(res->start, resource_size(res));
	if (reg == NULL) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "ioremap error.\n");
		goto clean_up;
	}

	if (pdev->dev.platform_data == NULL) {
		dev_err(&pdev->dev, "no platform data\n");
		ret = -ENODEV;
		goto clean_up;
	}

	/* initialize hcd */
	hcd = usb_create_hcd(&r8a66597_hc_driver, &pdev->dev, (char *)hcd_name);
	if (!hcd) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "Failed to create hcd\n");
		goto clean_up;
	}
	r8a66597 = hcd_to_r8a66597(hcd);
	memset(r8a66597, 0, sizeof(struct r8a66597));
	dev_set_drvdata(&pdev->dev, r8a66597);
	r8a66597->pdata = pdev->dev.platform_data;
	r8a66597->irq_sense_low = irq_trigger == IRQF_TRIGGER_LOW;

#if defined(CONFIG_SUPERH_ON_CHIP_R8A66597) && defined(CONFIG_HAVE_CLK)
	snprintf(clk_name, sizeof(clk_name), "usb%d", pdev->id);
	r8a66597->clk = clk_get(&pdev->dev, clk_name);
	if (IS_ERR(r8a66597->clk)) {
		dev_err(&pdev->dev, "cannot get clock \"%s\"\n", clk_name);
		ret = PTR_ERR(r8a66597->clk);
		goto clean_up2;
	}
#endif

	spin_lock_init(&r8a66597->lock);
	init_timer(&r8a66597->rh_timer);
	r8a66597->rh_timer.function = r8a66597_timer;
	r8a66597->rh_timer.data = (unsigned long)r8a66597;
	r8a66597->reg = (unsigned long)reg;

	for (i = 0; i < R8A66597_MAX_NUM_PIPE; i++) {
		INIT_LIST_HEAD(&r8a66597->pipe_queue[i]);
		init_timer(&r8a66597->td_timer[i]);
		r8a66597->td_timer[i].function = r8a66597_td_timer;
		r8a66597->td_timer[i].data = (unsigned long)r8a66597;
		setup_timer(&r8a66597->interval_timer[i],
				r8a66597_interval_timer,
				(unsigned long)r8a66597);
	}
	INIT_LIST_HEAD(&r8a66597->child_device);

	hcd->rsrc_start = res->start;

	ret = usb_add_hcd(hcd, irq, IRQF_DISABLED | irq_trigger);
	if (ret != 0) {
		dev_err(&pdev->dev, "Failed to add hcd\n");
		goto clean_up3;
	}

	return 0;

clean_up3:
#if defined(CONFIG_SUPERH_ON_CHIP_R8A66597) && defined(CONFIG_HAVE_CLK)
	clk_put(r8a66597->clk);
clean_up2:
#endif
	usb_put_hcd(hcd);

clean_up:
	if (reg)
		iounmap(reg);

	return ret;
}

static struct platform_driver r8a66597_driver = {
	.probe =	r8a66597_probe,
	.remove =	r8a66597_remove,
	.suspend =	r8a66597_suspend,
	.resume =	r8a66597_resume,
	.driver		= {
		.name = (char *) hcd_name,
		.owner	= THIS_MODULE,
	},
};

static int __init r8a66597_init(void)
{
	if (usb_disabled())
		return -ENODEV;

	printk(KERN_INFO KBUILD_MODNAME ": driver %s, %s\n", hcd_name,
	       DRIVER_VERSION);
	return platform_driver_register(&r8a66597_driver);
}
module_init(r8a66597_init);

static void __exit r8a66597_cleanup(void)
{
	platform_driver_unregister(&r8a66597_driver);
}
module_exit(r8a66597_cleanup);

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
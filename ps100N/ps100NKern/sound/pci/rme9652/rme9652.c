empty(&client->event_list))
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

const struct file_operations fw_device_ops = {
	.owner		= THIS_MODULE,
	.open		= fw_device_op_open,
	.read		= fw_device_op_read,
	.unlocked_ioctl	= fw_device_op_ioctl,
	.poll		= fw_device_op_poll,
	.release	= fw_device_op_release,
	.mmap		= fw_device_op_mmap,

#ifdef CONFIG_COMPAT
	.compat_ioctl	= fw_device_op_compat_ioctl,
#endif
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   /*
	STB0899 Multistandard Frontend driver
	Copyright (C) Manu Abraham (abraham.manu@gmail.com)

	Copyright (C) ST Microelectronics

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __STB0899_DRV_H
#define __STB0899_DRV_H

#include <linux/kernel.h>
#include <linux/module.h>

#include "dvb_frontend.h"

#define STB0899_TSMODE_SERIAL		1
#define STB0899_CLKPOL_FALLING		2
#define STB0899_CLKNULL_PARITY		3
#define STB0899_SYNC_FORCED		4
#define STB0899_FECMODE_DSS		5

struct stb0899_s1_reg {
	u16	address;
	u8	data;
};

struct stb0899_s2_reg {
	u16	offset;
	u32	base_address;
	u32	data;
};

enum stb0899_inversion {
	IQ_SWAP_OFF	= 0,
	IQ_SWAP_ON,
	IQ_SWAP_AUTO
};

#define STB0899_GPIO00				0xf140
#define STB0899_GPIO01				0xf141
#define STB0899_GPIO02				0xf142
#define STB0899_GPIO03				0xf143
#define STB0899_GPIO04				0xf144
#define STB0899_GPIO05				0xf145
#define STB0899_GPIO06				0xf146
#define STB0899_GPIO07				0xf147
#define STB0899_GPIO08				0xf148
#define STB0899_GPIO09				0xf149
#define STB0899_GPIO10				0xf14a
#define STB0899_GPIO11				0xf14b
#define STB0899_GPIO12				0xf14c
#define STB0899_GPIO13				0xf14d
#define STB0899_GPIO14				0xf14e
#define STB0899_GPIO15				0xf14f
#define STB0899_GPIO16				0xf150
#define STB0899_GPIO17				0xf151
#define STB0899_GPIO18				0xf152
#define STB0899_GPIO19				0xf153
#define STB0899_GPIO20				0xf154

#define STB0899_GPIOPULLUP			0x01 /* Output device is connected to Vdd */
#define STB0899_GPIOPULLDN			0x00 /* Output device is connected to Vss */

#define STB0899_POSTPROC_GPIO_POWER		0x00
#define STB0899_POSTPROC_GPIO_LOCK		0x01

/*
 * Post process output configuration control
 * 1. POWER ON/OFF		(index 0)
 * 2. FE_HAS_LOCK/LOCK_LOSS	(index 1)
 *
 * @gpio 	= one of the above listed GPIO's
 * @level	= output state: pulled up or low
 */
struct stb0899_postproc {
	u16	gpio;
	u8	level;
};

struct stb0899_config {
	const struct stb0899_s1_reg	*init_dev;
	const struct stb0899_s2_reg	*init_s2_demod;
	const struct stb0899_s1_reg	*init_s1_demod;
	const struct stb0899_s2_reg	*init_s2_fec;
	const struct stb0899_s1_reg	*init_tst;

	const struct stb0899_postproc	*postproc;

	enum stb0899_inversion		inversion;

	u32	xtal_freq;

	u8	demod_address;
	u8	ts_output_mode;
	u8	block_sync_mode;
	u8	ts_pfbit_toggle;

	u8	clock_polarity;
	u8	data_clk_parity;
	u8	fec_mode;
	u8	data_output_ctl;
	u8	data_fifo_mode;
	u8	out_rate_comp;
	u8	i2c_repeater;
//	int	inversion;
	int	lo_clk;
	int	hi_clk;

	u32	esno_ave;
	u32	esno_quant;
	u32	avframes_coarse;
	u32	avframes_fine;
	u32	miss_threshold;
	u32	uwp_threshold_acq;
	u32	uwp_threshold_track;
	u32	uwp_threshold_sof;
	u32	sof_search_timeout;

	u32	btr_nco_bits;
	u32	btr_gain_shift_offset;
	u32	crl_nco_bits;
	u32	ldpc_max_iter;

	int (*tuner_set_frequency)(struct dvb_frontend *fe, u32 frequency);
	int (*tuner_get_frequency)(struct dvb_frontend *fe, u32 *frequency);
	int (*tuner_set_bandwidth)(struct dvb_frontend *fe, u32 bandwidth);
	int (*tuner_get_bandwidth)(struct dvb_frontend *fe, u32 *bandwidth);
	int (*tuner_set_rfsiggain)(struct dvb_frontend *fe, u32 rf_gain);
};

#if defined(CONFIG_DVB_STB0899) || (defined(CONFIG_DVB_STB0899_MODULE) && defined(MODULE))

extern struct dvb_frontend *stb0899_attach(struct stb0899_config *config,
					   struct i2c_adapter *i2c);

#else

static inline struct dvb_frontend *stb0899_attach(struct stb0899_config *config,
						  struct i2c_adapter *i2c)
{
	printk(KERN_WARNING "%s: Driver disabled by Kconfig\n", __func__);
	return NULL;
}

#endif //CONFIG_DVB_STB0899


#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   /***************************************************************************
 * Video4Linux driver for W996[87]CF JPEG USB Dual Mode Camera Chip.       *
 *                                                                         *
 * Copyright (C) 2002-2004 by Luca Risolia <luca.risolia@studio.unibo.it>  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the Free Software             *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               *
 ***************************************************************************/

#ifndef _W9968CF_H_
#define _W9968CF_H_

#include <linux/videodev2.h>
#include <linux/usb.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/param.h>
#include <linux/types.h>
#include <linux/rwsem.h>
#include <linux/mutex.h>

#include <media/v4l2-device.h>
#include <media/ovcamchip.h>

#include "w9968cf_vpp.h"


/****************************************************************************
 * Default values                                                           *
 ****************************************************************************/

#define W9968CF_VPPMOD_LOAD     1  /* automatic 'w9968cf-vpp' module loading */

/* Comment/uncomment the following line to enable/disable debugging messages */
#define W9968CF_DEBUG

/* These have effect only if W9968CF_DEBUG is defined */
#define W9968CF_DEBUG_LEVEL    2 /* from 0 to 6. 0 for no debug informations */
#define W9968CF_SPECIFIC_DEBUG 0 /* 0 or 1 */

#define W9968CF_MAX_DEVICES    32
#define W9968CF_SIMCAMS        W9968CF_MAX_DEVICES /* simultaneous cameras */

#define W9968CF_MAX_BUFFERS   32
#define W9968CF_BUFFERS       2 /* n. of frame buffers from 2 to MAX_BUFFERS */

/* Maximum data payload sizes in bytes for alternate settings */
static const u16 wMaxPacketSize[] = {1023, 959, 895, 831, 767, 703, 639, 575,
				      511, 447, 383, 319, 255, 191, 127,  63};
#define W9968CF_PACKET_SIZE      1023 /* according to wMaxPacketSizes[] */
#define W9968CF_MIN_PACKET_SIZE  63 /* minimum value */
#define W9968CF_ISO_PACKETS      5 /* n.of packets for isochronous transfers */
#define W9968CF_USB_CTRL_TIMEOUT 1000 /* timeout (ms) for usb control commands */
#define W9968CF_URBS             2 /* n. of scheduled URBs for ISO transfer */

#define W9968CF_I2C_BUS_DELAY    4 /* delay in us for I2C bit r/w operations */
#define W9968CF_I2C_RW_RETRIES   15 /* number of max I2C r/w retries */

/* Available video formats */
struct w9968cf_format {
	const u16 palette;
	const u16 depth;
	const u8 compression;
};

static const struct w9968cf_format w9968cf_formatlist[] = {
	{ VIDEO_PALETTE_UYVY,    16, 0 }, /* original video */
	{ VIDEO_PALETTE_YUV422P, 16, 1 }, /* with JPEG compression */
	{ VIDEO_PALETTE_YUV420P, 12, 1 }, /* with JPEG compression */
	{ VIDEO_PALETTE_YUV420,  12, 1 }, /* same as YUV420P */
	{ VIDEO_PALETTE_YUYV,    16, 0 }, /* software conversion */
	{ VIDEO_PALETTE_YUV422,  16, 0 }, /* software conversion */
	{ VIDEO_PALETTE_GREY,     8, 0 }, /* software conversion */
	{ VIDEO_PALETTE_RGB555,  16, 0 }, /* software conversion */
	{ VIDEO_PALETTE_RGB565,  16, 0 }, /* software conversion */
	{ VIDEO_PALETTE_RGB24,   24, 0 }, /* software conversion */
	{ VIDEO_PALETTE_RGB32,   32, 0 }, /* software conversion */
	{ 0,                      0, 0 }  /* 0 is a terminating entry */
};

#define W9968CF_DECOMPRESSION    2 /* decomp:0=disable,1=force,2=any formats */
#define W9968CF_PALETTE_DECOMP_OFF   VIDEO_PALETTE_UYVY    /* when decomp=0 */
#define W9968CF_PALETTE_DECOMP_FORCE VIDEO_PALETTE_YUV420P /* when decomp=1 */
#define W9968CF_PALETTE_DECOMP_ON    VIDEO_PALETTE_UYVY    /* when decomp=2 */

#define W9968CF_FORCE_RGB        0  /* read RGB instead of BGR, yes=1/no=0 */

#define W9968CF_MAX_WIDTH      800 /* Has effect if up-scaling is on */
#define W9968CF_MAX_HEIGHT     600 /* Has effect if up-scaling is on */
#define W9968CF_WIDTH          320 /* from 128 to 352, multiple of 16 */
#define W9968CF_HEIGHT         240 /* from  96 to 288, multiple of 16 */

#define W9968CF_CLAMPING       0 /* 0 disable, 1 enable video data clamping */
#define W9968CF_FILTER_TYPE    0 /* 0 disable  1 (1-2-1), 2 (2-3-6-3-2) */
#define W9968CF_DOUBLE_BUFFER  1 /* 0 disable, 1 enable double buffer */
#define W9968CF_LARGEVIEW      1 /* 0 disable, 1 enable */
#define W9968CF_UPSCALING      0 /* 0 disable, 1 enable */

#define W9968CF_MONOCHROME     0 /* 0 not monochrome, 1 monochrome sensor */
#define W9968CF_BRIGHTNESS     31000 /* from 0 to 65535 */
#define W9968CF_HUE            32768 /* from 0 to 65535 */
#define W9968CF_COLOUR         32768 /* from 0 to 65535 */
#define W9968CF_CONTRAST       50000 /* from 0 to 65535 */
#define W9968CF_WHITENESS      32768 /* from 0 to 65535 */

#define W9968CF_AUTOBRIGHT     0 /* 0 disable, 1 enable automatic brightness */
#define W9968CF_AUTOEXP        1 /* 0 disable, 1 enable automatic exposure */
#define W9968CF_LIGHTFREQ      50 /* light frequency. 50Hz (Europe) or 60Hz */
#define W9968CF_BANDINGFILTER  0 /* 0 disable, 1 enable banding filter */
#define W9968CF_BACKLIGHT      0 /* 0 or 1, 1=object is lit from behind */
#define W9968CF_MIRROR         0 /* 0 or 1 [don't] reverse image horizontally*/

#define W9968CF_CLOCKDIV         -1 /* -1 = automatic clock divisor */
#define W9968CF_DEF_CLOCKDIVISOR  0 /* default sensor clock divisor value */


/****************************************************************************
 * Globals                                                                  *
 ****************************************************************************/

#define W9968CF_MODULE_NAME     "V4L driver for W996[87]CF JPEG USB " \
				"Dual Mode Camera Chip"
#define W9968CF_MODULE_VERSION  "1:1.34-basic"
#define W9968CF_MODULE_AUTHOR   "(C) 2002-2004 Luca Risolia"
#define W9968CF_AUTHOR_EMAIL    "<luca.risolia@studio.unibo.it>"
#define W9968CF_MODULE_LICENSE  "GPL"

static const struct usb_device_id winbond_id_table[] = {
	{
		/* Creative Labs Video Blaster WebCam Go Plus */
		USB_DEVICE(0x041e, 0x4003),
		.driver_info = (unsigned long)"w9968cf",
	},
	{
		/* Generic W996[87]CF JPEG USB Dual Mode Camera */
		USB_DEVICE(0x1046, 0x9967),
		.driver_info = (unsigned long)"w9968cf",
	},
	{ } /* terminating entry */
};

/* W996[87]CF camera models, internal ids: */
enum w9968cf_model_id {
	W9968CF_MOD_GENERIC = 1, /* Generic W996[87]CF based device */
	W9968CF_MOD_CLVBWGP = 11,/*Creative Labs Video Blaster WebCam Go Plus*/
	W9968CF_MOD_ADPVDMA = 21, /* Aroma Digi Pen VGA Dual Mode ADG-5000 */
	W9986CF_MOD_AAU = 31,     /* AVerMedia AVerTV USB */
	W9968CF_MOD_CLVBWG = 34,  /* Creative Labs Video Blaster WebCam Go */
	W9968CF_MOD_LL = 37,      /* Lebon LDC-035A */
	W9968CF_MOD_EEEMC = 40,   /* Ezonics EZ-802 EZMega Cam */
	W9968CF_MOD_OOE = 42,     /* OmniVision OV8610-EDE */
	W9968CF_MOD_ODPVDMPC = 43,/* OPCOM Digi Pen VGA Dual Mode Pen Camera */
	W9968CF_MOD_PDPII = 46,   /* Pretec Digi Pen-II */
	W9968CF_MOD_PDP480 = 49,  /* Pretec DigiPen-480 */
};

enum w9968cf_frame_status {
	F_READY,            /* finished grabbing & ready to be read/synced */
	F_GRABBING,         /* in the process of being grabbed into */
	F_ERROR,            /* something bad happened while processing */
	F_UNUSED            /* unused (no VIDIOCMCAPTURE) */
};

struct w9968cf_frame_t {
	void* buffer;
	unsigned long size;
	u32 length;
	int number;
	enum w9968cf_frame_status status;
	struct w9968cf_frame_t* next;
	u8 queued;
};

enum w9968cf_vpp_flag {
	VPP_NONE = 0x00,
	VPP_UPSCALE = 0x01,
	VPP_SWAP_YUV_BYTES = 0x02,
	VPP_DECOMPRESSION = 0x04,
	VPP_UYVY_TO_RGBX = 0x08,
};

/* Main device driver structure */
struct w9968cf_device {
	enum w9968cf_model_id id;   /* private device identifier */

	struct v4l2_device v4l2_dev;
	struct video_device* v4ldev; /* -> V4L structure */
	struct list_head v4llist;    /* entry of the list of V4L cameras */

	struct usb_device* usbdev;           /* -> main USB structure */
	struct urb* urb[W9968CF_URBS];       /* -> USB request block structs */
	void* transfer_buffer[W9968CF_URBS]; /* -> ISO transfer buffers */
	u16* control_buffer;                 /* -> buffer for control req.*/
	u16* data_buffer;                    /* -> data to send to the FSB */

	struct w9968cf_frame_t frame[W9968CF_MAX_BUFFERS];
	struct w9968cf_frame_t frame_tmp; /* temporary frame */
	struct w9968cf_frame_t frame_vpp; /* helper frame.*/
	struct w9968cf_frame_t* frame_current; /* -> frame being grabbed */
	struct w9968cf_frame_t* requested_frame[W9968CF_MAX_BUFFERS];

	u8 max_buffers,   /* number of requested buffers */
	   force_palette, /* yes=1/no=0 */
	   force_rgb,     /* read RGB instead of BGR, yes=1, no=0 */
	   double_buffer, /* hardware double buffering yes=1/no=0 */
	   clamping,      /* video data clamping yes=1/no=0 */
	   filter_type,   /* 0=disabled, 1=3 tap, 2=5 tap filter */
	   capture,       /* 0=disabled, 1=enabled */
	   largeview,     /* 0=disabled, 1=enabled */
	   decompression, /* 0=disabled, 1=forced, 2=allowed */
	   upscaling;     /* software image scaling, 0=enabled, 1=disabled */

	struct video_picture picture; /* current picture settings */
	struct video_window window;   /* current window settings */

	u16 hw_depth,    /* depth (used by the chip) */
	    hw_palette,  /* palette (used by the chip) */
	    hw_width,    /* width (used by the chip) */
	    hw_height,   /* height (used by the chip) */
	    hs_polarity, /* 0=negative sync pulse, 1=positive sync pulse */
	    vs_polarity, /* 0=negative sync pulse, 1=positive sync pulse */
	    start_cropx, /* pixels from HS inactive edge to 1st cropped pixel*/
	    start_cropy; /* pixels from VS inactive edge to 1st cropped pixel*/

	enum w9968cf_vpp_flag vpp_flag; /* post-processing routines in use */

	u8 nbuffers,      /* number of allocated frame buffers */
	   altsetting,    /* camera alternate setting */
	   disconnected,  /* flag: yes=1, no=0 */
	   misconfigured, /* flag: yes=1, no=0 */
	   users,         /* flag: number of users holding the device */
	   streaming;     /* flag: yes=1, no=0 */

	u8 sensor_initialized; /* flag: yes=1, no=0 */

	/* Determined by the image sensor type: */
	int sensor,       /* type of image sensor chip (CC_*) */
	    monochrome;   /* image sensor is (probably) monochrome */
	u16 maxwidth,     /* maximum width supported by the image sensor */
	    maxheight,    /* maximum height supported by the image sensor */
	    minwidth,     /* minimum width supported by the image sensor */
	    minheight;    /* minimum height supported by the image sensor */
	u8  auto_brt,     /* auto brightness enabled flag */
	    auto_exp,     /* auto exposure enabled flag */
	    backlight,    /* backlight exposure algorithm flag */
	    mirror,       /* image is reversed horizontally */
	    lightfreq,    /* power (lighting) frequency */
	    bandfilt;     /* banding filter enabled flag */
	s8  clockdiv;     /* clock divisor */

	/* I2C interface to kernel */
	struct i2c_adapter i2c_adapter;
	struct v4l2_subdev *sensor_sd;

	/* Locks */
	struct mutex dev_mutex,    /* for probe, disconnect,open and close */
			 fileop_mutex; /* for read and ioctl */
	spinlock_t urb_lock,   /* for submit_urb() and unlink_urb() */
		   flist_lock; /* for requested frame list accesses */
	wait_queue_head_t open, wait_queue;

	char command[16]; /* name of the program holding the device */
};

static inline struct w9968cf_device *to_cam(struct v4l2_device *v4l2_dev)
{
	return container_of(v4l2_dev, struct w9968cf_device, v4l2_dev);
}


/****************************************************************************
 * Macros for debugging                                                     *
 ****************************************************************************/

#undef DBG
#undef KDBG
#ifdef W9968CF_DEBUG
/* For device specific debugging messages */
#	define DBG(level, fmt, args...)                                       \
{                                                                             \
	if ( ((specific_debug) && (debug == (level))) ||                      \
	     ((!specific_debug) && (debug >= (level))) ) {                    \
		if ((level) == 1)                                             \
			v4l2_err(&cam->v4l2_dev, fmt "\n", ## args);          \
		else if ((level) == 2 || (level) == 3)                        \
			v4l2_info(&cam->v4l2_dev, fmt "\n", ## args);         \
		else if ((level) == 4)                                        \
			v4l2_warn(&cam->v4l2_dev, fmt "\n", ## args);         \
		else if ((level) >= 5)                                        \
			v4l2_info(&cam->v4l2_dev, "[%s:%d] " fmt "\n",        \
				 __func__, __LINE__ , ## args);               \
	}                                                                     \
}
/* For generic kernel (not device specific) messages */
#	define KDBG(level, fmt, args...)                                      \
{                                                                             \
	if ( ((specific_debug) && (debug == (level))) ||                      \
	     ((!specific_debug) && (debug >= (level))) ) {                    \
		if ((level) >= 1 && (level) <= 4)                             \
			pr_info("w9968cf: " fmt "\n", ## args);               \
		else if ((level) >= 5)                                        \
			pr_debug("w9968cf: [%s:%d] " fmt "\n", __func__,  \
				 __LINE__ , ## args);                         \
	}                                                                     \
}
#else
	/* Not debugging: nothing */
#	define DBG(level, fmt, args...) do {;} while(0);
#	define KDBG(level, fmt, args...) do {;} while(0);
#endif

#undef PDBG
#define PDBG(fmt, args...)                                                    \
v4l2_info(&cam->v4l2_dev, "[%s:%d] " fmt "\n", __func__, __LINE__ , ## args);

#undef PDBGG
#define PDBGG(fmt, args...) do {;} while(0); /* nothing: it's a placeholder */

#endif /* _W9968CF_H_ */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 /* OmniVision OV7610/OV7110 Camera Chip Support Code
 *
 * Copyright (c) 1999-2004 Mark McClelland <mark@alpha.dyndns.org>
 * http://alpha.dyndns.org/ov511/
 *
 * Color fixes by by Orion Sky Lawlor <olawlor@acm.org> (2/26/2000)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. NO WARRANTY OF ANY KIND is expressed or implied.
 */

#define DEBUG

#include <linux/slab.h>
#include "ovcamchip_priv.h"

/* Registers */
#define REG_GAIN		0x00	/* gain [5:0] */
#define REG_BLUE		0x01	/* blue channel balance */
#define REG_RED			0x02	/* red channel balance */
#define REG_SAT			0x03	/* saturation */
#define REG_CNT			0x05	/* Y contrast */
#define REG_BRT			0x06	/* Y brightness */
#define REG_BLUE_BIAS		0x0C	/* blue channel bias [5:0] */
#define REG_RED_BIAS		0x0D	/* red channel bias [5:0] */
#define REG_GAMMA_COEFF		0x0E	/* gamma settings */
#define REG_WB_RANGE		0x0F	/* AEC/ALC/S-AWB settings */
#define REG_EXP			0x10	/* manual exposure setting */
#define REG_CLOCK		0x11	/* polarity/clock prescaler */
#define REG_FIELD_DIVIDE	0x16	/* field interval/mode settings */
#define REG_HWIN_START		0x17	/* horizontal window start */
#define REG_HWIN_END		0x18	/* horizontal window end */
#define REG_VWIN_START		0x19	/* vertical window start */
#define REG_VWIN_END		0x1A	/* vertical window end */
#define REG_PIXEL_SHIFT   	0x1B	/* pixel shift */
#define REG_YOFFSET		0x21	/* Y channel offset */
#define REG_UOFFSET		0x22	/* U channel offset */
#define REG_ECW			0x24	/* exposure white level for AEC */
#define REG_ECB			0x25	/* exposure black level for AEC */
#define REG_FRAMERATE_H		0x2A	/* frame rate MSB + misc */
#define REG_FRAMERATE_L		0x2B	/* frame rate LSB */
#define REG_ALC			0x2C	/* Auto Level Control settings */
#define REG_VOFFSET		0x2E	/* V channel offset adjustment */
#define REG_ARRAY_BIAS		0x2F	/* array bias -- don't change */
#define REG_YGAMMA		0x33	/* misc gamma settings [7:6] */
#define REG_BIAS_ADJUST		0x34	/* misc bias settings */

/* Window parameters */
#define HWSBASE 0x38
#define HWEBASE 0x3a
#define VWSBASE 0x05
#define VWEBASE 0x05

struct ov7x10 {
	int auto_brt;
	int auto_exp;
	int bandfilt;
	int mirror;
};

/* Lawrence Glaister <lg@jfm.bc.ca> reports:
 *
 * Register 0x0f in the 7610 has the following effects:
 *
 * 0x85 (AEC method 1): Best overall, good contrast range
 * 0x45 (AEC method 2): Very overexposed
 * 0xa5 (spec sheet default): Ok, but the black level is
 *	shifted resulting in loss of contrast
 * 0x05 (old driver setting): very overexposed, too much
 *	contrast
 */
static struct ovcamchip_regvals regvals_init_7x10[] = {
	{ 0x10, 0xff },
	{ 0x16, 0x03 },
	{ 0x28, 0x24 },
	{ 0x2b, 0xac },
	{ 0x12, 0x00 },
	{ 0x38, 0x81 },
	{ 0x28, 0x24 },	/* 0c */
	{ 0x0f, 0x85 },	/* lg's setting */
	{ 0x15, 0x01 },
	{ 0x20, 0x1c },
	{ 0x23, 0x2a },
	{ 0x24, 0x10 },
	{ 0x25, 0x8a },
	{ 0x26, 0xa2 },
	{ 0x27, 0xc2 },
	{ 0x2a, 0x04 },
	{ 0x2c, 0xfe },
	{ 0x2d, 0x93 },
	{ 0x30, 0x71 },
	{ 0x31, 0x60 },
	{ 0x32, 0x26 },
	{ 0x33, 0x20 },
	{ 0x34, 0x48 },
	{ 0x12, 0x24 },
	{ 0x11, 0x01 },
	{ 0x0c, 0x24 },
	{ 0x0d, 0x24 },
	{ 0xff, 0xff },	/* END MARKER */
};

/* This initializes the OV7x10 camera chip and relevant variables. */
static int ov7x10_init(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x10 *s;
	int rc;

	DDEBUG(4, &c->dev, "entered");

	rc = ov_write_regvals(c, regvals_init_7x10);
	if (rc < 0)
		return rc;

	ov->spriv = s = kzalloc(sizeof *s, GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->auto_brt = 1;
	s->auto_exp = 1;

	return rc;
}

static int ov7x10_free(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);

	kfree(ov->spriv);
	return 0;
}

static int ov7x10_set_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x10 *s = ov->spriv;
	int rc;
	int v = ctl->value;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_write(c, REG_CNT, v >> 8);
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_write(c, REG_BRT, v >> 8);
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_write(c, REG_SAT, v >> 8);
		break;
	case OVCAMCHIP_CID_HUE:
		rc = ov_write(c, REG_RED, 0xFF - (v >> 8));
		if (rc < 0)
			goto out;

		rc = ov_write(c, REG_BLUE, v >> 8);
		break;
	case OVCAMCHIP_CID_EXP:
		rc = ov_write(c, REG_EXP, v);
		break;
	case OVCAMCHIP_CID_FREQ:
	{
		int sixty = (v == 60);

		rc = ov_write_mask(c, 0x2a, sixty?0x00:0x80, 0x80);
		if (rc < 0)
			goto out;

		rc = ov_write(c, 0x2b, sixty?0x00:0xac);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x13, 0x10, 0x10);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x13, 0x00, 0x10);
		break;
	}
	case OVCAMCHIP_CID_BANDFILT:
		rc = ov_write_mask(c, 0x2d, v?0x04:0x00, 0x04);
		s->bandfilt = v;
		break;
	case OVCAMCHIP_CID_AUTOBRIGHT:
		rc = ov_write_mask(c, 0x2d, v?0x10:0x00, 0x10);
		s->auto_brt = v;
		break;
	case OVCAMCHIP_CID_AUTOEXP:
		rc = ov_write_mask(c, 0x29, v?0x00:0x80, 0x80);
		s->auto_exp = v;
		break;
	case OVCAMCHIP_CID_MIRROR:
		rc = ov_write_mask(c, 0x12, v?0x40:0x00, 0x40);
		s->mirror = v;
		break;
	default:
		DDEBUG(2, &c->dev, "control not supported: %d", ctl->id);
		return -EPERM;
	}

out:
	DDEBUG(3, &c->dev, "id=%d, arg=%d, rc=%d", ctl->id, v, rc);
	return rc;
}

static int ov7x10_get_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x10 *s = ov->spriv;
	int rc = 0;
	unsigned char val = 0;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_read(c, REG_CNT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_read(c, REG_BRT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_read(c, REG_SAT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_HUE:
		rc = ov_read(c, REG_BLUE, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_EXP:
		rc = ov_read(c, REG_EXP, &val);
		ctl->value = val;
		break;
	case OVCAMCHIP_CID_BANDFILT:
		ctl->value = s->bandfilt;
		break;
	case OVCAMCHIP_CID_AUTOBRIGHT:
		ctl->value = s->auto_brt;
		break;
	case OVCAMCHIP_CID_AUTOEXP:
		ctl->value = s->auto_exp;
		break;
	case OVCAMCHIP_CID_MIRROR:
		ctl->value = s->mirror;
		break;
	default:
		DDEBUG(2, &c->dev, "control not supported: %d", ctl->id);
		return -EPERM;
	}

	DDEBUG(3, &c->dev, "id=%d, arg=%d, rc=%d", ctl->id, ctl->value, rc);
	return rc;
}

static int ov7x10_mode_init(struct i2c_client *c, struct ovcamchip_window *win)
{
	int qvga = win->quarter;

	/******** QVGA-specific regs ********/

	ov_write(c, 0x14, qvga?0x24:0x04);

	/******** Palette-specific regs ********/

	if (win->format == VIDEO_PALETTE_GREY) {
		ov_write_mask(c, 0x0e, 0x40, 0x40);
		ov_write_mask(c, 0x13, 0x20, 0x20);
	} else {
		ov_write_mask(c, 0x0e, 0x00, 0x40);
		ov_write_mask(c, 0x13, 0x00, 0x20);
	}

	/******** Clock programming ********/

	ov_write(c, 0x11, win->clockdiv);

	/******** Resolution-specific ********/

	if (win->width == 640 && win->height == 480)
		ov_write(c, 0x35, 0x9e);
	else
		ov_write(c, 0x35, 0x1e);

	return 0;
}

static int ov7x10_set_window(struct i2c_client *c, struct ovcamchip_window *win)
{
	int ret, hwscale, vwscale;

	ret = ov7x10_mode_init(c, win);
	if (ret < 0)
		return ret;

	if (win->quarter) {
		hwscale = 1;
		vwscale = 0;
	} else {
		hwscale = 2;
		vwscale = 1;
	}

	ov_write(c, 0x17, HWSBASE + (win->x >> hwscale));
	ov_write(c, 0x18, HWEBASE + ((win->x + win->width) >> hwscale));
	ov_write(c, 0x19, VWSBASE + (win->y >> vwscale));
	ov_write(c, 0x1a, VWEBASE + ((win->y + win->height) >> vwscale));

	return 0;
}

static int ov7x10_command(struct i2c_client *c, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case OVCAMCHIP_CMD_S_CTRL:
		return ov7x10_set_control(c, arg);
	case OVCAMCHIP_CMD_G_CTRL:
		return ov7x10_get_control(c, arg);
	case OVCAMCHIP_CMD_S_MODE:
		return ov7x10_set_window(c, arg);
	default:
		DDEBUG(2, &c->dev, "command not supported: %d", cmd);
		return -ENOIOCTLCMD;
	}
}

struct ovcamchip_ops ov7x10_ops = {
	.init    =	ov7x10_init,
	.free    =	ov7x10_free,
	.command =	ov7x10_command,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 M_ACCESS_READ | SNDRV_CTL_ELEM_ACCESS_VOLATILE, \
  .info = snd_rme9652_info_adat_sync, \
  .get = snd_rme9652_get_adat_sync, .private_value = xidx }

static int snd_rme9652_info_adat_sync(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	static char *texts[4] = {"No Lock", "Lock", "No Lock Sync", "Lock Sync"};

	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = 1;
	uinfo->value.enumerated.items = 4;
	if (uinfo->value.enumerated.item >= uinfo->value.enumerated.items)
		uinfo->value.enumerated.item = uinfo->value.enumerated.items - 1;
	strcpy(uinfo->value.enumerated.name, texts[uinfo->value.enumerated.item]);
	return 0;
}

static int snd_rme9652_get_adat_sync(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_rme9652 *rme9652 = snd_kcontrol_chip(kcontrol);
	unsigned int mask1, mask2, val;
	
	switch (kcontrol->private_value) {
	case 0: mask1 = RME9652_lock_0; mask2 = RME9652_sync_0; break;	
	case 1: mask1 = RME9652_lock_1; mask2 = RME9652_sync_1; break;	
	case 2: mask1 = RME9652_lock_2; mask2 = RME9652_sync_2; break;	
	default: return -EINVAL;
	}
	val = rme9652_read(rme9652, RME9652_status_register);
	ucontrol->value.enumerated.item[0] = (val & mask1) ? 1 : 0;
	ucontrol->value.enumerated.item[0] |= (val & mask2) ? 2 : 0;
	return 0;
}

#define RME9652_TC_VALID(xname, xindex) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, .index = xindex, \
  .access = SNDRV_CTL_ELEM_ACCESS_READ | SNDRV_CTL_ELEM_ACCESS_VOLATILE, \
  .info = snd_rme9652_info_tc_valid, \
  .get = snd_rme9652_get_tc_valid }

#define snd_rme9652_info_tc_valid	snd_ctl_boolean_mono_info

static int snd_rme9652_get_tc_valid(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_rme9652 *rme9652 = snd_kcontrol_chip(kcontrol);
	
	ucontrol->value.integer.value[0] = 
		(rme9652_read(rme9652, RME9652_status_register) & RME9652_tc_valid) ? 1 : 0;
	return 0;
}

#ifdef ALSA_HAS_STANDARD_WAY_OF_RETURNING_TIMECODE

/* FIXME: this routine needs a port to the new control API --jk */

static int snd_rme9652_get_tc_value(void *private_data,
				    snd_kswitch_t *kswitch,
				    snd_switch_t *uswitch)
{
	struct snd_rme9652 *s = (struct snd_rme9652 *) private_data;
	u32 value;
	int i;

	uswitch->type = SNDRV_SW_TYPE_DWORD;

	if ((rme9652_read(s, RME9652_status_register) &
	     RME9652_tc_valid) == 0) {
		uswitch->value.data32[0] = 0;
		return 0;
	}

	/* timecode request */

	rme9652_write(s, RME9652_time_code, 0);

	/* XXX bug alert: loop-based timing !!!! */

	for (i = 0; i < 50; i++) {
		if (!(rme9652_read(s, i * 4) & RME9652_tc_busy))
			break;
	}

	if (!(rme9652_read(s, i * 4) & RME9652_tc_busy)) {
		return -EIO;
	}

	value = 0;

	for (i = 0; i < 32; i++) {
		value >>= 1;

		if (rme9652_read(s, i * 4) & RME9652_tc_out)
			value |= 0x80000000;
	}

	if (value > 2 * 60 * 48000) {
		value -= 2 * 60 * 48000;
	} else {
		value = 0;
	}

	uswitch->value.data32[0] = value;

	return 0;
}

#endif				/* ALSA_HAS_STANDARD_WAY_OF_RETURNING_TIMECODE */

static struct snd_kcontrol_new snd_rme9652_controls[] = {
{
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,DEFAULT),
	.info =		snd_rme9652_control_spdif_info,
	.get =		snd_rme9652_control_spdif_get,
	.put =		snd_rme9652_control_spdif_put,
},
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READWRITE | SNDRV_CTL_ELEM_ACCESS_INACTIVE,
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,PCM_STREAM),
	.info =		snd_rme9652_control_spdif_stream_info,
	.get =		snd_rme9652_control_spdif_stream_get,
	.put =		snd_rme9652_control_spdif_stream_put,
},
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,CON_MASK),
	.info =		snd_rme9652_control_spdif_mask_info,
	.get =		snd_rme9652_control_spdif_mask_get,
	.private_value = IEC958_AES0_NONAUDIO |
			IEC958_AES0_PROFESSIONAL |
			IEC958_AES0_CON_EMPHASIS,	                                                                                      
},
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,PRO_MASK),
	.info =		snd_rme9652_control_spdif_mask_info,
	.get =		snd_rme9652_control_spdif_mask_get,
	.private_value = IEC958_AES0_NONAUDIO |
			IEC958_AES0_PROFESSIONAL |
			IEC958_AES0_PRO_EMPHASIS,
},
RME9652_SPDIF_IN("IEC958 Input Connector", 0),
RME9652_SPDIF_OUT("IEC958 Output also on ADAT1", 0),
RME9652_SYNC_MODE("Sync Mode", 0),
RME9652_SYNC_PREF("Preferred Sync Source", 0),
{
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Channels Thru",
	.index = 0,
	.info = snd_rme9652_info_thru,
	.get = snd_rme9652_get_thru,
	.put = snd_rme9652_put_thru,
},
RME9652_SPDIF_RATE("IEC958 Sample Rate", 0),
RME9652_ADAT_SYNC("ADAT1 Sync Check", 0, 0),
RME9652_ADAT_SYNC("ADAT2 Sync Check", 0, 1),
RME9652_TC_VALID("Timecode Valid", 0),
RME9652_PASSTHRU("Passthru", 0)
};

static struct snd_kcontrol_new snd_rme9652_adat3_check =
RME9652_ADAT_SYNC("ADAT3 Sync Check", 0, 2);

static struct snd_kcontrol_new snd_rme9652_adat1_input =
RME9652_ADAT1_IN("ADAT1 Input Source", 0);

static int snd_rme9652_create_controls(struct snd_card *card, struct snd_rme9652 *rme9652)
{
	unsigned int idx;
	int err;
	struct snd_kcontrol *kctl;

	for (idx = 0; idx < ARRAY_SIZE(snd_rme9652_controls); idx++) {
		if ((err = snd_ctl_add(card, kctl = snd_ctl_new1(&snd_rme9652_controls[idx], rme9652))) < 0)
			return err;
		if (idx == 1)	/* IEC958 (S/PDIF) Stream */
			rme9652->spdif_ctl = kctl;
	}

	if (rme9652->ss_channels == RME9652_NCHANNELS)
		if ((err = snd_ctl_add(card, kctl = snd_ctl_new1(&snd_rme9652_adat3_check, rme9652))) < 0)
			return err;

	if (rme9652->hw_rev >= 15)
		if ((err = snd_ctl_add(card, kctl = snd_ctl_new1(&snd_rme9652_adat1_input, rme9652))) < 0)
			return err;

	return 0;
}

/*------------------------------------------------------------
   /proc interface 
 ------------------------------------------------------------*/

static void
snd_rme9652_proc_read(struct snd_info_entry *entry, struct snd_info_buffer *buffer)
{
	struct snd_rme9652 *rme9652 = (struct snd_rme9652 *) entry->private_data;
	u32 thru_bits = rme9652->thru_bits;
	int show_auto_sync_source = 0;
	int i;
	unsigned int status;
	int x;

	status = rme9652_read(rme9652, RME9652_status_register);

	snd_iprintf(buffer, "%s (Card #%d)\n", rme9652->card_name, rme9652->card->number + 1);
	snd_iprintf(buffer, "Buffers: capture %p playback %p\n",
		    rme9652->capture_buffer, rme9652->playback_buffer);
	snd_iprintf(buffer, "IRQ: %d Registers bus: 0x%lx VM: 0x%lx\n",
		    rme9652->irq, rme9652->port, (unsigned long)rme9652->iobase);
	snd_iprintf(buffer, "Control register: %x\n", rme9652->control_register);

	snd_iprintf(buffer, "\n");

	x = 1 << (6 + rme9652_decode_latency(rme9652->control_register & 
					     RME9652_latency));

	snd_iprintf(buffer, "Latency: %d samples (2 periods of %lu bytes)\n", 
		    x, (unsigned long) rme9652->period_bytes);
	snd_iprintf(buffer, "Hardware pointer (frames): %ld\n",
		    rme9652_hw_pointer(rme9652));
	snd_iprintf(buffer, "Passthru: %s\n",
		    rme9652->passthru ? "yes" : "no");

	if ((rme9652->control_register & (RME9652_Master | RME9652_wsel)) == 0) {
		snd_iprintf(buffer, "Clock mode: autosync\n");
		show_auto_sync_source = 1;
	} else if (rme9652->control_register & RME9652_wsel) {
		if (status & RME9652_wsel_rd) {
			snd_iprintf(buffer, "Clock mode: word clock\n");
		} else {
			snd_iprintf(buffer, "Clock mode: word clock (no signal)\n");
		}
	} else {
		snd_iprintf(buffer, "Clock mode: master\n");
	}

	if (show_auto_sync_source) {
		switch (rme9652->control_register & RME9652_SyncPref_Mask) {
		case RME9652_SyncPref_ADAT1:
			snd_iprintf(buffer, "Pref. sync source: ADAT1\n");
			break;
		case RME9652_SyncPref_ADAT2:
			snd_iprintf(buffer, "Pref. sync source: ADAT2\n");
			break;
		case RME9652_SyncPref_ADAT3:
			snd_iprintf(buffer, "Pref. sync source: ADAT3\n");
			break;
		case RME9652_SyncPref_SPDIF:
			snd_iprintf(buffer, "Pref. sync source: IEC958\n");
			break;
		default:
			snd_iprintf(buffer, "Pref. sync source: ???\n");
		}
	}

	if (rme9652->hw_rev >= 15)
		snd_iprintf(buffer, "\nADAT1 Input source: %s\n",
			    (rme9652->control_register & RME9652_ADAT1_INTERNAL) ?
			    "Internal" : "ADAT1 optical");

	snd_iprintf(buffer, "\n");

	switch (rme9652_decode_spdif_in(rme9652->control_register & 
					RME9652_inp)) {
	case RME9652_SPDIFIN_OPTICAL:
		snd_iprintf(buffer, "IEC958 input: ADAT1\n");
		break;
	case RME9652_SPDIFIN_COAXIAL:
		snd_iprintf(buffer, "IEC958 input: Coaxial\n");
		break;
	case RME9652_SPDIFIN_INTERN:
		snd_iprintf(buffer, "IEC958 input: Internal\n");
		break;
	default:
		snd_iprintf(buffer, "IEC958 input: ???\n");
		break;
	}

	if (rme9652->control_register & RME9652_opt_out) {
		snd_iprintf(buffer, "IEC958 output: Coaxial & ADAT1\n");
	} else {
		snd_iprintf(buffer, "IEC958 output: Coaxial only\n");
	}

	if (rme9652->control_register & RME9652_PRO) {
		snd_iprintf(buffer, "IEC958 quality: Professional\n");
	} else {
		snd_iprintf(buffer, "IEC958 quality: Consumer\n");
	}

	if (rme9652->control_register & RME9652_EMP) {
		snd_iprintf(buffer, "IEC958 emphasis: on\n");
	} else {
		snd_iprintf(buffer, "IEC958 emphasis: off\n");
	}

	if (rme9652->control_register & RME9652_Dolby) {
		snd_iprintf(buffer, "IEC958 Dolby: on\n");
	} else {
		snd_iprintf(buffer, "IEC958 Dolby: off\n");
	}

	i = rme9652_spdif_sample_rate(rme9652);

	if (i < 0) {
		snd_iprintf(buffer,
			    "IEC958 sample rate: error flag set\n");
	} else if (i == 0) {
		snd_iprintf(buffer, "IEC958 sample rate: undetermined\n");
	} else {
		snd_iprintf(buffer, "IEC958 sample rate: %d\n", i);
	}

	snd_iprintf(buffer, "\n");

	snd_iprintf(buffer, "ADAT Sample rate: %dHz\n",
		    rme9652_adat_sample_rate(rme9652));

	/* Sync Check */

	x = status & RME9652_sync_0;
	if (status & RME9652_lock_0) {
		snd_iprintf(buffer, "ADAT1: %s\n", x ? "Sync" : "Lock");
	} else {
		snd_iprintf(buffer, "ADAT1: No Lock\n");
	}

	x = status & RME9652_sync_1;
	if (status & RME9652_lock_1) {
		snd_iprintf(buffer, "ADAT2: %s\n", x ? "Sync" : "Lock");
	} else {
		snd_iprintf(buffer, "ADAT2: No Lock\n");
	}

	x = status & RME9652_sync_2;
	if (status & RME9652_lock_2) {
		snd_iprintf(buffer, "ADAT3: %s\n", x ? "Sync" : "Lock");
	} else {
		snd_iprintf(buffer, "ADAT3: No Lock\n");
	}

	snd_iprintf(buffer, "\n");

	snd_iprintf(buffer, "Timecode signal: %s\n",
		    (status & RME9652_tc_valid) ? "yes" : "no");

	/* thru modes */

	snd_iprintf(buffer, "Punch Status:\n\n");

	for (i = 0; i < rme9652->ss_channels; i++) {
		if (thru_bits & (1 << i)) {
			snd_iprintf(buffer, "%2d:  on ", i + 1);
		} else {
			snd_iprintf(buffer, "%2d: off ", i + 1);
		}

		if (((i + 1) % 8) == 0) {
			snd_iprintf(buffer, "\n");
		}
	}

	snd_iprintf(buffer, "\n");
}

static void __devinit snd_rme9652_proc_init(struct snd_rme9652 *rme9652)
{
	struct snd_info_entry *entry;

	if (! snd_card_proc_new(rme9652->card, "rme9652", &entry))
		snd_info_set_text_ops(entry, rme9652, snd_rme9652_proc_read);
}

static void snd_rme9652_free_buffers(struct snd_rme9652 *rme9652)
{
	snd_hammerfall_free_buffer(&rme9652->capture_dma_buf, rme9652->pci);
	snd_hammerfall_free_buffer(&rme9652->playback_dma_buf, rme9652->pci);
}

static int snd_rme9652_free(struct snd_rme9652 *rme9652)
{
	if (rme9652->irq >= 0)
		rme9652_stop(rme9652);
	snd_rme9652_free_buffers(rme9652);

	if (rme9652->irq >= 0)
		free_irq(rme9652->irq, (void *)rme9652);
	if (rme9652->iobase)
		iounmap(rme9652->iobase);
	if (rme9652->port)
		pci_release_regions(rme9652->pci);

	pci_disable_device(rme9652->pci);
	return 0;
}

static int __devinit snd_rme9652_initialize_memory(struct snd_rme9652 *rme9652)
{
	unsigned long pb_bus, cb_bus;

	if (snd_hammerfall_get_buffer(rme9652->pci, &rme9652->capture_dma_buf, RME9652_DMA_AREA_BYTES) < 0 ||
	    snd_hammerfall_get_buffer(rme9652->pci, &rme9652->playback_dma_buf, RME9652_DMA_AREA_BYTES) < 0) {
		if (rme9652->capture_dma_buf.area)
			snd_dma_free_pages(&rme9652->capture_dma_buf);
		printk(KERN_ERR "%s: no buffers available\n", rme9652->card_name);
		return -ENOMEM;
	}

	/* Align to bus-space 64K boundary */

	cb_bus = ALIGN(rme9652->capture_dma_buf.addr, 0x10000ul);
	pb_bus/*
 * Interface to the 93C46/56 serial EEPROM that is used to store BIOS
 * settings for the aic7xxx based adaptec SCSI controllers.  It can
 * also be used for 93C26 and 93C06 serial EEPROMS.
 *
 * Copyright (c) 1994, 1995, 2000 Justin T. Gibbs.
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
 * $Id: //depot/aic7xxx/aic7xxx/aic7xxx_93cx6.h#12 $
 *
 * $FreeBSD$
 */
#ifndef _AIC7XXX_93CX6_H_
#define _AIC7XXX_93CX6_H_

typedef enum {
	C46 = 6,
	C56_66 = 8
} seeprom_chip_t;

struct seeprom_descriptor {
	struct ahc_softc *sd_ahc;
	u_int sd_control_offset;
	u_int sd_status_offset;
	u_int sd_dataout_offset;
	seeprom_chip_t sd_chip;
	uint16_t sd_MS;
	uint16_t sd_RDY;
	uint16_t sd_CS;
	uint16_t sd_CK;
	uint16_t sd_DO;
	uint16_t sd_DI;
};

/*
 * This function will read count 16-bit words from the serial EEPROM and
 * return their value in buf.  The port address of the aic7xxx serial EEPROM
 * control register is passed in as offset.  The following parameters are
 * also passed in:
 *
 *   CS  - Chip select
 *   CK  - Clock
 *   DO  - Data out
 *   DI  - Data in
 *   RDY - SEEPROM ready
 *   MS  - Memory port mode select
 *
 *  A failed read attempt returns 0, and a successful read returns 1.
 */

#define	SEEPROM_INB(sd) \
	ahc_inb(sd->sd_ahc, sd->sd_control_offset)
#define	SEEPROM_OUTB(sd, value)					\
do {								\
	ahc_outb(sd->sd_ahc, sd->sd_control_offset, value);	\
	ahc_flush_device_writes(sd->sd_ahc);			\
} while(0)

#define	SEEPROM_STATUS_INB(sd) \
	ahc_inb(sd->sd_ahc, sd->sd_status_offset)
#define	SEEPROM_DATA_INB(sd) \
	ahc_inb(sd->sd_ahc, sd->sd_dataout_offset)

int ahc_read_seeprom(struct seeprom_descriptor *sd, uint16_t *buf,
		     u_int start_addr, u_int count);
int ahc_write_seeprom(struct seeprom_descriptor *sd, uint16_t *buf,
		      u_int start_addr, u_int count);
int ahc_verify_cksum(struct seeprom_config *sc);

#endif /* _AIC7XXX_93CX6_H_ */
                                                                                                                                                                                                                                                                                                                                                                                                                                           = rme9652_channel_buffer_location (rme9652,
						       substream->pstr->stream,
						       channel);
	if (snd_BUG_ON(!channel_buf))
		return -EIO;
	memset(channel_buf + pos * 4, 0, count * 4);
	return count;
}

static int snd_rme9652_reset(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_rme9652 *rme9652 = snd_pcm_substream_chip(substream);
	struct snd_pcm_substream *other;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		other = rme9652->capture_substream;
	else
		other = rme9652->playback_substream;
	if (rme9652->running)
		runtime->status->hw_ptr = rme9652_hw_pointer(rme9652);
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

static int snd_rme9652_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params)
{
	struct snd_rme9652 *rme9652 = snd_pcm_substream_chip(substream);
	int err;
	pid_t this_pid;
	pid_t other_pid;

	spin_lock_irq(&rme9652->lock);

	if (substream->pstr->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		rme9652->control_register &= ~(RME9652_PRO | RME9652_Dolby | RME9652_EMP);
		rme9652_write(rme9652, RME9652_control_register, rme9652->control_register |= rme9652->creg_spdif_stream);
		this_pid = rme9652->playback_pid;
		other_pid = rme9652->capture_pid;
	} else {
		this_pid = rme9652->capture_pid;
		other_pid = rme9652->playback_pid;
	}

	if ((other_pid > 0) && (this_pid != other_pid)) {

		/* The other stream is open, and not by the same
		   task as this one. Make sure that the parameters
		   that matter are the same.
		 */

		if ((int)params_rate(params) !=
		    rme9652_adat_sample_rate(rme9652)) {
			spin_unlock_irq(&rme9652->lock);
			_snd_pcm_hw_param_setempty(params, SNDRV_PCM_HW_PARAM_RATE);
			return -EBUSY;
		}

		if (params_period_size(params) != rme9652->period_bytes / 4) {
			spin_unlock_irq(&rme9652->lock);
			_snd_pcm_hw_param_setempty(params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE);
			return -EBUSY;
		}

		/* We're fine. */

		spin_unlock_irq(&rme9652->lock);
 		return 0;

	} else {
		spin_unlock_irq(&rme9652->lock);
	}

	/* how to make sure that the rate matches an externally-set one ?
	 */

	if ((err = rme9652_set_rate(rme9652, params_rate(params))) < 0) {
		_snd_pcm_hw_param_setempty(params, SNDRV_PCM_HW_PARAM_RATE);
		return err;
	}

	if ((err = rme9652_set_interrupt_interval(rme9652, params_period_size(params))) < 0) {
		_snd_pcm_hw_param_setempty(params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE);
		return err;
	}

	return 0;
}

static int snd_rme9652_channel_info(struct snd_pcm_substream *substream,
				    struct snd_pcm_channel_info *info)
{
	struct snd_rme9652 *rme9652 = snd_pcm_substream_chip(substream);
	int chn;

	if (snd_BUG_ON(info->channel >= RME9652_NCHANNELS))
		return -EINVAL;

	if ((chn = rme9652->channel_map[info->channel]) < 0) {
		return -EINVAL;
	}

	info->offset = chn * RME9652_CHANNEL_BUFFER_BYTES;
	info->first = 0;
	info->step = 32;
	return 0;
}

static int snd_rme9652_ioctl(struct snd_pcm_substream *substream,
			     unsigned int cmd, void *arg)
{
	switch (cmd) {
	case SNDRV_PCM_IOCTL1_RESET:
	{
		return snd_rme9652_reset(substream);
	}
	case SNDRV_PCM_IOCTL1_CHANNEL_INFO:
	{
		struct snd_pcm_channel_info *info = arg;
		return snd_rme9652_channel_info(substream, info);
	}
	default:
		break;
	}

	return snd_pcm_lib_ioctl(substream, cmd, arg);
}

static void rme9652_silence_playback(struct snd_rme9652 *rme9652)
{
	memset(rme9652->playback_buffer, 0, RME9652_DMA_AREA_BYTES);
}

static int snd_rme9652_trigger(struct snd_pcm_substream *substream,
			       int cmd)
{
	struct snd_rme9652 *rme9652 = snd_pcm_substream_chip(substream);
	struct snd_pcm_substream *other;
	int running;
	spin_lock(&rme9652->lock);
	running = rme9652->running;
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		running |= 1 << substream->stream;
		break;
	case SNDRV_PCM_TRIGGER_oid esp_flush_fifo(struct esp *esp)
{
	scsi_esp_cmd(esp, ESP_CMD_FLUSH);
	if (esp->rev == ESP236) {
		int lim = 1000;

		while (esp_read8(ESP_FFLAGS) & ESP_FF_FBYTES) {
			if (--lim == 0) {
				printk(KERN_ALERT PFX "esp%d: ESP_FF_BYTES "
				       "will not clear!\n",
				       esp->host->unique_id);
				break;
			}
			udelay(1);
		}
	}
}

static void hme_read_fifo(struct esp *esp)
{
	int fcnt = esp_read8(ESP_FFLAGS) & ESP_FF_FBYTES;
	int idx = 0;

	while (fcnt--) {
		esp->fifo[idx++] = esp_read8(ESP_FDATA);
		esp->fifo[idx++] = esp_read8(ESP_FDATA);
	}
	if (esp->sreg2 & ESP_STAT2_F1BYTE) {
		esp_write8(0, ESP_FDATA);
		esp->fifo[idx++] = esp_read8(ESP_FDATA);
		scsi_esp_cmd(esp, ESP_CMD_FLUSH);
	}
	esp->fifo_cnt = idx;
}

static void esp_set_all_config3(struct esp *esp, u8 val)
{
	int i;

	for (i = 0; i < ESP_MAX_TARGET; i++)
		esp->target[i].esp_config3 = val;
}

/* Reset the ESP chip, _not_ the SCSI bus. */
static void esp_reset_esp(struct esp *esp)
{
	u8 family_code, version;

	/* Now reset the ESP chip */
	scsi_esp_cmd(esp, ESP_CMD_RC);
	scsi_esp_cmd(esp, ESP_CMD_NULL | ESP_CMD_DMA);
	if (esp->rev == FAST)
		esp_write8(ESP_CONFIG2_FENAB, ESP_CFG2);
	scsi_esp_cmd(esp, ESP_CMD_NULL | ESP_CMD_DMA);

	/* This is the only point at which it is reliable to read
	 * the ID-code for a fast ESP chip variants.
	 */
	esp->max_period = ((35 * esp->ccycle) / 1000);
	if (esp->rev == FAST) {
		version = esp_read8(ESP_UID);
		family_code = (version & 0xf8) >> 3;
		if (family_code == 0x02)
			esp->rev = FAS236;
		else if (family_code == 0x0a)
			esp->rev = FASHME; /* Version is usually '5'. */
		else
			esp->rev = FAS100A;
		esp->min_period = ((4 * esp->ccycle) / 1000);
	} else {
		esp->min_period = ((5 * esp->ccycle) / 1000);
	}
	esp->max_period = (esp->max_period + 3)>>2;
	esp->min_period = (esp->min_period + 3)>>2;

	esp_write8(esp->config1, ESP_CFG1);
	switch (esp->rev) {
	case ESP100:
		/* nothing to do */
		break;

	case ESP100A:
		esp_write8(esp->config2, ESP_CFG2);
		break;

	case ESP236:
		/* Slow 236 */
		esp_write8(esp->config2, ESP_CFG2);
		esp->prev_cfg3 = esp->target[0].esp_config3;
		esp_write8(esp->prev_cfg3, ESP_CFG3);
		break;

	case FASHME:
		esp->config2 |= (ESP_CONFIG2_HME32 | ESP_CONFIG2_HMEFENAB);
		/* fallthrough... */

	case FAS236:
		/* Fast 236 or HME */
		esp_write8(esp->config2, ESP_CFG2);
		if (esp->rev == FASHME) {
			u8 cfg3 = esp->target[0].esp_config3;

			cfg3 |= ESP_CONFIG3_FCLOCK | ESP_CONFIG3_OBPUSH;
			if (esp->scsi_id >= 8)
				cfg3 |= ESP_CONFIG3_IDBIT3;
			esp_set_all_config3(esp, cfg3);
		} else {
			u32 cfg3 = esp->target[0].esp_config3;

			cfg3 |= ESP_CONFIG3_FCLK;
			esp_set_all_config3(esp, cfg3);
		}
		esp->prev_cfg3 = esp->target[0].esp_config3;
		esp_write8(esp->prev_cfg3, ESP_CFG3);
		if (esp->rev == FASHME) {
			esp->radelay = 80;
		} else {
			if (esp->flags & ESP_FLAG_DIFFERENTIAL)
				esp->radelay = 0;
			else
				esp->radelay = 96;
		}
		break;

	case FAS100A:
		/* Fast 100a */
		esp_write8(esp->config2, ESP_CFG2);
		esp_set_all_config3(esp,
				    (esp->target[0].esp_config3 |
				     ESP_CONFIG3_FCLOCK));
		esp->prev_cfg3 = esp->target[0].esp_config3;
		esp_write8(esp->prev_cfg3, ESP_CFG3);
		esp->radelay = 32;
		break;

	default:
		break;
	}

	/* Reload the configuration registers */
	esp_write8(esp->cfact, ESP_CFACT);

	esp->prev_stp = 0;
	esp_write8(esp->prev_stp, ESP_STP);

	esp->prev_soff = 0;
	esp_write8(esp->prev_soff, ESP_SOFF);

	esp_write8(esp->neg_defp, ESP_TIMEO);

	/* Eat any bitrot in the chip */
	esp_read8(ESP_INTRPT);
	udelay(100);
}

static void esp_map_dma(struct esp *esp, struct scsi_cmnd *cmd)
{
	struct esp_cmd_priv *spriv = ESP_CMD_PRIV(cmd);
	struct scatterlist *sg = scsi_sglist(cmd);
	int dir = cmd->sc_data_direction;
	int total, i;

	if (dir == DMA_NONE)
		return;

	spriv->u.num_sg = esp->ops->map_sg(esp, sg, scsi_sg_count(cmd), dir);
	spriv->cur_residue = sg_dma_len(sg);
	spriv->cur_sg = sg;

	total = 0;
	for (i = 0; i < spriv->u.num_sg; i++)
		total += sg_dma_len(&sg[i]);
	spriv->tot_residue = total;
}

static dma_addr_t esp_cur_dma_addr(struct esp_cmd_entry *ent,
				   struct scsi_cmnd *cmd)
{
	struct esp_cmd_priv *p = ESP_CMD_PRIV(cmd);

	if (ent->flags & ESP_CMD_FLAG_AUTOSENSE) {
		return ent->sense_dma +
			(ent->sense_ptr - cmd->sense_buffer);
	}

	return sg_dma_address(p->cur_sg) +
		(sg_dma_len(p->cur_sg) -
		 p->cur_residue);
}

static unsigned int esp_cur_dma_len(struct esp_cmd_entry *ent,
				    struct scsi_cmnd *cmd)
{
	struct esp_cmd_priv *p = ESP_CMD_PRIV(cmd);

	if (ent->flags & ESP_CMD_FLAG_AUTOSENSE) {
		return SCSI_SENSE_BUFFERSIZE -
			(ent->sense_ptr - cmd->sense_buffer);
	}
	return p->cur_residue;
}

static void esp_advance_dma(struct esp *esp, struct esp_cmd_entry *ent,
			    struct scsi_cmnd *cmd, unsigned int len)
{
	struct esp_cmd_priv *p = ESP_CMD_PRIV(cmd);

	if (ent->flags & ESP_CMD_FLAG_AUTOSENSE) {
		ent->sense_ptr += len;
		return;
	}

	p->cur_residue -= len;
	p->tot_residue -= len;
	if (p->cur_residue < 0 || p->tot_residue < 0) {
		printk(KERN_ERR PFX "esp%d: Data transfer overflow.\n",
		       esp->host->unique_id);
		printk(KERN_ERR PFX "esp%d: cur_residue[%d] tot_residue[%d] "
		       "len[%u]\n",
		       esp->host->unique_id,
		       p->cur_residue, p->tot_residue, len);
		p->cur_residue = 0;
		p->tot_residue = 0;
	}
	if (!p->cur_residue && p->tot_residue) {
		p->cur_sg++;
		p->cur_residue = sg_dma_len(p->cur_sg);
	}
}

static void esp_unmap_dma(struct esp *esp, struct scsi_cmnd *cmd)
{
	struct esp_cmd_priv *spriv = ESP_CMD_PRIV(cmd);
	int dir = cmd->sc_data_direction;

	if (dir == DMA_NONE)
		return;

	esp->ops->unmap_sg(esp, scsi_sglist(cmd), spriv->u.num_sg, dir);
}

static void esp_save_pointers(struct esp *esp, struct esp_cmd_entry *ent)
{
	struct scsi_cmnd *cmd = ent->cmd;
	struct esp_cmd_priv *spriv = ESP_CMD_PRIV(cmd);

	if (ent->flags & ESP_CMD_FLAG_AUTOSENSE) {
		ent->saved_sense_ptr = ent->sense_ptr;
		return;
	}
	ent->saved_cur_residue = spriv->cur_residue;
	ent->saved_cur_sg = spriv->cur_sg;
	ent->saved_tot_residue = spriv->tot_residue;
}

static void esp_restore_pointers(struct esp *esp, struct esp_cmd_entry *ent)
{
	struct scsi_cmnd *cmd = ent->cmd;
	struct esp_cmd_priv *spriv = ESP_CMD_PRIV(cmd);

	if (ent->flags & ESP_CMD_FLAG_AUTOSENSE) {
		ent->sense_ptr = ent->saved_sense_ptr;
		return;
	}
	spriv->cur_residue = ent->saved_cur_residue;
	spriv->cur_sg = ent->saved_cur_sg;
	spriv->tot_residue = ent->saved_tot_residue;
}

static void esp_check_command_len(struct esp *esp, struct scsi_cmnd *cmd)
{
	if (cmd->cmd_len == 6 ||
	    cmd->cmd_len == 10 ||
	    cmd->cmd_len == 12) {
		esp->flags &= ~ESP_FLAG_DOING_SLOWCMD;
	} else {
		esp->flags |= ESP_FLAG_DOING_SLOWCMD;
	}
}

static void esp_write_tgt_config3(struct esp *esp, int tgt)
{
	if (esp->rev > ESP100A) {
		u8 val = esp->target[tgt].esp_config3;

		if (val != esp->prev_cfg3) {
			esp->prev_cfg3 = val;
			esp_write8(val, ESP_CFG3);
		}
	}
}

static void esp_write_tgt_sync(struct esp *esp, int tgt)
{
	u8 off = esp->target[tgt].esp_offset;
	u8 per = esp->target[tgt].esp_period;

	if (off != esp->prev_soff) {
		esp->prev_soff = off;
		esp_write8(off, ESP_SOFF);
	}
	if (per != esp->prev_stp) {
		esp->prev_stp = per;
		esp_write8(per, ESP_STP);
	}
}

static u32 esp_dma_length_limit(struct esp *esp, u32 dma_addr, u32 dma_len)
{
	if (esp->rev == FASHME) {
		/* Arbitrary segment boundaries, 24-bit counts.  */
		if (dma_len > (1U << 24))
			dma_len = (1U << 24);
	} else {
		u32 base, end;

		/* ESP chip limits other variants by 16-bits of transfer
		 * count.  Actually on FAS100A and FAS236 we could get
		 * 24-bits of transfer count by enabling ESP_CONFIG2_FENAB
		 * in the ESP_CFG2 register but that causes other unwanted
		 * changes so we don't use it currently.
		 */
		if (dma_len > (1U << 16))
			dma_len = (1U << 16);

		/* All of the DMA variants hooked up to these chips
		 * cannot handle crossing a 24-bit address boundary.
		 */
		base = dma_addr & ((1U << 24) - 1U);
		end = base + dma_len;
		if (end > (1U << 24))
			end = (1U <<24);
		dma_len = end - base;
	}
	return dma_len;
}

static int esp_need_to_nego_wide(struct esp_target_data *tp)
{
	struct scsi_target *target = tp->starget;

	return spi_width(target) != tp->nego_goal_width;
}

static int esp_need_to_nego_sync(struct esp_target_data *tp)
{
	struct scsi_target *target = tp->starget;

	/* When offset is zero, period is "don't care".  */
	if (!spi_offset(target) && !tp->nego_goal_offset)
		return 0;

	if (spi_offset(target) == tp->nego_goal_offset &&
	    spi_period(target) == tp->nego_goal_period)
		return 0;

	return 1;
}

static int esp_alloc_lun_tag(struct esp_cmd_entry *ent,
			     struct esp_lun_data *lp)
{
	if (!ent->tag[0]) {
		/* Non-tagged, slot already taken?  */
		if (lp->non_tagged_cmd)
			return -EBUSY;

		if (lp->hold) {
			/* We are being held by active tagged
			 * commands.
			 */
			if (lp->num_tagged)
				return -EBUSY;

			/* Tagged commands completed, we can unplug
			 * the queue and run this untagged command.
			 */
			lp->hold = 0;
		} else if (lp->num_tagged) {
			/* Plug the queue until num_tagged decreases
			 * to zero in esp_free_lun_tag.
			 */
			lp->hold = 1;
			return -EBUSY;
		}

		lp->non_tagged_cmd = ent;
		return 0;
	} else {
		/* Tagged command, see if blocked by a
		 * non-tagged one.
		 */
		if (lp->non_tagged_cmd || lp->hold)
			return -EBUSY;
	}

	BUG_ON(lp->tagged_cmds[ent->tag[1]]);

	lp->tagged_cmds[ent->tag[1]] = ent;
	lp->num_tagged++;

	return 0;
}

static void esp_free_lun_tag(struct esp_cmd_entry *ent,
			     struct esp_lun_data *lp)
{
	if (ent->tag[0]) {
		BUG_ON(lp->tagged_cmds[ent->tag[1]] != ent);
		lp->tagged_cmds[ent->tag[1]] = NULL;
		lp->num_tagged--;
	} else {
		BUG_ON(lp->non_tagged_cmd != ent);
		lp->non_tagged_cmd = NULL;
	}
}

/* When a contingent allegiance conditon is created, we force feed a
 * REQUEST_SENSE command to the device to fetch the sense data.  I
 * tried many other schemes, relying on the scsi error handling layer
 * to send out the REQUEST_SENSE automatically, but this was difficult
 * to get right especially in the presence of applications like smartd
 * which use SG_IO to send out their own REQUEST_SENSE commands.
 */
static void esp_autosense(struct esp *esp, struct esp_cmd_entry *ent)
{
	struct scsi_cmnd *cmd = ent->cmd;
	struct scsi_device *dev = cmd->device;
	int tgt, lun;
	u8 *p, val;

	tgt = dev->id;
	lun = dev->lun;


	if (!ent->sense_ptr) {
		esp_log_autosense("esp%d: Doing auto-sense for "
				  "tgt[%d] lun[%d]\n",
				  esp->host->unique_id, tgt, lun);

		ent->sense_ptr = cmd->sense_buffer;
		ent->sense_dma = esp->ops->map_single(esp,
						      ent->sense_ptr,
						      SCSI_SENSE_BUFFERSIZE,
						      DMA_FROM_DEVICE);
	}
	ent->saved_sense_ptr = ent->sense_ptr;

	esp->active_cmd = ent;

	p = esp->command_block;
	esp->msg_out_len = 0;

	*p++ = IDENTIFY(0, lun);
	*p++ = REQUEST_SENSE;
	*p++ = ((dev->scsi_level <= SCSI_2) ?
		(lun << 5) : 0);
	*p++ = 0;
	*p++ = 0;
	*p++ = SCSI_SENSE_BUFFERSIZE;
	*p++ = 0;

	esp->select_state = ESP_SELECT_BASIC;

	val = tgt;
	if (esp->rev == FASHME)
		val |= ESP_BUSID_RESELID | ESP_BUSID_CTR32BIT;
	esp_write8(val, ESP_BUSID);

	esp_write_tgt_sync(esp, tgt);
	esp_write_tgt_config3(esp, tgt);

	val = (p - esp->command_block);

	if (esp->rev == FASHME)
		scsi_esp_cmd(esp, ESP_CMD_FLUSH);
	esp->ops->send_dma_cmd(esp, esp->command_block_dma,
			       val, 16, 0, ESP_CMD_DMA | ESP_CMD_SELA);
}

static struct esp_cmd_entry *find_and_prep_issuable_command(struct esp *esp)
{
	struct esp_cmd_entry *ent;

	list_for_each_entry(ent, &esp->queued_cmds, list) {
		struct scsi_cmnd *cmd = ent->cmd;
		struct scsi_device *dev = cmd->device;
		struct esp_lun_data *lp = dev->hostdata;

		if (ent->flags & ESP_CMD_FLAG_AUTOSENSE) {
			ent->tag[0] = 0;
			ent->tag[1] = 0;
			return ent;
		}

		if (!scsi_populate_tag_msg(cmd, &ent->tag[0])) {
			ent->tag[0] = 0;
			ent->tag[1] = 0;
		}

		if (esp_alloc_lun_tag(ent, lp) < 0)
			continue;

		return ent;
	}

	return NULL;
}

static void esp_maybe_execute_command(struct esp *esp)
{
	struct esp_target_data *tp;
	struct esp_lun_data *lp;
	struct scsi_device *dev;
	struct scsi_cmnd *cmd;
	struct esp_cmd_entry *ent;
	int tgt, lun, i;
	u32 val, start_cmd;
	u8 *p;

	if (esp->active_cmd ||
	    (esp->flags & ESP_FLAG_RESETTING))
		return;

	ent = find_and_prep_issuable_command(esp);
	if (!ent)
		return;

	if (ent->flags & ESP_CMD_FLAG_AUTOSENSE) {
		esp_autosense(esp, ent);
		return;
	}

	cmd = ent->cmd;
	dev = cmd->device;
	tgt = dev->id;
	lun = dev->lun;
	tp = &esp->target[tgt];
	lp = dev->hostdata;

	list_del(&ent->list);
	list_add(&ent->list, &esp->active_cmds);

	esp->active_cmd = ent;

	esp_map_dma(esp, cmd);
	esp_save_pointers(esp, ent);

	esp_check_command_len(esp, cmd);

	p = esp->command_block;

	esp->msg_out_len = 0;
	if (tp->flags & ESP_TGT_CHECK_NEGO) {
		/* Need to negotiate.  If the target is broken
		 * go for synchronous transfers and non-wide.
		 */
		if (tp->flags & ESP_TGT_BROKEN) {
			tp->flags &= ~ESP_TGT_DISCONNECT;
			tp->nego_goal_period = 0;
			tp->nego_goal_offset = 0;
			tp->nego_goal_width = 0;
			tp->nego_goal_tags = 0;
		}

		/* If the settings are not changing, skip this.  */
		if (spi_width(tp->starget) == tp->nego_goal_width &&
		    spi_period(tp->starget) == tp->nego_goal_period &&
		    spi_offset(tp->starget) == tp->nego_goal_offset) {
			tp->flags &= ~ESP_TGT_CHECK_NEGO;
			goto build_identify;
		}

		if (esp->rev == FASHME && esp_need_to_nego_wide(tp)) {
			esp->msg_out_len =
				spi_populate_width_msg(&esp->msg_out[0],
						       (tp->nego_goal_width ?
							1 : 0));
			tp->flags |= ESP_TGT_NEGO_WIDE;
		} else if (esp_need_to_nego_sync(tp)) {
			esp->msg_out_len =
				spi_populate_sync_msg(&esp->msg_out[0],
						      tp->nego_goal_period,
						      tp->nego_goal_offset);
			tp->flags |= ESP_TGT_NEGO_SYNC;
		} else {
			tp->flags &= ~ESP_TGT_CHECK_NEGO;
		}

		/* Process it like a slow command.  */
		if (tp->flags & (ESP_TGT_NEGO_WIDE | ESP_TGT_NEGO_SYNC))
			esp->flags |= ESP_FLAG_DOING_SLOWCMD;
	}

build_identify:
	/* If we don't have a lun-data struct yet, we're probing
	 * so do not disconnect.  Also, do not disconnect unless
	 * we have a tag on this command.
	 */
	if (lp && (tp->flags & ESP_TGT_DISCONNECT) && ent->tag[0])
		*p++ = IDENTIFY(1, lun);
	else
		*p++ = IDENTIFY(0, lun);

	if (ent->tag[0] && esp->rev == ESP100) {
		/* ESP100 lacks select w/atn3 command, use select
		 * and stop instead.
		 */
		esp->flags |= ESP_FLAG_DOING_SLOWCMD;
	}

	if (!(esp->flags & ESP_FLAG_DOING_SLOWCMD)) {
		start_cmd = ESP_CMD_DMA | ESP_CMD_SELA;
		if 
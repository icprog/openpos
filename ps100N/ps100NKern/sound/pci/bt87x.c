INDX( 	                (   @   �                                                 �֍�:���/'
��X��:��>����        ~              
 e v x f r e g n . c                               N��:��:0'
��lG�:��[�C���� P      uJ              
 n s a c c e s s . c                               |��:��<�-'
��p��:���5X���� @      =8              
 r s c r e a t e . c                               |��:��<�-'
��p��:���5X���� @      =8              
 r s c r e a t e . c                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               A_LMT		(1 << 12)	/* limit audio data values */
#define CTL_DA_ES2		(1 << 13)	/* enable DDF stage 2 */
#define CTL_DA_SBR		(1 << 14)	/* samples rounded to 8 bits */
#define CTL_DA_DPM		(1 << 15)	/* data packet mode */
#define CTL_DA_LRD_SHIFT	      16	/* ALRCK delay */
#define CTL_DA_MLB		(1 << 21)	/* MSB/LSB format */
#define CTL_DA_LRI		(1 << 22)	/* left/right indication */
#define CTL_DA_SCE		(1 << 23)	/* sample clock edge */
#define CTL_A_SEL_STV		(0 << 24)	/* TV tuner audio input */
#define CTL_A_SEL_SFM		(1 << 24)	/* FM audio input */
#define CTL_A_SEL_SML		(2 << 24)	/* mic/line audio input */
#define CTL_A_SEL_SMXC		(3 << 24)	/* MUX bypass */
#define CTL_A_SEL_SHIFT		      24
#define CTL_A_SEL_MASK		(3 << 24)
#define CTL_A_PWRDN		(1 << 26)	/* analog audio power-down */
#define CTL_A_G2X		(1 << 27)	/* audio gain boost */
#define CTL_A_GAIN_SHIFT	      28	/* audio input gain */
#define CTL_A_GAIN_MASK		(0xf<<28)

/* RISC instruction opcodes */
#define RISC_WRITE	(0x1 << 28)	/* write FIFO data to memory at address */
#define RISC_WRITEC	(0x5 << 28)	/* write FIFO data to memory at current address */
#define RISC_SKIP	(0x2 << 28)	/* skip FIFO data */
#define RISC_JUMP	(0x7 << 28)	/* jump to address */
#define RISC_SYNC	(0x8 << 28)	/* synchronize with FIFO */

/* RISC instruction bits */
#define RISC_BYTES_ENABLE	(0xf << 12)	/* byte enable bits */
#define RISC_RESYNC		(  1 << 15)	/* disable FDSR errors */
#define RISC_SET_STATUS_SHIFT	        16	/* set status bits */
#define RISC_RESET_STATUS_SHIFT	        20	/* clear status bits */
#define RISC_IRQ		(  1 << 24)	/* interrupt */
#define RISC_EOL		(  1 << 26)	/* end of line */
#define RISC_SOL		(  1 << 27)	/* start of line */

/* SYNC status bits values */
#define RISC_SYNC_FM1	0x6
#define RISC_SYNC_VRO	0xc

#define ANALOG_CLOCK 1792000
#ifdef CONFIG_SND_BT87X_OVERCLOCK
#define CLOCK_DIV_MIN 1
#else
#define CLOCK_DIV_MIN 4
#endif
#define CLOCK_DIV_MAX 15

#define ERROR_INTERRUPTS (INT_FBUS | INT_FTRGT | INT_PPERR | \
			  INT_RIPERR | INT_PABORT | INT_OCERR)
#define MY_INTERRUPTS (INT_RISCI | ERROR_INTERRUPTS)

/* SYNC, one WRITE per line, one extra WRITE per page boundary, SYNC, JUMP */
#define MAX_RISC_SIZE ((1 + 255 + (PAGE_ALIGN(255 * 4092) / PAGE_SIZE - 1) + 1 + 1) * 8)

/* Cards with configuration information */
enum snd_bt87x_boardid {
	SND_BT87X_BOARD_UNKNOWN,
	SND_BT87X_BOARD_GENERIC,	/* both an & dig interfaces, 32kHz */
	SND_BT87X_BOARD_ANALOG,		/* board with no external A/D */
	SND_BT87X_BOARD_OSPREY2x0,
	SND_BT87X_BOARD_OSPREY440,
	SND_BT87X_BOARD_AVPHONE98,
};

/* Card configuration */
struct snd_bt87x_board {
	int dig_rate;		/* Digital input sampling rate */
	u32 digital_fmt;	/* Register settings for digital input */
	unsigned no_analog:1;	/* No analog input */
	unsigned no_digital:1;	/* No digital input */
};

static __devinitdata struct snd_bt87x_board snd_bt87x_boards[] = {
	[SND_BT87X_BOARD_UNKNOWN] = {
		.dig_rate = 32000, /* just a guess */
	},
	[SND_BT87X_BOARD_GENERIC] = {
		.dig_rate = 32000,
	},
	[SND_BT87X_BOARD_ANALOG] = {
		.no_digital = 1,
	},
	[SND_BT87X_BOARD_OSPREY2x0] = {
		.dig_rate = 44100,
		.digital_fmt = CTL_DA_LRI | (1 << CTL_DA_LRD_SHIFT),
	},
	[SND_BT87X_BOARD_OSPREY440] = {
		.dig_rate = 32000,
		.digital_fmt = CTL_DA_LRI | (1 << CTL_DA_LRD_SHIFT),
		.no_analog = 1,
	},
	[SND_BT87X_BOARD_AVPHONE98] = {
		.dig_rate = 48000,
	},
};

struct snd_bt87x {
	struct snd_card *card;
	struct pci_dev *pci;
	struct snd_bt87x_board board;

	void __iomem *mmio;
	int irq;

	spinlock_t reg_lock;
	unsigned long opened;
	struct snd_pcm_substream *substream;

	struct snd_dma_buffer dma_risc;
	unsigned int line_bytes;
	unsigned int lines;

	u32 reg_control;
	u32 interrupt_mask;

	int current_line;

	int pci_parity_errors;
};

enum { DEVICE_DIGITAL, DEVICE_ANALOG };

static inline u32 snd_bt87x_readl(struct snd_bt87x *chip, u32 reg)
{
	return readl(chip->mmio + reg);
}

static inline void snd_bt87x_writel(struct snd_bt87x *chip, u32 reg, u32 value)
{
	writel(value, chip->mmio + reg);
}

static int snd_bt87x_create_risc(struct snd_bt87x *chip, struct snd = speed;

	/* tell the driver to start working */
	if (host->driver->isoctl(iso, XMIT_INIT, 0))
		goto err;

	iso->flags |= HPSB_ISO_DRIVER_INIT;
	return iso;

      err:
	hpsb_iso_shutdown(iso);
	return NULL;
}

/**
 * hpsb_iso_recv_init - allocate the buffer and DMA context
 *
 * Note, if channel = -1, multi-channel receive is enabled.
 */
struct hpsb_iso *hpsb_iso_recv_init(struct hpsb_host *host,
				    unsigned int data_buf_size,
				    unsigned int buf_packets,
				    int channel,
				    int dma_mode,
				    int irq_interval,
				    void (*callback) (struct hpsb_iso *))
{
	struct hpsb_iso *iso = hpsb_iso_common_init(host, HPSB_ISO_RECV,
						    data_buf_size, buf_packets,
						    channel, dma_mode,
						    irq_interval, callback);
	if (!iso)
		return NULL;

	/* tell the driver to start working */
	if (host->driver->isoctl(iso, RECV_INIT, 0))
		goto err;

	iso->flags |= HPSB_ISO_DRIVER_INIT;
	return iso;

      err:
	hpsb_iso_shutdown(iso);
	return NULL;
}

/**
 * hpsb_iso_recv_listen_channel
 *
 * multi-channel only
 */
int hpsb_iso_recv_listen_channel(struct hpsb_iso *iso, unsigned char channel)
{
	if (iso->type != HPSB_ISO_RECV || iso->channel != -1 || channel >= 64)
		return -EINVAL;
	return iso->host->driver->isoctl(iso, RECV_LISTEN_CHANNEL, channel);
}

/**
 * hpsb_iso_recv_unlisten_channel
 *
 * multi-channel only
 */
int hpsb_iso_recv_unlisten_channel(struct hpsb_iso *iso, unsigned char channel)
{
	if (iso->type != HPSB_ISO_RECV || iso->channel != -1 || channel >= 64)
		return -EINVAL;
	return iso->host->driver->isoctl(iso, RECV_UNLISTEN_CHANNEL, channel);
}

/**
 * hpsb_iso_recv_set_channel_mask
 *
 * multi-channel only
 */
int hpsb_iso_recv_set_channel_mask(struct hpsb_iso *iso, u64 mask)
{
	if (iso->type != HPSB_ISO_RECV || iso->channel != -1)
		return -EINVAL;
	return iso->host->driver->isoctl(iso, RECV_SET_CHANNEL_MASK,
					 (unsigned long)&mask);
}

/**
 * hpsb_iso_recv_flush - check for arrival of new packets
 *
 * check for arrival of new packets immediately (even if irq_interval
 * has not yet been reached)
 */
int hpsb_iso_recv_flush(struct hpsb_iso *iso)
{
	if (iso->type != HPSB_ISO_RECV)
		return -EINVAL;
	return iso->host->driver->isoctl(iso, RECV_FLUSH, 0);
}

static int do_iso_xmit_start(struct hpsb_iso *iso, int cycle)
{
	int retval = iso->host->driver->isoctl(iso, XMIT_START, cycle);
	if (retval)
		return retval;

	iso->flags |= HPSB_ISO_DRIVER_STARTED;
	return retval;
}

/**
 * hpsb_iso_xmit_start - start DMA
 */
int hpsb_iso_xmit_start(struct hpsb_iso *iso, int cycle, int prebuffer)
{
	if (iso->type != HPSB_ISO_XMIT)
		return -1;

	if (iso->flags & HPSB_ISO_DRIVER_STARTED)
		return 0;

	if (cycle < -1)
		cycle = -1;
	else if (cycle >= 8000)
		cycle %= 8000;

	iso->xmit_cycle = cycle;

	if (prebuffer < 0)
		prebuffer = iso->buf_packets - 1;
	else if (prebuffer == 0)
		prebuffer = 1;

	if (prebuffer >= iso->buf_packets)
		prebuffer = iso->buf_packets - 1;

	iso->prebuffer = prebuffer;

	/* remember the starting cycle; DMA will commence from xmit_queue_packets()
	   once enough packets have been buffered */
	iso->start_cycle = cycle;

	return 0;
}

/**
 * hpsb_iso_recv_start - start DMA
 */
int hpsb_iso_recv_start(struct hpsb_iso *iso, int cycle, int tag_mask, int sync)
{
	int retval = 0;
	int isoctl_args[3];

	if (iso->type != HPSB_ISO_RECV)
		return -1;

	if (iso->flags & HPSB_ISO_DRIVER_STARTED)
		return 0;

	if (cycle < -1)
		cycle = -1;
	else if (cycle >= 8000)
		cycle %= 8000;

	isoctl_args[0] = cycle;

	if (tag_mask < 0)
		/* match all tags */
		tag_mask = 0xF;
	isoctl_args[1] = tag_mask;

	isoctl_args[2] = sync;

	retval =
	    iso->host->driver->isoctl(iso, RECV_START,
				      (unsigned long)&isoctl_args[0]);
	if (retval)
		return retval;

	iso->flags |= HPSB_ISO_DRIVER_STARTED;
	return retval;
}

/* check to make sure the user has not supplied bogus values of offset/len
 * that would cause the kernel to access memory outside the buffer */
static int hpsb_iso_check_offset_len(struct hpsb_iso *iso,
				     unsigned int offset, unsigned short len,
				     unsigned int *out_offset,
				     unsigned short *out_len)
{
	if (offset >= iso->buf_size)
		return -EFAULT;

	/* make sure the packet does not go beyond the end of the buffer */
	if (offset + len > iso->buf_size)
		return -EFAULT;

	/* check for wrap-around */
	if (offset + len < offset)
		return -EFAULT;

	/* now we can trust 'offset' and 'length' */
	*out_offset = offset;
	*out_len = len;

	return 0;
}

/**
 * hpsb_iso_xmit_queue_packet - queue a packet for transmission.
 *
 * @offset is relative to the beginning of the DMA buffer, where the packet's
 * data payload should already have been placed.
 */
int hpsb_iso_xmit_queue_packet(struct hpsb_iso *iso, u32 offset, u16 len,
			       u8 tag, u8 sy)
{
	struct hpsb_iso_packet_info *info;
	unsigned long flags;
	int rv;

	if (iso->type != HPSB_ISO_XMIT)
		return -EINVAL;

	/* is there space in the buffer? */
	if (iso->n_ready_packets <= 0) {
		return -EBUSY;
	}

	info = &iso->infos[iso->first_packet];

	/* check for bogus offset/length */
	if (hpsb_iso_check_offset_len
	    (iso, offset, len, &info->offset, &info->len))
		return -EFAULT;

	info->tag = tag;
	info->sy = sy;

	spin_lock_irqsave(&iso->lock, flags);

	rv = iso->host->driver->isoctl(iso, XMIT_QUEUE, (unsigned long)info);
	if (rv)
		goto out;

	/* increment cursors */
	iso->first_packet = (iso->first_packet + 1) % iso->buf_packets;
	iso->xmit_cycle = (iso->xmit_cycle + 1) % 8000;
	iso->n_ready_packets--;

	if (iso->prebuffer != 0) {
		iso->prebuffer--;
		if (iso->prebuffer <= 0) {
			iso->prebuffer = 0;
			rv = do_iso_xmit_start(iso, iso->start_cycle);
		}
	}

      out:
	spin_unlock_irqrestore(&iso->lock, flags);
	return rv;
}

/**
 * hpsb_iso_xmit_sync - wait until all queued packets have been transmitted
 */
int hpsb_iso_xmit_sync(struct hpsb_iso *iso)
{
	if (iso->type != HPSB_ISO_XMIT)
		return -EINVAL;

	return wait_event_interruptible(iso->waitq,
					hpsb_iso_n_ready(iso) ==
					iso->buf_packets);
}

/**
 * hpsb_iso_packet_sent
 *
 * Available to low-level drivers.
 *
 * Call after a packet has been transmitted to the bus (interrupt context is
 * OK).  @cycle is the _exact_ cycle the packet was sent on.  @error should be
 * non-zero if some sort of error occurred when sending the packet.
 */
void hpsb_iso_packet_sent(struct hpsb_iso *iso, int cycle, int error)
{
	unsigned long flags;
	spin_lock_irqsave(&iso->lock, flags);

	/* predict the cycle of the next packet to be queued */

	/* jump ahead by the number of packets that are already buffered */
	cycle += iso->buf_packets - iso->n_ready_packets;
	cycle %= 8000;

	iso->xmit_cycle = cycle;
	iso->n_ready_packets++;
	iso->pkt_dma = (iso->pkt_dma + 1) % iso->buf_packets;

	if (iso->n_ready_packets == iso->buf_packets || error != 0) {
		/* the buffer has run empty! */
		atomic_inc(&iso->overflows);
	}

	spin_unlock_irqrestore(&iso->lock, flags);
}

/**
 * hpsb_iso_packet_received
 *
 * Available to low-level drivers.
 *
 * Call after a packet has been received (interrupt context is OK).
 */
void hpsb_iso_packet_received(struct hpsb_iso *iso, u32 offset, u16 len,
			      u16 total_len, u16 cycle, u8 channel, u8 tag,
			      u8 sy)
{
	unsigned long flags;
	spin_lock_irqsave(&iso->lock, flags);

	if (iso->n_ready_packets == iso->buf_packets) {
		/* overflow! */
		atomic_inc(&iso->overflows);
		/* Record size of this discarded packet */
		iso->bytes_discarded += total_len;
	} else {
		struct hpsb_iso_packet_info *info = &iso->infos[iso->pkt_dma];
		info->offset = offset;
		info->len = len;
		info->total_len = total_len;
		info->cycle = cycle;
		info->channel = channel;
		info->tag = tag;
		info->sy = sy;

		iso->pkt_dma = (iso->pkt_dma + 1) % iso->buf_packets;
		iso->n_ready_packets++;
	}

	spin_unlock_irqrestore(&iso->lock, flags);
}

/**
 * hpsb_iso_recv_release_packets - release packets, reuse buffer
 *
 * @n_packets have been read out of the buffer, re-use the buffer space
 */
int hpsb_iso_recv_release_packets(struct hpsb_iso *iso, unsigned int n_packets)
{
	unsigned long flags;
	unsigned int i;
	int rv = 0;

	if (iso->type != HPSB_ISO_RECV)
		return -1;

	spin_lock_irqsave(&iso->lock, flags);
	for (i = 0; i < n_packets; i++) {
		rv = iso->host->driver->isoctl(iso, RECV_RELEASE,
					       (unsigned long)&iso->infos[iso->
									  first_packet]);
		if (rv)
			break;

		iso->first_packet = (iso->first_packet + 1) % iso->buf_packets;
		iso->n_ready_packets--;

		/* release memory from packets discarded when queue was full  */
		if (iso->n_ready_packets == 0) {	/* Release only after all prior packets handled */
			if (iso->bytes_discarded != 0) {
				struct hpsb_iso_packet_info inf;
				inf.total_len = iso->bytes_discarded;
				iso->host->driver->isoctl(iso, RECV_RELEASE,
							  (unsigned long)&inf);
				iso->bytes_discarded = 0;
			}
		}
	}
	spin_unlock_irqrestore(&iso->lock, flags);
	return rv;
}

/**
 * hpsb_iso_wake
 *
 * Available to low-level drivers.
 *
 * Call to wake waiting processes after buffer space has opened up.
 */
void hpsb_iso_wake(struct hpsb_iso *iso)
{
	wake_up_interruptible(&iso->waitq);

	if (iso->callback)
		iso->callback(iso);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              /*
 * drivers/media/video/omap24xxcam.c
 *
 * OMAP 2 camera block driver.
 *
 * Copyright (C) 2004 MontaVista Software, Inc.
 * Copyright (C) 2004 Texas Instruments.
 * Copyright (C) 2007-2008 Nokia Corporation.
 *
 * Contact: Sakari Ailus <sakari.ailus@nokia.com>
 *
 * Based on code from Andy Lowe <source@mvista.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/videodev2.h>
#include <linux/pci.h>		/* needed for videobufs */
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>

#include "omap24xxcam.h"

#define OMAP24XXCAM_VERSION KERNEL_VERSION(0, 0, 0)

#define RESET_TIMEOUT_NS 10000

static void omap24xxcam_reset(struct omap24xxcam_device *cam);
static int omap24xxcam_sensor_if_enable(struct omap24xxcam_device *cam);
static void omap24xxcam_device_unregister(struct v4l2_int_device *s);
static int omap24xxcam_remove(struct platform_device *pdev);

/* module parameters */
static int video_nr = -1;	/* video device minor (-1 ==> auto assign) */
/*
 * Maximum amount of memory to use for capture buffers.
 * Default is 4800KB, enough to double-buffer SXGA.
 */
static int capture_mem = 1280 * 960 * 2 * 2;

static struct v4l2_int_device omap24xxcam;

/*
 *
 * Clocks.
 *
 */

static void omap24xxcam_clock_put(struct omap24xxcam_device *cam)
{
	if (cam->ick != NULL && !IS_ERR(cam->ick))
		clk_put(cam->ick);
	if (cam->fck != NULL && !IS_ERR(cam->fck))
		clk_put(cam->fck);

	cam->ick = cam->fck = NULL;
}

static int omap24xxcam_clock_get(struct omap24xxcam_device *cam)
{
	int rval = 0;

	cam->fck = clk_get(cam->dev, "fck");
	if (IS_ERR(cam->fck)) {
		dev_err(cam->dev, "can't get camera fck");
		rval = PTR_ERR(cam->fck);
		omap24xxcam_clock_put(cam);
		return rval;
	}

	cam->ick = clk_get(cam->dev, "ick");
	if (IS_ERR(cam->ick)) {
		dev_err(cam->dev, "can't get camera ick");
		rval = PTR_ERR(cam->ick);
		omap24xxcam_clock_put(cam);
	}

	return rval;
}

static void omap24xxcam_clock_on(struct omap24xxcam_device *cam)
{
	clk_enable(cam->fck);
	clk_enable(cam->ick);
}

static void omap24xxcam_clock_off(struct omap24xxcam_device *cam)
{
	clk_disable(cam->fck);
	clk_disable(cam->ick);
}

/*
 *
 * Camera core
 *
 */

/*
 * Set xclk.
 *
 * To disable xclk, use value zero.
 */
static void omap24xxcam_core_xclk_set(const struct omap24xxcam_device *cam,
				      u32 xclk)
{
	if (xclk) {
		u32 divisor = CAM_MCLK / xclk;

		if (divisor == 1)
			omap24xxcam_reg_out(cam->mmio_base + CC_REG_OFFSET,
					    CC_CTRL_XCLK,
					    CC_CTRL_XCLK_DIV_BYPASS);
		else
			omap24xxcam_reg_out(cam->mmio_base + CC_REG_OFFSET,
					    CC_CTRL_XCLK, divisor);
	} else
		omap24xxcam_reg_out(cam->mmio_base + CC_REG_OFFSET,
				    CC_CTRL_XCLK, CC_CTRL_XCLK_DIV_STABLE_LOW);
}

static void omap24xxcam_core_hwinit(const struct omap24xxcam_device *cam)
{
	/*
	 * Setting the camera core AUTOIDLE bit causes problems with frame
	 * synchronization, so we will clear the AUTOIDLE bit instead.
	 */
	omap24xxcam_reg_out(cam->mmio_base + CC_REG_OFFSET, CC_SYSCONFIG,
			    CC_SYSCONFIG_AUTOIDLE);

	/* program the camera interface DMA packet size */
	omap24xxcam_reg_out(cam->mmio_base + CC_REG_OFFSET, CC_CTRL_DMA,
			    CC_CTRL_DMA_EN | (DMA_THRESHOLD / 4 - 1));

	/* enable camera core error interrupts */
	omap24xxcam_reg_out(cam->mmio_base + CC_REG_OFFSET, CC_IRQENABLE,
			    CC_IRQENABLE_FW_ERR_IRQ
			    | CC_IRQENABLE_FSC_ERR_IRQ
			    | CC_IRQENABLE_SSC_ERR_IRQ
			    | CC_IRQENABLE_FIFO_OF_IRQ);
}

/*
 * Enable the camera core.
 *
 * Data transfer to the camera DMA starts from next starting frame.
 */
static void omap24xxcam_core_enable(const struct omap24xxcam_device *cam)
{

	omap24xxcam_reg_out(cam->mmio_base + CC_REG_OFFSET, CC_CTRL,
			    cam->cc_ctrl);
}

/*
 * Disable camera core.
 *
 * The data transfer will be stopped immediately (CC_CTRL_CC_RST). The
 * core internal state machines will be reset. Use
 * CC_CTRL_CC_FRAME_TRIG instead if you want to transfer the current
 * frame completely.
 */
static void omap24xxcam_core_disable(const struct omap24xxcam_device *cam)
{
	omap24xxcam_reg_out(cam->mmio_base + CC_REG_OFFSET, CC_CTRL,
			    CC_CTRL_CC_RST);
}

/* Interrupt service routine for camera core interrupts. */
static void omap24xxcam_core_isr(struct omap24xxcam_device *cam)
{
	u32 cc_irqstatus;
	const u32 cc_irqstatus_err =
		CC_IRQSTATUS_FW_ERR_IRQ
		| CC_IRQSTATUS_FSC_ERR_IRQ
		| CC_IRQSTATUS_SSC_ERR_IRQ
		| CC_IRQSTATUS_FIFO_UF_IRQ
		| CC_IRQSTATUS_FIFO_OF_IRQ;

	cc_irqstatus = omap24xxcam_reg_in(cam->mmio_base + CC_REG_OFFSET,
					  CC_IRQSTATUS);
	omap24xxcam_reg_out(cam->mmio_base + CC_REG_OFFSET, CC_IRQSTATUS,
			    cc_irqstatus);

	if (cc_irqstatus & cc_irqstatus_err
	    && !atomic_read(&cam->in_reset)) {
		dev_dbg(cam->dev, "resetting camera, cc_irqstatus 0x%x\n",
			cc_irqstatus);
		omap24xxcam_reset(cam);
	}
}

/*
 *
 * videobuf_buffer handling.
 *
 * Memory for mmapped videobuf_buffers is not allocated
 * conventionally, but by several kmalloc allocations and then
 * creating the scatterlist on our own. User-space buffers are handled
 * normally.
 *
 */

/*
 * Free the memory-mapped buffer memory allocated for a
 * videobuf_buffer and the associated scatterlist.
 */
static void omap24xxcam_vbq_free_mmap_buffer(struct videobuf_buffer *vb)
{
	struct videobuf_dmabuf *dma = videobuf_to_dma(vb);
	size_t alloc_size;
	struct page *page;
	int i;

	if (dma->sglist == NULL)
		return;

	i = dma->sglen;
	while (i) {
		i--;
		alloc_size = sg_dma_len(&dma->sglist[i]);
		page = sg_page(&dma->sglist[i]);
		do {
			ClearPageReserved(page++);
		} while (alloc_size -= PAGE_SIZE);
		__free_pages(sg_page(&dma->sglist[i]),
			     get_order(sg_dma_len(&dma->sglist[i])));
	}

	kfree(dma->sglist);
	dma->sglist = NULL;
}

/* Release all memory related to the videobuf_queue. */
static void omap24xxcam_vbq_free_mmap_buffers(struct videobuf_queue *vbq)
{
	int i;

	mutex_lock(&vbq->vb_lock);

	for (i = 0; i < VIDEO_MAX_FRAME; i++) {
		if (NULL == vbq->bufs[i])
			continue;
		if (V4L2_MEMORY_MMAP != vbq->bufs[i]->memory)
			continue;
		vbq->ops->buf_release(vbq, vbq->bufs[i]);
		omap24xxcam_vbq_free_mmap_buffer(vbq->bufs[i]);
		kfree(vbq->bufs[i]);
		vbq->bufs[i] = NULL;
	}

	mutex_unlock(&vbq->vb_lock);

	videobuf_mmap_free(vbq);
}

/*
 * Allocate physically as contiguous as possible buffer for video
 * frame and allocate and build DMA scatter-gather list for it.
 */
static int omap24xxcam_vbq_alloc_mmap_buffer(struct videobuf_buffer *vb)
{
	unsigned int order;
	size_t alloc_size, size = vb->bsize; /* vb->bsize is page aligned */
	struct page *page;
	int max_pages, err = 0, i = 0;
	struct videobuf_dmabuf *dma = videobuf_to_dma(vb);

	/*
	 * allocate maximum size scatter-gather list. Note this is
	 * overhead. We may not use as many entries as we allocate
	 */
	max_pages = vb->bsize >> PAGE_SHIFT;
	dma->sglist = kcalloc(max_pages, sizeof(*dma->sglist), GFP_KERNEL);
	if (dma->sglist == NULL) {
		err = -ENOMEM;
		goto out;
	}

	while (size) {
		order = get_order(size);
		/*
		 * do not over-allocate even if we would get larger
		 * contiguous chunk that way
		 */
		if ((PAGE_SIZE << order) > size)
			order--;

		/* try to allocate as many contiguous pages as possible */
		page = alloc_pages(GFP_KERNEL | GFP_DMA, order);
		/* if allocation fails, try to allocate smaller amount */
		while (page == NULL) {
			order--;
			page = alloc_pages(GFP_KERNEL | GFP_DMA, order);
			id_ctl_new1(
				  &snd_bt87x_capture_volume, chip));
		if (err < 0)
			goto _error;
		err = snd_ctl_add(card, snd_ctl_new1(
				  &snd_bt87x_capture_boost, chip));
		if (err < 0)
			goto _error;
		err = snd_ctl_add(card, snd_ctl_new1(
				  &snd_bt87x_capture_source, chip));
		if (err < 0)
			goto _error;
	}
	snd_printk(KERN_INFO "bt87x%d: Using board %d, %sanalog, %sdigital "
		   "(rate %d Hz)\n", dev, boardid,
		   chip->board.no_analog ? "no " : "",
		   chip->board.no_digital ? "no " : "", chip->board.dig_rate);

	strcpy(card->driver, "Bt87x");
	sprintf(card->shortname, "Brooktree Bt%x", pci->device);
	sprintf(card->longname, "%s at %#llx, irq %i",
		card->shortname, (unsigned long long)pci_resource_start(pci, 0),
		chip->irq);
	strcpy(card->mixername, "Bt87x");

	err = snd_card_register(card);
	if (err < 0)
		goto _error;

	pci_set_drvdata(pci, card);
	++dev;
	return 0;

_error:
	snd_card_free(card);
	return err;
}

static void __devexit snd_bt87x_remove(struct pci_dev *pci)
{
	snd_card_free(pci_get_drvdata(pci));
	pci_set_drvdata(pci, NULL);
}

/* default entries for all Bt87x cards - it's not exported */
/* driver_data is set to 0 to call detection */
static struct pci_device_id snd_bt87x_default_ids[] __devinitdata = {
	BT_DEVICE(PCI_DEVICE_ID_BROOKTREE_878, PCI_ANY_ID, PCI_ANY_ID, UNKNOWN),
	BT_DEVICE(PCI_DEVICE_ID_BROOKTREE_879, PCI_ANY_ID, PCI_ANY_ID, UNKNOWN),
	{ }
};

static struct pci_driver driver = {
	.name = "Bt87x",
	.id_table = snd_bt87x_ids,
	.probe = snd_bt87x_probe,
	.remove = __devexit_p(snd_bt87x_remove),
};

static int __init alsa_card_bt87x_init(void)
{
	if (load_all)
		driver.id_table = snd_bt87x_default_ids;
	return pci_register_driver(&driver);
}

static void __exit alsa_card_bt87x_exit(void)
{
	pci_unregister_driver(&driver);
}

module_init(alsa_card_bt87x_init)
module_exit(alsa_card_bt87x_exit)

/*
 *  card-als4000.c - driver for Avance Logic ALS4000 based soundcards.
 *  Copyright (C) 2000 by Bart Hartgers <bart@etpmod.phys.tue.nl>,
 *			  Jaroslav Kysela <perex@perex.cz>
 *  Copyright (C) 2002, 2008 by Andreas Mohr <hw7oshyuv3001@sneakemail.com>
 *
 *  Framework borrowed from Massimo Piccioni's card-als100.c.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * NOTES
 *
 *  Since Avance does not provide any meaningful documentation, and I
 *  bought an ALS4000 based soundcard, I was forced to base this driver
 *  on reverse engineering.
 *
 *  Note: this is no longer true (thank you!):
 *  pretty verbose chip docu (ALS4000a.PDF) can be found on the ALSA web site.
 *  Page numbers stated anywhere below with the "SPECS_PAGE:" tag
 *  refer to: ALS4000a.PDF specs Ver 1.0, May 28th, 1998.
 *
 *  The ALS4000 seems to be the PCI-cousin of the ALS100. It contains an
 *  ALS100-like SB DSP/mixer, an OPL3 synth, a MPU401 and a gameport 
 *  interface. These subsystems can be mapped into ISA io-port space, 
 *  using the PCI-interface. In addition, the PCI-bit provides DMA and IRQ 
 *  services to the subsystems.
 * 
 * While ALS4000 is very similar to a SoundBlaster, the differences in
 * DMA and capturing require more changes to the SoundBlaster than
 * desirable, so I made this separate driver.
 * 
 * The ALS4000 can do real full duplex playback/capture.
 *
 * FMDAC:
 * - 0x4f -> port 0x14
 * - port 0x15 |= 1
 *
 * Enable/disable 3D sound:
 * - 0x50 -> port 0x14
 * - change bit 6 (0x40) of port 0x15
 *
 * Set QSound:
 * - 0xdb -> port 0x14
 * - set port 0x15:
 *   0x3e (mode 3), 0x3c (mode 2), 0x3a (mode 1), 0x38 (mode 0)
 *
 * Set KSound:
 * - value -> some port 0x0c0d
 *
 * ToDo:
 * - by default, don't enable legacy game and use PCI game I/O
 * - power management? (card can do voice wakeup according to datasheet!!)
 */

#include <asm/io.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/gameport.h>
#include <linux/moduleparam.h>
#include <linux/dma-mapping.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/mpu401.h>
#include <sound/opl3.h>
#include <sound/sb.h>
#include <sound/initval.h>

MODULE_AUTHOR("Bart Hartgers <bart@etpmod.phys.tue.nl>, Andreas Mohr");
MODULE_DESCRIPTION("Avance Logic ALS4000");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{Avance Logic,ALS4000}}");

#if defined(CONFIG_GAMEPORT) || (defined(MODULE) && defined(CONFIG_GAMEPORT_MODULE))
#define SUPPORT_JOYSTICK 1
#endif

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;	/* Index 0-MAX */
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;	/* ID for this card */
static int enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;	/* Enable this card */
#ifdef SUPPORT_JOYSTICK
static int joystick_port[SNDRV_CARDS];
#endif

module_param_array(index, int, NULL, 0444);
MODULE_PARM_DESC(index, "Index value for ALS4000 soundcard.");
module_param_array(id, charp, NULL, 0444);
MODULE_PARM_DESC(id, "ID string for ALS4000 soundcard.");
module_param_array(enable, bool, NULL, 0444);
MODULE_PARM_DESC(enable, "Enable ALS4000 soundcard.");
#ifdef SUPPORT_JOYSTICK
module_param_array(joystick_port, int, NULL, 0444);
MODULE_PARM_DESC(joystick_port, "Joystick port address for ALS4000 soundcard. (0 = disabled)");
#endif

struct snd_card_als4000 {
	/* most frequent access first */
	unsigned long iobase;
	struct pci_dev *pci;
	/*
 * public header file of the frontend drivers for mobile DVB-T demodulators
 * DiBcom 3000M-B and DiBcom 3000P/M-C (http://www.dibcom.fr/)
 *
 * Copyright (C) 2004-5 Patrick Boettcher (patrick.boettcher@desy.de)
 *
 * based on GPL code from DibCom, which has
 *
 * Copyright (C) 2004 Amaury Demol for DiBcom (ademol@dibcom.fr)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 * Acknowledgements
 *
 *  Amaury Demol (ademol@dibcom.fr) from DiBcom for providing specs and driver
 *  sources, on which this driver (and the dvb-dibusb) are based.
 *
 * see Documentation/dvb/README.dibusb for more information
 *
 */

#ifndef DIB3000_H
#define DIB3000_H

#include <linux/dvb/frontend.h>

struct dib3000_config
{
	/* the demodulator's i2c address */
	u8 demod_address;
};

struct dib_fe_xfer_ops
{
	/* pid and transfer handling is done in the demodulator */
	int (*pid_parse)(struct dvb_frontend *fe, int onoff);
	int (*fifo_ctrl)(struct dvb_frontend *fe, int onoff);
	int (*pid_ctrl)(struct dvb_frontend *fe, int index, int pid, int onoff);
	int (*tuner_pass_ctrl)(struct dvb_frontend *fe, int onoff, u8 pll_ctrl);
};

#if defined(CONFIG_DVB_DIB3000MB) || (defined(CONFIG_DVB_DIB3000MB_MODULE) && defined(MODULE))
extern struct dvb_frontend* dib3000mb_attach(const struct dib3000_config* config,
					     struct i2c_adapter* i2c, struct dib_fe_xfer_ops *xfer_ops);
#else
static inline struct dvb_frontend* dib3000mb_attach(const struct dib3000_config* config,
					     struct i2c_adapter* i2c, struct dib_fe_xfer_ops *xfer_ops)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif // CONFIG_DVB_DIB3000MB

#endif // DIB3000_H
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           K 25
svn:wc:ra_dav:version-url
V 72
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2
END
flexcop-hw-filter.c
K 25
svn:wc:ra_dav:version-url
V 92
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-hw-filter.c
END
flexcop-dma.c
K 25
svn:wc:ra_dav:version-url
V 86
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-dma.c
END
Kconfig
K 25
svn:wc:ra_dav:version-url
V 80
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/Kconfig
END
flexcop.c
K 25
svn:wc:ra_dav:version-url
V 82
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop.c
END
flexcop-sram.c
K 25
svn:wc:ra_dav:version-url
V 87
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-sram.c
END
flexcop_ibi_value_le.h
K 25
svn:wc:ra_dav:version-url
V 95
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop_ibi_value_le.h
END
flexcop.h
K 25
svn:wc:ra_dav:version-url
V 82
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop.h
END
flexcop-pci.c
K 25
svn:wc:ra_dav:version-url
V 86
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-pci.c
END
flexcop-i2c.c
K 25
svn:wc:ra_dav:version-url
V 86
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-i2c.c
END
flexcop-reg.h
K 25
svn:wc:ra_dav:version-url
V 86
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-reg.h
END
flexcop-fe-tuner.c
K 25
svn:wc:ra_dav:version-url
V 91
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-fe-tuner.c
END
flexcop-eeprom.c
K 25
svn:wc:ra_dav:version-url
V 89
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-eeprom.c
END
flexcop-usb.c
K 25
svn:wc:ra_dav:version-url
V 86
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-usb.c
END
flexcop-misc.c
K 25
svn:wc:ra_dav:version-url
V 87
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-misc.c
END
flexcop_ibi_value_be.h
K 25
svn:wc:ra_dav:version-url
V 95
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop_ibi_value_be.h
END
flexcop-common.h
K 25
svn:wc:ra_dav:version-url
V 89
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-common.h
END
flexcop-usb.h
K 25
svn:wc:ra_dav:version-url
V 86
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/flexcop-usb.h
END
Makefile
K 25
svn:wc:ra_dav:version-url
V 81
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/dvb/b2c2/Makefile
END
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 /*
 * adv7170 - adv7170, adv7171 video encoder driver version 0.0.1
 *
 * Copyright (C) 2002 Maxim Yevtyushkin <max@linuxmedialabs.com>
 *
 * Based on adv7176 driver by:
 *
 * Copyright (C) 1998 Dave Perks <dperks@ibm.net>
 * Copyright (C) 1999 Wolfgang Scherr <scherr@net4you.net>
 * Copyright (C) 2000 Serguei Miridonov <mirsev@cicese.mx>
 *    - some corrections for Pinnacle Systems Inc. DC10plus card.
 *
 * Changes by Ronald Bultje <rbultje@ronald.bitfreak.net>
 *    - moved over to linux>=2.4.x i2c protocol (1/1/2003)
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-i2c-drv.h>

MODULE_DESCRIPTION("Analog Devices ADV7170 video encoder driver");
MODULE_AUTHOR("Maxim Yevtyushkin");
MODULE_LICENSE("GPL");


static int debug;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0-1)");

/* ----------------------------------------------------------------------- */

struct adv7170 {
	struct v4l2_subdev sd;
	unsigned char reg[128];

	v4l2_std_id norm;
	int input;
};

static inline struct adv7170 *to_adv7170(struct v4l2_subdev *sd)
{
	return container_of(sd, struct adv7170, sd);
}

static char *inputs[] = { "pass_through", "play_back" };

/* ----------------------------------------------------------------------- */

static inline int adv7170_write(struct v4l2_subdev *sd, u8 reg, u8 value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct adv7170 *encoder = to_adv7170(sd);

	encoder->reg[reg] = value;
	return i2c_smbus_write_byte_data(client, reg, value);
}

static inline int adv7170_read(struct v4l2_subdev *sd, u8 reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return i2c_smbus_read_byte_data(client, reg);
}

static int adv7170_write_block(struct v4l2_subdev *sd,
		     const u8 *data, unsigned int len)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct adv7170 *encoder = to_adv7170(sd);
	int ret = -1;
	u8 reg;

	/* the adv7170 has an autoincrement function, use it if
	 * the adapter understands raw I2C */
	if (i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		/* do raw I2C, not smbus compatible */
		u8 block_data[32];
		int block_len;

		while (len >= 2) {
			block_len = 0;
			block_data[block_len++] = reg = data[0];
			do {
				block_data[block_len++] =
				    encoder->reg[reg++] = data[1];
				len -= 2;
				data += 2;
			} while (len >= 2 && data[0] == reg && block_len < 32);
			ret = i2c_master_send(client, block_data, block_len);
			if (ret < 0)
				break;
		}
	} else {
		/* do some slow I2C emulation kind of thing */
		while (len >= 2) {
			reg = *data++;
			ret = adv7170_write(sd, reg, *data++);
			if (ret < 0)
				break;
			len -= 2;
		}
	}
	return ret;
}

/* ----------------------------------------------------------------------- */

#define TR0MODE     0x4c
#define TR0RST	    0x80

#define TR1CAPT	    0x00
#define TR1PLAY	    0x00

static const unsigned char init_NTSC[] = {
	0x00, 0x10,		/* MR0 */
	0x01, 0x20,		/* MR1 */
	0x02, 0x0e,		/* MR2 RTC control: bits 2 and 1 */
	0x03, 0x80,		/* MR3 */
	0x04, 0x30,		/* MR4 */
	0x05, 0x00,		/* Reserved */
	0x06, 0x00,		/* Reserved */
	0x07, TR0MODE,		/* TM0 */
	0x08, TR1CAPT,		/* TM1 */
	0x09, 0x16,		/* Fsc0 */
	0x0a, 0x7c,		/* Fsc1 */
	0x0b, 0xf0,		/* Fsc2 */
	0x0c, 0x21,		/* Fsc3 */
	0x0d, 0x00,		/* Subcarrier Phase */
	0x0e, 0x00,		/* Closed Capt. Ext 0 */
	0x0f, 0x00,		/* Closed Capt. Ext 1 */
	0x10, 0x00,		/* Closed Capt. 0 */
	0x11, 0x00,		/* Closed Capt. 1 */
	0x12, 0x00,		/* Pedestal Ctl 0 */
	0x13, 0x00,		/* Pedestal Ctl 1 */
	0x14, 0x00,		/* Pedestal Ctl 2 */
	0x15, 0x00,		/* Pedestal Ctl 3 */
	0x16, 0x00,		/* CGMS_WSS_0 */
	0x17, 0x00,		/* CGMS_WSS_1 */
	0x18, 0x00,		/* CGMS_WSS_2 */
	0x19, 0x00,		/* Teletext Ctl */
};

static const unsigned char init_PAL[] = {
	0x00, 0x71,		/* MR0 */
	0x01, 0x20,		/* MR1 */
	0x02, 0x0e,		/* MR2 RTC control: bits 2 and 1 */
	0x03, 0x80,		/* MR3 */
	0x04, 0x30,		/* MR4 */
	0x05, 0x00,		/* Reserved */
	0x06, 0x00,		/* Reserved */
	0x07, TR0MODE,		/* TM0 */
	0x08, TR1CAPT,		/* TM1 */
	0x09, 0xcb,		/* Fsc0 */
	0x0a, 0x8a,		/* Fsc1 */
	0x0b, 0x09,		/* Fsc2 */
	0x0c, 0x2a,		/* Fsc3 */
	0x0d, 0x00,		/* Subcarrier Phase */
	0x0e, 0x00,		/* Closed Capt. Ext 0 */
	0x0f, 0x00,		/* Closed Capt. Ext 1 */
	0x10, 0x00,		/* Closed Capt. 0 */
	0x11, 0x00,		/* Closed Capt. 1 */
	0x12, 0x00,		/* Pedestal Ctl 0 */
	0x13, 0x00,		/* Pedestal Ctl 1 */
	0x14, 0x00,		/* Pedestal Ctl 2 */
	0x15, 0x00,		/* Pedestal Ctl 3 */
	0x16, 0x00,		/* CGMS_WSS_0 */
	0x17, 0x00,		/* CGMS_WSS_1 */
	0x18, 0x00,		/* CGMS_WSS_2 */
	0x19, 0x00,		/* Teletext Ctl */
};


static int adv7170_s_std_output(struct v4l2_subdev *sd, v4l2_std_id std)
{
	struct adv7170 *encoder = to_adv7170(sd);

	v4l2_dbg(1, debug, sd, "set norm %llx\n", (unsigned long long)std);

	if (std & V4L2_STD_NTSC) {
		adv7170_write_block(sd, init_NTSC, sizeof(init_NTSC));
		if (encoder->input == 0)
			adv7170_write(sd, 0x02, 0x0e);	/* Enable genlock */
		adv7170_write(sd, 0x07, TR0MODE | TR0RST);
		adv7170_write(sd, 0x07, TR0MODE);
	} else if (std & V4L2_STD_PAL) {
		adv7170_write_block(sd, init_PAL, sizeof(init_PAL));
		if (encoder->input == 0)
			adv7170_write(sd, 0x02, 0x0e);	/* Enable genlock */
		adv7170_write(sd, 0x07, TR0MODE | TR0RST);
		adv7170_write(sd, 0x07, TR0MODE);
	} else {
		v4l2_dbg(1, debug, sd, "illegal norm: %llx\n",
				(unsigned long long)std);
		return -EINVAL;
	}
	v4l2_dbg(1, debug, sd, "switched to %llx\n", (unsigned long long)std);
	encoder->norm = std;
	return 0;
}

static int adv7170_s_routing(struct v4l2_subdev *sd,
			     u32 input, u32 output, u32 config)
{
	struct adv7170 *encoder = to_adv7170(sd);

	/* RJ: input = 0: input is from decoder
	   input = 1: input is from ZR36060
	   input = 2: color bar */

	v4l2_dbg(1, debug, sd, "set input from %s\n",
			input == 0 ? "decoder" : "ZR36060");

	switch (input) {
	case 0:
		adv7170_write(sd, 0x01, 0x20);
		adv7170_write(sd, 0x08, TR1CAPT);	/* TR1 */
		adv7170_write(sd, 0x02, 0x0e);	/* Enable genlock */
		adv7170_write(sd, 0x07, TR0MODE | TR0RST);
		adv7170_write(sd, 0x07, TR0MODE);
		/* udelay(10); */
		break;

	case 1:
		adv7170_write(sd, 0x01, 0x00);
		adv7170_write(sd, 0x08, TR1PLAY);	/* TR1 */
		adv7170_write(sd, 0x02, 0x08);
		adv7170_write(sd, 0x07, TR0MODE | TR0RST);
		adv7170_write(sd, 0x07, TR0MODE);
		/* udelay(10); */
		break;

	default:
		v4l2_dbg(1, debug, sd, "illegal input: %d\n", input);
		return -EINVAL;
	}
	v4l2_dbg(1, debug, sd, "switched to %s\n", inputs[input]);
	encoder->input = input;
	return 0;
}

static int adv7170_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_ADV7170, 0);
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops adv7170_core_ops = {
	.g_chip_ident = adv7170_g_chip_ident,
};

static const struct v4l2_subdev_video_ops adv7170_video_ops = {
	.s_std_output = adv7170_s_std_output,
	.s_routing = adv7170_s_routing,
};

static const struct v4l2_subdev_ops adv7170_ops = {
	.core = &adv7170_core_ops,
	.video = &adv7170_video_ops,
};

/* ----------------------------------------------------------------------- */

static int adv7170_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct adv7170 *encoder;
	struct v4l2_subdev *sd;
	int i;

	/* Check if the adapter supports the needed features */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	v4l_info(client, "chip found @ 0x%x (%s)\n",
			client->addr << 1, client->adapter->name);

	encoder = kzalloc(sizeof(struct adv7170), GFP_KERNEL);
	if (encoder == NULL)
		return -ENOMEM;
	sd = &encoder->sd;
	v4l2_i2c_subdev_init(sd, client, &adv7170_ops);
	encoder->norm = V4L2_STD_NTSC;
	encoder->input = 0;

	i = adv7170_write_block(sd, init_NTSC, sizeof(init_NTSC));
	if (i >= 0) {
		i = adv7170_write(sd, 0x07, TR0MODE | TR0RST);
		i = adv7170_write(sd, 0x07, TR0MODE);
		i = adv7170_read(sd, 0x12);
		v4l2_dbg(1, debug, sd, "revision %d\n", i & 1);
	}
	if (i < 0)
		v4l2_dbg(1, debug, sd, "init error 0x%x\n", i);
	return 0;
}

static int adv7170_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	kfree(to_adv7170(sd));
	return 0;
}

/* ----------------------------------------------------------------------- */

static const struct i2c_device_id adv7170_id[] = {
	{ "adv7170", 0 },
	{ "adv7171", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, adv7170_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "adv7170",
	.probe = adv7170_probe,
	.remove = adv7170_remove,
	.id_table = adv7170_id,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   MISC_CTRL,
					ALS4K_GCR8C_IRQ_MASK_CTRL_ENABLE);

	/* SPECS_PAGE: 39 */
	for (i = ALS4K_GCR91_DMA0_ADDR; i <= ALS4K_GCR96_DMA3_MODE_COUNT; ++i)
		snd_als4k_gcr_write(chip, i, 0);
	
	snd_als4k_gcr_write(chip, ALS4K_GCR99_DMA_EMULATION_CTRL,
		snd_als4k_gcr_read(chip, ALS4K_GCR99_DMA_EMULATION_CTRL));
	spin_unlock_irq(&chip->reg_lock);
}

#ifdef SUPPORT_JOYSTICK
static int __devinit snd_als4000_create_gameport(struct snd_card_als4000 *acard, int dev)
{
	struct gameport *gp;
	struct resource *r;
	int io_port;

	if (joystick_port[dev] == 0)
		return -ENODEV;

	if (joystick_port[dev] == 1) { /* auto-detect */
		for (io_port = 0x200; io_port <= 0x218; io_port += 8) {
			r = request_region(io_port, 8, "ALS4000 gameport");
			if (r)
				break;
		}
	} else {
		io_port = joystick_port[dev];
		r = request_region(io_port, 8, "ALS4000 gameport");
	}

	if (!r) {
		printk(KERN_WARNING "als4000: cannot reserve joystick ports\n");
		return -EBUSY;
	}

	acard->gameport = gp = gameport_allocate_port();
	if (!gp) {
		printk(KERN_ERR "als4000: cannot allocate memory for gameport\n");
		release_and_free_resource(r);
		return -ENOMEM;
	}

	gameport_set_name(gp, "ALS4000 Gameport");
	gameport_set_phys(gp, "pci%s/gameport0", pci_name(acard->pci));
	gameport_set_dev_parent(gp, &acard->pci->dev);
	gp->io = io_port;
	gameport_set_port_data(gp, r);

	/* Enable legacy joystick port */
	snd_als4000_set_addr(acard->iobase, 0, 0, 0, 1);

	gameport_register_port(acard->gameport);

	return 0;
}

static void snd_als4000_free_gameport(struct snd_card_als4000 *acard)
{
	if (acard->gameport) {
		struct resource *r = gameport_get_port_data(acard->gameport);

		gameport_unregister_port(acard->gameport);
		acard->gameport = NULL;

		/* disable joystick */
		snd_als4000_set_addr(acard->iobase, 0, 0, 0, 0);

		release_and_free_resource(r);
	}
}
#else
static inline int snd_als4000_create_gameport(struct snd_card_als4000 *acard, int dev) { return -ENOSYS; }
static inline void snd_als4000_free_gameport(struct snd_card_als4000 *acard) { }
#endif

static void snd_card_als4000_free( struct snd_card *card )
{
	struct snd_card_als4000 *acard = card->private_data;

	/* make sure that interrupts are disabled */
	snd_als4k_gcr_write_addr(acard->iobase, ALS4K_GCR8C_MISC_CTRL, 0);
	/* free resources */
	snd_als4000_free_gameport(acard);
	pci_release_regions(acard->pci);
	pci_disable_device(acard->pci);
}

static int __devinit snd_card_als4000_probe(struct pci_dev *pci,
					  const struct pci_device_id *pci_id)
{
	static int dev;
	struct snd_card *card;
	struct snd_card_als4000 *acard;
	unsigned long iobase;
	struct snd_sb *chip;
	struct snd_opl3 *opl3;
	unsigned short word;
	int err;

	if (dev >= SNDRV_CARDS)
		return -ENODEV;
	if (!enable[dev]) {
		dev++;
		return -ENOENT;
	}

	/* enable PCI device */
	if ((err = pci_enable_device(pci)) < 0) {
		return err;
	}
	/* check, if we can restrict PCI DMA transfers to 24 bits */
	if (pci_set_dma_mask(pci, DMA_BIT_MASK(24)) < 0 ||
	    pci_set_consistent_dma_mask(pci, DMA_BIT_MASK(24)) < 0) {
		snd_printk(KERN_ERR "architecture does not support 24bit PCI busmaster DMA\n");
		pci_disable_device(pci);
		return -ENXIO;
	}

	if ((err = pci_request_regions(pci, "ALS4000")) < 0) {
		pci_disable_device(pci);
		return err;
	}
	iobase = pci_resource_start(pci, 0);

	pci_read_config_word(pci, PCI_COMMAND, &word);
	pci_write_config_word(pci, PCI_COMMAND, word | PCI_COMMAND_IO);
	pci_set_master(pci);
	
	err = snd_card_create(index[dev], id[dev], THIS_MODULE, 
			      sizeof(*acard) /* private_data: acard */,
			      &card);
	if (err < 0) {
		pci_release_regions(pci);
		pci_disable_device(pci);
		return err;
	}

	acard = card->private_data;
	acard->pci = pci;
	acard->iobase = iobase;
	card->private_free = snd_card_als4000_free;

	/* disable all legacy ISA stuff */
	snd_als4000_set_addr(acard->iobase, 0, 0, 0, 0);

	if ((err = snd_sbdsp_create(card,
				    iobase + ALS4K_IOB_10_ADLIB_ADDR0,
				    pci->irq,
		/* internally registered as IRQF_SHARED in case of ALS4000 SB */
				    snd_als4000_interrupt,
				    -1,
				    -1,
				    SB_HW_ALS4000,
				    &chip)) < 0) {
		goto out_err;
	}
	acard->chip = chip;

	chip->pci = pci;
	chip->alt_port = iobase;
	snd_card_set_dev(card, &pci->dev);

	snd_als4000_configure(chip);

	strcpy(card->driver, "ALS4000");
	strcpy(card->shortname, "Avance Logic ALS4000");
	sprintf(card->longname, "%s at 0x%lx, irq %i",
		card->shortname, chip->alt_port, chip->irq);

	if ((err = snd_mpu401_uart_new( card, 0, MPU401_HW_ALS4000,
					iobase + ALS4K_IOB_30_MIDI_DATA,
					MPU401_INFO_INTEGRATED,
					pci->irq, 0, &chip->rmidi)) < 0) {
		printk(KERN_ERR "als4000: no MPU-401 device at 0x%lx?\n",
				iobase + ALS4K_IOB_30_MIDI_DATA);
		goto out_err;
	}
	/* FIXME: ALS4000 has interesting MPU401 configuration features
	 * at ALS4K_CR1A_MPU401_UART_MODE_CONTROL
	 * (pass-thru / UART switching, fast MIDI clock, etc.),
	 * however there doesn't seem to be an ALSA API for this...
	 * SPECS_PAGE: 21 */

	if ((err = snd_als4000_pcm(chip, 0)) < 0) {
		goto out_err;
	}
	if ((err = snd_sbmixer_new(chip)) < 0) {
		goto out_err;
	}	    

	if (snd_opl3_create(card,
				iobase + ALS4K_IOB_10_ADLIB_ADDR0,
				iobase + ALS4K_IOB_12_ADLIB_ADDR2,
			    OPL3_HW_AUTO, 1, &opl3) < 0) {
		printk(KERN_ERR "als4000: no OPL device at 0x%lx-0x%lx?\n",
			   iobase + ALS4K_IOB_10_ADLIB_ADDR0,
			   iobase + ALS4K_IOB_12_ADLIB_ADDR2);
	} else {
		if ((err = snd_opl3_hwdep_new(opl3, 0, 1, NULL)) < 0) {
			goto out_err;
		}
	}

	snd_als4000_create_gameport(acard, dev);

	if ((err = snd_card_register(card)) < 0) {
		goto out_err;
	}
	pci_set_drvdata(pci, card);
	dev++;
	err = 0;
	goto out;

out_err:
	snd_card_free(card);
	
out:
	return err;
}

static void __devexit snd_card_als4000_remove(struct pci_dev *pci)
{
	snd_card_free(pci_get_drvdata(pci));
	pci_set_drvdata(pci, NULL);
}

#ifdef CONFIG_PM
static int snd_als4000_suspend(struct pci_dev *pci, pm_message_t state)
{
	struct snd_card *card = pci_get_drvdata(pci);
	struct snd_card_als4000 *acard = card->private_data;
	struct snd_sb *chip = acard->chip;

	snd_power_change_state(card, SNDRV_CTL_POWER_D3hot);
	
	snd_pcm_suspend_all(chip->pcm);
	snd_sbmixer_suspend(chip);

	pci_disable_device(pci);
	pci_save_state(pci);
	pci_set_power_state(pci, pci_choose_state(pci, state));
	return 0;
}

static int snd_als4000_resume(struct pci_dev *pci)
{
	struct snd_card *card = pci_get_drvdata(pci);
	struct snd_card_als4000 *acard = card->private_data;
	struct snd_sb *chip = acard->chip;

	pci_set_power_state(pci, PCI_D0);
	pci_restore_state(pci);
	if (pci_enable_device(pci) < 0) {
		printk(KERN_ERR "als4000: pci_enable_device failed, "
		       "disabling device\n");
		snd_card_disconnect(card);
		return -EIO;
	}
	pci_set_master(pci);

	snd_als4000_configure(chip);
	snd_sbdsp_reset(chip);
	snd_sbmixer_resume(chip);

#ifdef SUPPORT_JOYSTICK
	if (acard->gameport)
		snd_als4000_set_addr(acard->iobase, 0, 0, 0, 1);
#endif

	snd_power_change_state(card, SNDRV_CTL_POWER_D0);
	return 0;
}
#endif /* CONFIG_PM */


static struct pci_driver driver = {
	.name = "ALS4000",
	.id_table = snd_als4000_ids,
	.probe = snd_card_als4000_probe,
	.remove = __devexit_p(snd_card_als4000_remove),
#ifdef CONFIG_PM
	.suspend = snd_als4000_suspend,
	.resume = snd_als4000_resume,
#endif
};

static int __init alsa_card_als4000_init(void)
{
	return pci_register_driver(&driver);
}

static void __exit alsa_card_als4000_exit(void)
{
	pci_unregister_driver(&driver);
}

module_init(alsa_card_als4000_init)
module_exit(alsa_card_als4000_exit)

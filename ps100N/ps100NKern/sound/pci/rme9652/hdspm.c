#ifndef _S921_CORE_H
#define _S921_CORE_H
//#define u8 unsigned int
//#define u32 unsigned int



//#define EINVAL -1
#define E_OK 0

struct s921_isdb_t {
	void *priv_dev;
	int (*i2c_write)(void *dev, u8 reg, u8 val);
	int (*i2c_read)(void *dev, u8 reg);
};

#define ISDB_T_CMD_INIT       0
#define ISDB_T_CMD_SET_PARAM  1
#define ISDB_T_CMD_TUNE       2
#define ISDB_T_CMD_GET_STATUS 3

struct s921_isdb_t_tune_params {
	u32 frequency;
};

struct s921_isdb_t_status {
};

struct s921_isdb_t_transmission_mode_params {
	u8 mode;
	u8 layer_a_mode;
#define ISDB_T_LA_MODE_1 0
#define ISDB_T_LA_MODE_2 1
#define ISDB_T_LA_MODE_3 2
	u8 layer_a_carrier_modulation;
#define ISDB_T_LA_CM_DQPSK 0
#define ISDB_T_LA_CM_QPSK  1
#define ISDB_T_LA_CM_16QAM 2
#define ISDB_T_LA_CM_64QAM 3
#define ISDB_T_LA_CM_NOLAYER 4
	u8 layer_a_code_rate;
#define ISDB_T_LA_CR_1_2   0
#define ISDB_T_LA_CR_2_3   1
#define ISDB_T_LA_CR_3_4   2
#define ISDB_T_LA_CR_5_6   4
#define ISDB_T_LA_CR_7_8   8
#define ISDB_T_LA_CR_NOLAYER   16
	u8 layer_a_time_interleave;
#define ISDB_T_LA_TI_0  0
#define ISDB_T_LA_TI_1  1
#define ISDB_T_LA_TI_2  2
#define ISDB_T_LA_TI_4  4
#define ISDB_T_LA_TI_8  8
#define ISDB_T_LA_TI_16 16
#define ISDB_T_LA_TI_32 32
	u8 layer_a_nseg;

	u8 layer_b_mode;
#define ISDB_T_LB_MODE_1 0
#define ISDB_T_LB_MODE_2 1
#define ISDB_T_LB_MODE_3 2
	u8 layer_b_carrier_modulation;
#define ISDB_T_LB_CM_DQPSK 0
#define ISDB_T_LB_CM_QPSK  1
#define ISDB_T_LB_CM_16QAM 2
#define ISDB_T_LB_CM_64QAM 3
#define ISDB_T_LB_CM_NOLAYER 4
	u8 layer_b_code_rate;
#define ISDB_T_LB_CR_1_2   0
#define ISDB_T_LB_CR_2_3   1
#define ISDB_T_LB_CR_3_4   2
#define ISDB_T_LB_CR_5_6   4
#define ISDB_T_LB_CR_7_8   8
#define ISDB_T_LB_CR_NOLAYER   16
	u8 layer_b_time_interleave;
#define ISDB_T_LB_TI_0  0
#define ISDB_T_LB_TI_1  1
#define ISDB_T_LB_TI_2  2
#define ISDB_T_LB_TI_4  4
#define ISDB_T_LB_TI_8  8
#define ISDB_T_LB_TI_16 16
#define ISDB_T_LB_TI_32 32
	u8 layer_b_nseg;

	u8 layer_c_mode;
#define ISDB_T_LC_MODE_1 0
#define ISDB_T_LC_MODE_2 1
#define ISDB_T_LC_MODE_3 2
	u8 layer_c_carrier_modulation;
#define ISDB_T_LC_CM_DQPSK 0
#define ISDB_T_LC_CM_QPSK  1
#define ISDB_T_LC_CM_16QAM 2
#define ISDB_T_LC_CM_64QAM 3
#define ISDB_T_LC_CM_NOLAYER 4
	u8 layer_c_code_rate;
#define ISDB_T_LC_CR_1_2   0
#define ISDB_T_LC_CR_2_3   1
#define ISDB_T_LC_CR_3_4   2
#define ISDB_T_LC_CR_5_6   4
#define ISDB_T_LC_CR_7_8   8
#define ISDB_T_LC_CR_NOLAYER   16
	u8 layer_c_time_interleave;
#define ISDB_T_LC_TI_0  0
#define ISDB_T_LC_TI_1  1
#define ISDB_T_LC_TI_2  2
#define ISDB_T_LC_TI_4  4
#define ISDB_T_LC_TI_8  8
#define ISDB_T_LC_TI_16 16
#define ISDB_T_LC_TI_32 32
	u8 layer_c_nseg;
};

int s921_isdb_cmd(struct s921_isdb_t *dev, u32 cmd, void *data);
#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            /*
 * Linux driver for digital TV devices equipped with B2C2 FlexcopII(b)/III
 * flexcop.h - private header file for all flexcop-chip-source files
 * see flexcop.c for copyright information
 */
#ifndef __FLEXCOP_H__
#define __FLEXCOP_H___

#define FC_LOG_PREFIX "b2c2-flexcop"
#include "flexcop-common.h"

extern int b2c2_flexcop_debug;

/* debug */
#ifdef CONFIG_DVB_B2C2_FLEXCOP_DEBUG
#define dprintk(level,args...) \
	do { if ((b2c2_flexcop_debug & level)) printk(args); } while (0)
#else
#define dprintk(level,args...)
#endif

#define deb_info(args...) dprintk(0x01, args)
#define deb_tuner(args...) dprintk(0x02, args)
#define deb_i2c(args...) dprintk(0x04, args)
#define deb_ts(args...) dprintk(0x08, args)
#define deb_sram(args...) dprintk(0x10, args)
#define deb_rdump(args...) dprintk(0x20, args)

#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  /* saa711x - Philips SAA711x video decoder driver
 * This driver can work with saa7111, saa7111a, saa7113, saa7114,
 *			     saa7115 and saa7118.
 *
 * Based on saa7114 driver by Maxim Yevtyushkin, which is based on
 * the saa7111 driver by Dave Perks.
 *
 * Copyright (C) 1998 Dave Perks <dperks@ibm.net>
 * Copyright (C) 2002 Maxim Yevtyushkin <max@linuxmedialabs.com>
 *
 * Slight changes for video timing and attachment output by
 * Wolfgang Scherr <scherr@net4you.net>
 *
 * Moved over to the linux >= 2.4.x i2c protocol (1/1/2003)
 * by Ronald Bultje <rbultje@ronald.bitfreak.net>
 *
 * Added saa7115 support by Kevin Thayer <nufan_wfk at yahoo.com>
 * (2/17/2003)
 *
 * VBI support (2004) and cleanups (2005) by Hans Verkuil <hverkuil@xs4all.nl>
 *
 * Copyright (c) 2005-2006 Mauro Carvalho Chehab <mchehab@infradead.org>
 *	SAA7111, SAA7113 and SAA7118 support
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "saa711x_regs.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-i2c-drv.h>
#include <media/saa7115.h>
#include <asm/div64.h>

#define VRES_60HZ	(480+16)

MODULE_DESCRIPTION("Philips SAA7111/SAA7113/SAA7114/SAA7115/SAA7118 video decoder driver");
MODULE_AUTHOR(  "Maxim Yevtyushkin, Kevin Thayer, Chris Kennedy, "
		"Hans Verkuil, Mauro Carvalho Chehab");
MODULE_LICENSE("GPL");

static int debug;
module_param(debug, bool, 0644);

MODULE_PARM_DESC(debug, "Debug level (0-1)");


struct saa711x_state {
	struct v4l2_subdev sd;
	v4l2_std_id std;
	int input;
	int output;
	int enable;
	int radio;
	int bright;
	int contrast;
	int hue;
	int sat;
	int width;
	int height;
	u32 ident;
	u32 audclk_freq;
	u32 crystal_freq;
	u8 ucgc;
	u8 cgcdiv;
	u8 apll;
};

static inline struct saa711x_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct saa711x_state, sd);
}

/* ----------------------------------------------------------------------- */

static inline int saa711x_write(struct v4l2_subdev *sd, u8 reg, u8 value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return i2c_smbus_write_byte_data(client, reg, value);
}

/* Sanity routine to check if a register is present */
static int saa711x_has_reg(const int id, const u8 reg)
{
	if (id == V4L2_IDENT_SAA7111)
		return reg < 0x20 && reg != 0x01 && reg != 0x0f &&
		       (reg < 0x13 || reg > 0x19) && reg != 0x1d && reg != 0x1e;

	/* common for saa7113/4/5/8 */
	if (unlikely((reg >= 0x3b && reg <= 0x3f) || reg == 0x5c || reg == 0x5f ||
	    reg == 0xa3 || reg == 0xa7 || reg == 0xab || reg == 0xaf || (reg >= 0xb5 && reg <= 0xb7) ||
	    reg == 0xd3 || reg == 0xd7 || reg == 0xdb || reg == 0xdf || (reg >= 0xe5 && reg <= 0xe7) ||
	    reg == 0x82 || (reg >= 0x89 && reg <= 0x8e)))
		return 0;

	switch (id) {
	case V4L2_IDENT_SAA7113:
		return reg != 0x14 && (reg < 0x18 || reg > 0x1e) && (reg < 0x20 || reg > 0x3f) &&
		       reg != 0x5d && reg < 0x63;
	case V4L2_IDENT_SAA7114:
		return (reg < 0x1a || reg > 0x1e) && (reg < 0x20 || reg > 0x2f) &&
		       (reg < 0x63 || reg > 0x7f) && reg != 0x33 && reg != 0x37 &&
		       reg != 0x81 && reg < 0xf0;
	case V4L2_IDENT_SAA7115:
		return (reg < 0x20 || reg > 0x2f) && reg != 0x65 && (reg < 0xfc || reg > 0xfe);
	case V4L2_IDENT_SAA7118:
		return (reg < 0x1a || reg > 0x1d) && (reg < 0x20 || reg > 0x22) &&
		       (reg < 0x26 || reg > 0x28) && reg != 0x33 && reg != 0x37 &&
		       (reg < 0x63 || reg > 0x7f) && reg != 0x81 && reg < 0xf0;
	}
	return 1;
}

static int saa711x_writeregs(struct v4l2_subdev *sd, const unsigned char *regs)
{
	struct saa711x_state *state = to_state(sd);
	unsigned char reg, data;

	while (*regs != 0x00) {
		reg = *(regs++);
		data = *(regs++);

		/* According with datasheets, reserved regs should be
		   filled with 0 - seems better not to touch on they */
		if (saa711x_has_reg(state->ident, reg)) {
			if (saa711x_write(sd, reg, data) < 0)
				return -1;
		} else {
			v4l2_dbg(1, debug, sd, "tried to access reserved reg 0x%02x\n", reg);
		}
	}
	return 0;
}

static inline int saa711x_read(struct v4l2_subdev *sd, u8 reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return i2c_smbus_read_byte_data(client, reg);
}

/* ----------------------------------------------------------------------- */

/* SAA7111 initialization table */
static const unsigned char saa7111_init[] = {
	R_01_INC_DELAY, 0x00,		/* reserved */

	/*front end */
	R_02_INPUT_CNTL_1, 0xd0,	/* FUSE=3, GUDL=2, MODE=0 */
	R_03_INPUT_CNTL_2, 0x23,	/* HLNRS=0, VBSL=1, WPOFF=0, HOLDG=0,
					 * GAFIX=0, GAI1=256, GAI2=256 */
	R_04_INPUT_CNTL_3, 0x00,	/* GAI1=256 */
	R_05_INPUT_CNTL_4, 0x00,	/* GAI2=256 */

	/* decoder */
	R_06_H_SYNC_START, 0xf3,	/* HSB at  13(50Hz) /  17(60Hz)
					 * pixels after end of last line */
	R_07_H_SYNC_STOP, 0xe8,		/* HSS seems to be needed to
					 * work with NTSC, too */
	R_08_SYNC_CNTL, 0xc8,		/* AUFD=1, FSEL=1, EXFIL=0,
					 * VTRC=1, HPLL=0, VNOI=0 */
	R_09_LUMA_CNTL, 0x01,		/* BYPS=0, PREF=0, BPSS=0,
					 * VBLB=0, UPTCV=0, APER=1 */
	R_0A_LUMA_BRIGHT_CNTL, 0x80,
	R_0B_LUMA_CONTRAST_CNTL, 0x47,	/* 0b - CONT=1.109 */
	R_0C_CHROMA_SAT_CNTL, 0x40,
	R_0D_CHROMA_HUE_CNTL, 0x00,
	R_0E_CHROMA_CNTL_1, 0x01,	/* 0e - CDTO=0, CSTD=0, DCCF=0,
					 * FCTC=0, CHBW=1 */
	R_0F_CHROMA_GAIN_CNTL, 0x00,	/* reserved */
	R_10_CHROMA_CNTL_2, 0x48,	/* 10 - OFTS=1, HDEL=0, VRLN=1, YDEL=0 */
	R_11_MODE_DELAY_CNTL, 0x1c,	/* 11 - GPSW=0, CM99=0, FECO=0, COMPO=1,
					 * OEYC=1, OEHV=1, VIPB=0, COLO=0 */
	R_12_RT_SIGNAL_CNTL, 0x00,	/* 12 - output control 2 */
	R_13_RT_X_PORT_OUT_CNTL, 0x00,	/* 13 - output control 3 */
	R_14_ANAL_ADC_COMPAT_CNTL, 0x00,
	R_15_VGATE_START_FID_CHG, 0x00,
	R_16_VGATE_STOP, 0x00,
	R_17_MISC_VGATE_CONF_AND_MSB, 0x00,

	0x00, 0x00
};

/* SAA7113 init codes */
static const unsigned char saa7113_init[] = {
	R_01_INC_DELAY, 0x08,
	R_02_INPUT_CNTL_1, 0xc2,
	R_03_INPUT_CNTL_2, 0x30,
	R_04_INPUT_CNTL_3, 0x00,
	R_05_INPUT_CNTL_4, 0x00,
	R_06_H_SYNC_START, 0x89,
	R_07_H_SYNC_STOP, 0x0d,
	R_08_SYNC_CNTL, 0x88,
	R_09_LUMA_CNTL, 0x01,
	R_0A_LUMA_BRIGHT_CNTL, 0x80,
	R_0B_LUMA_CONTRAST_CNTL, 0x47,
	R_0C_CHROMA_SAT_CNTL, 0x40,
	R_0D_CHROMA_HUE_CNTL, 0x00,
	R_0E_CHROMA_CNTL_1, 0x01,
	R_0F_CHROMA_GAIN_CNTL, 0x2a,
	R_10_CHROMA_CNTL_2, 0x08,
	R_11_MODE_DELAY_CNTL, 0x0c,
	R_12_RT_SIGNAL_CNTL, 0x07,
	R_13_RT_X_PORT_OUT_CNTL, 0x00,
	R_14_ANAL_ADC_COMPAT_CNTL, 0x00,
	R_15_VGATE_START_FID_CHG, 0x00,
	R_16_VGATE_STOP, 0x00,
	R_17_MISC_VGATE_CONF_AND_MSB, 0x00,

	0x00, 0x00
};

/* If a value differs from the Hauppauge driver values, then the comment starts with
   'was 0xXX' to denote the Hauppauge value. Otherwise the value is identical to what the
   Hauppauge driver sets. */

/* SAA7114 and SAA7115 initialization table */
static const unsigned char saa7115_init_auto_input[] = {
		/* Front-End Part */
	R_01_INC_DELAY, 0x48,			/* white peak control disabled */
	R_03_INPUT_CNTL_2, 0x20,		/* was 0x30. 0x20: long vertical blanking */
	R_04_INPUT_CNTL_3, 0x90,		/* analog gain set to 0 */
	R_05_INPUT_CNTL_4, 0x90,		/* analog gain set to 0 */
		/* Decoder Part */
	R_06_H_SYNC_START, 0xeb,		/* horiz sync begin = -21 */
	R_07_H_SYNC_STOP, 0xe0,			/* horiz sync stop = -17 */
	R_09_LUMA_CNTL, 0x53,			/* 0x53, was 0x56 for 60hz. luminance control */
	R_0A_LUMA_BRIGHT_CNTL, 0x80,		/* was 0x88. decoder brightness, 0x80 is itu standard */
	R_0B_LUMA_CONTRAST_CNTL, 0x44,		/* was 0x48. decoder contrast, 0x44 is itu standard */
	R_0C_CHROMA_SAT_CNTL, 0x40,		/* was 0x47. decoder saturation, 0x40 is itu standard */
	R_0D_CHROMA_HUE_CNTL, 0x00,
	R_0F_CHROMA_GAIN_CNTL, 0x00,		/* use automatic gain  */
	R_10_CHROMA_CNTL_2, 0x06,		/* chroma: active adaptive combfilter */
	R_11_MODE_DELAY_CNTL, 0x00,
	R_12_RT_SIGNAL_CNTL, 0x9d,		/* RTS0 output control: VGATE */
	R_13_RT_X_PORT_OUT_CNTL, 0x80,		/* ITU656 standard mode, RTCO output enable RTCE */
	R_14_ANAL_ADC_COMPAT_CNTL, 0x00,
	R_18_RAW_DATA_GAIN_CNTL, 0x40,		/* gain 0x00 = nominal */
	R_19_RAW_DATA_OFF_CNTL, 0x80,
	R_1A_COLOR_KILL_LVL_CNTL, 0x77,		/* recommended value */
	R_1B_MISC_TVVCRDET, 0x42,		/* recommended value */
	R_1C_ENHAN_COMB_CTRL1, 0xa9,		/* recommended value */
	R_1D_ENHAN_COMB_CTRL2, 0x01,		/* recommended value */


	R_80_GLOBAL_CNTL_1, 0x0,		/* No tasks enabled at init */

		/* Power Device Control */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xd0,	/* reset device */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xf0,	/* set device programmed, all in operational mode */
	0x00, 0x00
};

/* Used to reset saa7113, saa7114 and saa7115 */
static const unsigned char saa7115_cfg_reset_scaler[] = {
	R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED, 0x00,	/* disable I-port output */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xd0,		/* reset scaler */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xf0,		/* activate scaler */
	R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED, 0x01,	/* enable I-port output */
	0x00, 0x00
};

/* ============== SAA7715 VIDEO templates =============  */

static const unsigned char saa7115_cfg_60hz_video[] = {
	R_80_GLOBAL_CNTL_1, 0x00,			/* reset tasks */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xd0,		/* reset scaler */

	R_15_VGATE_START_FID_CHG, 0x03,
	R_16_VGATE_STOP, 0x11,
	R_17_MISC_VGATE_CONF_AND_MSB, 0x9c,

	R_08_SYNC_CNTL, 0x68,			/* 0xBO: auto detection, 0x68 = NTSC */
	R_0E_CHROMA_CNTL_1, 0x07,		/* video autodetection is on */

	R_5A_V_OFF_FOR_SLICER, 0x06,		/* standard 60hz value for ITU656 line counting */

	/* Task A */
	R_90_A_TASK_HANDLING_CNTL, 0x80,
	R_91_A_X_PORT_FORMATS_AND_CONF, 0x48,
	R_92_A_X_PORT_INPUT_REFERENCE_SIGNAL, 0x40,
	R_93_A_I_PORT_OUTPUT_FORMATS_AND_CONF, 0x84,

	/* hoffset low (input), 0x0002 is minimum */
	R_94_A_HORIZ_INPUT_WINDOW_START, 0x01,
	R_95_A_HORIZ_INPUT_WINDOW_START_MSB, 0x00,

	/* hsize low (input), 0x02d0 = 720 */
	R_96_A_HORIZ_INPUT_WINDOW_LENGTH, 0xd0,
	R_97_A_HORIZ_INPUT_WINDOW_LENGTH_MSB, 0x02,

	R_98_A_VERT_INPUT_WINDOW_START, 0x05,
	R_99_A_VERT_INPUT_WINDOW_START_MSB, 0x00,

	R_9A_A_VERT_INPUT_WINDOW_LENGTH, 0x0c,
	R_9B_A_VERT_INPUT_WINDOW_LENGTH_MSB, 0x00,

	R_9C_A_HORIZ_OUTPUT_WINDOW_LENGTH, 0xa0,
	R_9D_A_HORIZ_OUTPUT_WINDOW_LENGTH_MSB, 0x05,

	R_9E_A_VERT_OUTPUT_WINDOW_LENGTH, 0x0c,
	R_9F_A_VERT_OUTPUT_WINDOW_LENGTH_MSB, 0x00,

	/* Task B */
	R_C0_B_TASK_HANDLING_CNTL, 0x00,
	R_C1_B_X_PORT_FORMATS_AND_CONF, 0x08,
	R_C2_B_INPUT_REFERENCE_SIGNAL_DEFINITION, 0x00,
	R_C3_B_I_PORT_FORMATS_AND_CONF, 0x80,

	/* 0x0002 is minimum */
	R_C4_B_HORIZ_INPUT_WINDOW_START, 0x02,
	R_C5_B_HORIZ_INPUT_WINDOW_START_MSB, 0x00,

	/* 0x02d0 = 720 */
	R_C6_B_HORIZ_INPUT_WINDOW_LENGTH, 0xd0,
	R_C7_B_HORIZ_INPUT_WINDOW_LENGTH_MSB, 0x02,

	/* vwindow start 0x12 = 18 */
	R_C8_B_VERT_INPUT_WINDOW_START, 0x12,
	R_C9_B_VERT_INPUT_WINDOW_START_MSB, 0x00,

	/* vwindow length 0xf8 = 248 */
	R_CA_B_VERT_INPUT_WINDOW_LENGTH, VRES_60HZ>>1,
	R_CB_B_VERT_INPUT_WINDOW_LENGTH_MSB, VRES_60HZ>>9,

	/* hwindow 0x02d0 = 720 */
	R_CC_B_HORIZ_OUTPUT_WINDOW_LENGTH, 0xd0,
	R_CD_B_HORIZ_OUTPUT_WINDOW_LENGTH_MSB, 0x02,

	R_F0_LFCO_PER_LINE, 0xad,		/* Set PLL Register. 60hz 525 lines per frame, 27 MHz */
	R_F1_P_I_PARAM_SELECT, 0x05,		/* low bit with 0xF0 */
	R_F5_PULSGEN_LINE_LENGTH, 0xad,
	R_F6_PULSE_A_POS_LSB_AND_PULSEGEN_CONFIG, 0x01,

	0x00, 0x00
};

static const unsigned char saa7115_cfg_50hz_video[] = {
	R_80_GLOBAL_CNTL_1, 0x00,
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xd0,	/* reset scaler */

	R_15_VGATE_START_FID_CHG, 0x37,		/* VGATE start */
	R_16_VGATE_STOP, 0x16,
	R_17_MISC_VGATE_CONF_AND_MSB, 0x99,

	R_08_SYNC_CNTL, 0x28,			/* 0x28 = PAL */
	R_0E_CHROMA_CNTL_1, 0x07,

	R_5A_V_OFF_FOR_SLICER, 0x03,		/* standard 50hz value */

	/* Task A */
	R_90_A_TASK_HANDLING_CNTL, 0x81,
	R_91_A_X_PORT_FORMATS_AND_CONF, 0x48,
	R_92_A_X_PORT_INPUT_REFERENCE_SIGNAL, 0x40,
	R_93_A_I_PORT_OUTPUT_FORMATS_AND_CONF, 0x84,

	/* This is weird: the datasheet says that you should use 2 as the minimum value, */
	/* but Hauppauge uses 0, and changing that to 2 causes indeed problems (for 50hz) */
	/* hoffset low (input), 0x0002 is minimum */
	R_94_A_HORIZ_INPUT_WINDOW_START, 0x00,
	R_95_A_HORIZ_INPUT_WINDOW_START_MSB, 0x00,

	/* hsize low (input), 0x02d0 = 720 */
	R_96_A_HORIZ_INPUT_WINDOW_LENGTH, 0xd0,
	R_97_A_HORIZ_INPUT_WINDOW_LENGTH_MSB, 0x02,

	R_98_A_VERT_INPUT_WINDOW_START, 0x03,
	R_99_A_VERT_INPUT_WINDOW_START_MSB, 0x00,

	/* vsize 0x12 = 18 */
	R_9A_A_VERT_INPUT_WINDOW_LENGTH, 0x12,
	R_9B_A_VERT_INPUT_WINDOW_LENGTH_MSB, 0x00,

	/* hsize 0x05a0 = 1440 */
	R_9C_A_HORIZ_OUTPUT_WINDOW_LENGTH, 0xa0,
	R_9D_A_HORIZ_OUTPUT_WINDOW_LENGTH_MSB, 0x05,	/* hsize hi (output) */
	R_9E_A_VERT_OUTPUT_WINDOW_LENGTH, 0x12,		/* vsize low (output), 0x12 = 18 */
	R_9F_A_VERT_OUTPUT_WINDOW_LENGTH_MSB, 0x00,	/* vsize hi (output) */

	/* Task B */
	R_C0_B_TASK_HANDLING_CNTL, 0x00,
	R_C1_B_X_PORT_FORMATS_AND_CONF, 0x08,
	R_C2_B_INPUT_REFERENCE_SIGNAL_DEFINITION, 0x00,
	R_C3_B_I_PORT_FORMATS_AND_CONF, 0x80,

	/* This is weird: the datasheet says that you should use 2 as the minimum value, */
	/* but Hauppauge uses 0, and changing that to 2 causes indeed problems (for 50hz) */
	/* hoffset low (input), 0x0002 is minimum. See comment above. */
	R_C4_B_HORIZ_INPUT_WINDOW_START, 0x00,
	R_C5_B_HORIZ_INPUT_WINDOW_START_MSB, 0x00,

	/* hsize 0x02d0 = 720 */
	R_C6_B_HORIZ_INPUT_WINDOW_LENGTH, 0xd0,
	R_C7_B_HORIZ_INPUT_WINDOW_LENGTH_MSB, 0x02,

	/* voffset 0x16 = 22 */
	R_C8_B_VERT_INPUT_WINDOW_START, 0x16,
	R_C9_B_VERT_INPUT_WINDOW_START_MSB, 0x00,

	/* vsize 0x0120 = 288 */
	R_CA_B_VERT_INPUT_WINDOW_LENGTH, 0x20,
	R_CB_B_VERT_INPUT_WINDOW_LENGTH_MSB, 0x01,

	/* hsize 0x02d0 = 720 */
	R_CC_B_HORIZ_OUTPUT_WINDOW_LENGTH, 0xd0,
	R_CD_B_HORIZ_OUTPUT_WINDOW_LENGTH_MSB, 0x02,

	R_F0_LFCO_PER_LINE, 0xb0,		/* Set PLL Register. 50hz 625 lines per frame, 27 MHz */
	R_F1_P_I_PARAM_SELECT, 0x05,		/* low bit with 0xF0, (was 0x05) */
	R_F5_PULSGEN_LINE_LENGTH, 0xb0,
	R_F6_PULSE_A_POS_LSB_AND_PULSEGEN_CONFIG, 0x01,

	0x00, 0x00
};

/* ============== SAA7715 VIDEO templates (end) =======  */

static const unsigned char saa7115_cfg_vbi_on[] = {
	R_80_GLOBAL_CNTL_1, 0x00,			/* reset tasks */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xd0,		/* reset scaler */
	R_80_GLOBAL_CNTL_1, 0x30,			/* Activate both tasks */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xf0,		/* activate scaler */
	R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED, 0x01,	/* Enable I-port output */

	0x00, 0x00
};

static const unsigned char saa7115_cfg_vbi_off[] = {
	R_80_GLOBAL_CNTL_1, 0x00,			/* reset tasks */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xd0,		/* reset scaler */
	R_80_GLOBAL_CNTL_1, 0x20,			/* Activate only task "B" */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xf0,		/* activate scaler */
	R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED, 0x01,	/* Enable I-port output */

	0x00, 0x00
};


static const unsigned char saa7115_init_misc[] = {
	R_81_V_SYNC_FLD_ID_SRC_SEL_AND_RETIMED_V_F, 0x01,
	R_83_X_PORT_I_O_ENA_AND_OUT_CLK, 0x01,
	R_84_I_PORT_SIGNAL_DEF, 0x20,
	R_85_I_PORT_SIGNAL_POLAR, 0x21,
	R_86_I_PORT_FIFO_FLAG_CNTL_AND_ARBIT, 0xc5,
	R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED, 0x01,

	/* Task A */
	R_A0_A_HORIZ_PRESCALING, 0x01,
	R_A1_A_ACCUMULATION_LENGTH, 0x00,
	R_A2_A_PRESCALER_DC_GAIN_AND_FIR_PREFILTER, 0x00,

	/* Configure controls at nominal value*/
	R_A4_A_LUMA_BRIGHTNESS_CNTL, 0x80,
	R_A5_A_LUMA_CONTRAST_CNTL, 0x40,
	R_A6_A_CHROMA_SATURATION_CNTL, 0x40,

	/* note: 2 x zoom ensures that VBI lines have same length as video lines. */
	R_A8_A_HORIZ_LUMA_SCALING_INC, 0x00,
	R_A9_A_HORIZ_LUMA_SCALING_INC_MSB, 0x02,

	R_AA_A_HORIZ_LUMA_PHASE_OFF, 0x00,

	/* must be horiz lum scaling / 2 */
	R_AC_A_HORIZ_CHROMA_SCALING_INC, 0x00,
	R_AD_A_HORIZ_CHROMA_SCALING_INC_MSB, 0x01,

	/* must be offset luma / 2 */
	R_AE_A_HORIZ_CHROMA_PHASE_OFF, 0x00,

	R_B0_A_VERT_LUMA_SCALING_INC, 0x00,
	R_B1_A_VERT_LUMA_SCALING_INC_MSB, 0x04,

	R_B2_A_VERT_CHROMA_SCALING_INC, 0x00,
	R_B3_A_VERT_CHROMA_SCALING_INC_MSB, 0x04,

	R_B4_A_VERT_SCALING_MODE_CNTL, 0x01,

	R_B8_A_VERT_CHROMA_PHASE_OFF_00, 0x00,
	R_B9_A_VERT_CHROMA_PHASE_OFF_01, 0x00,
	R_BA_A_VERT_CHROMA_PHASE_OFF_10, 0x00,
	R_BB_A_VERT_CHROMA_PHASE_OFF_11, 0x00,

	R_BC_A_VERT_LUMA_PHASE_OFF_00, 0x00,
	R_BD_A_VERT_LUMA_PHASE_OFF_01, 0x00,
	R_BE_A_VERT_LUMA_PHASE_OFF_10, 0x00,
	R_BF_A_VERT_LUMA_PHASE_OFF_11, 0x00,

	/* Task B */
	R_D0_B_HORIZ_PRESCALING, 0x01,
	R_D1_B_ACCUMULATION_LENGTH, 0x00,
	R_D2_B_PRESCALER_DC_GAIN_AND_FIR_PREFILTER, 0x00,

	/* Configure controls at nominal value*/
	R_D4_B_LUMA_BRIGHTNESS_CNTL, 0x80,
	R_D5_B_LUMA_CONTRAST_CNTL, 0x40,
	R_D6_B_CHROMA_SATURATION_CNTL, 0x40,

	/* hor lum scaling 0x0400 = 1 */
	R_D8_B_HORIZ_LUMA_SCALING_INC, 0x00,
	R_D9_B_HORIZ_LUMA_SCALING_INC_MSB, 0x04,

	R_DA_B_HORIZ_LUMA_PHASE_OFF, 0x00,

	/* must be hor lum scaling / 2 */
	R_DC_B_HORIZ_CHROMA_SCALING, 0x00,
	R_DD_B_HORIZ_CHROMA_SCALING_MSB, 0x02,

	/* must be offset luma / 2 */
	R_DE_B_HORIZ_PHASE_OFFSET_CRHOMA, 0x00,

	R_E0_B_VERT_LUMA_SCALING_INC, 0x00,
	R_E1_B_VERT_LUMA_SCALING_INC_MSB, 0x04,

	R_E2_B_VERT_CHROMA_SCALING_INC, 0x00,
	R_E3_B_VERT_CHROMA_SCALING_INC_MSB, 0x04,

	R_E4_B_VERT_SCALING_MODE_CNTL, 0x01,

	R_E8_B_VERT_CHROMA_PHASE_OFF_00, 0x00,
	R_E9_B_VERT_CHROMA_PHASE_OFF_01, 0x00,
	R_EA_B_VERT_CHROMA_PHASE_OFF_10, 0x00,
	R_EB_B_VERT_CHROMA_PHASE_OFF_11, 0x00,

	R_EC_B_VERT_LUMA_PHASE_OFF_00, 0x00,
	R_ED_B_VERT_LUMA_PHASE_OFF_01, 0x00,
	R_EE_B_VERT_LUMA_PHASE_OFF_10, 0x00,
	R_EF_B_VERT_LUMA_PHASE_OFF_11, 0x00,

	R_F2_NOMINAL_PLL2_DTO, 0x50,		/* crystal clock = 24.576 MHz, target = 27MHz */
	R_F3_PLL_INCREMENT, 0x46,
	R_F4_PLL2_STATUS, 0x00,
	R_F7_PULSE_A_POS_MSB, 0x4b,		/* not the recommended settings! */
	R_F8_PULSE_B_POS, 0x00,
	R_F9_PULSE_B_POS_MSB, 0x4b,
	R_FA_PULSE_C_POS, 0x00,
	R_FB_PULSE_C_POS_MSB, 0x4b,

	/* PLL2 lock detection settings: 71 lines 50% phase error */
	R_FF_S_PLL_MAX_PHASE_ERR_THRESH_NUM_LINES, 0x88,

	/* Turn off VBI */
	R_40_SLICER_CNTL_1, 0x20,             /* No framing code errors allowed. */
	R_41_LCR_BASE, 0xff,
	R_41_LCR_BASE+1, 0xff,
	R_41_LCR_BASE+2, 0xff,
	R_41_LCR_BASE+3, 0xff,
	R_41_LCR_BASE+4, 0xff,
	R_41_LCR_BASE+5, 0xff,
	R_41_LCR_BASE+6, 0xff,
	R_41_LCR_BASE+7, 0xff,
	R_41_LCR_BASE+8, 0xff,
	R_41_LCR_BASE+9, 0xff,
	R_41_LCR_BASE+10, 0xff,
	R_41_LCR_BASE+11, 0xff,
	R_41_LCR_BASE+12, 0xff,
	R_41_LCR_BASE+13, 0xff,
	R_41_LCR_BASE+14, 0xff,
	R_41_LCR_BASE+15, 0xff,
	R_41_LCR_BASE+16, 0xff,
	R_41_LCR_BASE+17, 0xff,
	R_41_LCR_BASE+18, 0xff,
	R_41_LCR_BASE+19, 0xff,
	R_41_LCR_BASE+20, 0xff,
	R_41_LCR_BASE+21, 0xff,
	R_41_LCR_BASE+22, 0xff,
	R_58_PROGRAM_FRAMING_CODE, 0x40,
	R_59_H_OFF_FOR_SLICER, 0x47,
	R_5B_FLD_OFF_AND_MSB_FOR_H_AND_V_OFF, 0x83,
	R_5D_DID, 0xbd,
	R_5E_SDID, 0x35,

	R_02_INPUT_CNTL_1, 0x84,		/* input tuner -> input 4, amplifier active */

	R_80_GLOBAL_CNTL_1, 0x20,		/* enable task B */
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xd0,
	R_88_POWER_SAVE_ADC_PORT_CNTL, 0xf0,
	0x00, 0x00
};

static int saa711x_odd_parity(u8 c)
{
	c ^= (c >> 4);
	c ^= (c >> 2);
	c ^= (c >> 1);

	return c & 1;
}

static int saa711x_decode_vps(u8 *dst, u8 *p)
{
	static const u8 biphase_tbl[] = {
		0xf0, 0x78, 0x70, 0xf0, 0xb4, 0x3c, 0x34, 0xb4,
		0xb0, 0x38, 0x30, 0xb0, 0xf0, 0x78, 0x70, 0xf0,
		0xd2, 0x5a, 0x52, 0xd2, 0x96, 0x1e, 0x16, 0x96,
		0x92, 0x1a, 0x12, 0x92, 0xd2, 0x5a, 0x52, 0xd2,
		0xd0, 0x58, 0x50, 0xd0, 0x94, 0x1c, 0x14, 0x94,
		0x90, 0x18, 0x10, 0x90, 0xd0, 0x58, 0x50, 0xd0,
		0xf0, 0x78, 0x70, 0xf0, 0xb4, 0x3c, 0x34, 0xb4,
		0xb0, 0x38, 0x30, 0xb0, 0xf0, 0x78, 0x70, 0xf0,
		0xe1, 0x69, 0x61, 0xe1, 0xa5, 0x2d, 0x25, 0xa5,
		0xa1, 0x29, 0x21, 0xa1, 0xe1, 0x69, 0x61, 0xe1,
		0xc3, 0x4b, 0x43, 0xc3, 0x87, 0x0f, 0x07, 0x87,
		0x83, 0x0b, 0x03, 0x83, 0xc3, 0x4b, 0x43, 0xc3,
		0xc1, 0x49, 0x41, 0xc1, 0x85, 0x0d, 0x05, 0x85,
		0x81, 0x09, 0x01, 0x81, 0xc1, 0x49, 0x41, 0xc1,
		0xe1, 0x69, 0x61, 0xe1, 0xa5, 0x2d, 0x25, 0xa5,
		0xa1, 0x29, 0x21, 0xa1, 0xe1, 0x69, 0x61, 0xe1,
		0xe0, 0x68, 0x60, 0xe0, 0xa4, 0x2c, 0x24, 0xa4,
		0xa0, 0x28, 0x20, 0xa0, 0xe0, 0x68, 0x60, 0xe0,
		0xc2, 0x4a, 0x42, 0xc2, 0x86, 0x0e, 0x06, 0x86,
		0x82, 0x0a, 0x02, 0x82, 0xc2, 0x4a, 0x42, 0xc2,
		0xc0, 0x48, 0x40, 0xc0, 0x84, 0x0c, 0x04, 0x84,
		0x80, 0x08, 0x00, 0x80, 0xc0, 0x48, 0x40, 0xc0,
		0xe0, 0x68, 0x60, 0xe0, 0xa4, 0x2c, 0x24, 0xa4,
		0xa0, 0x28, 0x20, 0xa0, 0xe0, 0x68, 0x60, 0xe0,
		0xf0, 0x78, 0x70, 0xf0, 0xb4, 0x3c, 0x34, 0xb4,
		0xb0, 0x38, 0x30, 0xb0, 0xf0, 0x78, 0x70, 0xf0,
		0xd2, 0x5a, 0x52, 0xd2, 0x96, 0x1e, 0x16, 0x96,
		0x92, 0x1a, 0x12, 0x92, 0xd2, 0x5a, 0x52, 0xd2,
		0xd0, 0x58, 0x50, 0xd0, 0x94, 0x1c, 0x14, 0x94,
		0x90, 0x18, 0x10, 0x90, 0xd0, 0x58, 0x50, 0xd0,
		0xf0, 0x78, 0x70, 0xf0, 0xb4, 0x3c, 0x34, 0xb4,
		0xb0, 0x38, 0x30, 0xb0, 0xf0, 0x78, 0x70, 0xf0,
	};
	int i;
	u8 c, err = 0;

	for (i = 0; i < 2 * 13; i += 2) {
		err |= biphase_tbl[p[i]] | biphase_tbl[p[i + 1]];
		c = (biphase_tbl[p[i + 1]] & 0xf) | ((biphase_tbl[p[i]] & 0xf) << 4);
		dst[i / 2] = c;
	}
	return err & 0xf0;
}

static int saa711x_decode_wss(u8 *p)
{
	static const int wss_bits[8] = {
		0, 0, 0, 1, 0, 1, 1, 1
	};
	unsigned char parity;
	int wss = 0;
	int i;

	for (i = 0; i < 16; i++) {
		int b1 = wss_bits[p[i] & 7];
		int b2 = wss_bits[(p[i] >> 3) & 7];

		if (b1 == b2)
			return -1;
		wss |= b2 << i;
	}
	parity = wss & 15;
	parity ^= parity >> 2;
	parity ^= parity >> 1;

	if (!(parity & 1))
		return -1;

	return wss;
}

static int saa711x_s_clock_freq(struct v4l2_subdev *sd, u32 freq)
{
	struct saa711x_state *state = to_state(sd);
	u32 acpf;
	u32 acni;
	u32 hz;
	u64 f;
	u8 acc = 0; 	/* reg 0x3a, audio clock control */

	/* Checks for chips that don't have audio clock (saa7111, saa7113) */
	if (!saa711x_has_reg(state->ident, R_30_AUD_MAST_CLK_CYCLES_PER_FIELD))
		return 0;

	v4l2_dbg(1, debug, sd, "set audio clock freq: %d\n", freq);

	/* sanity check */
	if (freq < 32000 || freq > 48000)
		return -EINVAL;

	/* hz is the refresh rate times 100 */
	hz = (state->std & V4L2_STD_525_60) ? 5994 : 5000;
	/* acpf = (256 * freq) / field_frequency == (256 * 100 * freq) / hz */
	acpf = (25600 * freq) / hz;
	/* acni = (256 * freq * 2^23) / crystal_frequency =
		  (freq * 2^(8+23)) / crystal_frequency =
		  (freq << 31) / crystal_frequency */
	f = freq;
	f = f << 31;
	do_div(f, state->crystal_freq);
	acni = f;
	if (state->ucgc) {
		acpf = acpf * state->cgcdiv / 16;
		acni = acni * state->cgcdiv / 16;
		acc = 0x80;
		if (state->cgcdiv == 3)
			acc |= 0x40;
	}
	if (state->apll)
		acc |= 0x08;

	saa711x_write(sd, R_38_CLK_RATIO_AMXCLK_TO_ASCLK, 0x03);
	saa711x_write(sd, R_39_CLK_RATIO_ASCLK_TO_ALRCLK, 0x10);
	saa711x_write(sd, R_3A_AUD_CLK_GEN_BASIC_SETUP, acc);

	saa711x_write(sd, R_30_AUD_MAST_CLK_CYCLES_PER_FIELD, acpf & 0xff);
	saa711x_write(sd, R_30_AUD_MAST_CLK_CYCLES_PER_FIELD+1,
							(acpf >> 8) & 0xff);
	saa711x_write(sd, R_30_AUD_MAST_CLK_CYCLES_PER_FIELD+2,
							(acpf >> 16) & 0x03);

	saa711x_write(sd, R_34_AUD_MAST_CLK_NOMINAL_INC, acni & 0xff);
	saa711x_write(sd, R_34_AUD_MAST_CLK_NOMINAL_INC+1, (acni >> 8) & 0xff);
	saa711x_write(sd, R_34_AUD_MAST_CLK_NOMINAL_INC+2, (acni >> 16) & 0x3f);
	state->audclk_freq = freq;
	return 0;
}

static int saa711x_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct saa711x_state *state = to_state(sd);

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		if (ctrl->value < 0 || ctrl->value > 255) {
			v4l2_err(sd, "invalid brightness setting %d\n", ctrl->value);
			return -ERANGE;
		}

		state->bright = ctrl->value;
		saa711x_write(sd, R_0A_LUMA_BRIGHT_CNTL, state->bright);
		break;

	case V4L2_CID_CONTRAST:
		if (ctrl->value < 0 || ctrl->value > 127) {
			v4l2_err(sd, "invalid contrast setting %d\n", ctrl->value);
			return -ERANGE;
		}

		state->contrast = ctrl->value;
		saa711x_write(sd, R_0B_LUMA_CONTRAST_CNTL, state->contrast);
		break;

	case V4L2_CID_SATURATION:
		if (ctrl->value < 0 || ctrl->value > 127) {
			v4l2_err(sd, "invalid saturation setting %d\n", ctrl->value);
			return -ERANGE;
		}

		state->sat = ctrl->value;
		saa711x_write(sd, R_0C_CHROMA_SAT_CNTL, state->sat);
		break;

	case V4L2_CID_HUE:
		if (ctrl->value < -128 || ctrl->value > 127) {
			v4l2_err(sd, "invalid hue setting %d\n", ctrl->value);
			return -ERANGE;
		}

		state->hue = ctrl->value;
		saa711x_write(sd, R_0D_CHROMA_HUE_CNTL, state->hue);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int saa711x_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct saa711x_state *state = to_state(sd);

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ctrl->value = state->bright;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->value = state->contrast;
		break;
	case V4L2_CID_SATURATION:
		ctrl->value = state->sat;
		break;
	case V4L2_CID_HUE:
		ctrl->value = state->hue;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int saa711x_set_size(struct v4l2_subdev *sd, int width, int height)
{
	struct saa711x_state *state = to_state(sd);
	int HPSC, HFSC;
	int VSCY;
	int res;
	int is_50hz = state->std & V4L2_STD_625_50;
	int Vsrc = is_50hz ? 576 : 480;

	v4l2_dbg(1, debug, sd, "decoder set size to %ix%i\n", width, height);

	/* FIXME need better bounds checking here */
	if ((width < 1) || (width > 1440))
		return -EINVAL;
	if ((height < 1) || (height > Vsrc))
		return -EINVAL;

	if (!saa711x_has_reg(state->ident, R_D0_B_HORIZ_PRESCALING)) {
		/* Decoder only supports 720 columns and 480 or 576 lines */
		if (width != 720)
			return -EINVAL;
		if (height != Vsrc)
			return -EINVAL;
	}

	state->width = width;
	state->height = height;

	if (!saa711x_has_reg(state->ident, R_CC_B_HORIZ_OUTPUT_WINDOW_LENGTH))
		return 0;

	/* probably have a valid size, let's set it */
	/* Set output width/height */
	/* width */

	saa711x_write(sd, R_CC_B_HORIZ_OUTPUT_WINDOW_LENGTH,
					(u8) (width & 0xff));
	saa711x_write(sd, R_CD_B_HORIZ_OUTPUT_WINDOW_LENGTH_MSB,
					(u8) ((width >> 8) & 0xff));

	/* Vertical Scaling uses height/2 */
	res = height / 2;

	/* On 60Hz, it is using a higher Vertical Output Size */
	if (!is_50hz)
		res += (VRES_60HZ - 480) >> 1;

		/* height */
	saa711x_write(sd, R_CE_B_VERT_OUTPUT_WINDOW_LENGTH,
					(u8) (res & 0xff));
	saa711x_write(sd, R_CF_B_VERT_OUTPUT_WINDOW_LENGTH_MSB,
					(u8) ((res >> 8) & 0xff));

	/* Scaling settings */
	/* Hprescaler is floor(inres/outres) */
	HPSC = (int)(720 / width);
	/* 0 is not allowed (div. by zero) */
	HPSC = HPSC ? HPSC : 1;
	HFSC = (int)((1024 * 720) / (HPSC * width));
	/* FIXME hardcodes to "Task B"
	 * write H prescaler integer */
	saa711x_write(sd, R_D0_B_HORIZ_PRESCALING,
				(u8) (HPSC & 0x3f));

	v4l2_dbg(1, debug, sd, "Hpsc: 0x%05x, Hfsc: 0x%05x\n", HPSC, HFSC);
	/* write H fine-scaling (luminance) */
	saa711x_write(sd, R_D8_B_HORIZ_LUMA_SCALING_INC,
				(u8) (HFSC & 0xff));
	saa711x_write(sd, R_D9_B_HORIZ_LUMA_SCALING_INC_MSB,
				(u8) ((HFSC >> 8) & 0xff));
	/* write H fine-scaling (chrominance)
	 * must be lum/2, so i'll just bitshift :) */
	saa711x_write(sd, R_DC_B_HORIZ_CHROMA_SCALING,
				(u8) ((HFSC >> 1) & 0xff));
	saa711x_write(sd, R_DD_B_HORIZ_CHROMA_SCALING_MSB,
				(u8) ((HFSC >> 9) & 0xff));

	VSCY = (int)((1024 * Vsrc) / height);
	v4l2_dbg(1, debug, sd, "Vsrc: %d, Vscy: 0x%05x\n", Vsrc, VSCY);

	/* Correct Contrast and Luminance */
	saa711x_write(sd, R_D5_B_LUMA_CONTRAST_CNTL,
					(u8) (64 * 1024 / VSCY));
	saa711x_write(sd, R_D6_B_CHROMA_SATURATION_CNTL,
					(u8) (64 * 1024 / VSCY));

		/* write V fine-scaling (luminance) */
	saa711x_write(sd, R_E0_B_VERT_LUMA_SCALING_INC,
					(u8) (VSCY & 0xff));
	saa711x_write(sd, R_E1_B_VERT_LUMA_SCALING_INC_MSB,
					(u8) ((VSCY >> 8) & 0xff));
		/* write V fine-scaling (chrominance) */
	saa711x_write(sd, R_E2_B_VERT_CHROMA_SCALING_INC,
					(u8) (VSCY & 0xff));
	saa711x_write(sd, R_E3_B_VERT_CHROMA_SCALING_INC_MSB,
					(u8) ((VSCY >> 8) & 0xff));

	saa711x_writeregs(sd, saa7115_cfg_reset_scaler);

	/* Activates task "B" */
	saa711x_write(sd, R_80_GLOBAL_CNTL_1,
				saa711x_read(sd, R_80_GLOBAL_CNTL_1) | 0x20);

	return 0;
}

static void saa711x_set_v4lstd(struct v4l2_subdev *sd, v4l2_std_id std)
{
	struct saa711x_state *state = to_state(sd);

	/* Prevent unnecessary standard changes. During a standard
	   change the I-Port is temporarily disabled. Any devices
	   reading from that port can get confused.
	   Note that s_std is also used to switch from
	   radio to TV mode, so if a s_std is broadcast to
	   all I2C devices then you do not want to have an unwanted
	   side-effect here. */
	if (std == state->std)
		return;

	state->std = std;

	// This works for NTSC-M, SECAM-L and the 50Hz PAL variants.
	if (std & V4L2_STD_525_60) {
		v4l2_dbg(1, debug, sd, "decoder set standard 60 Hz\n");
		saa711x_writeregs(sd, saa7115_cfg_60hz_video);
		saa711x_set_size(sd, 720, 480);
	} else {
		v4l2_dbg(1, debug, sd, "decoder set standard 50 Hz\n");
		saa711x_writeregs(sd, saa7115_cfg_50hz_video);
		saa711x_set_size(sd, 720, 576);
	}

	/* Register 0E - Bits D6-D4 on NO-AUTO mode
		(SAA7111 and SAA7113 doesn't have auto mode)
	    50 Hz / 625 lines           60 Hz / 525 lines
	000 PAL BGDHI (4.43Mhz)         NTSC M (3.58MHz)
	001 NTSC 4.43 (50 Hz)           PAL 4.43 (60 Hz)
	010 Combination-PAL N (3.58MHz) NTSC 4.43 (60 Hz)
	011 NTSC N (3.58MHz)            PAL M (3.58MHz)
	100 reserved                    NTSC-Japan (3.58MHz)
	*/
	if (state->ident == V4L2_IDENT_SAA7111 ||
	    state->ident == V4L2_IDENT_SAA7113) {
		u8 reg = saa711x_read(sd, R_0E_CHROMA_CNTL_1) & 0x8f;

		if (std == V4L2_STD_PAL_M) {
			reg |= 0x30;
		} else if (std == V4L2_STD_PAL_Nc) {
			reg |= 0x20;
		} else if (std == V4L2_STD_PAL_60) {
			reg |= 0x10;
		} else if (std == V4L2_STD_NTSC_M_JP) {
			reg |= 0x40;
		} else if (std & V4L2_STD_SECAM) {
			reg |= 0x50;
		}
		saa711x_write(sd, R_0E_CHROMA_CNTL_1, reg);
	} else {
		/* restart task B if needed */
		int taskb = saa711x_read(sd, R_80_GLOBAL_CNTL_1) & 0x10;

		if (taskb && state->ident == V4L2_IDENT_SAA7114) {
			saa711x_writeregs(sd, saa7115_cfg_vbi_on);
		}

		/* switch audio mode too! */
		saa711x_s_clock_freq(sd, state->audclk_freq);
	}
}

/* setup the sliced VBI lcr registers according to the sliced VBI format */
static void saa711x_set_lcr(struct v4l2_subdev *sd, struct v4l2_sliced_vbi_format *fmt)
{
	struct saa711x_state *state = to_state(sd);
	int is_50hz = (state->std & V4L2_STD_625_50);
	u8 lcr[24];
	int i, x;

#if 1
	/* saa7113/7114/7118 VBI support are experimental */
	if (!saa711x_has_reg(state->ident, R_41_LCR_BASE))
		return;

#else
	/* SAA7113 and SAA7118 also should support VBI - Need testing */
	if (state->ident != V4L2_IDENT_SAA7115)
		return;
#endif

	for (i = 0; i <= 23; i++)
		lcr[i] = 0xff;

	if (fmt == NULL) {
		/* raw VBI */
		if (is_50hz)
			for (i = 6; i <= 23; i++)
				lcr[i] = 0xdd;
		else
			for (i = 10; i <= 21; i++)
				lcr[i] = 0xdd;
	} else {
		/* sliced VBI */
		/* first clear lines that cannot be captured */
		if (is_50hz) {
			for (i = 0; i <= 5; i++)
				fmt->service_lines[0][i] =
					fmt->service_lines[1][i] = 0;
		}
		else {
			for (i = 0; i <= 9; i++)
				fmt->service_lines[0][i] =
					fmt->service_lines[1][i] = 0;
			for (i = 22; i <= 23; i++)
				fmt->service_lines[0][i] =
					fmt->service_lines[1][i] = 0;
		}

		/* Now set the lcr values according to the specified service */
		for (i = 6; i <= 23; i++) {
			lcr[i] = 0;
			for (x = 0; x <= 1; x++) {
				switch (fmt->service_lines[1-x][i]) {
					case 0:
						lcr[i] |= 0xf << (4 * x);
						break;
					case V4L2_SLICED_TELETEXT_B:
						lcr[i] |= 1 << (4 * x);
						break;
					case V4L2_SLICED_CAPTION_525:
						lcr[i] |= 4 << (4 * x);
						break;
					case V4L2_SLICED_WSS_625:
						lcr[i] |= 5 << (4 * x);
						break;
					case V4L2_SLICED_VPS:
						lcr[i] |= 7 << (4 * x);
						break;
				}
			}
		}
	}

	/* write the lcr registers */
	for (i = 2; i <= 23; i++) {
		saa711x_write(sd, i - 2 + R_41_LCR_BASE, lcr[i]);
	}

	/* enable/disable raw VBI capturing */
	saa711x_writeregs(sd, fmt == NULL ?
				saa7115_cfg_vbi_on :
				saa7115_cfg_vbi_off);
}

static int saa711x_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	static u16 lcr2vbi[] = {
		0, V4L2_SLICED_TELETEXT_B, 0,	/* 1 */
		0, V4L2_SLICED_CAPTION_525,	/* 4 */
		V4L2_SLICED_WSS_625, 0,		/* 5 */
		V4L2_SLICED_VPS, 0, 0, 0, 0,	/* 7 */
		0, 0, 0, 0
	};
	struct v4l2_sliced_vbi_format *sliced = &fmt->fmt.sliced;
	int i;

	if (fmt->type != V4L2_BUF_TYPE_SLICED_VBI_CAPTURE)
		return -EINVAL;
	memset(sliced, 0, sizeof(*sliced));
	/* done if using raw VBI */
	if (saa711x_read(sd, R_80_GLOBAL_CNTL_1) & 0x10)
		return 0;
	for (i = 2; i <= 23; i++) {
		u8 v = saa711x_read(sd, i - 2 + R_41_LCR_BASE);

		sliced->service_lines[0][i] = lcr2vbi[v >> 4];
		sliced->service_lines[1][i] = lcr2vbi[v & 0xf];
		sliced->service_set |=
			sliced->service_lines[0][i] | sliced->service_lines[1][i];
	}
	return 0;
}

static int saa711x_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	if (fmt->type == V4L2_BUF_TYPE_SLICED_VBI_CAPTURE) {
		saa711x_set_lcr(sd, &fmt->fmt.sliced);
		return 0;
	}
	if (fmt->type == V4L2_BUF_TYPE_VBI_CAPTURE) {
		saa711x_set_lcr(sd, NULL);
		return 0;
	}
	if (fmt->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	return saa711x_set_size(sd, fmt->fmt.pix.width, fmt->fmt.pix.height);
}

/* Decode the sliced VBI data stream as created by the saa7115.
   The format is described in the saa7115 datasheet in Tables 25 and 26
   and in Figure 33.
   The current implementation uses SAV/EAV codes and not the ancillary data
   headers. The vbi->p pointer points to the R_5E_SDID byte right after the SAV
   code. */
static int saa711x_decode_vbi_line(struct v4l2_subdev *sd, struct v4l2_decode_vbi_line *vbi)
{
	struct saa711x_state *state = to_state(sd);
	static const char vbi_no_data_pattern[] = {
		0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0
	};
	u8 *p = vbi->p;
	u32 wss;
	int id1, id2;   /* the ID1 and ID2 bytes from the internal header */

	vbi->type = 0;  /* mark result as a failure */
	id1 = p[2];
	id2 = p[3];
	/* Note: the field bit is inverted for 60 Hz video */
	if (state->std & V4L2_STD_525_60)
		id1 ^= 0x40;

	/* Skip internal header, p now points to the start of the payload */
	p += 4;
	vbi->p = p;

	/* calculate field and line number of the VBI packet (1-23) */
	vbi->is_second_field = ((id1 & 0x40) != 0);
	vbi->line = (id1 & 0x3f) << 3;
	vbi->line |= (id2 & 0x70) >> 4;

	/* Obtain data type */
	id2 &= 0xf;

	/* If the VBI slicer does not detect any signal it will fill up
	   the payload buffer with 0xa0 bytes. */
	if (!memcmp(p, vbi_no_data_pattern, sizeof(vbi_no_data_pattern)))
		return 0;

	/* decode payloads */
	switch (id2) {
	case 1:
		vbi->type = V4L2_SLICED_TELETEXT_B;
		break;
	case 4:
		if (!saa711x_odd_parity(p[0]) || !saa711x_odd_parity(p[1]))
			return 0;
		vbi->type = V4L2_SLICED_CAPTION_525;
		break;
	case 5:
		wss = saa711x_decode_wss(p);
		if (wss == -1)
			return 0;
		p[0] = wss & 0xff;
		p[1] = wss >> 8;
		vbi->type = V4L2_SLICED_WSS_625;
		break;
	case 7:
		if (saa711x_decode_vps(p, p) != 0)
			return 0;
		vbi->type = V4L2_SLICED_VPS;
		break;
	default:
		break;
	}
	return 0;
}

/* ============ SAA7115 AUDIO settings (end) ============= */

static int saa711x_g_tuner(struct v4l2_subdev *sd, struct v4l2_tuner *vt)
{
	struct saa711x_state *state = to_state(sd);
	int status;

	if (state->radio)
		return 0;
	status = saa711x_read(sd, R_1F_STATUS_BYTE_2_VD_DEC);

	v4l2_dbg(1, debug, sd, "status: 0x%02x\n", status);
	vt->signal = ((status & (1 << 6)) == 0) ? 0xffff : 0x0;
	return 0;
}

static int saa711x_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	switch (qc->id) {
	case V4L2_CID_BRIGHTNESS:
		return v4l2_ctrl_query_fill(qc, 0, 255, 1, 128);
	case V4L2_CID_CONTRAST:
	case V4L2_CID_SATURATION:
		return v4l2_ctrl_query_fill(qc, 0, 127, 1, 64);
	case V4L2_CID_HUE:
		return v4l2_ctrl_query_fill(qc, -128, 127, 1, 0);
	default:
		return -EINVAL;
	}
}

static int saa711x_s_std(struct v4l2_subdev *sd, v4l2_std_id std)
{
	struct saa711x_state *state = to_state(sd);

	state->radio = 0;
	saa711x_set_v4lstd(sd, std);
	return 0;
}

static int saa711x_s_radio(struct v4l2_subdev *sd)
{
	struct saa711x_state *state = to_state(sd);

	state->radio = 1;
	return 0;
}

static int saa711x_s_routing(struct v4l2_subdev *sd,
			     u32 input, u32 output, u32 config)
{
	struct saa711x_state *state = to_state(sd);
	u8 mask = (state->ident == V4L2_IDENT_SAA7111) ? 0xf8 : 0xf0;

	v4l2_dbg(1, debug, sd, "decoder set input %d output %d\n",
		input, output);

	/* saa7111/3 does not have these inputs */
	if ((state->ident == V4L2_IDENT_SAA7113 ||
	     state->ident == V4L2_IDENT_SAA7111) &&
	    (input == SAA7115_COMPOSITE4 ||
	     input == SAA7115_COMPOSITE5)) {
		return -EINVAL;
	}
	if (input > SAA7115_SVIDEO3)
		return -EINVAL;
	if (output > SAA7115_IPORT_ON)
		return -EINVAL;
	if (state->input == input && state->output == output)
		return 0;
	v4l2_dbg(1, debug, sd, "now setting %s input %s output\n",
		(input >= SAA7115_SVIDEO0) ? "S-Video" : "Composite",
		(output == SAA7115_IPORT_ON) ? "iport on" : "iport off");
	state->input = input;

	/* saa7111 has slightly different input numbering */
	if (state->ident == V4L2_IDENT_SAA7111) {
		if (input >= SAA7115_COMPOSITE4)
			input -= 2;
		/* saa7111 specific */
		saa711x_write(sd, R_10_CHROMA_CNTL_2,
				(saa711x_read(sd, R_10_CHROMA_CNTL_2) & 0x3f) |
				((output & 0xc0) ^ 0x40));
		saa711x_write(sd, R_13_RT_X_PORT_OUT_CNTL,
				(saa711x_read(sd, R_13_RT_X_PORT_OUT_CNTL) & 0xf0) |
				((output & 2) ? 0x0a : 0));
	}

	/* select mode */
	saa711x_write(sd, R_02_INPUT_CNTL_1,
		      (saa711x_read(sd, R_02_INPUT_CNTL_1) & mask) |
		       input);

	/* bypass chrominance trap for S-Video modes */
	saa711x_write(sd, R_09_LUMA_CNTL,
			(saa711x_read(sd, R_09_LUMA_CNTL) & 0x7f) |
			(state->input >= SAA7115_SVIDEO0 ? 0x80 : 0x0));

	state->output = output;
	if (state->ident == V4L2_IDENT_SAA7114 ||
			state->ident == V4L2_IDENT_SAA7115) {
		saa711x_write(sd, R_83_X_PORT_I_O_ENA_AND_OUT_CLK,
				(saa711x_read(sd, R_83_X_PORT_I_O_ENA_AND_OUT_CLK) & 0xfe) |
				(state->output & 0x01));
	}
	return 0;
}

static int saa711x_s_gpio(struct v4l2_subdev *sd, u32 val)
{
	struct saa711x_state *state = to_state(sd);

	if (state->ident != V4L2_IDENT_SAA7111)
		return -EINVAL;
	saa711x_write(sd, 0x11, (saa711x_read(sd, 0x11) & 0x7f) |
		(val ? 0x80 : 0));
	return 0;
}

static int saa711x_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct saa711x_state *state = to_state(sd);

	v4l2_dbg(1, debug, sd, "%s output\n",
			enable ? "enable" : "disable");

	if (state->enable == enable)
		return 0;
	state->enable = enable;
	if (!saa711x_has_reg(state->ident, R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED))
		return 0;
	saa711x_write(sd, R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED, state->enable);
	return 0;
}

static int saa711x_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	struct saa711x_state *state = to_state(sd);

	if (freq != SAA7115_FREQ_32_11_MHZ && freq != SAA7115_FREQ_24_576_MHZ)
		return -EINVAL;
	state->crystal_freq = freq;
	state->cgcdiv = (flags & SAA7115_FREQ_FL_CGCDIV) ? 3 : 4;
	state->ucgc = (flags & SAA7115_FREQ_FL_UCGC) ? 1 : 0;
	state->apll = (flags & SAA7115_FREQ_FL_APLL) ? 1 : 0;
	saa711x_s_clock_freq(sd, state->audclk_freq);
	return 0;
}

static int saa711x_reset(struct v4l2_subdev *sd, u32 val)
{
	v4l2_dbg(1, debug, sd, "decoder RESET\n");
	saa711x_writeregs(sd, saa7115_cfg_reset_scaler);
	return 0;
}

static int saa711x_g_vbi_data(struct v4l2_subdev *sd, struct v4l2_sliced_vbi_data *data)
{
	/* Note: the internal field ID is inverted for NTSC,
	   so data->field 0 maps to the saa7115 even field,
	   whereas for PAL it maps to the saa7115 odd field. */
	switch (data->id) {
	case V4L2_SLICED_WSS_625:
		if (saa711x_read(sd, 0x6b) & 0xc0)
			return -EIO;
		data->data[0] = saa711x_read(sd, 0x6c);
		data->data[1] = saa711x_read(sd, 0x6d);
		return 0;
	case V4L2_SLICED_CAPTION_525:
		if (data->field == 0) {
			/* CC */
			if (saa711x_read(sd, 0x66) & 0x30)
				return -EIO;
			data->data[0] = saa711x_read(sd, 0x69);
			data->data[1] = saa711x_read(sd, 0x6a);
			return 0;
		}
		/* XDS */
		if (saa711x_read(sd, 0x66) & 0xc0)
			return -EIO;
		data->data[0] = saa711x_read(sd, 0x67);
		data->data[1] = saa711x_read(sd, 0x68);
		return 0;
	default:
		return -EINVAL;
	}
}

static int saa711x_querystd(struct v4l2_subdev *sd, v4l2_std_id *std)
{
	struct saa711x_state *state = to_state(sd);
	int reg1e;

	*std = V4L2_STD_ALL;
	if (state->ident != V4L2_IDENT_SAA7115)
		return 0;
	reg1e = saa711x_read(sd, R_1E_STATUS_BYTE_1_VD_DEC);

	switch (reg1e & 0x03) {
	case 1:
		*std = V4L2_STD_NTSC;
		break;
	case 2:
		*std = V4L2_STD_PAL;
		break;
	case 3:
		*std = V4L2_STD_SECAM;
		break;
	default:
		break;
	}
	return 0;
}

static int saa711x_g_input_status(struct v4l2_subdev *sd, u32 *status)
{
	struct saa711x_state *state = to_state(sd);
	int reg1e = 0x80;
	int reg1f;

	*status = V4L2_IN_ST_NO_SIGNAL;
	if (state->ident == V4L2_IDENT_SAA7115)
		reg1e = saa711x_read(sd, R_1E_STATUS_BYTE_1_VD_DEC);
	reg1f = saa711x_read(sd, R_1F_STATUS_BYTE_2_VD_DEC);
	if ((reg1f & 0xc1) == 0x81 && (reg1e & 0xc0) == 0x80)
		*status = 0;
	return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int saa711x_g_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	reg->val = saa711x_read(sd, reg->reg & 0xff);
	reg->size = 1;
	return 0;
}

static int saa711x_s_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	saa711x_write(sd, reg->reg & 0xff, reg->val & 0xff);
	return 0;
}
#endif

static int saa711x_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct saa711x_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, state->ident, 0);
}

static int saa711x_log_status(struct v4l2_subdev *sd)
{
	struct saa711x_state *state = to_state(sd);
	int reg1e, reg1f;
	int signalOk;
	int vcr;

	v4l2_info(sd, "Audio frequency: %d Hz\n", state->audclk_freq);
	if (state->ident != V4L2_IDENT_SAA7115) {
		/* status for the saa7114 */
		reg1f = saa711x_read(sd, R_1F_STATUS_BYTE_2_VD_DEC);
		signalOk = (reg1f & 0xc1) == 0x81;
		v4l2_info(sd, "Video signal:    %s\n", signalOk ? "ok" : "bad");
		v4l2_info(sd, "Frequency:       %s\n", (reg1f & 0x20) ? "60 Hz" : "50 Hz");
		return 0;
	}

	/* status for the saa7115 */
	reg1e = saa711x_read(sd, R_1E_STATUS_BYTE_1_VD_DEC);
	reg1f = saa711x_read(sd, R_1F_STATUS_BYTE_2_VD_DEC);

	signalOk = (reg1f & 0xc1) == 0x81 && (reg1e & 0xc0) == 0x80;
	vcr = !(reg1f & 0x10);

	if (state->input >= 6)
		v4l2_info(sd, "Input:           S-Video %d\n", state->input - 6);
	else
		v4l2_info(sd, "Input:           Composite %d\n", state->input);
	v4l2_info(sd, "Video signal:    %s\n", signalOk ? (vcr ? "VCR" : "broadcast/DVD") : "bad");
	v4l2_info(sd, "Frequency:       %s\n", (reg1f & 0x20) ? "60 Hz" : "50 Hz");

	switch (reg1e & 0x03) {
	case 1:
		v4l2_info(sd, "Detected format: NTSC\n");
		break;
	case 2:
		v4l2_info(sd, "Detected format: PAL\n");
		break;
	case 3:
		v4l2_info(sd, "Detected format: SECAM\n");
		break;
	default:
		v4l2_info(sd, "Detected format: BW/No color\n");
		break;
	}
	v4l2_info(sd, "Width, Height:   %d, %d\n", state->width, state->height);
	return 0;
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops saa711x_core_ops = {
	.log_status = saa711x_log_status,
	.g_chip_ident = saa711x_g_chip_ident,
	.g_ctrl = saa711x_g_ctrl,
	.s_ctrl = saa711x_s_ctrl,
	.queryctrl = saa711x_queryctrl,
	.s_std = saa711x_s_std,
	.reset = saa711x_reset,
	.s_gpio = saa711x_s_gpio,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = saa711x_g_register,
	.s_register = saa711x_s_register,
#endif
};

static const struct v4l2_subdev_tuner_ops saa711x_tuner_ops = {
	.s_radio = saa711x_s_radio,
	.g_tuner = saa711x_g_tuner,
};

static const struct v4l2_subdev_audio_ops saa711x_audio_ops = {
	.s_clock_freq = saa711x_s_clock_freq,
};

static const struct v4l2_subdev_video_ops saa711x_video_ops = {
	.s_routing = saa711x_s_routing,
	.s_crystal_freq = saa711x_s_crystal_freq,
	.g_fmt = saa711x_g_fmt,
	.s_fmt = saa711x_s_fmt,
	.g_vbi_data = saa711x_g_vbi_data,
	.decode_vbi_line = saa711x_decode_vbi_line,
	.s_stream = saa711x_s_stream,
	.querystd = saa711x_querystd,
	.g_input_status = saa711x_g_input_status,
};

static const struct v4l2_subdev_ops saa711x_ops = {
	.core = &saa711x_core_ops,
	.tuner = &saa711x_tuner_ops,
	.audio = &saa711x_audio_ops,
	.video = &saa711x_video_ops,
};

/* ----------------------------------------------------------------------- */

static int saa711x_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct saa711x_state *state;
	struct v4l2_subdev *sd;
	int	i;
	char	name[17];
	char chip_id;
	int autodetect = !id || id->driver_data == 1;

	/* Check if the adapter supports the needed features */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	for (i = 0; i < 0x0f; i++) {
		i2c_smbus_write_byte_data(client, 0, i);
		name[i] = (i2c_smbus_read_byte_data(client, 0) & 0x0f) + '0';
		if (name[i] > '9')
			name[i] += 'a' - '9' - 1;
	}
	name[i] = '\0';

	chip_id = name[5];

	/* Check whether this chip is part of the saa711x series */
	if (memcmp(name, "1f711", 5)) {
		v4l_dbg(1, debug, client, "chip found @ 0x%x (ID %s) does not match a known saa711x chip.\n",
			client->addr << 1, name);
		return -ENODEV;
	}

	/* Safety check */
	if (!autodetect && id->name[6] != chip_id) {
		v4l_warn(client, "found saa711%c while %s was expected\n",
			 chip_id, id->name);
	}
	snprintf(client->name, sizeof(client->name), "saa711%c", chip_id);
	v4l_info(client, "saa711%c found (%s) @ 0x%x (%s)\n", chip_id, name,
		 client->addr << 1, client->adapter->name);

	state = kzalloc(sizeof(struct saa711x_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;
	sd = &state->sd;
	v4l2_i2c_subdev_init(sd, client, &saa711x_ops);
	state->input = -1;
	state->output = SAA7115_IPORT_ON;
	state->enable = 1;
	state->radio = 0;
	state->bright = 128;
	state->contrast = 64;
	state->hue = 0;
	state->sat = 64;
	switch (chip_id) {
	case '1':
		state->ident = V4L2_IDENT_SAA7111;
		break;
	case '3':
		state->ident = V4L2_IDENT_SAA7113;
		break;
	case '4':
		state->ident = V4L2_IDENT_SAA7114;
		break;
	case '5':
		state->ident = V4L2_IDENT_SAA7115;
		break;
	case '8':
		state->ident = V4L2_IDENT_SAA7118;
		break;
	default:
		state->ident = V4L2_IDENT_SAA7111;
		v4l2_info(sd, "WARNING: Chip is not known - Falling back to saa7111\n");

	}

	state->audclk_freq = 48000;

	v4l2_dbg(1, debug, sd, "writing init values\n");

	/* init to 60hz/48khz */
	state->crystal_freq = SAA7115_FREQ_24_576_MHZ;
	switch (state->ident) {
	case V4L2_IDENT_SAA7111:
		saa711x_writeregs(sd, saa7111_init);
		break;
	case V4L2_IDENT_SAA7113:
		saa711x_writeregs(sd, saa7113_init);
		break;
	default:
		state->crystal_freq = SAA7115_FREQ_32_11_MHZ;
		saa711x_writeregs(sd, saa7115_init_auto_input);
	}
	if (state->ident != V4L2_IDENT_SAA7111)
		saa711x_writeregs(sd, saa7115_init_misc);
	saa711x_set_v4lstd(sd, V4L2_STD_NTSC);

	v4l2_dbg(1, debug, sd, "status: (1E) 0x%02x, (1F) 0x%02x\n",
		saa711x_read(sd, R_1E_STATUS_BYTE_1_VD_DEC),
		saa711x_read(sd, R_1F_STATUS_BYTE_2_VD_DEC));
	return 0;
}

/* ----------------------------------------------------------------------- */

static int saa711x_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id saa7115_id[] = {
	{ "saa7115_auto", 1 }, /* autodetect */
	{ "saa7111", 0 },
	{ "saa7113", 0 },
	{ "saa7114", 0 },
	{ "saa7115", 0 },
	{ "saa7118", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, saa7115_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "saa7115",
	.probe = saa711x_probe,
	.remove = saa711x_remove,
	.id_table = saa7115_id,
};
                                                                                                                                                                                                                                                                                                                                                                                      /*
 * EHCI HCD (Host Controller Driver) PCI Bus Glue.
 *
 * Copyright (c) 2000-2004 by David Brownell
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

#ifndef CONFIG_PCI
#error "This file is PCI bus glue.  CONFIG_PCI must be defined."
#endif

/*-------------------------------------------------------------------------*/

/* called after powerup, by probe or system-pm "wakeup" */
static int ehci_pci_reinit(struct ehci_hcd *ehci, struct pci_dev *pdev)
{
	u32			temp;
	int			retval;

	/* optional debug port, normally in the first BAR */
	temp = pci_find_capability(pdev, 0x0a);
	if (temp) {
		pci_read_config_dword(pdev, temp, &temp);
		temp >>= 16;
		if ((temp & (3 << 13)) == (1 << 13)) {
			temp &= 0x1fff;
			ehci->debug = ehci_to_hcd(ehci)->regs + temp;
			temp = ehci_readl(ehci, &ehci->debug->control);
			ehci_info(ehci, "debug port %d%s\n",
				HCS_DEBUG_PORT(ehci->hcs_params),
				(temp & DBGP_ENABLED)
					? " IN USE"
					: "");
			if (!(temp & DBGP_ENABLED))
				ehci->debug = NULL;
		}
	}

	/* we expect static quirk code to handle the "extended capabilities"
	 * (currently just BIOS handoff) allowed starting with EHCI 0.96
	 */

	/* PCI Memory-Write-Invalidate cycle support is optional (uncommon) */
	retval = pci_set_mwi(pdev);
	if (!retval)
		ehci_dbg(ehci, "MWI active\n");

	return 0;
}

/* called during probe() after chip reset completes */
static int ehci_pci_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci(hcd);
	struct pci_dev		*pdev = to_pci_dev(hcd->self.controller);
	struct pci_dev		*p_smbus;
	u8			rev;
	u32			temp;
	int			retval;

	switch (pdev->vendor) {
	case PCI_VENDOR_ID_TOSHIBA_2:
		/* celleb's companion chip */
		if (pdev->device == 0x01b5) {
#ifdef CONFIG_USB_EHCI_BIG_ENDIAN_MMIO
			ehci->big_endian_mmio = 1;
#else
			ehci_warn(ehci,
				  "unsupported big endian Toshiba quirk\n");
#endif
		}
		break;
	}

	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs +
		HC_LENGTH(ehci_readl(ehci, &ehci->caps->hc_capbase));

	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

        /* ehci_init() causes memory for DMA transfers to be
         * allocated.  Thus, any vendor-specific workarounds based on
         * limiting the type of memory used for DMA transfers must
         * happen before ehci_init() is called. */
	switch (pdev->vendor) {
	case PCI_VENDOR_ID_NVIDIA:
		/* NVidia reports that certain chips don't handle
		 * QH, ITD, or SITD addresses above 2GB.  (But TD,
		 * data buffer, and periodic schedule are normal.)
		 */
		switch (pdev->device) {
		case 0x003c:	/* MCP04 */
		case 0x005b:	/* CK804 */
		case 0x00d8:	/* CK8 */
		case 0x00e8:	/* CK8S */
			if (pci_set_consistent_dma_mask(pdev,
						DMA_BIT_MASK(31)) < 0)
				ehci_warn(ehci, "can't enable NVidia "
					"workaround for >2GB RAM\n");
			break;
		}
		break;
	}

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	retval = ehci_halt(ehci);
	if (retval)
		return retval;

	/* data structure init */
	retval = ehci_init(hcd);
	if (retval)
		return retval;

	switch (pdev->vendor) {
	case PCI_VENDOR_ID_TDI:
		if (pdev->device == PCI_DEVICE_ID_TDI_EHCI) {
			hcd->has_tt = 1;
			tdi_reset(ehci);
		}
		break;
	case PCI_VENDOR_ID_AMD:
		/* AMD8111 EHCI doesn't work, according to AMD errata */
		if (pdev->device == 0x7463) {
			ehci_info(ehci, "ignoring AMD8111 (errata)\n");
			retval = -EIO;
			goto done;
		}
		break;
	case PCI_VENDOR_ID_NVIDIA:
		switch (pdev->device) {
		/* Some NForce2 chips have problems with selective suspend;
		 * fixed in newer silicon.
		 */
		case 0x0068:
			if (pdev->revision < 0xa4)
				ehci->no_selective_suspend = 1;
			break;
		}
		break;
	case PCI_VENDOR_ID_VIA:
		if (pdev->device == 0x3104 && (pdev->revision & 0xf0) == 0x60) {
			u8 tmp;

			/* The VT6212 defaults to a 1 usec EHCI sleep time which
			 * hogs the PCI bus *badly*. Setting bit 5 of 0x4B makes
			 * that sleep time use the conventional 10 usec.
			 */
			pci_read_config_byte(pdev, 0x4b, &tmp);
			if (tmp & 0x20)
				break;
			pci_write_config_byte(pdev, 0x4b, tmp | 0x20);
		}
		break;
	case PCI_VENDOR_ID_ATI:
		/* SB600 and old version of SB700 have a bug in EHCI controller,
		 * which causes usb devices lose response in some cases.
		 */
		if ((pdev->device == 0x4386) || (pdev->device == 0x4396)) {
			p_smbus = pci_get_device(PCI_VENDOR_ID_ATI,
						 PCI_DEVICE_ID_ATI_SBX00_SMBUS,
						 NULL);
			if (!p_smbus)
				break;
			rev = p_smbus->revision;
			if ((pdev->device == 0x4386) || (rev == 0x3a)
			    || (rev == 0x3b)) {
				u8 tmp;
				ehci_info(ehci, "applying AMD SB600/SB700 USB "
					"freeze workaround\n");
				pci_read_config_byte(pdev, 0x53, &tmp);
				pci_write_config_byte(pdev, 0x53, tmp | (1<<3));
			}
			pci_dev_put(p_smbus);
		}
		break;
	}

	ehci_reset(ehci);

	/* at least the Genesys GL880S needs fixup here */
	temp = HCS_N_CC(ehci->hcs_params) * HCS_N_PCC(ehci->hcs_params);
	temp &= 0x0f;
	if (temp && HCS_N_PORTS(ehci->hcs_params) > temp) {
		ehci_dbg(ehci, "bogus port configuration: "
			"cc=%d x pcc=%d < ports=%d\n",
			HCS_N_CC(ehci->hcs_params),
			HCS_N_PCC(ehci->hcs_params),
			HCS_N_PORTS(ehci->hcs_params));

		switch (pdev->vendor) {
		case 0x17a0:		/* GENESYS */
			/* GL880S: should be PORTS=2 */
			temp |= (ehci->hcs_params & ~0xf);
			ehci->hcs_params = temp;
			break;
		case PCI_VENDOR_ID_NVIDIA:
			/* NF4: should be PCC=10 */
			break;
		}
	}

	/* Serial Bus Release Number is at PCI 0x60 offset */
	pci_read_config_byte(pdev, 0x60, &ehci->sbrn);

	/* Keep this around for a while just in case some EHCI
	 * implementation uses legacy PCI PM support.  This test
	 * can be removed on 17 Dec 2009 if the dev_warn() hasn't
	 * been triggered by then.
	 */
	if (!device_can_wakeup(&pdev->dev)) {
		u16	port_wake;

		pci_read_config_word(pdev, 0x62, &port_wake);
		if (port_wake & 0x0001) {
			dev_warn(&pdev->dev, "Enabling legacy PCI PM\n");
			device_set_wakeup_capable(&pdev->dev, 1);
		}
	}

#ifdef	CONFIG_USB_SUSPEND
	/* REVISIT: the controller works fine for wakeup iff the root hub
	 * itself is "globally" suspended, but usbcore currently doesn't
	 * understand such things.
	 *
	 * System suspend currently expects to be able to suspend the entire
	 * device tree, device-at-a-time.  If we failed selective suspend
	 * reports, system suspend would fail; so the root hub code must claim
	 * success.  That's lying to usbcore, and it matters for for runtime
	 * PM scenarios with selective suspend and remote wakeup...
	 */
	if (ehci->no_selective_suspend && device_can_wakeup(&pdev->dev))
		ehci_warn(ehci, "selective suspend/wakeup unavailable\n");
#endif

	ehci_port_power(ehci, 1);
	retval = ehci_pci_reinit(ehci, pdev);
done:
	return retval;
}

/*-------------------------------------------------------------------------*/

#ifdef	CONFIG_PM

/* suspend/resume, section 4.3 */

/* These routines rely on the PCI bus glue
 * to handle powerdown and wakeup, and currently also on
 * transceivers that don't need any software attention to set up
 * the right sort of wakeup.
 * Also they depend on separate root hub suspend/resume.
 */

static int ehci_pci_suspend(struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci(hcd);
	unsigned long		flags;
	int			rc = 0;

	if (time_before(jiffies, ehci->next_statechange))
		msleep(10);

	/* Root hub was already suspended. Disable irq emission and
	 * mark HW unaccessible, bail out if RH has been resumed. Use
	 * the spinlock to properly synchronize with possible pending
	 * RH suspend or dspm))
		return -EBUSY;
	val = ucontrol->value.integer.value[0] & 1;
	spin_lock_irq(&hdspm->lock);
	change = (int) val != hdspm_input_select(hdspm);
	hdspm_set_input_select(hdspm, val);
	spin_unlock_irq(&hdspm->lock);
	return change;
}

#define HDSPM_DS_WIRE(xname, xindex) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .name = xname, \
  .index = xindex, \
  .info = snd_hdspm_info_ds_wire, \
  .get = snd_hdspm_get_ds_wire, \
  .put = snd_hdspm_put_ds_wire \
}

static int hdspm_ds_wire(struct hdspm * hdspm)
{
	return (hdspm->control_register & HDSPM_DS_DoubleWire) ? 1 : 0;
}

static int hdspm_set_ds_wire(struct hdspm * hdspm, int ds)
{
	if (ds)
		hdspm->control_register |= HDSPM_DS_DoubleWire;
	else
		hdspm->control_register &= ~HDSPM_DS_DoubleWire;
	hdspm_write(hdspm, HDSPM_controlRegister, hdspm->control_register);

	return 0;
}

static int snd_hdspm_info_ds_wire(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_info *uinfo)
{
	static char *texts[] = { "Single", "Double" };

	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = 1;
	uinfo->value.enumerated.items = 2;

	if (uinfo->value.enumerated.item >= uinfo->value.enumerated.items)
		uinfo->value.enumerated.item =
		    uinfo->value.enumerated.items - 1;
	strcpy(uinfo->value.enumerated.name,
	       texts[uinfo->value.enumerated.item]);

	return 0;
}

static int snd_hdspm_get_ds_wire(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct hdspm *hdspm = snd_kcontrol_chip(kcontrol);

	spin_lock_irq(&hdspm->lock);
	ucontrol->value.enumerated.item[0] = hdspm_ds_wire(hdspm);
	spin_unlock_irq(&hdspm->lock);
	return 0;
}

static int snd_hdspm_put_ds_wire(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct hdspm *hdspm = snd_kcontrol_chip(kcontrol);
	int change;
	unsigned int val;

	if (!snd_hdspm_use_is_exclusive(hdspm))
		return -EBUSY;
	val = ucontrol->value.integer.value[0] & 1;
	spin_lock_irq(&hdspm->lock);
	change = (int) val != hdspm_ds_wire(hdspm);
	hdspm_set_ds_wire(hdspm, val);
	spin_unlock_irq(&hdspm->lock);
	return change;
}

#define HDSPM_QS_WIRE(xname, xindex) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .name = xname, \
  .index = xindex, \
  .info = snd_hdspm_info_qs_wire, \
  .get = snd_hdspm_get_qs_wire, \
  .put = snd_hdspm_put_qs_wire \
}

static int hdspm_qs_wire(struct hdspm * hdspm)
{
	if (hdspm->control_register & HDSPM_QS_DoubleWire)
		return 1;
	if (hdspm->control_register & HDSPM_QS_QuadWire)
		return 2;
	return 0;
}

static int hdspm_set_qs_wire(struct hdspm * hdspm, int mode)
{
	hdspm->control_register &= ~(HDSPM_QS_DoubleWire | HDSPM_QS_QuadWire);
	switch (mode) {
	case 0:
		break;
	case 1:
		hdspm->control_register |= HDSPM_QS_DoubleWire;
		break;
	case 2:
		hdspm->control_register |= HDSPM_QS_QuadWire;
		break;
	}
	hdspm_write(hdspm, HDSPM_controlRegister, hdspm->control_register);

	return 0;
}

static int snd_hdspm_info_qs_wire(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_info *uinfo)
{
	static char *texts[] = { "Single", "Double", "Quad" };

	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = 1;
	uinfo->value.enumerated.items = 3;

	if (uinfo->value.enumerated.item >= uinfo->value.enumerated.items)
		uinfo->value.enumerated.item =
		    uinfo->value.enumerated.items - 1;
	strcpy(uinfo->value.enumerated.name,
	       texts[uinfo->value.enumerated.item]);

	return 0;
}

static int snd_hdspm_get_qs_wire(struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_value *ucontrol)
{
	struct hdspm *hdspm = snd_kcontrol_chip(kcontrol);

	spin_lock_irq(&hdspm->lock);
	ucontrol->value.enumerated.item[0] = hdspm_qs_wire(hdspm);
	spin_unlock_irq(&hdspm->lock);
	return 0;
}

static int snd_hdspm_put_qs_wire(struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_value *ucontrol)
{
	struct hdspm *hdspm = snd_kcontrol_chip(kcontrol);
	int change;
	int val;

	if (!snd_hdspm_use_is_exclusive(hdspm))
		return -EBUSY;
	val = ucontrol->value.integer.value[0];
	if (val < 0)
		val = 0;
	if (val > 2)
		val = 2;
	spin_lock_irq(&hdspm->lock);
	change = val != /*
 * Copyright (C) 2003-2008 Takahiro Hirofuchi
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */

#include "usbip_common.h"
#include "vhci.h"

#include <linux/in.h>

/* TODO: refine locking ?*/

/* Sysfs entry to show port status */
static ssize_t show_status(struct device *dev, struct device_attribute *attr,
			   char *out)
{
	char *s = out;
	int i = 0;

	BUG_ON(!the_controller || !out);

	spin_lock(&the_controller->lock);

	/*
	 * output example:
	 * prt sta spd dev socket           local_busid
	 * 000 004 000 000         c5a7bb80 1-2.3
	 * 001 004 000 000         d8cee980 2-3.4
	 *
	 * IP address can be retrieved from a socket pointer address by looking
	 * up /proc/net/{tcp,tcp6}. Also, a userland program may remember a
	 * port number and its peer IP address.
	 */
	out += sprintf(out, "prt sta spd bus dev socket           "
		       "local_busid\n");

	for (i = 0; i < VHCI_NPORTS; i++) {
		struct vhci_device *vdev = port_to_vdev(i);

		spin_lock(&vdev->ud.lock);

		out += sprintf(out, "%03u %03u ", i, vdev->ud.status);

		if (vdev->ud.status == VDEV_ST_USED) {
			out += sprintf(out, "%03u %08x ",
					vdev->speed, vdev->devid);
			out += sprintf(out, "%16p ", vdev->ud.tcp_socket);
			out += sprintf(out, "%s", dev_name(&vdev->udev->dev));

		} else
			out += sprintf(out, "000 000 000 0000000000000000 0-0");

		out += sprintf(out, "\n");

		spin_unlock(&vdev->ud.lock);
	}

	spin_unlock(&the_controller->lock);

	return out - s;
}
static DEVICE_ATTR(status, S_IRUGO, show_status, NULL);

/* Sysfs entry to shutdown a virtual connection */
static int vhci_port_disconnect(__u32 rhport)
{
	struct vhci_device *vdev;

	dbg_vhci_sysfs("enter\n");

	/* lock */
	spin_lock(&the_controller->lock);

	vdev = port_to_vdev(rhport);

	spin_lock(&vdev->ud.lock);
	if (vdev->ud.status == VDEV_ST_NULL) {
		uerr("not connected %d\n", vdev->ud.status);

		/* unlock */
		spin_unlock(&vdev->ud.lock);
		spin_unlock(&the_controller->lock);

		return -EINVAL;
	}

	/* unlock */
	spin_unlock(&vdev->ud.lock);
	spin_unlock(&the_controller->lock);

	usbip_event_add(&vdev->ud, VDEV_EVENT_DOWN);

	return 0;
}

static ssize_t store_detach(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int err;
	__u32 rhport = 0;

	sscanf(buf, "%u", &rhport);

	/* check rhport */
	if (rhport >= VHCI_NPORTS) {
		uerr("invalid port %u\n", rhport);
		return -EINVAL;
	}

	err = vhci_port_disconnect(rhport);
	if (err < 0)
		return -EINVAL;

	dbg_vhci_sysfs("Leave\n");
	return count;
}
static DEVICE_ATTR(detach, S_IWUSR, NULL, store_detach);

/* Sysfs entry to establish a virtual connection */
static int valid_args(__u32 rhport, enum usb_device_speed speed)
{
	/* check rhport */
	if ((rhport < 0) || (rhport >= VHCI_NPORTS)) {
		uerr("port %u\n", rhport);
		return -EINVAL;
	}

	/* check speed */
	switch (speed) {
	case USB_SPEED_LOW:
	case USB_SPEED_FULL:
	case USB_SPEED_HIGH:
	case USB_SPEED_VARIABLE:
		break;
	default:
		uerr("speed %d\n", speed);
		return -EINVAL;
	}

	return 0;
}

/*
 * To start a new USB/IP attachment, a userland program needs to setup a TCP
 * connection and then write its socket descriptor with remote device
 * information into this sysfs file.
 *
 * A remote device is virtually attached to the root-hub port of @rhport with
 * @speed. @devid is embedded into a request to specify the remote device in a
 * server host.
 *
 * write() returns 0 on success, else negative errno.
 */
static ssize_t store_attach(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct vhci_device *vdev;
	struct socket *socket;
	int sockfd = 0;
	__u32 rhport = 0, devid = 0, speed = 0;

	/*
	 * @rhport: port number of vhci_hcd
	 * @sockfd: socket descriptor of an established TCP connection
	 * @devid: unique device identifier in a remote host
	 * @speed: usb device speed in a remote host
	 */
	sscanf(buf, "%u %u %u %u", &rhport, &sockfd, &devid, &speed);

	dbg_vhci_sysfs("rhport(%u) sockfd(%u) devid(%u) speed(%u)\n",
			rhport, sockfd, devid, speed);


	/* check received parameters */
	if (valid_args(rhport, speed) < 0)
		return -EINVAL;

	/* check sockfd */
	socket = sockfd_to_socket(sockfd);
	if (!socket)
		return  -EINVAL;

	/* now need lock until setting vdev status as used */

	/* begin a lock */
	spin_lock(&the_controller->lock);

	vdev = port_to_vdev(rhport);

	spin_lock(&vdev->ud.lock);

	if (vdev->ud.status != VDEV_ST_NULL) {
		/* end of the lock */
		spin_unlock(&vdev->ud.lock);
		spin_unlock(&the_controller->lock);

		uerr("port %d already used\n", rhport);
		return -EINVAL;
	}

	uinfo("rhport(%u) sockfd(%d) devid(%u) speed(%u)\n",
			rhport, sockfd, devid, speed);

	vdev->devid         = devid;
	vdev->speed         = speed;
	vdev->ud.tcp_socket = socket;
	vdev->ud.status     = VDEV_ST_NOTASSIGNED;

	spin_unlock(&vdev->ud.lock);
	spin_unlock(&the_controller->lock);
	/* end the lock */

	/*
	 * this function will sleep, so should be out of the lock. but, it's ok
	 * because we already marked vdev as being used. really?
	 */
	usbip_start_threads(&vdev->ud);

	rh_port_connect(rhport, speed);

	return count;
}
static DEVICE_ATTR(attach, S_IWUSR, NULL, store_attach);

static struct attribute *dev_attrs[] = {
	&dev_attr_status.attr,
	&dev_attr_detach.attr,
	&dev_attr_attach.attr,
	&dev_attr_usbip_debug.attr,
	NULL,
};

struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     V�T$�D$�r�J�B�D$9ΉBt�A�)��Ѓ��ȉB�B     �   ^�      �F����9 �D$��������        �fF�����8 �D$��������        �FF�����8 �D$��������        �&F�����8 �D$��������        �F�����8 �D$��������        ��E����g8 �D$��������        S膘����F8 ���D$ ��������$�%����[�        S�V�����8 ���D$ ��������$��$����[�        S�&������7 ���D$ ��������$�$����[�        S������ö7 ���D$ ��������$�}$����[�        S�Ɨ���Æ7 ���D$ ��������$�M$����[�        S薗����V7 ���D$ ��������$�$����[�        S�f�����&7 ���D$ ��������$��#����[�        U��W��VS�/������6 ��,�$�   �~#���l$�ƋD$D�4$�D$�D$@�D$�������D$�f#���7��,[^_]�            VS���t$ �Ζ���Î6 �V�������҉t�N9�t�A�)��Ѓ��ȉF�$�?#�����������[^� VS���t$ �~�����>6 �V�������҉t�N9�t�A�)��Ѓ��ȉF�$��"����������4$��"����[^�         U��WVS����� �E�������5 �u�P9P(sQ�PP0�F�T$�D$�F�@�$�j����ǋF;Ft8��t,�8�F���F��t�F �e��   [^_]Ít& �PP0�1��Սt& +F��������������w��B9�B����$�T$�L$��!���T$�L$��ȅ҉D$t�:�F��+F�T$���)��D$�F�$�L$�D$��!���L$�F�T$�N�L$�����V�N�B����$�!���5���      UWVS���t$0�����ì4 �F(�$�!���F0�n,��9�tc�t$0��&    �u ��t-�V��t�N9�t�A�)��Ѓ��ȉF�$�<!���4$�4!����9�uŋt$0�N,�V09�t�B�)��Ѓ��ЉF0�F<�N8�D$9���   �ωt$0�/����   �U$��t�
�$�Q�u4��t(�U89�t����U8�$�b9���U89�u�u4�4$� ���u(��t)�U,9�tf����U,�$�B9���U,9�u�u(�4$� ���M��t�u9�t�V�)��҃���U�$�[ ���M��t�u9�t�V�)��҃���U�$�6 ���,$�. ����9|$�)����t$0�N8�V<9�t�B�)��Ѓ��ЉF<�~h�Fh    ��t���t;Gt�G�$�����<$�����~d�Fd    ��t���t;Gt�G�$����<$����~`�F`    ��t���t;Gt�G�$����<$����~\�F\    ��t���t;Gt�G�$�\���<$�T���~X�FX    ��t���t;Gt�G�$�0���<$�(���~T�FT    ��t���t;Gt�G�$����<$�����~P�FP    ��t���t;Gt�G�$�����<$�����~L�FL    ��t���t;Gt�G�$����<$����~H�FH    ��t���t;Gt�G�$����<$�x���~D�FD    ��t���t;Gt�G�$�T���<$�L���V8��t�N<9�t�A�)��Ѓ��ȉF<�$�'���V,��t�N09�t�A�)��Ѓ��ȉF0�$����>��t#�V9�t�B�F�B�u��9�u��$������[^_]Ð�B�$������   U��WVS�����p�E�������0 �@��@9D$�  �|$0�ЉT$��� �D$$�@ �@`���D$��  �D$(    ���t$(�E� �t$�t$$�t$�u�4$�����  �D$(�t$$��F8�@���d  F�[  �<$�D$\�D$@    �D$D    �D$H    �D$L    �D$P    �t$T�D$X    �D$`    �(���L$0�T$4�
���  �D$<D$8��t�<$��%���L$0�T$4�D$X9�v=��9ȉD$X�>  �9��,  �D$@D$`�D$`�<$�%���D$X�L$0�T$49�wÍ4
9��E  �t$8�t$,1��9�w|�L$,�9�vr�E�M� �|$�t$�$�P����  �D$X���L$0��9ȉD$X�[  L$49���  �D$@D$`�<$�D$`�#%���T$8�L$0�D$X�T$,�T$4�9�v�T$,9�vI�E�u� �4$�P���  �D$(�D$9D$(�4������D$�D$9D$������   ��  �T$<9�v��E�M� �|$�t$�$�P����  �D$X���T$0��9ЉD$Xs<T$49���  �D$@D$`�<$�D$`�\$���L$8�D$X�T$0�L$,�L$4��8������  �L$4�L$�9ȉL$ ��  �i  �L$8�L$,L$ 9�r�\  L$<9��<  �D$HD$`�  ��&    ��  �T$4�T$�9ЉT$ �V  �/  �T$8�T$,T$ 9�r�"  T$<9��  �D$HD$`�  ��&    ��  �4
9��  ��  t$89�w���  t$<9��}����D$HD$`�}  ���9ȉD$Xs59���   �D$@D$`�D$`�<$�)#���D$X�T$4�L$09�������p���9��P  9ƍ�    �b  f�9���  t$89�r��  t$<9��6����D$HD$`�<$��"���D$X�T$4�L$0뛋D$HD$`�C����v �D$HD$`�l����v �D$HD$`������v �D$HD$`�F����v �L$4���  1Ƀ|$8 u�D$,    �|$< �����D$`    �<$�P"��������v �T$4����   1҃|$8 u�D$,    �|$< ������D$`    �<$�"��������v ����   �t$8��u�t$<��������D$`    �<$��!���D$X�L$0�T$4������v ��t�D$`    �v�����&    9�������D$@D$`�Z����D$@�<$D$`�{!�������D$@�<$D$`�f!���>����D$@D$`�����t& �<$�D$`    �@!��������v �<$�D$`    �(!��� ����v �D$`    �[����v �t$8��u�t$<���\����D$`    �!�����&    �<$�� ���L$0�T$4������v 1��e�[^_]Í�    �L$������L$�B����L$�H����v �T$�����T$�d����T$�j���     WVS��0�t$@荊����M* �F�N�V�xH�|$,�~$�T$�T$,�L$�|$�N�T$�L$�@D�$�/���F(�$�/����0[^_�UWV���|$�D$�W�J�:�$)<$�<$�,$��tJ�t$�L$�|$�vl�i,1ɐ�t& �P�����)ǋD� ��P�D$�����@ P ;$��u֋|$�wl��^_]�        U��WVS豉����q) �����@�E�u�|$�|$�D$    �D$�������D$�E�4$�t$�D$ �����D$,�E�D$$    �D$(    �D$0�3�������   �T$�t$�D$    �D$ ������+E�T$�D$$    ���   �������D$�E�|$�4$�D$(    �D$,�E�D$0������T$��tx�L$�t$�D$    �D$ ������)Љ��   ������D$�E�|$�4$�L$�D$,�E�D$$    �D$(    �D$0�n����L$��t�D$��)ʉ��   �e�[^_]�1���            U��S�S�����( �����@�U�E�D$    �D$ �����T$��L����T$�U�$�D$�D$$    �T$,�U�D$(    �T$0�T$�T$�������t	�D$�]���1���    UWVS���t$0�T$4�D$8�ć���Ä' �F    �F    �F    ��P��t:�P�V�P�@t�P�V�P�@ �F    �V�x`��u��[^_]Í�    ���ōt& �,�    �,$�����t�    ����u�NŋV�L$)ʃ��)ЉT$�ǉ$����F�n�~�n��t��$����  �T$�B�J+B����D�           WVS���t$ �͆���Í& �|$$�D$   �F�D$�<$�0(���F�D$�F�<$�D$�(���F�D$�<$�D$   �(���F�D$�<$�D$   ��'���V�F�<$�T$)Ѓ���D$��'����[^_�          WVS���t$ �-������% �F$��t��$�R�~4��t-�F89�t��    ���F8�$�:+���F89�u�~4�<$����~(��t)�F,9�tf����F,�$�+���F,9�u�~(�<$�X���V��t�N9�t�A�)��Ѓ��ȉF�$�3���V��t�N9�t�A�)��Ѓ��ȉF�$�����[^_�       �T$�L$�B����t��          �D$�T$�@��� V�L$�q �Q,���+Q(�Ѓ�����T�^�             UWVS��,�|$D�����ì$ �t$@����  �D$D� ���D$�  �8���  �/���,  �E ����   ���t9�
�4$�D$�T$�L$�����T$�J�4$�L$�~����T$�$����D$�P��t9�
�4$�D$�T$�L$�Q����T$�J�4$�L$�>����T$�$�����D$�$�����E����   ���t9�
�4$�D$�T$�L$������T$�J�4$�L$������T$�$����D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$�@���D$�$�4���,$�,���o����   �E ����   ���t9�
�4$�D$�T$�L$�Z����T$�J�4$�L$�G����T$�$�����D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$����D$�$����E��t1��4$�D$�T$������D$�P�4$�T$������D$�$�W���,$�O���<$�G���D$�x����  �/����   �E ����   ���t9�
�4$�D$�T$�L$�g����T$�J�4$�L$�T����T$�$�����D$�P��t9�
�4$�D$�T$�L$�'����T$�J�4$�L$�����T$�$����D$�$����E��t1��4$�D$�T$������D$�P�4$�T$������D$�$�d���,$�\���o����   �E ����   ���t9�
�4$�D$�T$�L$�����T$�J�4$�L$�w����T$�$����D$�P��t9�
�4$�D$�T$�L$�J����T$�J�4$�L$�7����T$�$�����D$�$����E��t1��4$�D$�T$�����D$�P�4$�T$������D$�$����,$����<$�w���D$�$�k���D$D�@���D$��  �8���u  �/���,  �E ����   ���t9�
�4$�D$�T$�L$�}����T$�J�4$�L$�j����T$�$�����D$�P��t9�
�4$�D$�T$�L$�=����T$�J�4$�L$�*����T$�$����D$�$����E����   ���t9�
�4$�D$�T$�L$������T$�J�4$�L$������T$�$�l���D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$�,���D$�$� ���,$����o���,  �E ����   ���t9�
�4$�D$�T$�L$�F����T$�J�4$�L$�3����T$�$�����D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$������T$�$����D$�$�{���E����   ���t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$�5���D$�P��t9�
�4$�D$�T$�L$�t����T$�J�4$�L$�a����T$�$��
���D$�$��
���,$��
���<$��
���D$�x���  �/���,  �E ����   ���t9�
�4$�D$�T$�L$������T$�J�4$�L$������T$�$�z
���D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$�:
���D$�$�.
���E����   ���t9�
�4$�D$�T$�L$�g����T$�J�4$�L$�T����T$�$��	���D$�P��t9�
�4$�D$�T$�L$�'����T$�J�4$�L$�����T$�$�	���D$�$�	���,$�	���o����   �E ��t1��4$�D$�T$������D$�P�4$�T$�����D$�$�Q	���E����   ���t9�
�4$�D$�T$�L$�����T$�J�4$�L$�w����T$�$�	���D$�P��t9�
�4$�D$�T$�L$�J����T$�J�4$�L$�7����T$�$�����D$�$����,$����<$����D$�$����D$D�$�����,[^_]�               U��WVS��{���Ñ �����@�u�E�T$�T$�T$�4$�D$������D$0�t$�D$    �D$ �����D$$    �D$(    �|$�D$0    �D$4    �D$,�E����L$0�D$�|$�L$�L$,�$�D$�E����N�D$�T$���    tB�D$��,����T$�4$�t$�D$    �D$ �����D$$    �D$(    �D$������D$�e�[^_]�       UWVS���t$0��z���Ì �~ �n��������t%��,$�D$�����G�,$�D$�����<$�3�����������[^_]�   UWVS���t$0�lz����, �~ �n��������t%��,$�D$�J����G�,$�D$�;����<$������������4$������[^_]�           UWVS��<�D$P��y���ü �t$T��@)���i����̋D$P�@)���i�����=eff��  ��W9�B���  1�1����,�ȅ�D$(t1��E �F�E�F�E�F�    �F    �F    �E�F�E�E�D$$�D$P�p� 9ƉD$�`  �U�}��щՉ����4��    ���F�G�F�G�F��F�B������9t$��   ����l$t��tN�F��
�D$vs�A�D$�d$��D$�T$,�L$ �$�3���L$ �T$,�O�D$�G�D$����D$�L$�T$ �L$�D$�D$�$�(���D$�L$�T$ � �S����v �	��E�D$�f��D$P�l$�8�p�L$$9��(�H�L$(�Ht��&    �n��E u��9�u��t�4$������<[^_]Ð�t& �F��$����Ԑ������$�L$�_���L$�C������D$P닍,���    ��  �L$����q�U��WVSQ��X�1�A�w����~ �E� �u܉E���P��J����   �RH��t
�$�a����H�}ԉ<$�E��L$�W����<������D$�<$�R���<$�E��W����P��BL���t`�Mċz���M��M��}��}��	�D$�E��T$�|$�D$�E��D$�E��$�Q�}ă���t|�E؉$�9���e���Y[^_]�a�Ív �T$�<$�U������@������<$�D$�����D$    �$�Q�<$�E�����E��U����BL��z�����G�����&    ��P��B�$���D$�/���f���          UWVS��L�|$d�t$`�Hv���� ���  �� ����l$�F(�$�����)��Fp�F���    �L  �~ ���  �xD�,$�|$�zw���<$��� ����D$�F(�$�o���F�$��n���VD�҉FDt)���t;Bt�B�$�T$�P���T$�$�D���FD�P+�|$Չ,$�w���<$��蔃���D$�F(�$����F�$��p���VH�҉FHt)���t;Bt�B�$�T$�����T$�$�����FH�P+�|$Չ,$�v���<$���(����D$�F(�$����F�$�q���VL�҉FLt)���t;Bt�B�$�T$�x���T$�$�l���FL�P+�|$Չ,$�6v���<$��輂���D$�F(�$�[���F�$��n���VP�҉FPt)���t;Bt�B�$�T$����T$�$� ���FP�P+�|$Չ,$��u���<$���P����D$�F(�$�����F�$�ds���VT�҉FTt)���t;Bt�B�$�T$� ���T$�$� ���FT�P+�|$Չ,$�^u���<$�������D$�F(�$����F�$�q���VX�҉FXt)���t;Bt�B�$�T$�4 ���T$�$�( ���FX�P+�|$Չ,$��t���<$���x����D$�F(�$�G���F�$�Lv���V\�҉F\t)���t;Bt�B�$�T$������T$�$�����F\�P+�|$Չ,$�t���<$�������D$�F(�$�����F�$�`s���V`�҉F`t)���t;Bt�B�$�T$�\����T$�$�P����F`�P+�|$Չ,$�t���<$��蠀���D$�F(�$����F�$�d;���Vd�҉Fdt)���t;Bt�B�$�T$������T$�$������Fd�P+�|$Չ,$�s���<$���4����D$�F(�$�3���F�$��s���~h���Fht!���t;Gt�G�$�����<$�����Fh�P+��L[^Չ�_]Ít& �D$    �F(�$�E���D$    �F(�$�B���D$    �F(�$�?���D$    �F(�$�<���D$    �F(�$�9���D$    �F(�$�6���D$    �F(�$�3���D$    �F(�$�0���D$    �F(�$�-���D$    �F(�$�*���F �D$�F(�$�(����L��[^_]ÍD$<�D$�������D$�������D$�������D$ �������D$$�������D$(���v����Dv����Bv���D$,�D$�6  �D$�����$   �A����D$<�l$�$葄���L$�L$�$聄���t$�$�u����L$�L$�$�e����t$�$�Y����L$ �L$�$�I����L$$�L$�$�9����|$�$������L$(�L$�$�����D$    �$�����L$,�L$�$������D$�$�����.���            U��WVS�����`�u�o����h �}�D$   �F�D$�<$����F�D$   �D$�<$�����V ��tF�N�D$P   ��f�L$TuS�F �D$�D$   �<$�����F �D$�F$�@�<$�D$����V(�F,�<$�T$)Ѓ���D$����e�[^_]ÍD$L�D$8��Dv���D$0������D$ ��Bv���D$<�������D$,��'����D$��-����D$�D$P�D$(��F����D$4���v���D$$�D$8��  �D$�����$   �L$�D����T$0�D$L�T$�$萂���T$ �T$�$耂���T$<�T$�$�p����L$,�L$�$�`����T$<�T$�$�P����T$�T$�$�@����T$�T$�$�0����T$(�T$�$�@���T$4�T$�$�����L$�L$�$�`����T$$�T$�$������D$8�$������L$T9L$P�]�������U��WVS�����p�E�m����X �$   �D$L    �D$D�E�D$P�����D$T    �D$X    �D$H�������D$�D$@�D$\    �D$`    �����T$X�ƋD$T��)��)׉T$�D$�<$�����D$T�t$X��   �|$T�t$\��t�$�����D$@�D$�E�$��������M  �D$H�}�D$�E�0�@8�|$ �D$(�E�@,9���   �D$,�v �8�G�w�D$$9�t1�T$(�t& �
�����	�H�9�u�D$$��)ƋD$(���D��D$(�E�p(�G�D$�D$   �4$�����G�D$�G�4$�D$����G�D$�D$   �4$����G�D$�D$   �4$����G�W�4$�D$)���T$�s���D$,�D$,9D$ �0����E�|$(9x<��   �T$T�D$�҉D$@t �L$X9�t�A�)��Ѓ��ȉD$X�$�@����D$�e�[^_]Ít$<�	  ���D$�����$   �5�����O����D$�D$<�$�����Bv���T$�$�m���4$�u����_����t$<�  ���D$�����$   ��������n���D$�D$<�$�+����Bv���T$�$����4$�!�������            UWVS��<�L$P��j���Ì
 �A��@)������-  1��͍��n����D$�   �G�F�G�Gt�G�F�G�F    �F�G �x`����   �}0;}4�,  ����  �7�E0���E0�F���T$T+FV����D�D$T�E��@)���9���  �<�����  �$    �L$�f����L$�@    ���@    �@    �D$T��G��9������5���f���    �$�L$�D$�����L$��t�    ����u�~�V�L$�L$�|$)�����)ЉT$�ǉ$�L$�����F�L$�~���N�N�L$������$�L$������}0;}4�L$������t& +},������������w�?�G9�B����$�L$�T$�g����T$�L$�<�Ѕ��D$t�7�U,�E0�L$���T$)Ѓ��)��$�D$�L$�Y����L$�W�E,�U0�M,�L$���M4�L$�U����$�L$�:����L$�@����1��2�����D$T��<[^_]Ít$,��  ���D$�����$   �L$�����D$�D$�D$,�$�e|����Bv���T$�$�S|���4$�[����L$����  UWVS��\�t$p�|$x�h����� �D$$    �F    �F    �F    �o+/������  �D$$9D$|�F    �F    �F    �F(    �F,    �F0    �F4    �F8    �F<    ��  �|$$�D$t�����  1�1��D$(    1�1ҋL$|���  ��$�   �   f�~f�N9l$|s �L$|�<�9�t���F,�$�z���F,9�u�F8�V4��)���i�����9l$|�.  s)�L$|�I�<�9�t��    ���F8�$�"���F89�u�f�~�x  �F$    �   �F     �T$$1���u�1��,���f�~t+��9�t�F���F��u���    ��9�u吃�\[^_]É|$�F$�T$$�$�5����T$$뼍�&    �L$$9L$|����f�N��$�   9l$|f�N������L$|)�L$,�N0��)���9T$,��  +L$(���������  �;L$|BL$|����  1�1���l$,��L$(�ωD$0��t& ��t�<$�������u�L$,�D$(�~(�ȉL$,�N,9��m  �P���f�����t
�(�*�h�j��9�u�A���)��Ѓ��D$(�F(�D$,�F,�D$0�F0��&    ���,$�
��9�u�������<$����������t& ���,$����ŉn�F�O�F�9��4�����t& ���*t�(�F����9щFu�G�D$$�)D$$�|$$� �����N<�|$|�L$()���)�iɫ���9��  �D$()���i�����=TUU��  �;D$|BD$|����  1�1��Lm ��Љt$p�͉��L$(�D$,��    ��t�,$�
������u�t$p��T$(�N8�~4�D$09��  �R�ȍ�&    ����t"�(�*�h�j�h�j�h�j�h�j�h�j��9�uЍA��)���i����
%�����k��D$(�F4�D$0�F8�D$,�F<���,$����9�u��������<$�I���������t& ��t�$��	���F8�����F8u������|$,��t& ��t�$�	���F,�����F,u��6������	���$   �������l$�D$    �D$    �D$    �ǋD$$�<$�D$�	���o�~$���n ���F���������<$�V����	���������$�T$(�@����T$(�P�����&    �D$$�<�    �<$�����N�T$$�L$,�ŉȋN�)�����)ȉD$(��f���t�     ����u�D$,�L$�D$�D$(�$������L$(�F�~�N�L$$���T� �Vt�$������F,�V(�ŉ�)ՉT$(��������F(�D$,�F,�D$0�F0������D$0�V4�F8�D$,�F<�M����<�    ������@��������e����D$(������D$,�������D$0���n���D$4�������D$8���v���|$L��Dv����Bv���D$<��  ���D$�����$   �H����D$L�l$�$�u���L$(�L$�$�u���t$�$�|u���L$,�L$�$�lu���t$�$�`u���L$0�L$�$�Pu���L$4�L$�$�@u���L$|�L$�$�p����L$8�L$�$� u���L$$�L$�$�P����L$<�L$�$� u���<$�����0���   U��WVS�����0�u�`����x  �V�~�T$�|$�F�T$,�@�$��T���T$,����   ��$@   �T$,������T$,�T$�ǋF �D$�F�D$�F�<$�D$�1����V�J<;J@tv��tB�9�B<���B<�W �N���ыW,+W(�Ѓ�����TЉF�e��   [^_]Í�    1�뿍t& �T$�|$�F�@�@8�$��^�������M�����t& +J8�D$(�����������w�	�D$,�A9D$,CD$,���D$(�D$(�L$,�T$$�$������L$,�T$$��D$(�ɉL$,�D$ t�9�J8�B<�T$$�T$,�L$)ȃ��)$�D$�T$(������T$$�L$(�B8�J8�L$,�����J<�L$ �J@������$���������  �L$����q�U��WVS��^���÷� Q��  �A�9�Q��D�����D����O� �E�G�@8�4��F���D$    �D$    ��@�����x����$��4����)����@�����t�G�V(��x����T����G��|�����������D����W� �N(���   ��@����G��B8�V9���  �@ǅ<���   �7  ��t& ��p���1ɀ8 ��  ���   �M����E���  �@�E��M��E�C   �E� �A��C�E���  ���t�����$����ω�(����E��E��E���8�����H�������E��E���m  ��|�����,���� ��0����P��a����� �����t& �����  �p��H�����2�u勵0�����,����F��!t����  ��E��E���}  �E��C   �E�f�U��U��E���t�������(�����P  ��$����E���8����Eȋ��   �E�   �E�    �E�    �U��E�    ���   �G�D$��@����$�����U��E�    �E�    �����q�A��D����E��E���p����E��E���l����A9A(�Ủ�h�����   �AA0��<�����p����T$��l����T$��h����T$�L$�D$�G�D$��@����$�Q��������$  ��x����F(��|����F0�Eȋ��   ����   �E��$�yP����4����$�����D����   �M�9���  �e�Y[^_]�a�Í�&    V9�r9�@ǅ<���   �e�����D����@�   ��&    �AA0������t& �F1ɉ�<�����p�����<�����<�����<����8 �"�����t���� �$����������   �M����E�����1������t& �ȍM���8�����������D����@%   �����<�����<���������,����w�������������2ǅ���    ǅ���    �� tǅ�������ǅ��������|  ����������������������0���������D$������T$������|$�D$��8����T$�$�
�������������� �����������D$    �T$�D$    �D$    �D$�D$    �$�   ������������������������   ���������������������0��ti� �u���0�����D$�� ����D$   �D$�   �4$�D$�oY����~����D$�E��$�zm���|$�$�nm���4$�v������������������������������(��8����$���������������������X����0����w������������g����W��D����G9G(��  �GG0�}؉D$�T$�D$   �<$�[����M��)  ��D���P���D$�����$   �F����������D$�E��$�l����Bv���T$�$�l���|$�$��g����D����$�u����E��4����E��$������$�����������0����}�� �|$�$�E���������0����E��E��t-�������|$���0����$��������0����E��E��uً�8�����0����$������0���������|����0�F��!tǄ��      �F��0�����X�����,�����a����� ���������������0��������   �J��Љ�X�������,����u׃������������}����F ����   ��0����D$    �D$    �D$    �D$�����D$   �D$�$�   ��,����������,����#����� �����8����N�D$�����D$�D$   �D$�   �<$�����������V���������D$�E��$�j��������T$�$�&����������T$�$�tj��������L$�$�bj���<$�j���������F�������|�����D����GG0�N���    UWVS��<�t$P�|$T��U���ø� ��D$   �Vp�<$�T$�P�l$\�T$Xnp�9�u�F���    ��   ��<��[^_]Ít& �T$�D$������(�$�   ������D$   �l$�D$  �ƍ������D$�4$��������n���D$�4$�t$,�wi���L$�L$�$������������L$�$�Ui���T$�T$�$�����������T$�$�3i����1�|$�$�d���D$,�$�&�����<��[^_]Ít& �@D�,$�D$�D$�UV���)�T$�T$�D$   �<$�P�FD�T$��   ��@�<$�L$)ȉD$��P�T$����  �ND�A+�L$��   ōD �L$�$�D$��U���ŋ+l$�D$   �<$�l$�P�FH��   ��@�<$�T$)ЉD$��P���  �FH�L$�P+�D$��   �L$��$�D$�uU���ŋ+l$�D$   �<$�l$�P�FL��   ��@�<$�L$)ȉD$��P���[  �NL�A+�L$��   D$�L$�$�D$�U���ŋ+l$�D$   �<$�l$�P�FP��   ��@�<$�T$)ЉD$��P����  �VP�L$�B+��   D$�L$�$�D$�T���ŋ+l$�D$   �<$�l$�P�FT���   ��@�<$�T$)ЉD$�Q���Q  �VT�L$�B+��   D$�L$�$�D$�7T���ŋ+l$�D$   �<$�l$�P�FX���   ��@�<$�T$)ЉD$�Q���  �VX�L$�B+��   D$�L$�$�D$��S���ŋ+l$�D$   �<$�l$�P�F\���   ��@�<$�T$)ЉD$�Q����  �V\�L$�B+��   D$�L$�$�D$�cS���ŋ+l$�D$   �<$�l$�P�F`���   ��@�<$�T$)ЉD$�Q����  �V`�L$�B+��   D$�L$�$�D$��R���ŋ+l$�D$   �<$�l$�P�Fd���   ��@�<$�T$)ЉD$�Q���]  �Vd�L$�B+��   D$�L$�$�D$�R���ŋ+l$�D$   �<$�l$�P�Fh��   ��@�<$�L$)ȉD$��P���   �Nh�A+��   D$�������t& ������(�$�   ������D$   �l$�D$  �ƍ������D$�4$�������(o���t$,�D$�4$�G�����&    �s����(�$�   �����D$   �l$�D$  �ƍ������D$�4$�������o���t$,릐�+����t$,�  � �$   �D$��许����`o���D$�D$,��1�$��c���|$�$�G_���4$����������f�������t$,�   � �$   �D$���V������o��������t$,�!  � �$   �D$���,������o���y����|����t$,�"  � �$   �D$���������0p���L����O����t$,�#  � �$   �D$���ҷ����dp�������"����t$,�$  � �$   �D$��襷�����p�������������t$,�%  � �$   �D$���x������p�������������t$,�&  � �$   �D$���K����� q������UWVS��<�t$X�|$P�hN����(� �l$T��W�T$�D$   �4$�P�����A  �W���   ��T$�W�4$�T$�P������   �G�W��   ��T$�D$   �4$�P������  ��W���   �T$�D$   �4$�P�����  �W�G����   )Ѓ���T$�D$�4$�Q������  �G+G�����   ��<[��^_]Ív �T$�����(�$�   ������D$   �l$�D$d  �Ǎ������D$�<$������dq���|$,�<$���D$�Ha���t$�$�\���D$,�$�@����T$��<[^_��]�f��T$�����(�$�   �I����D$   �l$�D$_  �Ǎ������D$�<$�1�����4q���|$,�{���f��T$������(�$�   ������D$   �l$�D$n  �Ǎ������D$�<$��������q���|$,�+���f��T$�w����(�$�   �����D$   �l$�D$i  �Ǎ������D$�<$�������q���|$,�����f��T$�'����(�$�   �Y����D$   �l$�D$t  �Ǎ������D$�<$�A������q���|$,����  U��WVS�����`�u�}�K����e� ��W�D$   �T$�4$�P������   �E�W���   ��D$   �T$�4$�P�����O  �E�O ���   ��tt�O�D$P   ��f�L$T��  ��W �D$   �T$�4$�P�����[  �W �E���   ��T$�W$�R�4$�T$�P������  �U�G ��   �W(�G,��4$�T$)Ѓ���D$�Q�����U  �G,�u+G(�����   �ȍe�[^_]Ð�L$8������$�   �T$<������T$<�D$   �D$�  �T$�Ǎ������D$�<$������r���|$P�D$���<$�P^���t$�$�Y���D$P�$�H����L$8�e��[^_]�f��L$8������$�   �T$<�M����T$<�D$   �D$�  �T$�Ǎ������D$�<$�1�����,r���|$P�s���f��L$8�������$�   �T$<������T$<�D$   �D$�  �T$�Ǎ������D$�<$�������Pr���|$P����f��L$8�o�����$�   �T$<�����T$<�D$   �D$�  �T$�Ǎ������D$�<$�������r���|$P�����f��L$<�|$P��������  � �$   �D$��藱����xr���D$�D$P�$��\���t$�$�5X���<$������L$<�����D$P�D$�D$L�D$<��Dv���D$4������D$(��Bv���D$8�������D$0��'����D$$��-����D$ ��F����D$���v���D$,�D$<��  �D$�����$   �L$�����T$4�D$L�T$�$�/\���T$(�T$�$�\���T$8�T$�$�\���L$0�L$�$��[���T$8�T$�$��[���T$$�T$�$��[���T$ �T$�$��[���T$�T$�$������T$�T$�$�[���L$�L$�$������T$,�T$�$�[���D$<�$�����L$T9L$P�(�������               U1�WVS��<�t$P�*G������ �|$T�F,�V0)�����u�   �F,���V0)���9���   �L$X�|$�t$�L$���$�_�����uˉT$������(�$�   �"����D$   �l$�D$�  �ƍ������D$�4$�
������r���t$,�D$�4$�Z�����|$�$��U���D$,�$�����T$��<��[^_]Ív ���`  1��Q�t& �F�� ���B �T$�P �D$�<$�T$�Q�T$����  �B ���N0�@ F|�F,)���9��  ����    �L$X�T$H��D$    �L$�L$�<$�P�L$9��z����D$�F�|$�L$� �48��������(�$�   ������D$   �l$�D$�  �Ǎ������D$�<$��������r���<$�D$�|$,�zY���L$�L$�$������������L$�$�XY���T$�T$�$�����������T$�$�6Y���t$�$�T���D$,�$�.�����<1�[��^_]�f��V81�F<)�����u!�   �p�����t& �V8���F<)���9�tߋD$X�|$�t$�D$���$�������uωT$�����(�$�   ������D$   �l$�D$�  �ƍ������D$�4$�������s���t$,������&    �T$�T$�Y����(�$�   �����D$   �l$�D$�  �ƍ������D$�4$�s����������D$�4$�t$,�X���L$���L$�$�ZS���������L$�$�?���        U��WVS�����p�u�C����h� �}��D$   �D$    �4$�P�D$<�G(�$�������D$�G(�4$�D$�R������  �G(�GtT�$������Wx�D��Gx�D$<�t$�<$�D$����������8  ��D$   �D$    �4$�P�L$<�t$�<$�L$)ȉD$�6������~  �D$�D$<�t$�<$�D$���������  �D$H��L����D$@�D$<�<$�|$D�D$L    �D$`�D$@�D$�D$P�����D$T    �D$X    �t$\�D$8�\��������*  �L$H���  ��L$0�T$4�4$�D$   �D$    �P�w�|$<�T$4�L$0�9��D$,�Q  9���  ��Dv���D$4��Bv���D$<�������D$0���v���D$(������D$ ��*����D$,��3����t$������ΉD$$�D$8��  �D$�����$   �r����L$4�D$@�L$�$�U���|$�$�U���L$<�L$�$�U���T$0�T$�$�U���L$<�L$�$�U���L$ �L$�$�rU���L$,�L$�$�bU���L$�L$�$�����L$$�L$�$�BU���t$�$�v����L$(�L$�$�&U���D$8�$�*����&�����t& �$�   ���T$<�=����D$�����D$   �D${  �Ǎ������D$�<$�!�����ds���D$�<$�|$@�T���t$�$�P���D$@�$�����T$<�e��[^_]Ð�t& �T$8��������$�   �L$<�����L$<�D$   �D$t  �L$�Ǎ������D$�<$������Ds���<$�D$�|$@�0T���t$�$�O���D$@�$�(����T$8�e�[^_��]�f��$�   �<����D$�����D$   �D$�  �Ǎ��������D$�<$�����������<$�D$�|$@�S���t$�$�O���D$@�$�����e�1�[��^_]Ív �$�   ������D$�����D$   �D$�  ����$�   �������D$�����D$   �D$�  �Ǎ������D$�<$�}������s���D$�<$�|$@�S���t$�$�kN���D$8�$����1��Y�����Dv���D$4��Bv���D$<�������D$0���s���D$$�������D$ ���s���D$���v���������D$(�D$8��  �D$�����$   �/����L$4�D$@�L$�$�{R���t$�$�oR���L$<�L$�$�_R���T$0�T$�$�OR���L$<�L$�$�?R���L$$�L$�$�/R���L$ �L$�$�R���|$�$�S����L$�L$�$�R���L$,�L$�$�3����L$(�L$�$��Q���D$8�$������&���  �L$����q�U��WVS�=����W� Q��H�A�1�Q�E���D����N� �E�F�@8���A�<���t7�F�E��F�@��Q4�R�U�1҃}� u=���F�O8��y)�t9Vt6�   ��D����ȋM�9�u>�e�Y[^_]�a�Ít& �Q(���+U��f��M���$�|$�D$�R����t
~��<���V�F�r�uċu��V9V(��   �VV0�u؉T$�D$�4$�D$   �M��D���W�����}Ժ�  � �$   �D$���2�����f����D$�Eԉ$�}P����F����T$�$�kP���������T$�$�YP���t$�$�K���������T$�$�;P���uĉt$�$�K���<$�4����M��E�������E��Mĉ$�����M������u��VV0�!���   �L$����q�U��WVS�;����w� Q��H�A�1�Q�E���D����N� �E�F�@8���A�<���t6�F�E��F�@��Q4��U�1҃}� u>���F�O4��y)�t9Vt7�   ��D����ȋM�9�u?�e�Y[^_]�a�Ð�t& �Q(���+U��f��M���$�|$�D$�R����t
~���:���V�F�r�uċu��V9V(��   �VV0�u؉D$�}ԉT$�4$�D$   �M��a���P�������  � �$   �D$���R�����f����D$�Eԉ$�N����Q����T$�$�N���������T$�$�yN���t$�$��I���������T$�$�[N���uĉt$�$�I���<$�T����M��E�������E��Mĉ$�����M������u��VV0�!���   �L$����q�U��WVS��9���×� Q��H�A�1�y�Eċ�D����V� �E�F�@8���B����t$�N�R(�~�T����N�H<��I)�t9Vt)�   ����D����M�9�u4�e�Y[^_]�a�Í�&    ��M��L$�<$�D$�R�M�����t
N��9���V�F�r�u��uċV9V(��   �VV0�u؉D$�T$�4$�D$   ����P�����MԺ�  �Mċ �$   �D$��菡����f����D$�Eԉ$��L����_����T$�$��L���������T$�$�L���t$�$�
H���������T$�$�L���u��t$�$��G���Mĉ$�����E�������E��$�����������uċVV0�'���      �L$����q�U��WVS�8������ Q��X�y�1�A�}���D����N��U�V�R8�<��W�����i  �F�EċA���E���   �A���5  �F�$�D$��8���V)Љ���   �M��A+�E���  �F�O(�E�9�s/�M��   ���N��D����ЋM�9���  �e�Y[^_]�a�ËUč@��D$   �O4���D$�Eĉ$�R������   �F�}ċU����   �F��T$�U���<$�T$�P������  �F�}���   ~�F�E��W�����    �Eč�|Y���M�� �T$�UĉL$�$�P�M������{  �V�F���   �V�������    �F�v���uċu��N9N(�  �NN0�}؉L$�D$�D$   �<$�U���������g����0�$�   �����������L$�t$�D$   �D$|  �$�E������E���f����L$�$�E��J���������L$�$�J���������L$�$��I���|$�$�HE���������L$�$��I���uĉt$�$�'E���Eԉ$������U��E������UċE��$�!����U�� �����   �������    �NN0�������t& �}��F�v�O��9O(�u���   �OO0�}؉L$�D$�D$   �<$�U���������/����0�$�   �a����������L$�t$�D$   �D$|  �$�E��H����E���f����L$�$�E���H��������������OO0�u����4���EԉE���Bv���Eč�Vv���E���Sv���E��������E����v���E����v����Dv����}����E��E��X  �D$�����$   �����Eԉt$�$�RH���|$�$�FH���UĉT$�$�7H���M��L$�$�(H���UĉT$�$�H���U��T$�$�
H���U��T$�$��G���D$    �$�+����U��T$�$��G���D$    �$�����U��T$�$�G���E��$������0����EԺE  �D$�����$   �M��E��+�����t���D$�Eԉ$�vG����Bv���T$�$�dG���E��$�i����M��o����}��N�v�G9G(�  �GG0�}؉L$�D$�<$�D$   �U�����R�����8�$�   �B����������L$�|$�D$   �D$|  �$�E��)����Eč�f����L$�$�E���F����f����L$�$�F���������L$�$�F���U����Eب��   �E܋M��L$�D$�<$�U��:���������L$�$�aF���N�U���tD�N�v�L$�t$�$�U��l:���Eԉ$�A����U��E�������s����}��GG0�������������M��z���UWVS��,�D$D��1���Ì� �t$@����  �D$D� ���D$�  �8���  �/���,  �E ����   ���t9�
�4$�D$�T$�L$�����T$�J�4$�L$�~����T$�$�����D$�P��t9�
�4$�D$�T$�L$�Q����T$�J�4$�L$�>����T$�$貽���D$�$覽���E����   ���t9�
�4$�D$�T$�L$������T$�J�4$�L$������T$�$�`����D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$� ����D$�$�����,$�����o����   �E ����   ���t9�
�4$�D$�T$�L$�Z����T$�J�4$�L$�G����T$�$軼���D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$�{����D$�$�o����E��t1��4$�D$�T$������D$�P�4$�T$������D$�$�7����,$�/����<$�'����D$�x����  �/����   �E ����   ���t9�
�4$�D$�T$�L$�g����T$�J�4$�L$�T����T$�$�Ȼ���D$�P��t9�
�4$�D$�T$�L$�'����T$�J�4$�L$�����T$�$舻���D$�$�|����E��t1��4$�D$�T$������D$�P�4$�T$������D$�$�D����,$�<����o����   �E ����   ���t9�
�4$�D$�T$�L$�����T$�J�4$�L$�w����T$�$�����D$�P��t9�
�4$�D$�T$�L$�J����T$�J�4$�L$�7����T$�$諺���D$�$蟺���E��t1��4$�D$�T$�����D$�P�4$�T$������D$�$�g����,$�_����<$�W����D$�$�K����D$D�@���D$��  �8���u  �/���,  �E ����   ���t9�
�4$�D$�T$�L$�}����T$�J�4$�L$�j����T$�$�޹���D$�P��t9�
�4$�D$�T$�L$�=����T$�J�4$�L$�*����T$�$螹���D$�$蒹���E����   ���t9�
�4$�D$�T$�L$������T$�J�4$�L$������T$�$�L����D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$�����D$�$� ����,$������o���,  �E ����   ���t9�
�4$�D$�T$�L$�F����T$�J�4$�L$�3����T$�$觸���D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$������T$�$�g����D$�$�[����E����   ���t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$�����D$�P��t9�
�4$�D$�T$�L$�t����T$�J�4$�L$�a����T$�$�շ���D$�$�ɷ���,$������<$蹷���D$�x���  �/���,  �E ����   ���t9�
�4$�D$�T$�L$������T$�J�4$�L$������T$�$�Z����D$�P��t9�
�4$�D$�T$�L$�����T$�J�4$�L$�����T$�$�����D$�$�����E����   ���t9�
�4$�D$�T$�L$�g����T$�J�4$�L$�T����T$�$�ȶ���D$�P��t9�
�4$�D$�T$�L$�'����T$�J�4$�L$�����T$�$舶���D$�$�|����,$�t����o����   �E ��t1��4$�D$�T$������D$�P�4$�T$�����D$�$�1����E����   ���t9�
�4$�D$�T$�L$�����T$�J�4$�L$�w����T$�$�����D$�P��t9�
�4$�D$�T$�L$�J����T$�J�4$�L$�7����T$�$諵���D$�$蟵���,$藵���<$菵���D$�$胵���D$D�$�w�����,[^_]�               U��WVS�(����q� �����P�u�E�|$ �T$@�|$�4$��l����T$<�T$�t$$�L$ �L$�D$(�D$,    �D$0�����D$4    �D$8    �D$@    �D$D    �!����L$�D$(�L$ �L$@�D$�D$<�$�D$�L$�9����T$�������D$�|$�4$�T$<�t$$�L$ �L$�D$(�D$,    �D$0�����D$4    �D$8    �D$@    �D$D    蛖���D$(�L$�D$�D$�L$ �L$@�$�L$�����D$�������T$�|$�4$�t$$�L$ �L$�T$<�D$(�D$,    �D$0�����D$4    �D$8    �D$@    �D$D    �����D$@�L$�t$(�D$�D$�L$ �$�9����e��[^_]�               U��WVS��&���ñ� �����`�}�E�G    ��D������D$ �E �G    �O��D$<�E$�    �G(    �T$\�U���G$�D$L�G,    �W�U�G0    �G4    �G8    �W�U�G<    �W�U�W �G@    �GD    �GH    �GL    �GP    �GT    �GX    �G\    �G`    �Gd    �Gh    �Gl    �Gp    �Gt    �Gx    �G|    Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       Ǉ�       ��  �D$(�������D$�D$<�$�����G�O$�w�PH�L$�t$�T$L�W�T$�W�T$�T$(�T$�@D�$������G(�$������ƋD$<�$�ϸ���������D$�D$<�$詸���t$�<$�}����ƋD$<�$蟸���������D$�D$<�$�y����G��@�D$8)T$8�|$8�L$8��tI�O,�Gl�|$,�T$0�L$41ɍV�����)���|$4�4��|$0�V�4����v V 9L$8��uҋ|$,�Gl�D$<�$�����������D$�D$<�$�����t$�<$������ƋD$<�$�����������D$�D$<�$������t$�<$�����ƋD$<�$跷���������D$�D$<�$葷���t$�<$�����ƋD$<�$臷���� ����D$�D$<�$�a����t$�<$�5����ƋD$<�$�W����G�w�w0+w,�H+����9���  �G$�$   ���D$$�2����on  �D$T   �D$P   �D$Xf�P� imag�@e-lo�@cati�@ �G$�p�P�T$,�D$P���)  �T$T�T$8�T$X�T$0�ЉT$4�|$4���D$�D$,�D$<uo�D$Q�D$8�|$�B��L$4�����F9�FωL$�L$8�$�L$蓱������   ��   �t$<�6����   �V��t��~�L$4�F9�F�붐�t& �|$4�6f��~�L$8�F9�FωL$�L$0�$�L$�-�����u);|$8s%�v��t(�V��uËL$8�����F9�F��xۉt$<�6��u؋D$<9D$,�|$4�D$��   �@�uU�L$8���D$<��9�G΍P�Q��    ;|$4�)����v���,����D$4�|$�D$ �D$8�D$<9D$,tC�@�t��D$<�L$8�p�P9�G΀|$ �D$QED$0�L$�T$�$�O�����u[9t$8s�D$,�D$<�D$<9D$$�D$ �|$ ���   u88���  �w ���  �  ��D����T$\� 9�u=�e�[^_]Ðy�맍t& �L$X�D$8�T$<�$�����D$8�T$<몈D$�d$�w����$ ����Bv���D$<��Dv���D$8������D$0�������D$4��'����D$,��=����D$$��S����t$���v���ΉD$ �D$(��   �D$�����$   蘈���T$8�D$L�T$�$��3���T$0�T$�$��3���T$<�T$�$��3���L$4�L$�$�3���T$<�T$�$�3���T$,�T$�$�3���T$$�T$�$�3���t$�$踬���T$ �T$�$�h3���T$�T$�$蘬���|$�$�L3���D$(�$�P����&����_   ���D$�����$   �D$(輇����Xt���D$�D$L�$�3����Bv���T$�$��2���4$��������������D$0��Bv���D$<��Dv���D$8�������D$4��l����D$$���t���D$ �������D$���t�����v���D$,�D$(��   �D$�����$   �����T$8�D$L�T$�$�`2���T$$�T$�$�P2���T$<�T$�$�@2���L$4�L$�$�02���T$<�T$�$� 2���T$ �T$�$�2���T$�T$�$� 2���T$0�T$�$�0����T$,�T$�$��1���t$�$�����|$�$��1���D$(�$�̪���&����|$(��   �D$�����$   ���8�����(u���D$�D$L�$�1����Bv���T$�$�p1���t$�$�$����<$�l�������       UWVS���t$0�����ܼ �~ �n���������t%��,$�D$�����G�,$�D$�����<$胩�����������[^_]�   UWVS���t$0�����|� �~ �n���������t%��,$�D$�����G�,$�D$�����<$�#������������[^_]�   UWVS���t$0�\����� �~ �n��l������t%��,$�D$�Z����G�,$�D$�K����<$�è�����������[^_]�   UWVS���t$0�����ü� �~ �n���������t%��,$�D$������G�,$�D$������<$�c�����������4$�S�����[^_]�           UWVS���t$0�����L� �~ �n��l������t%��,$�D$�����G�,$�D$�{����<$������������4$������[^_]�           UWVS���t$0�����ܺ �~ �n���������t%��,$�D$�����G�,$�D$�����<$胧����������4$�s�����[^_]�           UWV���D$�l$ ��9�D$��   �D$(��E�Q;P�$�#  ��   ���Ɖ����@  �}���C  ���	��&    ���0��u�9D$�$��  �p;V��  ������  �D$�ϋ@���#  �p;Vt%�v �z  ����t3�p����   ���p;VuދN49O4u֋O8�v89��A  9���  �|$$����[����tx��&    �D$�u � 9�t/����u�E  �ЋP��u��L$(�x�	�Q9W�$tc��tl���  �|$$�����^_]Ít& �@4�y4�D$9�u��$�p89q8u�9|$�T  �$�y89x8�E  �D$$�(��^_]Ð�Q49W4u��Q89W8u��D$�ϋ$�@���  �p;Vt&�t& rF���������p���d������p;VuދN49O4u֋O8�v89�u9����������s�t& �0��uʋ|$$��,����������0��t�������v ��tr�<$�D$$�8����^_]�f��D$$�(����������	��&    �B;t�����f��F49A4������F89A8�����������v ����t& �ƋF;0u������D$$�(�E�����t& �|$$�D$��~��������g����������               �L$����q�U��WVS�����w� Q��   �A�1�y�E���D����V� �E�F�@8���E��@����tJ�Q�A�҉E�tj���	  �E��@����uM�D$�$����E�    �}��F�W(�}��<��F��D����   �M�9���  �e�Y[^_]�a�Ð�}�묐�E����1
  �E���|����F�$�M��D$�"���}��M��F�G+�E���  �$�M��$���M��E��F��l����F ��x����F ����  �y��t����U����t& �E�� ��t�P9z��  ��t�@��u�E�9�x�����t�����p�����  �@9x�  �����k  ��p���ƅx����@�E��F��p����@�E��@4�<ЋG���t����E�+E��҉M��4E֋�t������DƉƋE��D$�A0�D$�A,�D$�A(�t$��|����T$�D$�4$�^����E���E��G�E��G�E��G�E��G�E��G�4$�ú���M���x��� ��p����m  �F��|����@(�$�����}�)ǋF�}��P��d  ���}��G  ��|����A@���6  ��H�� ��|�����`�����\�����t���)׉�h�����|���)щ�l�����d�����ɉ�p�����  �E���t����@��)�9���  +�p���9���X�����  ��h�����`����9���\�����  �}���t�����|����΍�    ���
t��G����9�Gu狵|����}���|�����X�������  �E����l��������D:�U�������L:�U���������L:�M���D:��M��L8�E������ �T8	�E������ �T8
�E������ �T8�}��V�G9G(�  �GG0�}��D$   �T$�D$�<$�N�������x��� �?  �}�}��E���  �E��EȋE��E̋E��EЋE��V�}؉EԋB;B�  1Ʌ�t2�Mȉ�M̉H�MЉH�M��E�    �E�    �E�    �H�J�x���J�E���  �E�������E��$藟������f��q49r4�q����q89r8�P����`����v ��p���f��E���l����$   ��x����D8�E�������x�����t	�}��J�z�D$��|�����t�����x����D$��p����D$�F�$������x�����t����ǋ ���k  9�t��t�$��x����Ӟ����x���ƅx��� ������|�����p���)�)�=���?��  ��d�����l���9�BЅ���  1��<Ћ�`�����d�����h�����t���9��>  ���υ��t�
����9�u�+�h�����t����t�����|����E���p������)�)L$�$�D$��t���������E��P��|����|$)��T$�$��p����ڝ���E���t�����p�����8��|���ЋU��ɉB��d����B��������$讝���E��������t& �|$�D$   �F�@(�$�f����}��E��F�|$� �D$�F�@(�$�D����M�~�L�����}��GG0�����f��M��U���
��  �A�E��e���E���|����M��$�ۜ���M���|����M̉E��EЋE����EȋE��L$�T$�$�M��ڜ���E��M�� ������v �P49Q4u�@89A8�������������t& ��t�����\�����|�����d�����`���+�d���9�t�����|�����d�����v8��\����}���t�����&    ���
t��G����9�Gu狵\����}���p�����d�����|����|$)�)D$�$� �����`����<$�D$��h����D$���������EЉ$������C����������D$�<$�ݡ�������EȉD$�$����������|���������	�EȍEɉE����������$��t�����|����O�����|�����t���������E��    �B    �B��F� ��t�F���x����D$�F �$�y����x����F$�j�����t�����|����6�����|������������\���+�h�����`�����|�����\������Z����,���E��Y  �D$�����$   �M��E���v����hu���D$�E��$�8"����Bv���T$�$�&"���E��$�+����M������B4�x$ �����������}����E���Vv���E���Sv���E��������E����v���E����v����Bv����Dv���E���|����d  �D$�����$   �@v���E��t$�$�!���U��T$�$�!���|$�$�v!���U��T$�$�g!���|$�$�[!���U��T$�$�L!���U��T$�$�=!���D$    �$�m����U��T$�$�!���D$    �$�N����U��T$�$�� ����|����$�����-����_  �E��D$������Bv���$   �M���|����au�����u���D$�E��$� ���|$�$� ����|����$袙���M��v���          U��WVS�Q����� ����� �u�$   蚘�����t�U��P�U��P�}��9��|$t7�x;~r/��   ���u9��>t��t�$蒘���E�e�[^_]� ��t& �}��>9�t"����u	�Z  f��ʋJ��u��H9J��   ���S  �z�J�T$��u��}�     �@    �P������[  ���D$�E�T$�P�$�Rv���E�}�@�D$��E�e�[^_]� ��t& �N�ɉL$u��   �ы��u�9L$tr;yrm�u�V����   ;zsf��
��tj��;zr�vq�J��u��T$�J�I�����    �}�W��tU;Js��2��t2��;Jr�v9�r��tȉ��    �|$��tv�L$�ʋ9������v �T$���������t& ���z����|$�|$�������������t& �ыQ;
t�����f��t$��>�����������t& �ʋJ;u��
����t$�N�~������������          �L$����q�U��WVSQ��8�1�Q��	���î� �F�N�@8���A����t�P8�B+�U؉E�u�F���F�e�   Y[^_]�a�Ít& �F �ǉE̋F ��u�G��&    �ǋ ��t;Pv�@��u�9}�t'9Ww"�F�@��Q4�I(�����+O�J뒐�t& �V�R��A4�I(�у��+V�P�V�T$�U؍EԉT$�V�T$�$�|$�����E�F���D$�E؋ �D$�F�@(�$�m����F�"���     �L$����q�U��WVSQ��8�1�Q�����~� �F�N�@8���A����t�P4�B+�U؉E�u�F���F�e�   Y[^_]�a�Ít& �F �ǉE̋F ��u�G��&    �ǋ ��t;Pv�@��u�9}�t'9Ww"�F�@��Q4�I(�����+O�
듍�    �V�R��A4�I(�у��+V��V�T$�U؍EԉT$�V�T$�$�|$�����E�F���D$�E؋ �D$�F�@(�$�>����F�#���      �L$����q�U��WVSQ��8�1�Q�����N� �F�N�@8�<��G����t�P<�B+�U؉E�u�F���F�e�   Y[^_]�a�Ít& �F ���E̋F ��u�7��&    ��� ��t;Pv�@��u�9M�t9Qw�F�W(�I�L�렍v �G(�V�~�T��V�T$�U؍EԉT$�V�L$�T$�$�o����E�F���D$�E؋ �D$�F�@(�$�+����F�@���   S�������S� �D$ �@�������t�@�$������[�               VS���R����� �t$ �F�������t�F�$�Ӓ���4$�˒����[^�     UWVS��,�L$@�D$@�����ȥ �i�@�U�M ��)�9��T$tf�|$H�L$�9�0�L$H�T$DD$�L$�T$�$�T����D$@�x��,�   [^_]��2  sȋt$�>9�t��t$@�M�V�
�L$뫍v �t$D�<t$H�|$��+|$D�t$���|$��   �u�t$)�9���  �T$�|$)�)ρ����?��  D$�9�CЅ���  1��t$��9t$D�D$��  �D$D������t�
����9�u�+D$D�4�E �T$�D$)�)׉T$�<$�|$�_����}�T$�4$)׉|$��T$�D����E �L$�|$�u���M �}t�$�4����D$@�@�|$@D$H�G�������t& ��)��D$�E�L$��)�9L$�L$��  )�=���?��  � 9�Bׅ���  1��0ЉD$�D$��ȍ�&    ��t� ��9�u�U �E�T$)�)��$�D$�L$脐���E �L$�u�t$���M �ut�$�t����t$@�D$@�V�@�2�t$������v ��+D$9ǉD$�  �t$D$D9Ɖ��D$�A  �Љ׍t& ���t��E����9�Eu���D$�D$��������D$+D$D�D$�D$�D$�ǋD$�v +|$��9���v�t& ���t��E����9�Eu�t$�D$)�)��|$�t$�$蛐���D$�4$�D$�D$D�D$胐���D$@�@�J�����&    �D$�t& ��t� �U�����Uu��������    �����$�T$������T$�i�����    �D$�T$�|$�D$���1���������$�T$迎���T$�������a����T$�����      UWVS�3����� ��,�t$@�D$D�|$H�������� u-��V�P�V�@�F��,�����G+�~�F��,[^_]Ð�h�H��
vU�E�D$�d$��D$�L$�$�!����L$�n�F�D$���F�$�l$�L$�T$�'����T$�* 뉐�t& �D- �F�V��            UWVS��<�|$X�\����� �t$T���W  ���D$Pt4��tW���x�G����ŉT$t.)�1�9�,�D$P�p��<��[^_]�f��xp��G�T$����)�9�~�w%s��9�tʉW�ŋx�O���)ŉD$��멉�)�T$�W�T$)�9T$��   �D$����+D$=���?w	�9�BƉ��$�L$�����(�ʋL$��ȉD$�D$А�t& ��t�E  ��9�u��G�L$)�)$�D$�T$�Ԍ����T$�L$�o����O�����$����������t& �T$�t& ��t�  �G�����Gu�������$�   �J����D$�����D$   �D$   �ō��u���D$�,$�.������u���D$�,$�l$,������Bv���T$�$����|$�$�J����D$,�$讌���,���         WVS���D$$�]������ �|$ �0��3��  ���a����)���v d� |� �� �� ę ܙ �� � $� <� T� l� �� �� �� ̚ � �� � ,� D� \� t� �� �� �� ԛ � � � 4� L� d� |� �� �� Ĝ ܜ �� � $� <� T� l� �� �� �� ̝ � �� $� L� �t& �������D$�<$�v����    ����[^_Í�&    �������D$�<$�N���܍t& �������D$�<$�6���čt& �������D$�<$���묍t& ��v����D$�<$���딍t& ��k����D$�<$�����y������b����D$�<$�����a������Y����D$�<$����I������P����D$�<$����1������D����D$�<$����������8����D$�<$�v���������(����D$�<$�^��������������D$�<$�F��������������D$�<$�.�������������D$�<$���������� ����D$�<$����������������D$�<$�����q�����������D$�<$�����Y�����������D$�<$����A�����������D$�<$����)�����������D$�<$���������������D$�<$�n���������������D$�<$�V���������������D$�<$�>���������������D$�<$�&��������������D$�<$���������������D$�<$����������������D$�<$�����i������z����D$�<$�����Q������r����D$�<$����9������j����D$�<$����!������b����D$�<$�~���	������Z����D$�<$�f����������R����D$�<$�N����������G����D$�<$�6����������;����D$�<$����������/����D$�<$����������#����D$�<$�����y����������D$�<$�����a����������D$�<$����I����������D$�<$����1�����������D$�<$���������������D$�<$�v��������������D$�<$�^���������������D$�<$�F���������������D$�<$�.��������������D$�<$���������������D$�<$����������������D$�<$�����q�����������D$�<$�����Y�����������D$�<$����A�����������D$�<$����)�����������D$�<$����t$�$������
����T$�$�h�������   WVS���D$$�-������ �|$ �0����   ���8g����)���v l� L� ,� � �� �� �t& �������<$�D$��������[^_Ð�t& �������<$�D$��������[^_Ð�t& �������<$�D$�������[^_Ð�t& �������<$�D$�������[^_Ð�t& �������<$�D$�~������[^_Ð�t& �������<$�D$�^������[^_Ð�t& �������<$�D$�>���t$�$�҅����
����T$�$� ������[^_�       WVS���D$$������Ý� �|$ �0���t  ����h����)���v � � Ԗ �� �� �� t� \� D� ,� D� � �t& �������D$�<$�����    ����[^_Í�&    �������D$�<$�n���܍t& ������D$�<$�V���čt& ������D$�<$�>��묍t& ��%����D$�<$�&��딍t& ��9����D$�<$����y������K����D$�<$��
���a������`����D$�<$��
���I������k����D$�<$��
���1������z����D$�<$�
��������������D$�<$�
��������������D$�<$�~
���������������D$�<$�f
���t$�$�������
����T$�$�H
������   WVS���D$$������͕ �|$ �0����   ~J��t%��f�u^�� ����<$�D$��	������[^_Ív �������<$�D$��	������[^_Ð�t& ��u�������<$�D$�	������[^_Ð������<$�D$�	���t$�$�2�����
����T$�$�	������[^_Í�&    �������<$�D$�^	������[^_�     WVS���D$$������ݔ �|$ �0���  ���Hk����)���v � �� � ,� D� \� t� �� �� �� Ԓ � � � 4� L� d� |� �� �� ē ܓ �� � $� L� ̑ �������D$�<$�����    ����[^_Í�&    �������D$�<$�v���܍t& �������D$�<$�^���čt& �������D$�<$�F��묍t& �������D$�<$�.��딍t& �������D$�<$����y������~����D$�<$�����a������x����D$�<$�����I������r����D$�<$�����1������l����D$�<$����������f����D$�<$����������`����D$�<$�����������Z����D$�<$�n����������U����D$�<$�V���������P����D$�<$�>���������K����D$�<$�&���������F����D$�<$����q������A����D$�<$�����Y������<����D$�<$�����A������7����D$�<$�����)������2����D$�<$����������/����D$�<$�����������,����D$�<$�~����������)����D$�<$�f����������!����D$�<$�N�������������D$�<$�6��������������D$�<$���������������D$�<$����t$�$�����
����T$�$�����K���   WVS���D$$������m� �|$ �0����   ~J��t%��f�u^�������<$�D$�������[^_Ív �������<$�D$�~������[^_Ð�t& ��u�������<$�D$�Z������[^_Ð�������<$�D$�>���t$�$��~����
����T$�$� ������[^_Í�&    �������<$�D$��������[^_�     WVS���D$$������}� �|$ �0��	�<  ����o����)���v � ԏ �� �� �� t� \� D� ,� � �t& ��C����D$�<$�~����    ����[^_Í�&    ��P����D$�<$�V���܍t& ������D$�<$�>���čt& ������D$�<$�&��묍t& ������D$�<$���딍t& ������D$�<$�����y����������D$�<$�����a������$����D$�<$�����I������+����D$�<$����1������8����D$�<$����������[����D$�<$�~���t$�$�}����
����T$�$�`�������           WVS���D$$������ݎ �|$ �0��ta��t<��t����<$�D$����t$�$�|����
����T$�$��������[^_Ív ��k����<$�D$��������[^_Ð�t& ��c����<$�D$�������[^_�     WVS���D$$�}�����=� �|$ �0����   ~J��t%��f�u^�������<$�D$�l������[^_Ív �������<$�D$�N������[^_Ð�t& ��u�������<$�D$�*������[^_Ð�������<$�D$����t$�$�{����
����T$�$��������[^_Í�&    �������<$�D$��������[^_�     WVS���D$$������M� �t$ �8���������f  ����r����)����� � ,� D� \� t� �� �� �� Ԋ � � � 4� L� d� |� �� �� ċ ܋ � � $� <� T� l� �� �� � �������D$�4$�� ����    ����[^_Í�&    �������D$�4$�� ���܍t& �������D$�4$� ���čt& ��s����D$�4$� ��묍t& ��^����D$�4$� ��딍t& ��O����D$�4$�v ���y������A����D$�4$�^ ���a������2����D$�4$�F ���I����������D$�4$�. ���1�����������D$�4$� ��������������D$�4$�����������������D$�4$������������������D$�4$������������������D$�4$����������������D$�4$����������������D$�4$�����������z����D$�4$�n����q������j����D$�4$�V����Y������_����D$�4$�>����A������M����D$�4$�&����)������=����D$�4$�����������.����D$�4$�������������%����D$�4$�����������������D$�4$������������������D$�4$����������������D$�4$����������������D$�4$�~���������������D$�4$�f����i�����������D$�4$�N����Q�����������D$�4$�6����9�����������D$�4$�����|$�$�w����
����T$�$� �������           WVS���D$$������}� �|$ �0����  ����v����)���v Ԉ �� �� �� t� \� D� ,� � �� � ̇ �� �� � � �t& ��o����D$�<$�f�����    ����[^_Í�&    ��y����D$�<$�>����܍t& �������D$�<$�&����čt& �������D$�<$����묍t& �������D$�<$�����딍t& ������D$�<$������y����������D$�<$������a����������D$�<$�����I������!����D$�<$�����1������+����D$�<$�~����������6����D$�<$�f����������?����D$�<$�N�����������I����D$�<$�6�����������R����D$�<$�����������\����D$�<$�����������e����D$�<$�����������������D$�<$������t$�$�ju����
����T$�$�����S���   WVS���D$$�}�����=� �|$ �0����  ����x����)���v �� �� l� T� <� $� � � ܅ ą �� �� |� ܆ �� ��.����D$�<$�.�����    ����[^_Í�&    ��=����D$�<$�����܍t& �������D$�<$������čt& �������D$�<$�����묍t& �������D$�<$����딍t& �������D$�<$�����y�����������D$�<$�����a�����������D$�<$�v����I�����������D$�<$�^����1�����������D$�<$�F���������������D$�<$�.��������������D$�<$����������������D$�<$�����������������D$�<$����������������D$�<$������������I����D$�<$�����t$�$�Js����
����T$�$�����k���   WVS���D$$�]������ �|$ �0����   ~J��t%��f�u^��v����<$�D$�L�������[^_Ív ��k����<$�D$�.�������[^_Ð�t& ��u��T����<$�D$�
�������[^_Ð�������<$�D$������t$�$�r����
����T$�$���������[^_Í�&    ��_����<$�D$��������[^_�     WVS���D$$�m�����-� �|$ �0���t  ����{����)���v �� |� d� L� 4� � � � Ԃ �� ԃ �� �t& �������D$�<$�&�����    ����[^_Í�&    �������D$�<$������܍t& �������D$�<$������čt& �������D$�<$�����묍t& �������D$�<$����딍t& �������D$�<$�����y�����������D$�<$�����a�����������D$�<$�n����I�����������D$�<$�V����1�����������D$�<$�>���������������D$�<$�&���������������D$�<$����������������D$�<$������t$�$�p����
����T$�$���������   WVS���D$$������]� �|$ �0��$�,  ����}����)���v L~ d~ |~ �~ �~ �~ �~ �~  $ < T l � � � � � � � ,� D� \� t� �� �� �� Ԁ � � � 4� L� d� |� �� 4~ �������D$�<$�������    ����[^_Í�&    �������D$�<$������܍t& �������D$�<$�����čt& �������D$�<$����묍t& �������D$�<$����딍t& �������D$�<$�n����y�����������D$�<$�V����a�����������D$�<$�>����I�����������D$�<$�&����1�����������D$�<$����������������D$�<$�����������������D$�<$������������������D$�<$������������������D$�<$����������������D$�<$����������������D$�<$�~���������������D$�<$�f����q������|����D$�<$�N����Y������u����D$�<$�6����A������o����D$�<$�����)������i����D$�<$�����������c����D$�<$�������������]����D$�<$�������������W����D$�<$������������Q����D$�<$�����������L����D$�<$�����������F����D$�<$�v����������@����D$�<$�^����i������:����D$�<$�F����Q������4����D$�<$�.����9������.����D$�<$�����!������(����D$�<$������	������"����D$�<$�����������������D$�<$�����������������D$�<$����������������D$�<$����������������D$�<$���������������D$�<$�n����t$�$�l����

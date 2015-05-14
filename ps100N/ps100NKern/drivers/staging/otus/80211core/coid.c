/*
 * DVB USB Linux driver for Afatech AF9015 DVB-T USB2.0 receiver
 *
 * Copyright (C) 2007 Antti Palosaari <crope@iki.fi>
 *
 * Thanks to Afatech who kindly provided information.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _AF9013_H_
#define _AF9013_H_

#include <linux/dvb/frontend.h>

enum af9013_ts_mode {
	AF9013_OUTPUT_MODE_PARALLEL,
	AF9013_OUTPUT_MODE_SERIAL,
	AF9013_OUTPUT_MODE_USB, /* only for AF9015 */
};

enum af9013_tuner {
	AF9013_TUNER_MXL5003D   =   3, /* MaxLinear */
	AF9013_TUNER_MXL5005D   =  13, /* MaxLinear */
	AF9013_TUNER_MXL5005R   =  30, /* MaxLinear */
	AF9013_TUNER_ENV77H11D5 = 129, /* Panasonic */
	AF9013_TUNER_MT2060     = 130, /* Microtune */
	AF9013_TUNER_MC44S803   = 133, /* Freescale */
	AF9013_TUNER_QT1010     = 134, /* Quantek */
	AF9013_TUNER_UNKNOWN    = 140, /* for can tuners ? */
	AF9013_TUNER_MT2060_2   = 147, /* Microtune */
	AF9013_TUNER_TDA18271   = 156, /* NXP */
	AF9013_TUNER_QT1010A    = 162, /* Quantek */
};

/* AF9013/5 GPIOs (mostly guessed)
   demod#1-gpio#0 - set demod#2 i2c-addr for dual devices
   demod#1-gpio#1 - xtal setting (?)
   demod#1-gpio#3 - tuner#1
   demod#2-gpio#0 - tuner#2
   demod#2-gpio#1 - xtal setting (?)
*/
#define AF9013_GPIO_ON (1 << 0)
#define AF9013_GPIO_EN (1 << 1)
#define AF9013_GPIO_O  (1 << 2)
#define AF9013_GPIO_I  (1 << 3)

#define AF9013_GPIO_LO (AF9013_GPIO_ON|AF9013_GPIO_EN)
#define AF9013_GPIO_HI (AF9013_GPIO_ON|AF9013_GPIO_EN|AF9013_GPIO_O)

#define AF9013_GPIO_TUNER_ON  (AF9013_GPIO_ON|AF9013_GPIO_EN)
#define AF9013_GPIO_TUNER_OFF (AF9013_GPIO_ON|AF9013_GPIO_EN|AF9013_GPIO_O)

struct af9013_config {
	/* demodulator's I2C address */
	u8 demod_address;

	/* frequencies in kHz */
	u32 adc_clock;

	/* tuner ID */
	u8 tuner;

	/* tuner IF */
	u16 tuner_if;

	/* TS data output mode */
	u8 output_mode:2;

	/* RF spectrum inversion */
	u8 rf_spec_inv:1;

	/* API version */
	u8 api_version[4];

	/* GPIOs */
	u8 gpio[4];
};


#if defined(CONFIG_DVB_AF9013) || \
	(defined(CONFIG_DVB_AF9013_MODULE) && defined(MODULE))
extern struct dvb_frontend *af9013_attach(const struct af9013_config *config,
	struct i2c_adapter *i2c);
#else
static inline struct dvb_frontend *af9013_attach(
const struct af9013_config *config, struct i2c_adapter *i2c)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif /* CONFIG_DVB_AF9013 */

#endif /* _AF9013_H_ */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         /*
 * TerraTec Cinergy T2/qanu USB2 DVB-T adapter.
 *
 * Copyright (C) 2007 Tomi Orava (tomimo@ncircle.nullnet.fi)
 *
 * Based on the dvb-usb-framework code and the
 * original Terratec Cinergy T2 driver by:
 *
 * Copyright (C) 2004 Daniel Mack <daniel@qanu.de> and
 *                  Holger Waechtler <holger@qanu.de>
 *
 *  Protocol Spec published on http://qanu.de/specs/terratec_cinergyT2.pdf
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
 *
 */

#include "cinergyT2.h"


/**
 *  convert linux-dvb frontend parameter set into TPS.
 *  See ETSI ETS-300744, section 4.6.2, table 9 for details.
 *
 *  This function is probably reusable and may better get placed in a support
 *  library.
 *
 *  We replace errornous fields by default TPS fields (the ones with value 0).
 */

static uint16_t compute_tps(struct dvb_frontend_parameters *p)
{
	struct dvb_ofdm_parameters *op = &p->u.ofdm;
	uint16_t tps = 0;

	switch (op->code_rate_HP) {
	case FEC_2_3:
		tps |= (1 << 7);
		break;
	case FEC_3_4:
		tps |= (2 << 7);
		break;
	case FEC_5_6:
		tps |= (3 << 7);
		break;
	case FEC_7_8:
		tps |= (4 << 7);
		break;
	case FEC_1_2:
	case FEC_AUTO:
	default:
		/* tps |= (0 << 7) */;
	}

	switch (op->code_rate_LP) {
	case FEC_2_3:
		tps |= (1 << 4);
		break;
	case FEC_3_4:
		tps |= (2 << 4);
		break;
	case FEC_5_6:
		tps |= (3 << 4);
		break;
	case FEC_7_8:
		tps |= (4 << 4);
		break;
	case FEC_1_2:
	case FEC_AUTO:
	default:
		/* tps |= (0 << 4) */;
	}

	switch (op->constellation) {
	case QAM_16:
		tps |= (1 << 13);
		break;
	case QAM_64:
		tps |= (2 << 13);
		break;
	case QPSK:
	default:
		/* tps |= (0 << 13) */;
	}

	switch (op->transmission_mode) {
	case TRANSMISSION_MODE_8K:
		tps |= (1 << 0);
		break;
	case TRANSMISSION_MODE_2K:
	default:
		/* tps |= (0 << 0) */;
	}

	switch (op->guard_interval) {
	case GUARD_INTERVAL_1_16:
		tps |= (1 << 2);
		break;
	case GUARD_INTERVAL_1_8:
		tps |= (2 << 2);
		break;
	case GUARD_INTERVAL_1_4:
		tps |= (3 << 2);
		break;
	case GUARD_INTERVAL_1_32:
	default:
		/* tps |= (0 << 2) */;
	}

	switch (op->hierarchy_information) {
	case HIERARCHY_1:
		tps |= (1 << 10);
		break;
	case HIERARCHY_2:
		tps |= (2 << 10);
		break;
	case HIERARCHY_4:
		tps |= (3 << 10);
		break;
	case HIERARCHY_NONE:
	default:
		/* tps |= (0 << 10) */;
	}

	return tps;
}

struct cinergyt2_fe_state {
	struct dvb_frontend fe;
	struct dvb_usb_device *d;
};

static int cinergyt2_fe_read_status(struct dvb_frontend *fe,
					fe_status_t *status)
{
	struct cinergyt2_fe_state *state = fe->demodulator_priv;
	struct dvbt_get_status_msg result;
	u8 cmd[] = { CINERGYT2_EP1_GET_TUNER_STATUS };
	int ret;

	ret = dvb_usb_generic_rw(state->d, cmd, sizeof(cmd), (u8 *)&result,
			sizeof(result), 0);
	if (ret < 0)
		return ret;

	*status = 0;

	if (0xffff - le16_to_cpu(result.gain) > 30)
		*status |= FE_HAS_SIGNAL;
	if (result.lock_bits & (1 << 6))
		*status |= FE_HAS_LOCK;
	if (result.lock_bits & (1 << 5))
		*status |= FE_HAS_SYNC;
	if (result.lock_bits & (1 << 4))
		*status |= FE_HAS_CARRIER;
	if (result.lock_bits & (1 << 1))
		*status |= FE_HAS_VITERBI;

	if ((*status & (FE_HAS_CARRIER | FE_HAS_VITERBI | FE_HAS_SYNC)) !=
			(FE_HAS_CARRIER | FE_HAS_VITERBI | FE_HAS_SYNC))
		*status &= ~FE_HAS_LOCK;

	return 0;
}

static int cinergyt2_fe_read_ber(struct dvb_frontend *fe, u32 *ber)
{
	struct cinergyt2_fe_state *state = fe->demodulator_priv;
	struct dvbt_get_status_msg status;
	char cmd[] = { CINERGYT2_EP1_GET_TUNER_STATUS };
	int ret;

	ret = dvb_usb_generic_rw(state->d, cmd, sizeof(cmd), (char *)&status,
				sizeof(status), 0);
	if (ret < 0)
		return ret;

	*ber = le32_to_cpu(status.viterbi_error_rate);
	return 0;
}

static int cinergyt2_fe_read_unc_blocks(struct dvb_frontend *fe, u32 *unc)
{
	struct cinergyt2_fe_state *state = fe->demodulator_priv;
	struct dvbt_get_status_msg status;
	u8 cmd[] = { CINERGYT2_EP1_GET_TUNER_STATUS };
	int ret;

	ret = dvb_usb_generic_rw(state->d, cmd, sizeof(cmd), (u8 *)&status,
				sizeof(status), 0);
	if (ret < 0) {
		err("cinergyt2_fe_read_unc_blocks() Failed! (Error=%d)\n",
			ret);
		return ret;
	}
	*unc = le32_to_cpu(status.uncorrected_block_count);
	return 0;
}

static int cinergyt2_fe_read_signal_strength(struct dvb_frontend *fe,
						u16 *strength)
{
	struct cinergyt2_fe_state *state = fe->demodulator_priv;
	struct dvbt_get_status_msg status;
	char cmd[] = { CINERGYT2_EP1_GET_TUNER_STATUS };
	int ret;

	ret = dvb_usb_generic_rw(state->d, cmd, sizeof(cmd), (char *)&status,
				sizeof(status), 0);
	if (ret < 0) {
		err("cinergyt2_fe_read_signal_strength() Failed!"
			" (Error=%d)\n", ret);
		return ret;
	}
	*strength = (0xffff - le16_to_cpu(status.gain));
	return 0;
}

static int cinergyt2_fe_read_snr(struct dvb_frontend *fe, u16 *snr)
{
	struct cinergyt2_fe_state *state = fe->demodulator_priv;
	struct dvbt_get_status_msg status;
	char cmd[] = { CINERGYT2_EP1_GET_TUNER_STATUS };
	int ret;

	ret = dvb_usb_generic_rw(state->d, cmd, sizeof(cmd), (char *)&status,
				sizeof(status), 0);
	if (ret < 0) {
		err("cinergyt2_fe_read_snr() Failed! (Error=%d)\n", ret);
		return ret;
	}
	*snr = (status.snr << 8) | status.snr;
	return 0;
}

static int cinergyt2_fe_init(struct dvb_frontend *fe)
{
	return 0;
}

static int cinergyt2_fe_sleep(struct dvb_frontend *fe)
{
	deb_info("cinergyt2_fe_sleep() Called\n");
	return 0;
}

static int cinergyt2_fe_get_tune_settings(struct dvb_frontend *fe,
				struct dvb_frontend_tune_settings *tune)
{
	tune->min_delay_ms = 800;
	return 0;
}

static int cinergyt2_fe_set_frontend(struct dvb_frontend *fe,
				  struct dvb_frontend_parameters *fep)
{
	struct cinergyt2_fe_state *state = fe->demodulator_priv;
	struct dvbt_set_parameters_msg param;
	char result[2];
	int err;

	param.cmd = CINERGYT2_EP1_SET_TUNER_PARAMETERS;
	param.tps = cpu_to_le16(compute_tps(fep));
	param.freq = cpu_to_le32(fep->frequency / 1000);
	param.bandwidth = 8 - fep->u.ofdm.bandwidth - BANDWIDTH_8_MHZ;

	err = dvb_usb_generic_rw(state->d,
			(char *)&param, sizeof(param),
			result, sizeof(result), 0);
	if (err < 0)
		err("cinergyt2_fe_set_frontend() Failed! err=%d\n", err);

	return (err < 0) ? err : 0;
}

static int cinergyt2_fe_get_frontend(struct dvb_frontend *fe,
				  struct dvb_frontend_parameters *fep)
{
	return 0;
}

static void cinergyt2_fe_release(struct dvb_frontend *fe)
{
	struct cinergyt2_fe_state *state = fe->demodulator_priv;
	if (state != NULL)
		kfree(state);
}

static struct dvb_frontend_ops cinergyt2_fe_ops;

struct dvb_frontend *cinergyt2_fe_attach(struct dvb_usb_device *d)
{
	struct cinergyt2_fe_state *s = kzalloc(sizeof(
					struct cinergyt2_fe_state), GFP_KERNEL);
	if (s == NULL)
		return NULL;

	s->d = d;
	memcpy(&s->fe.ops, &cinergyt2_fe_ops, sizeof(struct dvb_frontend_ops));
	s->fe.demodulator_priv = s;
	return &s->fe;
}


static struct dvb_frontend_ops cinergyt2_fe_ops = {
	.info = {
		.name			= DRIVER_NAME,
		.type			= FE_OFDM,
		.frequency_min		= 174000000,
		.frequency_max		= 862000000,
		.frequency_stepsize	= 166667,
		.caps = FE_CAN_INVERSION_AUTO | FE_CAN_FEC_1_2
			| FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4
			| FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8
			| FE_CAN_FEC_AUTO | FE_CAN_QPSK
			| FE_CAN_QAM_16 | FE_CAN_QAM_64
			| FE_CAN_QAM_AUTO
			| FE_CAN_TRANSMISSION_MODE_AUTO
			| FE_CAN_GUARD_INTERVAL_AUTO
			| FE_CAN_HIERARCHY_AUTO
			| FE_CAN_RECOVER
			| FE_CAN_MUTE_TS
	},

	.release		= cinergyt2_fe_release,

	.init			= cinergyt2_fe_init,
	.sleep			= cinergyt2_fe_sleep,

	.set_frontend		= cinergyt2_fe_set_frontend,
	.get_frontend		= cinergyt2_fe_get_frontend,
	.get_tune_settings	= cinergyt2_fe_get_tune_settings,

	.read_status		= cinergyt2_fe_read_status,
	.read_ber		= cinergyt2_fe_read_ber,
	.read_signal_strength	= cinergyt2_fe_read_signal_strength,
	.read_snr		= cinergyt2_fe_read_snr,
	.read_ucblocks		= cinergyt2_fe_read_unc_blocks,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        10

dir
1524
http://172.16.250.7/svn/YF-110510-01/soft/trunk/kernel/drivers/media/video/bt8xx
http://172.16.250.7/svn/YF-110510-01



2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu














260be20b-3001-0010-90b4-153382fa7df0

Kconfig
file




2013-08-27T09:45:08.302401Z
34fa9d38350d24264c362e3bc27f4c41
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















940

bttvp.h
file




2013-08-27T09:45:08.304401Z
86c208f06efc15955cf51ffdcdd76f79
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















14108

bttv-driver.c
file




2013-08-27T09:45:08.304401Z
af2ddb4afa7d081adc22ce3b8137413e
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















121655

bttv-cards.c
file




2013-08-27T09:45:08.305401Z
ede62e7ac1a9c920b01f5b1cfd17fd2c
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















151305

bttv-if.c
file




2013-08-27T09:45:08.307401Z
4948b2561a10234c3162c44af040521a
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















2894

bttv-gpio.c
file




2013-08-27T09:45:08.306401Z
ee107146314fc3308aba36ec3eb98f06
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















4858

bttv-input.c
file




2013-08-27T09:45:08.308401Z
77738b0b21362eb37955e70933b8349f
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















10597

bttv-audio-hook.c
file




2013-08-27T09:45:08.307401Z
c4cc572b5b40dc37a4b0b3a9eb678d7c
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















9617

bttv-risc.c
file




2013-08-27T09:45:08.308401Z
80c6e406bf21ebc90fbb35a333fe742e
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















25965

bt848.h
file




2013-08-27T09:45:08.302401Z
385bf2a63eb497c95bef6fb3cce4f85b
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















11788

bttv-audio-hook.h
file




2013-08-27T09:45:08.303401Z
90c58fd1318ee160a197eeeddce4500f
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















1126

bttv.h
file




2013-08-27T09:45:08.303401Z
3ec2d92c88e6a98fad1478ad50de431f
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















14924

bttv-i2c.c
file




2013-08-27T09:45:08.306401Z
33d11b2d88cf5fb2350aec82fa740f73
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















10677

Makefile
file




2013-08-27T09:45:08.307401Z
39b2270d9b7e860b7cfc5c2beadb4578
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















389

bttv-vbi.c
file




2013-08-27T09:45:08.308401Z
4e9e22b8b7a3a0a252e6df71d49e8c32
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















12835

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  /* cs8420.h - cs8420 initializations
   Copyright (C) 1999 Nathan Laredo (laredo@gnu.org)

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
#ifndef __CS8420_H__
#define __CS8420_H__

/* Initialization Sequence */

static __u8 init8420[] = {
	1, 0x01,	2, 0x02,	3, 0x00,	4, 0x46,
	5, 0x24,	6, 0x84,	18, 0x18,	19, 0x13,
};

#define INIT8420LEN	(sizeof(init8420)/2)

static __u8 mode8420pro[] = {	/* professional output mode */
	32, 0xa1,	33, 0x00,	34, 0x00,	35, 0x00,
	36, 0x00,	37, 0x00,	38, 0x00,	39, 0x00,
	40, 0x00,	41, 0x00,	42, 0x00,	43, 0x00,
	44, 0x00,	45, 0x00,	46, 0x00,	47, 0x00,
	48, 0x00,	49, 0x00,	50, 0x00,	51, 0x00,
	52, 0x00,	53, 0x00,	54, 0x00,	55, 0x00,
};
#define MODE8420LEN	(sizeof(mode8420pro)/2)

static __u8 mode8420con[] = {	/* consumer output mode */
	32, 0x20,	33, 0x00,	34, 0x00,	35, 0x48,
	36, 0x00,	37, 0x00,	38, 0x00,	39, 0x00,
	40, 0x00,	41, 0x00,	42, 0x00,	43, 0x00,
	44, 0x00,	45, 0x00,	46, 0x00,	47, 0x00,
	48, 0x00,	49, 0x00,	50, 0x00,	51, 0x00,
	52, 0x00,	53, 0x00,	54, 0x00,	55, 0x00,
};

#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               dex]);
                }
            }
            else if ( keyInfo.keyLength == 16 )
            {   /* AES */
#ifdef ZM_ENABLE_IBSS_WPA2PSK
                if ( wd->sta.ibssWpa2Psk == 1 )
                {
                    wd->sta.oppositeInfo[userIdx].encryMode = ZM_AES;
                    encryType = wd->sta.oppositeInfo[userIdx].encryMode;
                }
                else
                {
                    wd->sta.encryMode = ZM_AES;
                    encryType = wd->sta.encryMode;
                }
#else
                wd->sta.encryMode = ZM_AES;
#endif
            }
            else
            {
                return ZM_STATUS_FAILURE;
            }

            /* user 0 */
            //zfCoreSetKey(dev, 0, 0, wd->sta.encryMode,
            //         wd->sta.bssid, (u32_t*) keyInfo.key);
            //zfHpSetStaPairwiseKey(dev, wd->sta.bssid, wd->sta.encryMode,
            //    (u32_t*) keyInfo.key, (u32_t*) &keyInfo.key[16]);

#ifdef ZM_ENABLE_IBSS_WPA2PSK
            if ( (keyInfo.keyLength==16) && (wd->sta.ibssWpa2Psk==1) )
            { /* If not AES-CCMP and ibss network , use traditional */
                zfHpSetPerUserKey(dev,
                                userIdx,
                                keyInfo.keyIndex,  // key id == 0 ( Pairwise key = 0 )
                                (u8_t*)keyInfo.macAddr,   // RX need Source Address ( Address 2 )
                                encryType,
//                              wd->sta.encryMode,
                                (u32_t*) keyInfo.key, (u32_t*) &keyInfo.key[16]);

                wd->sta.oppositeInfo[userIdx].wpaState = ZM_STA_WPA_STATE_PK_OK ;
            }
            else
            {/* Big Endian and Little Endian Compatibility */
                for (i = 0; i < 3; i++)
                {
                    addr[2 * i] = wd->sta.bssid[i] & 0xff;
                    addr[2 * i + 1] = wd->sta.bssid[i] >> 8;
                }
                zfHpSetPerUserKey(dev,
                                    ZM_USER_KEY_PK,   // user id
                                    0,                // key id
                                    addr,//(u8_t *)wd->sta.bssid,
                              wd->sta.encryMode,
                              (u32_t*) keyInfo.key, (u32_t*) &keyInfo.key[16]);

                wd->sta.keyId = 4;
            }
#else
            /* Big Endian and Little Endian Compatibility */
            for (i = 0; i < 3; i++)
            {
                addr[2 * i] = wd->sta.bssid[i] & 0xff;
                addr[2 * i + 1] = wd->sta.bssid[i] >> 8;
            }
            zfHpSetPerUserKey(dev,
                              ZM_USER_KEY_PK,   // user id
                              0,                // key id
                              addr,//(u8_t *)wd->sta.bssid,
                              wd->sta.encryMode,
                              (u32_t*) keyInfo.key, (u32_t*) &keyInfo.key[16]);

            wd->sta.keyId = 4;
#endif

            wd->sta.wpaState = ZM_STA_WPA_STATE_PK_OK;
        }
        else if ( keyInfo.flag & ZM_KEY_FLAG_GK )
        {   /* set group key */

            zfTkipInit(keyInfo.key, (u8_t*) wd->sta.bssid,
                       &wd->sta.rxSeed[keyInfo.keyIndex], keyInfo.initIv);

            if ( keyInfo.keyLength == 32 )
            {   /* TKIP */
#ifdef ZM_ENABLE_CENC
                if (keyInfo.flag & ZM_KEY_FLAG_CENC)
                {
                    encryMode = ZM_CENC;
                    zm_debug_msg0("Set CENC group Key");

                    /* Reset txiv and rxiv */
                    wd->sta.rxivGK[0] = 0x5c365c36;
                    wd->sta.rxivGK[1] = 0x5c365c36;
                    wd->sta.rxivGK[2] = 0x5c365c36;
                    wd->sta.rxivGK[3] = 0x5c365c36;

                    //zfCoreSetKey(dev, 0, 1, ZM_CENC, keyInfo.vapAddr,
                    //         (u32_t*) &keyInfo.key[16]);
                    key = (u32_t*) keyInfo.key;
                }
                else
#endif //ZM_ENABLE_CENC
                {
                    encryMode = ZM_TKIP;
                    key = (u32_t*) wd->sta.rxSeed[keyInfo.keyIndex].tk;

                    if ( !(keyInfo.flag & ZM_KEY_FLAG_INIT_IV) )
                    {
                        wd->sta.rxSeed[keyInfo.keyIndex].iv16 = 0;
                        wd->sta.rxSeed[keyInfo.keyIndex].iv32 = 0;
                    }

                    /* set MIC key to HMAC */
                    //zfCoreSetKey(dev, 8, 1, ZM_TKIP, broadcast,
                    //         (u32_t*) (&keyInfo.key[16]));

                    zfMicSetKey(&keyInfo.key[24],
                                &wd->sta.rxMicKey[keyInfo.keyIndex]);
                }
            }
            else if ( keyInfo.keyLength == 16 )
            {   /* AES */
                encryMode = ZM_AES;
                //key = (u32_t*) wd->sta.rxSeed[keyInfo.keyIndex].tk;
            }
            else
            {   /* WEP */
                if ( keyInfo.keyLength == 5 )
                {
                    encryMode = ZM_WEP64;
                }
                else if ( keyInfo.keyLength == 13 )
                {
                    encryMode = ZM_WEP128;
                }
                else if ( keyInfo.keyLength == 29 )
                {
                    encryMode = ZM_WEP256;
                }

                key = (u32_t*) keyInfo.key;
            }

            /* user 8 */
            //zfCoreSetKey(dev, 8, 0, encryMode, broadcast, key);
            //zfHpSetStaGroupKey(dev, broadcast, encryMode,
            //        (u32_t*) keyInfo.key, (u32_t*) (&keyInfo.key[16]));

#ifdef ZM_ENABLE_IBSS_WPA2PSK
            if ( (keyInfo.keyLength==16) && (wd->sta.ibssWpa2Psk==1) )
            {/* If not AES-CCMP and ibss network , use traditional */
                zfHpSetPerUserKey(dev,
                              userIdx,
                              keyInfo.keyIndex,                // key id
                              // (u8_t *)broadcast,                  // for only 2 stations IBSS netwrl ( A2 )
                              (u8_t*)keyInfo.macAddr,   // for multiple ( > 2 ) stations IBSS network ( A2 )
                              encryMode,
                              (u32_t*) keyInfo.key, (u32_t*) &keyInfo.key[16]);
            }
            else
            {
                zfHpSetPerUserKey(dev,
                                ZM_USER_KEY_GK,   // user id
                                0,                // key id
                                (u8_t *)broadcast,
                                encryMode,
                                (u32_t*) keyInfo.key, (u32_t*) &keyInfo.key[16]);

                wd->sta.wpaState = ZM_STA_WPA_STATE_GK_OK;
            }
#else
            zfHpSetPerUserKey(dev,
                              ZM_USER_KEY_GK,   // user id
                              0,                // key id
                              (u8_t *)broadcast,
                              encryMode,
                              (u32_t*) keyInfo.key, (u32_t*) &keyInfo.key[16]);

            wd->sta.wpaState = ZM_STA_WPA_STATE_GK_OK;
#endif
        }
        else
        {   /* legacy WEP */
            zm_debug_msg0("legacy WEP");

            if ( keyInfo.keyIndex >= 4 )
            {
                return ZM_STATUS_FAILURE;
            }

            if ( keyInfo.keyLength == 5 )
            {
                zm_debug_msg0("WEP 64");

                encryMode = ZM_WEP64;
            }
            else if ( keyInfo.keyLength == 13 )
            {
                zm_debug_msg0("WEP 128");

                encryMode = ZM_WEP128;
            }
            else if ( keyInfo.keyLength == 32 )
            {
                /* TKIP */
                #if 0
                // Don't reset the IV since some AP would fail in IV check and drop our connection
                if ( wd->sta.wpaState != ZM_STA_WPA_STATE_PK_OK )
                {
                    wd->sta.iv16 = 0;
                    wd->sta.iv32 = 0;
                }
                #endif

                encryMode = ZM_TKIP;

                zfTkipInit(keyInfo.key, (u8_t*) wd->sta.bssid,
                           &wd->sta.rxSeed[keyInfo.keyIndex], keyInfo.initIv);
                zfMicSetKey(&keyInfo.key[24],
                           &wd->sta.rxMicKey[keyInfo.keyIndex]);
            }
            else if ( keyInfo.keyLength == 16 )
            {
                /* AES */
                #if 0
                // Don't reset the IV since some AP would fail in IV check and drop our connection
                if ( wd->sta.wpaState != ZM_STA_WPA_STATE_PK_OK )
                {
                    /* broadcast -- > group key */
                    /* Only initialize when set our default key ! */
                    wd->sta.iv16 = 0;
                    wd->sta.iv32 = 0;
                }
                #endif

                encryMode = ZM_AES;
            }
            else if ( keyInfo.keyLength == 29 )
            {
                zm_debug_msg0("WEP 256");

                encryMode = ZM_WEP256;
                //zfCoreSetKey(dev, 64, 1, wd->sta.encryMode,
                //         wd->sta.bssid, (u32_t*) (&keyInfo.key[16]));
            }
            else
            {
                return ZM_STATUS_FAILURE;
            }

            {
                u8_t i;

                zm_debug_msg0("key = ");
                for(i = 0; i < keyInfo.keyLength; i++)
                {
                    zm_debug_msg2("", keyInfo.key[i]);
                }
            }

            if ( keyInfo.flag & ZM_KEY_FLAG_DEFAULT_KEY )
            {
                //for WEP default key 1~3 and ATOM platform--CWYang(+)
                vapId = 0;
                wd->ap.bcHalKeyIdx[vapId] = keyInfo.keyIndex;
                wd->ap.bcKeyIndex[vapId] = keyInfo.keyIndex;
                wd->sta.keyId = keyInfo.keyIndex;
            }

			if(encryMode == ZM_TKIP)
			{
				if(wd->TKIP_Group_KeyChanging == 0x1)
				{
					zm_debug_msg0("Countermeasure : Cancel Old Timer ");
					zfTimerCancel(dev,	ZM_EVENT_SKIP_COUNTERMEASURE);
				}
				else
				{
					zm_debug_msg0("Countermeasure : Create New Timer ");
				}

				wd->TKIP_Group_KeyChanging = 0x1;
				zfTimerSchedule(dev, ZM_EVENT_SKIP_COUNTERMEASURE, 150);
			}



			//------------------------------------------------------------------------

            /* use default key */
            //zfCoreSetKey(dev, ZM_USER_KEY_DEFAULT+keyInfo.keyIndex, 0,
            //         wd->sta.encryMode, wd->sta.bssid, (u32_t*) keyInfo.key);

            if ( encryMode == ZM_TKIP ||
                 encryMode == ZM_AES )
            {
                zfHpSetDefaultKey(dev, keyInfo.keyIndex, encryMode,
                                 (u32_t*) keyInfo.key, (u32_t*) &keyInfo.key[16]);

#ifdef ZM_ENABLE_IBSS_WPA2PSK
            if ( (keyInfo.keyLength==16) && (wd->sta.ibssWpa2Psk==1) )
            {/* If not AES-CCMP and ibss network , use traditional */
                wd->sta.wpaState = ZM_STA_WPA_STATE_PK_OK;
            }
            else
            {
                if (wd->sta.wpaState == ZM_STA_WPA_STATE_PK_OK)
                    wd->sta.wpaState = ZM_STA_WPA_STATE_GK_OK;
                else
                {
                    wd->sta.wpaState = ZM_STA_WPA_STATE_PK_OK;
                    wd->sta.encryMode = encryMode;
                    wd->ws.encryMode = encryMode;
                }
            }
#else
                if (wd->sta.wpaState == ZM_STA_WPA_STATE_PK_OK)
                    wd->sta.wpaState = ZM_STA_WPA_STATE_GK_OK;
                else if ( wd->sta.wpaState == ZM_STA_WPA_STATE_INIT )
                {
                    wd->sta.wpaState = ZM_STA_WPA_STATE_PK_OK;
                    wd->sta.encryMode = encryMode;
                    wd->ws.encryMode = encryMode;
                }
#endif
            }
            else
            {
                zfHpSetDefaultKey(dev, keyInfo.keyIndex, encryMode,
                               (u32_t*) keyInfo.key, NULL);

                /* Save key for software WEP */
                zfMemoryCopy(wd->sta.wepKey[keyInfo.keyIndex], keyInfo.key,
                        keyInfo.keyLength);

                /* TODO: Check whether we need to save the SWEncryMode */
                wd->sta.SWEncryMode[keyInfo.keyIndex] = encryMode;

                wd->sta.encryMode = encryMode;
                wd->ws.encryMode = encryMode;
            }
        }
    }

//    wd->sta.flagKeyChanging = 1;
    return ZM_STATUS_SUCCESS;
}

/* PSEUDO test */
u8_t zfiWlanPSEUDOSetKey(zdev_t* dev, struct zsKeyInfo keyInfo)
{
    //u16_t  broadcast[3] = {0xffff, 0xffff, 0xffff};
    //u32_t* key;
    u8_t   micKey[16];

    zmw_get_wlan_dev(dev);

    switch (keyInfo.keyLength)
    {
        case 5:
            wd->sta.encryMode = ZM_WEP64;
            /* use default key */
            zfCoreSetKey(dev, 64, 0, ZM_WEP64, (u16_t *)keyInfo.macAddr, (u32_t*) keyInfo.key);
		          break;

       	case 13:
            wd->sta.encryMode = ZM_WEP128;
            /* use default key */
            zfCoreSetKey(dev, 64, 0, ZM_WEP128, (u16_t *)keyInfo.macAddr, (u32_t*) keyInfo.key);
          		break;

       	case 29:
            wd->sta.encryMode = ZM_WEP256;
            /* use default key */
            zfCoreSetKey(dev, 64, 1, ZM_WEP256,  (u16_t *)keyInfo.macAddr, (u32_t*) (&keyInfo.key[16]));
            zfCoreSetKey(dev, 64, 0, ZM_WEP256, (u16_t *)keyInfo.macAddr, (u32_t*) keyInfo.key);
		          break;

       	case 16:
            wd->sta.encryMode = ZM_AES;
            //zfCoreSetKey(dev, 0, 0, ZM_AES, (u16_t *)keyInfo.macAddr, (u32_t*) keyInfo.key);
            zfCoreSetKey(dev, 64, 0, ZM_AES, (u16_t *)keyInfo.macAddr, (u32_t*) keyInfo.key);
            break;

       	case 32:
#ifdef ZM_ENABLE_CENC
            if (keyInfo.flag & ZM_KEY_FLAG_CENC)
            {
                u16_t boardcastAddr[3] = {0xffff, 0xffff, 0xffff};
                u16_t Addr_a[] = { 0x0000, 0x0080, 0x0901};
                u16_t Addr_b[] = { 0x0000, 0x0080, 0x0902};
                /* CENC test: user0,1 and user2 for boardcast */
                wd->sta.encryMode = ZM_CENC;
                zfCoreSetKey(dev, 0, 1, ZM_CENC, (u16_t *)Addr_a, (u32_t*) (&keyInfo.key[16]));
                zfCoreSetKey(dev, 0, 0, ZM_CENC, (u16_t *)Addr_a, (u32_t*) keyInfo.key);

                zfCoreSetKey(dev, 1, 1, ZM_CENC, (u16_t *)Addr_b, (u32_t*) (&keyInfo.key[16]));
                zfCoreSetKey(dev, 1, 0, ZM_CENC, (u16_t *)Addr_b, (u32_t*) keyInfo.key);

                zfCoreSetKey(dev, 2, 1, ZM_CENC, (u16_t *)boardcastAddr, (u32_t*) (&keyInfo.key[16]));
                zfCoreSetKey(dev, 2, 0, ZM_CENC, (u16_t *)boardcastAddr, (u32_t*) keyInfo.key);

                /* Initialize PN sequence */
                wd->sta.txiv[0] = 0x5c365c36;
                wd->sta.txiv[1] = 0x5c365c36;
                wd->sta.txiv[2] = 0x5c365c36;
                wd->sta.txiv[3] = 0x5c365c36;
            }
            else
#endif //ZM_ENABLE_CENC
            {
                wd->sta.encryMode = ZM_TKIP;
                zfCoreSetKey(dev, 64, 1, ZM_TKIP, (u16_t *)keyInfo.macAddr, (u32_t*) micKey);
                zfCoreSetKey(dev, 64, 0, ZM_TKIP, (u16_t *)keyInfo.macAddr, (u32_t*) keyInfo.key);
            }
            break;
        default:
            wd->sta.encryMode = ZM_NO_WEP;
    }

    return ZM_STATUS_SUCCESS;
}

void zfiWlanSetPowerSaveMode(zdev_t* dev, u8_t mode)
{
#if 0
    zmw_get_wlan_dev(dev);

    wd->sta.powerSaveMode = mode;

    /* send null data with PwrBit to inform AP */
    if ( mode > ZM_STA_PS_NONE )
    {
        if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
        {
            zfSendNullData(dev, 1);
        }

        /* device into PS mode */
        zfPSDeviceSleep(dev);
    }
#endif

    zfPowerSavingMgrSetMode(dev, mode);
}

void zfiWlanSetMacAddress(zdev_t* dev, u16_t* mac)
{
    zmw_get_wlan_dev(dev);

    wd->macAddr[0] = mac[0];
    wd->macAddr[1] = mac[1];
    wd->macAddr[2] = mac[2];

    zfHpSetMacAddress(dev, mac, 0);
}

u8_t zfiWlanQueryWlanMode(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->wlanMode;
}

u8_t zfiWlanQueryAdapterState(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->state;
}

u8_t zfiWlanQueryAuthenticationMode(zdev_t* dev, u8_t bWrapper)
{
    u8_t   authMode;

    zmw_get_wlan_dev(dev);

    if ( bWrapper )
    {
        authMode = wd->ws.authMode;
    }
    else
    {
        //authMode = wd->sta.authMode;
        authMode = wd->sta.currentAuthMode;
    }

    return authMode;
}

u8_t zfiWlanQueryWepStatus(zdev_t* dev, u8_t bWrapper)
{
    u8_t wepStatus;

    zmw_get_wlan_dev(dev);

    if ( bWrapper )
    {
        wepStatus = wd->ws.wepStatus;
    }
    else
    {
        wepStatus = wd->sta.wepStatus;
    }

    return wepStatus;
}

void zfiWlanQuerySSID(zdev_t* dev, u8_t* ssid, u8_t* pSsidLength)
{
    u16_t vapId = 0;
    zmw_get_wlan_dev(dev);

    if (wd->wlanMode == ZM_MODE_AP)
    {
        vapId = zfwGetVapId(dev);

        if (vapId == 0xffff)
        {
            *pSsidLength = wd->ap.ssidLen[0];
            zfMemoryCopy(ssid, wd->ap.ssid[0], wd->ap.ssidLen[0]);
        }
        else
        {
            *pSsidLength = wd->ap.ssidLen[vapId + 1];
            zfMemoryCopy(ssid, wd->ap.ssid[vapId + 1], wd->ap.ssidLen[vapId + 1]);
        }
    }
    else
    {
        *pSsidLength = wd->sta.ssidLen;
        zfMemoryCopy(ssid, wd->sta.ssid, wd->sta.ssidLen);
    }
}

u16_t zfiWlanQueryFragThreshold(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->fragThreshold;
}

u16_t zfiWlanQueryRtsThreshold(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->rtsThreshold;
}

u32_t zfiWlanQueryFrequency(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return (wd->frequency*1000);
}

/***********************************************************
 * Function: zfiWlanQueryCurrentFrequency
 * Return value:
 *   -   0 : no validate current frequency
 *   - (>0): current frequency depend on "qmode"
 * Input:
 *   - qmode:
 *      0: return value depend on the support mode, this
           qmode is use to solve the bug #31223
 *      1: return the actually current frequency
 ***********************************************************/
u32_t zfiWlanQueryCurrentFrequency(zdev_t* dev, u8_t qmode)
{
    u32_t frequency;

    zmw_get_wlan_dev(dev);

    switch (qmode)
    {
    case 0:
        if (wd->sta.currentFrequency > 3000)
        {
            if (wd->supportMode & ZM_WIRELESS_MODE_5)
            {
                frequency = wd->sta.currentFrequency;
            }
            else if (wd->supportMode & ZM_WIRELESS_MODE_24)
            {
                frequency = zfChGetFirst2GhzChannel(dev);
            }
            else
            {
                frequency = 0;
            }
        }
        else
        {
            if (wd->supportMode & ZM_WIRELESS_MODE_24)
            {
                frequency = wd->sta.currentFrequency;
            }
            else if (wd->supportMode & ZM_WIRELESS_MODE_5)
            {
                frequency = zfChGetLast5GhzChannel(dev);
            }
            else
            {
                frequency = 0;
            }
        }
        break;

    case 1:
        frequency = wd->sta.currentFrequency;
        break;

    default:
        frequency = 0;
    }

    return (frequency*1000);
}

u32_t zfiWlanQueryFrequencyAttribute(zdev_t* dev, u32_t freq)
{
    u8_t  i;
    u16_t frequency = (u16_t) (freq/1000);
    u32_t ret = 0;

    zmw_get_wlan_dev(dev);

    for (i = 0; i < wd->regulationTable.allowChannelCnt; i++)
    {
        if ( wd->regulationTable.allowChannel[i].channel == frequency )
        {
            ret = wd->regulationTable.allowChannel[i].channelFlags;
        }
    }

    return ret;
}

/* BandWidth  0=>20  1=>40 */
/* ExtOffset  0=>20  1=>high control 40   3=>low control 40 */
void zfiWlanQueryFrequencyHT(zdev_t* dev, u32_t *bandWidth, u32_t *extOffset)
{
    zmw_get_wlan_dev(dev);

    *bandWidth = wd->BandWidth40;
    *extOffset = wd->ExtOffset;
}

u8_t zfiWlanQueryCWMode(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->cwm.cw_mode;
}

u32_t zfiWlanQueryCWEnable(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->cwm.cw_enable;
}

void zfiWlanQueryBssid(zdev_t* dev, u8_t* bssid)
{
    u8_t   addr[6];

    zmw_get_wlan_dev(dev);

    ZM_MAC_WORD_TO_BYTE(wd->sta.bssid, addr);
    zf_close;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);

	for (i = 0; i < gf2k_axes[gf2k->id]; i++)
		set_bit(gf2k_abs[i], input_dev->absbit);

	for (i = 0; i < gf2k_hats[gf2k->id]; i++) {
		set_bit(ABS_HAT0X + i, input_dev->absbit);
		input_dev->absmin[ABS_HAT0X + i] = -1;
		input_dev->absmax[ABS_HAT0X + i] = 1;
	}

	for (i = 0; i < gf2k_joys[gf2k->id]; i++)
		set_bit(gf2k_btn_joy[i], input_dev->keybit);

	for (i = 0; i < gf2k_pads[gf2k->id]; i++)
		set_bit(gf2k_btn_pad[i], input_dev->keybit);

	gf2k_read_packet(gameport, gf2k->length, data);
	gf2k_read(gf2k, data);

	for (i = 0; i < gf2k_axes[gf2k->id]; i++) {
		input_dev->absmax[gf2k_abs[i]] = (i < 2) ? input_dev->abs[gf2k_abs[i]] * 2 - 32 :
			  input_dev->abs[gf2k_abs[0]] + input_dev->abs[gf2k_abs[1]] - 32;
		input_dev->absmin[gf2k_abs[i]] = 32;
		input_dev->absfuzz[gf2k_abs[i]] = 8;
		input_dev->absflat[gf2k_abs[i]] = (i < 2) ? 24 : 0;
	}

	err = input_register_device(gf2k->dev);
	if (err)
		goto fail2;

	return 0;

 fail2:	gameport_close(gameport);
 fail1:	gameport_set_drvdata(gameport, NULL);
	input_free_device(input_dev);
	kfree(gf2k);
	return err;
}

static void gf2k_disconnect(struct gameport *gameport)
{
	struct gf2k *gf2k = gameport_get_drvdata(gameport);

	input_unregister_device(gf2k->dev);
	gameport_close(gameport);
	gameport_set_drvdata(gameport, NULL);
	kfree(gf2k);
}

static struct gameport_driver gf2k_drv = {
	.driver		= {
		.name	= "gf2k",
	},
	.description	= DRIVER_DESC,
	.connect	= gf2k_connect,
	.disconnect	= gf2k_disconnect,
};

static int __init gf2k_init(void)
{
	return gameport_register_driver(&gf2k_drv);
}

static void __exit gf2k_exit(void)
{
	gameport_unregister_driver(&gf2k_drv);
}

module_init(gf2k_init);
module_exit(gf2k_exit);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      /*******************************************************************
 * This file is part of the Emulex Linux Device Driver for         *
 * Fibre Channel Host Bus Adapters.                                *
 * Copyright (C) 2004-2009 Emulex.  All rights reserved.           *
 * EMULEX and SLI are trademarks of Emulex.                        *
 * www.emulex.com                                                  *
 *                                                                 *
 * This program is free software; you can redistribute it and/or   *
 * modify it under the terms of version 2 of the GNU General       *
 * Public License as published by the Free Software Foundation.    *
 * This program is distributed in the hope that it will be useful. *
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND          *
 * WARRANTIES, INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY,  *
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE      *
 * DISCLAIMED, EXCEPT TO THE EXTENT THAT SUCH DISCLAIMERS ARE HELD *
 * TO BE LEGALLY INVALID.  See the GNU General Public License for  *
 * more details, a copy of which can be found in the file COPYING  *
 * included with this package.                                     *
 *******************************************************************/

#define LPFC_DRIVER_VERSION "8.3.3"

#define LPFC_DRIVER_NAME		"lpfc"
#define LPFC_SP_DRIVER_HANDLER_NAME	"lpfc:sp"
#define LPFC_FP_DRIVER_HANDLER_NAME	"lpfc:fp"

#define LPFC_MODULE_DESC "Emulex LightPulse Fibre Channel SCSI driver " \
		LPFC_DRIVER_VERSION
#define LPFC_COPYRIGHT "Copyright(c) 2004-2009 Emulex.  All rights reserved."
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              config SCSI_QLA_FC
	tristate "QLogic QLA2XXX Fibre Channel Support"
	depends on PCI && SCSI
	select SCSI_FC_ATTRS
	select FW_LOADER
	---help---
	This qla2xxx driver supports all QLogic Fibre Channel
	PCI and PCIe host adapters.

	By default, firmware for the ISP parts will be loaded
	via the Firmware Loader interface.

	ISP               Firmware Filename
	----------        -----------------
	21xx              ql2100_fw.bin
	22xx              ql2200_fw.bin
	2300, 2312, 6312  ql2300_fw.bin
	2322, 6322        ql2322_fw.bin
	24xx, 54xx        ql2400_fw.bin
	25xx              ql2500_fw.bin

	Upon request, the driver caches the firmware image until
	the driver is unloaded.

	Firmware images can be retrieved from:

		ftp://ftp.qlogic.com/outgoing/linux/firmware/
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 Addr[1];
        wd->ap.wds.macAddr[wdsPortId][2] = wdsAddr[2];

        wd->ap.wds.wdsBitmap |= (1 << wdsPortId);
        wd->ap.wds.encryMode[wdsPortId] = (u8_t) encType;

        zfCoreSetKey(dev, 10+ZM_MAX_WDS_SUPPORT, 0, (u8_t) encType, wdsAddr, wdsKey);
    }
    else
    {
        /* Disable WDS port */
        addr[0] = addr[1] = addr[2] = 0;
        key[0] = key[1] = key[2] = key[3] = 0;
        wd->ap.wds.wdsBitmap &= (~(1 << wdsPortId));
        zfCoreSetKey(dev, 10+ZM_MAX_WDS_SUPPORT, 0, ZM_NO_WEP, addr, key);
    }

    return ZM_SUCCESS;
}
#ifdef ZM_ENABLE_CENC
/* CENC */
void zfiWlanQueryGSN(zdev_t* dev, u8_t *gsn, u16_t vapId)
{
    //struct zsWlanDev* wd = (struct zsWlanDev*) zmw_wlan_dev(dev);
    u32_t txiv[4];
    zmw_get_wlan_dev(dev);

    /* convert little endian to big endian for 32 bits */
    txiv[3] = wd->ap.txiv[vapId][0];
    txiv[2] = wd->ap.txiv[vapId][1];
    txiv[1] = wd->ap.txiv[vapId][2];
    txiv[0] = wd->ap.txiv[vapId][3];

    zfMemoryCopy(gsn, (u8_t*)txiv, 16);
}
#endif //ZM_ENABLE_CENC
//CWYang(+)
void zfiWlanQuerySignalInfo(zdev_t* dev, u8_t *buffer)
{
    zmw_get_wlan_dev(dev);

    /*Change Signal Strength/Quality Value to Human Sense Here*/

    buffer[0] = wd->SignalStrength;
    buffer[1] = wd->SignalQuality;
}

/* OS-XP */
u16_t zfiStaAddIeWpaRsn(zdev_t* dev, zbuf_t* buf, u16_t offset, u8_t frameType)
{
    return  zfStaAddIeWpaRsn(dev, buf, offset, frameType);
}

/* zfiDebugCmd                                                                        */
/*     cmd       value-description                                                  */
/*         0       schedule timer                                                     */
/*         1       cancel timer                                                         */
/*         2       clear timer                                                           */
/*         3       test timer                                                            */
/*         4                                                                                 */
/*         5                                                                                 */
/*         6       checksum test     0/1                                           */
/*         7       enableProtectionMode                                          */
/*         8       rx packet content dump    0/1                               */

u32_t zfiDebugCmd(zdev_t* dev, u32_t cmd, u32_t value)
{
    u16_t event;
    u32_t tick;
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();


    zmw_enter_critical_section(dev);

    if ( cmd == 0 )
    {   /* schedule timer */
        event = (u16_t) ((value >> 16) & 0xffff);
        tick = value & 0xffff;
        zfTimerSchedule(dev, event, tick);
    }
    else if ( cmd == 1 )
    {   /* cancel timer */
        event = (u16_t) (value & 0xffff);
        zfTimerCancel(dev, event);
    }
    else if ( cmd == 2 )
    {   /* clear timer */
        zfTimerClear(dev);
    }
    else if ( cmd == 3 )
    {   /* test timer */
        zfTimerSchedule(dev, 1,  500);
        zfTimerSchedule(dev, 2, 1000);
        zfTimerSchedule(dev, 3, 1000);
        zfTimerSchedule(dev, 4, 1000);
        zfTimerSchedule(dev, 5, 1500);
        zfTimerSchedule(dev, 6, 2000);
        zfTimerSchedule(dev, 7, 2200);
        zfTimerSchedule(dev, 6, 2500);
        zfTimerSchedule(dev, 8, 2800);
    }
    else if ( cmd == 4)
    {
        zfTimerSchedule(dev, 1,  500);
        zfTimerSchedule(dev, 2, 1000);
        zfTimerSchedule(dev, 3, 1000);
        zfTimerSchedule(dev, 4, 1000);
        zfTimerSchedule(dev, 5, 1500);
        zfTimerSchedule(dev, 6, 2000);
        zfTimerSchedule(dev, 7, 2200);
        zfTimerSchedule(dev, 6, 2500);
        zfTimerSchedule(dev, 8, 2800);
        zfTimerCancel(dev, 1);
        zfTimerCancel(dev, 3);
        zfTimerCancel(dev, 6);
    }
    else if ( cmd == 5 )
    {
        wd->sta.keyId = (u8_t) value;
    }
	else if ( cmd == 6 )
	{
	    /* 0: normal    1: always set TCP/UDP checksum zero */
        wd->checksumTest = value;
MD);
	urccmd &= ~(URCCMD_EARV | URCCMD_SIZE_MASK);
	le_writel(urccmd | URCCMD_ACTIVE | URCCMD_IWR | cmd_size,
		  whcrc->rc_base + URCCMD);
	spin_unlock(&whcrc->irq_lock);

	return 0;
}

static int whcrc_reset(struct uwb_rc *rc)
{
	struct whcrc *whcrc = rc->priv;

	return umc_controller_reset(whcrc->umc_dev);
}

/**
 * Reset event reception mechanism and tell hw we are ready to get more
 *
 * We have read all the events in the event buffer, so we are ready to
 * reset it to the beginning.
 *
 * This is only called during initialization or after an event buffer
 * has been retired.  This means we can be sure that event processing
 * is disabled and it's safe to update the URCEVTADDR register.
 *
 * There's no need to wait for the event processing to start as the
 * URC will not clear URCCMD_ACTIVE until (internal) event buffer
 * space is available.
 */
static
void whcrc_enable_events(struct whcrc *whcrc)
{
	u32 urccmd;

	le_writeq(whcrc->evt_dma_buf, whcrc->rc_base + URCEVTADDR);

	spin_lock(&whcrc->irq_lock);
	urccmd = le_readl(whcrc->rc_base + URCCMD) & ~URCCMD_ACTIVE;
	le_writel(urccmd | URCCMD_EARV, whcrc->rc_base + URCCMD);
	spin_unlock(&whcrc->irq_lock);
}

static void whcrc_event_work(struct work_struct *work)
{
	struct whcrc *whcrc = container_of(work, struct whcrc, event_work);
	size_t size;
	u64 urcevtaddr;

	urcevtaddr = le_readq(whcrc->rc_base + URCEVTADDR);
	size = urcevtaddr & URCEVTADDR_OFFSET_MASK;

	uwb_rc_neh_grok(whcrc->uwb_rc, whcrc->evt_buf, size);
	whcrc_enable_events(whcrc);
}

/**
 * Catch interrupts?
 *
 * We ack inmediately (and expect the hw to do the right thing and
 * raise another IRQ if things have changed :)
 */
static
irqreturn_t whcrc_irq_cb(int irq, void *_whcrc)
{
	struct whcrc *whcrc = _whcrc;
	struct device *dev = &whcrc->umc_dev->dev;
	u32 urcsts;

	urcsts = le_readl(whcrc->rc_base + URCSTS);
	if (!(urcsts & URCSTS_INT_MASK))
		return IRQ_NONE;
	le_writel(urcsts & URCSTS_INT_MASK, whcrc->rc_base + URCSTS);

	if (urcsts & URCSTS_HSE) {
		dev_err(dev, "host system error -- hardware halted\n");
		/* FIXME: do something sensible here */
		goto out;
	}
	if (urcsts & URCSTS_ER)
		schedule_work(&whcrc->event_work);
	if (urcsts & URCSTS_RCI)
		wake_up_all(&whcrc->cmd_wq);
out:
	return IRQ_HANDLED;
}


/**
 * Initialize a UMC RC interface: map regions, get (shared) IRQ
 */
static
int whcrc_setup_rc_umc(struct whcrc *whcrc)
{
	int result = 0;
	struct device *dev = &whcrc->umc_dev->dev;
	struct umc_dev *umc_dev = whcrc->umc_dev;

	whcrc->area = umc_dev->resource.start;
	whcrc->rc_len = umc_dev->resource.end - umc_dev->resource.start + 1;
	result = -EBUSY;
	if (request_mem_region(whcrc->area, whcrc->rc_len, KBUILD_MODNAME) == NULL) {
		dev_err(dev, "can't request URC region (%zu bytes @ 0x%lx): %d\n",
			whcrc->rc_len, whcrc->area, result);
		goto error_request_region;
	}

	whcrc->rc_base = ioremap_nocache(whcrc->area, whcrc->rc_len);
	if (whcrc->rc_base == NULL) {
		dev_err(dev, "can't ioremap registers (%zu bytes @ 0x%lx): %d\n",
			whcrc->rc_len, whcrc->area, result);
		goto error_ioremap_nocache;
	}

	result = request_irq(umc_dev->irq, whcrc_irq_cb, IRQF_SHARED,
			     KBUILD_MODNAME, whcrc);
	if (result < 0) {
		dev_err(dev, "can't allocate IRQ %d: %d\n",
			umc_dev->irq, result);
		goto error_request_irq;
	}

	result = -ENOMEM;
	whcrc->cmd_buf = dma_alloc_coherent(&umc_dev->dev, PAGE_SIZE,
					    &whcrc->cmd_dma_buf, GFP_KERNEL);
	if (whcrc->cmd_buf == NULL) {
		dev_err(dev, "Can't allocate cmd transfer buffer\n");
		goto error_cmd_buffer;
	}

	whcrc->evt_buf = dma_alloc_coherent(&umc_dev->dev, PAGE_SIZE,
					    &whcrc->evt_dma_buf, GFP_KERNEL);
	if (whcrc->evt_buf == NULL) {
		dev_err(dev, "Can't allocate evt transfer buffer\n");
		goto error_evt_buffer;
	}
	return 0;

error_evt_buffer:
	dma_free_coherent(&umc_dev->dev, PAGE_SIZE, whcrc->cmd_buf,
			  whcrc->cmd_dma_buf);
error_cmd_buffer:
	free_irq(umc_dev->irq, whcrc);
error_request_irq:
	iounmap(whcrc->rc_base);
error_ioremap_nocache:
	release_mem_region(whcrc->area, whcrc->rc_len);
error_request_region:
	return result;
}


/**
 /   Adapter->ZD80211HSetting.DFSEnable=TRUE;
    //}
    //else
    //{
    //   Adapter->ZD80211HSetting.DFSEnable=FALSE;
    //}

    wd->sta.DFSEnable = mode;
    if (mode)
        wd->sta.capability[1] |= ZM_BIT_0;
    else
        wd->sta.capability[1] &= (~ZM_BIT_0);

    return 0;
}

u8_t zfiWlanSetDot11HTPCMode(zdev_t* dev, u8_t mode)
{
    zmw_get_wlan_dev(dev);

    // TODO : TPC Enable in 5150~5350 MHz and 5470~5725MHz.
    //if ( Adapter->ZD80211HSupport &&
    //   Adapter->CardSetting.NetworkTypeInUse == Ndis802_11OFDM5 &&
    //   ((ChannelNo == 36 || ChannelNo == 40 || ChannelNo == 44 || ChannelNo == 48) ||	//5150~5250 MHZ , Not Japan
    //    (ChannelNo >=52 && ChannelNo <= 64) ||				//5250~5350 MHZ
    //    (ChannelNo >=100 && ChannelNo <= 140))) 			//5470~5725 MHZ
    //{
    //   Adapter->ZD80211HSetting.TPCEnable=TRUE;
    //}
    //else
    //{
    //   Adapter->ZD80211HSetting.TPCEnable=FALSE;
    //}

    wd->sta.TPCEnable = mode;
    if (mode)
        wd->sta.capability[1] |= ZM_BIT_0;
    else
        wd->sta.capability[1] &= (~ZM_BIT_0);

    return 0;
}

u8_t zfiWlanSetAniMode(zdev_t* dev, u8_t mode)
{
    zmw_get_wlan_dev(dev);

    wd->aniEnable = mode;
    if (mode)
        zfHpAniAttach(dev);

    return 0;
}

#ifdef ZM_OS_LINUX_FUNC
void zfiWlanShowTally(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    zm_msg1_mm(ZM_LV_0, "Hw_UnderrunCnt    = ", wd->commTally.Hw_UnderrunCnt);
    zm_msg1_mm(ZM_LV_0, "Hw_TotalRxFrm     = ", wd->commTally.Hw_TotalRxFrm);
    zm_msg1_mm(ZM_LV_0, "Hw_CRC32Cnt       = ", wd->commTally.Hw_CRC32Cnt);
    zm_msg1_mm(ZM_LV_0, "Hw_CRC16Cnt       = ", wd->commTally.Hw_CRC16Cnt);
    zm_msg1_mm(ZM_LV_1, "Hw_DecrypErr_UNI  = ", wd->commTally.Hw_DecrypErr_UNI);
    zm_msg1_mm(ZM_LV_0, "Hw_RxFIFOOverrun  = ", wd->commTally.Hw_RxFIFOOverrun);
    zm_msg1_mm(ZM_LV_1, "Hw_DecrypErr_Mul  = ", wd->commTally.Hw_DecrypErr_Mul);
    zm_msg1_mm(ZM_LV_1, "Hw_RetryCnt       = ", wd->commTally.Hw_RetryCnt);
    zm_msg1_mm(ZM_LV_0, "Hw_TotalTxFrm     = ", wd->commTally.Hw_TotalTxFrm);
    zm_msg1_mm(ZM_LV_0, "Hw_RxTimeOut      = ", wd->commTally.Hw_RxTimeOut);
    zm_msg1_mm(ZM_LV_0, "Tx_MPDU           = ", wd->commTally.Tx_MPDU);
    zm_msg1_mm(ZM_LV_0, "BA_Fail           = ", wd->commTally.BA_Fail);
    zm_msg1_mm(ZM_LV_0, "Hw_Tx_AMPDU       = ", wd->commTally.Hw_Tx_AMPDU);
    zm_msg1_mm(ZM_LV_0, "Hw_Tx_MPDU        = ", wd->commTally.Hw_Tx_MPDU);

    zm_msg1_mm(ZM_LV_1, "Hw_RxMPDU          = ", wd->commTally.Hw_RxMPDU);
    zm_msg1_mm(ZM_LV_1, "Hw_RxDropMPDU      = ", wd->commTally.Hw_RxDropMPDU);
    zm_msg1_mm(ZM_LV_1, "Hw_RxDelMPDU       = ", wd->commTally.Hw_RxDelMPDU);
    zm_msg1_mm(ZM_LV_1, "Hw_RxPhyMiscError  = ", wd->commTally.Hw_RxPhyMiscError);
    zm_msg1_mm(ZM_LV_1, "Hw_RxPhyXRError    = ", wd->commTally.Hw_RxPhyXRError);
    zm_msg1_mm(ZM_LV_1, "Hw_RxPhyOFDMError  = ", wd->commTally.Hw_RxPhyOFDMError);
    zm_msg1_mm(ZM_LV_1, "Hw_RxPhyCCKError   = ", wd->commTally.Hw_RxPhyCCKError);
    zm_msg1_mm(ZM_LV_1, "Hw_RxPhyHTError    = ", wd->commTally.Hw_RxPhyHTError);
    zm_msg1_mm(ZM_LV_1, "Hw_RxPhyTotalCount = ", wd->commTally.Hw_RxPhyTotalCount);

    if (!((wd->commTally.Tx_MPDU == 0) && (wd->commTally.BA_Fail == 0)))
    {
        zm_debug_msg_p("BA Fail Ratio(%)  = ", wd->commTally.BA_Fail * 100,
                (wd->commTally.BA_Fail + wd->commTally.Tx_MPDU));
    }

    if (!((wd->commTally.Hw_Tx_MPDU == 0) && (wd->commTally.Hw_Tx_AMPDU == 0)))
    {
        zm_debug_msg_p("Avg Agg Number    = ",
                wd->commTally.Hw_Tx_MPDU, wd->commTally.Hw_Tx_AMPDU);
    }
}
#endif

void zfiWlanSetMaxTxPower(zdev_t* dev, u8_t power2, u8_t power5)
{
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);
    wd->maxTxPower2 = power2;
    wd->maxTxPower5 = power5;
    zmw_leave_critical_section(dev);
}

void zfiWlanQueryMaxTxPower(zdev_t* dev, u8_t *power2, u8_t *power5)
{
    zmw_get_wlan_dev(dev);

    *power2 = wd->maxTxPower2;
    *power5 = wd->maxTxPower5;
}

void zfiWlanSetConnectMode(zdev_t* dev, u8_t mode)
{
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);
    wd->connectMode = mode;
    zmw_leave_critical_section(dev);
}

void zfiWlanSetSupportMode(zdev_t* dev, u32_t mode)
{
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);
    wd->supportMode = mode;
    zmw_leave_critical_section(dev);
}

void zfiWlanSetAdhocMode(zdev_t* dev, u32_t mode)
{
    zmw_get_wlan_dev(dev);

    wd->ws.adhocMode = mode;
}

u32_t zfiWlanQueryAdhocMode(zdev_t* dev, u8_t bWrapper)
{
    u32_t adhocMode;

    zmw_get_wlan_dev(dev);

    if ( bWrapper )
    {
        adhocMode = wd->ws.adhocMode;
    }
    else
    {
        adhocMode = wd->wfc.bIbssGMode;
    }

    return adhocMode;
}


u8_t zfiWlanSetCountryIsoName(zdev_t* dev, u8_t *countryIsoName, u8_t length)
{
    u8_t buf[5];
    zmw_get_wlan_dev(dev);

    if (length == 4)
    {
        buf[2] = wd->ws.countryIsoName[0] = countryIsoName[2];
        buf[3] = wd->ws.countryIsoName[1] = countryIsoName[1];
        buf[4] = wd->ws.countryIsoName[2] = countryIsoName[0];
    }
    else if (length == 3)
    {
        buf[2] = wd->ws.countryIsoName[0] = countryIsoName[1];
        buf[3] = wd->ws.countryIsoName[1] = countryIsoName[0];
        buf[4] = wd->ws.countryIsoName[2] = '\0';
    }
    else
    {
        return 1;
    }

    return zfHpGetRegulationTablefromISO(dev, buf, length);
}


const char* zfiWlanQueryCountryIsoName(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->ws.countryIsoName;
}



void zfiWlanSetRegulatory(zdev_t* dev, u8_t CCS, u16_t Code, u8_t bfirstChannel)
{
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    if (CCS)
    {
        /* Reset Regulation Table by Country Code */
        zfHpGetRegulationTablefromCountry(dev, Code);
    }
    else
    {
        /* Reset Regulation Table by Region Code */
        zfHpGetRegulationTablefromRegionCode(dev, Code);
    }

    if (bfirstChannel) {
        zmw_enter_critical_section(dev);
        wd->frequency = zfChGetFirstChannel(dev, NULL);
        zmw_leave_critical_section(dev);
        zfCoreSetFrequency(dev, wd->frequency);
    }
}


const char* zfiHpGetisoNamefromregionCode(zdev_t* dev, u16_t regionCode)
{
    return zfHpGetisoNamefromregionCode(dev, regionCode);
}

u16_t zfiWlanChannelToFrequency(zdev_t* dev, u8_t channel)
{
    return zfChNumToFreq(dev, channel, 0);
}

u8_t zfiWlanFrequencyToChannel(zdev_t* dev, u16_t freq)
{
    u8_t is5GBand = 0;

    return zfChFreqToNum(freq, &is5GBand);
}

void zfiWlanDisableDfsChannel(zdev_t* dev, u8_t disableFlag)
{
    zfHpDisableDfsChannel(dev, disableFlag);
    return;
}

void zfiWlanSetLEDCtrlParam(zdev_t* dev, u8_t type, u8_t flag)
{
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);
    wd->ledStruct.LEDCtrlType = type;
    wd->ledStruct.LEDCtrlFlagFromReg  = flag;
    zmw_leave_critical_section(dev);
}

void zfiWlanEnableLeapConfig(zdev_t* dev, u8_t leapEnabled)
{
    zmw_get_wlan_dev(dev);

    wd->sta.leapEnabled = leapEnabled;
}

u32_t zfiWlanQueryHwCapability(zdev_t* dev)
{
    return zfHpCapability(dev);
}

u32_t zfiWlanQueryReceivedPacket(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->sta.ReceivedPktRatePerSecond;
}

void zfiWlanCheckSWEncryption(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    if (wd->sta.SWEncryptEnable != 0)
    {
        zfHpSWDecrypt(dev, 1);
    }
}

u16_t zfiWlanQueryAllowChannels(zdev_t* dev, u16_t *channels)
{
    u16_t ii;
    zmw_get_wlan_dev(dev);

    for (ii = 0; ii < wd->regulationTable.allowChannelCnt; ii++)
    {
        channels[ii] = wd->regulationTable.allowChannel[ii].channel;
    }

    return wd->regulationTable.allowChannelCnt;
}

void zfiWlanSetDynamicSIFSParam(zdev_t* dev, u8_t val)
{
    zmw_get_wlan_dev(dev);

    wd->dynamicSIFSEnable = val;

    zm_debug_msg1("wd->dynamicSIFSEnable = ", wd->dynamicSIFSEnable)
}

u16_t zfiWlanGetMulticastAddressCount(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->sta.multicastList.size;
}

void zfiWlanGetMulticastList(zdev_t* dev, u8_t* pMCList)
{
    struct zsMulticastAddr* pMacList = (struct zsMulticastAddr*) pMCList;
    u8_t i;

    zmw_get_wlan_dev(dev);

    for ( i=0; i<wd->sta.multicastList.size; i++ )
    {
        zfMemoryCopy(pMacList[i].addr, wd->sta.multicastList.macAddr[i].addr, 6);
    }
}

void zfiWlanSetPacketFilter(zdev_t* dev, u32_t PacketFilter)
{
    u8_t  bAllMulticast = 0;
    u32_t oldFilter;

    zmw_get_wlan_dev(dev);

    oldFilter = wd->sta.osRxFilter;

    wd->sta.osRxFilter = PacketFilter;

    if ((oldFilter & ZM_PACKET_TYPE_ALL_MULTICAST) !=
        (wd->sta.osRxFilter & ZM_PACKET_TYPE_ALL_MULTICAST))
    {
        if ( wd->sta.osRxFilter & ZM_PACKET_TYPE_ALL_MULTICAST )
            bAllMulticast = 1;
        zfHpSetMulticastList(dev, wd->sta.multicastList.size,
                             (u8_t*)wd->sta.multicastList.macAddr, bAllMulticast);
    }
}

u8_t zfiCompareWithMulticastListAddress(zdev_t* dev, u16_t* dstMacAddr)
{
    u8_t i;
    u8_t bIsInMCListAddr = 0;

    zmw_get_wlan_dev(dev);

    for ( i=0; i<wd->sta.multicastList.size; i++ )
    {
    	if ( zfwMemoryIsEqual((u8_t*)dstMacAddr, (u8_t*)wd->sta.multicastList.macAddr[i].addr, 6) )
    	{
            bIsInMCListAddr = 1;
            break;
    	}
    }

    return bIsInMCListAddr;
}

void zfiWlanSetSafeModeEnabled(zdev_t* dev, u8_t safeMode)
{
    zmw_get_wlan_dev(dev);

    wd->sta.bSafeMode = safeMode;

    if ( safeMode )
    	zfStaEnableSWEncryption(dev, 1);
    else
        zfStaDisableSWEncryption(dev);
}

void zfiWlanSetIBSSAdditionalIELength(zdev_t* dev, u32_t ibssAdditionalIESize, u8_t* ibssAdditionalIE)
{
	zmw_get_wlan_dev(dev);

	if ( ibssAdditionalIESize )
    {
	    wd->sta.ibssAdditionalIESize = ibssAdditionalIESize;
        zfMemoryCopy(wd->sta.ibssAdditionalIE, ibssAdditionalIE, (u16_t)ibssAdditionalIESize);
    }
    else
    	wd->sta.ibssAdditionalIESize = 0;
}

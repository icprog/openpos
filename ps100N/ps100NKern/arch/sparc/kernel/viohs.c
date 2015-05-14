/* viohs.c: LDOM Virtual I/O handshake helper layer.
 *
 * Copyright (C) 2007 David S. Miller <davem@davemloft.net>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <asm/ldc.h>
#include <asm/vio.h>

int vio_ldc_send(struct vio_driver_state *vio, void *data, int len)
{
	int err, limit = 1000;

	err = -EINVAL;
	while (limit-- > 0) {
		err = ldc_write(vio->lp, data, len);
		if (!err || (err != -EAGAIN))
			break;
		udelay(1);
	}

	return err;
}
EXPORT_SYMBOL(vio_ldc_send);

static int send_ctrl(struct vio_driver_state *vio,
		     struct vio_msg_tag *tag, int len)
{
	tag->sid = vio_send_sid(vio);
	return vio_ldc_send(vio, tag, len);
}

static void init_tag(struct vio_msg_tag *tag, u8 type, u8 stype, u16 stype_env)
{
	tag->type = type;
	tag->stype = stype;
	tag->stype_env = stype_env;
}

static int send_version(struct vio_driver_state *vio, u16 major, u16 minor)
{
	struct vio_ver_info pkt;

	vio->_local_sid = (u32) sched_clock();

	memset(&pkt, 0, sizeof(pkt));
	init_tag(&pkt.tag, VIO_TYPE_CTRL, VIO_SUBTYPE_INFO, VIO_VER_INFO);
	pkt.major = major;
	pkt.minor = minor;
	pkt.dev_class = vio->dev_class;

	viodbg(HS, "SEND VERSION INFO maj[%u] min[%u] devclass[%u]\n",
	       major, minor, vio->dev_class);

	return send_ctrl(vio, &pkt.tag, sizeof(pkt));
}

static int start_handshake(struct vio_driver_state *vio)
{
	int err;

	viodbg(HS, "START HANDSHAKE\n");

	vio->hs_state = VIO_HS_INVALID;

	err = send_version(vio,
			   vio->ver_table[0].major,
			   vio->ver_table[0].minor);
	if (err < 0)
		return err;

	return 0;
}

static void flush_rx_dring(struct vio_driver_state *vio)
{
	struct vio_dring_state *dr;
	u64 ident;

	BUG_ON(!(vio->dr_state & VIO_DR_STATE_RXREG));

	dr = &vio->drings[VIO_DRIVER_RX_RING];
	ident = dr->ident;

	BUG_ON(!vio->desc_buf);
	kfree(vio->desc_buf);
	vio->desc_buf = NULL;

	memset(dr, 0, sizeof(*dr));
	dr->ident = ident;
}

void vio_link_state_change(struct vio_driver_state *vio, int event)
{
	if (event == LDC_EVENT_UP) {
		vio->hs_state = VIO_HS_INVALID;

		switch (vio->dev_class) {
		case VDEV_NETWORK:
		case VDEV_NETWORK_SWITCH:
			vio->dr_state = (VIO_DR_STATE_TXREQ |
					 VIO_DR_STATE_RXREQ);
			break;

		case VDEV_DISK:
			vio->dr_state = VIO_DR_STATE_TXREQ;
			break;
		case VDEV_DISK_SERVER:
			vio->dr_state = VIO_DR_STATE_RXREQ;
			break;
		}
		start_handshake(vio);
	} else if (event == LDC_EVENT_RESET) {
		vio->hs_state = VIO_HS_INVALID;

		if (vio->dr_state & VIO_DR_STATE_RXREG)
			flush_rx_dring(vio);

		vio->dr_state = 0x00;
		memset(&vio->ver, 0, sizeof(vio->ver));

		ldc_disconnect(vio->lp);
	}
}
EXPORT_SYMBOL(vio_link_state_change);

static int handshake_failure(struct vio_driver_state *vio)
{
	struct vio_dring_state *dr;

	/* XXX Put policy here...  Perhaps start a timer to fire
	 * XXX in 100 ms, which will bring the link up and retry
	 * XXX the handshake.
	 */

	viodbg(HS, "HANDSHAKE FAILURE\n");

	vio->dr_state &= ~(VIO_DR_STATE_TXREG |
			   VIO_DR_STATE_RXREG);

	dr = &vio->drings[VIO_DRIVER_RX_RING];
	memset(dr, 0, sizeof(*dr));

	kfree(vio->desc_buf);
	vio->desc_buf = NULL;
	vio->desc_buf_len = 0;

	vio->hs_state = VIO_HS_INVALID;

	return -ECONNRESET;
}

static int process_unknown(struct vio_driver_state *vio, void *arg)
{
	struct vio_msg_tag *pkt = arg;

	viodbg(HS, "UNKNOWN CONTROL [%02x:%02x:%04x:%08x]\n",
	       pkt->type, pkt->stype, pkt->stype_env, pkt->sid);

	printk(KERN_ERR "vio: ID[%lu] Resetting connection.\n",
	       vio->vdev->channel_id);

	ldc_disconnect(vio->lp);

	return -ECONNRESET;
}

static int send_dreg(struct vio_driver_state *vio)
{
	struct vio_dring_state *dr = &vio->drings[VIO_DRIVER_TX_RING];
	union {
		struct vio_dring_register pkt;
		char all[sizeof(struct vio_dring_register) +
			 (sizeof(struct ldc_trans_cookie) *
			  dr->ncookies)];
	} u;
	int i;

	memset(&u, 0, sizeof(u));
	init_tag(&u.pkt.tag, VIO_TYPE_CTRL, VIO_SUBTYPE_INFO, VIO_DRING_REG);
	u.pkt.dring_ident = 0;
	u.pkt.num_descr = dr->num_entries;
	u.pkt.descr_size = dr->ent/*
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
 *
 * File: tkip.h
 *
 * Purpose: Implement functions for 802.11i TKIP
 *
 * Author: Jerry Chen
 *
 * Date: Mar. 11, 2003
 *
 */


#ifndef __TKIP_H__
#define __TKIP_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

#if !defined(__TETHER_H__)
#include "tether.h"
#endif



/*---------------------  Export Definitions -------------------------*/
#define TKIP_KEY_LEN        16

/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

VOID TKIPvMixKey(
    PBYTE   pbyTKey,
    PBYTE   pbyTA,
    WORD    wTSC15_0,
    DWORD   dwTSC47_16,
    PBYTE   pbyRC4Key
    );

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */


#endif // __TKIP_H__



                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 vio,
			  struct vio_dring_unregister *pkt)
{
	struct vio_dring_state *dr = &vio->drings[VIO_DRIVER_RX_RING];

	viodbg(HS, "GOT DRING_UNREG\n");

	if (pkt->dring_ident != dr->ident)
		return 0;

	vio->dr_state &= ~VIO_DR_STATE_RXREG;

	memset(dr, 0, sizeof(*dr));

	kfree(vio->desc_buf);
	vio->desc_buf = NULL;
	vio->desc_buf_len = 0;

	return 0;
}

static int process_rdx_info(struct vio_driver_state *vio, struct vio_rdx *pkt)
{
	viodbg(HS, "GOT RDX INFO\n");

	pkt->tag.stype = VIO_SUBTYPE_ACK;
	viodbg(HS, "SEND RDX ACK\n");
	if (send_ctrl(vio, &pkt->tag, sizeof(*pkt)) < 0)
		return handshake_failure(vio);

	vio->hs_state |= VIO_HS_SENT_RDX_ACK;
	return 0;
}

static int process_rdx_ack(struct vio_driver_state *vio, struct vio_rdx *pkt)
{
	viodbg(HS, "GOT RDX ACK\n");

	if (!(vio->hs_state & VIO_HS_SENT_RDX))
		return handshake_failure(vio);

	vio->hs_state |= VIO_HS_GOT_RDX_ACK;
	return 0;
}

static int process_rdx_nack(struct vio_driver_state *vio, struct vio_rdx *pkt)
{
	viodbg(HS, "GOT RDX NACK\n");

	return handshake_failure(vio);
}

static int process_rdx(struct vio_driver_state *vio, struct vio_rdx *pkt)
{
	if (!all_drings_registered(vio))
		handshake_failure(vio);

	switch (pkt->tag.stype) {
	case VIO_SUBTYPE_INFO:
		return process_rdx_info(vio, pkt);

	case VIO_SUBTYPE_ACK:
		return process_rdx_ack(vio, pkt);

	case VIO_SUBTYPE_NACK:
		return process_rdx_nack(vio, pkt);

	default:
		return handshake_failure(vio);
	}
}

int vio_control_pkt_engine(struct vio_driver_state *vio, void *pkt)
{
	struct vio_msg_tag *tag = pkt;
	u8 prev_state = vio->hs_state;
	int err;

	switch (tag->stype_env) {
	case VIO_VER_INFO:
		err = process_ver(vio, pkt);
		break;

	case VIO_ATTR_INFO:
		err = process_attr(vio, pkt);
		break;

	case VIO_DRING_REG:
		err = process_dreg(vio, pkt);
		break;

	case VIO_DRING_UNREG:
		err = process_dunreg(vio, pkt);
		break;

	case VIO_RDX:
		err = process_rdx(vio, pkt);
		break;

	default:
		err = process_unknown(vio, pkt);
		break;
	}
	if (!err &&
	    vio->hs_state != prev_state &&
	    (vio->hs_state & VIO_HS_COMPLETE))
		vio->ops->handshake_complete(vio);

	return err;
}
EXPORT_SYMBOL(vio_control_pkt_engine);

void vio_conn_reset(struct vio_driver_state *vio)
{
}
EXPORT_SYMBOL(vio_conn_reset);

/* The issue is that the Solaris virtual disk server just mirrors the
 * SID values it gets from the client peer.  So we work around that
 * here in vio_{validate,send}_sid() so that the drivers don't need
 * to be aware of this crap.
 */
int vio_validate_sid(struct vio_driver_state *vio, struct vio_msg_tag *tp)
{
	u32 sid;

	/* Always let VERSION+INFO packets through unchecked, they
	 * define the new SID.
	 */
	if (tp->type == VIO_TYPE_CTRL &&
	    tp->stype == VIO_SUBTYPE_INFO &&
	    tp->stype_env == VIO_VER_INFO)
		return 0;

	/* Ok, now figure out which SID to use.  */
	switch (vio->dev_class) {
	case VDEV_NETWORK:
	case VDEV_NETWORK_SWITCH:
	case VDEV_DISK_SERVER:
	default:
		sid = vio->_peer_sid;
		break;

	case VDEV_DISK:
		sid = vio->_local_sid;
		break;
	}

	if (sid == tp->sid)
		return 0;
	viodbg(DATA, "BAD SID tag->sid[%08x] peer_sid[%08x] local_sid[%08x]\n",
	       tp->sid, vio->_peer_sid, vio->_local_sid);
	return -EINVAL;
}
EXPORT_SYMBOL(vio_validate_sid);

u32 vio_send_sid(struct vio_driver_state *vio)
{
	switch (vio->dev_class) {
	case VDEV_NETWORK:
	case VDEV_NETWORK_SWITCH:
	case VDEV_DISK:
	default:
		return vio->_local_sid;

	case VDEV_DISK_SERVER:
		return vio->_peer_sid;
	}
}
EXPORT_SYMBOL(vio_send_sid);

int vio_ldc_alloc(struct vio_driver_state *vio,
			 struct ldc_channel_config *base_cfg,
			 void *event_arg)
{
	struct ldc_channel_config cfg = *base_cfg;
	struct ldc_channel *lp;

	cfg.tx_irq = vio->vdev->tx_irq;
	cfg.rx_irq = vio->vdev->rx_irq;

	lp = ldc_alloc(vio->vdev->channel_id, &cfg, event_arg);
	if (IS_ERR(lp))
		return PTR_ERR(lp);

	vio->lp = lp;

	return 0;
}
EXPORT_SYMBOL(vio_ldc_alloc);

void vio_ldc_free(struct vio_driver_state *vio)
{
	ldc_free(vio->lp);
	vio->lp = NULL;

	kfree(vio->desc_buf);
	vio->desc_buf = NULL;
	vio->desc_buf_len = 0;
}
EXPORT_SYMBOL(vio_ldc_fr                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
dif

static int upd64083_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_UPD64083, 0);
}

static int upd64083_log_status(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 buf[7];

	i2c_master_recv(client, buf, 7);
	v4l2_info(sd, "Status: SA00=%02x SA01=%02x SA02=%02x SA03=%02x "
		      "SA04=%02x SA05=%02x SA06=%02x\n",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
	return 0;
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops upd64083_core_ops = {
	.log_status = upd64083_log_status,
	.g_chip_ident = upd64083_g_chip_ident,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = upd64083_g_register,
	.s_register = upd64083_s_register,
#endif
};

static const struct v4l2_subdev_video_ops upd64083_video_ops = {
	.s_routing = upd64083_s_routing,
};

static const struct v4l2_subdev_ops upd64083_ops = {
	.core = &upd64083_core_ops,
	.video = &upd64083_video_ops,
};

/* ------------------------------------------------------------------------ */

/* i2c implementation */

static int upd64083_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	struct upd64083_state *state;
	struct v4l2_subdev *sd;
	int i;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	v4l_info(client, "chip found @ 0x%x (%s)\n",
			client->addr << 1, client->adapter->name);

	state = kmalloc(sizeof(struct upd64083_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;
	sd = &state->sd;
	v4l2_i2c_subdev_init(sd, client, &upd64083_ops);
	/* Initially assume that a ghost reduction chip is present */
	state->mode = 0;  /* YCS mode */
	state->ext_y_adc = (1 << 5);
	memcpy(state->regs, upd64083_init, TOT_REGS);
	for (i = 0; i < TOT_REGS; i++)
		upd64083_write(sd, i, state->regs[i]);
	return 0;
}

static int upd64083_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	return 0;
}

/* ----------------------------------------------------------------------- */

static const struct i2c_device_id upd64083_id[] = {
	{ "upd64083", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, upd64083_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "upd64083",
	.probe = upd64083_probe,
	.remove = upd64083_remove,
	.id_table = upd64083_id,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          � p*     �            �            $� ��J     3�            9� �     K� @��     a�            m�            w�            ��            ��            �� CU     �� �CO     ͨ P�x     ܨ `��     � ��W    �            � @:     "� �7�     5� 0��     J� ��2     `�  �K     u� ���     �� ��     ��            �� @��    é �8     թ P�N     � ���     �� `�\     � ��^     � �3     -� `�w     ;� ��     I� ��x     X� p�-    r� ��N    �� ���     �� `�L    �� �T;!     �� P6�     �� 0�:     ʪ p��     ު  ��     �� 0��     � ���     � ��#     � ��X     /�  �     ?� ��;     L� @�^     T� ��/     ]� �4     n� �q     �� �t|     �� ���     �� ��)     �� ��      ԫ ��.     � `�E     � ��5     � ��    '� p �     6� �h    E�  �     X� �?     n� Pa     �� ��     �� `#     �� �#     Ŭ �    ج �	?    � ��     � p�     �     =� 0�    V� �M     w� @,5     �� �N     �� �`     í 0H     ޭ �H     �� :     � @m3     �  #     1� 09     H� �{     e� Ph     �� �P     �� l     �� @�     Ʈ ��     Ӯ �     � �*     � �      +� �/�     I� +D     _� `+D     v� �8�     �� �+-    �� �-�     �� p.     �� �.     �� �.     ί �.=     ܯ �.B     � 0/     � @/P     � �/�     '� �0�     7� 00     E� P0P     Y� �1�     c� �3S     q� 04(     � `4B     �� �98     ��  `5     ��  n     �� 0n     �� Pn-     �� �n-     ǰ �|2     ٰ  J,    � PL�    � �~�     �            �� Ђ�     � `�h      � Ѓ@     7� `�g     D� 0�f     Q� ��v    ]�  �R    i� ��+     q� �x+     y� �t+     �� ��+     �� �|+     �� ��i    �� ��+     �� ��+     �� ��+     �� ��+     �� `�i    ɱ �p+     ѱ @��     ۱ О4     � �i     � ��|     �� ��     �            � �    ,� ��     ;� ��a     G� �)    [�            c�            h� @�A     w� ��0     � ���     �� ��d     ��  �W     �� ��k     �� �v     ǲ  �`     ϲ p�8     � ���     �� ��     � ��4     � ��B     )� �>     4� P�     A� p�     Q� ��     b� ��0     m� ��<     }� p�     �� ��!     �� ��     ֳ Ю8     �� �J     �            � �      � p�v     #�            *�  <4     ;� �l     J�            O� `�n     ^� б     o� �     �� �^     ��            �� p�     �� 0�[     �� �o     ô ��    Ӵ ��Q     � ��    �� ���     � P�    � �-     '� ��y     4� p�p     J� �c3     _� �c3     o� �d3     �� $e3     �� �]     �� @�(     �� p�1     õ лg     ϵ @�     ܵ `�/     � �8     �� P�     � p�7    "� ��g     0� `��     E� @�Q     Z� ��9    s� P��     �� 0�k     �� ��}     �� ��P     �� ��     ζ ��P     � ��     � ��P     � ��     � @�P     "� p�#     8� ��W     F� ��7     V� ���     f� ��d     {� ��Y     �� 0�X     ��  �v     �� ��/     �� ���     ȷ `,5    � �*"     �� P/f     � �1(     !�            (�            3�            ?�            V�            p�            ��            ��            ��            /*
    file operation functions
    Copyright (C) 2003-2004  Kevin Thayer <nufan_wfk at yahoo.com>
    Copyright (C) 2004  Chris Kennedy <c@groovy.org>
    Copyright (C) 2005-2007  Hans Verkuil <hverkuil@xs4all.nl>

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ivtv-driver.h"
#include "ivtv-fileops.h"
#include "ivtv-i2c.h"
#include "ivtv-queue.h"
#include "ivtv-udma.h"
#include "ivtv-irq.h"
#include "ivtv-vbi.h"
#include "ivtv-mailbox.h"
#include "ivtv-routing.h"
#include "ivtv-streams.h"
#include "ivtv-yuv.h"
#include "ivtv-ioctl.h"
#include "ivtv-cards.h"
#include <media/saa7115.h>

/* This function tries to claim the stream for a specific file descriptor.
   If no one else is using this stream then the stream is claimed and
   associated VBI streams are also automatically claimed.
   Possible error returns: -EBUSY if someone else has claimed
   the stream or 0 on success. */
static int ivtv_claim_stream(struct ivtv_open_id *id, int type)
{
	struct ivtv *itv = id->itv;
	struct ivtv_stream *s = &itv->streams[type];
	struct ivtv_stream *s_vbi;
	int vbi_type;

	if (test_and_set_bit(IVTV_F_S_CLAIMED, &s->s_flags)) {
		/* someone already claimed this stream */
		if (s->id == id->open_id) {
			/* yes, this file descriptor did. So that's OK. */
			return 0;
		}
		if (s->id == -1 && (type == IVTV_DEC_STREAM_TYPE_VBI ||
					 type == IVTV_ENC_STREAM_TYPE_VBI)) {
			/* VBI is handled already internally, now also assign
			   the file descriptor to this stream for external
			   reading of the stream. */
			s->id = id->open_id;
			IVTV_DEBUG_INFO("Start Read VBI\n");
			return 0;
		}
		/* someone else is using this stream already */
		IVTV_DEBUG_INFO("Stream %d is busy\n", type);
		return -EBUSY;
	}
	s->id = id->open_id;
	if (type == IVTV_DEC_STREAM_TYPE_VBI) {
		/* Enable reinsertion interrupt */
		ivtv_clear_irq_mask(itv, IVTV_IRQ_DEC_VBI_RE_INSERT);
	}

	/* IVTV_DEC_STREAM_TYPE_MPG needs to claim IVTV_DEC_STREAM_TYPE_VBI,
	   IVTV_ENC_STREAM_TYPE_MPG needs to claim IVTV_ENC_STREAM_TYPE_VBI
	   (provided VBI insertion is on and sliced VBI is selected), for all
	   other streams we're done */
	if (type == IVTV_DEC_STREAM_TYPE_MPG) {
		vbi_type = IVTV_DEC_STREAM_TYPE_VBI;
	} else if (type == IVTV_ENC_STREAM_TYPE_MPG &&
		   itv->vbi.insert_mpeg && !ivtv_raw_vbi(itv)) {
		vbi_type = IVTV_ENC_STREAM_TYPE_VBI;
	} else {
		return 0;
	}
	s_vbi = &itv->streams[vbi_type];

	if (!test_and_set_bit(IVTV_F_S_CLAIMED, &s_vbi->s_flags)) {
		/* Enable reinsertion interrupt */
		if (vbi_type == IVTV_DEC_STREAM_TYPE_VBI)
			ivtv_clear_irq_mask(itv, IVTV_IRQ_DEC_VBI_RE_INSERT);
	}
	/* mark that it is used internally */
	set_bit(IVTV_F_S_INTERNAL_USE, &s_vbi->s_flags);
	return 0;
}

/* This function releases a previously claimed stream. It will take into
   account associated VBI streams. */
void ivtv_release_stream(struct ivtv_stream *s)
{
	struct ivtv *itv = s->itv;
	struct ivtv_stream *s_vbi;

	s->id = -1;
	if ((s->type == IVTV_DEC_STREAM_TYPE_VBI || s->type == IVTV_ENC_STREAM_TYPE_VBI) &&
		test_bit(IVTV_F_S_INTERNAL_USE, &s->s_flags)) {
		/* this stream is still in use internally */
		return;
	}
	if (!test_and_clear_bit(IVTV_F_S_CLAIMED, &s->s_flags)) {
		IVTV_DEBUG_WARN("Release stream %s not in use!\n", s->name);
		return;
	}

	ivtv_flush_queues(s);

	/* disable reinsertion interrupt */
	if (s->type == IVTV_DEC_STREAM_TYPE_VBI)
		ivtv_set_irq_mask(itv, IVTV_IRQ_DEC_VBI_RE_INSERT);

	/* IVTV_DEC_STREAM_TYPE_MPG needs to release IVTV_DEC_STREAM_TYPE_VBI,
	   IVTV_ENC_STREAM_TYPE_MPG needs to release IVTV_ENC_STREAM_TYPE_VBI,
	   for all other streams we're done */
	if (s->type == IVTV_DEC_STREAM_TYPE_MPG)
		s_vbi = &itv->streams[IVTV_DEC_STREAM_TYPE_VBI];
	else if (s->type == IVTV_ENC_STREAM_TYPE_MPG)
		s_vbi = &itv->streams[IVTV_ENC_STREAM_TYPE_VBI];
	else
		return;

	/* clear internal use flag */
	if (!test_and_clear_bit(IVTV_F_S_INTERNAL_USE, &s_vbi->s_flags)) {
		/* was already cleared */
		return;
	}
	if (s_vbi->id != -1) {
		/* VBI stream still claimed by a file descriptor */
		return;
	}
	/* disable reinsertion interrupt */
	if (s_vbi->type == IVTV_DEC_STREAM_TYPE_VBI)
		ivtv_set_irq_mask(itv, IVTV_IRQ_DEC_VBI_RE_INSERT);
	clear_bit(IVTV_F_S_CLAIMED, &s_vbi->s_flags);
	ivtv_flush_queues(s_vbi);
}

static void ivtv_dualwatch(struct ivtv *itv)
{
	struct v4l2_tuner vt;
	u32 new_bitmap;
	u32 new_stereo_mode;
	const u32 stereo_mask = 0x0300;
	const u32 dual = 0x0200;

	new_stereo_mode = itv->params.audio_properties & stereo_mask;
	memset(&vt, 0, sizeof(vt));
	ivtv_call_all(itv, tuner, g_tuner, &vt);
	if (vt.audmode == V4L2_TUNER_MODE_LANG1_LANG2 && (vt.rxsubchans & V4L2_TUNER_SUB_LANG2))
		new_stereo_mode = dual;

	if (new_stereo_mode == itv->dualwatch_stereo_mode)
		return;

	new_bitmap = new_stereo_mode | (itv->params.audio_properties & ~stereo_mask);

	IVTV_DEBUG_INFO("dualwatch: change stereo flag from 0x%x to 0x%x. new audio_bitmask=0x%ux\n",
			   itv->dualwatch_stereo_mode, new_stereo_mode, new_bitmap);

	if (ivtv_vapi(itv, CX2341X_ENC_SET_AUDIO_PROPERTIES, 1, new_bitmap) == 0) {
		itv->dualwatch_stereo_mode = new_stereo_mode;
		return;
	}
	IVTV_DEBUG_INFO("dualwatch: changing stereo flag failed\n");
}

static void ivtv_update_pgm_info(struct ivtv *itv)
{
	u32 wr_idx = (read_enc(itv->pgm_info_offset) - itv->pgm_info_offset - 4) / 24;
	int cnt;
	int i = 0;

	if (wr_idx >= itv->pgm_info_num) {
		IVTV_DEBUG_WARN("Invalid PGM index %d (>= %d)\n", wr_idx, itv->pgm_info_num);
		return;
	}
	cnt = (wr_idx + itv->pgm_info_num - itv->pgm_info_write_idx) % itv->pgm_info_num;
	while (i < cnt) {
		int idx = (itv->pgm_info_write_idx + i) % itv->pgm_info_num;
		struct v4l2_enc_idx_entry *e = itv->pgm_info + idx;
		u32 addr = itv->pgm_info_offset + 4 + idx * 24;
		const int mapping[8] = { -1, V4L2_ENC_IDX_FRAME_I, V4L2_ENC_IDX_FRAME_P, -1,
			V4L2_ENC_IDX_FRAME_B, -1, -1, -1 };
					// 1=I, 2=P, 4=B

		e->offset = read_enc(addr + 4) + ((u64)read_enc(addr + 8) << 32);
		if (e->offset > itv->mpg_data_received) {
			break;
		}
		e->offset += itv->vbi_data_inserted;
		e->length = read_enc(addr);
		e->pts = read_enc(addr + 16) + ((u64)(read_enc(addr + 20) & 1) << 32);
		e->flags = mapping[read_enc(addr + 12) & 7];
		i++;
	}
	itv->pgm_info_write_idx = (itv->pgm_info_write_idx + i) % itv->pgm_info_num;
}

static struct ivtv_buffer *ivtv_get_buffer(struct ivtv_stream *s, int non_block, int *err)
{
	struct ivtv *itv = s->itv;
	struct ivtv_stream *s_vbi = &itv->streams[IVTV_ENC_STREAM_TYPE_VBI];
	struct ivtv_buffer *buf;
	DEFINE_WAIT(wait);

	*err = 0;
	while (1) {
		if (s->type == IVTV_ENC_STREAM_TYPE_MPG) {
			/* Process pending program info updates and pending VBI data */
			ivtv_update_pgm_info(itv);

			if (time_after(jiffies,
				       itv->dualwatch_jiffies +
				       msecs_to_jiffies(1000))) {
				itv->dualwatch_jiffies = jiffies;
				ivtv_dualwatch(itv);
			}

			if (test_bit(IVTV_F_S_INTERNAL_USE, &s_vbi->s_flags) &&
			    !test_bit(IVTV_F_S_APPL_IO, &s_vbi->s_flags)) {
				while ((buf = ivtv_dequeue(s_vbi, &s_vbi->q_full))) {
					/* byteswap and process VBI data */
					ivtv_process_vbi_data(itv, buf, s_vbi->dma_pts, s_vbi->type);
					ivtv_enqueue(s_vbi, buf, &s_vbi->q_free);
				}
			}
			buf = &itv->vbi.sliced_mpeg_buf;
			if (buf->readpos != buf->bytesused) {
				return buf;
			}
		}

		/* do we have leftover data? */
		buf = ivtv_dequeue(s, &s->q_io);
		if (buf)
			return buf;

		/* do we have new data? */
		buf = ivtv_dequeue(s, &s->q_full);
		if (buf) {
			if ((buf->b_flags & IVTV_F_B_NEED_BUF_SWAP) == 0)
				return buf;
			buf->b_flags &= ~IVTV_F_B_NEED_BUF_SWAP;
			if (s->type == IVTV_ENC_STREAM_TYPE_MPG)
				/* byteswap MPG data */
				ivtv_buf_swap(buf);
			else if (s->type != IVTV_DEC_STREAM_TYPE_VBI) {
				/* byteswap and process VBI data */
				ivtv_process_vbi_data(itv, buf, s->dma_pts, s->type);
			}
			return buf;
		}

		/* return if end of stream */
		if (s->type != IVTV_DEC_STREAM_TYPE_VBI && !test_bit(IVTV_F_S_STREAMING, &s->s_flags)) {
			IVTV_DEBUG_INFO("EOS %s\n", s->name);
			return NULL;
		}

		/* return if file was opened with O_NONBLOCK */
		if (non_block) {
			*err = -EAGAIN;
			return NULL;
		}

		/* wait for more data to arrive */
		prepare_to_wait(&s->waitq, &wait, TASK_INTERRUPTIBLE);
		/* New buffers might have become available before we were added to the waitqueue */
		if (!s->q_full.buffers)
			schedule();
		finish_wait(&s->waitq, &wait);
		if (signal_pending(current)) {
			/* return if a signal was received */
			IVTV_DEBUG_INFO("User stopped %s\n", s->name);
			*err = -EINTR;
			return NULL;
		}
	}
}

static void ivtv_setup_sliced_vbi_buf(struct ivtv *itv)
{
	int idx = itv->vbi.inserted_frame % IVTV_VBI_FRAMES;

	itv->vbi.sliced_mpeg_buf.buf = itv->vbi.sliced_mpeg_data[idx];
	itv->vbi.sliced_mpeg_buf.bytesused = itv->vbi.sliced_mpeg_size[idx];
	itv->vbi.sliced_mpeg_buf.readpos = 0;
}

static size_t ivtv_copy_buf_to_user(struct ivtv_stream *s, struct ivtv_buffer *buf,
		char __user *ubuf, size_t ucount)
{
	struct ivtv *itv = s->itv;
	size_t len = buf->bytesused - buf->readpos;

	if (len > ucount) len = ucount;
	if (itv->vbi.insert_mpeg && s->type == IVTV_ENC_STREAM_TYPE_MPG &&
	    !ivtv_raw_vbi(itv) && buf != &itv->vbi.sliced_mpeg_buf) {
		const char *start = buf->buf + buf->readpos;
		const char *p = start + 1;
		const u8 *q;
		u8 ch = itv->search_pack_header ? 0xba : 0xe0;
		int stuffing, i;

		while (start + len > p && (q = memchr(p, 0, start + len - p))) {
			p = q + 1;
			if ((char *)q + 15 >= buf->buf + buf->bytesused ||
			    q[1] != 0 || q[2] != 1 || q[3] != ch) {
				continue;
			}
			if (!itv->search_pack_header) {
				if ((q[6] & 0xc0) != 0x80)
					continue;
				if (((q[7] & 0xc0) == 0x80 && (q[9] & 0xf0) == 0x20) ||
				    ((q[7] & 0xc0) == 0
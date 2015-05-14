/*
    Audio/video-routing-related ivtv functions.
    Copyright (C) 2003-2004  Kevin Thayer <nufan_wfk at yahoo.com>
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
#include "ivtv-i2c.h"
#include "ivtv-cards.h"
#include "ivtv-gpio.h"
#include "ivtv-routing.h"

#include <media/msp3400.h>
#include <media/m52790.h>
#include <media/upd64031a.h>
#include <media/upd64083.h>

/* Selects the audio input and output according to the current
   settings. */
void ivtv_audio_set_io(struct ivtv *itv)
{
	const struct ivtv_card_audio_input *in;
	u32 input, output = 0;

	/* Determine which input to use */
	if (test_bit(IVTV_F_I_RADIO_USER, &itv->i_flags))
		in = &itv->card->radio_input;
	else
		in = &itv->card->audio_inputs[itv->audio_input];

	/* handle muxer chips */
	input = in->muxer_input;
	if (itv->card->hw_muxer & IVTV_HW_M52790)
		output = M52790_OUT_STEREO;
	v4l2_subdev_call(itv->sd_muxer, audio, s_routing,
			input, output, 0);

	input = in->audio_input;
	output = 0;
	if (itv->card->hw_audio & IVTV_HW_MSP34XX)
		output = MSP_OUTPUT(MSP_SC_IN_DSP_SCART1);
	ivtv_call_hw(itv, itv->card->hw_audio, audio, s_routing,
			input, output, 0);
}

/* Selects the video input and output according to the current
   settings. */
void ivtv_video_set_io(struct ivtv *itv)
{
	int inp = itv->active_input;
	u32 input;
	u32 type;

	v4l2_subdev_call(itv->sd_video, video, s_routing,
		itv->card->video_inputs[inp].video_input, 0, 0);

	type = itv->card->video_inputs[inp].video_type;

	if (type == IVTV_CARD_INPUT_VID_TUNER) {
		input = 0;  /* Tuner */
	} else if (type < IVTV_CARD_INPUT_COMPOSITE1) {
		input = 2;  /* S-Video */
	} else {
		input = 1;  /* Composite */
	}

	if (itv->card->hw_video & IVTV_HW_GPIO)
		ivtv_call_hw(itv, IVTV_HW_GPIO, video, s_routing,
				input, 0, 0);

	if (itv->card->hw_video & IVTV_HW_UPD64031A) {
		if (type == IVTV_CARD_INPUT_VID_TUNER ||
		    type >= IVTV_CARD_INPUT_COMPOSITE1) {
			/* Composite: GR on, connect to 3DYCS */
			input = UPD64031A_GR_ON | UPD64031A_3DYCS_COMPOSITE;
		} else {
			/* S-Video: GR bypassed, turn it off */
			input = UPD64031A_GR_OFF | UPD64031A_3DYCS_DISABLE;
		}
		input |= itv->card->gr_config;

		ivtv_call_hw(itv, IVTV_HW_UPD64031A, video, s_routing,
				input, 0, 0);
	}

	if (itv->card->hw_video & IVTV_HW_UPD6408X) {
		input = UPD64083_YCS_MODE;
		if (type > IVTV_CARD_INPUT_VID_TUNER &&
		    type < IVTV_CARD_INPUT_COMPOSITE1) {
			/* S-Video uses YCNR mode and internal Y-ADC, the
			   upd64031a is not used. */
			input |= UPD64083_YCNR_MODE;
		}
		else if (itv->card->hw_video & IVTV_HW_UPD64031A) {
			/* Use upd64031a output for tuner and
			   composite(CX23416GYC only) inputs */
			if (type == IVTV_CARD_INPUT_VID_TUNER ||
			    itv->card->type == IVTV_CARD_CX23416GYC) {
				input |= UPD64083_EXT_Y_ADC;
			}
		}
		ivtv_call_hw(itv, IVTV_HW_UPD6408X, video, s_routing,
				input, 0, 0);
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                        /*
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

/*!
 * @file mx27_v4l2_capture.c
 *
 * @brief MX27 Video For Linux 2 driver
 *
 * @ingroup MXC_V4L2_CAPTURE
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/pagemap.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/fb.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/semaphore.h>
#include <linux/version.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>

#include "mxc_v4l2_capture.h"
#include "mx27_prp.h"
#include "mx27_csi.h"

static int csi_mclk_flag_backup;
static int video_nr = -1;
static cam_data *g_cam;

/*!
 * Free frame buffers
 *
 * @param cam      Structure cam_data *
 *
 * @return status  0 success.
 */
static int mxc_free_frame_buf(cam_data *cam)
{
	int i;

	for (i = 0; i < FRAME_NUM; i++) {
		if (cam->frame[i].vaddress != 0) {
			dma_free_coherent(0,
					  cam->frame[i].buffer.length,
					  cam->frame[i].vaddress,
					  cam->frame[i].paddress);
			cam->frame[i].vaddress = 0;
		}
	}

	return 0;
}

/*!
 * Allocate frame buffers
 *
 * @param cam      Structure cam_data *
 *
 * @param count    int number of buffer need to allocated
 *
 * @return status  -0 Successfully allocated a buffer, -ENOBUFS	failed.
 */
static int mxc_allocate_frame_buf(cam_data *cam, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		cam->frame[i].vaddress =
			dma_alloc_coherent(0,
					PAGE_ALIGN(cam->v2f. fmt.pix.sizeimage),
					   &cam->frame[i].paddress,
					   GFP_DMA | GFP_KERNEL);
		if (cam->frame[i].vaddress == 0) {
			pr_debug("mxc_allocate_frame_buf failed.\n");
			mxc_free_frame_buf(cam);
			return -ENOBUFS;
		}
		cam->frame[i].buffer.index = i;
		cam->frame[i].buffer.flags = V4L2_BUF_FLAG_MAPPED;
		cam->frame[i].buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cam->frame[i].buffer.length =
		    PAGE_ALIGN(cam->v2f.fmt.pix.sizeimage);
		cam->frame[i].buffer.memory = V4L2_MEMORY_MMAP;
		cam->frame[i].buffer.m.offset = cam->frame[i].paddress;
		cam->frame[i].index = i;
	}

	return 0;
}

/*!
 * Free frame buffers status
 *
 * @param cam    Structure cam_data *
 *
 * @return none
 */
static void mxc_free_frames(cam_data *cam)
{
	int i;

	for (i = 0; i < FRAME_NUM; i++) {
		cam->frame[i].buffer.flags = V4L2_BUF_FLAG_MAPPED;
	}

	cam->enc_counter = 0;
	cam->skip_frame = 0;
	INIT_LIST_HEAD(&cam->ready_q);
	INIT_LIST_HEAD(&cam->working_q);
	INIT_LIST_HEAD(&cam->done_q);
}

/*!
 * Return the buffer status
 *
 * @param cam 	   Structure cam_data *
 * @param buf      Structure v4l2_buffer *
 *
 * @return status  0 success, EINVAL failed.
 */
static int mxc_v4l2_buffer_status(cam_data *cam, struct v4l2_buffer *buf)
{
	/* check range */
	if (buf->index < 0 || buf->index >= FRAME_NUM) {
		pr_debug("mxc_v4l2_buffer_status buffers not allocated\n");
		return -EINVAL;
	}

	memcpy(buf, &(cam->frame[buf->index].buffer), sizeof(*buf));
	return 0;
}

/*!
 * start the encoder job
 *
 * @param cam      structure cam_data *
 *
 * @return status  0 Success
 */
static int mxc_streamon(cam_data *cam)
{
	struct mxc_v4l_frame *frame;
	int err = 0;

	if (!cam)
		return -EIO;

	if (list_empty(&cam->ready_q)) {
		printk(KERN_ERR "mxc_streamon buffer not been queued yet\n");
		return -EINVAL;
	}

	cam->capture_pid = current->pid;

	if (cam->enc_enable) {
		err = cam->enc_enable(cam);
		if (err != 0) {
			return err;
		}
	}

	cam->ping_pong_csi = 0;
	if (cam->enc_update_eba) {
		frame =
		    list_entry(cam->ready_q.next, struct mxc_v4l_frame, queue);
		list_del(cam->ready_q.next);
		list_add_tail(&frame->queue, &cam->working_q);
		err = cam->enc_update_eba(frame->paddress, &cam->ping_pong_csi);

		frame =
		    list_entry(cam->ready_q.next, struct mxc_v4l_frame, queue);
		list_del(cam->ready_q.next);
		list_add_tail(&frame->queue, &cam->working_q);
		err |=
		    cam->enc_update_eba(frame->paddress, &cam->ping_pong_csi);
	} else {
		return -EINVAL;
	}

	return err;
}

/*!
 * Shut down the encoder job
 *
 * @param cam      structure cam_data *
 *
 * @return status  0 Success
 */
static int mxc_streamoff(cam_data *cam)
{
	int err = 0;

	if (!cam)
		return -EIO;

	if (cam->enc_disable) {
		err = cam->enc_disable(cam);
	}
	mxc_free_frames(cam);
	return err;
}

/*!
 * Valid whether the palette is supported
 *
 * @param palette pixel format
 *
 * @return 0 if failed
 */
static inline int valid_mode(u32 palette)
{
	/*
	 * MX27 PrP channel 2 supports YUV444, but YUV444 is not
	 * defined by V4L2 :(
	 */
	return ((palette == V4L2_PIX_FMT_YUYV) ||
		(palette == V4L2_PIX_FMT_YUV420));
}

/*!
 * Valid and adjust the overlay window size, position
 *
 * @param cam      structure cam_data *
 * @param win      struct v4l2_window  *
 *
 * @return 0
 */
static int verify_preview(cam_data *cam, struct v4l2_window *win)
{
	if (cam->output >= num_registered_fb) {
		pr_debug("verify_preview No matched.\n");
		return -1;
	}
	cam->overlay_fb = (struct fb_info *)registered_fb[cam->output];

	/* TODO: suppose 16bpp, 4 bytes alignment */
	win->w.left &= ~0x1;

	if (win->w.width + win->w.left > cam->overlay_fb->var.xres)
		win->w.width = cam->overlay_fb->var.xres - win->w.left;
	if (win->w.height + win->w.top > cam->overlay_fb->var.yres)
		win->w.height = cam->overlay_fb->var.yres - win->w.top;

	/*
	 * TODO: suppose 16bpp. Rounded down to a multiple of 2 pixels for
	 * width according to PrP limitations.
	 */
	if ((cam->rotation == V4L2_MXC_ROTATE_90_RIGHT)
	    || (cam->rotation == V4L2_MXC_ROTATE_90_RIGHT_VFLIP)
	    || (cam->rotation == V4L2_MXC_ROTATE_90_RIGHT_HFLIP)
	    || (cam->rotation == V4L2_MXC_ROTATE_90_LEFT))
		win->w.height &= ~0x1;
	else
		win->w.width &= ~0x1;

	return 0;
}

/*!
 * start the viewfinder job
 *
 * @param cam      structure cam_data *
 *
 * @return status  0 Success
 */
static int start_preview(cam_data *cam)
{
	int err = 0;

	err = prp_vf_select(cam);
	if (err != 0)
		return err;

	cam->overlay_pid = current->pid;
	err = cam->vf_start_sdc(cam);

	return err;
}

/*!
 * shut down the viewfinder job
 *
 * @param cam      structure cam_data *
 *
 * @return status  0 Success
 */
static int stop_preview(cam_data *cam)
{
	int err = 0;

	err = prp_vf_deselect(cam);
	return err;
}

/*!
 * V4L2 - mxc_v4l2_g_fmt function
 *
 * @param cam         structure cam_data *
 *
 * @param f           structure v4l2_format *
 *
 * @return  status    0 success, EINVAL failed
 */
static int mxc_v4l2_g_fmt(cam_data *cam, struct v4l2_format *f)
{
	int retval = 0;

	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		f->fmt.pix.width = cam->v2f.fmt.pix.width;
		f->fmt.pix.height = cam->v2f.fmt.pix.height;
		f->fmt.pix.sizeimage = cam->v2f.fmt.pix.sizeimage;
		f->fmt.pix.pixelformat = cam->v2f.fmt.pix.pixelformat;
		f->fmt.pix.bytesperline = cam->v2f.fmt.pix.bytesperline;
		f->fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;
		retval = 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		f->fmt.win = cam->win;
		break;
	default:
		retval = -EINVAL;
	}
	return retval;
}

/*!
 * V4L2 - mxc_v4l2_s_fmt function
 *
 * @param cam         structure cam_data *
 *
 * @param f           structure v4l2_format *
 *
 * @return  status    0 success, EINVAL failed
 */
static int mxc_v4l2_s_fmt(cam_data *cam, struct v4l2_format *f)
{
	int retval = 0;
	int size = 0;
	int bytesperline = 0;

	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		if (!valid_mode(f->fmt.pix.pixelformat)) {
			pr_debug("mxc_v4l2_s_fmt: format not supported\n");
			retval = -EINVAL;
		}

		if (cam->rotation != V4L2_MXC_ROTATE_NONE)
			pr_debug("mxc_v4l2_s_fmt: capture rotation ignored\n");

		switch (f->fmt.pix.pixelformat) {
		case V4L2_PIX_FMT_YUYV:
			f->fmt.pix.width &= ~0x1;	/* Multiple of 2 */
			size = f->fmt.pix.width * f->fmt.pix.height * 2;
			bytesperline = f->fmt.pix.width * 2;
			break;
		case V4L2_PIX_FMT_YUV420:
			f->fmt.pix.width &= ~0x7;	/* Multiple of 8 */
			f->fmt.pix.height &= ~0x1;	/* Multiple of 2 */
			size = f->fmt.pix.width * f->fmt.pix.height * 3 / 2;
			bytesperline = f->fmt.pix.width * 3 / 2;
			break;
		default:
			/* Suppose it's YUV444 or 32bpp */
			size = f->fmt.pix.width * f->fmt.pix.height * 4;
			bytesperline = f->fmt.pix.width * 4;
			pr_info("mxc_v4l2_s_fmt: default assume"
				" to be YUV444 interleaved.\n");
			break;
		}

		if (f->fmt.pix.bytesperline < bytesperline) {
			f->fmt.pix.bytesperline = bytesperline;
		} else {
			bytesperline = f->fmt.pix.bytesperline;
		}

		if (f->fmt.pix.sizeimage > size) {
			pr_debug("mxc_v4l2_s_fmt: sizeimage bigger than"
				 " needed.\n");
			size = f->fmt.pix.sizeimage;
		}
		f->fmt.pix.sizeimage = size;

		cam->v2f.fmt.pix.sizeimage = size;
		cam->v2f.fmt.pix.bytesperline = bytesperline;
		cam->v2f.fmt.pix.width = f->fmt.pix.width;
		cam->v2f.fmt.pix.height = f->fmt.pix.height;
		cam->v2f.fmt.pix.pixelformat = f->fmt.pix.pixelformat;
		retval = 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		retval = verify_preview(cam, &f->fmt.win);
		cam->win = f->fmt.win;
		break;
	default:
		retval = -EINVAL;
	}
	return retval;
}

/*!
 * get control param
 *
 * @param cam         structure cam_data *
 *
 * @param c           structure v4l2_control *
 *
 * @return  status    0 success, EINVAL failed
 */
static int mxc_get_v42l_control(cam_data *cam, struct v4l2_control *c)
{
	int status = 0;

	switch (c->id) {
	case V4L2_CID_HFLIP:
		c->value = cam->rotation;
		break;
	case V4L2_CID_VFLIP:
		c->value = cam->rotation;
		break;
	case V4L2_CID_MXC_ROT:
		c->value = cam->rotation;
		break;
	case V4L2_CID_BRIGHTNESS:
		c->value = cam->bright;
		break;
	case V4L2_CID_HUE:
		c->value = cam->hue;
		break;
	case V4L2_CID_CONTRAST:
		c->value = cam->contrast;
		break;
	case V4L2_CID_SATURATION:
		c->value = cam->saturation;
		break;
	case V4L2_CID_RED_BALANCE:
		c->value = cam->red;
		break;
	case V4L2_CID_BLUE_BALANCE:
		c->value = cam->blue;
		break;
	case V4L2_CID_BLACK_LEVEL:
		c->value = cam->ae_mode;
		break;
	default:
		status = -EINVAL;
	}
	return status;
}

/*!
 * V4L2 - set_control function
 * V4L2_CID_MXC_ROT is the extention for rotation/mirroring.
 *
 * @param cam         structure cam_data *
 *
 * @param c           structure v4l2_control *
 *
 * @return  status    0 success, EINVAL failed
 */
static int mxc_set_v42l_control(cam_data *cam, struct v4l2_control *c)
{
	switch (c->id) {
	case V4L2_CID_HFLIP:
		if (c->value == 1) {
			if ((cam->rotation != V4L2_MXC_ROTATE_VERT_FLIP) &&
			    (cam->rotation != V4L2_MXC_ROTATE_180))
				cam->rotation = V4L2_MXC_ROTATE_HORIZ_FLIP;
			else
				cam->rotation = V4L2_MXC_ROTATE_180;
		} else {
			if (cam->rotation == V4L2_MXC_ROTATE_HORIZ_FLIP)
				cam->rotation = V4L2_MXC_ROTATE_NONE;
			else if (cam->rotation == V4L2_MXC_ROTATE_180)
				cam->rotation = V4L2_MXC_ROTATE_VERT_FLIP;
		}
		break;
	case V4L2_CID_VFLIP:
		if (c->value == 1) {
			if ((cam->rotation != V4L2_MXC_ROTATE_HORIZ_FLIP) &&
			    (cam->rotation != V4L2_MXC_ROTATE_180))
				cam->rotation = V4L2_MXC_ROTATE_VERT_FLIP;
			else
				cam->rotation = V4L2_MXC_ROTATE_180;
		} else {
			if (cam->rotation == V4L2_MXC_ROTATE_VERT_FLIP)
				cam->rotation = V4L2_MXC_ROTATE_NONE;
			if (cam->rotation == V4L2_MXC_ROTATE_180)
				cam->rotation = V4L2_MXC_ROTATE_HORIZ_FLIP;
		}
		break;
	case V4L2_CID_MXC_ROT:
		switch (c->value) {
		case V4L2_MXC_ROTATE_NONE:
		case V4L2_MXC_ROTATE_VERT_FLIP:
		case V4L2_MXC_ROTATE_HORIZ_FLIP:
		case V4L2_MXC_ROTATE_180:
		case V4L2_MXC_ROTATE_90_RIGHT:
		case V4L2_MXC_ROTATE_90_RIGHT_VFLIP:
		case V4L2_MXC_ROTATE_90_RIGHT_HFLIP:
		case V4L2_MXC_ROTATE_90_LEFT:
			cam->rotation = c->value;
			break;
		default:
			return -EINVAL;
		}
		break;
	case V4L2_CID_HUE:
		cam->hue = c->value;
		break;
	case V4L2_CID_CONTRAST:
		cam->contrast = c->value;
		break;
	case V4L2_CID_BRIGHTNESS:
		cam->bright = c->value;
	case V4L2_CID_SATURATION:
		cam->saturation = c->value;
	case V4L2_CID_RED_BALANCE:
		cam->red = c->value;
	case not
		 * written out to the file."
		 */
		unsigned offset = i_size & (PAGE_CACHE_SIZE - 1);

		if (page->index > end_index || !offset)
			goto confused;
		zero_user_segment(page, offset, PAGE_CACHE_SIZE);
	}

	/*
	 * This page will go to BIO.  Do we need to send this BIO off first?
	 */
	if (bio && mpd->last_block_in_bio != blocks[0] - 1)
		bio = mpage_bio_submit(WRITE, bio);

alloc_new:
	if (bio == NULL) {
		bio = mpage_alloc(bdev, blocks[0] << (blkbits - 9),
				bio_get_nr_vecs(bdev), GFP_NOFS|__GFP_HIGH);
		if (bio == NULL)
			goto confused;
	}

	/*
	 * Must try to add the page before marking the buffer clean or
	 * the confused fail path above (OOM) will be very confused when
	 * it finds all bh marked clean (i.e. it will not write anything)
	 */
	length = first_unmapped << blkbits;
	if (bio_add_page(bio, page, length, 0) < length) {
		bio = mpage_bio_submit(WRITE, bio);
		goto alloc_new;
	}

	/*
	 * OK, we have our BIO, so we can now mark the buffers clean.  Make
	 * sure to only clean buffers which we know we'll be writing.
	 */
	if (page_has_buffers(page)) {
		struct buffer_head *head = page_buffers(page);
		struct buffer_head *bh = head;
		unsigned buffer_counter = 0;

		do {
			if (buffer_counter++ == first_unmapped)
				break;
			clear_buffer_dirty(bh);
			bh = bh->b_this_page;
		} while (bh != head);

		/*
		 * we cannot drop the bh if the page is not uptodate
		 * or a concurrent readpage would fail to serialize with the bh
		 * and it would read from disk before we reach the platter.
		 */
		if (buffer_heads_over_limit && PageUptodate(page))
			try_to_free_buffers(page);
	}

	BUG_ON(PageWriteback(page));
	set_page_writeback(page);
	unlock_page(page);
	if (boundary || (first_unmapped != blocks_per_page)) {
		bio = mpage_bio_submit(WRITE, bio);
		if (boundary_block) {
			write_boundary_block(boundary_bdev,
					boundary_block, 1 << blkbits);
		}
	} else {
		mpd->last_block_in_bio = blocks[blocks_per_page - 1];
	}
	goto out;

confused:
	if (bio)
		bio = mpage_bio_submit(WRITE, bio);

	if (mpd->use_writepage) {
		ret = mapping->a_ops->writepage(page, wbc);
	} else {
		ret = -EAGAIN;
		goto out;
	}
	/*
	 * The caller has a ref on the inode, so *mapping is stable
	 */
	mapping_set_error(mapping, ret);
out:
	mpd->bio = bio;
	return ret;
}

/**
 * mpage_writepages - walk the list of dirty pages of the given address space & writepage() all of them
 * @mapping: address space structure to write
 * @wbc: subtract the number of written pages from *@wbc->nr_to_write
 * @get_block: the filesystem's block mapper function.
 *             If this is NULL then use a_ops->writepage.  Otherwise, go
 *             direct-to-BIO.
 *
 * This is a library function, which implements the writepages()
 * address_space_operation.
 *
 * If a page is already under I/O, generic_writepages() skips it, even
 * if it's dirty.  This is desirable behaviour for memory-cleaning writeback,
 * but it is INCORRECT for data-integrity system calls such as fsync().  fsync()
 * and msync() need to guarantee that all the data which was dirty at the time
 * the call was made get new I/O started against them.  If wbc->sync_mode is
 * WB_SYNC_ALL then we were called for data integrity and we must wait for
 * existing IO to complete.
 */
int
mpage_writepages(struct address_space *mapping,
		struct writeback_control *wbc, get_block_t get_block)
{
	int ret;

	if (!get_block)
		ret = generic_writepages(mapping, wbc);
	else {
		struct mpage_data mpd = {
			.bio = NULL,
			.last_block_in_bio = 0,
			.get_block = get_block,
			.use_writepage = 1,
		};

		ret = write_cache_pages(mapping, wbc, __mpage_writepage, &mpd);
		if (mpd.bio)
			mpage_bio_submit(WRITE, mpd.bio);
	}
	return ret;
}
EXPORT_SYMBOL(mpage_writepages);

int mpage_writepage(struct page *page, get_block_t get_block,
	struct writeback_control *wbc)
{
	struct mpage_data mpd = {
		.bio = NULL,
		.last_block_in_bio = 0,
		.get_block = get_block,
		.use_writepage = 0,
	};
	int ret = __mpage_writepage(page, wbc, &mpd);
	if (mpd.bio)
		mpage_bio_submit(WRITE, mpd.bio);
	return ret;
}
EXPORT_SYMBOL(mpage_mage;
	buf->
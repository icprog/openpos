h = hexium_attach,
	.detach = hexium_detach,

	.irq_mask = 0,
	.irq_func = NULL,
};

static int __init hexium_init_module(void)
{
	if (0 != saa7146_register_extension(&extension)) {
		DEB_S(("failed to register extension.\n"));
		return -ENODEV;
	}

	return 0;
}

static void __exit hexium_cleanup_module(void)
{
	saa7146_unregister_extension(&extension);
}

module_init(hexium_init_module);
module_exit(hexium_cleanup_module);

MODULE_DESCRIPTION("video4linux-2 driver for Hexium Orion frame grabber cards");
MODULE_AUTHOR("Michael Hunold <michael@mihu.de>");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        /* OmniVision OV7620/OV7120 Camera Chip Support Code
 *
 * Copyright (c) 1999-2004 Mark McClelland <mark@alpha.dyndns.org>
 * http://alpha.dyndns.org/ov511/
 *
 * OV7620 fixes by Charl P. Botha <cpbotha@ieee.org>
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
#define REG_BLUE		0x01	/* blue gain */
#define REG_RED			0x02	/* red gain */
#define REG_SAT			0x03	/* saturation */
#define REG_BRT			0x06	/* Y brightness */
#define REG_SHARP		0x07	/* analog sharpness */
#define REG_BLUE_BIAS		0x0C	/* WB blue ratio [5:0] */
#define REG_RED_BIAS		0x0D	/* WB red ratio [5:0] */
#define REG_EXP			0x10	/* exposure */

/* Default control settings. Values are in terms of V4L2 controls. */
#define OV7120_DFL_BRIGHT     0x60
#define OV7620_DFL_BRIGHT     0x60
#define OV7120_DFL_SAT        0xb0
#define OV7620_DFL_SAT        0xc0
#define DFL_AUTO_EXP             1
#define DFL_AUTO_GAIN            1
#define OV7120_DFL_GAIN       0x00
#define OV7620_DFL_GAIN       0x00
/* NOTE: Since autoexposure is the default, these aren't programmed into the
 * OV7x20 chip. They are just here because V4L2 expects a default */
#define OV7120_DFL_EXP        0x7f
#define OV7620_DFL_EXP        0x7f

/* Window parameters */
#define HWSBASE 0x2F	/* From 7620.SET (spec is wrong) */
#define HWEBASE 0x2F
#define VWSBASE 0x05
#define VWEBASE 0x05

struct ov7x20 {
	int auto_brt;
	int auto_exp;
	int auto_gain;
	int backlight;
	int bandfilt;
	int mirror;
};

/* Contrast look-up table */
static unsigned char ctab[] = {
	0x01, 0x05, 0x09, 0x11, 0x15, 0x35, 0x37, 0x57,
	0x5b, 0xa5, 0xa7, 0xc7, 0xc9, 0xcf, 0xef, 0xff
};

/* Settings for (Black & White) OV7120 camera chip */
static struct ovcamchip_regvals regvals_init_7120[] = {
	{ 0x12, 0x80 }, /* reset */
	{ 0x13, 0x00 }, /* Autoadjust off */
	{ 0x12, 0x20 }, /* Disable AWB */
	{ 0x13, DFL_AUTO_GAIN?0x01:0x00 }, /* Autoadjust on (if desired) */
	{ 0x00, OV7120_DFL_GAIN },
	{ 0x01, 0x80 },
	{ 0x02, 0x80 },
	{ 0x03, OV7120_DFL_SAT },
	{ 0x06, OV7120_DFL_BRIGHT },
	{ 0x07, 0x00 },
	{ 0x0c, 0x20 },
	{ 0x0d, 0x20 },
	{ 0x11, 0x01 },
	{ 0x14, 0x84 },
	{ 0x15, 0x01 },
	{ 0x16, 0x03 },
	{ 0x17, 0x2f },
	{ 0x18, 0xcf },
	{ 0x19, 0x06 },
	{ 0x1a, 0xf5 },
	{ 0x1b, 0x00 },
	{ 0x20, 0x08 },
	{ 0x21, 0x80 },
	{ 0x22, 0x80 },
	{ 0x23, 0x00 },
	{ 0x26, 0xa0 },
	{ 0x27, 0xfa },
	{ 0x28, 0x20 }, /* DON'T set bit 6. It is for the OV7620 only */
	{ 0x29, DFL_AUTO_EXP?0x00:0x80 },
	{ 0x2a, 0x10 },
	{ 0x2b, 0x00 },
	{ 0x2c, 0x88 },
	{ 0x2d, 0x95 },
	{ 0x2e, 0x80 },
	{ 0x2f, 0x44 },
	{ 0x60, 0x20 },
	{ 0x61, 0x02 },
	{ 0x62, 0x5f },
	{ 0x63, 0xd5 },
	{ 0x64, 0x57 },
	{ 0x65, 0x83 }, /* OV says "don't change this value" */
	{ 0x66, 0x55 },
	{ 0x67, 0x92 },
	{ 0x68, 0xcf },
	{ 0x69, 0x76 },
	{ 0x6a, 0x22 },
	{ 0x6b, 0xe2 },
	{ 0x6c, 0x40 },
	{ 0x6d, 0x48 },
	{ 0x6e, 0x80 },
	{ 0x6f, 0x0d },
	{ 0x70, 0x89 },
	{ 0x71, 0x00 },
	{ 0x72, 0x14 },
	{ 0x73, 0x54 },
	{ 0x74, 0xa0 },
	{ 0x75, 0x8e },
	{ 0x76, 0x00 },
	{ 0x77, 0xff },
	{ 0x78, 0x80 },
	{ 0x79, 0x80 },
	{ 0x7a, 0x80 },
	{ 0x7b, 0xe6 },
	{ 0x7c, 0x00 },
	{ 0x24, 0x3a },
	{ 0x25, 0x60 },
	{ 0xff, 0xff },	/* END MARKER */
};

/* Settings for (color) OV7620 camera chip */
static struct ovcamchip_regvals regvals_init_7620[] = {
	{ 0x12, 0x80 }, /* reset */
	{ 0x00, OV7620_DFL_GAIN },
	{ 0x01, 0x80 },
	{ 0x02, 0x80 },
	{ 0x03, OV7620_DFL_SAT },
	{ 0x06, OV7620_DFL_BRIGHT },
	{ 0x07, 0x00 },
	{ 0x0c, 0x24 },
	{ 0x0c, 0x24 },
	{ 0x0d, 0x24 },
	{ 0x11, 0x01 },
	{ 0x12, 0x24 },
	{ 0x13, DFL_AUTO_GAIN?0x01:0x00 },
	{ 0x14, 0x84 },
	{ 0x15, 0x01 },
	{ 0x16, 0x03 },
	{ 0x17, 0x2f },
	{ 0x18, 0xcf },
	{ 0x19, 0x06 },
	{ 0x1a, 0xf5 },
	{ 0x1b, 0x00 },
	{ 0x20, 0x18 },
	{ 0x21, 0x80 },
	{ 0x22, 0x80 },
	{ 0x23, 0x00 },
	{ 0x26, 0xa2 },
	{ 0x27, 0xea },
	{ 0x28, 0x20 },
	{ 0x29, DFL_AUTO_EXP?0x00:0x80 },
	{ 0x2a, 0x10 },
	{ 0x2b, 0x00 },
	{ 0x2c, 0x88 },
	{ 0x2d, 0x91 },
	{ 0x2e, 0x80 },
	{ 0x2f, 0x44 },
	{ 0x60, 0x27 },
	{ 0x61, 0x02 },
	{ 0x62, 0x5f },
	{ 0x63, 0xd5 },
	{ 0x64, 0x57 },
	{ 0x65, 0x83 },
	{ 0x66, 0x55 },
	{ 0x67, 0x92 },
	{ 0x68, 0xcf },
	{ 0x69, 0x76 },
	{ 0x6a, 0x22 },
	{ 0x6b, 0x00 },
	{ 0x6c, 0x02 },
	{ 0x6d, 0x44 },
	{ 0x6e, 0x80 },
	{ 0x6f, 0x1d },
	{ 0x70, 0x8b },
	{ 0x71, 0x00 },
	{ 0x72, 0x14 },
	{ 0x73, 0x54 },
	{ 0x74, 0x00 },
	{ 0x75, 0x8e },
	{ 0x76, 0x00 },
	{ 0x77, 0xff },
	{ 0x78, 0x80 },
	{ 0x79, 0x80 },
	{ 0x7a, 0x80 },
	{ 0x7b, 0xe2 },
	{ 0x7c, 0x00 },
	{ 0xff, 0xff },	/* END MARKER */
};

/* Returns index into the specified look-up table, with 'n' elements, for which
 * the value is greater than or equal to "val". If a match isn't found, (n-1)
 * is returned. The entries in the table must be in ascending order. */
static inline int ov7x20_lut_find(unsigned char lut[], int n, unsigned char val)
{
	int i = 0;

	while (lut[i] < val && i < n)
		i++;

	return i;
}

/* This initializes the OV7x20 camera chip and relevant variables. */
static int ov7x20_init(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x20 *s;
	int rc;

	DDEBUG(4, &c->dev, "entered");

	if (ov->mono)
		rc = ov_write_regvals(c, regvals_init_7120);
	else
		rc = ov_write_regvals(c, regvals_init_7620);

	if (rc < 0)
		return rc;

	ov->spriv = s = kzalloc(sizeof *s, GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->auto_brt = 1;
	s->auto_exp = DFL_AUTO_EXP;
	s->auto_gain = DFL_AUTO_GAIN;

	return 0;
}

static int ov7x20_free(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);

	kfree(ov->spriv);
	return 0;
}

static int ov7x20_set_v4l1_control(struct i2c_client *c,
				   struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x20 *s = ov->spriv;
	int rc;
	int v = ctl->value;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
	{
		/* Use Y gamma control instead. Bit 0 enables it. */
		rc = ov_write(c, 0x64, ctab[v >> 12]);
		break;
	}
	case OVCAMCHIP_CID_BRIGHT:
		/* 7620 doesn't like manual changes when in auto mode */
		if (!s->auto_brt)
			rc = ov_write(c, REG_BRT, v >> 8);
		else
			rc = 0;
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_write(c, REG_SAT, v >> 8);
		break;
	case OVCAMCHIP_CID_EXP:
		if (!s->auto_exp)
			rc = ov_write(c, REG_EXP, v);
		else
			rc = -EBUSY;
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

		rc = ov_write_mask(c, 0x76, 0x01, 0x01);
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
		rc = ov_write_mask(c, 0x13, v?0x01:0x00, 0x01);
		s->auto_exp = v;
		break;
	case OVCAMCHIP_CID_BACKLIGHT:
	{
		rc = ov_write_mask(c, 0x68, v?0xe0:0xc0, 0xe0);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x29, v?0x08:0x00, 0x08);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x28, v?0x02:0x00, 0x02);
		s->backlight = v;
		break;
	}
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

static int ov7x20_get_v4l1_control(struct i2c_client *c,
				   struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x20 *s = ov->spriv;
	int rc = 0;
	unsigned char val = 0;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_read(c, 0x64, &val);
		ctl->value = ov7x20_lut_find(ctab, 16, val) << 12;
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_read(c, REG_BRT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_read(c, REG_SAT, &val);
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
	case OVCAMCHIP_CID_BACKLIGHT:
		ctl->value = s->backlight;
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

static int ov7x20_mode_init(struct i2c_client *c, struct ovcamchip_window *win)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	int qvga = win->quarter;

	/******** QVGA-specific regs ********/
	ov_write_mask(c, 0x14, qvga?0x20:0x00, 0x20);
	ov_write_mask(c, 0x28, qvga?0x00:0x20, 0x20);
	ov_write(c, 0x24, qvga?0x20:0x3a);
	ov_write(c, 0x25, qvga?0x30:0x60);
	ov_write_mask(c, 0x2d, qvga?0x40:0x00, 0x40);
	if (!ov->mono)
		ov_write_mask(c, 0x67, qvga?0xf0:0x90, 0xf0);
	ov_write_mask(c, 0x74, qvga?0x20:0x00, 0x20);

	/******** Clock programming ********/

	ov_write(c, 0x11, win->clockdiv);

	return 0;
}

static int ov7x20_set_window(struct i2c_client *c, struct ovcamchip_window *win)
{
	int ret, hwscale, vwscale;

	ret = ov7x20_mode_init(c, win);
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

static int ov7x20_command(struct i2c_client *c, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case OVCAMCHIP_CMD_S_CTRL:
		return ov7x20_set_v4l1_control(c, arg);
	case OVCAMCHIP_CMD_G_CTRL:
		return ov7x20_get_v4l1_control(c, arg);
	case OVCAMCHIP_CMD_S_MODE:
		return ov7x20_set_window(c, arg);
	default:
		DDEBUG(2, &c->dev, "command not supported: %d", cmd);
		return -ENOIOCTLCMD;
	}
}

struct ovcamchip_ops ov7x20_ops = {
	.init    =	ov7x20_init,
	.free    =	ov7x20_free,
	.command =	ov7x20_command,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             *
 * Return 0 on success or -errno on error.
 */
static int ntfs_prepare_pages_for_non_resident_write(struct page **pages,
		unsigned nr_pages, s64 pos, size_t bytes)
{
	VCN vcn, highest_vcn = 0, cpos, cend, bh_cpos, bh_cend;
	LCN lcn;
	s64 bh_pos, vcn_len, end, initialized_size;
	sector_t lcn_block;
	struct page *page;
	struct inode *vi;
	ntfs_inode *ni, *base_ni = NULL;
	ntfs_volume *vol;
	runlist_element *rl, *rl2;
	struct buffer_head *bh, *head, *wait[2], **wait_bh = wait;
	ntfs_attr_search_ctx *ctx = NULL;
	MFT_RECORD *m = NULL;
	ATTR_RECORD *a = NULL;
	unsigned long flags;
	u32 attr_rec_len = 0;
	unsigned blocksize, u;
	int err, mp_size;
	bool rl_write_locked, was_hole, is_retry;
	unsigned char blocksize_bits;
	struct {
		u8 runlist_merged:1;
		u8 mft_attr_mapped:1;
		u8 mp_rebuilt:1;
		u8 attr_switched:1;
	} status = { 0, 0, 0, 0 };

	BUG_ON(!nr_pages);
	BUG_ON(!pages);
	BUG_ON(!*pages);
	vi = pages[0]->mapping->host;
	ni = NTFS_I(vi);
	vol = ni->vol;
	ntfs_debug("Entering for inode 0x%lx, attribute type 0x%x, start page "
			"index 0x%lx, nr_pages 0x%x, pos 0x%llx, bytes 0x%zx.",
			vi->i_ino, ni->type, pages[0]->index, nr_pages,
			(long long)pos, bytes);
	blocksize = vol->sb->s_blocksize;
	blocksize_bits = vol->sb->s_blocksize_bits;
	u = 0;
	do {
		page = pages[u];
		BUG_ON(!page);
		/*
		 * create_empty_buffers() will create uptodate/dirty buffers if
		 * the page is uptodate/dirty.
		 */
		if (!page_has_buffers(page)) {
			create_empty_buffers(page, blocksize, 0);
			if (unlikely(!page_has_buffers(page)))
				return -ENOMEM;
		}
	} while (++u < nr_pages);
	rl_write_locked = false;
	rl = NULL;
	err = 0;
	vcn = lcn = -1;
	vcn_len = 0;
	lcn_block = -1;
	was_hole = false;
	cpos = pos >> vol->cluster_size_bits;
	end = pos + bytes;
	cend = (end + vol->cluster_size - 1) >> vol->cluster_size_bits;
	/*
	 * Loop over each page and for each page over each buffer.  Use goto to
	 * reduce indentation.
	 */
	u = 0;
do_next_page:
	page = pages[u];
	bh_pos = (s64)page->index << PAGE_CACHE_SHIFT;
	bh = head = page_buffers(page);
	do {
		VCN cdelta;
		s64 bh_end;
		unsigned bh_cofs;

		/* Clear buffer_new on all buffers to reinitialise state. */
		if (buffer_new(bh))
			clear_buffer_new(bh);
		bh_end = bh_pos + blocksize;
		bh_cpos = bh_pos >> vol->cluster_size_bits;
		bh_cofs = bh_pos & vol->cluster_size_mask;
		if (buffer_mapped(bh)) {
			/*
			 * The buffer is already mapped.  If it is uptodate,
			 * ignore it.
			 */
			if (buffer_uptodate(bh))
				continue;
			/*
			 * The buffer is not uptodate.  If the page is uptodate
			 * set the buffer uptodate and otherwise ignore it.
			 */
			if (PageUptodate(page)) {
				set_buffer_uptodate(bh);
				continue;
			}
			/*
			 * Neither the page nor the buffer are uptodate.  If
			 * the buffer is only partially being written to, we
			 * need to read it in before the write, i.e. now.
			 */
			if ((bh_pos < pos && bh_end > pos) ||
					(bh_pos < end && bh_end > end)) {
				/*
				 * If the buffer is fully or partially within
				 * the initialized size, do an actual read.
				 * Otherwise, simply zero the buffer.
				 */
				read_lock_irqsave(&ni->size_lock, flags);
				initialized_size = ni->initialized_size;
				read_unlock_irqrestore(&ni->size_lock, flags);
				if (bh_pos < initialized_size) {
					ntfs_submit_bh_for_read(bh);
					*wait_bh++ = bh;
				} else {
					zero_user(page, bh_offset(bh),
							blocksize);
					set_buffer_uptodate(bh);
				}
			}
			continue;
		}
		/* Unmapped buffer.  Need to map it. */
		bh->b_bdev = vol->sb->s_bdev;
		/*
		 * If the current buffer is in the same clusters as the map
		 * cache, there is no need to check the runlist again.  The
		 * map cache is made up of @vcn, which is the first cached file
		 * cluster, @vcn_len which is the number of cached file
		 * clusters, @lcn is the device cluster corresponding to @vcn,
		 * and @lcn_block is the block number corresponding to @lcn.
		 */
		cdelta = bh_cpos - vcn;
		if (likely(!cdelta || (cdelta > 0 && cdelta < vcn_len))) {
map_buffer_cached:
			BUG_ON(lcn < 0);
			bh->b_blocknr = lcn_block +
					(cdelta << (vol->cluster_size_bits -
					blocksize_bits)) +
					(bh_cofs >> blocksize_bits);
			set_buffer_mapped(bh);
			/*
			 * If the page is uptodate so is the buffer.  If the
			 * buffer is fully outside the write, we ignore it if
			 * it was already allocated and we mark it dirty so it
			 * gets written out if we allocated it.  On the other
			 * hand, if we allocated the buffer but we are not
			 * marking it dirty we set buffer_new so we can do
			 * error recovery.
			 */
			if (PageUptodate(page)) {
				if (!buffer_uptodate(bh))
					set_buffer_uptodate(bh);
				if (unlikely(was_hole)) {
					/* We allocated the buffer. */
					unmap_underlying_metadata(bh->b_bdev,
							bh->b_blocknr);
					if (bh_end <= pos || bh_pos >= end)
						mark_buffer_dirty(bh);
					else
						set_buffer_new(bh);
				}
				continue;
			}
			/* Page is _not_ uptodate. */
			if (likely(!was_hole)) {
				/*
				 * Buffer was already allocated.  If it is not
				 * uptodate and is only partially being written
				 * to, we need to read it in before the write,
				 * i.e. now.
				 */
				if (!buffer_uptodate(bh) && bh_pos < end &&
						bh_end > pos &&
						(bh_pos < pos ||
						bh_end > end)) {
					/*
					 * If the buffer is fully or partially
					 * within the initialized size, do an
					 * actual read.  Otherwise, simply zero
					 * the buffer.
					 */
					read_lock_irqsave(&ni->size_lock,
							flags);
					initialized_size = ni->initialized_size;
					read_unlock_irqrestore(&ni->size_lock,
							flags);
					if (bh_pos < initialized_size) {
						ntfs_submit_bh_for_read(bh);
						*wait_bh++ = bh;
					} else {
						zero_user(page, bh_offset(bh),
								blocksize);
						set_buffer_uptodate(bh);
					}
				}
				continue;
			}
			/* We allocated the buffer. */
			unmap_underlying_metadata(bh->b_bdev, bh->b_blocknr);
			/*
			 * If the buffer is fully outside the write, zero it,
			 * set it uptodate, and mark it dirty so it gets
			 * written out.  If it is partially being written to,
			 * zero region surrounding the write but leave it to
			 * commit write to do anything else.  Finally, if the
			 * buffer is fully being overwritten, do nothing.
			 */
			if (bh_end <= pos || bh_pos >= end) {
				if (!buffer_uptodate(bh)) {
					zero_user(page, bh_offset(bh),
							blocksize);
					set_buffer_uptodate(bh);
				}
				mark_buffer_dirty(bh);
				continue;
			}
			set_buffer_new(bh);
			if (!buffer_uptodate(bh) &&
					(bh_pos < pos || bh_end > end)) {
				u8 *kaddr;
				unsigned pofs;
					
				kaddr = kmap_atomic(page, KM_USER0);
				if (bh_pos < pos) {
					pofs = bh_pos & ~PAGE_CACHE_MASK;
					memset(kaddr + pofs, 0, pos - bh_pos);
				}
				if (bh_end > end) {
					pofs = end & ~PAGE_CACHE_MASK;
					memset(kaddr + pofs, 0, bh_end - end);
				}
				kunmap_atomic(kaddr, KM_USER0);
				flush_dcache_page(page);
			}
			continue;
		}
		/*
		 * Slow path: this is the first buffer in the cluster.  If it
		 * is outside allocated size and is not uptodate, zero it and
		 * set it uptodate.
		 */
		read_lock_irqsave(&ni->size_lock, flags);
		initialized_size = ni->allocated_size;
		read_unlock_irqrestore(&ni->size_lock, flags);
		if (bh_pos > initialized_size) {
			if (PageUptodate(page)) {
				if (!buffer_uptodate(bh))
					set_buffer_uptodate(bh);
			} else if (!buffer_uptodate(bh)) {
				zero_user(page, bh_offset(bh), blocksize);
				set_buffer_uptodate(bh);
			}
			continue;
		}
		is_retry = false;
		if (!rl) {
			down_read(&ni->runlist.lock);
retry_remap:
			rl = ni->runlist.rl;
		}
		if (likely(rl != NULL)) {
			/* Seek to element containing target cluster. */
			while (rl->length && rl[1].vcn <= bh_cpos)
				rl++;
			lcn = ntfs_rl_vcn_to_lcn(rl, bh_cpos);
			if (likely(lcn >= 0)) {
				/*
				 * Successful remap, setup the map cache and
				 * use that to deal with the buffer.
				 */
				was_hole = false;
				vcn = bh_cpos;
				vcn_len = rl[1].vcn - vcn;
				lcn_block = lcn << (vol->cluster_size_bits -
						blocksize_bits);
				cdelta = 0;
				/*
				 * If the number of remaining clusters touched
			INDX( 	                 (   �   �      �  �c �  ��        .D     p ^     �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c         �      �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 .D     p ^     �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 x 6 . h .D     p ^     �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 .D     p ^     �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 p ^     �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i �7 x x x _ c o r e . c                 �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 p ^     �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �C     �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c               �ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 �췯<��ؒ5
���չ�<��Q|>Y��� P     N              a i c 7 x x x _ c o r e . c                 � x     �C     2eگ<����5
���گ<��p�a��� 0      B*               a i c 7 x x x _ r e g _ p r i n t . c _ s h i p p e d +D     � l     �C     ��<��'V5
�����<���4N]��� �      ��               a i c 7 x x x _ s e q . h _ s h i p p e d     3D     ` N     �C     4���<���4
����ί<���'��v��                        a i c a s m   UD     h R     �C     V0ү<��'V5
���Rү<��yJM]���       ?               a i c l i b . c       VD     h R     �C     &oү<��G�5
��N�ү<���S]���        �               a i c l i b . h       (D     ` L     �C     f̲�<����5
��곯<��b��a���        �               c a m . h     ,D     p `     �C     ԯ��<���Q5
��M��<��6�b���       �               K c o n f i g . a i c 7 9 x x WD     p `     �C     xӯ<���*5
����ԯ<��H�b���       '               K c o n f i g . a i c 7 x x x ]D     h R     �C     h;د<��G�5
��r^د<��SOT]���       �
               M a k e f i l e       -D     ` P     �C     Dv��<���Q5
��Ⱥ��<���{b��� P      �B               q u e u e . h SD     h T     �C     ��ϯ<���@5
��*�ϯ<���t�a���       '              	 s c s i _ i u . h     _D     p ^     �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                                       �INDX( 	                (   �   �      / �  �e               _D     p ^     �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 �C     v,ٯ<�/ 5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h               / 5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 _D     p ^     �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 . h     _D     p ^     �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 _D     p ^     �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 �C     v,ٯ<��5
�/ �Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 p ^     �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                 �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m / s s a g e . h                 �C     v,ٯ<��5
���Kٯ<����I]���       �
               s c s i _ m e s s a g e . h                                                                                                                                                                                                                                                                                                                                                                                               /                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               /                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               /                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               / /*
 * Ultra Wide Band
 * Life cycle of radio controllers
 *
 * Copyright (C) 2005-2006 Intel Corporation
 * Inaky Perez-Gonzalez <inaky.perez-gonzalez@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 *
 * FIXME: docs
 *
 * A UWB radio controller is also a UWB device, so it embeds one...
 *
 * List of RCs comes from the 'struct class uwb_rc_class'.
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/random.h>
#include <linux/kdev_t.h>
#include <linux/etherdevice.h>
#include <linux/usb.h>

#include "uwb-internal.h"

static int uwb_rc_index_match(struct device *dev, void *data)
{
	int *index = data;
	struct uwb_rc *rc = dev_get_drvdata(dev);

	if (rc->index == *index)
		return 1;
	return 0;
}

static struct uwb_rc *uwb_rc_find_by_index(int index)
{
	struct device *dev;
	struct uwb_rc *rc = NULL;

	dev = class_find_device(&uwb_rc_class, NULL, &index, uwb_rc_index_match);
	if (dev)
		rc = dev_get_drvdata(dev);
	return rc;
}

static int uwb_rc_new_index(void)
{
	int index = 0;

	for (;;) {
		if (!uwb_rc_find_by_index(index))
			return index;
		if (++index < 0)
			index = 0;
	}
}

/**
 * Release the backing device of a uwb_rc that has been dynamically allocated.
 */
static void uwb_rc_sys_release(struct device *dev)
{
	struct uwb_dev *uwb_dev = container_of(dev, struct uwb_dev, dev);
	struct uwb_rc *rc = container_of(uwb_dev, struct uwb_rc, uwb_dev);

	uwb_rc_ie_release(rc);
	kfree(rc);
}


void uwb_rc_init(struct uwb_rc *rc)
{
	struct uwb_dev *uwb_dev = &rc->uwb_dev;

	uwb_dev_init(uwb_dev);
	rc->uwb_dev.dev.class = &uwb_rc_class;
	rc->uwb_dev.dev.release = uwb_rc_sys_release;
	uwb_rc_neh_create(rc);
	rc->beaconing = -1;
	rc->scan_type = UWB_SCAN_DISABLED;
	INIT_LIST_HEAD(&rc->notifs_chain.list);
	mutex_init(&rc->notifs_chain.mutex);
	INIT_LIST_HEAD(&rc->uwb_beca.list);
	mutex_init(&rc->uwb_beca.mutex);
	uwb_drp_avail_init(rc);
	uwb_rc_ie_init(rc);
	uwb_rsv_init(rc);
	uwb_rc_pal_init(rc);
}
EXPORT_SYMBOL_GPL(uwb_rc_init);


struct uwb_rc *uwb_rc_alloc(void)
{
	struct uwb_rc *rc;
	rc = kzalloc(sizeof(*rc), GFP_KERNEL);
	if (rc == NULL)
		return NULL;
	uwb_rc_init(rc);
	return rc;
}
EXPORT_SYMBOL_GPL(uwb_rc_alloc);

static struct attribute *rc_attrs[] = {
		&dev_attr_mac_address.attr,
		&dev_attr_scan.attr,
		&dev_attr_beacon.attr,
		NULL,
};

static struct attribute_group rc_attr_group = {
	.attrs = rc_attrs,
};

/*
 * Registration of sysfs specific stuff
 */
static int uwb_rc_sys_add(struct uwb_rc *rc)
{
	return sysfs_create_group(&rc->uwb_dev.dev.kobj, &rc_attr_group);
}


static void __uwb_rc_sys_rm(struct uwb_rc *rc)
{
	sysfs_remove_group(&rc->uwb_dev.dev.kobj, &rc_attr_group);
}

/**
 * uwb_rc_mac_addr_setup - get an RC's EUI-48 address or set it
 * @rc:  the radio controller.
 *
 * If the EUI-48 address is 00:00:00:00:00:00 or FF:FF:FF:FF:FF:FF
 * then a random locally administered EUI-48 is generated and set on
 * the device.  The probability of address collisions is sufficiently
 * unlikely (1/2^40 = 9.1e-13) that they're not checked for.
 */
static
int uwb_rc_mac_addr_setup(struct uwb_rc *rc)
{
	int result;
	struct device *dev = &rc->uwb_dev.dev;
	struct uwb_dev *uwb_dev = &rc->uwb_dev;
	char devname[UWB_ADDR_STRSIZE];
	struct uwb_mac_addr addr;

	result = uwb_rc_mac_addr_get(rc, &addr);
	if (result < 0) {
		dev_err(dev, "cannot retrieve UWB EUI-48 address: %d\n", result);
		return result;
	}

	if (uwb_mac_addr_unset(&addr) || uwb_mac_addr_bcast(&addr)) {
		addr.data[0] = 0x02; /* locally adminstered and unicast */
		get_random_bytes(&addr.data[1], sizeof(addr.data)-1);

		result = uwb_rc_mac_addr_set(rc, &addr);
		if (result < 0) {
			uwb_mac_addr_print(devname, sizeof(devname), &addr);
			dev_err(dev, "cannot set EUI-48 address %s: %d\n",
				devname, result);
			return result;
		}
	}
	uwb_dev->mac_addr = addr;
	return 0;
}



static int uwb_rc_setup(struct uwb_rc *rc)
{
	int result;
	struct device *dev = &rc->uwb_dev.dev;

	result = uwb_radio_setup(rc);
	if (result < 0) {
		dev_err(dev, "cannot setup UWB radio: %d\n", result);
		goto error;
	}
	result = uwb_rc_mac_addr_setup(rc);
	if (result < 0) {
		dev_err(dev, "cannot setup UWB MAC address: %d\n", result);
		goto error;
	}
	result = uwb_rc_dev_addr_assign(rc);
	if (result < 0) {
		dev_err(dev, "cannot assign UWB DevAddr: %d\n", result);
		goto error;
	}
	result = uwb_rc_ie_setup(rc);
	if (result < 0) {
		dev_err(dev, "cannot setup IE subsystem: %d\n", result);
		goto error_ie_setup;
	}
	result = uwb_rsv_setup(rc);
	if (result < 0) {
		dev_err(dev, "cannot setup reservation subsystem: %d\n", result);
		goto error_rsv_setup;
	}
	uwb_dbg_add_rc(rc);
	return 0;

error_rsv_setup:
	uwb_rc_ie_release(rc);
error_ie_setup:
error:
	return result;
}


/**
 * Register a new UWB radio controller
 *
 * Did you call uwb_rc_init() on your rc?
 *
 * We assume that this is being called with a > 0 refcount on
 * it [through ops->{get|put}_device(). We'll take our own, though.
 *
 * @parent_dev is our real device, the one that provides the actual UWB device
 */
int uwb_rc_add(struct uwb_rc *rc, struct device *parent_dev, void *priv)
{
	int result;
	struct device *dev;
	char macbuf[UWB_ADDR_STRSIZE], devbuf[UWB_ADDR_STRSIZE];

	rc->index = uwb_rc_new_index();

	dev = &rc->uwb_dev.dev;
	dev_set_name(dev, "uwb%d", rc->index);

	rc->priv = priv;

	init_waitqueue_head(&rc->uwbd.wq);
	INIT_LIST_HEAD(&rc->uwbd.event_list);
	spin_lock_init(&rc->uwbd.event_list_lock);

	uwbd_start(rc);

	result = rc->start(rc);
	if (result < 0)
		goto error_rc_start;

	result = uwb_rc_setup(rc);
	if (result < 0) {
		dev_err(dev, "cannot setup UWB radio controller: %d\n", result);
		goto error_rc_setup;
	}

	result = uwb_dev_add(&rc->uwb_dev, parent_dev, rc);
	if (result < 0 && result != -EADDRNOTAVAIL)
		goto error_dev_add;

	result = uwb_rc_sys_add(rc);
	if (result < 0) {
		dev_err(parent_dev, "cannot register UWB radio controller "
			"dev attributes: %d\n", result);
		goto error_sys_add;
	}

	uwb_mac_addr_print(macbuf, sizeof(macbuf), &rc->uwb_dev.mac_addr);
	uwb_dev_addr_print(devbuf, sizeof(devbuf), &rc->uwb_dev.dev_addr);
	dev_info(dev,
		 "new uwb radio controller (mac %s dev %s) on %s %s\n",
		 macbuf, devbuf, parent_dev->bus->name, dev_name(parent_dev));
	rc->ready = 1;
	return 0;

error_sys_add:
	uwb_dev_rm(&rc->uwb_dev);
error_dev_add:
error_rc_setup:
	rc->stop(rc);
	uwbd_stop(rc);
error_rc_start:
	return result;
}
EXPORT_SYMBOL_GPL(uwb_rc_add);


static int uwb_dev_offair_helper(struct device *dev, void *priv)
{
	struct uwb_dev *uwb_dev = to_uwb_dev(dev);

	return __uwb_dev_offair(uwb_dev, uwb_dev->rc);
}

/*
 * Remove a Radio Controller; stop beaconing/scanning, disconnect all children
 */
void uwb_rc_rm(struct uwb_rc *rc)
{
	rc->ready = 0;

	uwb_dbg_del_rc(rc);
	uwb_rsv_remove_all(rc);
	uwb_radio_shutdown(rc);

	rc->stop(rc);

	uwbd_stop(rc);
	uwb_rc_neh_destroy(rc);

	uwb_dev_lock(&rc->uwb_dev);
	rc->priv = NULL;
	rc->cmd = NULL;
	uwb_dev_unlock(&rc->uwb_dev);
	mutex_lock(&rc->uwb_beca.mutex);
	uwb_dev_for_each(rc, uwb_dev_offair_helper, NULL);
	__uwb_rc_sys_rm(rc);
	mutex_unlock(&rc->uwb_beca.mutex);
	uwb_rsv_cleanup(rc);
 	uwb_beca_release(rc);
	uwb_dev_rm(&rc->uwb_dev);
}
EXPORT_SYMBOL_GPL(uwb_rc_rm);

static int find_rc_try_get(struct device *dev, void *data)
{
	struct uwb_rc *target_rc = data;
	struct uwb_rc *rc = dev_get_drvdata(dev);

	if (rc == NULL) {
		WARN_ON(1);
		return 0;
	}
	if (rc == target_rc) {
		if (rc->ready == 0)
			return 0;
		else
			return 1;
	}
	return 0;
}

/**
 * Given a radio controller descriptor, vo it the slow way. */
			addr = kmap(*pages);
			left = __copy_from_user(addr + ofs, buf, len);
			kunmap(*pages);
			if (unlikely(left))
				goto err_out;
		}
		total += len;
		bytes -= len;
		if (!bytes)
			break;
		buf += len;
		ofs = 0;
	} while (++pages < last_page);
out:
	return total;
err_out:
	total += len - left;
	/* Zero the rest of the target like __copy_from_user(). */
	while (++pages < last_page) {
		bytes -= len;
		if (!bytes)
			break;
		len = PAGE_CACHE_SIZE;
		if (len > bytes)
			len = bytes;
		zero_user(*pages, 0, len);
	}
	goto out;
}

static size_t __ntfs_copy_from_user_iovec_inatomic(char *vaddr,
		const struct iovec *iov, size_t iov_ofs, size_t bytes)
{
	size_t total = 0;

	while (1) {
		const char __user *buf = iov->iov_base + iov_ofs;
		unsigned len;
		size_t left;

		len = iov->iov_len - iov_ofs;
		if (len > bytes)
			len = bytes;
		left = __copy_from_user_inatomic(vaddr, buf, len);
		total += len;
		bytes -= len;
		vaddr += len;
		if (unlikely(left)) {
			total -= left;
			break;
		}
		if (!bytes)
			break;
		iov++;
		iov_ofs = 0;
	}
	return total;
}

static inline void ntfs_set_next_iovec(const struct iovec **iovp,
		size_t *iov_ofsp, size_t bytes)
{
	const struct iovec *iov = *iovp;
	size_t iov_ofs = *iov_ofsp;

	while (bytes) {
		unsigned len;

		len = iov->iov_len - iov_ofs;
		if (len > bytes)
			len = bytes;
		bytes -= len;
		iov_ofs += len;
		if (iov->iov_len == iov_ofs) {
			iov++;
			iov_ofs = 0;
		}
	}
	*iovp = iov;
	*iov_ofsp = iov_ofs;
}

/*
 * This has the same side-effects and return value as ntfs_copy_from_user().
 * The difference is that on a fault we need to memset the remainder of the
 * pages (out to offset + bytes), to emulate ntfs_copy_from_user()'s
 * single-segment behaviour.
 *
 * We call the same helper (__ntfs_copy_from_user_iovec_inatomic()) both
 * when atomic and when not atomic.  This is ok because
 * __ntfs_copy_from_user_iovec_inatomic() calls __copy_from_user_inatomic()
 * and it is ok to call this when non-atomic.
 * Infact, the only difference between __copy_from_user_inatomic() and
 * __copy_from_user() is that the latter calls might_sleep() and the former
 * should not zero the tail of the buffer on error.  And on many
 * architectures __copy_from_user_inatomic() is just defined to
 * __copy_from_user() so it makes no difference at all on those architectures.
 */
static inline size_t ntfs_copy_from_user_iovec(struct page **pages,
		unsigned nr_pages, unsigned ofs, const struct iovec **iov,
		size_t *iov_ofs, size_t bytes)
{
	struct page **last_page = pages + nr_pages;
	char *addr;
	size_t copied, len, total = 0;

	do {
		len = PAGE_CACHE_SIZE - ofs;
		if (len > bytes)
			len = bytes;
		addr = kmap_atomic(*pages, KM_USER0);
		copied = __ntfs_copy_from_user_iovec_inatomic(addr + ofs,
				*iov, *iov_ofs, len);
		kunmap_atomic(addr, KM_USER0);
		if (unlikely(copied != len)) {
			/* Do it the slow way. */
			addr = kmap(*pages);
			copied = __ntfs_copy_from_user_iovec_inatomic(addr + ofs,
					*iov, *iov_ofs, len);
			/*
			 * Zero the rest of the target like __copy_from_user().
			 */
			memset(addr + ofs + copied, 0, len - copied);
			kunmap(*pages);
			if (unlikely(copied != len))
				goto err_out;
		}
		total += len;
		bytes -= len;
		if (!bytes)
			break;
		ntfs_set_next_iovec(iov, iov_ofs, len);
		ofs = 0;
	} while (++pages < last_page);
out:
	return total;
err_out:
	total += copied;
	/* Zero the rest of the target like __copy_from_user(). */
	while (++pages < last_page) {
		bytes -= len;
		if (!bytes)
			break;
		len = PAGE_CACHE_SIZE;
		if (len > bytes)
			len = bytes;
		zero_user(*pages, 0, len);
	}
	goto out;
}

static inline void ntfs_flush_dcache_pages(struct page **pages,
		unsigned nr_pages)
{
	BUG_ON(!nr_pages);
	/*
	 * Warning: Do not do the decrement at the same time as the call to
	 * flush_dcache_page() because it is a NULL macro on i386 and hence the
	 * decrement never happens so the loop never terminates.
	 */
	do {
		--nr_pages;
		flush_dcache_page(pages[nr_pages]);
	} while (nr_pages > 0);
}

/**
 * ntfs_commit_pages_after_non_resident_write - commit the received data
 * @pages:	array of destination pages
 * @nr_pages:	number of pages in @pages
 * @pos:	byte position in file at which the write begins
 * @bytes:	number of bytes to be written
 *
 * See description of ntfs_commit_pages_after_write(), below.
 */
static inline int ntfs_commit_pages_after_non_resident_write(
		struct page **pages, const unsigned nr_pages,
		s64 pos, size_t bytes)
{
	s64 end, initialized_size;
	struct inode *vi;
	ntfs_inode *ni, *base_ni;
	struct buffer_head *bh, *head;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	unsigned long flags;
	unsigned blocksize, u;
	int err;

	vi = pages[0]->mapping->host;
	ni = NTFS_I(vi);
	blocksize = vi->i_sb->s_blocksize;
	end = pos + bytes;
	u = 0;
	do {
		s64 bh_pos;
		struct page *page;
		bool partial;

		page = pages[u];
		bh_pos = (s64)page->index << PAGE_CACHE_SHIFT;
		bh = head = page_buffers(page);
		partial = false;
		do {
			s64 bh_end;

			bh_end = bh_pos + blocksize;
			if (bh_end <= pos || bh_pos >= end) {
				if (!buffer_uptodate(bh))
					partial = true;
			} else {
				set_buffer_uptodate(bh);
				mark_buffer_dirty(bh);
			}
		} while (bh_pos += blocksize, (bh = bh->b_this_page) != head);
		/*
		 * If all buffers are now uptodate but the page is not, set the
		 * page uptodate.
		 */
		if (!partial && !PageUptodate(page))
			SetPageUptodate(page);
	} while (++u < nr_pages);
	/*
	 * Finally, if we do not need to update initialized_size or i_size we
	 * are finished.
	 */
	read_lock_irqsave(&ni->size_lock, flags);
	initialized_size = ni->initialized_size;
	read_unlock_irqrestore(&ni->size_lock, flags);
	if (end <= initialized_size) {
		ntfs_debug("Done.");
		return 0;
	}
	/*
	 * Update initialized_size/i_size as appropriate, both in the inode and
	 * the mft record.
	 */
	if (!NInoAttr(ni))
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;
	/* Map, pin, and lock the mft record. */
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		m = NULL;
		ctx = NULL;
		goto err_out;
	}
	BUG_ON(!NInoNonResident(ni));
	ctx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!ctx)) {
		err = -ENOMEM;
		goto err_out;
	}
	err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err)) {
		if (err == -ENOENT)
			err = -EIO;
		goto err_out;
	}
	a = ctx->attr;
	BUG_ON(!a->non_resident);
	write_lock_irqsave(&ni->size_lock, flags);
	BUG_ON(end > ni->allocated_size);
	ni->initialized_size = end;
	a->data.non_resident.initialized_size = cpu_to_sle64(end);
	if (end > i_size_read(vi)) {
		i_size_write(vi, end);
		a->data.non_resident.data_size =
				a->data.non_resident.initialized_size;
	}
	write_unlock_irqrestore(&ni->size_lock, flags);
	/* Mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	ntfs_debug("Done.");
	return 0;
err_out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (m)
		unmap_mft_record(base_ni);
	ntfs_error(vi->i_sb, "Failed to update initialized_size/i_size (error "
			"code %i).", err);
	if (err != -ENOMEM)
		NVolSetErrors(ni->vol);
	return err;
}

/**
 * ntfs_commit_pages_after_write - commit the received data
 * @pages:	array of destination pages
 * @nr_pages:	number of pages in @pages
 * @pos:	byte position in file at which the write begins
 * @bytes:	number of bytes to be written
 *
 * This is called from ntfs_file_buffered_write() with i_mutex held on the inode
 * (@pages[0]->mapping->host).  There are @nr_pages pages in @pages which are
 * locked but not kmap()ped.  The source data has already been copied into the
 * @page.  ntfs_prepare_pages_for_non_resident_write() has been called before
 * the data was copied (for non-resident attributes only) and it returned
 * success.
 *
 * Need to set uptodate and mark dirty all buffers within the boundary of the
 * write.  If all buffers in a page are uptodate we set the page uptodate, too.
 *
 * Setting the buffers dirty ensures that they get written out later when
 * ntfs_writepage() is invoked by the VM.
 *
 * Finally, we need to update i_size and initialized_size as appropriate both
 * in the inode and the mft record.
 *
 * This is modelled after fs/buffer.c::generic_commit_write(), which marks
 * buffers uptodate and dirty, sets the page uptodate if all buffers in the
 * page are uptodate, and updates i_size if the end of io is beyond i_size.  In
 * that case, it also marks the inode dirty.
 *
 * If things have gone as outlined in
 * ntfs_prepare_pages_for_non_resident_write(), we do not need to do any page
 * content modifications here for non-resident attributes.  For resident
 * attributes we need to do the uptodate bringing here which we combine with
 * the copying into the mft record which means we save one atomic kmap.
 *
 * Return 0 on success or -errno on error.
 */
static int ntfs_commit_pages_after_write(struct page **pages,
		const unsigned nr_pages, s64 pos, size_t bytes)
{
	s64 end, initialized_size;
	loff_t i_size;
	struct inode *vi;
	ntfs_inode *ni, *base_ni;
	struct page *page;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	char *kattr, *kaddr;
	unsigned long flags;
	u32 attr_len;
	int err;

	BUG_ON(!nr_pages);
	BUG_ON(!pages);
	page = pages[0];
	BUG_ON(!page);
	vi = page->mapping->host;
	ni = NTFS_I(vi);
	ntfs_debug("Entering for inode 0x%lx, attribute type 0x%x, start page "
			"index 0x%lx, nr_pages 0x%x, pos 0x%llx, bytes 0x%zx.",
			vi->i_ino, ni->type, page->index, nr_pages,
			(long long)pos, bytes);
	if (NInoNonResident(ni))
		return ntfs_commit_pages_after_non_resident_write(pages,
				nr_pages, pos, bytes);
	BUG_ON(nr_pages > 1);
	/*
	 * Attribute is resident, implying it is not compressed, encrypted, or
	 * sparse.
	 */
	if (!NInoAttr(ni))
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;
	BUG_ON(NInoNonResident(ni));
	/* Map, pin, and lock the mft record. */
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		m = NULL;
		ctx = NULL;
		goto err_out;
	}
	ctx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!ctx)) {
		err = -ENOMEM;
		goto err_out;
	}
	err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err)) {
		if (err == -ENOENT)
			err = -EIO;
		goto err_out;
	}
	a = ctx->attr;
	BUG_ON(a->non_resident);
	/* The total length of the attribute value. */
	attr_len = le32_to_cpu(a->data.resident.value_length);
	i_size = i_size_read(vi);
	BUG_ON(attr_len != i_size);
	BUG_ON(pos > attr_len);
	end = pos + bytes;
	BUG_ON(end > le32_to_cpu(a->length) -
			le16_to_cpu(a->data.resident.value_offset));
	kattr = (u8*)a + le16_to_cpu(a->data.resident.value_offset);
	kaddr = kmap_atomic(page, KM_USER0);
	/* Copy the received data from the page to the mft record. */
	memcpy(kattr + pos, kaddr + pos, bytes);
	/* Update the attribute length if necessary. */
	if (end > attr_len) {
		attr_len = end;
		a->data.resident.value_length = cpu_to_le32(attr_len);
	}
	/*
	 * If the page is not uptodate, bring the out of bounds area(s)
	 * uptodate by copying data from the mft record to the page.
	 */
	if (!PageUptodate(page)) {
		if (pos > 0)
			memcpy(kaddr, kattr, pos);
		if (end < attr_len)
			memcpy(kaddr + end, kattr + end, attr_len - end);
		/* Zero the region outside the end of the attribute value. */
		memset(kaddr + attr_len, 0, PAGE_CACHE_SIZE - attr_len);
		flush_dcache_page(page);
		SetPageUptodate(page);
	}
	kunmap_atomic(kaddr, KM_USER0);
	/* Update initialized_size/i_size if necessary. */
	read_lock_irqsave(&ni->size_lock, flags);
	initialized_size = ni->initialized_size;
	BUG_ON(end > ni->allocated_size);
	read_unlock_irqrestore(&ni->size_lock, flags);
	BUG_ON(initialized_size != i_size);
	if (end > initialized_size) {
		write_lock_irqsave(&ni->size_lock, flags);
		ni->initialized_size = end;
		i_size_write(vi, end);
		write_unlock_irqrestore(&ni->size_lock, flags);
	}
	/* Mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	ntfs_debug("Done.");
	return 0;
err_out:
	if (err == -ENOMEM) {
		ntfs_warning(vi->i_sb, "Error allocating memory required to "
				"commit the write.");
		if (PageUptodate(page)) {
			ntfs_warning(vi->i_sb, "Page is uptodate, setting "
					"dirty so the write will be retried "
					"later on by the VM.");
			/*
			 * Put the page on mapping->dirty_pages, but leave its
			 * buffers' dirty state as-is.
			 */
			__set_page_dirty_nobuffers(page);
			err = 0;
		} else
			ntfs_error(vi->i_sb, "Page is not uptodate.  Written "
					"data has been lost.");
	} else {
		ntfs_error(vi->i_sb, "Resident attribute commit write failed "
				"with error %i.", err);
		NVolSetErrors(ni->vol);
	}
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (m)
		unmap_mft_record(base_ni);
	return err;
}

/**
 * ntfs_file_buffered_write -
 *
 * Locking: The vfs is holding ->i_mutex on the inode.
 */
static ssize_t ntfs_file_buffered_write(struct kiocb *iocb,
		const struct iovec *iov, unsigned long nr_segs,
		loff_t pos, loff_t *ppos, size_t count)
{
	struct file *file = iocb->ki_filp;
	struct address_space *mapping = file->f_mapping;
	struct inode *vi = mapping->host;
	ntfs_inode *ni = NTFS_I(vi);
	ntfs_volume *vol = ni->vol;
	struct page *pages[NTFS_MAX_PAGES_PER_CLUSTER];
	struct page *cached_page = NULL;
	char __user *buf = NULL;
	s64 end, ll;
	VCN last_vcn;
	LCN lcn;
	unsigned long flags;
	size_t bytes, iov_ofs = 0;	/* Offset in the current iovec. */
	ssize_t status, written;
	unsigned nr_pages;
	int err;
	struct pagevec lru_pvec;

	ntfs_debug("Entering for i_ino 0x%lx, attribute type 0x%x, "
			"pos 0x%llx, count 0x%lx.",
			vi->i_ino, (unsigned)le32_to_cpu(ni->type),
			(unsigned long long)pos, (unsigned long)count);
	if (unlikely(!count))
		return 0;
	BUG_ON(NInoMstProtected(ni));
	/*
	 * If the attribute is not an index root and it is encrypted or
	 * compressed, we cannot write to it yet.  Note we need to check for
	 * AT_INDEX_ALLOCATION since this is the type of both directory and
	 * index inodes.
	 */
	if (ni->type != AT_INDEX_ALLOCATION) {
		/* If file is encrypted, deny access, just like NT4. */
		if (NInoEncrypted(ni)) {
			/*
			 * Reminder for later: Encrypted files are _always_
			 * non-resident so that the content can always be
			 * encrypted.
			 */
			ntfs_debug("Denying write access to encrypted file.");
			return -EACCES;
		}
		if (NInoCompressed(ni)) {
			/* Only unnamed $DATA attribute can be compressed. */
			BUG_ON(ni->type != AT_DATA);
			BUG_ON(ni->name_len);
			/*
			 * Reminder for later: If resident, the data is not
			 * actually compressed.  Only on the switch to non-
			 * resident does compression kick in.  This is in
			 * contrast to encrypted files (see above).
			 */
			ntfs_error(vi->i_sb, "Writing to compressed files is "
					"not implemented yet.  Sorry.");
			return -EOPNOTSUPP;
		}
	}
	/*
	 * If a previous ntfs_truncate() failed, repeat it and abort if it
	 * fails again.
	 */
	if (unlikely(NInoTruncateFailed(ni))) {
		down_write(&vi->i_alloc_sem);
		err = ntfs_truncate(vi);
		up_write(&vi->i_alloc_sem);
		if (err || NInoTruncateFailed(ni)) {
			if (!err)
				err = -EIO;
			ntfs_error(vol->sb, "Cannot perform write to inode "
					"0x%lx, attribute type 0x%x, because "
					"ntfs_truncate() failed (error code "
					"%i).", vi->i_ino,
					(unsigned)le32_to_cpu(ni->type), err);
			return err;
		}
	}
	/* The first byte after the write. */
	end = pos + count;
	/*
	 * If the write goes beyond the allocated size, extend the allocation
	 * to cover the whole of the write, rounded up to the nearest cluster.
	 */
	read_lock_irqsave(&ni->size_lock, flags);
	ll = ni->allocated_size;
	read_unlock_irqrestore(&ni->size_lock, flags);
	if (end > ll) {
		/* Extend the allocation without changing the data size. */
		ll = ntfs_attr_extend_allocation(ni, end, -1, pos);
		if (likely(ll >= 0)) {
			BUG_ON(pos >= ll);
			/* If the extension was partial truncate the write. */
			if (end > ll) {
				ntfs_debug("Truncating write to inode 0x%lx, "
						"attribute type 0x%x, because "
						"the allocation was only "
						"partially extended.",
						vi->i_ino, (unsigned)
						le32_to_cpu(ni->type));
				end = ll;
				count = ll - pos;
			}
		} else {
			err = ll;
			read_lock_irqsave(&ni->size_lock, flags);
			ll = ni->allocated_size;
			read_unlock_irqrestore(&ni->size_lock, flags);
			/* Perform a partial write if possible or fail. */
			if (pos < ll) {
				ntfs_debug("Truncating write to inode 0x%lx, "
						"attribute type 0x%x, because "
						"extending the allocation "
						"failed (error code %i).",
						vi->i_ino, (unsigned)
						le32_to_cpu(ni->type), err);
				end = ll;
				count = ll - pos;
			} else {
				ntfs_error(vol->sb, "Cannot perform write to "
						"inode 0x%lx, attribute type "
						"0x%x, because extending the "
						"allocation failed (error "
						"code %i).", vi->i_ino,
						(unsigned)
						le32_to_cpu(ni->type), err);
				return err;
			}
		}
	}
	pagevec_init(&lru_pvec, 0);
	written = 0;
	/*
	 * If the write starts beyond the initialized size, extend it up to the
	 * beginning of the write and initialize all non-sparse space between
	 * the old initialized size and the new one.  This automatically also
	 * increments the vfs inode->i_size to keep it above or equal to the
	 * initialized_size.
	 */
	read_lock_irqsave(&ni->size_lock, flags);
	ll = ni->initialized_size;
	read_unlock_irqrestore(&ni->size_lock, flags);
	if (pos > ll) {
		err = ntfs_attr_extend_initialized(ni, pos, &cached_page,
				&lru_pvec);
		if (err < 0) {
			ntfs_error(vol->sb, "Cannot perform write to inode "
					"0x%lx, attribute type 0x%x, because "
					"extending the initialized size "
					"failed (error code %i).", vi->i_ino,
					(unsigned)le32_to_cpu(ni->type), err);
			status = err;
			goto err_out;
		}
	}
	/*
	 * Determine the number of pages per cluster for non-resident
	 * attributes.
	 */
	nr_pages = 1;
	if (vol->cluster_size > PAGE_CACHE_SIZE && NInoNonResident(ni))
		nr_pages = vol->cluster_size >> PAGE_CACHE_SHIFT;
	/* Finally, perform the actual write. */
	last_vcn = -1;
	if (likely(nr_segs == 1))
		buf = iov->iov_base;
	do {
		VCN vcn;
		pgoff_t idx, start_idx;
		unsigned ofs, do_pages, u;
		size_t copied;

		start_idx = idx = pos >> PAGE_CACHE_SHIFT;
		ofs = pos & ~PAGE_CACHE_MASK;
		bytes = PAGE_CACHE_SIZE - ofs;
		do_pages = 1;
		if (nr_pages > 1) {
			vcn = pos >> vol->cluster_size_bits;
			if (vcn != last_vcn) {
				last_vcn = vcn;
				/*
				 * Get the lcn of the vcn the write is in.  If
				 * it is a hole, need to lock down all pages in
				 * the cluster.
				 */
				down_read(&ni->runlist.lock);
				lcn = ntfs_attr_vcn_to_lcn_nolock(ni, pos >>
						vol->cluster_size_bits, false);
				up_read(&ni->runlist.lock);
				if (unlikely(lcn < LCN_HOLE)) {
					status = -EIO;
					if (lcn == LCN_ENOMEM)
						status = -ENOMEM;
					else
						ntfs_error(vol->sb, "Cannot "
							"perform write to "
							"inode 0x%lx, "
							"attribute type 0x%x, "
							"because the attribute "
							"is corrupt.",
							vi->i_ino, (unsigned)
							le32_to_cpu(ni->type));
					break;
				}
				if (lcn == LCN_HOLE) {
					start_idx = (pos & ~(s64)
							vol->cluster_size_mask)
							>> PAGE_CACHE_SHIFT;
					bytes = vol->cluster_size - (pos &
							vol->cluster_size_mask);
					do_pages = nr_pages;
				}
			}
		}
		if (bytes > count)
			bytes = count;
		/*
		 * Bring in the user page(s) that we will copy from _first_.
		 * Otherwise there is a nasty deadlock on copying from the same
		 * page(s) as we are writing to, without it/them being marked
		 * up-to-date.  Note, at present there is nothing to stop the
		 * pages being swapped out between us bringing them into memory
		 * and doing the actual copying.
		 */
		if (likely(nr_segs == 1))
			ntfs_fault_in_pages_readable(buf, bytes);
		else
			ntfs_fault_in_pages_readable_iovec(iov, iov_ofs, bytes);
		/* Get and lock @do_pages starting at index @start_idx. */
		status = __ntfs_grab_cache_pages(mapping, start_idx, do_pages,
				pages, &cached_page, &lru_pvec);
		if (unlikely(status))
			break;
		/*
		 * For non-resident attributes, we need to fill any holes with
		 * actual clusters and ensure all bufferes are mapped.  We also
		 * need to bring uptodate any buffers that are only partially
		 * being written to.
		 */
		if (NInoNonResident(ni)) {
			status = ntfs_prepare_pages_for_non_resident_write(
					pages, do_pages, pos, bytes);
			if (unlikely(status)) {
				loff_t i_size;

				do {
					unlock_page(pages[--do_pages]);
					page_cache_release(pages[do_pages]);
				} while (do_pages);
				/*
				 * The write preparation may have instantiated
				 * allocated space outside i_size.  Trim this
				 * off again.  We can ignore any errors in this
				 * case as we will just be waisting a bit of
				 * allocated space, which is not a disaster.
				 */
				i_size = i_size_read(vi);
				if (pos + bytes > i_size)
					vmtruncate(vi, i_size);
				break;
			}
		}
		u = (pos >> PAGE_CACHE_SHIFT) - pages[0]->index;
		if (likely(nr_segs == 1)) {
			copied = ntfs_copy_from_user(pages + u, do_pages - u,
					ofs, buf, bytes);
			buf += copied;
		} else
			copied = ntfs_copy_from_user_iovec(pages + u,
					do_pages - u, ofs, &iov, &iov_ofs,
					bytes);
		ntfs_flush_dcache_pages(pages + u, do_pages - u);
		status = ntfs_commit_pages_after_write(pages, do_pages, pos,
				bytes);
		if (likely(!status)) {
			written += copied;
			count -= copied;
			pos += copied;
			if (unlikely(copied != bytes))
				status = -EFAULT;
		}
		do {
			unlock_page(pages[--do_pages]);
			mark_page_accessed(pages[do_pages]);
			page_cache_release(pages[do_pages]);
		} while (do_pages);
		if (unlikely(status))
			break;
		balance_dirty_pages_ratelimited(mapping);
		cond_resched();
	} while (count);
err_out:
	*ppos = pos;
	if (cached_page)
		page_cache_release(cached_page);
	/* For now, when the user asks for O_SYNC, we actually give O_DSYNC. */
	if (likely(!status)) {
		if (unlikely((file->f_flags & O_SYNC) || IS_SYNC(vi))) {
			if (!mapping->a_ops->writepage || !is_sync_kiocb(iocb))
				status = generic_osync_inode(vi, mapping,
						OSYNC_METADATA|OSYNC_DATA);
		}
  	}
	pagevec_lru_add_file(&lru_pvec);
	ntfs_debug("Done.  Returning %s (written 0x%lx, status %li).",
			written ? "written" : "status", (unsigned long)written,
			(long)status);
	return written ? written : status;
}

/**
 * ntfs_file_aio_write_nolock -
 */
static ssize_t ntfs_file_aio_write_nolock(struct kiocb *iocb,
		const struct iovec *iov, unsigned long nr_segs, loff_t *ppos)
{
	struct file *file = iocb->ki_filp;
	struct address_space *mapping = file->f_mapping;
	struct inode *inode = mapping->host;
	loff_t pos;
	size_t count;		/* after file limit checks */
	ssize_t written, err;

	count = 0;
	err = generic_segment_checks(iov, &nr_segs, &count, VERIFY_READ);
	if (err)
		return err;
	pos = *ppos;
	vfs_check_frozen(inode->i_sb, SB_FREEZE_WRITE);
	/* We can write back this queue in page reclaim. */
	current->backing_dev_info = mapping->backing_dev_info;
	written = 0;
	err = generic_write_checks(file, &pos, &count, S_ISBLK(inode->i_mode));
	if (err)
		goto out;
	if (!count)
		goto out;
	err = file_remove_suid(file);
	if (err)
		goto out;
	file_update_time(file);
	written = ntfs_file_buffered_write(iocb, iov, nr_segs, pos, ppos,
			count);
out:
	current->backing_dev_info = NULL;
	return written ? written : err;
}

/**
 * ntfs_file_aio_write -
 */
static ssize_t ntfs_file_aio_write(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t pos)
{
	struct file *file = iocb->ki_filp;
	struct address_space *mapping = file->f_mapping;
	struct inode *inode = mapping->host;
	ssize_t ret;

	BUG_ON(iocb->ki_pos != pos);

	mutex_lock(&inode->i_mutex);
	ret = ntfs_file_aio_write_nolock(iocb, iov, nr_segs, &iocb->ki_pos);
	mutex_unlock(&inode->i_mutex);
	if (ret > 0 && ((file->f_flags & O_SYNC) || IS_SYNC(inode))) {
		int err = sync_page_range(inode, mapping, pos, ret);
		if (err < 0)
			ret = err;
	}
	return ret;
}

/**
 * ntfs_file_writev -
 *
 * Basically the same as generic_file_writev() except that it ends up calling
 * ntfs_file_aio_write_nolock() instead of __generic_file_aio_write_nolock().
 */
static ssize_t ntfs_file_writev(struct file *file, const struct iovec *iov,
		unsigned long nr_segs, loff_t *ppos)
{
	struct address_space *mapping = file->f_mapping;
	struct inode *inode = mapping->host;
	struct kiocb kiocb;
	ssize_t ret;

	mutex_lock(&inode->i_mutex);
	init_sync_kiocb(&kiocb, file);
	ret = ntfs_file_aio_write_nolock(&kiocb, iov, nr_segs, ppos);
	if (ret == -EIOCBQUEUED)
		ret = wait_on_sync_kiocb(&kiocb);
	mutex_unlock(&inode->i_mutex);
	if (ret > 0 && ((file->f_flags & O_SYNC) || IS_SYNC(inode))) {
		int err = sync_page_range(inode, mapping, *ppos - ret, ret);
		if (err < 0)
			ret = err;
	}
	return ret;
}

/**
 * ntfs_file_write - simple wrapper for ntfs_file_writev()
 */
static ssize_t ntfs_file_write(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	struct iovec local_iov = { .iov_base = (void __user *)buf,
				   .iov_len = count };

	return ntfs_file_writev(file, &local_iov, 1, ppos);
}

/**
 * ntfs_file_fsync - sync a file to disk
 * @filp:	file to be synced
 * @dentry:	dentry describing the file to sync
 * @datasync:	if non-zero only flush user data and not metadata
 *
 * Data integrity sync of a file to disk.  Used for fsync, fdatasync, and msync
 * system calls.  This function is inspired by fs/buffer.c::file_fsync().
 *
 * If @datasync is false, write the mft record and all associated extent mft
 * records as well as the $DATA attribute and then sync the block device.
 *
 * If @datasync is true and the attribute is non-resident, we skip the writing
 * of the mft record and all associated extent mft records (this might still
 * happen due to the write_inode_now() call).
 *
 * Also, if @datasync is true, we do not wait on the inode to be written out
 * but we always wait on the page cache pages to be written out.
 *
 * Note: In the past @filp could be NULL so we ignore it as we don't need it
 * anyway.
 *
 * Locking: Caller must hold i_mutex on the inode.
 *
 * TODO: We should probably also write all attribute/index inodes associated
 * with this inode but since we have no simple way of getting to them we ignore
 * this problem for now.
 */
static int ntfs_file_fsync(struct file *filp, struct dentry *dentry,
		int datasync)
{
	struct inode *vi = dentry->d_inode;
	int err, ret = 0;

	ntfs_debug("Entering for inode 0x%lx.", vi->i_ino);
	BUG_ON(S_ISDIR(vi->i_mode));
	if (!datasync || !NInoNonResident(NTFS_I(vi)))
		ret = ntfs_write_inode(vi, 1);
	write_inode_now(vi, !datasync);
	/*
	 * NOTE: If we were to use mapping->private_list (see ext2 and
	 * fs/buffer.c) for dirty blocks then we could optimize the below to be
	 * sync_mapping_buffers(vi->i_mapping).
	 */
	err = sync_blockdev(vi->i_sb->s_bdev);
	if (unlikely(err && !ret))
		ret = err;
	if (likely(!ret))
		ntfs_debug("Done.");
	else
		ntfs_warning(vi->i_sb, "Failed to f%ssync inode 0x%lx.  Error "
				"%u.", datasync ? "data" : "", vi->i_ino, -ret);
	return ret;
}

#endif /* NTFS_RW */

const struct file_operations ntfs_file_ops = {
	.llseek		= generic_file_llseek,	 /* Seek inside file. */
	.read		= do_sync_read,		 /* Read from file. */
	.aio_read	= generic_file_aio_read, /* Async read from file. */
#ifdef NTFS_RW
	.write		= ntfs_file_write,	 /* Write to file. */
	.aio_write	= ntfs_file_aio_write,	 /* Async write to file. */
	/*.release	= ,*/			 /* Last file is closed.  See
						    fs/ext2/file.c::
						    ext2_release_file() for
						    how to use this to discard
						    preallocated space for
						    write opened files. */
	.fsync		= ntfs_file_fsync,	 /* Sync a file to disk. */
	/*.aio_fsync	= ,*/			 /* Sync all outstanding async
						    i/o operations on a
						    kiocb. */
#endif /* NTFS_RW */
	/*.ioctl	= ,*/			 /* Perform function on the
						    mounted filesystem. */
	.mmap		= generic_file_mmap,	 /* Mmap file. */
	.open		= ntfs_file_open,	 /* Open file. */
	.splice_read	= generic_file_splice_read /* Zero-copy data send with
						    the data source being on
						    the ntfs partition.  We do
						    not need to care about the
						    data destination. */
	/*.sendpage	= ,*/			 /* Zero-copy data send with
						    the data destination being
						    on the ntfs partition.  We
						    do not need to care about
						    the data source. */
};

const struct inode_operations ntfs_file_inode_ops = {
#ifdef NTFS_RW
	.truncate	= ntfs_truncate_vfs,
	.setattr	= ntfs_setattr,
#endif /* NTFS_RW */
};

const struct file_operations ntfs_empty_file_ops = {};

const struct inode_operations ntfs_empty_inode_ops = {};

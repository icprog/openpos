INDX( 	                 (   �   �      � ��k                 y7     h R     V7     �m��;����
��̎��;��x�����      �               M a k e f i l e                     �m��;����
��̎��;��x�����      �               M a k e f i l e                     �m��;����
��̎��;��x�����      �               M a k e f i l e                     �m��;����
��̎��;��x�����      �               M a k e f i l e                     V7     �m��;����
��̎��;��x���� �      �               M a k e f i l e                     V7     �m��;����
��̎��;��x�����      �               M a k e f i l e                     h R     V7     �m��;����
��̎��;��x�����      �               M a k e f i l e                     V7     �m��;����
��̎��;��x�����      �               M a k e f i l e                     V7     �m��;����
��̎��;��x�����      �               M a k e f i l e                     V7     �m��;����
�� ̎��;��x�����      �               M a k e f i l e                     h R     V7     �m��;����
��̎��;��x�����      �               M a k e f i l e                     h R     V7     �m��;����
��̎��;��x�����      �               M a k e f i l e                     y7     h R     V7     �m��;����
��̎��;��x�����      �               M a k e f i l e                     y7     h R     V7     �m��;����
��̎��;��x�����      �               M a � e f i l e                     �m��;����
��̎��;��x�����      �               M a k e f i l e                     �m��;����
��̎��;��x�����      �               M a k e f i l e                                                                                                                                                                                                                                                                                                                   �                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               �                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               �                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               �                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               � /* OmniVision OV76BE Camera Chip Support Code
 *
 * Copyright (c) 1999-2004 Mark McClelland <mark@alpha.dyndns.org>
 * http://alpha.dyndns.org/ov511/
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. NO WARRANTY OF ANY KIND is expressed or implied.
 */

#define DEBUG

#include <linux/slab.h>
#include "ovcamchip_priv.h"

/* OV7610 registers: Since the OV76BE is undocumented, we'll settle for these
 * for now. */
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

struct ov76be {
	int auto_brt;
	int auto_exp;
	int bandfilt;
	int mirror;
};

/* NOTE: These are the same as the 7x10 settings, but should eventually be
 * optimized for the OV76BE */
static struct ovcamchip_regvals regvals_init_76be[] = {
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

/* This initializes the OV76be camera chip and relevant variables. */
static int ov76be_init(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov76be *s;
	int rc;

	DDEBUG(4, &c->dev, "entered");

	rc = ov_write_regvals(c, regvals_init_76be);
	if (rc < 0)
		return rc;

	ov->spriv = s = kzalloc(sizeof *s, GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->auto_brt = 1;
	s->auto_exp = 1;

	return rc;
}

static int ov76be_free(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);

	kfree(ov->spriv);
	return 0;
}

static int ov76be_set_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov76be *s = ov->spriv;
	int rc;
	int v = ctl->value;

	switch (ctl->id) {
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_write(c, REG_BRT, v >> 8);
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_write(c, REG_SAT, v >> 8);
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

static int ov76be_get_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov76be *s = ov->spriv;
	int rc = 0;
	unsigned char val = 0;

	switch (ctl->id) {
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

static int ov76be_mode_init(struct i2c_client *c, struct ovcamchip_window *win)
{
	int qvga = win->quarter;

	/******** QVGA-specific regs ********/

	ov_write(c, 0x14, qvga?0xa4:0x84);

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

static int ov76be_set_window(struct i2c_client *c, struct ovcamchip_window *win)
{
	int ret, hwscale, vwscale;

	ret = ov76be_mode_init(c, win);
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

static int ov76be_command(struct i2c_client *c, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case OVCAMCHIP_CMD_S_CTRL:
		return ov76be_set_control(c, arg);
	case OVCAMCHIP_CMD_G_CTRL:
		return ov76be_get_control(c, arg);
	case OVCAMCHIP_CMD_S_MODE:
		return ov76be_set_window(c, arg);
	default:
		DDEBUG(2, &c->dev, "command not supported: %d", cmd);
		return -ENOIOCTLCMD;
	}
}

struct ovcamchip_ops ov76be_ops = {
	.init    =	ov76be_init,
	.free    =	ov76be_free,
	.command =	ov76be_command,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ags_proc_fops;
static const struct file_operations cifs_experimental_proc_fops;
static const struct file_operations cifs_linux_ext_proc_fops;

void
cifs_proc_init(void)
{
	proc_fs_cifs = proc_mkdir("fs/cifs", NULL);
	if (proc_fs_cifs == NULL)
		return;

	proc_create("DebugData", 0, proc_fs_cifs, &cifs_debug_data_proc_fops);

#ifdef CONFIG_CIFS_STATS
	proc_create("Stats", 0, proc_fs_cifs, &cifs_stats_proc_fops);
#endif /* STATS */
	proc_create("cifsFYI", 0, proc_fs_cifs, &cifsFYI_proc_fops);
	proc_create("traceSMB", 0, proc_fs_cifs, &traceSMB_proc_fops);
	proc_create("OplockEnabled", 0, proc_fs_cifs, &cifs_oplock_proc_fops);
	proc_create("Experimental", 0, proc_fs_cifs,
		    &cifs_experimental_proc_fops);
	proc_create("LinuxExtensionsEnabled", 0, proc_fs_cifs,
		    &cifs_linux_ext_proc_fops);
	proc_create("MultiuserMount", 0, proc_fs_cifs,
		    &cifs_multiuser_mount_proc_fops);
	proc_create("SecurityFlags", 0, proc_fs_cifs,
		    &cifs_security_flags_proc_fops);
	proc_create("LookupCacheEnabled", 0, proc_fs_cifs,
		    &cifs_lookup_cache_proc_fops);
}

void
cifs_proc_clean(void)
{
	if (proc_fs_cifs == NULL)
		return;

	remove_proc_entry("DebugData", proc_fs_cifs);
	remove_proc_entry("cifsFYI", proc_fs_cifs);
	remove_proc_entry("traceSMB", proc_fs_cifs);
#ifdef CONFIG_CIFS_STATS
	remove_proc_entry("Stats", proc_fs_cifs);
#endif
	remove_proc_entry("MultiuserMount", proc_fs_cifs);
	remove_proc_entry("OplockEnabled", proc_fs_cifs);
	remove_proc_entry("SecurityFlags", proc_fs_cifs);
	remove_proc_entry("LinuxExtensionsEnabled", proc_fs_cifs);
	remove_proc_entry("Experimental", proc_fs_cifs);
	remove_proc_entry("LookupCacheEnabled", proc_fs_cifs);
	remove_proc_entry("fs/cifs", NULL);
}

static int cifsFYI_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", cifsFYI);
	return 0;
}

static int cifsFYI_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifsFYI_proc_show, NULL);
}

static ssize_t cifsFYI_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	char c;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;
	if (c == '0' || c == 'n' || c == 'N')
		cifsFYI = 0;
	else if (c == '1' || c == 'y' || c == 'Y')
		cifsFYI = 1;
	else if ((c > '1') && (c <= '9'))
		cifsFYI = (int) (c - '0'); /* see cifs_debug.h for meanings */

	return count;
}

static const struct file_operations cifsFYI_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifsFYI_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifsFYI_proc_write,
};

static int cifs_oplock_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", oplockEnabled);
	return 0;
}

static int cifs_oplock_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_oplock_proc_show, NULL);
}

static ssize_t cifs_oplock_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char c;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;
	if (c == '0' || c == 'n' || c == 'N')
		oplockEnabled = 0;
	else if (c == '1' || c == 'y' || c == 'Y')
		oplockEnabled = 1;

	return count;
}

static const struct file_operations cifs_oplock_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_oplock_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_oplock_proc_write,
};

static int cifs_experimental_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", experimEnabled);
	return 0;
}

static int cifs_experimental_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_experimental_proc_show, NULL);
}

static ssize_t cifs_experimental_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char c;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;
	if (c == '0' || c == 'n' || c == 'N')
		experimEnabled = 0;
	else if (c == '1' || c == 'y' || c == 'Y')
		experimEnabled = 1;
	else if (c == '2')
		experimEnabled = 2;

	return count;
}

static const struct file_operations cifs_experimental_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_experimental_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_experimental_proc_write,
};

static int cifs_linux_ext_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", linuxExtEnabled);
	return 0;
}

static int cifs_linux_ext_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_linux_ext_proc_show, NULL);
}

static ssize_t cifs_linux_ext_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char c;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;
	if (c == '0' || c == 'n' || c == 'N')
		linuxExtEnabled = 0;
	else if (c == '1' || c == 'y' || c == 'Y')
		linuxExtEnabled = 1;

	return count;
}

static const struct file_operations cifs_linux_ext_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_linux_ext_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_linux_ext_proc_write,
};

static int cifs_lookup_cache_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", lookupCacheEnabled);
	return 0;
}

static int cifs_lookup_cache_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_lookup_cache_proc_show, NULL);
}

static ssize_t cifs_lookup_cache_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char c;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;
	if (c == '0' || c == 'n' || c == 'N')
		lookupCacheEnabled = 0;
	else if (c == '1' || c == 'y' || c == 'Y')
		lookupCacheEnabled = 1;

	return count;
}

static const struct file_operations cifs_lookup_cache_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_lookup_cache_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_lookup_cache_proc_write,
};

static int traceSMB_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", traceSMB);
	return 0;
}

static int traceSMB_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, traceSMB_proc_show, NULL);
}

static ssize_t traceSMB_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	char c;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;
	if (c == '0' || c == 'n' || c == 'N')
		traceSMB = 0;
	else if (c == '1' || c == 'y' || c == 'Y')
		traceSMB = 1;

	return count;
}

static const struct file_operations traceSMB_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= traceSMB_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= traceSMB_proc_write,
};

static int cifs_multiuser_mount_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", multiuser_mount);
	return 0;
}

static int cifs_multiuser_mount_proc_open(struct inode *inode, struct file *fh)
{
	return single_open(fh, cifs_multiuser_mount_proc_show, NULL);
}

static ssize_t cifs_multiuser_mount_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char c;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;
	if (c == '0' || c == 'n' || c == 'N')
		multiuser_mount = 0;
	else if (c == '1' || c == 'y' || c == 'Y')
		multiuser_mount = 1;

	return count;
}

static const struct file_operations cifs_multiuser_mount_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_multiuser_mount_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_multiuser_mount_proc_write,
};

static int cifs_security_flags_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%x\n", extended_security);
	return 0;
}

static int cifs_security_flags_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_security_flags_proc_show, NULL);
}

static ssize_t cifs_security_flags_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned int flags;
	char flags_string[12];
	char c;

	if ((count < 1) || (count > 11))
		return -EINVAL;

	memset(flags_string, 0, 12);

	if (copy_from_user(flags_string, buffer, count))
		return -EFAULT;

	if (count < 3) {
		/* single char or single char followed by null */
		c = flags_string[0];
		if (c == '0' || c == 'n' || c == 'N') {
			extended_security = CIFSSEC_DEF; /* default */
			return count;
		} else if (c == '1' || c == 'y' || c == 'Y') {
			extended_security = CIFSSEC_MAX;
			return count;
		} else if (!isdigit(c)) {
			cERROR(1, ("invalid flag %c", c));
			return -EINVAL;
		}
	}
	/* else we have a number */

	flags = simple_strtoul(flags_string, NULL, 0);

	cFYI(1, ("sec flags 0x%x", flags));

	if (flags <= 0)  {
		cERROR(1, ("invalid security flags %s", flags_string));
		return -EINVAL;
	}

	if (flags & ~CIFSSEC_MASK) {
		cERROR(1, ("attempt to set unsupported security flags 0x%x",
			flags & ~CIFSSEC_MASK));
		return -EINVAL;
	}
	/* flags look ok - update the global security flags for cifs module */
	extended_security = flags;
	if (extended_security & CIFSSEC_MUST_SIGN) {
		/* requiring signing implies signing is allowed */
		extended_security |= CIFSSEC_MAY_SIGN;
		cFYI(1, ("packet signing now required"));
	} else if ((extended_security & CIFSSEC_MAY_SIGN) == 0) {
		cFYI(1, ("packet signing disabled"));
	}
	/* BB should we turn on MAY flags for other MUST options? */
	return count;
}

static const struct file_operations cifs_security_flags_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_security_flags_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_security_flags_proc_write,
};
#else
inline void cifs_proc_init(void)
{
}

inline void cifs_proc_clean(void)
{
}
#endif /* PROC_FS */

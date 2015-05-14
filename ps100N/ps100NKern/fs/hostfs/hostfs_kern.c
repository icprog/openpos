le        */
#define PCI_DEVICE_ID_RP8OCTA		0x0005	/* Rocketport 8 port w/octa cable        */
#define PCI_DEVICE_ID_RP8J		0x0006	/* Rocketport 8 port w/RJ11 connectors   */
#define PCI_DEVICE_ID_RP4J		0x0007	/* Rocketport 4 port w/RJ11 connectors   */
#define PCI_DEVICE_ID_RP8SNI		0x0008	/* Rocketport 8 port w/ DB78 SNI (Siemens) connector */
#define PCI_DEVICE_ID_RP16SNI		0x0009	/* Rocketport 16 port w/ DB78 SNI (Siemens) connector   */
#define PCI_DEVICE_ID_RPP4		0x000A	/* Rocketport Plus 4 port                */
#define PCI_DEVICE_ID_RPP8		0x000B	/* Rocketport Plus 8 port                */
#define PCI_DEVICE_ID_RP6M		0x000C	/* RocketModem 6 port                    */
#define PCI_DEVICE_ID_RP4M		0x000D	/* RocketModem 4 port                    */
#define PCI_DEVICE_ID_RP2_232           0x000E	/* Rocketport Plus 2 port RS232          */
#define PCI_DEVICE_ID_RP2_422           0x000F	/* Rocketport Plus 2 port RS422          */ 

/* Universal PCI boards  */
#define PCI_DEVICE_ID_URP32INTF		0x0801	/* Rocketport UPCI 32 port w/external I/F */ 
#define PCI_DEVICE_ID_URP8INTF		0x0802	/* Rocketport UPCI 8 port w/external I/F  */
#define PCI_DEVICE_ID_URP16INTF		0x0803	/* Rocketport UPCI 16 port w/external I/F */
#define PCI_DEVICE_ID_URP8OCTA		0x0805	/* Rocketport UPCI 8 port w/octa cable    */
#define PCI_DEVICE_ID_UPCI_RM3_8PORT    0x080C	/* Rocketmodem III 8 port                 */
#define PCI_DEVICE_ID_UPCI_RM3_4PORT    0x080D	/* Rocketmodem III 4 port                 */

/* Compact PCI device */ 
#define PCI_DEVICE_ID_CRP16INTF		0x0903	/* Rocketport Compact PCI 16 port w/external I/F */

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  /*
 * Colour AR M64278(VGA) driver for Video4Linux
 *
 * Copyright (C) 2003	Takeo Takahashi <takahashi.takeo@renesas.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Some code is taken from AR driver sample program for M3T-M32700UT.
 *
 * AR driver sample (M32R SDK):
 *     Copyright (c) 2003 RENESAS TECHNOROGY CORPORATION
 *     AND RENESAS SOLUTIONS CORPORATION
 *     All Rights Reserved.
 *
 * 2003-09-01:	Support w3cam by Takeo Takahashi
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/videodev.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <linux/mutex.h>

#include <asm/uaccess.h>
#include <asm/m32r.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <asm/byteorder.h>

#if 0
#define DEBUG(n, args...) printk(args)
#define CHECK_LOST	1
#else
#define DEBUG(n, args...)
#define CHECK_LOST	0
#endif

/*
 * USE_INT is always 0, interrupt mode is not available
 * on linux due to lack of speed
 */
#define USE_INT		0	/* Don't modify */

#define VERSION	"0.03"

#define ar_inl(addr) 		inl((unsigned long)(addr))
#define ar_outl(val, addr)	outl((unsigned long)(val),(unsigned long)(addr))

extern struct cpuinfo_m32r	boot_cpu_data;

/*
 * CCD pixel size
 *	Note that M32700UT does not support CIF mode, but QVGA is
 *	supported by M32700UT hardware using VGA mode of AR LSI.
 *
 * 	Supported: VGA  (Normal mode, Interlace mode)
 *		   QVGA (Always Interlace mode of VGA)
 *
 */
#define AR_WIDTH_VGA		640
#define AR_HEIGHT_VGA		480
#define AR_WIDTH_QVGA		320
#define AR_HEIGHT_QVGA		240
#define MIN_AR_WIDTH		AR_WIDTH_QVGA
#define MIN_AR_HEIGHT		AR_HEIGHT_QVGA
#define MAX_AR_WIDTH		AR_WIDTH_VGA
#define MAX_AR_HEIGHT		AR_HEIGHT_VGA

/* bits & bytes per pixel */
#define AR_BITS_PER_PIXEL	16
#define AR_BYTES_PER_PIXEL	(AR_BITS_PER_PIXEL/8)

/* line buffer size */
#define AR_LINE_BYTES_VGA	(AR_WIDTH_VGA * AR_BYTES_PER_PIXEL)
#define AR_LINE_BYTES_QVGA	(AR_WIDTH_QVGA * AR_BYTES_PER_PIXEL)
#define MAX_AR_LINE_BYTES	AR_LINE_BYTES_VGA

/* frame size & type */
#define AR_FRAME_BYTES_VGA \
	(AR_WIDTH_VGA * AR_HEIGHT_VGA * AR_BYTES_PER_PIXEL)
#define AR_FRAME_BYTES_QVGA \
	(AR_WIDTH_QVGA * AR_HEIGHT_QVGA * AR_BYTES_PER_PIXEL)
#define MAX_AR_FRAME_BYTES \
	(MAX_AR_WIDTH * MAX_AR_HEIGHT * AR_BYTES_PER_PIXEL)

#define AR_MAX_FRAME		15

/* capture size */
#define AR_SIZE_VGA		0
#define AR_SIZE_QVGA		1

/* capture mode */
#define AR_MODE_INTERLACE	0
#define AR_MODE_NORMAL		1

struct ar_device {
	struct video_device *vdev;
	unsigned int start_capture;	/* duaring capture in INT. mode. */
#if USE_INT
	unsigned char *line_buff;	/* DMA line buffer */
#endif
	unsigned char *frame[MAX_AR_HEIGHT];	/* frame data */
	short size;			/* capture size */
	short mode;			/* capture mode */
	int width, height;
	int frame_bytes, line_bytes;
	wait_queue_head_t wait;
	unsigned long in_use;
	struct mutex lock;
};

static int video_nr = -1;	/* video device number (first free) */
static unsigned char	yuv[MAX_AR_FRAME_BYTES];

/* module parameters */
/* default frequency */
#define DEFAULT_FREQ	50	/* 50 or 75 (MHz) is available as BCLK */
static int freq = DEFAULT_FREQ;	/* BCLK: available 50 or 70 (MHz) */
static int vga;			/* default mode(0:QVGA mode, other:VGA mode) */
static int vga_interlace;	/* 0 is normal mode for, else interlace mode */
module_param(freq, int, 0);
module_param(vga, int, 0);
module_param(vga_interlace, int, 0);

static int ar_initialize(struct video_device *dev);

static inline void wait_for_vsync(void)
{
	while (ar_inl(ARVCR0) & ARVCR0_VDS)	/* wait for VSYNC */
		cpu_relax();
	while (!(ar_inl(ARVCR0) & ARVCR0_VDS))	/* wait for VSYNC */
		cpu_relax();
}

static inline void wait_acknowledge(void)
{
	int i;

	for (i = 0; i < 1000; i++)
		cpu_relax();
	while (ar_inl(PLDI2CSTS) & PLDI2CSTS_NOACK)
		cpu_relax();
}

/*******************************************************************
 * I2C functions
 *******************************************************************/
void iic(int n, unsigned long addr, unsigned long data1, unsigned long data2,
	 unsigned long data3)
{
	int i;

	/* Slave Address */
	ar_outl(addr, PLDI2CDATA);
	wait_for_vsync();

	/* Start */
	ar_outl(1, PLDI2CCND);
	wait_acknowledge();

	/* Transfer data 1 */
	ar_outl(data1, PLDI2CDATA);
	wait_for_vsync();
	ar_outl(PLDI2CSTEN_STEN, PLDI2CSTEN);
	wait_acknowledge();

	/* Transfer data 2 */
	ar_outl(data2, PLDI2CDATA);
	wait_for_vsync();
	ar_outl(PLDI2CSTEN_STEN, PLDI2CSTEN);
	wait_acknowledge();

	if (n == 3) {
		/* Transfer data 3 */
		ar_outl(data3, PLDI2CDATA);
		wait_for_vsync();
		ar_outl(PLDI2CSTEN_STEN, PLDI2CSTEN);
		wait_acknowledge();
	}

	/* Stop */
	for (i = 0; i < 100; i++)
		cpu_relax();
	ar_outl(2, PLDI2CCND);
	ar_outl(2, PLDI2CCND);

	while (ar_inl(PLDI2CSTS) & PLDI2CSTS_BB)
		cpu_relax();
}


void init_iic(void)
{
	DEBUG(1, "init_iic:\n");

	/*
	 * ICU Setting (iic)
	 */
	/* I2C Setting */
	ar_outl(0x0, PLDI2CCR);      	/* I2CCR Disable                   */
	ar_outl(0x0300, PLDI2CMOD); 	/* I2CMOD ACK/8b-data/7b-addr/auto */
	ar_outl(0x1, PLDI2CACK);	/* I2CACK ACK                      */

	/* I2C CLK */
	/* 50MH-100k */
	if (freq == 75) {
		ar_outl(369, PLDI2CFREQ);	/* BCLK = 75MHz */
	} else if (freq == 50) {
		ar_outl(244, PLDI2CFREQ);	/* BCLK = 50MHz */
	} else {
		ar_outl(244, PLDI2CFREQ);	/* default: BCLK = 50MHz */
	}
	ar_outl(0x1, PLDI2CCR); 	/* I2CCR Enable */
}

/**************************************************************************
 *
 * Video4Linux Interface functions
 *
 **************************************************************************/

static inline void disable_dma(void)
{
	ar_outl(0x8000, M32R_DMAEN_PORTL);	/* disable DMA0 */
}

static inline void enable_dma(void)
{
	ar_outl(0x8080, M32R_DMAEN_PORTL);	/* enable DMA0 */
}

static inline void clear_dma_status(void)
{
	ar_outl(0x8000, M32R_DMAEDET_PORTL);	/* clear status */
}

static inline void wait_for_vertical_sync(int exp_line)
{
#if CHECK_LOST
	int tmout = 10000;	/* FIXME */
	int l;

	/*
	 * check HCOUNT because we cannot check vertical sync.
	 */
	for (; tmout >= 0; tmout--) {
		l = ar_inl(ARVHCOUNT);
		if (l == exp_line)
			break;
	}
	if (tmout < 0)
		printk("arv: lost %d -> %d\n", exp_line, l);
#else
	while (ar_inl(ARVHCOUNT) != exp_line)
		cpu_relax();
#endif
}

static ssize_t ar_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct video_device *v = video_devdata(file);
	struct ar_device *ar = video_get_drvdata(v);
	long ret = ar->frame_bytes;		/* return read bytes */
	unsigned long arvcr1 = 0;
	unsigned long flags;
	unsigned char *p;
	int h, w;
	unsigned char *py, *pu, *pv;
#if ! USE_INT
	int l;
#endif

	DEBUG(1, "ar_read()\n");

	if (ar->size == AR_SIZE_QVGA)
		arvcr1 |= ARVCR1_QVGA;
	if (ar->mode == AR_MODE_NORMAL)
		arvcr1 |= ARVCR1_NORMAL;

	mutex_lock(&ar->lock);

#if USE_INT
	local_irq_save(flags);
	disable_dma();
	ar_outl(0xa1871300, M32R_DMA0CR0_PORTL);
	ar_outl(0x01000000, M32R_DMA0CR1_PORTL);

	/* set AR FIFO address as source(BSEL5) */
	ar_outl(ARDATA32, M32R_DMA0CSA_PORTL);
	ar_outl(ARDATA32, M32R_DMA0RSA_PORTL);
	ar_outl(ar->line_buff, M32R_DMA0CDA_PORTL);	/* destination addr. */
	ar_outl(ar->line_buff, M32R_DMA0RDA_PORTL); 	/* reload address */
	ar_outl(ar->line_bytes, M32R_DMA0CBCUT_PORTL); 	/* byte count (bytes) */
	ar_outl(ar->line_bytes, M32R_DMA0RBCUT_PORTL); 	/* reload count (bytes) */

	/*
	 * Okey , kicks AR LSI to invoke an interrupt
	 */
	ar->start_capture = 0;
	ar_outl(arvcr1 | ARVCR1_HIEN, ARVCR1);
	local_irq_restore(flags);
	/* .... AR interrupts .... */
	interruptible_sleep_on(&ar->wait);
	if (signal_pending(current)) {
		printk("arv: interrupted while get frame data.\n");
		ret = -EINTR;
		goto out_up;
	}
#else	/* ! USE_INT */
	/* polling */
	ar_outl(arvcr1, ARVCR1);
	disable_dma();
	ar_outl(0x8000, M32R_DMAEDET_PORTL);
	ar_outl(0xa0861300pe, err = -ENOMEM;
	int maj, min;
	dev_t rdev = 0;

	if (dentry) {
		name = dentry_name(dentry, 0);
		if (name == NULL)
			goto out;
		type = file_type(name, &maj, &min);
		/* Reencode maj and min with the kernel encoding.*/
		rdev = MKDEV(maj, min);
		kfree(name);
	}
	else type = OS_TYPE_DIR;

	err = 0;
	if (type == OS_TYPE_SYMLINK)
		inode->i_op = &page_symlink_inode_operations;
	else if (type == OS_TYPE_DIR)
		inode->i_op = &hostfs_dir_iops;
	else inode->i_op = &hostfs_iops;

	if (type == OS_TYPE_DIR) inode->i_fop = &hostfs_dir_fops;
	else inode->i_fop = &hostfs_file_fops;

	if (type == OS_TYPE_SYMLINK)
		inode->i_mapping->a_ops = &hostfs_link_aops;
	else inode->i_mapping->a_ops = &hostfs_aops;

	switch (type) {
	case OS_TYPE_CHARDEV:
		init_special_inode(inode, S_IFCHR, rdev);
		break;
	case OS_TYPE_BLOCKDEV:
		init_special_inode(inode, S_IFBLK, rdev);
		break;
	case OS_TYPE_FIFO:
		init_special_inode(inode, S_IFIFO, 0);
		break;
	case OS_TYPE_SOCK:
		init_special_inode(inode, S_IFSOCK, 0);
		break;
	}
 out:
	return err;
}

int hostfs_create(struct inode *dir, struct dentry *dentry, int mode,
		  struct nameidata *nd)
{
	struct inode *inode;
	char *name;
	int error, fd;

	inode = hostfs_iget(dir->i_sb);
	if (IS_ERR(inode)) {
		error = PTR_ERR(inode);
		goto out;
	}

	error = init_inode(inode, dentry);
	if (error)
		goto out_put;

	error = -ENOMEM;
	name = dentry_name(dentry, 0);
	if (name == NULL)
		goto out_put;

	fd = file_create(name,
			 mode & S_IRUSR, mode & S_IWUSR, mode & S_IXUSR,
			 mode & S_IRGRP, mode & S_IWGRP, mode & S_IXGRP,
			 mode & S_IROTH, mode & S_IWOTH, mode & S_IXOTH);
	if (fd < 0)
		error = fd;
	else error = read_name(inode, name);

	kfree(name);
	if (error)
		goto out_put;

	HOSTFS_I(inode)->fd = fd;
	HOSTFS_I(inode)->mode = FMODE_READ | FMODE_WRITE;
	d_instantiate(dentry, inode);
	return 0;

 out_put:
	iput(inode);
 out:
	return error;
}

struct dentry *hostfs_lookup(struct inode *ino, struct dentry *dentry,
			     struct nameidata *nd)
{
	struct inode *inode;
	char *name;
	int err;

	inode = hostfs_iget(ino->i_sb);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto out;
	}

	err = init_inode(inode, dentry);
	if (err)
		goto out_put;

	err = -ENOMEM;
	name = dentry_name(dentry, 0);
	if (name == NULL)
		goto out_put;

	err = read_name(inode, name);
	kfree(name);
	if (err == -ENOENT) {
		iput(inode);
		inode = NULL;
	}
	else if (err)
		goto out_put;

	d_add(dentry, inode);
	dentry->d_op = &hostfs_dentry_ops;
	return NULL;

 out_put:
	iput(inode);
 out:
	return ERR_PTR(err);
}

static char *inode_dentry_name(struct inode *ino, struct dentry *dentry)
{
	char *file;
	int len;

	file = inode_name(ino, dentry->d_name.len + 1);
	if (file == NULL)
		return NULL;
	strcat(file, "/");
	len = strlen(file);
	strncat(file, dentry->d_name.name, dentry->d_name.len);
	file[len + dentry->d_name.len] = '\0';
	return file;
}

int hostfs_link(struct dentry *to, struct inode *ino, struct dentry *from)
{
	char *from_name, *to_name;
	int err;

	if ((from_name = inode_dentry_name(ino, from)) == NULL)
		return -ENOMEM;
	to_name = dentry_name(to, 0);
	if (to_name == NULL) {
		kfree(from_name);
		return -ENOMEM;
	}
	err = link_file(to_name, from_name);
	kfree(from_name);
	kfree(to_name);
	return err;
}

int hostfs_unlink(struct inode *ino, struct dentry *dentry)
{
	char *file;
	int err;

	if ((file = inode_dentry_name(ino, dentry)) == NULL)
		return -ENOMEM;
	if (append)
		return -EPERM;

	err = unlink_file(file);
	kfree(file);
	return err;
}

int hostfs_symlink(struct inode *ino, struct dentry *dentry, const char *to)
{
	char *file;
	int err;

	if ((file = inode_dentry_name(ino, dentry)) == NULL)
		return -ENOMEM;
	err = make_symlink(file, to);
	kfree(file);
	return err;
}

int hostfs_mkdir(struct inode *ino, struct dentry *dentry, int mode)
{
	char *file;
	int err;

	if ((file = inode_dentry_name(ino, dentry)) == NULL)
		return -ENOMEM;
	err = do_mkdir(file, mode);
	kfree(file);
	return err;
}

int hostfs_rmdir(struct inode *ino, struct dentry *dentry)
{
	char *file;
	int err;

	if ((file = inode_dentry_name(ino, durn -EINVAL;
	case VIDIOCGAUDIO:
		DEBUG(1, "VIDIOCGAUDIO:\n");
		return -EINVAL;
	case VIDIOCSAUDIO:
		DEBUG(1, "VIDIOCSAUDIO:\n");
		return -EINVAL;
	case VIDIOCSYNC:
		DEBUG(1, "VIDIOCSYNC:\n");
		return -EINVAL;
	case VIDIOCMCAPTURE:
		DEBUG(1, "VIDIOCMCAPTURE:\n");
		return -EINVAL;
	case VIDIOCGMBUF:
		DEBUG(1, "VIDIOCGMBUF:\n");
		return -EINVAL;
	case VIDIOCGUNIT:
		DEBUG(1, "VIDIOCGUNIT:\n");
		return -EINVAL;
	case VIDIOCGCAPTURE:
		DEBUG(1, "VIDIOCGCAPTURE:\n");
		return -EINVAL;
	case VIDIOCSCAPTURE:
		DEBUG(1, "VIDIOCSCAPTURE:\n");
		return -EINVAL;
	case VIDIOCSPLAYMODE:
		DEBUG(1, "VIDIOCSPLAYMODE:\n");
		return -EINVAL;
	case VIDIOCSWRITEMODE:
		DEBUG(1, "VIDIOCSWRITEMODE:\n");
		return -EINVAL;
	case VIDIOCGPLAYINFO:
		DEBUG(1, "VIDIOCGPLAYINFO:\n");
		return -EINVAL;
	case VIDIOCSMICROCODE:
		DEBUG(1, "VIDIOCSMICROCODE:\n");
		return -EINVAL;
	case VIDIOCGVBIFMT:
		DEBUG(1, "VIDIOCGVBIFMT:\n");
		return -EINVAL;
	case VIDIOCSVBIFMT:
		DEBUG(1, "VIDIOCSVBIFMT:\n");
		return -EINVAL;
	default:
		DEBUG(1, "Unknown ioctl(0x%08x)\n", cmd);
		return -ENOIOCTLCMD;
	}
	return 0;
}

static long ar_ioctl(struct file *file, unsigned int cmd,
		    unsigned long arg)
{
	return video_usercopy(file, cmd, arg, ar_do_ioctl);
}

#if USE_INT
/*
 * Interrupt handler
 */
static void ar_interrupt(int irq, void *dev)
{
	struct ar_device *ar = dev;
	unsigned int line_count;
	unsigned int line_number;
	unsigned int arvcr1;

	line_count = ar_inl(ARVHCOUNT);			/* line number */
	if (ar->mode == AR_MODE_INTERLACE && ar->size == AR_SIZE_VGA) {
		/* operations for interlace mode */
		if ( line_count < (AR_HEIGHT_VGA/2) ) 	/* even line */
			line_number = (line_count << 1);
		else 					/* odd line */
			line_number =
			(((line_count - (AR_HEIGHT_VGA/2)) << 1) + 1);
	} else {
		line_number = line_count;
	}

	if (line_number == 0) {
		/*
		 * It is an interrupt for line 0.
		 * we have to start capture.
		 */
		disable_dma();
#if 0
		ar_outl(ar->line_buff, M32R_DMA0CDA_PORTL);	/* needless? */
#endif
		memcpy(ar->frame[0], ar->line_buff, ar->line_bytes);
#if 0
		ar_outl(0xa1861300, M32R_DMA0CR0_PORTL);
#endif
		enable_dma();
		ar->start_capture = 1;			/* during capture */
		return;
	}

	if (ar->start_capture == 1 && line_number <= (ar->height - 1)) {
		disable_dma();
		memcpy(ar->frame[line_number], ar->line_buff, ar->line_bytes);

		/*
		 * if captured all line of a frame, disable AR interrupt
		 * and wake a process up.
		 */
		if (line_number == (ar->height - 1)) { 	/* end  of line */

			ar->start_capture = 0;

			/* disable AR interrupt request */
			arvcr1 = ar_inl(ARVCR1);
			arvcr1 &= ~ARVCR1_HIEN;		/* clear int. flag */
			ar_outl(arvcr1, ARVCR1);	/* disable */
			wake_up_interruptible(&ar->wait);
		} else {
#if 0
			ar_outl(ar->line_buff, M32R_DMA0CDA_PORTL);
			ar_outl(0xa1861300, M32R_DMA0CR0_PORTL);
#endif
			enable_dma();
		}
	}
}
#endif

/*
 * ar_initialize()
 * 	ar_initialize() is called by video_register_device() and
 *	initializes AR LSI and peripherals.
 *
 *	-1 is returned in all failures.
 *	0 is returned in success.
 *
 */
static int ar_initialize(struct video_device *dev)
{
	struct ar_device *ar = video_get_drvdata(dev);
	unsigned long cr = 0;
	int i,found=0;

	DEBUG(1, "ar_initialize:\n");

	/*
	 * initialize AR LSI
	 */
	ar_outl(0, ARVCR0);		/* assert reset of AR LSI */
	for (i = 0; i < 0x18; i++)	/* wait for over 10 cycles @ 27MHz */
		cpu_relax();
	ar_outl(ARVCR0_RST, ARVCR0);	/* negate reset of AR LSI (enable) */
	for (i = 0; i < 0x40d; i++)	/* wait for over 420 cycles @ 27MHz */
		cpu_relax();

	/* AR uses INT3 of CPU as interrupt pin. */
	ar_outl(ARINTSEL_INT3, ARINTSEL);

	if (ar->size == AR_SIZE_QVGA)
		cr |= ARVCR1_QVGA;
	if (ar->mode == AR_MODE_NORMAL)
		cr |= ARVCR1_NORMAL;
	ar_outl(cr, ARVCR1);

	/*
	 * Initialize IIC so that CPU can communicate with AR LSI,
	 * and send boot commands to AR LSI.
	 */
	init_iic();

	for (i = 0; i < 0x100000; i++) {	/* > 0xa1d10,  56ms */
		if ((ar_inl(ARVCR0) & ARVCR0_VDS)) {	/* VSYNC */
			found = 1;
			break;
		}
	}

	if (found == 0)
		return -ENODEV;

	printk("arv: Initializing ");

	iic(2,0x78,0x11,0x01,0x00);	/* start */
	iic(3,0x78,0x12,0x00,0x06);
	iic(3,0x78,0x12,0x12,0x30);
	iic(3,0x78,0x12,0x15,0x58);
	iic(3,0x78,0x12,0x17,0x30);
	printk(".");
	iic(3,0x78,0x12,0x1a,0x97);
	iic(3,0x78,0x12,0x1b,0xff);
	iic(3,0x78,0x12,0x1c,0xff);
	iic(3,0x78,0x12,0x26,0x10);
	iic(3,0x78,0x12,0x27,0x00);
	printk(".");
	iic(2,0x78,0x34,0x02,0x00);
	iic(2,0x78,0x7a,0x10,0x00);
	iic(2,0x78,0x80,0x39,0x00);
	iic(2,0x78,0x81,0xe6,0x00);
	iic(2,0x78,0x8d,0x00,0x00);
	printk(".");
	iic(2,0x78,0x8e,0x0c,0x00);
	iic(2,0x78,0x8f,0x00,0x00);
#if 0
	iic(2,0x78,0x90,0x00,0x00);	/* AWB on=1 off=0 */
#endif
	iic(2,0x78,0x93,0x01,0x00);
	iic(2,0x78,0x94,0xcd,0x00);
	iic(2,0x78,0x95,0x00,0x00);
	printk(".");
	iic(2,0x78,0x96,0xa0,0x00);
	iic(2,0x78,0x97,0x00,0x00);
	iic(2,0x78,0x98,0x60,0x00);
	iic(2,0x78,0x99,0x01,0x00);
	iic(2,0x78,0x9a,0x19,0x00);
	printk(".");
	iic(2,0x78,0x9b,0x02,0x00);
	iic(2,0x78,0x9c,0xe8,0x00);
	iic(2,0x78,0x9d,0x02,0x00);
	iic(2,0x78,0x9e,0x2e,0x00);
	iic(2,0x78,0xb8,0x78,0x00);
	iic(2,0x78,0xba,0x05,0x00);
#if 0
	iic(2,0x78,0x83,0x8c,0x00);	/* brightness */
#endif
	printk(".");

	/* color correction */
	iic(3,0x78,0x49,0x00,0x95);	/* a		*/
	iic(3,0x78,0x49,0x01,0x96);	/* b		*/
	iic(3,0x78,0x49,0x03,0x85);	/* c		*/
	iic(3,0x78,0x49,0x04,0x97);	/* d		*/
	iic(3,0x78,0x49,0x02,0x7e);	/* e(Lo)	*/
	iic(3,0x78,0x49,0x05,0xa4);	/* f(Lo)	*/
	iic(3,0x78,0x49,0x06,0x04);	/* e(Hi)	*/
	iic(3,0x78,0x49,0x07,0x04);	/* e(Hi)	*/
	iic(2,0x78,0x48,0x01,0x00);	/* on=1 off=0	*/

	printk(".");
	iic(2,0x78,0x11,0x00,0x00);	/* end */
	printk(" done\n");
	return 0;
}


void ar_release(struct video_device *vfd)
{
	struct ar_device *ar = video_get_drvdata(vfd);
	mutex_lock(&ar->lock);
	video_device_release(vfd);
}

/****************************************************************************
 *
 * Video4Linux Module functions
 *
 ****************************************************************************/
static struct ar_device ardev;

static int ar_exclusive_open(struct file *file)
{
	return test_and_set_bit(0, &ardev.in_use) ? -EBUSY : 0;
}

static int ar_exclusive_release(struct file *file)
{
	clear_bit(0, &ardev.in_use);
	return 0;
}

static const struct v4l2_file_operations ar_fops = {
	.owner		= THIS_MODULE,
	.ope
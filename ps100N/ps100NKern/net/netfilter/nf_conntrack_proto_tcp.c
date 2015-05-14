ESOURCE_MEM, 1))
		dma_release_declared_memory(&pdev->dev);
exit_iounmap:
	iounmap(base);
exit_kfree:
	kfree(pcdev);
exit:
	return err;
}

static int sh_mobile_ceu_remove(struct platform_device *pdev)
{
	struct soc_camera_host *soc_host = to_soc_camera_host(&pdev->dev);
	struct sh_mobile_ceu_dev *pcdev = container_of(soc_host,
					struct sh_mobile_ceu_dev, ici);

	soc_camera_host_unregister(soc_host);
	clk_put(pcdev->clk);
	free_irq(pcdev->irq, pcdev);
	if (platform_get_resource(pdev, IORESOURCE_MEM, 1))
		dma_release_declared_memory(&pdev->dev);
	iounmap(pcdev->base);
	kfree(pcdev);
	return 0;
}

static struct platform_driver sh_mobile_ceu_driver = {
	.driver 	= {
		.name	= "sh_mobile_ceu",
	},
	.probe		= sh_mobile_ceu_probe,
	.remove		= sh_mobile_ceu_remove,
};

static int __init sh_mobile_ceu_init(void)
{
	return platform_driver_register(&sh_mobile_ceu_driver);
}

static void __exit sh_mobile_ceu_exit(void)
{
	platform_driver_unregister(&sh_mobile_ceu_driver);
}

module_init(sh_mobile_ceu_init);
module_exit(sh_mobile_ceu_exit);

MODULE_DESCRIPTION("SuperH Mobile CEU driver");
MODULE_AUTHOR("Magnus Damm");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   or valid
 *	and the receiver may send back a connection
 *	closing RST or a SYN/ACK.
 *
 * Packets marked as INVALID (sIV):
 *	if we regard them as truly invalid packets
 */
static const u8 tcp_conntracks[2][6][TCP_CONNTRACK_MAX] = {
	{
/* ORIGINAL */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*syn*/	   { sSS, sSS, sIG, sIG, sIG, sIG, sIG, sSS, sSS, sS2 },
/*
 *	sNO -> sSS	Initialize a new connection
 *	sSS -> sSS	Retransmitted SYN
 *	sS2 -> sS2	Late retransmitted SYN
 *	sSR -> sIG
 *	sES -> sIG	Error: SYNs in window outside the SYN_SENT state
 *			are errors. Receiver will reply with RST
 *			and close the connection.
 *			Or we are not in sync and hold a dead connection.
 *	sFW -> sIG
 *	sCW -> sIG
 *	sLA -> sIG
 *	sTW -> sSS	Reopened connection (RFC 1122).
 *	sCL -> sSS
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*synack*/ { sIV, sIV, sIG, sIG, sIG, sIG, sIG, sIG, sIG, sSR },
/*
 *	sNO -> sIV	Too late and no reason to do anything
 *	sSS -> sIV	Client can't send SYN and then SYN/ACK
 *	sS2 -> sSR	SYN/ACK sent to SYN2 in simultaneous open
 *	sSR -> sIG
 *	sES -> sIG	Error: SYNs in window outside the SYN_SENT state
 *			are errors. Receiver will reply with RST
 *			and close the connection.
 *			Or we are not in sync and hold a dead connection.
 *	sFW -> sIG
 *	sCW -> sIG
 *	sLA -> sIG
 *	sTW -> sIG
 *	sCL -> sIG
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*fin*/    { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },
/*
 *	sNO -> sIV	Too late and no reason to do anything...
 *	sSS -> sIV	Client migth not send FIN in this state:
 *			we enforce waiting for a SYN/ACK reply first.
 *	sS2 -> sIV
 *	sSR -> sFW	Close started.
 *	sES -> sFW
 *	sFW -> sLA	FIN seen in both directions, waiting for
 *			the last ACK.
 *			Migth be a retransmitted FIN as well...
 *	sCW -> sLA
 *	sLA -> sLA	Retransmitted FIN. Remain in the same state.
 *	sTW -> sTW
 *	sCL -> sCL
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*ack*/	   { sES, sIV, sES, sES, sCW, sCW, sTW, sTW, sCL, sIV },
/*
 *	sNO -> sES	Assumed.
 *	sSS -> sIV	ACK is invalid: we haven't seen a SYN/ACK yet.
 *	sS2 -> sIV
 *	sSR -> sES	Established state is reached.
 *	sES -> sES	:-)
 *	sFW -> sCW	Normal close request answered by ACK.
 *	sCW -> sCW
 *	sLA -> sTW	Last ACK detected.
 *	sTW -> sTW	Retransmitted last ACK. Remain in the same state.
 *	sCL -> sCL
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*rst*/    { sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL },
/*none*/   { sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV }
	},
	{
/* REPLY */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*syn*/	   { sIV, sS2, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sS2 },
/*
 *	sNO -> sIV	Never reached.
 *	sSS -> sS2	Simultaneous open
 *	sS2 -> sS2	Retransmitted simultaneous SYN
 *	sSR -> sIV	Invalid SYN packets sent by the server
 *	sES -> sIV
 *	sFW -> sIV
 *	sCW -> sIV
 *	sLA -> sIV
 *	sTW -> sIV	Reopened connection, but server may not do it.
 *	sCL -> sIV
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*synack*/ { sIV, sSR, sSR, sIG, sIG, sIG, sIG, sIG, sIG, sSR },
/*
 *	sSS -> sSR	Standard open.
 *	sS2 -> sSR	Simultaneous open
 *	sSR -> sSR	Retransmitted SYN/ACK.
 *	sES -> sIG	Late retransmitted SYN/ACK?
 *	sFW -> sIG	Might be SYN/ACK answering ignored SYN
 *	sCW -> sIG
 *	sLA -> sIG
 *	sTW -> sIG
 *	sCL -> sIG
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*fin*/    { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },
/*
 *	sSS -> sIV	Server might not send FIN in this state.
 *	sS2 -> sIV
 *	sSR -> sFW	Close started.
 *	sES -> sFW
 *	sFW -> sLA	FIN seen in both directions.
 *	sCW -> sLA
 *	sLA -> sLA	Retransmitted FIN.
 *	sTW -> sTW
 *	sCL -> sCL
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*ack*/	   { sIV, sIG, sSR, sES, sCW, sCW, sTW, sTW, sCL, sIG },
/*
 *	sSS -> sIG	Might be a half-open connection.
 *	sS2 -> sIG
 *	sSR -> sSR	Might answer late resent SYN.
 *	sES -> sES	:-)
 *	sFW -> sCW	Normal close request answered b     4096
#define MAX_DMA_SIZE                (1024 * 4096)
#define SAREA_SIZE                  4096
#define PCI_I810_MISCC              0x72
#define MMIO_SIZE                   (512*1024)
#define GTT_SIZE                    (16*1024) 
#define RINGBUFFER_SIZE             (64*1024)
#define CURSOR_SIZE                 4096 
#define OFF                         0
#define ON                          1
#define MAX_KEY                     256
#define WAIT_COUNT                  10000000
#define IRING_PAD                   8
#define FONTDATAMAX                 8192
/* Masks (AND ops) and OR's */
#define FB_START_MASK               (0x3f << (32 - 6))
#define MMIO_ADDR_MASK              (0x1FFF << (32 - 13))
#define FREQ_MASK                   (1 << 4)
#define SCR_OFF                     0x20
#define DRAM_ON                     0x08            
#define DRAM_OFF                    0xE7
#define PG_ENABLE_MASK              0x01
#define RING_SIZE_MASK              (RINGBUFFER_SIZE - 1);

/* defines for restoring registers partially */
#define ADDR_MAP_MASK               (0x07 << 5)
#define DISP_CTRL                   ~0
#define PIXCONF_0                   (0x64 << 8)
#define PIXCONF_2                   (0xF3 << 24)
#define PIXCONF_1                   (0xF0 << 16)
#define MN_MASK                     0x3FF03FF
#define P_OR                        (0x7 << 4)                    
#define DAC_BIT                     (1 << 16)
#define INTERLACE_BIT               (1 << 7)
#define IER_MASK                    (3 << 13)
#define IMR_MASK                    (3 << 13)

/* Power Management */
#define DPMS_MASK                   0xF0000
#define POWERON                     0x00000
#define STANDBY                     0x20000
#define SUSPEND                     0x80000
#define POWERDOWN                   0xA0000
#define EMR_MASK                    ~0x3F
#define FW_BLC_MASK                 ~(0x3F|(7 << 8)|(0x3F << 12)|(7 << 20))

/* Ringbuffer */
#define RBUFFER_START_MASK          0xFFFFF000
#define RBUFFER_SIZE_MASK           0x001FF000
#define RBUFFER_HEAD_MASK           0x001FFFFC
#define RBUFFER_TAIL_MASK           0x001FFFF8

/* Video Timings */
#define REF_FREQ                    24000000
#define TARGET_N_MAX                30

#define MAX_PIXELCLOCK              230000000
#define MIN_PIXELCLOCK               15000000
#define VFMAX                       60
#define VFMIN                       60
#define HFMAX                       30000
#define HFMIN                       29000

/* Cursor */
#define CURSOR_ENABLE_MASK          0x1000             
#define CURSOR_MODE_64_TRANS        4
#define CURSOR_MODE_64_XOR	    5
#define CURSOR_MODE_64_3C	    6	
#define COORD_INACTIVE              0
#define COORD_ACTIVE                (1 << 4)
#define EXTENDED_PALETTE	    1
  
/* AGP Memory Types*/
#define AGP_NORMAL_MEMORY           0
#define AGP_DCACHE_MEMORY	    1
#define AGP_PHYSICAL_MEMORY         2

/* Allocated resource Flags */
#define FRAMEBUFFER_REQ             1
#define MMIO_REQ                    2
#define PCI_DEVICE_ENABLED          4
#define HAS_FONTCACHE               8 

/* driver flags */
#define HAS_MTRR                    1
#define HAS_ACCELERATION            2
#define ALWAYS_SYNC                 4
#define LOCKUP                      8

struct gtt_data {
	struct agp_memory *i810_fb_memory;
	struct agp_memory *i810_cursor_memory;
};

struct mode_registers {
	u32 pixclock, M, N, P;
	u8 cr00, cr01, cr02, cr03;
	u8 cr04, cr05, cr06, cr07;
	u8 cr09, cr10, cr11, cr12;
	u8 cr13, cr15, cr16, cr30;
	u8 cr31, cr32, cr33, cr35, cr39;
	u32 bpp8_100, bpp16_100;
	u32 bpp24_100, bpp8_133;
	u32 bpp16_133, bpp24_133;
	u8 msr;
};

struct heap_data {
        unsigned long physical;
	__u8 __iomem *virtual;
	u32 offset;
	u32 size;
};	

struct state_registers {
	u32 dclk_1d, dclk_2d, dclk_0ds;
	u32 pixconf, fw_blc, pgtbl_ctl;
	u32 fence0, hws_pga, dplystas;
	u16 bltcntl, hwstam, ier, iir, imr;
	u8 cr00, cr01, cr02, cr03, cr04;
	u8 cr05, cr06, cr07, cr08, cr09;
	u8 cr10, cr11, cr12, cr13, cr14;
	u8 cr15, cr16, cr17, cr80, gr10;
	u8 cr30, cr31, cr32, cr33, cr35;
	u8 cr39, cr41, cr70, sr01, msr;
};

struct i810fb_par;

struct i810fb_i2c_chan {
	struct i810fb_par *par;
	struct i2c_adapter adapter;
	struct i2c_algo_bit_data algo;
	unsigned long ddc_base;
};

struct i810fb_par {
	struct mode_registers    regs;
	struct state_registers   hw_state;
	struct gtt_data          i810_gtt;
	struct fb_ops            i810fb_ops;
	struct pci_dev           *dev;
	struct heap_data         aperture;
	struct heap_data         fb;
	struct heap_data         iring;
	struct heap_data         cursor_heap;
	struct vgastate          state;
	struct i810fb_i2c_chan   chan[3];
	struct mutex		 open_lock;
	unsigned int		 use_count;
	u32 pseudo_palette[16];
	unsigned long mmio_start_phys;
	u8 __iomem *mmio_start_virtual;
	u8 *edid;
	u32 pitch;
	u32 pixconf;
	u32 watermark;
	u32 mem_freq;
	u32 res_flags;
	u32 dev_flags;
	u32 cur_tail;
	u32 depth;
	u32 blit_bpp;
	u32 ovract;
	u32 cur_state;
	u32 ddc_num;
	int mtrr_reg;
	u16 bltcntl;
	u8 interlace;
};

/* 
 * Register I/O
 */
#define i810_readb(where, mmio) readb(mmio + where)
#define i810_readw(where, mmio) readw(mmio + where)
#define i810_readl(where, mmio) readl(mmio + where)
#define i810_writeb(where, mmio, val) writeb(val, mmio + where) 
#define i810_writew(where, mmio, val) writew(val, mmio + where)
#define i810_writel(where, mmio, val) writel(val, mmio + where)

#endif /* __I810_H__ */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         /*
 * scsicam.c - SCSI CAM support functions, use for HDIO_GETGEO, etc.
 *
 * Copyright 1993, 1994 Drew Eckhardt
 *      Visionary Computing 
 *      (Unix and Linux consulting and custom programming)
 *      drew@Colorado.EDU
 *      +1 (303) 786-7975
 *
 * For more information, please consult the SCSI-CAM draft.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <asm/unaligned.h>

#include <scsi/scsicam.h>


static int setsize(unsigned long capacity, unsigned int *cyls, unsigned int *hds,
		   unsigned int *secs);

/**
 * scsi_bios_ptable - Read PC partition table out of first sector of device.
 * @dev: from this device
 *
 * Description: Reads the first sector from the device and returns %0x42 bytes
 *              starting at offset %0x1be.
 * Returns: partition table in kmalloc(GFP_KERNEL) memory, or NULL on error.
 */
unsigned char *scsi_bios_ptable(struct block_device *dev)
{
	unsigned char *res = kmalloc(66, GFP_KERNEL);
	if (res) {
		struct block_device *bdev = dev->bd_contains;
		Sector sect;
		void *data = read_dev_sector(bdev, 0, &sect);
		if (data) {
			memcpy(res, data + 0x1be, 66);
			put_dev_sector(sect);
		} else {
			kfree(res);
			res = NULL;
		}
	}
	return res;
}
EXPORT_SYMBOL(scsi_bios_ptable);

/**
 * scsicam_bios_param - Determine geometry of a disk in cylinders/heads/sectors.
 * @bdev: which device
 * @capacity: size of the disk in sectors
 * @ip: return value: ip[0]=heads, ip[1]=sectors, ip[2]=cylinders
 *
 * Description : determine the BIOS mapping/geometry used for a drive in a
 *      SCSI-CAM system, storing the results in ip as required
 *      by the HDIO_GETGEO ioctl().
 *
 * Returns : -1 on failure, 0 on success.
 */

int scsicam_bios_param(struct block_device *bdev, sector_t capacity, int *ip)
{
	unsigned char *p;
	u64 capacity64 = capacity;	/* Suppress gcc warning */
	int ret;

	p = scsi_bios_ptable(bdev);
	if (!p)
		return -1;

	/* try to infer mapping from partition table */
	ret = scsi_partsize(p, (unsigned long)capacity, (unsigned int *)ip + 2,
			       (unsigned int *)ip + 0, (unsigned int *)ip + 1);
	kfree(p);

	if (ret == -1 && capacity64 < (1ULL << 32)) {
		/* pick some standard mapping with at most 1024 cylinders,
		   and at most 62 sectors per track - this works up to
		   7905 MB */
		ret = setsize((unsigned long)capacity, (unsigned int *)ip + 2,
		       (unsigned int *)ip + 0, (unsigned int *)ip + 1);
	}

	/* if something went wrong, then apparently we have to return
	   a geometry with more than 1024 cylinders */
	if (ret || ip[0] > 255 || ip[1] > 63) {
		if ((capacity >> 11) > 65534) {
			ip[0] = 255;
			ip[1] = 63;
		} else {
			ip[0] = 64;
			ip[1] = 32;
		}

		if (capacity > 65535*63*255)
			ip[2] = 65535;
		else
			ip[2] = (unsigned long)capacity / (ip[0] * ip[1]);
	}

	return 0;
}
EXPORT_SYMBOL(scsicam_bios_param);

/**
 * scsi_partsize - Parse cylinders/heads/sectors from PC partition table
 * @buf: partition table, see scsi_bios_ptable()
 * @capacity: size of the disk in sectors
 * @cyls: put cylinders here
 * @hds: put heads here
 * @secs: put sectors here
 *
 * Description: determine the BIOS mapping/geometry used to create the partition
 *      table, storing the results in *cyls, *hds, and *secs 
 *
 * Returns: -1 on failure, 0 on success.
 */

int scsi_partsize(unsigned char *buf, unsigned long capacity,
	       unsigned int *cyls, unsigned int *hds, unsigned int *secs)
{
	struct partition *p = (struct partition *)buf, *largest = NULL;
	int i, largest_cyl;
	int cyl, ext_cyl, end_head, end_cyl, end_sector;
	unsigned int logical_end, physical_end, ext_physical_end;


	if (*(unsigned short *) (buf + 64) == 0xAA55) {
		for (largest_cyl = -1, i = 0; i < 4; ++i, ++p) {
			if (!p->sys_ind)
				continue;
#ifdef DEBUG
			printk("scsicam_bios_param : partition %d has system \n",
			       i);
#endif
			cyl = p->cyl + ((p->sector & 0xc0) << 2);
			if (cyl > largest_cyl) {
				largest_cyl = cyl;
				largest = p;
			}
		}
	}
	if (largest) {
		end_cyl = largest->end_cyl + ((largest->end_sector & 0xc0) << 2);
		end_head = largest->end_head;
		end_sector = largest->end_sector & 0x3f;

		if (end_head + 1 == 0 || end_sector == 0)
			return -1;

#ifdef DEBUG
		printk("scsicam_bios_param : end at h = %d, c = %d, s = %d\n",
		       end_head, end_cyl, end_sector);
#endif

		physical_end = end_cyl * (end_head + 1) * end_sector +
		    end_head * end_sector + end_sector;

		/* This is the actual _sector_ number at the end */
		logical_end = get_unaligned(&largest->start_sect)
		    + get_unaligned(&largest->nr_sects);

		/* This is for >1023 cylinders */
		ext_cyl = (logical_end - (end_head * end_sector + end_sector))
		    / (end_head + 1) / end_sector;
		ext_physical_end = ext_cyl * (end_head + 1) * end_sector +
		    end_head * end_sector + end_sector;

#ifdef DEBUG
		printk("scsicam_bios_param : logical_end=%d physical_end=%d ext_physical_end=%d ext_cyl=%d\n"
		  ,logical_end, physical_end, ext_physical_end, ext_cyl);
#endif

		if ((logical_end == physical_end) ||
		  (end_cyl == 1023 && ext_physical_end == logical_end)) {
			*secs = end_sector;
			*hds = end_head + 1;
			*cyls = capacity / ((end_head + 1) * end_sector);
			return 0;
		}
#ifdef DEBUG
		printk("scsicam_bios_param : logical (%u) != physical (%u)\n",
		       logical_end, physical_end);
#endif
	}
	return -1;
}
EXPORT_SYMBOL(scsi_partsize);

/*
 * Function : static int setsize(unsigned long capacity,unsigned int *cyls,
 *      unsigned int *hds, unsigned int *secs);
 *
 * Purpose : to determine a near-optimal int 0x13 mapping for a
 *      SCSI disk in terms of lost space of size capacity, storing
 *      the results in *cyls, *hds, and *secs.
 *
 * Returns : -1 on failure, 0 on success.
 *
 * Extracted from
 *
 * WORKING                                                    X3T9.2
 * DRAFT                                                        792D
 * see http://www.t10.org/ftp/t10/drafts/cam/cam-r12b.pdf
 *
 *                                                        Revision 6
 *                                                         10-MAR-94
 * Information technology -
 * SCSI-2 Common access method
 * transport and SCSI interface module
 * 
 * ANNEX A :
 *
 * setsize() converts a read capacity value to int 13h
 * head-cylinder-sector requirements. It minimizes the value for
 * number of heads and maximizes the number of cylinders. This
 * will support rather large disks before the number of heads
 * will not fit in 4 bits (or 6 bits). This algorithm also
 * minimizes the number of sectors that will be unused at the end
 * of the disk while allowing for very large disks to be
 * accommodated. This algorithm does not use physical geometry. 
 */

static int setsize(unsigned long capacity, unsigned int *cyls, unsigned int *hds,
		   unsigned int *secs)
{
	unsigned int rv = 0;
	unsigned long heads, sectors, cylinders, temp;

	cylinders = 1024L;	/* Set number of cylinders to max */
	sectors = 62L;		/* Maximize sectors per track */

	temp = cylinders * sectors;	/* Compute divisor for heads */
	heads = capacity / temp;	/* Compute value for number of heads */
	if (capacity % temp) {	/* If no remainder, done! */
		heads++;	/* Else, increment number of heads */
		temp = cylinders * heads;	/* Compute divisor for sectors */
		sectors = capacity / temp;	/* Compute value for sectors per
						   track */
		if (capacity % temp) {	/* If no remainder, done! */
			sectors++;	/* Else, increment number of sectors */
			temp = heads * sectors;		/* Compute divisor for cylinders */
			cylinders = capacity / temp;	/* Compute number of cylinders */
		}
	}
	if (cylinders == 0)
		rv = (unsigned) -1;	/* Give error if 0 cylinders */

	*cyls = (unsigned int) cylinders;	/* Stuff return values */
	*secs = (unsigned int) sectors;
	*hds = (unsigned int) heads;
	return (rv);
}
                                                                                                                                                                                                                                                                                   /*
 * WUSB Wire Adapter: WLP interface
 * Driver for the Linux Network stack.
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
 * Implementation of the netdevice linkage (except tx and rx related stuff).
 *
 * ROADMAP:
 *
 *   ENTRY POINTS (Net device):
 *
 *     i1480u_open(): Called when we ifconfig up the interface;
 *                    associates to a UWB host controller, reserves
 *                    bandwidth (MAS), sets up RX USB URB and starts
 *                    the queue.
 *
 *     i1480u_stop(): Called when we ifconfig down a interface;
 *                    reverses _open().
 *
 *     i1480u_set_config():
 */

#include <linux/if_arp.h>
#include <linux/etherdevice.h>

#include "i1480u-wlp.h"

struct i1480u_cmd_set_ip_mas {
	struct uwb_rccb     rccb;
	struct uwb_dev_addr addr;
	u8                  stream;
	u8                  owner;
	u8                  type;	/* enum uwb_drp_type */
	u8                  baMAS[32];
} __attribute__((packed));


static
int i1480u_set_ip_mas(
	struct uwb_rc *rc,
	const struct uwb_dev_addr *dstaddr,
	u8 stream, u8 owner, u8 type, unsigned long *mas)
{

	int result;
	struct i1480u_cmd_set_ip_mas *cmd;
	struct uwb_rc_evt_confirm reply;

	result = -ENOMEM;
	cmd = kzalloc(sizeof(*cmd), GFP_KERNEL);
	if (cmd == NULL)
		goto error_kzalloc;
	cmd->rccb.bCommandType = 0xfd;
	cmd->rccb.wCommand = cpu_to_le16(0x000e);
	cmd->addr = *dstaddr;
	cmd->stream = stream;
	cmd->owner = owner;
	cmd->type = type;
	if (mas == NULL)
		memset(cmd->baMAS, 0x00, sizeof(cmd->baMAS));
	else
		memcpy(cmd->baMAS, mas, sizeof(cmd->baMAS));
	reply.rceb.bEventType = 0xfd;
	reply.rceb.wEvent = cpu_to_le16(0x000e);
	result = uwb_rc_cmd(rc, "SET-IP-MAS", &cmd->rccb, sizeof(*cmd),
			    &reply.rceb, sizeof(reply));
	if (result < 0)
		goto error_cmd;
	if (reply.bResultCode != UWB_RC_RES_FAIL) {
		dev_err(&rc->uwb_dev.dev,
			"SET-IP-MAS: command execution failed: %d\n",
			reply.bResultCode);
		result = -EIO;
	}
error_cmd:
	kfree(cmd);
error_kzalloc:
	return result;
}

/*
 * Inform a WLP interface of a MAS reservation
 *
 * @rc is assumed refcnted.
 */
/* FIXME: detect if remote device is WLP capable? */
static int i1480u_mas_set_dev(struct uwb_dev *uwb_dev, struct uwb_rc *rc,
			      u8 stream, u8 owner, u8 type, unsigned long *mas)
{
	int result = 0;
	struct device *dev = &rc->uwb_dev.dev;

	result = i1480u_set_ip_mas(rc, &uwb_dev->dev_addr, stream, owner,
				   type, mas);
	if (result < 0) {
		char rcaddrbuf[UWB_ADDR_STRSIZE], devaddrbuf[UWB_ADDR_STRSIZE];
		uwb_dev_addr_print(rcaddrbuf, sizeof(rcaddrbuf),
				   &rc->uwb_dev.dev_addr);
		uwb_dev_addr_print(devaddrbuf, sizeof(devaddrbuf),
				   &uwb_dev->dev_addr);
		dev_err(dev, "Set IP MAS (%s to %s) failed: %d\n",
			rcaddrbuf, devaddrbuf, result);
	}
	return result;
}

/**
 * Called by bandwidth allocator when change occurs in reservation.
 *
 * @rsv:     The reservation that is being established, modified, or
 *           terminated.
 *
 * When a reservation is established, modified, or terminated the upper layer
 * (WLP here) needs set/update the currently available Media Access Slots
 * that can be use for IP traffic.
 *
 * Our action taken during failure depends on how the reservation is being
 * changed:
 * - if reservation is being established we do nothing if we cannot set the
 *   new MAS to be used
 * - if reservation is being terminated we revert back to PCA whether the
 *   SET IP MAS command succeeds or not.
 */
void i1480u_bw_alloc_cb(struct uwb_rsv *rsv)
{
	int result = 0;
	struct i1480u *i1480u = rsv->pal_priv;
	struct device *dev = &i1480u->usb_iface->dev;
	struct uwb_dev *target_dev = rsv->target.dev;
	struct uwb_rc *rc = i1480u->wlp.rc;
	u8 stream = rsv->stream;
	int type = rsv->type;
	int is_owner = rsv->owner == &rc->uwb_dev;
	unsigned long *bmp = rsv->mas.bm;

	dev_err(dev, "WLP callback called - sending set ip mas\n");
	/*user cannot change options while setting configuration*/
	mutex_lock(&i1480u->options.mutex);
	switch (rsv->state) {
	case UWB_RSV_STATE_T_ACCEPTED:
	case UWB_RSV_STATE_O_ESTABLISHED:
		result = i1480u_mas_set_dev(target_dev, rc, stream, is_owner,
					type, bmp);
		if (result < 0) {
			dev_err(dev, "MAS reservation failed: %d\n", result);
			goto out;
		}
		if (is_owner) {
			wlp_tx_hdr_set_delivery_id_type(&i1480u->options.def_tx_hdr,
							WLP_DRP | stream);
			wlp_tx_hdr_set_rts_cts(&i1480u->options.def_tx_hdr, 0);
		}
		break;
	case UWB_RSV_STATE_NONE:
		/* revert back to PCA */
		result = i1480u_mas_set_dev(target_dev, rc, stream, is_owner,
					    type, bmp);
		if (result < 0)
			dev_err(dev, "MAS reservation failed: %d\n", result);
		/* Revert to PCA even though SET IP MAS failed. */
		wlp_tx_hdr_set_delivery_id_type(&i1480u->options.def_tx_hdr,
						i1480u->options.pca_base_priority);
		wlp_tx_hdr_set_rts_cts(&i1480u->options.def_tx_hdr, 1);
		break;
	default:
		dev_err(dev, "unexpected WLP reservation state: %s (%d).\n",
			uwb_rsv_state_str(rsv->state), rsv->state);
		break;
	}
out:
	mutex_unlock(&i1480u->options.mutex);
	return;
}

/**
 *
 * Called on 'ifconfig up'
 */
int i1480u_open(struct net_device *net_dev)
{
	int result;
	struct i1480u *i1480u = netdev_priv(net_dev);
	struct wlp *wlp = &i1480u->wlp;
	struct uwb_rc *rc;
	struct device *dev = &i1480u->usb_iface->dev;

	rc = wlp->rc;
	result = i1480u_rx_setup(i1480u);		/* Alloc RX stuff */
	if (result < 0)
		goto error_rx_setup;

	result = uwb_radio_start(&wlp->pal);
	if (result < 0)
		goto error_radio_start;

	netif_wake_queue(net_dev);
#ifdef i1480u_FLOW_CONTROL
	result = usb_submit_urb(i1480u->notif_urb, GFP_KERNEL);;
	if (result < 0) {
		dev_err(dev, "Can't submit notification URB: %d\n", result);
		goto error_notif_urb_submit;
	}
#endif
	/* Interface is up with an address, now we can create WSS */
	result = wlp_wss_setup(net_dev, &wlp->wss);
	if (result < 0) {
		dev_err(dev, "Can't create WSS: %d. \n", result);
		goto error_wss_setup;
	}
	return 0;
error_wss_setup:
#ifdef i1480u_FLOW_CONTROL
	usb_kill_urb(i1480u->notif_urb);
error_notif_urb_submit:
#endif
	uwb_radio_stop(&wlp->pal);
error_radio_start:
	netif_stop_queue(net_dev);
	i1480u_rx_release(i1480u);
error_rx_setup:
	return result;
}


/**
 * Called on 'ifconfig down'
 */
int i1480u_stop(struct net_device *net_dev)
{
	struct i1480u *i1480u = netdev_priv(net_dev);
	struct wlp *wlp = &i1480u->wlp;

	BUG_ON(wlp->rc == NULL);
	wlp_wss_remove(&wlp->wss);
	netif_carrier_off(net_dev);
#ifdef i1480u_FLOW_CONTROL
	usb_kill_urb(i1480u->notif_urb);
#endif
	netif_stop_queue(net_dev);
	uwb_radio_stop(&wlp->pal);
	i1480u_rx_release(i1480u);
	i1480u_tx_release(i1480u);
	return 0;
}

/**
 *
 * Change the interface config--we probably don't have to do anything.
 */
int i1480u_set_config(struct net_device *net_dev, struct ifmap *map)
{
	int result;
	struct i1480u *i1480u = netdev_priv(net_dev);
	BUG_ON(i1480u->wlp.rc == NULL);
	result = 0;
	return result;
}

/**
 * Change the MTU of the interface
 */
int i1480u_change_mtu(struct net_device *net_dev, int mtu)
{
	static union {
		struct wlp_tx_hdr tx;
		struct wlp_rx_hdr rx;
	} i1480u_all_hdrs;

	if (mtu < ETH_HLEN)	/* We encap eth frames */
		return -ERANGE;
	if (mtu > 4000 - sizeof(i1480u_all_hdrs))
		return -ERANGE;
	net_dev->mtu = mtu;
	return 0;
}

/**
 * Stop the network queue
 *
 * Enable WLP substack to stop network queue. We also set the flow control
 * threshold at this time to prevent the flow control from restarting the
 * queue.
 *
 * we are loosing the current threshold value here ... FIXME?
 */
void i1480u_stop_queue(struct wlp *wlp)
{
	struct i1480u *i1480u = container_of(wlp, struct i1480u, wlp);
	struct net_device *net_dev = i1480u->net_dev;
	i1480u->tx_inflight.threshold = 0;
	netif_stop_queue(net_dev);
}

/**
 * Start the network queue
 *
 * Enable WLP substack to start network queue. Also re-enable the flow
 * control to manage the queue again.
 *
 * We re-enable the flow control by storing the default threshold in the
 * flow control threshold. This means that if the user modified the
 * threshold before the queue was stopped and restarted that information
 * will be lost. FIXME?
 */
void i1480u_start_queue(struct wlp *wlp)
{
	struct i1480u *i1480u = container_of(wlp, struct i1480u, wlp);
	struct net_device *net_dev = i1480u->net_dev;
	i1480u->tx_inflight.threshold = i1480u_TX_INFLIGHT_THRESHOLD;
	netif_start_queue(net_dev);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          /*****************************************************************************
 *  File: drivers/usb/misc/vstusb.c
 *
 *  Purpose: Support for the bulk USB Vernier Spectrophotometers
 *
 *  Author:     Johnnie Peters
 *              Axian Consulting
 *              Beaverton, OR, USA 97005
 *
 *  Modified by:     EQware Engineering, Inc.
 *                   Oregon City, OR, USA 97045
 *
 *  Copyright:  2007, 2008
 *              Vernier Software & Technology
 *              Beaverton, OR, USA 97005
 *
 *  Web:        www.vernier.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *****************************************************************************/
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/usb.h>

#include <linux/usb/vstusb.h>

#define DRIVER_VERSION "VST USB Driver Version 1.5"
#define DRIVER_DESC "Vernier Software Technology Bulk USB Driver"

#ifdef CONFIG_USB_DYNAMIC_MINORS
	#define VSTUSB_MINOR_BASE	0
#else
	#define VSTUSB_MINOR_BASE	199
#endif

#define USB_VENDOR_OCEANOPTICS	0x2457
#define USB_VENDOR_VERNIER	0x08F7	/* Vernier Software & Technology */

#define USB_PRODUCT_USB2000	0x1002
#define USB_PRODUCT_ADC1000_FW	0x1003	/* firmware download (renumerates) */
#define USB_PRODUCT_ADC1000	0x1004
#define USB_PRODUCT_HR2000_FW	0x1009	/* firmware download (renumerates) */
#define USB_PRODUCT_HR2000	0x100A
#define USB_PRODUCT_HR4000_FW	0x1011	/* firmware download (renumerates) */
#define USB_PRODUCT_HR4000	0x1012
#define USB_PRODUCT_USB650	0x1014	/* "Red Tide" */
#define USB_PRODUCT_QE65000	0x1018
#define USB_PRODUCT_USB4000	0x1022
#define USB_PRODUCT_USB325	0x1024	/* "Vernier Spectrometer" */

#define USB_PRODUCT_LABPRO	0x0001
#define USB_PRODUCT_LABQUEST	0x0005

#define VST_MAXBUFFER		(64*1024)

static struct usb_device_id id_table[] = {
	{ USB_DEVICE(USB_VENDOR_OCEANOPTICS, USB_PRODUCT_USB2000)},
	{ USB_DEVICE(USB_VENDOR_OCEANOPTICS, USB_PRODUCT_HR4000)},
	{ USB_DEVICE(USB_VENDOR_OCEANOPTICS, USB_PRODUCT_USB650)},
	{ USB_DEVICE(USB_VENDOR_OCEANOPTICS, USB_PRODUCT_USB4000)},
	{ USB_DEVICE(USB_VENDOR_OCEANOPTICS, USB_PRODUCT_USB325)},
	{ USB_DEVICE(USB_VENDOR_VERNIER, USB_PRODUCT_LABQUEST)},
	{ USB_DEVICE(USB_VENDOR_VERNIER, USB_PRODUCT_LABPRO)},
	{},
};

MODULE_DEVICE_TABLE(usb, id_table);

struct vstusb_device {
	struct kref				kref;
	struct mutex            lock;
	struct usb_device       *usb_dev;
	char                    present;
	char                    isopen;
	struct usb_anchor       submitted;
	int                     rd_pipe;
	int                     rd_timeout_ms;
	int                     wr_pipe;
	int                     wr_timeout_ms;
};
#define to_vst_dev(d) container_of(d, struct vstusb_device, kref)

static struct usb_driver vstusb_driver;

static void vstusb_delete(struct kref *kref)
{
	struct vstusb_device *vstdev = to_vst_dev(kref);

	usb_put_dev(vstdev->usb_dev);
	kfree(vstdev);
}

static int vstusb_open(struct inode *inode, struct file *file)
{
	struct vstusb_device *vstdev;
	struct usb_interface *interface;

	interface = usb_find_interface(&vstusb_driver, iminor(inode));

	if (!interface) {
		printk(KERN_ERR KBUILD_MODNAME
		       ": %s - error, can't find device for minor %d\n",
		       __func__, iminor(inode));
		return -ENODEV;
	}

	vstdev = usb_get_intfdata(interface);

	if (!vstdev)
		return -ENODEV;

	/* lock this device */
	mutex_lock(&vstdev->lock);

	/* can only open one time */
	if ((!vstdev->present) || (vstdev->isopen)) {
		mutex_unlock(&vstdev->lock);
		return -EBUSY;
	}

	/* increment our usage count */
	kref_get(&vstdev->kref);

	vstdev->isopen = 1;

	/* save device in the file's private structure */
	file->private_data = vstdev;

	dev_dbg(&vstdev->usb_dev->dev, "%s: opened\n", __func__);

	mutex_unlock(&vstdev->lock);

	return 0;
}

static int vstusb_release(struct inode *inode44,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_established",
		.data		= &tcp_timeouts[TCP_CONNTRACK_ESTABLISHED],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_fin_wait",
		.data		= &tcp_timeouts[TCP_CONNTRACK_FIN_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_close_wait",
		.data		= &tcp_timeouts[TCP_CONNTRACK_CLOSE_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_last_ack",
		.data		= &tcp_timeouts[TCP_CONNTRACK_LAST_ACK],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_time_wait",
		.data		= &tcp_timeouts[TCP_CONNTRACK_TIME_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_close",
		.data		= &tcp_timeouts[TCP_CONNTRACK_CLOSE],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_max_retrans",
		.data		= &nf_ct_tcp_timeout_max_retrans,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.ctl_name	= NET_IPV4_NF_CONNTRACK_TCP_LOOSE,
		.procname	= "ip_conntrack_tcp_loose",
		.data		= &nf_ct_tcp_loose,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_NF_CONNTRACK_TCP_BE_LIBERAL,
		.procname	= "ip_conntrack_tcp_be_liberal",
		.data		= &nf_ct_tcp_be_liberal,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_NF_CONNTRACK_TCP_MAX_RETRANS,
		.procname	= "ip_conntrack_tcp_max_retrans",
		.data		= &nf_ct_tcp_max_retrans,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= 0
	}
};
#endif /* CONFIG_NF_CONNTRACK_PROC_COMPAT */
#endif /* CONFIG_SYSCTL */

struct nf_conntrack_l4proto nf_conntrack_l4proto_tcp4 __read_mostly =
{
	.l3proto		= PF_INET,
	.l4proto 		= IPPROTO_TCP,
	.name 			= "tcp",
	.pkt_to_tuple 		= tcp_pkt_to_tuple,
	.invert_tuple 		= tcp_invert_tuple,
	.print_tuple 		= tcp_print_tuple,
	.print_conntrack 	= tcp_print_conntrack,
	.packet 		= tcp_packet,
	.new 			= tcp_new,
	.error			= tcp_error,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.to_nlattr		= tcp_to_nlattr,
	.nlattr_size		= tcp_nlattr_size,
	.from_nlattr		= nlattr_to_tcp,
	.tuple_to_nlattr	= nf_ct_port_tuple_to_nlattr,
	.nlattr_to_tuple	= nf_ct_port_nlattr_to_tuple,
	.nlattr_tuple_size	= tcp_nlattr_tuple_size,
	.nla_policy		= nf_ct_port_nla_policy,
#endif
#ifdef CONFIG_SYSCTL
	.ctl_table_users	= &tcp_sysctl_table_users,
	.ctl_table_header	= &tcp_sysctl_header,
	.ctl_table		= tcp_sysctl_table,
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
	.ctl_compat_table	= tcp_compat_sysctl_table,
#endif
#endif
};
EXPORT_SYMBOL_GPL(nf_conntrack_l4proto_tcp4);

struct nf_conntrack_l4proto nf_conntrack_l4proto_tcp6 __read_mostly =
{
	.l3proto		= PF_INET6,
	.l4proto 		= IPPROTO_TCP,
	.name 			= "tcp",
	.pkt_to_tuple 		= tcp_pkt_to_tuple,
	.invert_tuple 		= tcp_invert_tuple,
	.print_tuple 		= tcp_print_tuple,
	.print_conntrack 	= tcp_print_conntrack,
	.packet 		= tcp_packet,
	.new 			= tcp_new,
	.error			= tcp_error,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.to_nlattr		= tcp_to_nlattr,
	.nlattr_size		= tcp_nlattr_size,
	.from_nlattr		= nlattr_to_tcp,
	.tuple_to_nlattr	= nf_ct_port_tuple_to_nlattr,
	.nlattr_to_tuple	= nf_ct_port_nlattr_to_tuple,
	.nlattr_tuple_size	= tcp_nlattr_tuple_size,
	.nla_policy		= nf_ct_port_nla_policy,
#endif
#ifdef CONFIG_SYSCTL
	.ctl_table_users	= &tcp_sysctl_table_users,
	.ctl_table_header	= &tcp_sysctl_header,
	.ctl_table		= tcp_sysctl_table,
#endif
};
EXPORT_SYMBr_urb(urb);
		dev_err(&dev->dev, "%
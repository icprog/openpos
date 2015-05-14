/*
 * fs/direct-io.c
 *
 * Copyright (C) 2002, Linus Torvalds.
 *
 * O_DIRECT
 *
 * 04Jul2002	Andrew Morton
 *		Initial version
 * 11Sep2002	janetinc@us.ibm.com
 * 		added readv/writev support.
 * 29Oct2002	Andrew Morton
 *		rewrote bio_add_page() support.
 * 30Oct2002	pbadari@us.ibm.com
 *		added support for non-aligned IO.
 * 06Nov2002	pbadari@us.ibm.com
 *		added asynchronous IO support.
 * 21Jul2003	nathans@sgi.com
 *		added IO completion notifier.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/task_io_accounting_ops.h>
#include <linux/bio.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/rwsem.h>
#include <linux/uio.h>
#include <asm/atomic.h>

/*
 * How many user pages to map in one call to get_user_pages().  This determines
 * the size of a structure on the stack.
 */
#define DIO_PAGES	64

/*
 * This code generally works in units of "dio_blocks".  A dio_block is
 * somewhere between the hard sector size and the filesystem block size.  it
 * is determined on a per-invocation basis.   When talking to the filesystem
 * we need to convert dio_blocks to fs_blocks by scaling the dio_block quantity
 * down by dio->blkfactor.  Similarly, fs-blocksize quantities are converted
 * to bio_block quantities by shifting left by blkfactor.
 *
 * If blkfactor is zero then the user's request was aligned to the filesystem's
 * blocksize.
 *
 * lock_type is DIO_LOCKING for regular files on direct-IO-naive filesystems.
 * This determines whether we need to do the fancy locking which prevents
 * direct-IO from being able to read uninitialised disk blocks.  If its zero
 * (blockdev) this locking is not done, and if it is DIO_OWN_LOCKING i_mutex is
 * not held for the entire direct write (taken briefly, initially, during a
 * direct read though, but its never held for the duration of a direct-IO).
 */

struct dio {
	/* BIO submission state */
	struct bio *bio;		/* bio under assembly */
	struct inode *inode;
	int rw;
	loff_t i_size;			/* i_size when submitted */
	int lock_type;			/* doesn't change */
	unsigned blkbits;		/* doesn't change */
	unsigned blkfactor;		/* When we're using an alignment which
					   is finer than the filesystem's soft
					   blocksize, this specifies how much
					   finer.  blkfactor=2 means 1/4-block
					   alignment.  Does not change */
	unsigned start_zero_done;	/* flag: sub-blocksize zeroing has
					   been performed at the start of a
					   write */
	int pages_in_io;		/* approximate total IO pages */
	size_t	size;			/* total request size (doesn't change)*/
	sector_t block_in_file;		/* Current offset into the underlying
					   file in dio_block units. */
	unsigned blocks_available;	/* At block_in_file.  changes */
	sector_t final_block_in_request;/* doesn't change */
	unsigned first_block_in_page;	/* doesn't change, Used only once */
	int boundary;			/* prev block is at a boundary */
	int reap_counter;		/* rate limit reaping */
	get_block_t *get_block;		/* block mapping function */
	dio_iodone_t *end_io;		/* IO completion function */
	sector_t final_block_in_bio;	/* current final block in bio + 1 */
	sector_t next_block_for_io;	/* next block to be put under IO,
					   in dio_blocks units */
	struct buffer_head map_bh;	/* last get_block() result */

	/*
	 * Deferred addition of a page to the dio.  These variables are
	 * private to dio_send_cur_page(), submit_page_section() and
	 * dio_bio_add_page().
	 */
	struct page *cur_page;		/* The page */
	unsigned cur_page_offset;	/* Offset into it, in bytes */
	unsigned cur_page_len;		/* Nr of bytes at cur_page_offset */
	sector_t cur_page_block;	/* Where it starts */

	/*
	 * Page fetching state. These variables belong to dio_refill_pages().
	 */
	int curr_page;			/* changes */
	int total_pages;		/* doesn't change */
	unsigned long curr_user_address;/* changes */

	/*
	 * Page queue.  These variables belong to dio_refilINDX( 	                 (   �   �       �                    �?     X H     �?      �_<����j@����j@��Z��j@��                        t m p               �?     X H     �?      �_<����Xh������_<���2D�>��                        t m p               X H     �?      �_<����Xh������_<���2D�>��                        t m p               �?     X H     �?      �_<����Xh������_<���2D�>��                        t m p               X H     �?      �_<����Xh������_<� �2D�>��                        t m p               �?     X H     �?      �_<����Xh������_<���2D�>��                        t m p                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        /* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_type.h,v 1.35 2002/08/05 20:47:06 mvojkovi Exp $ */

#ifndef __NV_STRUCT_H__
#define __NV_STRUCT_H__

#define NV_CHIP_RIVA_128            ((PCI_VENDOR_ID_NVIDIA_SGS << 16)| PCI_DEVICE_ID_NVIDIA_RIVA128)
#define NV_CHIP_TNT                 ((PCI_VENDOR_ID_NVIDIA << 16)| PCI_DEVICE_ID_NVIDIA_TNT)
#define NV_CHIP_TNT2                ((PCI_VENDOR_ID_NVIDIA << 16)| PCI_DEVICE_ID_NVIDIA_TNT2)
#define NV_CHIP_UTNT2               ((PCI_VENDOR_ID_NVIDIA << 16)| PCI_DEVICE_ID_NVIDIA_UTNT2)
#define NV_CHIP_VTNT2               ((PCI_VENDOR_ID_NVIDIA << 16)| PCI_DEVICE_ID_NVIDIA_VTNT2)
#define NV_CHIP_UVTNT2              ((PCI_VENDOR_ID_NVIDIA << 16)| PCI_DEVICE_ID_NVIDIA_UVTNT2)
#define NV_CHIP_ITNT2               ((PCI_VENDOR_ID_NVIDIA << 16)| PCI_DEVICE_ID_NVIDIA_ITNT2)
#define NV_CHIP_GEFORCE_256         ((PCI_VENDOR_ID_NVIDIA << 16)| PCI_DEVICE_ID_NVIDIA_GEFORCE_256)
#define NV_CHIP_GEFORCE_DDR         ((PCI_VENDOR_ID_NVIDIA << 16)| PCI_DEVICE_ID_NVIDIA_GEFORCE_DDR)
#define NV_CHIP_QUADRO              ((PCI_VENDOR_ID_NVIDIA << 16)| PCI_DEVICE_ID_NVIDIA_QUADRO)
#define NV_CHIP_GEFORCE2_MX         ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE2_MX)
#define NV_CHIP_GEFORCE2_MX_100     ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE2_MX_100)
#define NV_CHIP_QUADRO2_MXR         ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO2_MXR)
#define NV_CHIP_GEFORCE2_GO         ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE2_GO)
#define NV_CHIP_GEFORCE2_GTS        ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE2_GTS)
#define NV_CHIP_GEFORCE2_TI         ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE2_TI)
#define NV_CHIP_GEFORCE2_ULTRA      ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE2_ULTRA)
#define NV_CHIP_QUADRO2_PRO         ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO2_PRO)
#define NV_CHIP_GEFORCE4_MX_460     ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_MX_460)
#define NV_CHIP_GEFORCE4_MX_440     ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_MX_440)
#define NV_CHIP_GEFORCE4_MX_420     ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_MX_420)
#define NV_CHIP_GEFORCE4_440_GO     ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_440_GO)
#define NV_CHIP_GEFORCE4_420_GO     ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_420_GO)
#define NV_CHIP_GEFORCE4_420_GO_M32 ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_420_GO_M32)
#define NV_CHIP_QUADRO4_500XGL      ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO4_500XGL)
#define NV_CHIP_GEFORCE4_440_GO_M64 ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_440_GO_M64)
#define NV_CHIP_QUADRO4_200         ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO4_200)
#define NV_CHIP_QUADRO4_550XGL      ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO4_550XGL)
#define NV_CHIP_QUADRO4_500_GOGL    ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO4_500_GOGL)
#define NV_CHIP_0x0180              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x0180)
#define NV_CHIP_0x0181              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x0181)
#define NV_CHIP_0x0182              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x0182)
#define NV_CHIP_0x0188              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x0188)
#define NV_CHIP_0x018A              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x018A)
#define NV_CHIP_0x018B              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x018B)
#define NV_CHIP_IGEFORCE2           ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_IGEFORCE2)
#define NV_CHIP_0x01F0              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x01F0)
#define NV_CHIP_GEFORCE3            ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE3)
#define NV_CHIP_GEFORCE3_TI_200     ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE3_TI_200)
#define NV_CHIP_GEFORCE3_TI_500     ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE3_TI_500)
#define NV_CHIP_QUADRO_DCC          ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO_DCC)
#define NV_CHIP_GEFORCE4_TI_4600    ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_TI_4600)
#define NV_CHIP_GEFORCE4_TI_4400    ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_TI_4400)
#define NV_CHIP_GEFORCE4_TI_4200    ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_GEFORCE4_TI_4200)
#define NV_CHIP_QUADRO4_900XGL      ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO4_900XGL)
#define NV_CHIP_QUADRO4_750XGL      ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO4_750XGL)
#define NV_CHIP_QUADRO4_700XGL      ((PCI_VENDOR_ID_NVIDIA << 16) | PCI_DEVICE_ID_NVIDIA_QUADRO4_700XGL)
#define NV_CHIP_0x0280              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x0280)
#define NV_CHIP_0x0281              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x0281)
#define NV_CHIP_0x0288              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x0288)
#define NV_CHIP_0x0289              ((PCI_VENDOR_ID_NVIDIA << 16) | 0x0289)

#endif /* __NV_STRUCT_H__ */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  /*******************************************************************
 * This file is part of the Emulex Linux Device Driver for         *
 * Fibre Channel Host Bus Adapters.                                *
 * Copyright (C) 2004-2009 Emulex.  All rights reserved.           *
 * EMULEX and SLI are trademarks of Emulex.                        *
 * www.emulex.com                                                  *
 * Portions Copyright (C) 2004-2005 Christoph Hellwig              *
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

#include <scsi/scsi_host.h>

struct lpfc_sli2_slim;

#define LPFC_PCI_DEV_LP		0x1
#define LPFC_PCI_DEV_OC		0x2

#define LPFC_SLI_REV2		2
#define LPFC_SLI_REV3		3
#define LPFC_SLI_REV4		4

#define LPFC_MAX_TARGET		4096	/* max number of targets supported */
#define LPFC_MAX_DISC_THREADS	64	/* max outstanding discovery els
					   requests */
#define LPFC_MAX_NS_RETRY	3	/* Number of retry attempts to contact
					   the NameServer  before giving up. */
#define LPFC_CMD_PER_LUN	3	/* max outstanding cmds per lun */
#define LPFC_DEFAULT_SG_SEG_CNT 64	/* sg element count per scsi cmnd */
#define LPFC_DEFAULT_PROT_SG_SEG_CNT 4096 /* sg protection elements count */
#define LPFC_MAX_SG_SEG_CNT	4096	/* sg element count per scsi cmnd */
#define LPFC_MAX_PROT_SG_SEG_CNT 4096	/* prot sg element count per scsi cmd*/
#define LPFC_IOCB_LIST_CNT	2250	/* list of IOCBs for fast-path usage. */
#define LPFC_Q_RAMP_UP_INTERVAL 120     /* lun q_depth ramp up interval */
#define LPFC_VNAME_LEN		100	/* vport symbolic name length */
#define LPFC_TGTQ_INTERVAL	40000	/* Min amount of time between tgt
					   queue depth change in millisecs */
#define LPFC_TGTQ_RAMPUP_PCENT	5	/* Target queue rampup in percentage */
#define LPFC_MIN_TGT_QDEPTH	100
#define LPFC_MAX_TGT_QDEPTH	0xFFFF

#define  LPFC_MAX_BUCKET_COUNT 20	/* Maximum no. of buckets for stat data
					   collection. */
/*
 * Following time intervals are used of adjusting SCSI device
 * queue depths when there are driver resource error or Firmware
 * resource error.
 */
#define QUEUE_RAMP_DOWN_INTERVAL	(1 * HZ)   /* 1 Second */
#define QUEUE_RAMP_UP_INTERVAL		(300 * HZ) /* 5 minutes */

/* Number of exchanges reserved for discovery to complete */
#define LPFC_DISC_IOCB_BUFF_COUNT 20

#define LPFC_HB_MBOX_INTERVAL   5	/* Heart beat interval in seconds. */
#define LPFC_HB_MBOX_TIMEOUT    30	/* Heart beat timeout  in seconds. */

/* Error Attention event polling interval */
#define LPFC_ERATT_POLL_INTERVAL	5 /* EATT poll interval in seconds */

/* Define macros for 64 bit support */
#define putPaddrLow(addr)    ((uint32_t) (0xffffffff & (u64)(addr)))
#define putPaddrHigh(addr)   ((uint32_t) (0xffffffff & (((u64)(addr))>>32)))
#define getPaddr(high, low)  ((dma_addr_t)( \
			     (( (u64)(high)<<16 ) << 16)|( (u64)(low))))
/* Provide maximum configuration definitions. */
#define LPFC_DRVR_TIMEOUT	16	/* driver iocb timeout value in sec */
#define FC_MAX_ADPTMSG		64

#define MAX_HBAEVT	32

/* Number of MSI-X vectors the driver uses */
#define LPFC_MSIX_VECTORS	2

/* lpfc wait event data ready flag */
#define LPFC_DATA_READY		(1<<0)

enum lpfc_polling_flags {
	ENABLE_FCP_RING_POLLING = 0x1,
	DISABLE_FCP_RING_INT    = 0x2
};

/* Provide DMA memory definitions the driver uses per port instance. */
struct lpfc_dmabuf {
	struct list_head list;
	void *virt;		/* virtual address ptr */
	dma_addr_t phys;	/* mapped address */
	uint32_t   buffer_tag;	/* used for tagged queue ring */
};

struct lpfc_dma_pool {
	struct lpfc_dmabuf   *elements;
	uint32_t    max_count;
	uint32_t    current_count;
};

struct hbq_dmabuf {
	struct lpfc_dmabuf hbuf;
	struct lpfc_dmabuf dbuf;
	uint32_t size;
	uint32_t tag;
	struct lpfc_rcqe rcqe;
};

/* Priority bit.  Set value to exceed low water mark in lpfc_mem. */
#define MEM_PRI		0x100


/****************************************************************************/
/*      Device VPD save area                                                */
/****************************************************************************/
typedef struct lpfc_vpd {
	uint32_t status;	/* vpd status value */
	uint32_t length;	/* number of bytes actually returned */
	struct {
		uint32_t rsvd1;	/* Revision numbers */
		uint32_t biuRev;
		uint32_t smRev;
		uint32_t smFwRev;
		uint32_t endecRev;
		uint16_t rBit;
		uint8_t fcphHigh;
		uint8_t fcphLow;
		uint8_t feaLevelHigh;
		uint8_t feaLevelLow;
		uint32_t postKernRev;
		uint32_t opFwRev;
		uint8_t opFwName[16];
		uint32_t sli1FwRev;
		uint8_t sli1FwName[16];
		uint32_t sli2FwRev;
		uint8_t sli2FwName[16];
	} rev;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint32_t rsvd3  :19;  /* Reserved                             */
		uint32_t cdss	: 1;  /* Configure Data Security SLI          */
		uint32_t rsvd2	: 3;  /* Reserved                             */
		uint32_t cbg	: 1;  /* Configure BlockGuard                 */
		uint32_t cmv	: 1;  /* Configure Max VPIs                   */
		uint32_t ccrp   : 1;  /* Config Command Ring Polling          */
		uint32_t csah   : 1;  /* Configure Synchronous Abort Handling */
		uint32_t chbs   : 1;  /* Cofigure Host Backing store          */
		uint32_t cinb   : 1;  /* Enable Interrupt Notification Block  */
		uint32_t cerbm	: 1;  /* Configure Enhanced Receive Buf Mgmt  */
		uint32_t cmx	: 1;  /* Configure Max XRIs                   */
		uint32_t cmr	: 1;  /* Configure Max RPIs                   */
#else	/*  __LITTLE_ENDIAN */
		uint32_t cmr	: 1;  /* Configure Max RPIs                   */
		uint32_t cmx	: 1;  /* Configure Max XRIs                   */
		uint32_t cerbm	: 1;  /* Configure Enhanced Receive Buf Mgmt  */
		uint32_t cinb   : 1;  /* Enable Interrupt Notification Block  */
		uint32_t chbs   : 1;  /* Cofigure Host Backing store          */
		uint32_t csah   : 1;  /* Configure Synchronous Abort Handling */
		uint32_t ccrp   : 1;  /* Config Command Ring Polling          */
		uint32_t cmv	: 1;  /* Configure Max VPIs                   */
		uint32_t cbg	: 1;  /* Configure BlockGuard                 */
		uint32_t rsvd2	: 3;  /* Reserved                             */
		uint32_t cdss	: 1;  /* Configure Data Security SLI          */
		uint32_t rsvd3  :19;  /* Reserved                             */
#endif
	} sli3Feat;
} lpfc_vpd_t;

struct lpfc_scsi_buf;


/*
 * lpfc stat counters
 */
struct lpfc_stats {
	/* Statistics for ELS commands */
	uint32_t elsLogiCol;
	uint32_t elsRetryExceeded;
	uint32_t elsXmitRetry;
	uint32_t elsDelayRetry;
	uint32_t elsRcvDrop;
	uint32_t elsRcvFrame;
	uint32_t elsRcvRSCN;
	uint32_t elsRcvRNID;
	uint32_t elsRcvFARP;
	uint32_t elsRcvFARPR;
	uint32_t elsRcvFLOGI;
	uint32_t elsRcvPLOGI;
	uint32_t elsRcvADISC;
	uint32_t elsRcvPDISC;
	uint32_t elsRcvFAN;
	uint32_t elsRcvLOGO;
	uint32_t elsRcvPRLO;
	uint32_t elsRcvPRLI;
	uint32_t elsRcvLIRR;
	uint32_t elsRcvRPS;
	uint32_t elsRcvRPL;
	uint32_t elsXmitFLOGI;
	uint32_t elsXmitFDISC;
	uint32_t elsXmitPLOGI;
	uint32_t elsXmitPRLI;
	uint32_t elsXmitADISC;
	uint32_t elsXmitLOGO;
	uint32_t elsXmitSCR;
	uint32_t elsXmitRNID;
	uint32_t elsXmitFARP;
	uint32_t elsXmitFARPR;
	uint32_t elsXmitACC;
	uint32_t elsXmitLSRJT;

	uint32_t frameRcvBcast;
	uint32_t frameRcvMulti;
	uint32_t strayXmitCmpl;
	uint32_t frameXmitDelay;
	uint32_t xriCmdCmpl;
	uint32_t xriStatErr;
	uint32_t LinkUp;
	uint32_t LinkDown;
	uint32_t LinkMultiEvent;
	uint32_t NoRcvBuf;
	uint32_t fcpCmd;
	uint32_t fcpCmpl;
	uint32_t fcpRspErr;
	uint32_t fcpRemoteStop;
	uint32_t fcpPortRjt;
	uint32_t fcpPortBusy;
	uint32_t fcpError;
	uint32_t fcpLocalErr;
};

enum sysfs_mbox_state {
	SMBOX_IDLE,
	SMBOX_WRITING,
	SMBOX_READING
};

struct lpfc_sysfs_mbox {
	enum sysfs_mbox_state state;
	size_t                offset;
	struct lpfcMboxq *    mbox;
};

struct lpfc_hba;


enum discovery_state {
	LPFC_VPORT_UNKNOWN     =  0,    /* vport state is unknown */
	LPFC_VPORT_FAILED      =  1,    /* vport has failed */
	LPFC_LOCAL_CFG_LINK    =  6,    /* local NPORT Id configured */
	LPFC_FLOGI             =  7,    /* FLOGI sent to Fabric */
	LPFC_FDISC             =  8,    /* FDISC sent for vport */
	LPFC_FABRIC_CFG_LINK   =  9,    /* Fabric assigned NPORT Id
				         * configured */
	LPFC_NS_REG            =  10,   /* Register with NameServer */
	LPFC_NS_QRY            =  11,   /* Query NameServer for NPort ID list */
	LPFC_BUILD_DISC_LIST   =  12,   /* Build ADISC and PLOGI lists for
				         * device authentication / discovery */
	LPFC_DISC_AUTH         =  13,   /* Processing ADISC list */
	LPFC_VPORT_READY       =  32,
};

enum hba_state {
	LPFC_LINK_UNKNOWN    =   0,   /* HBA state is unknown */
	LPFC_WARM_START      =   1,   /* HBA state after selective reset */
	LPFC_INIT_START      =   2,   /* Initial state after board reset */
	LPFC_INIT_MBX_CMDS   =   3,   /* Initialize HBA with mbox commands */
	LPFC_LINK_DOWN       =   4,   /* HBA initialized, link is down */
	LPFC_LINK_UP         =   5,   /* Link is up  - issue READ_LA */
	LPFC_CLEAR_LA        =   6,   /* authentication cmplt - issue
				       * CLEAR_LA */
	LPFC_HBA_READY       =  32,
	LPFC_HBA_ERROR       =  -1
};

struct lpfc_vport {
	struct lpfc_hba *phba;
	struct list_head listentry;
	uint8_t port_type;
#define LPFC_PHYSICAL_PORT 1
#define LPFC_NPIV_PORT  2
#define LPFC_FABRIC_PORT 3
	enum discovery_state port_state;

	uint16_t vpi;
	uint16_t vfi;
	uint8_t vfi_state;
#define LPFC_VFI_REGISTERED	0x1

	uint32_t fc_flag;	/* FC flags */
/* Several of these flags are HBA centric and should be moved to
 * phba->link_flag (e.g. FC_PTP, FC_PUBLIC_LOOP)
 */
#define FC_PT2PT                0x1	 /* pt2pt with no fabric */
#define FC_PT2PT_PLOGI          0x2	 /* pt2pt initiate PLOGI */
#define FC_DISC_TMO             0x4	 /* Discovery timer running */
#define FC_PUBLIC_LOOP          0x8	 /* Public loop */
#define FC_LBIT                 0x10	 /* LOGIN bit in loopinit set */
#define FC_RSCN_MODE            0x20	 /* RSCN cmd rcv'ed */
#define FC_NLP_MORE             0x40	 /* More node to process in node tbl */
#define FC_OFFLINE_MODE         0x80	 /* Interface is offline for diag */
#define FC_FABRIC               0x100	 /* We are fabric attached */
#define FC_RSCN_DISCOVERY       0x400	 /* Auth all devices after RSCN */
#define FC_SCSI_SCAN_TMO        0x4000	 /* scsi scan timer running */
#define FC_ABORT_DISCOVERY      0x8000	 /* we want to abort discovery */
#define FC_NDISC_ACTIVE         0x10000	 /* NPort discovery active */
#define FC_BYPASSED_MODE        0x20000	 /* NPort is in bypassed mode */
#define FC_VPORT_NEEDS_REG_VPI	0x80000  /* Needs to have its vpi registered */
#define FC_RSCN_DEFERRED	0x100000 /* A deferred RSCN being processed */

	uint32_t ct_flags;
#define FC_CT_RFF_ID		0x1	 /* RFF_ID accepted by switch */
#define FC_CT_RNN_ID		0x2	 /* RNN_ID accepted by switch */
#define FC_CT_RSNN_NN		0x4	 /* RSNN_NN accepted by switch */
#define FC_CT_RSPN_ID		0x8	 /* RSPN_ID accepted by switch */
#define FC_CT_RFT_ID		0x10	 /* RFT_ID accepted by switch */

	struct list_head fc_nodes;

	/* Keep counters for the number of entries in each list. */
	uint16_t fc_plogi_cnt;
	uint16_t fc_adisc_cnt;
	uint16_t fc_reglogin_cnt;
	uint16_t fc_prli_cnt;
	uint16_t fc_unmap_cnt;
	uint16_t fc_map_cnt;
	uint16_t fc_npr_cnt;
	uint16_t fc_unused_cnt;
	struct serv_parm fc_sparam;	/* buffer for our service parameters */

	uint32_t fc_myDID;	/* fibre channel S_ID */
	uint32_t fc_prevDID;	/* previous fibre channel S_ID */

	int32_t stopped;   /* HBA has not been restarted since last ERATT */
	uint8_t fc_linkspeed;	/* Link speed after last READ_LA */

	uint32_t num_disc_nodes;	/*in addition to hba_state */

	uint32_t fc_nlp_cnt;	/* outstanding NODELIST requests */
	uint32_t fc_rscn_id_cnt;	/* count of RSCNs payloads in list */
	uint32_t fc_rscn_flush;		/* flag use of fc_rscn_id_list */
	struct lpfc_dmabuf *fc_rscn_id_list[FC_MAX_HOLD_RSCN];
	struct lpfc_name fc_nodename;	/* fc nodename */
	struct lpfc_name fc_portname;	/* fc portname */

	struct lpfc_work_evt disc_timeout_evt;

	struct timer_list fc_disctmo;	/* Discovery rescue timer */
	uint8_t fc_ns_retry;	/* retries for fabric nameserver */
	uint32_t fc_prli_sent;	/* cntr for outstanding PRLIs */

	spinlock_t work_port_lock;
	uint32_t work_port_events; /* Timeout to be handled  */
#define WORKER_DISC_TMO                0x1	/* vport: Discovery timeout */
#define WORKER_ELS_TMO                 0x2	/* vport: ELS timeout */
#define WORKER_FDMI_TMO                0x4	/* vport: FDMI timeout */

#define WORKER_MBOX_TMO                0x100	/* hba: MBOX timeout */
#define WORKER_HB_TMO                  0x200	/* hba: Heart beat timeout */
#define WORKER_FABRIC_BLOCK_TMO        0x400	/* hba: fabric block timeout */
#define WORKER_RAMP_DOWN_QUEUE         0x800	/* hba: Decrease Q depth */
#define WORKER_RAMP_UP_QUEUE           0x1000	/* hba: Increase Q depth */

	struct timer_list fc_fdmitmo;
	struct timer_list els_tmofunc;

	int unreg_vpi_cmpl;

	uint8_t load_flag;
#define FC_LOADING		0x1	/* HBA in process of loading drvr */
#define FC_UNLOADING		0x2	/* HBA in process of unloading drvr */
	/* Vport Config Parameters */
	uint32_t cfg_scan_down;
	uint32_t cfg_lun_queue_depth;
	uint32_t cfg_nodev_tmo;
	uint32_t cfg_devloss_tmo;
	uint32_t cfg_restrict_login;
	uint32_t cfg_peer_port_login;
	uint32_t cfg_fcp_class;
	uint32_t cfg_use_adisc;
	uint32_t cfg_fdmi_on;
	uint32_t cfg_discovery_threads;
	uint32_t cfg_log_verbose;
	uint32_t cfg_max_luns;
	uint32_t cfg_enable_da_id;
	uint32_t cfg_max_scsicmpl_time;

	uint32_t dev_loss_tmo_changed;

	struct fc_vport *fc_vport;

#ifdef CONFIG_SCSI_LPFC_DEBUG_FS
	struct dentry *debug_disc_trc;
	struct dentry *debug_nodelist;
	struct dentry *vport_debugfs_root;
	struct lpfc_debugfs_trc *disc_trc;
	atomic_t disc_trc_cnt;
#endif
	uint8_t stat_data_enabled;
	uint8_t stat_data_blocked;
	struct list_head rcv_buffer_list;
	uint32_t vport_flag;
#define STATIC_VPORT	1
};

struct hbq_s {
	uint16_t entry_count;	  /* Current number of HBQ slots */
	uint16_t buffer_count;	  /* Current number of buffers posted */
	uint32_t next_hbqPutIdx;  /* Index to next HBQ slot to use */
	uint32_t hbqPutIdx;	  /* HBQ slot to use */
	uint32_t local_hbqGetIdx; /* Local copy of Get index from Port */
	void    *hbq_virt;	  /* Virtual ptr to this hbq */
	struct list_head hbq_buffer_list;  /* buffers assigned to this HBQ */
				  /* Callback for HBQ buffer allocation */
	struct hbq_dmabuf *(*hbq_alloc_buffer) (struct lpfc_hba *);
				  /* Callback for HBQ buffer free */
	void               (*hbq_free_buffer) (struct lpfc_hba *,
					       struct hbq_dmabuf *);
};

#define LPFC_MAX_HBQS  4
/* this matches the position in the lpfc_hbq_defs array */
#define LPFC_ELS_HBQ	0
#define LPFC_EXTRA_HBQ	1

enum hba_temp_state {
	HBA_NORMAL_TEMP,
	HBA_OVER_TEMP
};

enum intr_type_t {
	NONE = 0,
	INTx,
	MSI,
	MSIX,
};

struct lpfc_hba {
	/* SCSI interface function jump table entries */
	int (*lpfc_new_scsi_buf)
		(struct lpfc_vport *, int);
	struct lpfc_scsi_buf * (*lpfc_get_scsi_buf)
		(struct lpfc_hba *);
	int (*lpfc_scsi_prep_dma_buf)
		(struct lpfc_hba *, struct lpfc_scsi_buf *);
	void (*lpfc_scsi_unprep_dma_buf)
		(struct lpfc_hba *, struct lpfc_scsi_buf *);
	void (*lpfc_release_scsi_buf)
		(struct lpfc_hba *, struct lpfc_scsi_buf *);
	void (*lpfc_rampdown_queue_depth)
		(struct lpfc_hba *);
	void (*lpfc_scsi_prep_cmnd)
		(struct lpfc_vport *, struct lpfc_scsi_buf *,
		 struct lpfc_nodelist *);
	/* IOCB interface function jump table entries */
	int (*__lpfc_sli_issue_iocb)
		(struct lpfc_hba *, uint32_t,
		 struct lpfc_iocbq *, uint32_t);
	void (*__lpfc_sli_release_iocbq)(struct lpfc_hba *,
			 struct lpfc_iocbq *);
	int (*lpfc_hba_down_post)(struct lpfc_hba *phba);


	IOCB_t * (*lpfc_get_iocb_from_iocbq)
		(struct lpfc_iocbq *);
	void (*lpfc_scsi_cmd_iocb_cmpl)
		(struct lpfc_hba *, struct lpfc_iocbq *, struct lpfc_iocbq *);

	/* MBOX interface function jump table entries */
	int (*lpfc_sli_issue_mbox)
		(struct lpfc_hba *, LPFC_MBOXQ_t *, uint32_t);
	/* Slow-path IOCB process function jump table entries */
	void (*lpfc_sli_handle_slow_ring_event)
		(struct lpfc_hba *phba, struct lpfc_sli_ring *pring,
		 uint32_t mask);
	/* INIT device interface function jump table entries */
	int (*lpfc_sli_hbq_to_firmware)
		(struct lpfc_hba *, uint32_t, struct hbq_dmabuf *);
	int (*lpfc_sli_brdrestart)
		(struct lpfc_hba *);
	int (*lpfc_sli_brdready)
		(struct lpfc_hba *, uint32_t);
	void (*lpfc_handle_eratt)
		(struct lpfc_hba *);
	void (*lpfc_stop_port)
		(struct lpfc_hba *);


	/* SLI4 specific HBA data structure */
	struct lpfc_sli4_hba sli4_hba;

	struct lpfc_sli sli;
	uint8_t pci_dev_grp;	/* lpfc PCI dev group: 0x0, 0x1, 0x2,... */
	uint32_t sli_rev;		/* SLI2, SLI3, or SLI4 */
	uint32_t sli3_options;		/* Mask of enabled SLI3 options */
#define LPFC_SLI3_HBQ_ENABLED		0x01
#define LPFC_SLI3_NPIV_ENABLED		0x02
#define LPFC_SLI3_VPORT_TEARDOWN	0x04
#define LPFC_SLI3_CRP_ENABLED		0x08
#define LPFC_SLI3_INB_ENABLED		0x10
#define LPFC_SLI3_BG_ENABLED		0x20
#define LPFC_SLI3_DSS_ENABLED		0x40
	uint32_t iocb_cmd_size;
	uint32_t iocb_rsp_size;

	enum hba_state link_state;
	uint32_t link_flag;	/* link state flags */
#define LS_LOOPBACK_MODE      0x1	/* NPort is in Loopback mode */
					/* This flag is set while issuing */
					/* INIT_LINK mailbox command */
#define LS_NPIV_FAB_SUPPORTED 0x2	/* Fabric supports NPIV */
#define LS_IGNORE_ERATT       0x4	/* intr handler should ignore ERATT */

	uint32_t hba_flag;	/* hba generic flags */
#define HBA_ERATT_HANDLED	0x1 /* This flag is set when eratt handled */
#define DEFER_ERATT		0x2 /* Deferred error attention in progress */
#define HBA_FCOE_SUPPORT	0x4 /* HBA function supports FCOE */
#define HBA_RECEIVE_BUFFER	0x8 /* Rcv buffer posted to worker thread */
#define HBA_POST_RECEIVE_BUFFER 0x10 /* Rcv buffers need to be posted */
#define FCP_XRI_ABORT_EVENT	0x20
#define ELS_XRI_ABORT_EVENT	0x40
#define ASYNC_EVENT		0x80
	struct lpfc_dmabuf slim2p;

	MAILBOX_t *mbox;
	uint32_t *inb_ha_copy;
	uint32_t *inb_counter;
	uint3
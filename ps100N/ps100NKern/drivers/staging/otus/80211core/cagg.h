#ifndef __NWBUTTON_H
#define __NWBUTTON_H

/*
 * 	NetWinder Button Driver-
 *	Copyright (C) Alex Holden <alex@linuxhacker.org> 1998, 1999.
 */

#ifdef __NWBUTTON_C	/* Actually compiling the driver itself */

/* Various defines: */

#define NUM_PRESSES_REBOOT 2	/* How many presses to activate shutdown */
#define BUTTON_DELAY 30 	/* How many jiffies for sequence to end */
#define VERSION "0.3"		/* Driver version number */
#define BUTTON_MINOR 158	/* Major 10, Minor 158, /dev/nwbutton */

/* Structure definitions: */

struct button_callback {
	void (*callback) (void);
	int count;
};

/* Function prototypes: */

static void button_sequence_finished (unsigned long parameters);
static irqreturn_t button_handler (int irq, void *dev_id);
int button_init (void);
int button_add_callback (void (*callback) (void), int count);
int button_del_callback (void (*callback) (void));
static void button_consume_callbacks (int bpcount);

#else /* Not compiling the driver itself */

extern int button_add_callback (void (*callback) (void), int count);
extern int button_del_callback (void (*callback) (void));

#endif /* __NWBUTTON_C */
#endif /* __NWBUTTON_H */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             #
# PCMCIA character device configuration
#

menu "PCMCIA character devices"
	depends on HOTPLUG && PCMCIA!=n

config SYNCLINK_CS
	tristate "SyncLink PC Card support"
	depends on PCMCIA
	help
	  Enable support for the SyncLink PC Card serial adapter, running
	  asynchronous and HDLC communications up to 512Kbps. The port is
	  selectable for RS-232, V.35, RS-449, RS-530, and X.21

	  This driver may be built as a module ( = code which can be
	  inserted in and removed from the running kernel whenever you want).
	  The module will be called synclinkmp.  If you want to do that, say M
	  here.

config CARDMAN_4000
	tristate "Omnikey Cardman 4000 support"
	depends on PCMCIA
	select BITREVERSE
	help
	  Enable support for the Omnikey Cardman 4000 PCMCIA Smartcard
	  reader.

	  This kernel driver requires additional userspace support, either
	  by the vendor-provided PC/SC ifd_handler (http://www.omnikey.com/),
	  or via the cm4000 backend of OpenCT (http://www.opensc.com/).

config CARDMAN_4040
	tristate "Omnikey CardMan 4040 support"
	depends on PCMCIA
	help
	  Enable support for the Omnikey CardMan 4040 PCMCIA Smartcard
	  reader.

	  This card is basically a USB CCID device connected to a FIFO
	  in I/O space.  To use the kernel driver, you will need either the
	  PC/SC ifdhandler provided from the Omnikey homepage
	  (http://www.omnikey.com/), or a current development version of OpenCT
	  (http://www.opensc.org/).

config IPWIRELESS
	tristate "IPWireless 3G UMTS PCMCIA card support"
	depends on PCMCIA && NETDEVICES
	select PPP
	help
	  This is a driver for 3G UMTS PCMCIA card from IPWireless company. In
	  some countries (for example Czech Republic, T-Mobile ISP) this card
	  is shipped for service called UMTS 4G.

endmenu

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       clearFlag;
    u16_t       deleteFlag;
    u32_t       lastArrival;
    u16_t       aggFrameSize;
    u16_t       bar_ssn;    /* starting sequence number in BAR */
    u16_t       dst[3];
    u16_t       complete;     /* complete indication pointer */
};

struct aggSta
{
    u16_t       count[ZM_AC];
    TID_TX      tid_tx[8];
    u16_t       aggFlag[ZM_AC];
};

struct agg_tid_rx
{
    u16_t       aid;
    u16_t       ac;
    u16_t       addBaExchangeStatusCode;
    //struct zsAdditionInfo *addInfo;
    u16_t       seq_start;		/* first seq expected next */
    u16_t       baw_head;		/* head of valid block ack window */
    u16_t       baw_tail;		/* tail of valid block ack window */
    //u16_t       free_count;		/* block ack window size	*/
    u8_t        sq_exceed_count;
    u8_t        sq_behind_count;
    struct aggElement frame[ZM_AGG_BAW_SIZE + 1]; /* out-of-order rx frames */
};

struct aggControl
{
    u16_t       aggEnabled;
    u16_t       ampduIndication;
    u16_t       addbaIndication;
    //TID_BAW     tid_baw;
    u32_t       timestamp;
};

struct aggBaFrameParameter
{
    zbuf_t*     buf;
    u16_t       ba_parameter;
    u8_t        dialog;
    u16_t       ba_policy;
    u16_t       tid;
    u16_t       buffer_size;
    u16_t       ba_timeout;
    u16_t       ba_start_seq;
    u16_t       status_code;
};

struct aggBarControl
{
    u16_t       bar_ack_policy      ;
    u16_t       multi_tid           ;
    u16_t       compressed_bitmap   ;
    u16_t       tid_info            ;
};

struct aggTally
{
    u32_t       got_packets_sum;
    u32_t       got_bytes_sum;
    u32_t       sent_packets_sum;
    u32_t       sent_bytes_sum;
    u32_t       avg_got_packets;
    u32_t       avg_got_bytes;
    u32_t       avg_sent_packets;
    u32_t       avg_sent_bytes;
    u16_t       time;
};


struct destQ {
    struct dest{
        u16_t   Qtype : 1; /* 0 aggr, 1 vtxq */
        TID_TX  tid_tx;
        void*   vtxq;

        struct dest* next;
    } *dest[4];
    struct dest* Head[4];
    //s16_t   size[4];
    u16_t   ppri;
    void    (*insert)(zdev_t* dev, u16_t Qtype, u16_t ac, TID_TX tid_tx, void* vtxq);
    void    (*delete)(zdev_t* dev, u16_t Qtype, TID_TX tid_tx, void* vtxq);
    void    (*init)(zdev_t* dev);
    struct dest* (*getNext)(zdev_t* dev, u16_t ac);
    u16_t   (*exist)(zdev_t* dev, u16_t Qtype, u16_t ac, TID_TX tid_tx, void* vtxq);
    //void    (*scan)(zdev_t* dev);
};
/*
 * aggregation tx
 */
void    zfAggInit(zdev_t* dev);
u16_t   zfApFindSta(zdev_t* dev, u16_t* addr);
u16_t   zfAggGetSta(zdev_t* dev, zbuf_t* buf);
TID_TX  zfAggTxGetQueue(zdev_t* dev, u16_t aid, u16_t tid);
TID_TX  zfAggTxNewQueue(zdev_t* dev, u16_t aid, u16_t tid, zbuf_t* buf);
u16_t   zfAggTxEnqueue(zdev_t* dev, zbuf_t* buf, u16_t aid, TID_TX tid_tx);
u16_t   zfAggTx(zdev_t* dev, zbuf_t* buf, u16_t tid);
u16_t   zfAggTxReadyCount(zdev_t* dev, u16_t ac);
u16_t   zfAggTxPartial(zdev_t* dev, u16_t ac, u16_t readycount);
u16_t   zfAggTxSend(zdev_t* dev, u32_t freeTxd, TID_TX tid_tx);
TID_TX  zfAggTxGetReadyQueue(zdev_t* dev, u16_t ac);
zbuf_t* zfAggTxGetVtxq(zdev_t* dev, TID_TX tid_tx);
u16_t   zfAggTxDeleteQueue(zdev_t* dev, u16_t qnum);
u16_t   zfAggScanAndClear(zdev_t* dev, u32_t time);
u16_t   zfAggClearQueue(zdev_t* dev);
void    zfAggTxScheduler(zdev_t* dev, u8_t ScanAndClear);

/* tid_tx manipulation */
#ifndef ZM_ENABLE_FW_BA_RETRANSMISSION //disable BAW
u16_t   zfAggTidTxInsertHead(zdev_t* dev, struct bufInfo* buf_info, TID_TX tid_tx);
#endif
void    zfAggDestInsert(zdev_t* dev, u16_t Qtype, u16_t ac, TID_TX tid_tx, void* vtxq);
void    zfAggDestDelete(zdev_t* dev, u16_t Qtype, TID_TX tid_tx, void* vtxq);
void    zfAggDestInit(zdev_t* dev);
struct dest* zfAggDestGetNext(zdev_t* dev, u16_t ac);
u16_t   zfAggDestExist(zdev_t* dev, u16_t Qtype, u16_t ac, TID_TX tid_tx, void* vtxq);
/*
 * aggregation rx
 */
struct agg_tid_rx *zfAggRxEnabled(zdev_t* dev, zbuf_t* buf);
u16_t   zfAggRx(zdev_t* dev, zbuf_t* buf, struct zsAdditionInfo *addInfo, struct agg_tid_rx *tid_rx);
struct agg_tid_rx *zfAggRxGetQueue(zdev_t* dev, zbuf_t* bufour starting QPN.
				 * We just need to be sure we don't loop
				 * forever.
				 */
			} while (offset < BITS_PER_PAGE && qpn < QPN_MAX);
		}
		/*
		 * In order to keep the number of pages allocated to a
		 * minimum, we scan the all existing pages before increasing
		 * the size of the bitmap table.
		 */
		if (++i > max_scan) {
			if (qpt->nmaps == QPNMAP_ENTRIES)
				break;
			map = &qpt->map[qpt->nmaps++];
			offset = 0;
		} else if (map < &qpt->map[qpt->nmaps]) {
			++map;
			offset = 0;
		} else {
			map = &qpt->map[0];
			offset = 2;
		}
		qpn = mk_qpn(qpt, map, offset);
	}

	ret = -ENOMEM;

bail:
	return ret;
}

static void free_qpn(struct ipath_qp_table *qpt, u32 qpn)
{
	struct qpn_map *map;

	map = qpt->map + qpn / BITS_PER_PAGE;
	if (map->page)
		clear_bit(qpn & BITS_PER_PAGE_MASK, map->page);
	atomic_inc(&map->n_free);
}

/**
 * ipath_alloc_qpn - allocate a QP number
 * @qpt: the QP table
 * @qp: the QP
 * @type: the QP type (IB_QPT_SMI and IB_QPT_GSI are special)
 *
 * Allocate the next available QPN and put the QP into the hash table.
 * The hash table holds a reference to the QP.
 */
static int ipath_alloc_qpn(struct ipath_qp_table *qpt, struct ipath_qp *qp,
			   enum ib_qp_type type)
{
	unsigned long flags;
	int ret;

	ret = alloc_qpn(qpt, type);
	if (ret < 0)
		goto bail;
	qp->ibqp.qp_num = ret;

	/* Add the QP to the hash table. */
	spin_lock_irqsave(&qpt->lock, flags);

	ret %= qpt->max;
	qp->next = qpt->table[ret];
	qpt->table[ret] = qp;
	atomic_inc(&qp->refcount);

	spin_unlock_irqrestore(&qpt->lock, flags);
	ret = 0;

bail:
	return ret;
}

/**
 * ipath_free_qp - remove a QP from the QP table
 * @qpt: the QP table
 * @qp: the QP to remove
 *
 * Remove the QP from the table so it can't be found asynchronously by
 * the receive interrupt routine.
 */
static void ipath_free_qp(struct ipath_qp_table *qpt, struct ipath_qp *qp)
{
	struct ipath_qp *q, **qpp;
	unsigned long flags;

	spin_lock_irqsave(&qpt->lock, flags);

	/* Remove QP from the hash table. */
	qpp = &qpt->table[qp->ibqp.qp_num % qpt->max];
	for (; (q = *qpp) != NULL; qpp = &q->next) {
		if (q == qp) {
			*qpp = qp->next;
			qp->next = NULL;
			atomic_dec(&qp->refcount);
			break;
		}
	}

	spin_unlock_irqrestore(&qpt->lock, flags);
}

/**
 * ipath_free_all_qps - check for QPs still in use
 * @qpt: the QP table to empty
 *
 * There should not be any QPs still in use.
 * Free memory for table.
 */
unsigned ipath_free_all_qps(struct ipath_qp_table *qpt)
{
	unsigned long flags;
	struct ipath_qp *qp;
	u32 n, qp_inuse = 0;

	spin_lock_irqsave(&qpt->lock, flags);
	for (n = 0; n < qpt->max; n++) {
		qp = qpt->table[n];
		qpt->table[n] = NULL;

		for (; qp; qp = qp->next)
			qp_inuse++;
	}
	spin_unlock_irqrestore(&qpt->lock, flags);

	for (n = 0; n < ARRAY_
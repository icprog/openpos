D in the list (provided
 *          ntds < WHCI_QSET_TD_MAX)
 *
 * Queue Sets (qsets) are added to the asynchronous schedule list
 * (ASL) or the periodic zone list (PZL).
 *
 * qsets may contain up to 8 TDs (either qTDs or iTDs as appropriate).
 * Each TD may refer to at most 1 MiB of data. If a single transfer
 * has > 8MiB of data, TDs can be reused as they are completed since
 * the TD list is used as a circular buffer.  Similarly, several
 * (smaller) transfers may be queued in a qset.
 *
 * WHCI controllers may cache portions of the qsets in the ASL and
 * PZL, requiring the WHCD to inform the WHC that the lists have been
 * updated (fields changed or qsets inserted or removed).  For safe
 * insertion and removal of qsets from the lists the schedule must be
 * stopped to avoid races in updating the QH link pointers.
 *
 * Since the HC is free to execute qsets in any order, all transfers
 * to an endpoint should use the same qset to ensure transfers are
 * executed in the order they're submitted.
 *
 * [WHCI] section 3.2.3
 */
struct whc_qset {
	struct whc_qhead qh;
	union {
		struct whc_qtd qtd[WHCI_QSET_TD_MAX];
		struct whc_itd itd[WHCI_QSET_TD_MAX];
	};

	/* private data for WHCD */
	dma_addr_t qset_dma;
	struct whc *whc;
	struct usb_host_endpoint *ep;
	struct list_head stds;
	int ntds;
	int td_start;
	int td_end;
	struct list_head list_node;
	unsigned in_sw_list:1;
	unsigned in_hw_list:1;
	unsigned remove:1;
	struct urb *pause_after_urb;
	struct completion remove_complete;
	int max_burst;
	int max_seq;
};

static inline void whc_qset_set_link_ptr(u64 *ptr, u64 target)
{
	if (target)
		*ptr = (*ptr & ~(QH_LINK_PTR_MASK | QH_LINK_T)) | QH_LINK_PTR(target);
	else
		*ptr = QH_LINK_T;
}

/**
 * struct di_buf_entry - Device Information (DI) buffer entry.
 *
 * There's one of these per connected device.
 */
struct di_buf_entry {
	__le32 availability_info[8]; /*< MAS availability information, one MAS per bit */
	__le32 addr_sec_info;        /*< addressing and security info */
	__le32 reserved[7];
} __attribute__((packed));

#define WHC_DI_SECURE           (1 << 31)
#define WHC_DI_DISABLE          (1 << 30)
#define WHC_DI_KEY_IDX(k)       ((k) << 8)
#define WHC_DI_KEY_IDX_MASK     0x0000ff00
#define WHC_DI_DEV_ADDR(a)      ((a) << 0)
#define WHC_DI_DEV_ADDR_MASK    0x000000ff

/**
 * struct dn_buf_entry - Device Notification (DN) buffer entry.
 *
 * [WHCI] section 3.2.8
 */
struct dn_buf_entry {
	__u8   msg_size;    /*< number of octets of valid DN data */
	__u8   reserved1;
	__u8   src_addr;    /*< source address */
	__u8   status;      /*< buffer entry status */
	__le32 tkid;        /*< TKID for source device, valid if secure bit is set */
	__u8   dn_data[56]; /*< up to 56 octets of DN data */
} __attribute__((packed));

#define WHC_DN_STATUS_VALID  (1 << 7) /* buffer entry is valid */
#define WHC_DN_STATUS_SECURE (1 << 6) /* notification received using secure frame */

#define WHC_N_DN_ENTRIES (4096 / sizeof(struct dn_buf_entry))

/* The Add MMC IE WUSB Generic Command may take up to 256 bytes of
   data. [WHCI] section 2.4.7. */
#define WHC_GEN_CMD_DATA_LEN 256

/*
 * HC registers.
 *
 * [WHCI] section 2.4
 */

#define WHCIVERSION          0x00

#define WHCSPARAMS           0x04
#  define WHCSPARAMS_TO_N_MMC_IES(p) (((p) >> 16) & 0xff)
#  define WHCSPARAMS_TO_N_KEYS(p)    (((p) >> 8) & 0xff)
#  define WHCSPARAMS_TO_N_DEVICES(p) (((p) >> 0) & 0x7f)

#define WUSBCMD              0x08
#  define WUSBCMD_BCID(b)            ((b) << 16)
#  define WUSBCMD_BCID_MASK          (0xff << 16)
#  define WUSBCMD_ASYNC_QSET_RM      (1 << 12)
#  define WUSBCMD_PERIODIC_QSET_RM   (1 << 11)
#  define WUSBCMD_WUSBSI(s)          ((s) << 8)
#  define WUSBCMD_WUSBSI_MASK        (0x7 << 8)
#  define WUSBCMD_ASYNC_SYNCED_DB    (1 << 7)
#  define WUSBCMD_PERIODIC_SYNCED_DB (1 << 6)
#  define WUSBCMD_ASYNC_UPDATED      (1 << 5)
#  define WUSBCMD_PERIODIC_UPDATED   (1 << 4)
#  define WUSBCMD_ASYNC_EN           (1 << 3)
#  define WUSBCMD_PERIODIC_EN        (1 << 2)
#  define WUSBCMD_WHCRESET           (1 << 1)
#  define WUSBCMD_RUN                (1 << 0)

#define WUSBSTS              0x0c
#  define WUSBSTS_ASYNC_SCHED             (1 << 15)
#  define WUSBSTS_PERIODIC_SCHED          (1 << 14)
#  define WUSBSTS_DNTS_SCHED              (1 << 13)
#  define WUSBSTS_HCHALTED                (1 << 12)
#  define WUSBSTS_GEN_CMD_DONE            (1 << 9)
#  define WUSBSTS_CHAN_TIME_ROLLOVER      (1 << 8)
#  define WUSBSTS_DNTS_OVERFLOW           (1 << 7)
#  define WUSBSTS_BPST_ADJUSTMENT_CHANGED (1 << 6)
#  define WUSBSTS_HOST_ERR                (1 << 5)
#  define WUSBSTS_ASYNC_SCHED_SYNCED      (1 << 4)
#  define WUSBSTS_PERIODIC_SCHED_SYNCED   (1 << 3)
#  define WUSBSTS_DNTS_INT                (1 << 2)
#  define WUSBSTS_ERR_INT                 (1 << 1)
#  define WUSBSTS_INT                     (1 << 0)
#  define WUSBSTS_INT_MASK                0x3ff

#define WUSBINTR             0x10
#  define WUSBINTR_GEN_CMD_DONE             (1 << 9)
#  define WUSBINTR_CHAN_TIME_ROLLOVER       (1 << 8)
#  define WUSBINTR_DNTS_OVERFLOW            (1 << 7)
#  define WUSBINTR_BPST_ADJUSTMENT_CHANGED  (1 << 6)
#  define WUSBINTR_HOST_ERR                 (1 << 5)
#  define WUSBINTR_ASYNC_SCHED_SYNCED       (1 << 4)
#  define WUSBINTR_PERIODIC_SCHED_SYNCED    (1 << 3)
#  define WUSBINTR_DNTS_INT                 (1 << 2)
#  define WUSBINTR_ERR_INT                  (1 << 1)
#  define WUSBINTR_INT                      (1 << 0)
#  define WUSBINTR_ALL 0x3ff

#define WUSBGENCMDSTS        0x14
#  define WUSBGENCMDSTS_ACTIVE (1 << 31)
#  define WUSBGENCMDSTS_ERROR  (1 << 24)
#  define WUSBGENCMDSTS_IOC    (1 << 23)
#  define WUSBGENCMDSTS_MMCIE_ADD 0x01
#  define WUSBGENCMDSTS_MMCIE_RM  0x02
#  define WUSBGENCMDSTS_SET_MAS   0x03
#  define WUSBGENCMDSTS_CHAN_STOP 0x04
#  define WUSBGENCMDSTS_RWP_EN    0x05

#define WUSBGENCMDPARAMS     0x18
#define WUSBGENADDR          0x20
#define WUSBASYNCLISTADDR    0x28
#define WUSBDNTSBUFADDR      0x30
#define WUSBDEVICEINFOADDR   0x38

#define WUSBSETSECKEYCMD     0x40
#  define WUSBSETSECKEYCMD_SET    (1 << 31)
#  define WUSBSETSECKEYCMD_ERASE  (1 << 30)
#  define WUSBSETSECKEYCMD_GTK    (1 << 8)
#  define WUSBSETSECKEYCMD_IDX(i) ((i) << 0)

#define WUSBTKID             0x44
#define WUSBSECKEY           0x48
#define WUSBPERIODICLISTBASE 0x58
#define WUSBMASINDEX         0x60

#define WUSBDNTSCTRL         0x64
#  define WUSBDNTSCTRL_ACTIVE      (1 << 31)
#  define WUSBDNTSCTRL_INTERVAL(i) ((i) << 8)
#  define WUSBDNTSCTRL_SLOTS(s)    ((s) << 0)

#define WUSBTIME             0x68
#  define WUSBTIME_CHANNEL_TIME_MASK 0x00ffffff

#define WUSBBPST             0x6c
#define WUSBDIBUPDATED       0x70

#endif /* #ifndef _WHCI_WHCI_HC_H */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                INDX( 	                 (   �   �      ) �                    P     � r     P     �s��=��?�| 
�����=��E*�����       _	               p m i c _ e x t e r n a l . c . s v n - b a s e                     �s��=��?�| 
�����=��E*�����       _	               p m i c _ e x t e r n a l . c . s v n - b a s e                     P     �s��=��?�| 
�����=��E*�����       _	               p m i c _ e x t e r n a l . c . s v n - b a s e                     P     �s��=��?�| 
�����=�) E*�����       _	               p m i c _ e x t e r n a l . c . s v n - b a s e                     �s��=��?�| 
�����=��E*�����       _	              
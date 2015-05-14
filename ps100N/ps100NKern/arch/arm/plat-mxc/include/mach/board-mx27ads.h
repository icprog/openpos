                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   0X0080
#define PBC_INTR_ENET_INT       0X0100
#define PBC_INTR_OTGFS_INT      0X0200
#define PBC_INTR_XUART_INT      0X0400
#define PBC_INTR_CCTL12         0X0800
#define PBC_INTR_SD2_EN         0x1000
#define PBC_INTR_SD3_EN         0x2000
#define PBC_INTR_MS_EN          0x4000
#define PBC_INTR_SD1_EN         0x8000



/* For interrupts like xuart, enet etc */
#define EXPIO_PARENT_INT        IOMUX_TO_IRQ(MX27_PIN_TIN)
#define MXC_MAX_EXP_IO_LINES    16

/*
 * This corresponds to PBC_INTMASK_SET_REG at offset 0x38.
 *
 */
#define EXPIO_INT_LOW_BAT       (MXC_EXP_IO_BASE + 1)
#define EXPIO_INT_OTG_FS_OVR    (MXC_EXP_IO_BASE + 2)
#define EXPIO_INT_FSH_OVR       (MXC_EXP_IO_BASE + 3)
#define EXPIO_INT_RES4          (MXC_EXP_IO_BASE + 4)
#define EXPIO_INT_RES5          (MXC_EXP_IO_BASE + 5)
#define EXPIO_INT_RES6          (MXC_EXP_IO_BASE + 6)
#define EXPIO_INT_FEC           (MXC_EXP_IO_BASE + 7)
#define EXPIO_INT_ENET_INT      (MXC_EXP_IO_BASE + 8)
#define EXPIO_INT_OTG_FS_INT    (MXC_EXP_IO_BASE + 9)
#define EXPIO_INT_XUART_INTA    (MXC_EXP_IO_BASE + 10)
#define EXPIO_INT_CCTL12_INT    (MXC_EXP_IO_BASE + 11)
#define EXPIO_INT_SD2_EN        (MXC_EXP_IO_BASE + 12)
#define EXPIO_INT_SD3_EN        (MXC_EXP_IO_BASE + 13)
#define EXPIO_INT_MS_EN         (MXC_EXP_IO_BASE + 14)
#define EXPIO_INT_SD1_EN        (MXC_EXP_IO_BASE + 15)

/*
 * This is System IRQ used by CS8900A for interrupt generation
 * taken from platform.h
 */
#define CS8900AIRQ              EXPIO_INT_ENET_INT
/* This is I/O Base address used to access registers of CS8900A on MXC ADS */
#define CS8900A_BASE_ADDRESS    (PBC_CS8900A_IOBASE_REG + 0x300)

#define MXC_PMIC_INT_LINE       IOMUX_TO_IRQ(MX27_PIN_TOUT)

/*
* This is used to detect if the CPLD version is for mx27 evb board rev-a
*/
#define PBC_CPLD_VERSION_IS_REVA() \
	((__raw_readw(PBC_VERSION_REG) & \
	(PBC_VERSION_ADS | PBC_VERSION_EVB_REVB))\
	== 0)

/* This is used to active or inactive ata signal in CPLD .
 *  It is dependent with hardware
 */
#define PBC_ATA_SIGNAL_ACTIVE() \
	__raw_writew(           \
		PBC_BCTRL2_ATAFEC_EN|PBC_BCTRL2_ATAFEC_SEL|PBC_BCTRL2_ATA_EN, \
		PBC_BCTRL2_CLEAR_REG)

#define PBC_ATA_SIGNAL_INACTIVE() \
	__raw_writew(  \
		PBC_BCTRL2_ATAFEC_EN|PBC_BCTRL2_ATAFEC_SEL|PBC_BCTRL2_ATA_EN, \
		PBC_BCTRL2_SET_REG)

#define MXC_BD_LED1             (1 << 5)
#define MXC_BD_LED2             (1 << 6)
#define MXC_BD_LED_ON(led) \
	__raw_writew(led, PBC_BCTRL1_SET_REG)
#define MXC_BD_LED_OFF(led) \
	__raw_writew(led, PBC_BCTRL1_CLEAR_REG)

/* to determine the correct external crystal reference */
#define CKIH_27MHZ_BIT_SET      (1 << 3)

#endif				/* __ASM_ARCH_MXC_BOARD_MX27ADS_H__ */

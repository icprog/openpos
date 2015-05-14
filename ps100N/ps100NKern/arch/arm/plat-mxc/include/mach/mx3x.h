                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                S2_BASE_ADDR + 0x000D8000)
#define WDOG1_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000DC000)
#define PWM_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000E0000)
#define RTIC_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000EC000)

/*
 * ROMP and AVIC
 */
#define ROMP_BASE_ADDR		0x60000000
#define ROMP_BASE_ADDR_VIRT	0xFC500000
#define ROMP_SIZE		SZ_1M

#define AVIC_BASE_ADDR		0x68000000
#define AVIC_BASE_ADDR_VIRT	0xFC400000
#define AVIC_SIZE		SZ_1M

/*
 * NAND, SDRAM, WEIM, M3IF, EMI controllers
 */
#define X_MEMC_BASE_ADDR	0xB8000000
#define X_MEMC_BASE_ADDR_VIRT	0xFC320000
#define X_MEMC_SIZE		SZ_64K

#define ESDCTL_BASE_ADDR	(X_MEMC_BASE_ADDR + 0x1000)
#define WEIM_BASE_ADDR		(X_MEMC_BASE_ADDR + 0x2000)
#define M3IF_BASE_ADDR		(X_MEMC_BASE_ADDR + 0x3000)
#define EMI_CTL_BASE_ADDR	(X_MEMC_BASE_ADDR + 0x4000)
#define PCMCIA_CTL_BASE_ADDR	EMI_CTL_BASE_ADDR

/*
 * Memory regions and CS
 */
#define IPU_MEM_BASE_ADDR	0x70000000
#define CSD0_BASE_ADDR		0x80000000
#define CSD1_BASE_ADDR		0x90000000

/*!
 * This macro defines the physical to virtual address mapping for all the
 * peripheral modules. It is used by passing in the physical address as x
 * and returning the virtual address. If the physical address is not mapped,
 * it returns 0xDEADBEEF
 */
#define IO_ADDRESS(x)   \
	(void __force __iomem *) \
	(((x >= AIPS1_BASE_ADDR) && (x < (AIPS1_BASE_ADDR + AIPS1_SIZE))) ? AIPS1_IO_ADDRESS(x):\
	((x >= SPBA0_BASE_ADDR) && (x < (SPBA0_BASE_ADDR + SPBA0_SIZE))) ? SPBA0_IO_ADDRESS(x):\
	((x >= AIPS2_BASE_ADDR) && (x < (AIPS2_BASE_ADDR + AIPS2_SIZE))) ? AIPS2_IO_ADDRESS(x):\
	((x >= ROMP_BASE_ADDR) && (x < (ROMP_BASE_ADDR + ROMP_SIZE))) ? ROMP_IO_ADDRESS(x):\
	((x >= AVIC_BASE_ADDR) && (x < (AVIC_BASE_ADDR + AVIC_SIZE))) ? AVIC_IO_ADDRESS(x):\
	((x >= CS4_BASE_ADDR) && (x < (CS4_BASE_ADDR + CS4_SIZE))) ? CS4_IO_ADDRESS(x):\
	((x >= X_MEMC_BASE_ADDR) && (x < (X_MEMC_BASE_ADDR + X_MEMC_SIZE))) ? X_MEMC_IO_ADDRESS(x):\
	((x >= NFC_BASE_ADDR) && (x < (NFC_BASE_ADDR + NFC_SIZE))) ? NFC_IO_ADDRESS(x):\
	0xDEADBEEF)

/*
 * define the address mapping macros: in physical address order
 */
#define L2CC_IO_ADDRESS(x)  \
	(((x) - L2CC_BASE_ADDR) + L2CC_BASE_ADDR_VIRT)

#define AIPS1_IO_ADDRESS(x)  \
	(((x) - AIPS1_BASE_ADDR) + AIPS1_BASE_ADDR_VIRT)

#define SPBA0_IO_ADDRESS(x)  \
	(((x) - SPBA0_BASE_ADDR) + SPBA0_BASE_ADDR_VIRT)

#define AIPS2_IO_ADDRESS(x)  \
	(((x) - AIPS2_BASE_ADDR) + AIPS2_BASE_ADDR_VIRT)

#define ROMP_IO_ADDRESS(x)  \
	(((x) - ROMP_BASE_ADDR) + ROMP_BASE_ADDR_VIRT)

#define AVIC_IO_ADDRESS(x)  \
	(((x) - AVIC_BASE_ADDR) + AVIC_BASE_ADDR_VIRT)

#define CS4_IO_ADDRESS(x)  \
	(((x) - CS4_BASE_ADDR) + CS4_BASE_ADDR_VIRT)

#define CS5_IO_ADDRESS(x)  \
	(((x) - CS5_BASE_ADDR) + CS5_BASE_ADDR_VIRT)

#define X_MEMC_IO_ADDRESS(x)  \
	(((x) - X_MEMC_BASE_ADDR) + X_MEMC_BASE_ADDR_VIRT)

#define PCMCIA_IO_ADDRESS(x) \
	(((x) - X_MEMC_BASE_ADDR) + X_MEMC_BASE_ADDR_VIRT)

/*
 * DMA request assignments
 */
#define DMA_REQ_ECT        31
#define DMA_REQ_NFC        30
#define DMA_REQ_SSI1_TX1   29
#define DMA_REQ_SSI1_RX1   28
#define DMA_REQ_SSI1_TX2   27
#define DMA_REQ_SSI1_RX2   26
#define DMA_REQ_SSI2_TX1   25
#define DMA_REQ_SSI2_RX1   24
#define DMA_REQ_SSI2_TX2   23
#define DMA_REQ_SSI2_RX2   22
#define DMA_REQ_UART1_TX   19
#define DMA_REQ_UART1_RX   18
#define DMA_REQ_UART2_TX   17
#define DMA_REQ_UART2_RX   16
#define DMA_REQ_EXTREQ1    15
#define DMA_REQ_EXTREQ2    14
#define DMA_REQ_CSPI1_TX   9
#define DMA_REQ_CSPI1_RX   8
#define DMA_REQ_CSPI2_TX   7
#define DMA_REQ_CSPI2_RX   6
#define DMA_REQ_ATA_RX     4
#define DMA_REQ_ATA_TX     3
#define DMA_REQ_ATA_TX_END 2
#define DMA_REQ_CCM        1
#define DMA_REQ_EXTREQ0    0

/*
 * Interrupt numbers
 */
#define MXC_INT_RESV1		1
#define MXC_INT_I2C3		3
#define MXC_INT_I2C2		4
#define MXC_INT_RTIC		6
#define MXC_INT_I2C		10
#define MXC_INT_CSPI2		13
#define MXC_INT_CSPI1		14
#define MXC_INT_ATA		15
#define MXC_INT_UART3		18
#define MXC_INT_IIM		19
#define MXC_INT_RNGA		22
#define MXC_INT_EVTMON		23
#define MXC_INT_KPP		24
#define MXC_INT_RTC		25
#define MXC_INT_PWM		26
#define MXC_INT_EPIT2		27
#define MXC_INT_EPIT                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
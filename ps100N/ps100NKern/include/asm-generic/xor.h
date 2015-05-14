c, *master_pdev;
	unsigned int master_devfn = PCI_DEVFN(PCI_SLOT(pdev->devfn), 0);

	master_pdev = pci_get_slot(pdev->bus, master_devfn);
	if (master_pdev) {
		struct ahd_softc *master = pci_get_drvdata(master_pdev);
		if (master) {
			ahd->flags &= ~AHD_BIOS_ENABLED;
			ahd->flags |= master->flags & AHD_BIOS_ENABLED;
		} else
			printk(KERN_ERR "aic79xx: no multichannel peer found!\n");
		pci_dev_put(master_pdev);
	}
}

static int
ahd_linux_pci_dev_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	char		 buf[80];
	struct		 ahd_softc *ahd;
	ahd_dev_softc_t	 pci;
	const struct ahd_pci_identity *entry;
	char		*name;
	int		 error;
	struct device	*dev = &pdev->dev;

	pci = pdev;
	entry = ahd_find_pci_device(pci);
	if (entry == NULL)
		return (-ENODEV);

	/*
	 * Allocate a softc for this card and
	 * set it up for attachment by our
	 * common detect routine.
	 */
	sprintf(buf, "ahd_pci:%d:%d:%d",
		ahd_get_pci_bus(pci),
		ahd_get_pci_slot(pci),
		ahd_get_pci_function(pci));
	name = malloc(strlen(buf) + 1, M_DEVBUF, M_NOWAIT);
	if (name == NULL)
		return (-ENOMEM);
	strcpy(name, buf);
	ahd = ahd_alloc(NULL, name);
	if (ahd == NULL)
		return (-ENOMEM);
	if (pci_enable_device(pdev)) {
		ahd_free(ahd);
		return (-ENODEV);
	}
	pci_set_master(pdev);

	if (sizeof(dma_addr_t) > 4) {
		const u64 required_mask = dma_get_required_mask(dev);

		if (required_mask > DMA_BIT_MASK(39) &&
		    dma_set_mask(dev, DMA_BIT_MASK(64)) == 0)
			ahd->flags |= AHD_64BIT_ADDRESSING;
		else if (required_mask > DMA_BIT_MASK(32) &&
			 dma_set_mask(dev, DMA_BIT_MASK(39)) == 0)
			ahd->flags |= AHD_39BIT_ADDRESSING;
		else
			dma_set_mask(dev, DMA_BIT_MASK(32));
	} else {
		dma_set_mask(dev, DMA_BIT_MASK(32));
	}
	ahd->dev_softc = pci;
	error = ahd_pci_config(ahd, entry);
	if (error != 0) {
		ahd_free(ahd);
		return (-error);
	}

	/*
	 * Second Function PCI devices need to inherit some
	 * * settings from function 0.
	 */
	if ((ahd->features & AHD_MULTI_FUNC) && PCI_FUNC(pdev->devfn) != 0)
		ahd_linux_pci_inherit_flags(ahd);

	pci_set_drvdata(pdev, ahd);

	ahd_linux_register_host(ahd, &aic79xx_driver_template);
	return (0);
}

static struct pci_driver aic79xx_pci_driver = {
	.name		= "aic79xx",
	.probe		= ahd_linux_pci_dev_probe,
#ifdef CONFIG_PM
	.suspend	= ahd_linux_pci_dev_suspend,
	.resume		= ahd_linux_pci_dev_resume,
#endif
	.remove		= ahd_linux_pci_dev_remove,
	.id_table	= ahd_linux_pci_id_table
};

int
ahd_linux_pci_init(void)
{
	return pci_register_driver(&aic79xx_pci_driver);
}

void
ahd_linux_pci_exit(void)
{
	pci_unregister_driver(&aic79xx_pci_driver);
}

static int
ahd_linux_pci_reserve_io_regions(struct ahd_softc *ahd, resource_size_t *base,
				 resource_size_t *base2)
{
	*base = pci_resource_start(ahd->dev_softc, 0);
	/*
	 * This is really the 3rd bar and should be at index 2,
	 * but the Linux PCI code doesn't know how to "count" 64bit
	 * bars.
	 */
	*base2 = pci_resource_start(ahd->dev_softc, 3);
	if (*base == 0 || *base2 == 0)
		return (ENOMEM);
	if (!request_region(*base, 256, "aic79xx"))
		return (ENOMEM);
	if (!request_region(*base2, 256, "aic79xx")) {
		release_region(*base, 256);
		return (ENOMEM);
	}
	return (0);
}

static int
ahd_linux_pci_reserve_mem_region(struct ahd_softc *ahd,
				 resource_size_t *bus_addr,
				 uint8_t __iomem **maddr)
{
	resource_size_t	start;
	resource_size_t	base_page;
	u_long	base_offset;
	int	error = 0;

	if (aic79xx_allow_memio == 0)
		return (ENOMEM);

	if ((ahd->bugs & AHD_PCIX_MMAPIO_BUG) != 0)
		return (ENOMEM);

	start = pci_resource_start(ahd->dev_softc, 1);
	base_page = start & PAGE_MASK;
	base_offset = start - base_page;
	if (start != 0) {
		*bus_addr = start;
		if (!request_mem_region(start, 0x1000, "aic79xx"))
			error = ENOMEM;
		if (!error) {
			*maddr = ioremap_nocache(base_page, base_offset + 512);
			if (*maddr == NULL) {
				error = ENOMEM;
				release_mem_region(start, 0x1000);
			} else
				*maddr += base_offset;
		}
	} else
		error = ENOMEM;
	return (error);
}

int
ahd_pci_map_registers(struct ahd_softc *ahd)
{
	uint32_t command;
	resource_size_t base;
	uint8_t	__iomem *maddr;
	int	 error;

	/*
	 * If its allowed, we prefer memory mapped access.
	 */
	command = ahd_pci_read_config(ahd->dev_softc, PCIR_COMMAND, 4);
	command &= ~(PCIM_CMD_PORTEN|PCIM_CMD_MEMEN);
	base = 0;
	maddr = NULL;
	error = ahd_linux_pci_reserve_mem_region(ahd, &base, &maddr);
	if (error == 0) {
		ahd->platform_data->mem_busaddr = base;
		ahd->tags[0] = BUS_SPACE_MEMIO;
		ahd->bshs[0].maddr = maddr;
		ahd->tags[1] = BUS_SPACE_MEMIO;
		ahd->bshs[1].maddr = maddr + 0x100;
		ahd_pci_write_config(ahd->dev_softc, PCIR_COMMAND,
				     command | PCIM_CMD_MEMEN, 4);

		if (ahd_pci_test_register_access(ahd) != 0) {

			printf("aic79xx: PCI Device %d:%d:%d "
			       "failed memory mapped test.  Using PIO.\n",
			       ahd_get_pci_bus(ahd->dev_softc),
			       ahd_get_pci_slot(ahd->dev_softc),
			       ahd_get_pci_function(ahd->dev_softc));
			iounmap(maddr);
			release_mem_region(ahd->platform_data->mem_busaddr,
					   0x1000);
			ahd->bshs[0].maddr = NULL;
			maddr = NULL;
		} else
			command |= PCIM_CMD_MEMEN;
	} else if (bootverbose) {
		printf("aic79xx: PCI%d:%d:%d MEM region 0x%llx "
		       "unavailable. Cannot memory map device.\n",
		       ahd_get_pci_bus(ahd->dev_softc),
		       ahd_get_pci_slot(ahd->dev_softc),
		       ahd_get_pci_function(ahd->dev_softc),
		       (unsigned long long)base);
	}

	if (maddr == NULL) {
		resource_size_t base2;

		error = ahd_linux_pci_reserve_io_regions(ahd, &base, &base2);
		if (error == 0) {
			ahd->tags[0] = BUS_SPACE_PIO;
			ahd->tags[1] = BUS_SPACE_PIO;
			ahd->bshs[0].ioport = (u_long)base;
			ahd->bshs[1].ioport = (u_long)base2;
			command |= PCIM_CMD_PORTEN;
		} else {
			printf("aic79xx: PCI%d:%d:%d IO regions 0x%llx and "
			       "0x%llx unavailable. Cannot map device.\n",
			       ahd_get_pci_bus(ahd->dev_softc),
			       ahd_get_pci_slot(ahd->dev_softc),
			       ahd_get_pci_function(ahd->dev_softc),
			       (unsigned long long)base,
			       (unsigned long long)base2);
		}
	}
	ahd_pci_write_config(ahd->dev_softc, PCIR_COMMAND, command, 4);
	return (error);
}

int
ahd_pci_map_int(struct ahd_softc *ahd)
{
	int error;

	error = request_irq(ahd->dev_softc->irq, ahd_linux_isr,
			    IRQF_SHARED, "aic79xx", ahd);
	if (!error)
		ahd->platform_data->irq = ahd->dev_softc->irq;
	
	return (-error);
}

void
ahd_power_state_change(struct ahd_softc *ahd, ahd_power_state new_state)
{
	pci_set_power_state(ahd->dev_softc, new_state);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              /*
 * QLogic Fibre Channel HBA Driver
 * Copyright (c)  2003-2008 QLogic Corporation
 *
 * See LICENSE.qla2xxx for copyright and licensing details.
 */
#include "qla_def.h"

#include <linux/delay.h>

static inline void
qla2xxx_prep_dump(struct qla_hw_data *ha, struct qla2xxx_fw_dump *fw_dump)
{
	fw_dump->fw_major_version = htonl(ha->fw_major_version);
	fw_dump->fw_minor_version = htonl(ha->fw_minor_version);
	fw_dump->fw_subminor_version = htonl(ha->fw_subminor_version);
	fw_dump->fw_attributes = htonl(ha->fw_attributes);

	fw_dump->vendor = htonl(ha->pdev->vendor);
	fw_dump->device = htonl(ha->pdev->device);
	fw_dump->subsystem_vendor = htonl(ha->pdev->subsystem_vendor);
	fw_dump->subsystem_device = htonl(ha->pdev->subsystem_device);
}

static inline void *
qla2xxx_copy_queues(struct qla_hw_data *ha, void *ptr)
{
	struct req_que *req = ha->req_q_map[0];
	struct rsp_que *rsp = ha->rsp_q_map[0];
	/* Request queue. */
	memcpy(ptr, req->ring, req->length *
	    sizeof(request_t));

	/* Response queue. */
	ptr += req->length * sizeof(request_t);
	memcpy(ptr, rsp->ring, rsp->length  *
	    sizeof(response_t));

	return ptr + (rsp->length * sizeof(response_t));
}

static int
qla24xx_dump_ram(struct qla_hw_data *ha, uint32_t addr, uint32_t *ram,
    uint32_t ram_dwords, void **nxt)
{
	int rval;
	uint32_t cnt, stat, timer, dwords, idx;
	uint16_t mb0;
	struct device_reg_24xx __iomem *reg = &ha->iobase->isp24;
	dma_addr_t dump_dma = ha->gid_list_dma;
	uint32_t *dump = (uint32_t *)ha->gid_list;

	rval = QLA_SUCCESS;
	mb0 = 0;

	WRT_REG_WORD(&reg->mailbox0, MBC_DUMP_RISC_RAM_EXTENDED);
	clear_bit(MBX_INTERRUPT, &ha->mbx_cmd_flags);

	dwords = GID_LIST_SIZE / 4;
	for (cnt = 0; cnt < ram_dwords && rval == QLA_SUCCESS;
	    cnt += dwords, addr += dwords) {
		if (cnt + dwords > ram_dwords)
			dwords = ram_dwords - cnt;

		WRT_REG_WORD(&reg->mailbox1, LSW(addr));
		WRT_REG_WORD(&reg->mailbox8, MSW(addr));

		WRT_REG_WORD(&reg->mailbox2, MSW(dump_dma));
		WRT_REG_WORD(&reg->mailbox3, LSW(dump_dma));
		WRT_REG_WORD(&reg->mailbox6, MSW(MSD(dump_dma)));
		WRT_REG_WORD(&reg->mailbox7, LSW(MSD(dump_dma)));

		WRT_REG_WORD(&reg->mailbox4, MSW(dwords));
		WRT_REG_WORD(&reg->mailbox5, LSW(dwords));
		WRT_REG_DWORD(&reg->hccr, HCCRX_SET_HOST_INT);

		for (timer = 6000000; timer; timer--) {
			/* Check for pending interrupts. */
			stat = RD_REG_DWORD(&reg->host_status);
			if (stat & HSRX_RISC_INT) {
				stat &= 0xff;

				if (stat == 0x1 || stat == 0x2 ||
				    stat == 0x10 || stat == 0x11) {
					set_bit(MBX_INTERRUPT,
					    &ha->mbx_cmd_flags);

					mb0 = RD_REG_WORD(&reg->mailbox0);

					WRT_REG_DWORD(&reg->hccr,
					    HCCRX_CLR_RISC_INT);
					RD_REG_DWORD(&reg->hccr);
					break;
				}

				/* Clear this intr; it wasn't a mailbox intr */
				WRT_REG_DWORD(&reg->hccr, HCCRX_CLR_RISC_INT);
				RD_REG_DWORD(&reg->hccr);
			}
			udelay(5);
		}

		if (test_and_clear_bit(MBX_INTERRUPT, &ha->mbx_cmd_flags)) {
			rval = mb0 & MBS_MASK;
			for (idx = 0; idx < dwords; idx++)
				ram[cnt + idx] = swab32(dump[idx]);
		} else {
			rval = QLA_FUNCTION_FAILED;
		}
	}

	*nxt = rval == QLA_SUCCESS ? &ram[cnt]: NULL;
	return rval;
}

static int
qla24xx_dump_memory(struct qla_hw_data *ha, uint32_t *code_ram,
    uint32_t cram_size, void **nxt)
{
	int rval;

	/* Code RAM. */
	rval = qla24xx_dump_ram(ha, 0x20000, code_ram, cram_size / 4, nxt);
	if (rval != QLA_SUCCESS)
		return rval;

	/* External Memory. */
	return qla24xx_dump_ram(ha, 0x100000, *nxt,
	    ha->fw_memory_size - 0x100000 + 1, nxt);
}

static uint32_t *
qla24xx_read_window(struct device_reg_24xx __iomem *reg, uint32_t iobase,
    uint32_t count, uint32_t *buf)
{
	uint32_t __iomem *dmp_reg;

	WRT_REG_DWORD(&reg->iobase_addr, iobase);
	dmp_reg = &reg->iobase_window;
	while (count--)
		*buf++ = htonl(RD_REG_DWORD(dmp_reg++));

	return buf;
}

static inline int
qla24xx_pause_risc(struct device_reg_24xx __iomem *reg)
{
	int rval = QLA_SUCCESS;
	uint32_t cnt;

	WRT_REG_DWORD(&reg->hccr, HCCRX_SET_RISC_PAUSE);
	for (cnt = 30000;
	    ((RD_REG_DWORD(&reg->host_status) & HSRX_RISC_PAUSED) == 0) &&
	    rval == QLA_SUCCESS; cnt--) {
		if (cnt)
			udelay(100);
		else
			rval = QLA_FUNCTION_TIMEOUT;
	}

	return rval;
}

static int
qla24xx_soft_reset(struct qla_hw_data *ha)
{
	int rval = QLA_SUCCESS;
	uint32_t cnt;
	uint16_t mb0, wd;
	struct device_reg_24xx __iomem *reg = &ha->iobase->isp24;

	/* Reset RISC. */
	WRT_REG_DWORD(&reg->ctrl_status, CSRX_DMA_SHUTDOWN|MWB_4096_BYTES);
	for (cnt = 0; cnt < 30000; cnt++) {
		if ((RD_REG_DWORD(&reg->ctrl_status) & CSRX_DMA_ACTIVE) == 0)
			break;

		udelay(10);
	}

	WRT_REG_DWORD(&reg->ctrl_status,
	    CSRX_ISP_SOFT_RESET|CSRX_DMA_SHUTDOWN|MWB_4096_BYTES);
	pci_read_config_word(ha->pdev, PCI_COMMAND, &wd);

	udelay(100);
	/* Wait for firmware to complete NVRAM accesses. */
	mb0 = (uint32_t) RD_REG_WORD(&reg->mailbox0);
	for (cnt = 10000 ; cnt && mb0; cnt--) {
		udelay(5);
		mb0 = (uint32_t) RD_REG_WORD(&reg->mailbox0);
		barrier();
	}

	/* Wait for soft-reset to complete. */
	for (cnt = 0; cnt < 30000; cnt++) {
		if ((RD_REG_DWORD(&reg->ctrl_status) &
		    CSRX_ISP_SOFT_RESET) == 0)
			break;

		udelay(10);
	}
	WRT_REG_DWORD(&reg->hccr, HCCRX_CLR_RISC_RESET);
	RD_REG_DWORD(&reg->hccr);             /* PCI Posting. */

	for (cnt = 30000; RD_REG_WORD(&reg->mailbox0) != 0 &&
	    rval == QLA_SUCCESS; cnt--) {
		if (cnt)
			udelay(100);
		else
			rval = QLA_FUNCTION_TIMEOUT;
	}

	return rval;
}

static int
qla2xxx_dump_ram(struct qla_hw_data *ha, uint32_t addr, uint16_t *ram,
    uint32_t ram_words, void **nxt)
{
	int rval;
	uint32_t cnt, stat, timer, words, idx;
	uint16_t mb0;
	struct device_reg_2xxx __iomem *reg = &ha->iobase->isp;
	dma_addr_t dump_dma = ha->gid_list_dm
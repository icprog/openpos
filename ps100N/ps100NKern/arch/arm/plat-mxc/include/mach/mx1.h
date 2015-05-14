reg, 0x4010, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x4020, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x4030, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x4040, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x4050, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x4060, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x4070, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x4080, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x4090, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x40A0, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x40B0, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x40C0, 16, iter_reg);
	qla24xx_read_window(reg, 0x40D0, 16, iter_reg);

	/* Frame Buffer registers. */
	iter_reg = fw->fb_hdw_reg;
	iter_reg = qla24xx_read_window(reg, 0x6000, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x6010, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x6020, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x6030, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x6040, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x6100, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x6130, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x6150, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x6170, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x6190, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x61B0, 16, iter_reg);
	iter_reg = qla24xx_read_window(reg, 0x61C0, 16, iter_reg);
	qla24xx_read_window(reg, 0x6F00, 16, iter_reg);

	/* Multi queue registers */
	nxt_chain = qla25xx_copy_mq(ha, (void *)ha->fw_dump + ha->chain_offset,
	    &last_chain);

	rval = qla24xx_soft_reset(ha);
	if (rval != QLA_SUCCESS)
		goto qla81xx_fw_dump_failed_0;

	rval = qla24xx_dump_memory(ha, fw->code_ram, sizeof(fw->code_ram),
	    &nxt);
	if (rval != QLA_SUCCESS)
		goto qla81xx_fw_dump_failed_0;

	nxt = qla2xxx_copy_queues(ha, nxt);

	nxt = qla24xx_copy_eft(ha, nxt);

	/* Chain entries -- started with MQ. */
	qla25xx_copy_fce(ha, nxt_chain, &last_chain);
	if (last_chain) {
		ha->fw_dump->version |= __constant_htonl(DUMP_CHAIN_VARIANT);
		*last_chain |= __constant_htonl(DUMP_CHAIN_LAST);
	}

qla81xx_fw_dump_failed_0:
	if (rval != QLA_SUCCESS) {
		qla_printk(KERN_WARNING, ha,
		    "Failed to dump firmware (%x)!!!\n", rval);
		ha->fw_dumped = 0;

	} else {
		qla_printk(KERN_INFO, ha,
		    "Firmware dump saved to temp buffer (%ld/%p).\n",
		    base_vha->host_no, ha->fw_dump);
		ha->fw_dumped = 1;
	}

qla81xx_fw_dump_failed:
	if (!hardware_locked)
		spin_unlock_irqrestore(&ha->hardware_lock, flags);
}

/****************************************************************************/
/*                         Driver Debug Functions.                          */
/****************************************************************************/

void
qla2x00_dump_regs(scsi_qla_host_t *vha)
{
	int i;
	struct qla_hw_data *ha = vha->hw;
	struct device_reg_2xxx __iomem *reg = &ha->iobase->isp;
	struct device_reg_24xx __iomem *reg24 = &ha->iobase->isp24;
	uint16_t __iomem *mbx_reg;

	mbx_reg = IS_FWI2_CAPABLE(ha) ? &reg24->mailbox0:
	    MAILBOX_REG(ha, reg, 0);

	printk("Mailbox registers:\n");
	for (i = 0; i < 6; i++)
		printk("scsi(%ld): mbox %d 0x%04x \n", vha->host_no, i,
		    RD_REG_WORD(mbx_reg++));
}


void
qla2x00_dump_buffer(uint8_t * b, uint32_t size)
{
	uint32_t cnt;
	uint8_t c;

	printk(" 0   1   2   3   4   5   6   7   8   9  "
	    "Ah  Bh  Ch  Dh  Eh  Fh\n");
	printk("----------------------------------------"
	    "----------------------\n");

	for (cnt = 0; cnt < size;) {
		c = *b++;
		printk("%02x",(uint32_t) c);
		cnt++;
		if (!(cnt % 16))
			printk("\n");
		else
			printk("  ");
	}
	if (cnt % 16)
		printk("\n");
}


                                                                                                                                                                                                                                                                                                                            _ZN3art8verifier14MethodVerifierD2Ev _ZN3art8verifier12RegTypeCacheD1Ev _ZN3art6mirror9ArtMethod23GetReturnTypeDescriptorEv _ZNSt3__16vectorIiNS_9allocatorIiEEE21__push_back_slow_pathIiEEvOT_ _ZNK3art11Instruction5VRegBEv _ZNK3art11Instruction5VRegCEv _ZNK3art11Instruction5VRegAEv _ZN3art8verifier14MethodVerifier19GetMethodReturnTypeEv _ZN3art8verifier12RegTypeCache22primitive_initialized_E _ZN3art8verifier12RegTypeCache16primitive_count_E _ZN3art8verifier12RegTypeCache36CreatePrimitiveAndSmallConstantTypesEv _ZN8Indenter8overflowEi _ZN3art8verifier14MethodVerifier21GetQuickInvokedMethodEPKNS_11InstructionEPNS0_12RegisterLineEb _ZN3art8verifier12RegisterLine17GetInvocationThisEPKNS_11InstructionEb _ZN3art8verifierlsERNSt3__113basic_ostreamIcNS1_11char_traitsIcEEEERKNS0_7RegTypeE _ZN3art8verifier14MethodVerifier19GetQuickFieldAccessEPKNS_11InstructionEPNS0_12RegisterLineE _ZN3art8verifier14MethodVerifier23AppendToLastFailMessageENSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE _ZN3art8verifier14MethodVerifier13LogVerifyInfoEv _ZN3art8verifier14MethodVerifier12DumpFailuresERNSt3__113basic_ostreamIcNS2_11char_traitsIcEEEE _ZN3art8veri
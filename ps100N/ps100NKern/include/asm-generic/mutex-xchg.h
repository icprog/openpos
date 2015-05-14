-ENODEV;

	/* Use power settings for Vcc and Vpp if present */
	/*  Note that the CIS values need to be rescaled */
	if (cfg->vcc.present & (1<<CISTPL_POWER_VNOM)) {
		if (cfg->vcc.param[CISTPL_POWER_VNOM]/10000 != vcc)
			return -ENODEV;
	} else if (dflt->vcc.present & (1<<CISTPL_POWER_VNOM)) {
		if (dflt->vcc.param[CISTPL_POWER_VNOM]/10000 != vcc)
			return -ENODEV;
		}

	if (cfg->vpp1.present & (1<<CISTPL_POWER_VNOM))
		p_dev->conf.Vpp =
			cfg->vpp1.param[CISTPL_POWER_VNOM]/10000;
	else if (dflt->vpp1.present & (1<<CISTPL_POWER_VNOM))
		p_dev->conf.Vpp =
			dflt->vpp1.param[CISTPL_POWER_VNOM]/10000;

	/* we need an interrupt */
	if (cfg->irq.IRQInfo1 || dflt->irq.IRQInfo1)
		p_dev->conf.Attributes |= CONF_ENABLE_IRQ;

	/* IO window settings */
	p_dev->io.NumPorts1 = p_dev->io.NumPorts2 = 0;
	if ((cfg->io.nwin > 0) || (dflt->io.nwin > 0)) {
		cistpl_io_t *io = (cfg->io.nwin) ? &cfg->io : &dflt->io;

		p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
		p_dev->io.IOAddrLines = io->flags & CISTPL_IO_LINES_MASK;
		p_dev->io.BasePort1 = io->win[0].base;
		p_dev->io.NumPorts1 = io->win[0].len;

		return pcmcia_request_io(p_dev, &p_dev->io);
	}
	pcmcia_disable_device(p_dev);
	return -ENODEV;
}


static int sl811_cs_config(struct pcmcia_device *link)
{
	struct device		*parent = &handle_to_dev(link);
	local_info_t		*dev = link->priv;
	int			last_fn, last_ret;

	DBG(0, "sl811_cs_config(0x%p)\n", link);

	if (pcmcia_loop_config(link, sl811_cs_config_check, NULL))
		goto failed;

	/* require an IRQ and two registers */
	if (!link->io.NumPorts1 || link->io.NumPorts1 < 2)
		goto failed;
	if (link->conf.Attributes & CONF_ENABLE_IRQ)
		CS_CHECK(RequestIRQ,
			pcmcia_request_irq(link, &link->irq));
	else
		goto failed;

	CS_CHECK(RequestConfiguration,
		pcmcia_request_configuration(link, &link->conf));

	sprintf(dev->node.dev_name, driver_name);
	dev->node.major = dev->node.minor = 0;
	link->dev_node = &dev->node;

	printk(KERN_INFO "%s: index 0x%02x: ",
	       dev->node.dev_name, link->conf.ConfigIndex);
	if (link->conf.Vpp)
		printk(", Vpp %d.%d", link->conf.Vpp/10, link->conf.Vpp%10);
	printk(", irq %d", link->irq.AssignedIRQ);
	printk(", io 0x%04x-0x%04x", link->io.BasePort1,
	       link->io.BasePort1+link->io.NumPorts1-1);
	printk("\n");

	if (sl811_hc_init(parent, link->io.BasePort1, link->irq.AssignedIRQ)
			< 0) {
cs_failed:
		cs_error(link, last_fn, last_ret);
failed:
		printk(KERN_WARNING "sl811_cs_config failed\n");
		sl811_cs_release(link);
		return  -ENODEV;
	}
	return 0;
}

static int sl811_cs_probe(struct pcmcia_device *link)
{
	local_info_t *local;

	local = kzalloc(sizeof(local_info_t), GFP_KERNEL);
	if (!local)
		return -ENOMEM;
	local->p_dev = link;
	link->priv = local;

	/* Initialize */
	link->irq.Attributes = IRQ_TYPE_EXCLUSIVE;
	link->irq.IRQInfo1 = IRQ_INFO2_VALID|IRQ_LEVEL_ID;
	link->irq.Handler = NULL;

	link->conf.Attributes = 0;
	link->conf.IntType = INT_MEMORY_AND_IO;

	return sl811_cs_config(link);
}

static struct pcmcia_device_id sl811_ids[] = {
	PCMCIA_DEVICE_MANF_CARD(0xc015, 0x0001), /* RATOC USB HOST CF+ Card */
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, sl811_ids);

static struct pcmcia_driver sl811_cs_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= (char *)driver_name,
	},
	.probe		= sl811_cs_probe,
	.remove		= sl811_cs_detach,
	.id_table	= sl811_ids,
};

/*====================================================================*/

static int __init init_sl811_cs(void)
{
	return pcmcia_register_driver(&sl811_cs_driver);
}
module_init(init_sl811_cs);

static void __exit exit_sl811_cs(void)
{
	pcmcia_unregister_driver(&sl811_cs_driver);
}
module_exit(exit_sl811_cs);
                                                                                     
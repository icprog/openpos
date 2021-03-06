	/* Platform hasn't set available data widths. This is bad.
		 * Warn and use a default. */
		dev_warn(&pdev->dev, "WARNING! Platform hasn't set available "
			 "data widths, using default 10 bit\n");
		pcdev->platform_flags |= PXA_CAMERA_DATAWIDTH_10;
	}
	pcdev->mclk = pcdev->pdata->mclk_10khz * 10000;
	if (!pcdev->mclk) {
		dev_warn(&pdev->dev,
			 "mclk == 0! Please, fix your platform data. "
			 "Using default 20MHz\n");
		pcdev->mclk = 20000000;
	}

	pcdev->soc_host.dev = &pdev->dev;
	pcdev->mclk_divisor = mclk_get_divisor(pcdev);

	INIT_LIST_HEAD(&pcdev->capture);
	spin_lock_init(&pcdev->lock);

	/*
	 * Request the regions.
	 */
	if (!request_mem_region(res->start, resource_size(res),
				PXA_CAM_DRV_NAME)) {
		err = -EBUSY;
		goto exit_clk;
	}

	base = ioremap(res->start, resource_size(res));
	if (!base) {
		err = -ENOMEM;
		goto exit_release;
	}
	pcdev->irq = irq;
	pcdev->base = base;

	/* request dma */
	err = pxa_request_dma("CI_Y", DMA_PRIO_HIGH,
			      pxa_camera_dma_irq_y, pcdev);
	if (err < 0) {
		dev_err(&pdev->dev, "Can't request DMA for Y\n");
		goto exit_iounmap;
	}
	pcdev->dma_chans[0] = err;
	dev_dbg(&pdev->dev, "got DMA channel %d\n", pcdev->dma_chans[0]);

	err = pxa_request_dma("CI_U", DMA_PRIO_HIGH,
			      pxa_camera_dma_irq_u, pcdev);
	if (err < 0) {
		dev_err(&pdev->dev, "Can't request DMA for U\n");
		goto exit_free_dma_y;
	}
	pcdev->dma_chans[1] = err;
	dev_dbg(&pdev->dev, "got DMA channel (U) %d\n", pcdev->dma_chans[1]);

	err = pxa_request_dma("CI_V", DMA_PRIO_HIGH,
			      pxa_camera_dma_irq_v, pcdev);
	if (err < 0) {
		dev_err(&pdev->dev, "Can't request DMA for V\n");
		goto exit_free_dma_u;
	}
	pcdev->dma_chans[2] = err;
	dev_dbg(&pdev->dev, "got DMA channel (V) %d\n", pcdev->dma_chans[2]);

	DRCMR(68) = pcdev->dma_chans[0] | DRCMR_MAPVLD;
	DRCMR(69) = pcdev->dma_chans[1] | DRCMR_MAPVLD;
	DRCMR(70) = pcdev->dma_chans[2] | DRCMR_MAPVLD;

	/* request irq */
	err = request_irq(pcdev->irq, pxa_camera_irq, 0, PXA_CAM_DRV_NAME,
			  pcdev);
	if (err) {
		dev_err(&pdev->dev, "Camera interrupt register failed \n");
		goto exit_free_dma;
	}

	pcdev->soc_host.drv_name	= PXA_CAM_DRV_NAME;
	pcdev->soc_host.ops		= &pxa_soc_camera_host_ops;
	pcdev->soc_host.priv		= pcdev;
	pcdev->soc_host.nr		= pdev->id;

	err = soc_camera_host_register(&pcdev->soc_host);
	if (err)
		goto exit_free_irq;

	return 0;

exit_free_irq:
	free_irq(pcdev->irq, pcdev);
exit_free_dma:
	pxa_free_dma(pcdev->dma_chans[2]);
exit_free_dma_u:
	pxa_free_dma(pcdev->dma_chans[1]);
exit_free_dma_y:
	pxa_free_dma(pcdev->dma_chans[0]);
exit_iounmap:
	iounmap(base);
exit_release:
	release_mem_region(res->start, resource_size(res));
exit_clk:
	clk_put(pcdev->clk);
exit_kfree:
	kfree(pcdev);
exit:
	return err;
}

static int __devexit pxa_camera_remove(struct platform_device *pdev)
{
	struct soc_camera_host *soc_host = to_soc_camera_host(&pdev->dev);
	struct pxa_camera_dev *pcdev = container_of(soc_host,
					struct pxa_camera_dev, soc_host);
	struct resource *res;

	clk_put(pcdev->clk);

	pxa_free_dma(pcdev->dma_chans[0]);
	pxa_free_dma(pcdev->dma_chans[1]);
	pxa_free_dma(pcdev->dma_chans[2]);
	free_irq(pcdev->irq, pcdev);

	soc_camera_host_unregister(soc_host);

	iounmap(pcdev->base);

	res = pcdev->res;
	release_mem_region(res->start, resource_size(res));

	kfree(pcdev);

	dev_info(&pdev->dev, "PXA Camera driver unloaded\n");

	return 0;
}

static struct platform_driver pxa_camera_driver = {
	.driver 	= {
		.name	= PXA_CAM_DRV_NAME,
	},
	.probe		= pxa_camera_probe,
	.remove		= __devexit_p(pxa_camera_remove),
};


static int __init pxa_camera_init(void)
{
	return platform_driver_register(&pxa_camera_driver);
}

static void __exit pxa_camera_exit(void)
{
	platform_driver_unregister(&pxa_camera_driver);
}

module_init(pxa_camera_init);
module_exit(pxa_camera_exit);

MODULE_DESCRIPTION("PXA27x SoC Camera Host driver");
MODULE_AUTHOR("Guennadi Liakhovetski <kernel@pengutronix.de>");
MODULE_LICENSE("GPL");
                                                                                                                                          /*
 * Host interface registers
 */

#define OXU_DEVICEID			0x00
	#define OXU_REV_MASK		0xffff0000
	#define OXU_REV_SHIFT		16
	#define OXU_REV_2100		0x2100
	#define OXU_BO_SHIFT		8
	#define OXU_BO_MASK		(0x3 << OXU_BO_SHIFT)
	#define OXU_MAJ_REV_SHIFT	4
	#define OXU_MAJ_REV_MASK	(0xf << OXU_MAJ_REV_SHIFT)
	#define OXU_MIN_REV_SHIFT	0
	#
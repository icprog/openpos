L;
		}
		break;

	default:
		dev_err(wm8350->dev,
			"WM835x configuration mode %d not supported\n",
			mode);
		return -EINVAL;
	}

	wm8350->reg_cache =
		kmalloc(sizeof(u16) * (WM8350_MAX_REGISTER + 1), GFP_KERNEL);
	if (wm8350->reg_cache == NULL)
		return -ENOMEM;

	/* Read the initial cache state back from the device - this is
	 * a PMIC so the device many not be in a virgin state and we
	 * can't rely on the silicon values.
	 */
	ret = wm8350->read_dev(wm8350, 0,
			       sizeof(u16) * (WM8350_MAX_REGISTER + 1),
			       wm8350->reg_cache);
	if (ret < 0) {
		dev_err(wm8350->dev,
			"failed to read initial cache values\n");
		goto out;
	}

	/* Mask out uncacheable/unreadable bits and the audio. */
	for (i = 0; i < WM8350_MAX_REGISTER; i++) {
		if (wm8350_reg_io_map[i].readable &&
		    (i < WM8350_CLOCK_CONTROL_1 || i > WM8350_AIF_TEST)) {
			value = be16_to_cpu(wm8350->reg_cache[i]);
			value &= wm8350_reg_io_map[i].readable;
			wm8350->reg_cache[i] = value;
		} else
			wm8350->reg_cache[i] = reg_map[i];
	}

out:
	return ret;
}

/*
 * Register a client device.  This is non-fatal since there is no need to
 * fail the entire device init due to a single platform device failing.
 */
static void wm8350_client_dev_register(struct wm8350 *wm8350,
				       const char *name,
				       struct platform_device **pdev)
{
	int ret;

	*pdev = platform_device_alloc(name, -1);
	if (pdev == NULL) {
		dev_err(wm8350->dev, "Failed to allocate %s\n", name);
		return;
	}

	(*pdev)->dev.parent = wm8350->dev;
	platform_set_drvdata(*pdev, wm8350);
	ret = platform_device_add(*pdev);
	if (ret != 0) {
		dev_err(wm8350->dev, "Failed to register %s: %d\n", name, ret);
		platform_device_put(*pdev);
		*pdev = NULL;
	}
}

int wm8350_device_init(struct wm8350 *wm8350, int irq,
		       struct wm8350_platform_data *pdata)
{
	int ret;
	u16 id1, id2, mask_rev;
	u16 cust_id, mode, chip_rev;

	/* get WM8350 revision and config mode */
	ret = wm8350->read_dev(wm8350, WM8350_RESET_ID, sizeof(id1), &id1);
	if (ret != 0) {
		dev_err(wm8350->dev, "Failed to read ID: %d\n", ret);
		goto err;
	}

	ret = wm8350->read_dev(wm8350, WM8350_ID, sizeof(id2), &id2);
	if (ret != 0) {
		dev_err(wm8350->dev, "Failed to read ID: %d\n", ret);
		goto err;
	}

	ret = wm8350->read_dev(wm8350, WM8350_REVISION, sizeof(mask_rev),
			       &mask_rev);
	if (ret != 0) {
		dev_err(wm8350->dev, "Failed to read revision: %d\n", ret);
		goto err;
	}

	id1 = be16_to_cpu(id1);
	id2 = be16_to_cpu(id2);
	mask_rev = be16_to_cpu(mask_rev);

	if (id1 != 0x6143) {
		dev_err(wm8350->dev,
			"Device with ID %x is not a WM8350\n", id1);
		ret = -ENODEV;
		goto err;
	}

	mode = id2 & WM8350_CONF_STS_MASK >> 10;
	cust_id = id2 & WM8350_CUST_ID_MASK;
	chip_rev = (id2 & WM8350_CHIP_REV_MASK) >> 12;
	dev_info(wm8350->dev,
		 "CONF_STS %d, CUST_ID %d, MASK_REV %d, CHIP_REV %d\n",
		 mode, cust_id, mask_rev, chip_rev);

	if (cust_id != 0) {
		dev_err(wm8350->dev, "Unsupported CUST_ID\n");
		ret = -ENODEV;
		goto err;
	}

	switch (mask_rev) {
	case 0:
		wm8350->pmic.max_dcdc = WM8350_DCDC_6;
		wm8350->pmic.max_isink = WM8350_ISINK_B;

		switch (chip_rev) {
		case WM8350_REV_E:
			dev_info(wm8350->dev, "WM8350 Rev E\n");
			break;
		case WM8350_REV_F:
			dev_info(wm8350->dev, "WM8350 Rev F\n");
			break;
		case WM8350_REV_G:
			dev_info(wm8350->dev, "WM8350 Rev G\n");
			wm8350->power.rev_g_coeff = 1;
			break;
		case WM8350_REV_H:
			dev_info(wm8350->dev, "WM8350 Rev H\n");
			wm8350->power.rev_g_coeff = 1;
			break;
		default:
			/* For safety we refuse to run on unknown hardware */
			dev_err(wm8350->dev, "Unknown WM8350 CHIP_REV\n");
			ret = -ENODEV;
			goto err;
		}
		break;

	case 1:
		wm8350->pmic.max_dcdc = WM8350_DCDC_4;
		wm8350->pmic.max_isink = WM8350_ISINK_A;

		switch (chip_rev) {
		case 0:
			dev_info(wm8350->dev, "WM8351 Rev A\n");
			wm8350->power.rev_g_coeff = 1;
			break;

		case 1:
			dev_info(wm8350->dev, "WM8351 Rev B\n");
			wm8350->power.rev_g_coeff = 1;
			break;

		default:
			dev_err(wm8350->dev, "Unknown WM8351 CHIP_REV\n");
			ret = -ENODEV;
			goto err;
		}
		break;

	case 2:
		wm8350->pmic.max_dcdc = WM8350_DCDC_6;
		wm8350->pmic.max_isink = WM8350_ISINK_B;

		switch (chip_rev) {
		case 0:
			dev_info(wm8350->dev, "WM8352 Rev A\n");
			wm8350->power.rev_g_coeff = 1;
			break;

		default:
			dev_err(wm8350->dev, "Unknown WM8352 CHIP_REV\n");
			ret = -ENODEV;
			goto err;
		}
		break;

	default:
		dev_err(wm8350->dev, "Unknown MASK_REV\n");
		ret = -ENODEV;
		goto err;
	}

	ret = wm8350_create_cache(wm8350, mask_rev, mode);
	if (ret < 0) {
		dev_err(wm8350->dev, "Failed to create register cache\n");
		return ret;
	}

	wm8350_reg_write(wm8350, WM8350_SYSTEM_INTERRUPTS_MASK, 0xFFFF);
	wm8350_reg_write(wm8350, WM8350_INT_STATUS_1_MASK, 0xFFFF);
	wm8350_reg_write(wm8350, WM8350_INT_STATUS_2_MASK, 0xFFFF);
	wm8350_reg_write(wm8350, WM8350_UNDER_VOLTAGE_INT_STATUS_MASK, 0xFFFF);
	wm8350_reg_write(wm8350, WM8350_GPIO_INT_STATUS_MASK, 0xFFFF);
	wm8350_reg_write(wm8350, WM8350_COMPARATOR_INT_STATUS_MASK, 0xFFFF);

	mutex_init(&wm8350->auxadc_mutex);
	mutex_init(&wm8350->irq_mutex);
	INIT_WORK(&wm8350->irq_work, wm8350_irq_worker);
	if (irq) {
		int flags = 0;

		if (pdata && pdata->irq_high) {
			flags |= IRQF_TRIGGER_HIGH;

			wm8350_set_bits(wm8350, WM8350_SYSTEM_CONTROL_1,
					WM8350_IRQ_POL);
		} else {
			flags |= IRQF_TRIGGER_LOW;

			wm8350_clear_bits(wm8350, WM8350_SYSTEM_CONTROL_1,
					  WM8350_IRQ_POL);
		}

		ret = request_irq(irq, wm8350_irq, flags,
				  "wm8350", wm8350);
		if (ret != 0) {
			dev_err(wm8350->dev, "Failed to request IRQ: %d\n",
				ret);
			goto err;
		}
	} else {
		dev_err(wm8350->dev, "No IRQ configured\n");
		goto err;
	}
	wm8350->chip_irq = irq;

	if (pdata && pdata->init) {
		ret = pdata->init(wm8350);
		if (ret != 0) {
			dev_err(wm8350->dev, "Platform init() failed: %d\n",
				ret);
			goto err;
		}
	}

	/*mask gpio and rtc interrupt*/
	wm8350_reg_write(wm8350, WM8350_SYSTEM_INTERRUPTS_MASK, 0x50);

	wm8350_client_dev_register(wm8350, "wm8350-codec",
				   &(wm8350->codec.pdev));
	wm8350_client_dev_register(wm8350, "wm8350-gpio",
				   &(wm8350->gpio.pdev));
	wm8350_client_dev_register(wm8350, "wm8350-power",
				   &(wm8350->power.pdev));
	wm8350_client_dev_register(wm8350, "wm8350-rtc", &(wm8350->rtc.pdev));
	wm8350_client_dev_register(wm8350, "wm8350-wdt", &(wm8350->wdt.pdev));

	return 0;

err:
	kfree(wm8350->reg_cache);
	return ret;
}
EXPORT_SYMBOL_GPL(wm8350_device_init);

void wm8350_device_exit(struct wm8350 *wm8350)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(wm8350->pmic.led); i++)
		platform_device_unregister(wm8350->pmic.led[i].pdev);

	for (i = 0; i < ARRAY_SIZE(wm8350->pmic.pdev); i++)
		platform_device_unregister(wm8350->pmic.pdev[i]);

	platform_device_unregister(wm8350->wdt.pdev);
	platform_device_unregister(wm8350->rtc.pdev);
	platform_device_unregister(wm8350->power.pdev);
	platform_device_unregister(wm8350->gpio.pdev);
	platform_device_unregister(wm8350->codec.pdev);

	free_irq(wm8350->chip_irq, wm8350);
	flush_work(&wm8350->irq_work);
	kfree(wm8350->reg_cache);
}
EXPORT_SYMBOL_GPL(wm8350_device_exit);

MODULE_DESCRIPTION("WM8350 AudioPlus PMIC core driver");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
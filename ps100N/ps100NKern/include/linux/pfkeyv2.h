rightness_set = xpad_led_set;

	error = led_classdev_register(&xpad->udev->dev, led_cdev);
	if (error) {
		kfree(led);
		xpad->led = NULL;
		return error;
	}

	/*
	 * Light up the segment corresponding to controller number
	 */
	xpad_send_led_command(xpad, (led_no % 4) + 2);

	return 0;
}

static void xpad_led_disconnect(struct usb_xpad *xpad)
{
	struct xpad_led *xpad_led = xpad->led;

	if (xpad_led) {
		led_classdev_unregister(&xpad_led->led_cdev);
		kfree(xpad_led->name);
	}
}
#else
static int xpad_led_probe(struct usb_xpad *xpad) { return 0; }
static void xpad_led_disconnect(struct usb_xpad *xpad) { }
#endif


static int xpad_open(struct input_dev *dev)
{
	struct usb_xpad *xpad = input_get_drvdata(dev);

	/* URB was submitted in probe */
	if(xpad->xtype == XTYPE_XBOX360W)
		return 0;

	xpad->irq_in->dev = xpad->udev;
	if (usb_submit_urb(xpad->irq_in, GFP_KERNEL))
		return -EIO;

	return 0;
}

static void xpad_close(struct input_dev *dev)
{
	struct usb_xpad *xpad = input_get_drvdata(dev);

	if(xpad->xtype != XTYPE_XBOX360W)
		usb_kill_urb(xpad->irq_in);
	xpad_stop_output(xpad);
}

static void xpad_set_up_abs(struct input_dev *input_dev, signed short abs)
{
	set_bit(abs, input_dev->absbit);

	switch (abs) {
	case ABS_X:
	case ABS_Y:
	case ABS_RX:
	case ABS_RY:	/* the two sticks */
		input_set_abs_params(input_dev, abs, -32768, 32767, 16, 128);
		break;
	case ABS_Z:
	case ABS_RZ:	/* the triggers */
		input_set_abs_params(input_dev, abs, 0, 255, 0, 0);
		break;
	case ABS_HAT0X:
	case ABS_HAT0Y:	/* the d-pad (only if MAP_DPAD_TO_AXES) */
		input_set_abs_params(input_dev, abs, -1, 1, 0, 0);
		break;
	}
}

static int xpad_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_xpad *xpad;
	struct input_dev *input_dev;
	struct usb_endpoint_descriptor *ep_irq_in;
	int i;
	int error = -ENOMEM;

	for (i = 0; xpad_device[i].idVendor; i++) {
		if ((le16_to_cpu(udev->descriptor.idVendor) == xpad_device[i].idVendor) &&
		    (le16_to_cpu(udev->descriptor.idProduct) == xpad_device[i].idProduct))
			break;
	}

	xpad = kzalloc(sizeof(struct usb_xpad), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!xpad || !input_dev)
		goto fail1;

	xpad->idata = usb_buffer_alloc(udev, XPAD_PKT_LEN,
				       GFP_KERNEL, &xpad->idata_dma);
	if (!xpad->idata)
		goto fail1;

	xpad->irq_in = usb_alloc_urb(0, GFP_KERNEL);
	if (!xpad->irq_in)
		goto fail2;

	xpad->udev = udev;
	xpad->dpad_mapping = xpad_device[i].dpad_mapping;
	xpad->xtype = xpad_device[i].xtype;
	if (xpad->dpad_mapping == MAP_DPAD_UNKNOWN)
		xpad->dpad_mapping = !dpad_to_buttons;
	if (xpad->xtype == XTYPE_UNKNOWN) {
		if (intf->cur_altsetting->desc.bInterfaceClass == USB_CLASS_VENDOR_SPEC) {
			if (intf->cur_altsetting->desc.bInterfaceProtocol == 129)
				xpad->xtype = XTYPE_XBOX360W;
			else
				xpad->xtype = XTYPE_XBOX360;
		} else
			xpad->xtype = XTYPE_XBOX;
	}
	xpad->dev = input_dev;
	usb_make_path(udev, xpad->phys, sizeof(xpad->phys));
	strlcat(xpad->phys, "/input0", sizeof(xpad->phys));

	input_dev->name = xpad_device[i].name;
	input_dev->phys = xpad->phys;
	usb_to_input_id(udev, &input_dev->id);
	input_dev->dev.parent = &intf->dev;

	input_set_drvdata(input_dev, xpad);

	input_dev->open = xpad_open;
	input_dev->close = xpad_close;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);

	/* set up buttons */
	for (i = 0; xpad_common_btn[i] >= 0; i++)
		set_bit(xpad_common_btn[i], input_dev->keybit);
	if ((xpad->xtype == XTYPE_XBOX360) || (xpad->xtype == XTYPE_XBOX360W))
		for (i = 0; xpad360_btn[i] >= 0; i++)
			set_bit(xpad360_btn[i], input_dev->keybit);
	else
		for (i = 0; xpad_btn[i] >= 0; i++)
			set_bit(xpad_btn[i], input_dev->keybit);
	if (xpad->dpad_mapping == MAP_DPAD_TO_BUTTONS)
		for (i = 0; xpad_btn_pad[i] >= 0; i++)
			set_bit(xpad_btn_pad[i], input_dev->keybit);

	/* set up axes */
	for (i = 0; xpad_abs[i] >= 0; i++)
		xpad_set_up_abs(input_dev, xpad_abs[i]);
	if (xpad->dpad_mapping == MAP_DPAD_TO_AXES)
		for (i = 0; xpad_abs_pad[i] >= 0; i++)
		    xpad_set_up_abs(input_dev, xpad_abs_pad[i]);

	error = xpad_init_output(intf, xpad);
	if (error)
		goto fail2;

	error = xpad_init_ff(xpad);
	if (error)
		goto fail3;

	error = xpad_led_probe(xpad);
	if (error)
		goto fail3;

	ep_irq_in = &intf->cur_altsetting->endpoint[0].desc;
	usb_fill_int_urb(xpad->irq_in, udev,
			 usb_rcvintpipe(udev, ep_irq_in->bEndpointAddress),
			 xpad->idata, XPAD_PKT_LEN, xpad_irq_in,
			 xpad, ep_irq_in->bInterval);
	xpad->irq_in->transfer_dma = xpad->idata_dma;
	xpad->irq_in->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	error = input_register_device(xpad->dev);
	if (error)
		goto fail4;

	usb_set_intfdata(intf, xpad);

	/*
	 * Submit the int URB immediatly rather than waiting for open
	 * because we get status messages from the device whether
	 * or not any controllers are attached.  In fact, it's
	 * exactly the message that a controller has arrived that
	 * we're waiting for.
	 */
	if (xpad->xtype == XTYPE_XBOX360W) {
		xpad->irq_in->dev = xpad->udev;
		error = usb_submit_urb(xpad->irq_in, GFP_KERNEL);
		if (error)
			goto fail4;

		/*
		 * Setup the message to set the LEDs on the
		 * controller when it shows up
		 */
		xpad->bulk_out = usb_alloc_urb(0, GFP_KERNEL);
		if(!xpad->bulk_out)
			goto fail5;

		xpad->bdata = kzalloc(XPAD_PKT_LEN, GFP_KERNEL);
		if(!xpad->bdata)
			goto fail6;

		xpad->bdata[2] = 0x08;
		switch (intf->cur_altsetting->desc.bInterfaceNumber) {
		case 0:
			xpad->bdata[3] = 0x42;
			break;
		case 2:
			xpad->bdata[3] = 0x43;
			break;
		case 4:
			xpad->bdata[3] = 0x44;
			break;
		case 6:
			xpad->bdata[3] = 0x45;
		}

		ep_irq_in = &intf->cur_altsetting->endpoint[1].desc;
		usb_fill_bulk_urb(xpad->bulk_out, udev,
				usb_sndbulkpipe(udev, ep_irq_in->bEndpointAddress),
				xpad->bdata, XPAD_PKT_LEN, xpad_bulk_out, xpad);
	}

	return 0;

 fail6:	usb_free_urb(xpad->bulk_out);
 fail5:	usb_kill_urb(xpad->irq_in);
 fail4:	usb_free_urb(xpad->irq_in);
 fail3:	xpad_deinit_output(xpad);
 fail2:	usb_buffer_free(udev, XPAD_PKT_LEN, xpad->idata, xpad->idata_dma);
 fail1:	input_free_device(input_dev);
	kfree(xpad);
	return error;

}

static void xpad_disconnect(struct usb_interface *intf)
{
	struct usb_xpad *xpad = usb_get_intfdata (intf);

	usb_set_intfdata(intf, NULL);
	if (xpad) {
		xpad_led_disconnect(xpad);
		input_unregister_device(xpad->dev);
		xpad_deinit_output(xpad);
		if (xpad->xtype == XTYPE_XBOX360W) {
			usb_kill_urb(xpad->bulk_out);
			usb_free_urb(xpad->bulk_out);
			usb_kill_urb(xpad->irq_in);
		}
		usb_free_urb(xpad->irq_in);
		usb_buffer_free(xpad->udev, XPAD_PKT_LEN,
				xpad->idata, xpad->idata_dma);
		kfree(xpad);
	}
}

static struct usb_driver xpad_driver = {
	.name		= "xpad",
	.probe		= xpad_probe,
	.disconnect	= xpad_disconnect,
	.id_table	= xpad_table,
};

static int __init usb_xpad_init(void)
{
	int result = usb_register(&xpad_driver);
	if (result == 0)
		printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_DESC "\n");
	return result;
}

static void __exit usb_xpad_exit(void)
{
	usb_deregister(&xpad_driver);
}

module_init(usb_xpad_init);
module_exit(usb_xpad_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       /*
 * EHCI HCD (Host Controller Driver) for USB.
 *
 * Bus Glue for AMD Alchemy Au1xxx
 *
 * Based on "ohci-au1xxx.c" by Matt Porter <mporter@kernel.crashing.org>
 *
 * Modified for AMD Alchemy Au1200 EHC
 *  by K.Boge <karsten.boge@amd.com>
 *
 * This file is licenced under the GPL.
 */

#include <linux/platform_device.h>
#include <asm/mach-au1x00/au1000.h>

#define USB_HOST_CONFIG   (USB_MSR_BASE + USB_MSR_MCFG)
#define USB_MCFG_PFEN     (1<<31)
#define USB_MCFG_RDCOMB   (1<<30)
#define USB_MCFG_SSDEN    (1<<23)
#define USB_MCFG_PHYPLLEN (1<<19)
#define USB_MCFG_UCECLKEN (1<<18)
#define USB_MCFG_EHCCLKEN (1<<17)
#ifdef CONFIG_DMA_COHERENT
#define USB_MCFG_UCAM     (1<<7)
#else
#define USB_MCFG_UCAM     (0)
#endif
#define USB_MCFG_EBMEN    (1<<3)
#define USB_MCFG_EMEMEN   (1<<2)

#define USBH_ENABLE_CE	(USB_MCFG_PHYPLLEN | USB_MCFG_EHCCLKEN)
#define USBH_ENABLE_INIT (USB_MCFG_PFEN  | USB_MCFG_RDCOMB |	\
			  USBH_ENABLE_CE | USB_MCFG_SSDEN  |	\
			  USB_MCFG_UCAM  | USB_MCFG_EBMEN  |	\
			  USB_MCFG_EMEMEN)

#define USBH_DISABLE      (USB_MCFG_EBMEN | USB_MCFG_EMEMEN)

extern int usb_disabled(void);

static void au1xxx_start_ehc(void)
{
	/* enable clock to EHCI block and HS PHY PLL*/
	au_writel(au_readl(USB_HOST_CONFIG) | USBH_ENABLE_CE, USB_HOST_CONFIG);
	au_sync();
	udelay(1000);

	/* enable EHCI mmio */
	au_writel(au_readl(USB_HOST_CONFIG) | USBH_ENABLE_INIT, USB_HOST_CONFIG);
	au_sync();
	udelay(1000);
}

static void au1xxx_stop_ehc(void)
{
	unsigned long c;

	/* Disable mem */
	au_writel(au_readl(USB_HOST_CONFIG) & ~USBH_DISABLE, USB_HOST_CONFIG);
	au_sync();
	udelay(1000);

	/* Disable EHC clock. If the HS PHY is unused disable it too. */
	c = au_readl(USB_HOST_CONFIG) & ~USB_MCFG_EHCCLKEN;
	if (!(c & USB_MCFG_UCECLKEN))		/* UDC disabled? */
		c &= ~USB_MCFG_PHYPLLEN;	/* yes: disable HS PHY PLL */
	au_writel(c, USB_HOST_CONFIG);
	au_sync();
}

static const struct hc_driver ehci_
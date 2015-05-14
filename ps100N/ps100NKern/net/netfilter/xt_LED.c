ail;

	if (!(iforce->out = usb_alloc_urb(0, GFP_KERNEL)))
		goto fail;

	if (!(iforce->ctrl = usb_alloc_urb(0, GFP_KERNEL)))
		goto fail;

	iforce->bus = IFORCE_USB;
	iforce->usbdev = dev;

	iforce->cr.bRequestType = USB_TYPE_VENDOR | USB_DIR_IN | USB_RECIP_INTERFACE;
	iforce->cr.wIndex = 0;
	iforce->cr.wLength = cpu_to_le16(16);

	usb_fill_int_urb(iforce->irq, dev, usb_rcvintpipe(dev, epirq->bEndpointAddress),
			iforce->data, 16, iforce_usb_irq, iforce, epirq->bInterval);

	usb_fill_int_urb(iforce->out, dev, usb_sndintpipe(dev, epout->bEndpointAddress),
			iforce + 1, 32, iforce_usb_out, iforce, epout->bInterval);

	usb_fill_control_urb(iforce->ctrl, dev, usb_rcvctrlpipe(dev, 0),
			(void*) &iforce->cr, iforce->edata, 16, iforce_usb_ctrl, iforce);

	err = iforce_init_device(iforce);
	if (err)
		goto fail;

	usb_set_intfdata(intf, iforce);
	return 0;

fail:
	if (iforce) {
		usb_free_urb(iforce->irq);
		usb_free_urb(iforce->out);
		usb_free_urb(iforce->ctrl);
		kfree(iforce);
	}

	return err;
}

/* Called by iforce_delete() */
void iforce_usb_delete(struct iforce* iforce)
{
	usb_kill_urb(iforce->irq);
	usb_kill_urb(iforce->out);
	usb_kill_urb(iforce->ctrl);

	usb_free_urb(iforce->irq);
	usb_free_urb(iforce->out);
	usb_free_urb(iforce->ctrl);
}

static void iforce_usb_disconnect(struct usb_interface *intf)
{
	struct iforce *iforce = usb_get_intfdata(intf);
	int open = 0; /* FIXME! iforce->dev.handle->open; */

	usb_set_intfdata(intf, NULL);
	if (iforce) {
		iforce->usbdev = NULL;
		input_unregister_device(iforce->dev);

		if (!open) {
			iforce_delete_device(iforce);
			kfree(iforce);
		}
	}
}

static struct usb_device_id iforce_usb_ids [] = {
	{ USB_DEVICE(0x044f, 0xa01c) },		/* Thrustmaster Motor Sport GT */
	{ USB_DEVICE(0x046d, 0xc281) },		/* Logitech WingMan Force */
	{ USB_DEVICE(0x046d, 0xc291) },		/* Logitech WingMan Formula Force */
	{ USB_DEVICE(0x05ef, 0x020a) },		/* AVB Top Shot Pegasus */
	{ USB_DEVICE(0x05ef, 0x8884) },		/* AVB Mag Turbo Force */
	{ USB_DEVICE(0x05ef, 0x8888) },		/* AVB Top Shot FFB Racing Wheel */
	{ USB_DEVICE(0x061c, 0xc0a4) },         /* ACT LABS Force RS */
	{ USB_DEVICE(0x061c, 0xc084) },         /* ACT LABS Force RS */
	{ USB_DEVICE(0x06f8, 0x0001) },		/* Guillemot Race Leader Force Feedback */
	{ USB_DEVICE(0x06f8, 0x0004) },		/* Guillemot Force Feedback Racing Wheel */
	{ USB_DEVICE(0x06f8, 0xa302) },		/* Guillemot Jet Leader 3D */
	{ }					/* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, iforce_usb_ids);

struct usb_driver iforce_usb_driver = {
	.name =		"iforce",
	.probe =	iforce_usb_probe,
	.disconnect =	iforce_usb_disconnect,
	.id_table =	iforce_usb_ids,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    /*
 * intelfb
 *
 * Linux framebuffer driver for Intel(R) 865G integrated graphics chips.
 *
 * Copyright © 2002, 2003 David Dawes <dawes@xfree86.org>
 *                   2004 Sylvain Meyer
 *
 * This driver consists of two parts.  The first part (intelfbdrv.c) provides
 * the basic fbdev interfaces, is derived in part from the radeonfb and
 * vesafb drivers, and is covered by the GPL.  The second part (intelfbhw.c)
 * provides the code 
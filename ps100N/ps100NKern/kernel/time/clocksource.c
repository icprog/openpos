en = 1;		/* only one open per device */

	DEBUGP(2, dev, "<- cmm_open\n");
	ret = nonseekable_open(inode, filp);
out:
	unlock_kernel();
	return ret;
}

static int cmm_close(struct inode *inode, struct file *filp)
{
	struct cm4000_dev *dev;
	struct pcmcia_device *link;
	int minor = iminor(inode);

	if (minor >= CM4000_MAX_DEV)
		return -ENODEV;

	link = dev_table[minor];
	if (link == NULL)
		return -ENODEV;

	dev = link->priv;

	DEBUGP(2, dev, "-> cmm_close(maj/min=%d.%d)\n",
	       imajor(inode), minor);

	stop_monitor(dev);

	ZERO_DEV(dev);

	link->open = 0;		/* only one open per device */
	wake_up(&dev->devq);	/* socket removed? */

	DEBUGP(2, dev, "cmm_close\n");
	return 0;
}

static void cmm_cm4000_release(struct pcmcia_device * link)
{
	struct cm4000_dev *dev = link->priv;

	/* dont terminate the monitor, rather rely on
	 * close doing that for us.
	 */
	DEBUGP(3, dev, "-> cmm_cm4000_release\n");
	while (link->open) {
		printk(KERN_INFO MODULE_NAME ": delaying release until "
		       "process has terminated\n");
		/* note: don't interrupt us:
		 * close the applications which own
		 * the devices _first_ !
		 */
		wait_event(dev->devq, (link->open == 0));
	}
	/* dev->devq=NULL;	this cannot be zeroed earlier */
	DEBUGP(3, dev, "<- cmm_cm4000_release\n");
	return;
}

/*==== Interface to PCMCIA Layer =======================================*/

static int cm4000_config_check(struct pcmcia_device *p_dev,
			       cistpl_cftable_entry_t *cfg,
			       cistpl_cftable_entry_t *dflt,
			       unsigned int vcc,
			       void *priv_data)
{
	if (!cfg->io.nwin)
		return -ENODEV;

	/* Get the IOaddr */
	p_dev->io.BasePort1 = cfg->io.win[0].base;
	p_dev->io.NumPorts1 = cfg->io.win[0].len;
	p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
	if (!(cfg->io.flags & CISTPL_IO_8BIT))
		p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_16;
	if (!(cfg->io.flags & CISTPL_IO_16BIT))
		p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
	p_dev->io.IOAddrLines = cfg->io.flags & CISTPL_IO_LINES_MASK;

	return pcmcia_request_io(p_dev, &p_dev->io);
}

static int cm4000_config(struct pcmcia_device * link, int devno)
{
	struct cm4000_dev *dev;

	/* read the config-tuples */
	if (pcmcia_loop_config(link, cm4000_config_check, NULL))
		goto cs_release;

	link->conf.IntType = 00000002;

	if (pcmcia_request_configuration(link, &link->conf))
		goto cs_release;

	dev = link->priv;
	sprintf(dev->node.dev_name, DEVICE_NAME "%d", devno);
	dev->node.major = major;
	dev->node.minor = devno;
	dev->node.next = NULL;
	link->dev_node = &dev->node;

	return 0;

cs_release:
	cm4000_release(link);
	return -ENODEV;
}

static int cm4000_suspend(struct pcmcia_device *link)
{
	struct cm4000_dev *dev;

	dev = link->priv;
	stop_monitor(dev);

	return 0;
}

static int cm4000_resume(struct pcmcia_device *link)
{
	struct cm4000_dev *dev;

	dev = link->priv;
	if (link->open)
		start_monitor(dev);

	return 0;
}

static void cm4000_release(struct pcmcia_device *link)
{
	cmm_cm4000_release(link);	/* delay release until device closed */
	pcmcia_disable_device(link);
}

static int cm4000_probe(struct pcmcia_device *link)
{
	struct cm4000_dev *dev;
	int i, ret;

	for (i = 0; i < CM4000_MAX_DEV; i++)
		if (dev_table[i] == NULL)
			break;

	if (i == CM4000_MAX_DEV) {
		printk(KERN_NOTICE MODULE_NAME ": all devices in use\n");
		return -ENODEV;
	}

	/* create a new cm4000_cs device */
	dev = kzalloc(sizeof(struct cm4000_dev), GFP_KERNEL);
	if (dev == NULL)
		return -ENOMEM;

	dev->p_dev = link;
	link->priv = dev;
	link->conf.IntType = INT_MEMORY_AND_IO;
	dev_table[i] = link;

	init_waitqueue_head(&dev->devq);
	init_waitqueue_head(&dev->ioq);
	init_waitqueue_head(&dev->atrq);
	init_waitqueue_head(&dev->readq);

	ret = cm4000_config(link, i);
	if (ret) {
		dev_table[i] = NULL;
		kfree(dev);
		return ret;
	}

	device_create(cmm_class, NULL, MKDEV(major, i), NULL, "cmm%d", i);

	return 0;
}

static void cm4000_detach(struct pcmcia_device *link)
{
	struct cm4000_dev *dev = link->priv;
	int devno;

	/* find device */
	for (devno = 0; devno < CM4000_MAX_DEV; devno++)
		if (dev_table[devno] == link)
			break;
	if (devno == CM4000_MAX_DEV)
		return;

	stop_monitor(dev);

	cm4000_release(link);

	dev_table[devno] = NULL;
	kfree(dev);

	device_destroy(cmm_class, MKDEV(major, devno));

	return;
}

static const struct file_operations cm4000_fops = {
	.owner	= THIS_MODULE,
	.read	= cmm_read,
	.write	= cmm_write,
	.unlocked_ioctl	= cmm_ioctl,
	.open	= cmm_open,
	.release= cmm_close,
};

static struct pcmcia_device_id cm4000_ids[] = {
	PCMCIA_DEVICE_MANF_CARD(0x0223, 0x0002),
	PCMCIA_DEVICE_PROD_ID12("CardMan", "4000", 0x2FB368CA, 0xA2BD8C39),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, cm4000_ids);

static struct pcmcia_driver cm4000_driver = {
	.owner	  = THIS_MODULE,
	.drv	  = {
		.name = "cm4000_cs",
		},
	.probe    = cm4000_probe,
	.remove   = cm4000_detach,
	.suspend  = cm4000_suspend,
	.resume   = cm4000_resume,
	.id_table = cm4000_ids,
};

static int __init cmm_init(void)
{
	int rc;

	printk(KERN_INFO "%s\n", version);

	cmm_class = class_create(THIS_MODULE, "cardman_4000");
	if (IS_ERR(cmm_class))
		return PTR_ERR(cmm_class);

	major = register_chrdev(0, DEVICE_NAME, &cm4000_fops);
	if (major < 0) {
		printk(KERN_WARNING MODULE_NAME
			": could not get major number\n");
		class_destroy(cmm_class);
		return major;
	}

	rc = pcmcia_register_driver(&cm4000_driver);
	if (rc < 0) {
		unregister_chrdev(major, DEVICE_NAME);
		class_destroy(cmm_class);
		return rc;
	}

	return 0;
}

static void __exit cmm_exit(void)
{
	printk(KERN_INFO MODULE_NAME ": unloading\n");
	pcmcia_unregister_driver(&cm4000_driver);
	unregister_chrdev(major, DEVICE_NAME);
	class_destroy(cmm_class);
};

module_init(cmm_init);
module_exit(cmm_exit);
MODULE_LICENSE("Dual BSD/GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         INDX( 	                 (   �   �                             �?     � j     �?     n(�_<��G*:%
�����_<���F�����        �               v e r m i l i o n . h . s v n - b a s e                     �?     n(�_<��G*:%
�����_<���F�����        �               v e r m i l i o n . h . s v n - b a s e                     n(�_<��G*:%
�����_<���F�����        �               v e r m i l i o n . h . s v n - b a s e                     n(�_<��G*:%
�����_<���F�����        �              v e r m i l i o n . h . s v n - b a s e                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              /* $XConsortium: nv_driver.c /main/3 1996/10/28 05:13:37 kaleb $ */
/*
 * Copyright 1996-1997  David J. McKay
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * DAVID J. MCKAY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * GPL licensing note -- nVidia is allowing a liberal interpretation of
 * the documentation restriction above, to merely say that this nVidia's
 * copyright and disclaimer should be included with all code derived
 * from this source.  -- Jeff Garzik <jgarzik@pobox.com>, 01/Nov/99 
 */

/* Hacked together from mga driver and 3.3.4 NVIDIA driver by Jarno Paananen
   <jpaana@s2.org> */

/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_setup.c,v 1.18 2002/08/0
5 20:47:06 mvojkovi Exp $ */

#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include "nv_type.h"
#include "rivafb.h"
#include "nvreg.h"

#define PFX "rivafb: "

static inline unsigned char MISCin(struct riva_par *par)
{
	return (VGA_RD08(par->riva.PVIO, 0x3cc));
}

static Bool 
riva_is_connected(struct riva_par *par, Bool second)
{
	volatile U032 __iomem *PRAMDAC = par->riva.PRAMDAC0;
	U032 reg52C, reg608;
	Bool present;

	if(second) PRAMDAC += 0x800;

	reg52C = NV_RD32(PRAMDAC, 0x052C);
	reg608 = NV_RD32(PRAMDAC, 0x0608);

	NV_WR32(PRAMDAC, 0x0608, reg608 & ~0x00010000);

	NV_WR32(PRAMDAC, 0x052C, reg52C & 0x0000FEEE);
	mdelay(1); 
	NV_WR32(PRAMDAC, 0x052C, NV_RD32(PRAMDAC, 0x052C) | 1);

	NV_WR32(par->riva.PRAMDAC0, 0x0610, 0x94050140);
	NV_WR32(par->riva.PRAMDAC0, 0x0608, 0x00001000);

	mdelay(1);

	present = (NV_RD32(PRAMDAC, 0x0608) & (1 << 28)) ? TRUE : FALSE;

	NV_WR32(par->riva.PRAMDAC0, 0x0608,
		NV_RD32(par->riva.PRAMDAC0, 0x0608) & 0x0000EFFF);

	NV_WR32(PRAMDAC, 0x052C, reg52C);
	NV_WR32(PRAMDAC, 0x0608, reg608);

	return present;
}

static void
riva_override_CRTC(struct riva_par *par)
{
	printk(KERN_INFO PFX
		"Detected CRTC controller %i being used\n",
		par->SecondCRTC ? 1 : 0);

	if(par->forceCRTC != -1) {
		printk(KERN_INFO PFX
			"Forcing usage of CRTC %i\n", par->forceCRTC);
		par->SecondCRTC = par->forceCRTC;
	}
}

static void
riva_is_second(struct riva_par *par)
{
	if (par->FlatPanel == 1) {
		switch(par->Chipset & 0xffff) {
		case 0x0174:
		case 0x0175:
		case 0x0176:
		case 0x0177:
		case 0x0179:
		case 0x017C:
		case 0x017D:
		case 0x0186:
		case 0x0187:
		/* this might not be a good default for the chips below */
		case 0x0286:
		case 0x028C:
		case 0x0316:
		case 0x0317:
		case 0x031A:
		case 0x031B:
		case 0x031C:
		case 0x031D:
		case 0x031E:
		case 0x031F:
		case 0x0324:
		case 0x0325:
		case 0x0328:
		case 0x0329:
		case 0x032C:
		case 0x032D:
			par->SecondCRTC = TRUE;
			break;
		default:
			par->SecondCRTC = FALSE;
			break;
		}
	} else {
		if(riva_is_connected(par, 0)) {

			if (NV_RD32(par->riva.PRAMDAC0, 0x0000052C) & 0x100)
				par->SecondCRTC = TRUE;
			else
				par->SecondCRTC = FALSE;
		} else 
		if (riva_is_connected(par, 1)) {
			if(NV_RD32(par->riva.PRAMDAC0, 0x0000252C) & 0x100)
				par->SecondCRTC = TRUE;
			else
				par->SecondCRTC = FALSE;
		} else /* default */
			par->SecondCRTC = FALSE;
	}
	riva_override_CRTC(par);
}

unsigned lion
 * @str:	override name
 *
 * DEPRECATED! Takes a clock= boot argument and uses it
 * as the clocksource override name
 */
static int __init boot_override_clock(char* str)
{
	if (!strcmp(str, "pmtmr")) {
		printk("Warning: clock=pmtmr is deprecated. "
			"Use clocksource=acpi_pm.\n");
		return boot_override_clocksource("acpi_pm");
	}
	printk("Warning! clock= boot option is deprecated. "
		"Use clocksource=xyz\n");
	return boot_override_clocksource(str);
}

__setup("clock=", boot_override_clock);

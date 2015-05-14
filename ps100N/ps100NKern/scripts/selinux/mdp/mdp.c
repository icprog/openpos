r;

	unregister_framebuffer(info);
	fb_dealloc_cmap(&info->cmap);

	of_iounmap(&op->resource[0], par->regs, sizeof(struct p9100_regs));
	of_iounmap(&op->resource[2], info->screen_base, info->fix.smem_len);

	framebuffer_release(info);

	dev_set_drvdata(&op->dev, NULL);

	return 0;
}

static const struct of_device_id p9100_match[] = {
	{
		.name = "p9100",
	},
	{},
};
MODULE_DEVICE_TABLE(of, p9100_match);

static struct of_platform_driver p9100_driver = {
	.name		= "p9100",
	.match_table	= p9100_match,
	.probe		= p9100_probe,
	.remove		= __devexit_p(p9100_remove),
};

static int __init p9100_init(void)
{
	if (fb_get_options("p9100fb", NULL))
		return -ENODEV;

	return of_register_driver(&p9100_driver, &of_bus_type);
}

static void __exit p9100_exit(void)
{
	of_unregister_driver(&p9100_driver);
}

module_init(p9100_init);
module_exit(p9100_exit);

MODULE_DESCRIPTION("framebuffer driver for P9100 chipsets");
MODULE_AUTHOR("David S. Miller <davem@davemloft.net>");
MODULE_VERSION("2.0");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   #ifndef _AHA1542_H

/* $Id: aha1542.h,v 1.1 1992/07/24 06:27:38 root Exp root $
 *
 * Header file for the adaptec 1542 driver for Linux
 *
 * $Log: aha1542.h,v $
 * Revision 1.1  1992/07/24  06:27:38  root
 * Initial revision
 *
 * Revision 1.2  1992/07/04  18:41:49  root
 * Replaced distribution with current drivers
 *
 * Revision 1.3  1992/06/23  23:58:20  root
 * Fixes.
 *
 * Revision 1.2  1992/05/26  22:13:23  root
 * Changed bug that prevented DMA above first 2 mbytes.
 *
 * Revision 1.1  1992/05/22  21:00:29  root
 * Initial revision
 *
 * Revision 1.1  1992/04/24  18:01:50  root
 * Initial revision
 *
 * Revision 1.1  1992/04/02  03:23:13  drew
 * Initial revision
 *
 * Revision 1.3  1992/01/27  14:46:29  tthorn
 * *** empty log message ***
 *
 */

#include <linux/types.h>

/* I/O Port interface 4.2 */
/* READ */
#define STATUS(base) base
#define STST	0x80		/* Self Test in Progress */
#define DIAGF	0x40		/* Internal Diagnostic Failure */
#define INIT	0x20		/* Mailbox Initialization Required */
#define IDLE	0x10		/* SCSI Host Adapter Idle */
#define CDF	0x08		/* Command/Data Out Port Full */
#define DF	0x04		/* Data In Port Full */
#define INVDCMD	0x01		/* Invalid H A Command */
#define STATMASK 0xfd		/* 0x02 is reserved */

#define INTRFLAGS(base) (STATUS(base)+2)
#define ANYINTR	0x80		/* Any Interrupt */
#define SCRD	0x08		/* SCSI Reset Detected */
#define HACC	0x04		/* HA Command Complete */
#define MBOA	0x02		/* MBO Empty */
#define MBIF	0x01		/* MBI Full */
#define INTRMASK 0x8f

/* WRITE */
#define CONTROL(base) STATUS(base)
#define HRST	0x80		/* Hard Reset */
#define SRST	0x40		/* Soft Reset */
#define IRST	0x20		/* Interrupt Reset */
#define SCRST	0x10		/* SCSI Bus Reset */

/* READ/WRITE */
#define DATA(base) (STATUS(base)+1)
#define CMD_NOP		0x00	/* No Operation */
#define CMD_MBINIT	0x01	/* Mailbox Initialization */
#define CMD_START_SCSI	0x
*/
				NLP_SET_FREE_REQ(ndlp);
			else {
				/* Skip this if ndlp is already in free mode */
				spin_unlock_irq(&phba->ndlp_lock);
				goto skip_logo;
			}
			spin_unlock_irq(&phba->ndlp_lock);
		}
		vport->unreg_vpi_cmpl = VPORT_INVAL;
		timeout = msecs_to_jiffies(phba->fc_ratov * 2000);
		if (!lpfc_issue_els_npiv_logo(vport, ndlp))
			while (vport->unreg_vpi_cmpl == VPORT_INVAL && timeout)
				timeout = schedule_timeout(timeout);
	}

	if (!(phba->pport->load_flag & FC_UNLOADING))
		lpfc_discovery_wait(vport);

skip_logo:
	lpfc_cleanup(vport);
	lpfc_sli_host_down(vport);

	lpfc_stop_vport_timers(vport);

	if (!(phba->pport->load_flag & FC_UNLOADING)) {
		lpfc_unreg_all_rpis(vport);
		lpfc_unreg_default_rpis(vport);
		/*
		 * Completion of unreg_vpi (lpfc_mbx_cmpl_unreg_vpi)
		 * does the scsi_host_put() to release the vport.
		 */
		if (lpfc_mbx_unreg_vpi(vport))
			scsi_host_put(shost);
	} else
		scsi_host_put(shost);

	lpfc_free_vpi(phba, vport->vpi);
	vport->work_port_events = 0;
	spin_lock_irq(&phba->hbalock);
	list_del_init(&vport->listentry);
	spin_unlock_irq(&phba->hbalock);
	lpfc_printf_vlog(vport, KERN_ERR, LOG_VPORT,
			 "1828 Vport Deleted.\n");
	scsi_host_put(shost);
	return VPORT_OK;
}

struct lpfc_vport **
lpfc_create_vport_work_array(struct lpfc_hba *phba)
{
	struct lpfc_vport *port_iterator;
	struct lpfc_vport **vports;
	int index = 0;
	vports = kzalloc((phba->max_vports + 1) * sizeof(struct lpfc_vport *),
			 GFP_KERNEL);
	if (vports == NULL)
		return NULL;
	spin_lock_irq(&phba->hbalock);
	list_for_each_entry(port_iterator, &phba->port_list, listentry) {
		if (!scsi_host_get(lpfc_shost_from_vport(port_iterator))) {
			lpfc_printf_vlog(port_iterator, KERN_WARNING, LOG_VPORT,
					 "1801 Create vport work array FAILED: "
					 "cannot do scsi_host_get\n");
			continue;
		}
		vports[index++] = port_iterator;
	}
	spin_unlock_irq(&phba->hbalock);
	return vports;
}

void
lpfc_destroy_vport_work_array(struct lpfc_hba *phba, struct lpfc_vport **vports)
{
	int i;
	if (vports == NULL)
		return;
	for (i = 0; vports[i] != NULL && i <= phba->max_vports; i++)
		scsi_host_put(lpfc_shost_from_vport(vports[i]));
	kfree(vports);
}


/**
 * lpfc_vport_reset_stat_data - Reset the statistical data for the vport
 * @vport: Pointer to vport object.
 *
 * This function resets the statistical data for the vport. This function
 * is called with the host_lock held
 **/
void
lpfc_vport_reset_stat_data(struct lpfc_vport *vport)
{
	struct lpfc_nodelist *ndlp = NULL, *next_ndlp = NULL;

	list_for_each_entry_safe(ndlp, next_ndlp, &vport->fc_nodes, nlp_listp) {
		if (!NLP_CHK_NODE_ACT(ndlp))
			continue;
		if (ndlp->lat_data)
			memset(ndlp->lat_data, 0, LPFC_MAX_BUCKET_COUNT *
				sizeof(struct lpfc_scsicmd_bkt));
	}
}


/**
 * lpfc_alloc_bucket - Allocate data buffer required for statistical data
 * @vport: Pointer to vport object.
 *
 * This function allocates data buffer required for all the FC
 * nodes of the vport to collect statistical data.
 **/
void
lpfc_alloc_bucket(struct lpfc_vport *vport)
{
	struct lpfc_nodelist *ndlp = NULL, *next_ndlp = NULL;

	list_for_each_entry_safe(ndlp, next_ndlp, &vport->fc_nodes, nlp_listp) {
		if (!NLP_CHK_NODE_ACT(ndlp))
			continue;

		kfree(ndlp->lat_data);
		ndlp->lat_data = NULL;

		if (ndlp->nlp_state == NLP_STE_MAPPED_NODE) {
			ndlp->lat_data = kcalloc(LPFC_MAX_BUCKET_COUNT,
					 sizeof(struct lpfc_scsicmd_bkt),
					 GFP_ATOMIC);

			if (!ndlp->lat_data)
				lpfc_printf_vlog(vport, KERN_ERR, LOG_NODE,
					"0287 lpfc_alloc_bucket failed to "
					"allocate statistical data buffer DID "
					"0x%x\n", ndlp->nlp_DID);
		}
	}
}

/**
 * lpfc_free_bucket - Free data buffer required for statistical data
 * @vport: Pointer to vport object.
 *
 * Th function frees statistical data buffer of all the FC
 * nodes of the vport.
 **/
void
lpfc_free_bucket(struct lpfc_vport *vport)
{
	struct lpfc_nodelist *ndlp = NULL, *next_ndlp = NULL;

	list_for_each_entry_safe(ndlp, next_ndlp, &vport->fc_nodes, nlp_listp) {
		if (!NLP_CHK_NODE_ACT(ndlp))
			continue;

		kfree(ndlp->lat_data);
		ndlp->lat_data = NULL;
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     /*
 * drivers/usb/host/ohci-pnx4008.c
 *
 * driver for Philips PNX4008 USB Host
 *
 * Authors: Dmitry Chigirev <source@mvista.com>
 *	    Vitaly Wool <vitalywool@gmail.com>
 *
 * register initialization is based on code examples provided by Philips
 * Copyright (c) 2005 Koninklijke Philips Electronics N.V.
 *
 * NOTE: This driver does not have suspend/resume functionality
 * This driver is intended for engineering development purposes only
 *
 * 2005-2006 (c) MontaVista Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

#include <mach/hardware.h>
#include <asm/io.h>

#include <mach/platform.h>
#include <mach/irqs.h>
#include <mach/gpio.h>

#define USB_CTRL	IO_ADDRESS(PNX4008_PWRMAN_BASE + 0x64)

/* USB_CTRL bit defines */
#define USB_SLAVE_HCLK_EN	(1 << 24)
#define USB_HOST_NEED_CLK_EN	(1 << 21)

#define USB_OTG_CLK_CTRL	IO_ADDRESS(PNX4008_USB_CONFIG_BASE + 0xFF4)
#define USB_OTG_CLK_STAT	IO_ADDRESS(PNX4008_USB_CONFIG_BASE + 0xFF8)

/* USB_OTG_CLK_CTRL bit defines */
#define AHB_M_CLOCK_ON		(1 << 4)
#define OTG_CLOCK_ON		(1 << 3)
#define I2C_CLOCK_ON		(1 << 2)
#define DEV_CLOCK_ON		(1 << 1)
#define HOST_CLOCK_ON		(1 << 0)

#define USB_OTG_STAT_CONTROL	IO_ADDRESS(PNX4008_USB_CONFIG_BASE + 0x110)

/* USB_OTG_STAT_CONTROL bit defines */
#define TRANSPARENT_I2C_EN	(1 << 7)
#define HOST_EN			(1 << 0)

/* ISP1301 USB transceiver I2C registers */
#define	ISP1301_MODE_CONTROL_1		0x04	/* u8 read, set, +1 clear */

#define	MC1_SPEED_REG		(1 << 0)
#define	MC1_SUSPEND_REG		(1 << 1)
#define	MC1_DAT_SE0		(1 << 2)
#define	MC1_TRANSPARENT		(1 << 3)
#define	MC1_BDIS_ACON_EN	(1 << 4)
#define	MC1_OE_INT_EN		(1 << 5)
#define	MC1_UART_EN		(1 << 6)
#define	MC1_MASK		0x7f

#define	ISP1301_MODE_CONTROL_2		0x12	/* u8 read, set, +1 clear */

#define	MC2_GLOBAL_PWR_DN	(1 << 0)
#define	MC2_SPD_SUSP_CTRL	(1 << 1)
#define	MC2_BI_DI		(1 << 2)
#define	MC2_TRANSP_BDIR0	(1 << 3)
#define	MC2_TRANSP_BDIR1	(1 << 4)
#define	MC2_AUDIO_EN		(1 << 5)
#define	MC2_PSW_EN		(1 << 6)
#define	MC2_EN2V7		(1 << 7)

#define	ISP1301_OTG_CONTROL_1		0x06	/* u8 read, set, +1 clear */
#	define	OTG1_DP_PULLUP		(1 << 0)
#	define	OTG1_DM_PULLUP		(1 << 1)
#	define	OTG1_DP_PULLDOWN	(1 << 2)
#	define	OTG1_DM_PULLDOWN	(1 << 3)
#	define	OTG1_ID_PULLDOWN	(1 << 4)
#	define	OTG1_VBUS_DRV		(1 << 5)
#	define	OTG1_VBUS_DISCHRG	(1 << 6)
#	define	OTG1_VBUS_CHRG		(1 << 7)
#define	ISP1301_OTG_STATUS		0x10	/* u8 readonly */
#	define	OTG_B_SESS_END		(1 << 6)
#	define	OTG_B_SESS_VLD		(1 << 7)

#define ISP1301_I2C_ADDR 0x2C

#define ISP1301_I2C_MODE_CONTROL_1 0x4
#define ISP1301_I2C_MODE_CONTROL_2 0x12
#define ISP1301_I2C_OTG_CONTROL_1 0x6
#define ISP1301_I2C_OTG_CONTROL_2 0x10
#define ISP1301_I2C_INTERRUPT_SOURCE 0x8
#define ISP1301_I2C_INTERRUPT_LATCH 0xA
#define ISP1301_I2C_INTERRUPT_FALLING 0xC
#define ISP1301_I2C_INTERRUPT_RISING 0xE
#define ISP1301_I2C_REG_CLEAR_ADDR 1

struct i2c_driver isp1301_driver;
struct i2c_client *isp1301_i2c_client;

extern int usb_disabled(void);
extern int ocpi_enable(void);

static struct clk *usb_clk;

static const unsigned short normal_i2c[] =
    { ISP1301_I2C_ADDR, ISP1301_I2C_ADDR + 1, I2C_CLIENT_END };

static int isp1301_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	return 0;
}

static int isp1301_remove(struct i2c_client *client)
{
	return 0;
}

const struct i2c_device_id isp1301_id[] = {
	{ "isp1301_pnx", 0 },
	{ }
};

struct i2c_driver isp1301_driver = {
	.driver = {
		.name = "isp1301_pnx",
	},
	.probe = isp1301_probe,
	.remove = isp1301_remove,
	.id_table = isp1301_id,
};

static void i2c_write(u8 buf, u8 subaddr)
{
	char tmpbuf[2];

	tmpbuf[0] = subaddr;	/*register number */
	tmpbuf[1] = buf;	/*register data */
	i2c_master_send(isp1301_i2c_client, &tmpbuf[0], 2);
}

static void isp1301_configure(void)
{
	/* PNX4008 only supports DAT_SE0 USB mode */
	/* PNX4008 R2A requires setting the MAX603 to output 3.6V */
	/* Power up externel charge-pump */

	i2c_write(MC1_DAT_SE0 | MC1_SPEED_REG, ISP1301_I2C_MODE_CONTROL_1);

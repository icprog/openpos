 += sprintf(buffer+result, "--\ndebug_level = 0x%08X (* = enabled)\n",
			     acpi_dbg_level);

	return result;
}

module_param_call(debug_layer, param_set_uint, param_get_debug_layer, &acpi_dbg_layer, 0644);
module_param_call(debug_level, param_set_uint, param_get_debug_level, &acpi_dbg_level, 0644);

static char trace_method_name[6];
module_param_string(trace_method_name, trace_method_name, 6, 0644);
static unsigned int trace_debug_layer;
module_param(trace_debug_layer, uint, 0644);
static unsigned int trace_debug_level;
module_param(trace_debug_level, uint, 0644);

static int param_set_trace_state(const char *val, struct kernel_param *kp)
{
	int result = 0;

	if (!strncmp(val, "enable", strlen("enable") - 1)) {
		result = acpi_debug_trace(trace_method_name, trace_debug_level,
					  trace_debug_layer, 0);
		if (result)
			result = -EBUSY;
		goto exit;
	}

	if (!strncmp(val, "disable", strlen("disable") - 1)) {
		int name = 0;
		result = acpi_debug_trace((char *)&name, trace_debug_level,
					  trace_debug_layer, 0);
		if (result)
			result = -EBUSY;
		goto exit;
	}

	if (!strncmp(val, "1", 1)) {
		result = acpi_debug_trace(trace_method_name, trace_debug_level,
					  trace_debug_layer, 1);
		if (result)
			result = -EBUSY;
		goto exit;
	}

	result = -EINVAL;
exit:
	return result;
}

static int param_get_trace_state(char *buffer, struct kernel_param *kp)
{
	if (!acpi_gbl_trace_method_name)
		return sprintf(buffer, "disable");
	else {
		if (acpi_gbl_trace_flags & 1)
			return sprintf(buffer, "1");
		else
			return sprintf(buffer, "enable");
	}
	return 0;
}

module_param_call(trace_state, param_set_trace_state, param_get_trace_state,
		  NULL, 0644);

/* --------------------------------------------------------------------------
                              FS Interface (/proc)
   -------------------------------------------------------------------------- */
#ifdef CONFIG_ACPI_PROCFS
#define ACPI_SYSTEM_FILE_DEBUG_LAYER	"debug_layer"
#define ACPI_SYSTEM_FILE_DEBUG_LEVEL		"debug_level"

static int
acpi_system_read_debug(char *page,
		       char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;
	int size = 0;
	unsigned int i;

	if (off != 0)
		goto end;

	p += sprintf(p, "%-25s\tHex        SET\n", "Description");

	switch ((unsigned long)data) {
	case 0:
		for (i = 0; i < ARRAY_SIZE(acpi_debug_layers); i++) {
			p += sprintf(p, "%-25s\t0x%08lX [%c]\n",
				     acpi_debug_layers[i].name,
				     acpi_debug_layers[i].value,
				     (acpi_dbg_layer & acpi_debug_layers[i].
				      value) ? '*' : ' ');
		}
		p += sprintf(p, "%-25s\t0x%08X [%c]\n", "ACPI_ALL_DRIVERS",
			     ACPI_ALL_DRIVERS,
			     (acpi_dbg_layer & ACPI_ALL_DRIVERS) ==
			     ACPI_ALL_DRIVERS ? '*' : (acpi_dbg_layer &
						       ACPI_ALL_DRIVERS) ==
			     0 ? ' ' : '-');
		p += sprintf(p,
			     "--\ndebug_layer = 0x%08X (* = enabled, - = partial)\n",
			     acpi_dbg_layer);
		break;
	case 1:
		for (i = 0; i < ARRAY_SIZE(acpi_debug_levels); i++) {
			p += sprintf(p, "%-25s\t0x%08lX [%c]\n",
				     acpi_debug_levels[i].name,
				     acpi_debug_levels[i].value,
				     (acpi_dbg_level & acpi_debug_levels[i].
				      value) ? '*' : ' ');
		}
		p += sprintf(p, "--\ndebug_level = 0x%08X (* = enabled)\n",
			     acpi_dbg_level);
		break;
	default:
		p += sprintf(p, "Invalid debug option\n");
		break;
	}

      end:
	size = (p - page);
	if (size <= off + count)
		*eof = 1;
	*start = page + off;
	size -= off;
	if (size > count)
		size = count;
	if (size < 0)
		size = 0;

	return size;
}

static int
acpi_system_write_debug(struct file *file,
			const char __user * buffer,
			unsigned long count, void *data)
{
	char debug_string[12] = { '\0' };


	if (count > sizeof(debug_string) - 1)
		return -EINVAL;

	if (copy_from_user(debug_string, buffer, count))
		return -EFAULT;

	debug_string[count] = '\0';

	switch ((unsigned long)data) {
	case 0:
		acpi_dbg_layer = simple_strtoul(debug_string, NULL, 0);
		break;
	case 1:
		acpi_dbg_level = simple_strtoul(debug_string, NULL, 0);
		break;
	default:
		return -EINVAL;
	}

	return count;
}
#endif

int __init acpi_debug_init(void)
{
#ifdef CONFIG_ACPI_PROCFS
	struct proc_dir_entry *entry;
	int error = 0;
	char *name;

	/* 'debug_layer' [R/W] */
	name = ACPI_SYSTEM_FILE_DEBUG_LAYER;
	entry =
	    create_proc_read_entry(name, S_IFREG | S_IRUGO | S_IWUSR,
				   acpi_root_dir, acpi_system_read_debug,
				   (void *)0);
	if (entry)
		entry->write_proc = acpi_system_write_debug;
	else
		goto Error;

	/* 'debug_level' [R/W] */
	name = ACPI_SYSTEM_FILE_DEBUG_LEVEL;
	entry =
	    create_proc_read_entry(name, S_IFREG | S_IRUGO | S_IWUSR,
				   acpi_root_dir, acpi_system_read_debug,
				   (void *)1);
	if (entry)
		entry->write_proc = acpi_system_write_debug;
	else
		goto Error;

      Done:
	return error;

      Error:
	remove_proc_entry(ACPI_SYSTEM_FILE_DEBUG_LEVEL, acpi_root_dir);
	remove_proc_entry(ACPI_SYSTEM_FILE_DEBUG_LAYER, acpi_root_dir);
	error = -ENODEV;
	goto Done;
#else
	return 0;
#endif
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    /*
 * linux/drivers/char/pcmcia/synclink_cs.c
 *
 * $Id: synclink_cs.c,v 4.34 2005/09/08 13:20:54 paulkf Exp $
 *
 * Device driver for Microgate SyncLink PC Card
 * multiprotocol serial adapter.
 *
 * written by Paul Fulghum for Microgate Corporation
 * paulkf@microgate.com
 *
 * Microgate and SyncLink are trademarks of Microgate Corporation
 *
 * This code is released under the GNU General Public License (GPL)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define VERSION(ver,rel,seq) (((ver)<<16) | ((rel)<<8) | (seq))
#if defined(__i386__)
#  define BREAKPOINT() asm("   int $3");
#else
#  define BREAKPOINT() { }
#endif

#define MAX_DEVICE_COUNT 4

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/synclink.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/dma.h>
#include <linux/bitops.h>
#include <asm/types.h>
#include <linux/termios.h>
#include <linux/workqueue.h>
#include <linux/hdlc.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>

#if defined(CONFIG_HDLC) || (defined(CONFIG_HDLC_MODULE) && defined(CONFIG_SYNCLINK_CS_MODULE))
#define SYNCLINK_GENERIC_HDLC 1
#else
#define SYNCLINK_GENERIC_HDLC 0
#endif

#define GET_USER(error,value,addr) error = get_user(value,addr)
#define COPY_FROM_USER(error,dest,src,size) error = copy_from_user(dest,src,size) ? -EFAULT : 0
#define PUT_USER(error,value,addr) error = put_user(value,addr)
#define COPY_TO_USER(error,dest,src,size) error = copy_to_user(dest,src,size) ? -EFAULT : 0

#include <asm/uaccess.h>

static MGSL_PARAMS default_params = {
	MGSL_MODE_HDLC,			/* unsigned long mode */
	0,				/* unsigned char loopback; */
	HDLC_FLAG_UNDERRUN_ABORT15,	/* unsigned short flags; */
	HDLC_ENCODING_NRZI_SPACE,	/* unsigned char encoding; */
	0,				/* unsigned long clock_speed; */
	0xff,				/* unsigned char addr_filter; */
	HDLC_CRC_16_CCITT,		/* unsigned short crc_type; */
	HDLC_PREAMBLE_LENGTH_8BITS,	/* unsigned char preamble_length; */
	HDLC_PREAMBLE_PATTERN_NONE,	/* unsigned char preamble; */
	9600,				/* unsigned long data_rate; */
	8,				/* unsigned char data_bits; */
	1,				/* unsigned char stop_bits; */
	ASYNC_PARITY_NONE		/* unsigned char parity; */
};

typedef struct
{
	int count;
	unsigned char status;
	char data[1];
} RXBUF;

/* The queue of BH actions to be performed */

#define BH_RECEIVE  1
#define BH_TRANSMIT 2
#define BH_STATUS   4

#define IO_PIN_SHUTDOWN_LIMIT 100

#define RELEVANT_IFLAG(iflag) (iflag & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))

struct _input_signal_events {
	int	ri_up;
	int	ri_down;
	int	dsr_up;
	int	dsr_down;
	int	dcd_up;
	int	dcd_down;
	int	cts_up;
	int	cts_down;
};


/*
 * Device instance data structure
 */

typedef struct _mgslpc_info {
	struct tty_port		port;
	void *if_ptr;	/* General purpose pointer (used by SPPP) */
	int			magic;
	int			line;

	struct mgsl_icount	icount;

	int			timeout;
	int			x_char;		/* xon/xoff character */
	unsigned char		read_status_mask;
	unsigned char		ignore_status_mask;

	unsigned char *tx_buf;
	int            tx_put;
	int            tx_get;
	int            tx_count;

	/* circular list of fixed length rx buffers */

	unsigned char  *rx_buf;        /* memory allocated for all rx buffers */
	int            rx_buf_total_size; /* size of memory allocated for rx buffers */
	int            rx_put;         /* index of next empty rx buffer */
	int            rx_get;         /* index of next full rx buffer */
	int            rx_buf_size;    /* size in bytes of single rx buffer */
	int            rx_buf_count;   /* total number of rx buffers */
	int            rx_frame_count; /* number of full rx buffers */

	wait_queue_head_t	status_event_wait_q;
	wait_queue_head_t	event_wait_q;
	struct timer_list	tx_timer;	/* HDLC transmit timeout timer */
	struct _mgslpc_info	*next_device;	/* device list link */

	unsigned short imra_value;
	unsigned short imrb_value;
	unsigned char  pim_value;

	spinlock_t lock;
	struct work_struct task;		/* task structure for scheduling bh */

	u32 max_frame_size;

	u32 pending_bh;

	bool bh_running;
	bool bh_requested;

	int dcd_chkcount; /* check counts to prevent */
	int cts_chkcount; /* too many IRQs if a signal */
	int dsr_chkcount; /* is floating */
	int ri_chkcount;

	bool rx_enabled;
	bool rx_overflow;

	bool tx_enabled;
	bool tx_active;
	bool tx_aborting;
	u32 idle_mode;

	int if_mode; /* serial interface selection (RS-232, v.35 etc) */

	char device_name[25];		/* device instance name */

	unsigned int io_base;	/* base I/O address of adapter */
	unsigned int irq_level;

	MGSL_PARAMS params;		/* communications parameters */

	unsigned char serial_signals;	/* current serial signal states */

	bool irq_occurred;		/* for diagnostics use */
	char testing_irq;
	unsigned int init_error;	/* startup error (DIAGS)	*/

	char flag_buf[MAX_ASYNC_BUFFER_SIZE];
	bool drop_rts_on_tx_done;

	struct	_input_signal_events	input_signal_events;

	/* PCMCIA support */
	struct pcmcia_device	*p_dev;
	dev_node_t	      node;
	int		      stop;

	/* SPPP/Cisco HDLC device parts */
	int netcount;
	spinlock_t netlock;

#if SYNCLINK_GENERIC_HDLC
	struct net_device *netdev;
#endif

} MGSLPC_INFO;

#define MGSLPC_MAGIC 0x5402

/*
 * The size of the serial xmit buffer is 1 page, or 4096 bytes
 */
#define TXBUFSIZE 4096


#define CHA     0x00   /* channel A offset */
#define CHB     0x40   /* channel B offset */

/*
 *  FIXME: PPC has PVR defined in asm/reg.h.  For now we just undef it.
 */
#undef PVR

#define RXFIFO  0
#define TXFIFO  0
#define STAR    0x20
#define CMDR    0x20
#define RSTA    0x21
#define PRE     0x21
#define MODE    0x22
#define TIMR    0x23
#define XAD1    0x24
#define XAD2    0x25
#define RAH1    0x26
#define RAH2    0x27
#define DAFO    0x27
#define RAL1    0x28
#define RFC     0x28
#define RHCR    0x29
#define RAL2    0x29
#define RBCL    0x2a
#define XBCL    0x2a
#define RBCH    0x2b
#define XBCH    0x2b
#define CCR0    0x2c
#define CCR1    0x2d
#define CCR2    0x2e
#define CCR3    0x2f
#define VSTR    0x34
#define BGR     0x34
#define RLCR    0x35
#define AML     0x36
#define AMH     0x37
#define GIS     0x38
#define IVA     0x38
#define IPC     0x39
#define ISR     0x3a
#define IMR     0x3a
#define PVR     0x3c
#define PIS     0x3d
#define PIM     0x3d
#define PCR     0x3e
#define CCR4    0x3f

// IMR/ISR

#define IRQ_BREAK_ON    BIT15   // rx break detected
#define IRQ_DATAOVERRUN BIT14	// receive data overflow
#define IRQ_ALLSENT     BIT13	// all sent
#define IRQ_UNDERRUN    BIT12	// transmit data underrun
#define IRQ_TIMER       BIT11	// timer interrupt
#define IRQ_CTS         BIT10	// CTS status change
#define IRQ_TXREPEAT    BIT9	// tx message repeat
#define IRQ_TXFIFO      BIT8	// transmit pool ready
#define IRQ_RXEOM       BIT7	// receive message end
#define IRQ_EXITHUNT    BIT6	// receive frame start
#define IRQ_RXTIME      BIT6    // rx char timeout
#define IRQ_DCD         BIT2	// carrier detect status change
#define IRQ_OVERRUN     BIT1	// receive frame overflow
#define IRQ_RXFIFO      BIT0	// receive pool full

// STAR

#define XFW   BIT6		// transmit FIFO write enable
#define CEC   BIT2		// command executing
#define CTS   BIT1		// CTS state

#define PVR_DTR      BIT0
#define PVR_DSR      BIT1
#define PVR_RI       BIT2
#define PVR_AUTOCTS  BIT3
#define PVR_RS232    0x20   /* 0010b */
#define PVR_V35      0xe0   /* 1110b */
#define PVR_RS422    0x40   /* 0100b */

/* Register access functions */

#define write_reg(info, reg, val) outb((val),(info)->io_base + (reg))
#define read_reg(info, reg) inb((info)->io_base + (reg))

#define read_reg16(info, reg) inw((info)->io_base + (reg))
#define write_reg16(info, reg, val) outw((val), (info)->io_base + (reg))

#define set_reg_bits(info, reg, mask) \
    write_reg(info, (reg), \
		 (unsigned char) (read_reg(info, (reg)) | (mask)))
#define clear_reg_bits(info, reg, mask) \
    write_reg(info, (reg), \
		 (unsigned char) (read_reg(info, (reg)) & ~(mask)))
/*
 * interrupt enable/disable routines
 */
static void irq_disable(MGSLPC_INFO *info, unsigned char channel, unsigned short mask)
{
	if (channel == CHA) {
		info->imra_value |= mask;
		write_reg16(info, CHA + IMR, info->imra_value);
	} else {
		info->imrb_value |= mask;
		write_reg16(info, CHB + IMR, info->imrb_value);
	}
}
static void irq_enable(MGSLPC_INFO *info, unsigned char channel, unsigned short mask)
{
	if (channel == CHA) {
		info->imra_value &= ~mask;
		write_reg16(info, CHA + IMR, info->imra_value);
	} else {
		info->imrb_value &= ~mask;
		write_reg16(info, CHB + IMR, info->imrb_value);
	}
}

#define port_irq_disable(info, mask) \
  { info->pim_value |= (mask); write_reg(info, PIM, info->pim_value); }

#define port_irq_enable(info, mask) \
  { info->pim_value &= ~(mask); write_reg(info, PIM, info->pim_value); }

static void rx_start(MGSLPC_INFO *info);
static void rx_stop(MGSLPC_INFO *info);

static void tx_start(MGSLPC_INFO *info, struct tty_struct *tty);
static void tx_stop(MGSLPC_INFO *info);
static void tx_set_idle(MGSLPC_INFO *info);

static void get_signals(MGSLPC_INFO *info);
static void set_signals(MGSLPC_INFO *info);

static void reset_device(MGSLPC_INFO *info);

static void hdlc_mode(MGSLPC_INFO *info);
static void async_mode(MGSLPC_INFO *info);

static void tx_timeout(unsigned long context);

static int carrier_raised(struct tty_port *port);
static void dtr_rts(struct tty_port *port, int onoff);

#if SYNCLINK_GENERIC_HDLC
#define dev_to_port(D) (dev_to_hdlc(D)->priv)
static void hdlcdev_tx_done(MGSLPC_INFO *info);
static void hdlcdev_rx(MGSLPC_INFO *info, char *buf, int size);
static int  hdlcdev_init(MGSLPC_INFO *info);
static void hdlcdev_exit(MGSLPC_INFO *info);
#endif

static void trace_block(MGSLPC_INFO *info,const char* data, int count, int xmit);

static bool register_test(MGSLPC_INFO *info);
static bool irq_test(MGSLPC_INFO *info);
static int adapter_test(MGSLPC_INFO *info);

static int claim_resources(MGSLPC_INFO *info);
static void release_resources(MGSLPC_INFO *info);
static void mgslpc_add_device(MGSLPC_INFO *info);
static void mgslpc_remove_device(MGSLPC_INFO *info);

static bool rx_get_frame(MGSLPC_INFO *info, struct tty_struct *tty);
static void rx_reset_buffers(MGSLPC_INFO *info);
static int  rx_alloc_buffers(MGSLPC_INFO *info);
static void rx_free_buffers(MGSLPC_INFO *info);

static irqreturn_t mgslpc_isr(int irq, void *dev_id);

/*
 * Bottom half interrupt handlers
 */
static void bh_handler(struct work_struct *work);
static void bh_transmit(MGSLPC_INFO *info, struct tty_struct *tty);
static void bh_status(MGSLPC_INFO *info);

/*
 * ioctl handlers
 */
static int tiocmget(struct tty_struct *tty, struct file *file);
static int tiocmset(struct tty_struct *tty, struct file *file,
		    unsigned int set, unsigned int clear);
static int get_stats(MGSLPC_INFO *info, struct mgsl_icount __user *user_icount);
static int get_params(MGSLPC_INFO *info, MGSL_PARAMS __user *user_params);
static int set_params(MGSLPC_INFO *info, MGSL_PARAMS __user *new_params, struct tty_struct *tty);
static int get_txidle(MGSLPC_INFO *info, int __user *idle_mode);
static int set_txidle(MGSLPC_INFO *info, int idle_mode);
static int set_txenable(MGSLPC_INFO *info, int enable, struct tty_struct *tty);
static int tx_abort(MGSLPC_INFO *info);
static int set_rxenable(MGSLPC_INFO *info, int enable);
static int wait_events(MGSLPC_INFO *info, int __user *mask);

static MGSLPC_INFO *mgslpc_device_list = NULL;
static int mgslpc_device_count = 0;

/*
 * Set this param to non-zero to load eax with the
 * .text section address and breakpoint on module load.
 * This is useful for use with gdb and add-symbol-file command.
 */
static int break_on_load=0;

/*
 * Driver major number, defaults to zero to get auto
 * assigned major number. May be forced as module parameter.
 */
static int ttymajor=0;

static int debug_level = 0;
static int maxframe[MAX_DEVICE_COUNT] = {0,};

module_param(break_on_load, bool, 0);
module_param(ttymajor, int, 0);
module_param(debug_level, int, 0);
module_param_array(maxframe, int, NULL, 0);

MODULE_LICENSE("GPL");

static char *driver_name = "SyncLink PC Card driver";
static char *driver_version = "$Revision: 4.34 $";

static struct tty_driver *serial_driver;

/* number of characters left in xmit buffer before we ask for more */
#define WAKEUP_CHARS 256

static void mgslpc_change_params(MGSLPC_INFO *info, struct tty_struct *tty);
static void mgslpc_wait_until_sent(struct tty_struct *tty, int timeout);

/* PCMCIA prototypes */

static int mgslpc_config(struct pcmcia_device *link);
static void mgslpc_release(u_long arg);
static void mgslpc_detach(struct pcmcia_device *p_dev);

/*
 * 1st function defined in .text section. Calling this function in
 * init_module() followed by a breakpoint allows a remote debugger
 * (gdb) to get the .text address for the add-symbol-file command.
 * This allows remote debugging of dynamically loadable modules.
 */
static void* mgslpc_get_text_ptr(void)
{
	return mgslpc_get_text_ptr;
}

/**
 * line discipline callback wrappers
 *
 * The wrappers maintain line discipline references
 * while calling into the line discipline.
 *
 * ldisc_receive_buf  - pass receive data to line discipline
 */

static void ldisc_receive_buf(struct tty_struct *tty,
			      const __u8 *data, char *flags, int count)
{
	struct tty_ldisc *ld;
	if (!tty)
		return;
	ld = tty_ldisc_ref(tty);
	if (ld) {
		if (ld->ops->receive_buf)
			ld->ops->receive_buf(tty, data, flags, count);
		tty_ldisc_deref(ld);
	}
}

static const struct tty_port_operations mgslpc_port_ops = {
	.carrier_raised = carrier_raised,
	.dtr_rts = dtr_rts
};

static int mgslpc_probe(struct pcmcia_device *link)
{
    MGSLPC_INFO *info;
    int ret;

    if (debug_level >= DEBUG_LEVEL_INFO)
	    printk("mgslpc_attach\n");

    info = kzalloc(sizeof(MGSLPC_INFO), GFP_KERNEL);
    if (!info) {
	    printk("Error can't allocate device instance data\n");
	    return -ENOMEM;
    }

    info->magic = MGSLPC_MAGIC;
    tty_port_init(&info->port);
    info->port.ops = &mgslpc_port_ops;
    INIT_WORK(&info->task, bh_handler);
    info->max_frame_size = 4096;
    info->port.close_delay = 5*HZ/10;
    info->port.closing_wait = 30*HZ;
    init_waitqueue_head(&info->status_event_wait_q);
    init_waitqueue_head(&info->event_wait_q);
    spin_lock_init(&info->lock);
    spin_lock_init(&info->netlock);
    memcpy(&info->params,&default_params,sizeof(MGSL_PARAMS));
    info->idle_mode = HDLC_TXIDLE_FLAGS;
    info->imra_value = 0xffff;
    info->imrb_value = 0xffff;
    info->pim_value = 0xff;

    info->p_dev = link;
    link->priv = info;

    /* Initialize the struct pcmcia_device structure */

    /* Interrupt setup */
    link->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING;
    link->irq.IRQInfo1   = IRQ_LEVEL_ID;
    link->irq.Handler = NULL;

    link->conf.Attributes = 0;
    link->conf.IntType = INT_MEMORY_AND_IO;

    ret = mgslpc_config(link);
    if (ret)
	    return ret;

    mgslpc_add_device(info);

    return 0;
}

/* Card has been inserted.
 */

#define CS_CHECK(fn, ret) \
do { last_fn = (fn); if ((last_ret = (ret)) != 0) goto cs_failed; } while (0)

static int mgslpc_config(struct pcmcia_device *link)
{
    MGSLPC_INFO *info = link->priv;
    tuple_t tuple;
    cisparse_t parse;
    int last_fn, last_ret;
    u_char buf[64];
    cistpl_cftable_entry_t dflt = { 0 };
    cistpl_cftable_entry_t *cfg;

    if (debug_level >= DEBUG_LEVEL_INFO)
	    printk("mgslpc_config(0x%p)\n", link);

    tuple.Attributes = 0;
    tuple.TupleData = buf;
    tuple.TupleDataMax = sizeof(buf);
    tuple.TupleOffset = 0;

    /* get CIS configuration entry */

    tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
    CS_CHECK(GetFirstTuple, pcmcia_get_first_tuple(link, &tuple));

    cfg = &(parse.cftable_entry);
    CS_CHECK(GetTupleData, pcmcia_get_tuple_data(link, &tuple));
    CS_CHECK(ParseTuple, pcmcia_parse_tuple(&tuple, &parse));

    if (cfg->flags & CISTPL_CFTABLE_DEFAULT) dflt = *cfg;
    if (cfg->index == 0)
	    goto cs_failed;

    link->conf.ConfigIndex = cfg->index;
    link->conf.Attributes |= CONF_ENABLE_IRQ;

    /* IO window settings */
    link->io.NumPorts1 = 0;
    if ((cfg->io.nwin > 0) || (dflt.io.nwin > 0)) {
	    cistpl_io_t *io = (cfg->io.nwin) ? &cfg->io : &dflt.io;
	    link->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
	    if (!(io->flags & CISTPL_IO_8BIT))
		    link->io.Attributes1 = IO_DATA_PATH_WIDTH_16;
	    if (!(io->flags & CISTPL_IO_16BIT))
		    link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
	    link->io.IOAddrLines = io->flags & CISTPL_IO_LINES_MASK;
	    link->io.BasePort1 = io->win[0].base;
	    link->io.NumPorts1 = io->win[0].len;
	    CS_CHECK(RequestIO, pcmcia_request_io(link, &link->io));
    }

    link->conf.Attributes = CONF_ENABLE_IRQ;
    link->conf.IntType = INT_MEMORY_AND_IO;
    link->conf.ConfigIndex = 8;
    link->conf.Present = PRESENT_OPTION;

    link->irq.Attributes |= IRQ_HANDLE_PRESENT;
    link->irq.Handler     = mgslpc_isr;
    link->irq.Instance    = info;
    CS_CHECK(RequestIRQ, pcmcia_request_irq(link, &link->irq));

    CS_CHECK(RequestConfiguration, pcmcia_request_configuration(link, &link->conf));

    info->io_base = link->io.BasePort1;
    info->irq_level = link->irq.AssignedIRQ;

    /* add to linked list of devices */
    sprintf(info->node.dev_name, "mgslpc0");
    info->node.major = info->node.minor = 0;
    link->dev_node = &info->node;

    printk(KERN_INFO "%s: index 0x%02x:",
	   info->node.dev_name, link->conf.ConfigIndex);
    if (link->conf.Attributes & CONF_ENABLE_IRQ)
	    printk(", irq %d", link->irq.AssignedIRQ);
    if (link->io.NumPorts1)
	    printk(", io 0x%04x-0x%04x", link->io.BasePort1,
		   link->io.BasePort1+link->io.NumPorts1-1);
    printk("\n");
    return 0;

cs_failed:
    cs_error(link, last_fn, last_ret);
    mgslpc_release((u_long)link);
    return -ENODEV;
}

/* Card has been removed.
 * Unregister device and release PCMCIA configuration.
 * If device is open, postpone until it is closed.
 */
static void mgslpc_release(u_long arg)
{
	struct pcmcia_device *link = (struct pcmcia_device *)arg;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("mgslpc_release(0x%p)\n", link);

	pcmcia_disable_device(link);
}

static void mgslpc_detach(struct pcmcia_device *link)
{
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("mgslpc_detach(0x%p)\n", link);

	((MGSLPC_INFO *)link->priv)->stop = 1;
	mgslpc_release((u_long)link);

	mgslpc_remove_device((MGSLPC_INFO *)link->priv);
}

static int mgslpc_suspend(struct pcmcia_device *link)
{
	MGSLPC_INFO *info = link->priv;

	info->stop = 1;

	return 0;
}

static int mgslpc_resume(struct pcmcia_device *link)
{
	MGSLPC_INFO *info = link->priv;

	info->stop = 0;

	return 0;
}


static inline bool mgslpc_paranoia_check(MGSLPC_INFO *info,
					char *name, const char *routine)
{
#ifdef MGSLPC_PARANOIA_CHECK
	static const char *badmagic =
		"Warning: bad magic number for mgsl struct (%s) in %s\n";
	static const char *badinfo =
		"Warning: null mgslpc_info for (%s) in %s\n";

	if (!info) {
		printk(badinfo, name, routine);
		return true;
	}
	if (info->magic != MGSLPC_MAGIC) {
		printk(badmagic, name, routine);
		return true;
	}
#else
	if (!info)
		return true;
#endif
	return false;
}


#define CMD_RXFIFO      BIT7	// release current rx FIFO
#define CMD_RXRESET     BIT6	// receiver reset
#define CMD_RXFIFO_READ BIT5
#define CMD_START_TIMER BIT4
#define CMD_TXFIFO      BIT3	// release current tx FIFO
#define CMD_TXEOM       BIT1	// transmit end message
#define CMD_TXRESET     BIT0	// transmit reset

static bool wait_command_complete(MGSLPC_INFO *info, unsigned char channel)
{
	int i = 0;
	/* wait for command completion */
	while (read_reg(info, (unsigned char)(channel+STAR)) & BIT2) {
		udelay(1);
		if (i++ == 1000)
			return false;
	}
	return true;
}

static void issue_command(MGSLPC_INFO *info, unsigned char channel, unsigned char cmd)
{
	wait_command_complete(info, channel);
	write_reg(info, (unsigned char) (channel + CMDR), cmd);
}

static void tx_pause(struct tty_struct *tty)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (mgslpc_paranoia_check(info, tty->name, "tx_pause"))
		return;
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("tx_pause(%s)\n",info->device_name);

	spin_lock_irqsave(&info->lock,flags);
	if (info->tx_enabled)
	 	tx_stop(info);
	spin_unlock_irqrestore(&info->lock,flags);
}

static void tx_release(struct tty_struct *tty)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (mgslpc_paranoia_check(info, tty->name, "tx_release"))
		return;
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("tx_release(%s)\n",info->device_name);

	spin_lock_irqsave(&info->lock,flags);
	if (!info->tx_enabled)
	 	tx_start(info, tty);
	spin_unlock_irqrestore(&info->lock,flags);
}

/* Return next bottom half action to perform.
 * or 0 if nothing to do.
 */
static int bh_action(MGSLPC_INFO *info)
{
	unsigned long flags;
	int rc = 0;

	spin_lock_irqsave(&info->lock,flags);

	if (info->pending_bh & BH_RECEIVE) {
		info->pending_bh &= ~BH_RECEIVE;
		rc = BH_RECEIVE;
	} else if (info->pending_bh & BH_TRANSMIT) {
		info->pending_bh &= ~BH_TRANSMIT;
		rc = BH_TRANSMIT;
	} else if (info->pending_bh & BH_STATUS) {
		info->pending_bh &= ~BH_STATUS;
		rc = BH_STATUS;
	}

	if (!rc) {
		/* Mark BH routine as complete */
		info->bh_running = false;
		info->bh_requested = false;
	}

	spin_unlock_irqrestore(&info->lock,flags);

	return rc;
}

static void bh_handler(struct work_struct *work)
{
	MGSLPC_INFO *info = container_of(work, MGSLPC_INFO, task);
	struct tty_struct *tty;
	int action;

	if (!info)
		return;

	if (debug_level >= DEBUG_LEVEL_BH)
		printk( "%s(%d):bh_handler(%s) entry\n",
			__FILE__,__LINE__,info->device_name);

	info->bh_running = true;
	tty = tty_port_tty_get(&info->port);

	while((action = bh_action(info)) != 0) {

		/* Process work item */
		if ( debug_level >= DEBUG_LEVEL_BH )
			printk( "%s(%d):bh_handler() work item action=%d\n",
				__FILE__,__LINE__,action);

		switch (action) {

		case BH_RECEIVE:
			while(rx_get_frame(info, tty));
			break;
		case BH_TRANSMIT:
			bh_transmit(info, tty);
			break;
		case BH_STATUS:
			bh_status(info);
			break;
		default:
			/* unknown work item ID */
			printk("Unknown work item ID=%08X!\n", action);
			break;
		}
	}

	tty_kref_put(tty);
	if (debug_level >= DEBUG_LEVEL_BH)
		printk( "%s(%d):bh_handler(%s) exit\n",
			__FILE__,__LINE__,info->device_name);
}

static void bh_transmit(MGSLPC_INFO *info, struct tty_struct *tty)
{
	if (debug_level >= DEBUG_LEVEL_BH)
		printk("bh_transmit() entry on %s\n", info->device_name);

	if (tty)
		tty_wakeup(tty);
}

static void bh_status(MGSLPC_INFO *info)
{
	info->ri_chkcount = 0;
	info->dsr_chkcount = 0;
	info->dcd_chkcount = 0;
	info->cts_chkcount = 0;
}

/* eom: non-zero = end of frame */
static void rx_ready_hdlc(MGSLPC_INFO *info, int eom)
{
	unsigned char data[2];
	unsigned char fifo_count, read_count, i;
	RXBUF *buf = (RXBUF*)(info->rx_buf + (info->rx_put * info->rx_buf_size));

	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("%s(%d):rx_ready_hdlc(eom=%d)\n",__FILE__,__LINE__,eom);

	if (!info->rx_enabled)
		return;

	if (info->rx_frame_count >= info->rx_buf_count) {
		/* no more free buffers */
		issue_command(info, CHA, CMD_RXRESET);
		info->pendin-----------------------------------------------*/
static int m66592_enable(struct usb_ep *_ep,
			 const struct usb_endpoint_descriptor *desc)
{
	struct m66592_ep *ep;

	ep = container_of(_ep, struct m66592_ep, ep);
	return alloc_pipe_config(ep, desc);
}

static int m66592_disable(struct usb_ep *_ep)
{
	struct m66592_ep *ep;
	struct m66592_request *req;
	unsigned long flags;

	ep = container_of(_ep, struct m66592_ep, ep);
	BUG_ON(!ep);

	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct m66592_request, queue);
		spin_lock_irqsave(&ep->m66592->lock, flags);
		transfer_complete(ep, req, -ECONNRESET);
		spin_unlock_irqrestore(&ep->m66592->lock, flags);
	}

	pipe_irq_disable(ep->m66592, ep->pipenum);
	return free_pipe_config(ep);
}

static struct usb_request *m66592_alloc_request(struct usb_ep *_ep,
						gfp_t gfp_flags)
{
	struct m66592_request *req;

	req = kzalloc(sizeof(struct m66592_request), gfp_flags);
	if (!req)
		return NULL;

	INIT_LIST_HEAD(&req->queue);

	return &req->req;
}

static void m66592_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct m66592_request *req;

	req = container_of(_req, struct m66592_request, req);
	kfree(req);
}

static int m66592_queue(struct usb_ep *_ep, struct usb_request *_req,
			gfp_t gfp_flags)
{
	struct m66592_ep *ep;
	struct m66592_request *req;
	unsigned long flags;
	int request = 0;

	ep = container_of(_ep, struct m66592_ep, ep);
	req = container_of(_req, struct m66592_request, req);

	if (ep->m66592->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;

	spin_lock_irqsave(&ep->m66592->lock, flags);

	if (list_empty(&ep->queue))
		request = 1;

	list_add_tail(&req->queue, &ep->queue);
	req->req.actual = 0;
	req->req.status = -EINPROGRESS;

	if (ep->desc == NULL)	/* control */
		start_ep0(ep, req);
	else {
		if (request && !ep->busy)
			start_packet(ep, req);
	}

	spin_unlock_irqrestore(&ep->m66592->lock, flags);

	return 0;
}

static int m66592_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct m66592_ep *ep;
	struct m66592_request *req;
	unsigned long flags;

	ep = container_of(_ep, struct m66592_ep, ep);
	req = container_of(_req, struct m66592_request, req);

	spin_lock_irqsave(&ep->m66592->lock, flags);
	if (!list_empty(&ep->queue))
		transfer_complete(ep, req, -ECONNRESET);
	spin_unlock_irqrestore(&ep->m66592->lock, flags);

	return 0;
}

static int m66592_set_halt(struct usb_ep *_ep, int value)
{
	struct m66592_ep *ep;
	struct m66592_request *req;
	unsigned long flags;
	int ret = 0;

	ep = container_of(_ep, struct m66592_ep, ep);
	req = list_entry(ep->queue.next, struct m66592_request, queue);

	spin_lock_irqsave(&ep->m66592->lock, flags);
	if (!list_empty(&ep->queue)) {
		ret = -EAGAIN;
		goto out;
	}
	if (value) {
		ep->busy = 1;
		pipe_stall(ep->m66592, ep->pipenum);
	} else {
		ep->busy = 0;
		pipe_stop(ep->m66592, ep->pipenum);
	}

out:
	spin_unlock_irqrestore(&ep->m66592->lock, flags);
	return ret;
}

static void m66592_fifo_flush(struct usb_ep *_ep)
{
	struct m66592_ep *ep;
	unsigned long flags;

	ep = container_of(_ep, struct m66592_ep, ep);
	spin_lock_irqsave(&ep->m66592->lock, flags);
	if (list_empty(&ep->queue) && !ep->busy) {
		pipe_stop(ep->m66592, ep->pipenum);
		m66592_bclr(ep->m66592, M66592_BCLR, ep->fifoctr);
	}
	spin_unlock_irqrestore(&ep->m66592->lock, flags);
}

static struct usb_ep_ops m66592_ep_ops = {
	.enable		= m66592_enable,
	.disable	= m66592_disable,

	.alloc_request	= m66592_alloc_request,
	.free_request	= m66592_free_request,

	.queue		= m66592_queue,
	.dequeue	= m66592_dequeue,

	.set_halt	= m66592_set_halt,
	.fifo_flush	= m66592_fifo_flush,
};

/*-------------------------------------------------------------------------*/
static struct m66592 *the_controller;

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct m66592 *m66592 = the_controller;
	int retval;

	if (!driver
			|| driver->speed != USB_SPEED_HIGH
			|| !driver->bind
			|| !driver->setup)
		return -EINVAL;
	if (!m66592)
		return -ENODEV;
	if (m66592->driver)
		return -EBUSY;

	/* hook up the driver */>tx_buf + info->tx_get));
		} else {
			write_reg16(info, CHA + TXFIFO,
					  *((unsigned short*)(info->tx_buf + info->tx_get)));
		}
		info->tx_count -= c;
		info->tx_get = (info->tx_get + c) & (TXBUFSIZE - 1);
		fifo_count -= c;
	}

	if (info->params.mode == MGSL_MODE_ASYNC) {
		if (info->tx_count < WAKEUP_CHARS)
			info->pending_bh |= BH_TRANSMIT;
		issue_command(info, CHA, CMD_TXFIFO);
	} else {
		if (info->tx_count)
			issue_command(info, CHA, CMD_TXFIFO);
		else
			issue_command(info, CHA, CMD_TXFIFO + CMD_TXEOM);
	}
}

static void cts_change(MGSLPC_INFO *info, struct tty_struct *tty)
{
	get_signals(info);
	if ((info->cts_chkcount)++ >= IO_PIN_SHUTDOWN_LIMIT)
		irq_disable(info, CHB, IRQ_CTS);
	info->icount.cts++;
	if (info->serial_signals & SerialSignal_CTS)
		info->input_signal_events.cts_up++;
	else
		info->input_signal_events.cts_down++;
	wake_up_interruptible(&info->status_event_wait_q);
	wake_up_interruptible(&info->event_wait_q);

	if (info->port.flags & ASYNC_CTS_FLOW) {
		if (tty->hw_stopped) {
			if (info->serial_signals & SerialSignal_CTS) {
				if (debug_level >= DEBUG_LEVEL_ISR)
					printk("CTS tx start...");
				if (tty)
					tty->hw_stopped = 0;
				tx_start(info, tty);
				info->pending_bh |= BH_TRANSMIT;
				return;
			}
		} else {
			if (!(info->serial_signals & SerialSignal_CTS)) {
				if (debug_level >= DEBUG_LEVEL_ISR)
					printk("CTS tx stop...");
				if (tty)
					tty->hw_stopped = 1;
				tx_stop(info);
			}
		}
	}
	info->pending_bh |= BH_STATUS;
}

static void dcd_change(MGSLPC_INFO *info, struct tty_struct *tty)
{
	get_signals(info);
	if ((info->dcd_chkcount)++ >= IO_PIN_SHUTDOWN_LIMIT)
		irq_disable(info, CHB, IRQ_DCD);
	info->icount.dcd++;
	if (info->serial_signals & SerialSignal_DCD) {
		info->input_signal_events.dcd_up++;
	}
	else
		info->input_signal_events.dcd_down++;
#if SYNCLINK_GENERIC_HDLC
	if (info->netcount) {
		if (info->serial_signals & SerialSignal_DCD)
			netif_carrier_on(info->netdev);
		else
			netif_carrier_off(info->netdev);
	}
#endif
	wake_up_interruptible(&info->status_event_wait_q);
	wake_up_interruptible(&info->event_wait_q);

	if (info->port.flags & ASYNC_CHECK_CD) {
		if (debug_level >= DEBUG_LEVEL_ISR)
			printk("%s CD now %s...", info->device_name,
			       (info->serial_signals & SerialSignal_DCD) ? "on" : "off");
		if (info->serial_signals & SerialSignal_DCD)
			wake_up_interruptible(&info->port.open_wait);
		else {
			if (debug_level >= DEBUG_LEVEL_ISR)
				printk("doing serial hangup...");
			if (tty)
				tty_hangup(tty);
		}
	}
	info->pending_bh |= BH_STATUS;
}

static void dsr_change(MGSLPC_INFO *info)
{
	get_signals(info);
	if ((info->dsr_chkcount)++ >= IO_PIN_SHUTDOWN_LIMIT)
		port_irq_disable(info, PVR_DSR);
	info->icount.dsr++;
	if (info->serial_signals & SerialSignal_DSR)
		info->input_signal_events.dsr_up++;
	else
		info->input_signal_events.dsr_down++;
	wake_up_interruptible(&info->status_event_wait_q);
	wake_up_interruptible(&info->event_wait_q);
	info->pending_bh |= BH_STATUS;
}

static void ri_change(MGSLPC_INFO *info)
{
	get_signals(info);
	if ((info->ri_chkcount)++ >= IO_PIN_SHUTDOWN_LIMIT)
		port_irq_disable(info, PVR_RI);
	info->icount.rng++;
	if (info->serial_signals & SerialSignal_RI)
		info->input_signal_events.ri_up++;
	else
		info->input_signal_events.ri_down++;
	wake_up_interruptible(&info->status_event_wait_q);
	wake_up_interruptible(&info->event_wait_q);
	info->pending_bh |= BH_STATUS;
}

/* Interrupt service routine entry point.
 *
 * Arguments:
 *
 * irq     interrupt number that caused interrupt
 * dev_id  device ID supplied during interrupt registration
 */
static irqreturn_t mgslpc_isr(int dummy, void *dev_id)
{
	MGSLPC_INFO *info = dev_id;
	struct tty_struct *tty;
	unsigned short isr;
	unsigned char gis, pis;
	int count=0;

	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("mgslpc_isr(%d) entry.\n", info->irq_level);

	if (!(info->p_dev->_locked))
		return IRQ_HANDLED;

	tty = tty_port_tty_get(&info->port);

	spin_lock(&info->lock);

	while ((gis = read_reg(info, CHA + GIS))) {
		if (debug_level >= DEBUG_LEVEL_ISR)
			printk("mgslpc_isr %s gis=%04X\n", info->device_name,gis);

		if ((gis & 0x70) || count > 1000) {
			printk("synclink_cs:hardware failed or ejected\n");
			break;
		}
		count++;

		if (gis & (BIT1 + BIT0)) {
			isr = read_reg16(info, CHB + ISR);
			if (isr & IRQ_DCD)
				dcd_change(info, tty);
			if (isr & IRQ_CTS)
				cts_change(info, tty);
		}
		if (gis & (BIT3 + BIT2))
		{
			isr = read_reg16(info, CHA + ISR);
			if (isr & IRQ_TIMER) {
				info->irq_occurred = true;
				irq_disable(info, CHA, IRQ_TIMER);
			}

			/* receive IRQs */
			if (isr & IRQ_EXITHUNT) {
				info->icount.exithunt++;
				wake_up_interruptible(&info->event_wait_q);
			}
			if (isr & IRQ_BREAK_ON) {
				info->icount.brk++;
				if (info->port.flags & ASYNC_SAK)
					do_SAK(tty);
			}
			if (isr & IRQ_RXTIME) {
				issue_command(info, CHA, CMD_RXFIFO_READ);
			}
			if (isr & (IRQ_RXEOM + IRQ_RXFIFO)) {
				if (info->params.mode == MGSL_MODE_HDLC)
					rx_ready_hdlc(info, isr & IRQ_RXEOM);
				else
					rx_ready_async(info, isr & IRQ_RXEOM, tty);
			}

			/* transmit IRQs */
			if (isr & IRQ_UNDERRUN) {
				if (info->tx_aborting)
					info->icount.txabort++;
				else
					info->icount.txunder++;
				tx_done(info, tty);
			}
			else if (isr & IRQ_ALLSENT) {
				info->icount.txok++;
				tx_done(info, tty);
			}
			else if (isr & IRQ_TXFIFO)
				tx_ready(info, tty);
		}
		if (gis & BIT7) {
			pis = read_reg(info, CHA + PIS);
			if (pis & BIT1)
				dsr_change(info);
			if (pis & BIT2)
				ri_change(info);
		}
	}

	/* Request bottom half processing if there's something
	 * for it to do and the bh is not already running
	 */

	if (info->pending_bh && !info->bh_running && !info->bh_requested) {
		if ( debug_level >= DEBUG_LEVEL_ISR )
			printk("%s(%d):%s queueing bh task.\n",
				__FILE__,__LINE__,info->device_name);
		schedule_work(&info->task);
		info->bh_requested = true;
	}

	spin_unlock(&info->lock);
	tty_kref_put(tty);

	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("%s(%d):mgslpc_isr(%d)exit.\n",
		       __FILE__, __LINE__, info->irq_level);

	return IRQ_HANDLED;
}

/* Initialize and start device.
 */
static int startup(MGSLPC_INFO * info, struct tty_struct *tty)
{
	int retval = 0;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):sta
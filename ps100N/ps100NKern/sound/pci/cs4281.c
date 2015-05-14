s = acpi_ds_obj_stack_push(obj_desc, walk_state);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		ACPI_DEBUGGER_EXEC(acpi_db_display_argument_object
				   (obj_desc, walk_state));
	}

	return_ACPI_STATUS(AE_OK);
}

/*******************************************************************************
 *
 * FUNCTION:    acpi_ds_create_operands
 *
 * PARAMETERS:  walk_state          - Current state
 *              first_arg           - First argument of a parser argument tree
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Convert an operator's arguments from a parse tree format to
 *              namespace objects and place those argument object on the object
 *              stack in preparation for evaluation by the interpreter.
 *
 ******************************************************************************/

acpi_status
acpi_ds_create_operands(struct acpi_walk_state *walk_state,
			union acpi_parse_object *first_arg)
{
	acpi_status status = AE_OK;
	union acpi_parse_object *arg;
	union acpi_parse_object *arguments[ACPI_OBJ_NUM_OPERANDS];
	u32 arg_count = 0;
	u32 index = walk_state->num_operands;
	u32 i;

	ACPI_FUNCTION_TRACE_PTR(ds_create_operands, first_arg);

	/* Get all arguments in the list */

	arg = first_arg;
	while (arg) {
		if (index >= ACPI_OBJ_NUM_OPERANDS) {
			return_ACPI_STATUS(AE_BAD_DATA);
		}

		arguments[index] = arg;
		walk_state->operands[index] = NULL;

		/* Move on to next argument, if any */

		arg = arg->common.next;
		arg_count++;
		index++;
	}

	index--;

	/* It is the appropriate order to get objects from the Result stack */

	for (i = 0; i < arg_count; i++) {
		arg = arguments[index];

		/* Force the filling of the operand stack in inverse order */

		walk_state->operand_index = (u8) index;

		status = acpi_ds_create_operand(walk_state, arg, index);
		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}

		index--;

		ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
				  "Arg #%d (%p) done, Arg1=%p\n", index, arg,
				  first_arg));
	}

	return_ACPI_STATUS(status);

      cleanup:
	/*
	 * We must undo everything done above; meaning that we must
	 * pop everything off of the operand stack and delete those
	 * objects
	 */
	acpi_ds_obj_stack_pop_and_delete(arg_count, walk_state);

	ACPI_EXCEPTION((AE_INFO, status, "While creating Arg %d", index));
	return_ACPI_STATUS(status);
}

/*****************************************************************************
 *
 * FUNCTION:    acpi_ds_evaluate_name_path
 *
 * PARAMETERS:  walk_state      - Current state of the parse tree walk,
 *                                the opcode of current operation should be
 *                                AML_INT_NAMEPATH_OP
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Translate the -name_path- parse tree object to the equivalent
 *              interpreter object, convert it to value, if needed, duplicate
 *              it, if needed, and push it onto the current result stack.
 *
 ****************************************************************************/

acpi_status acpi_ds_evaluate_name_path(struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_parse_object *op = walk_state->op;
	union acpi_operand_object **operand = &walk_state->operands[0];
	union acpi_operand_object *new_obj_desc;
	u8 type;

	ACPI_FUNCTION_TRACE_PTR(ds_evaluate_name_path, walk_state);

	if (!op->common.parent) {

		/* This happens after certain exception processing */

		goto exit;
	}

	if ((op->common.parent->common.aml_opcode == AML_PACKAGE_OP) ||
	    (op->common.parent->common.aml_opcode == AML_VAR_PACKAGE_OP) ||
	    (op->common.parent->common.aml_opcode == AML_REF_OF_OP)) {

		/* TBD: Should we specify this feature as a bit of op_info->Flags of these opcodes? */

		goto exit;
	}

	status = acpi_ds_create_operand(walk_state, op, 0);
	if (ACPI_FAILURE(status)) {
		goto exit;
	}

	if (op->common.flags & ACPI_PARSEOP_TARGET) {
		new_obj_desc = *operand;
		goto push_result;
	}

	type = (*operand)->common.type;

	status = acpi_ex_resolve_to_value(operand, walk_state);
	if (ACPI_FAILURE(status)) {
		goto exit;
	}

	if (type == ACPI_TYPE_INTEGER) {

		/* It was incremented by acpi_ex_resolve_to_value */

		acpi_ut_remove_reference(*operand);

		status =
		    acpi_ut_copy_iobject_to_iobject(*operand, &new_obj_desc,
						    walk_state);
		if (ACPI_FAILURE(status)) {
			goto exit;
		}
	} else {
		/*
		 * The object either was anew created or is
		 * a Namespace node - don't decrement it.
		 */
		new_obj_desc = *operand;
	}

	/* Cleanup for name-path operand */

	status = acpi_ds_obj_stack_pop(1, walk_state);
	if (ACPI_FAILURE(status)) {
		walk_state->result_obj = new_obj_desc;
		goto exit;
	}

      push_result:

	walk_state->result_obj = new_obj_desc;

	status = acpi_ds_result_push(walk_state->result_obj, walk_state);
	if (ACPI_SUCCESS(status)) {

		/* Force to take it from stack */

		op->common.flags |= ACPI_PARSEOP_IN_STACK;
	}

      exit:

	return_ACPI_STATUS(status);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      /*
 * DMA region bookkeeping routines
 *
 * Copyright (C) 2002 Maas Digital LLC
 *
 * This code is licensed under the GPL.  See the file COPYING in the root
 * directory of the kernel sources for details.
 */

#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/scatterlist.h>

#include "dma.h"

/* dma_prog_region */

void dma_prog_region_init(struct dma_prog_region *prog)
{
	prog->kvirt = NULL;
	prog->dev = NULL;
	prog->n_pages = 0;
	prog->bus_addr = 0;
}

int dma_prog_region_alloc(struct dma_prog_region *prog, unsigned long n_bytes,
			  struct pci_dev *dev)
{
	/* round up to page size */
	n_bytes = PAGE_ALIGN(n_bytes);

	prog->n_pages = n_bytes >> PAGE_SHIFT;

	prog->kvirt = pci_alloc_consistent(dev, n_bytes, &prog->bus_addr);
	if (!prog->kvirt) {
		printk(KERN_ERR
		       "dma_prog_region_alloc: pci_alloc_consistent() failed\n");
		dma_prog_region_free(prog);
		return -ENOMEM;
	}

	prog->dev = dev;

	return 0;
}

void dma_prog_region_free(struct dma_prog_region *prog)
{
	if (prog->kvirt) {
		pci_free_consistent(prog->dev, prog->n_pages << PAGE_SHIFT,
				    prog->kvirt, prog->bus_addr);
	}

	prog->kvirt = NULL;
	prog->dev = NULL;
	prog->n_pages = 0;
	prog->bus_addr = 0;
}

/* dma_region */

/**
 * dma_region_init - clear out all fields but do not allocate anything
 */
void dma_region_init(struct dma_region *dma)
{
	dma->kvirt = NULL;
	dma->dev = NULL;
	dma->n_pages = 0;
	dma->n_dma_pages = 0;
	dma->sglist = NULL;
}

/**
 * dma_region_alloc - allocate the buffer and map it to the IOMMU
 */
int dma_region_alloc(struct dma_region *dma, unsigned long n_bytes,
		     struct pci_dev *dev, int direction)
{
	unsigned int i;

	/* round up to page size */
	n_bytes = PAGE_ALIGN(n_bytes);

	dma->n_pages = n_bytes >> PAGE_SHIFT;

	dma->kvirt = vmalloc_32(n_bytes);
	if (!dma->kvirt) {
		printk(KERN_ERR "dma_region_alloc: vmalloc_32() failed\n");
		goto err;
	}

	/* Clear the ram out, no junk to the user */
	memset(dma->kvirt, 0, n_bytes);

	/* allocate scatter/gather list */
	dma->sglist = vmalloc(dma->n_pages * sizeof(*dma->sglist));
	if (!dma->sglist) {
		printk(KERN_ERR "dma_region_alloc: vmalloc(sglist) failed\n");
		goto err;
	}

	sg_init_table(dma->sglist, dma->n_pages);

	/* fill scatter/gather list with pages */
	for (i = 0; i < dma->n_pages; i++) {
		unsigned long va =
		    (unsigned long)dma->kvirt + (i << PAGE_SHIFT);

		sg_set_page(&dma->sglist[i], vmalloc_to_page((void *)va),
				PAGE_SIZE, 0);
	}

	/* map sglist to the IOMMU */
	dma->n_dma_pages =
	    pci_map_sg(dev, dma->sglist, dma->n_pages, direction);

	if (dma->n_dma_pages == 0) {
		printk(KERN_ERR "dma_region_alloc: pci_map_sg() failed\n");
		goto err;
	}

	dma->dev = dev;
	dma->direction = direction;

	return 0;

      err:
	dma_region_free(dma);
	return -ENOMEM;
}

/**
 * dma_region_free - unmap and free the buffer
 */
void dma_region_free(struct dma_region *dma)
{
	if (dma->n_dma_pages) {
		pci_unmap_sg(dma->dev, dma->sglist, dma->n_pages,
			     dma->direction);
		dma->n_dma_pages = 0;
		dma->dev = NULL;
	}

	vfree(dma->sglist);
	dma->sglist = NULL;

	vfree(dma->kvirt);
	dma->kvirt = NULL;
	dma->n_pages = 0;
}

/* find the scatterlist index and remaining offset corresponding to a
   given offset from the beginning of the buffer */
static inline int dma_region_find(struct dma_region *dma, unsigned long offset,
				  unsigned int start, unsigned long *rem)
{
	int i;
	unsigned long off = offset;

	for (i = start; i < dma->n_dma_pages; i++) {
		if (off < sg_dma_len(&dma->sglist[i])) {
			*rem = off;
			break;
		}

		off -= sg_dma_len(&dma->sglist[i]);
	}

	BUG_ON(i >= dma->n_dma_pages);

	return i;
}

/**
 * dma_region_offset_to_bus - get bus address of an offset within a DMA region
 *
 * Returns the DMA bus address of the byte with the given @offset relative to
 * the beginning of the @dma.
 */
dma_addr_t dma_region_offset_to_bus(struct dma_region * dma,
				    unsigned long offset)
{
	unsigned long rem = 0;

	struct scatterlist *sg =
	    &dma->sglist[dma_region_find(dma, offset, 0, &rem)];
	return sg_dma_address(sg) + rem;
}

/**
 * dma_region_sync_for_cpu - sync the CPU's view of the buffer
 */
void dma_region_sync_for_cpu(struct dma_region *dma, unsigned long offset,
			     unsigned long len)
{
	int first, last;
	unsigned long rem = 0;

	if (!len)
		len = 1;

	first = dma_region_find(dma, offset, 0, &rem);
	last = dma_region_find(dma, rem + len - 1, first, &rem);

	pci_dma_sync_sg_for_cpu(dma->dev, &dma->sglist[first], last - first + 1,
				dma->direction);
}

/**
 * dma_region_sync_for_device - sync the IO bus' view of the buffer
 */
void dma_region_sync_for_device(struct dma_region *dma, unsigned long offset,
				unsigned long len)
{
	int first, last;
	unsigned long rem = 0;

	if (!len)
		len = 1;

	first = dma_region_find(dma, offset, 0, &rem);
	last = dma_region_find(dma, rem + len - 1, first, &rem);

	pci_dma_sync_sg_for_device(dma->dev, &dma->sglist[first],
				   last - first + 1, dma->direction);
}

#ifdef CONFIG_MMU

static int dma_region_pagefault(struct vm_area_struct *vma,
				struct vm_fault *vmf)
{
	struct dma_region *dma = (struct dma_region *)vma->vm_private_data;

	if (!dma->kvirt)
		return VM_FAULT_SIGBUS;

	if (vmf->pgoff >= dma->n_pages)
		return VM_FAULT_SIGBUS;

	vmf->page = vmalloc_to_page(dma->kvirt + (vmf->pgoff << PAGE_SHIFT));
	get_page(vmf->page);
	return 0;
}

static struct vm_operations_struct dma_region_vm_ops = {
	.fault = dma_region_pagefault,
};

/**
 * dma_region_mmap - map the buffer into a user space process
 */
int dma_region_mmap(struct dma_region *dma, struct file *file,
		    struct vm_area_struct *vma)
{
	unsigned long size;

	if (!dma->kvirt)
		return -EINVAL;

	/* must be page-aligned (XXX: comment is wrong, we could allow pgoff) */
	if (vma->vm_pgoff != 0)
		return -EINVAL;

	/* check the length */
	size = vma->vm_end - vma->vm_start;
	if (size > (dma->n_pages << PAGE_SHIFT))
		return -EINVAL;

	vma->vm_ops = &dma_region_vm_ops;
	vma->vm_private_data = dma;
	vma->vm_file = file;
	vma->vm_flags |= VM_RESERVED | VM_ALWAYSDUMP;

	return 0;
}

#else				/* CONFIG_MMU */

int dma_region_mmap(struct dma_region *dma, struct file *file,
		    struct vm_area_struct *vma)
{
	return -EINVAL;
}

#endif				/* CONFIG_MMU */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        /*
 *	Video4Linux bw-qcam driver
 *
 *	Derived from code..
 */

/******************************************************************

Copyright (C) 1996 by Scott Laird

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL SCOTT LAIRD BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

/* One from column A... */
#define QC_NOTSET 0
#define QC_UNIDIR 1
#define QC_BIDIR  2
#define QC_SERIAL 3

/* ... and one from column B */
#define QC_ANY          0x00
#define QC_FORCE_UNIDIR 0x10
#define QC_FORCE_BIDIR  0x20
#define QC_FORCE_SERIAL 0x30
/* in the port_mode member */

#define QC_MODE_MASK    0x07
#define QC_FORCE_MASK   0x70

#define MAX_HEIGHT 243
#define MAX_WIDTH 336

/* Bit fields for status flags */
#define QC_PARAM_CHANGE	0x01 /* Camera status change has occurred */

struct qcam_device {
	struct video_device vdev;
	struct pardevice *pdev;
	struct parport *pport;
	struct mutex lock;
	int width, height;
	int bpp;
	int mode;
	int contrast, brightness, whitebal;
	int port_mode;
	int transfer_scale;
	int top, left;
	int status;
	unsigned int saved_bits;
	unsigned long in_use;
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               /*

    bttv-i2c.c  --  all the i2c code is here

    bttv - Bt848 frame grabber driver

    Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
			   & Marcus Metzler (mocm@thp.uni-koeln.de)
    (c) 1999-2003 Gerd Knorr <kraxel@bytesex.org>

    (c) 2005 Mauro Carvalho Chehab <mchehab@infradead.org>
	- Multituner support and i2c address binding

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>

#include "bttvp.h"
#include <media/v4l2-common.h>
#include <linux/jiffies.h>
#include <asm/io.h>

static int i2c_debug;
static int i2c_hw;
static int i2c_scan;
module_param(i2c_debug, int, 0644);
MODULE_PARM_DESC(i2c_hw,"configure i2c debug level");
module_param(i2c_hw,    int, 0444);
MODULE_PARM_DESC(i2c_hw,"force use of hardware i2c support, "
			"instead of software bitbang");
module_param(i2c_scan,  int, 0444);
MODULE_PARM_DESC(i2c_scan,"scan i2c bus at insmod time");

static unsigned int i2c_udelay = 5;
module_param(i2c_udelay, int, 0444);
MODULE_PARM_DESC(i2c_udelay,"soft i2c delay at insmod time, in usecs "
		"(should be 5 or higher). Lower value means higher bus speed.");

/* ----------------------------------------------------------------------- */
/* I2C functions - bitbanging adapter (software i2c)                       */

static void bttv_bit_setscl(void *data, int state)
{
	struct bttv *btv = (struct bttv*)data;

	if (state)
		btv->i2c_state |= 0x02;
	else
		btv->i2c_state &= ~0x02;
	btwrite(btv->i2c_state, BT848_I2C);
	btread(BT848_I2C);
}

static void bttv_bit_setsda(void *data, int state)
{
	struct bttv *btv = (struct bttv*)data;

	if (state)
		btv->i2c_state |= 0x01;
	else
		btv->i2c_state &= ~0x01;
	btwrite(btv->i2c_state, BT848_I2C);
	btread(BT848_I2C);
}

static int bttv_bit_getscl(void *data)
{
	struct bttv *btv = (struct bttv*)data;
	int state;

	state = btread(BT848_I2C) & 0x02 ? 1 : 0;
	return state;
}

static int bttv_bit_getsda(void *data)
{
	struct bttv *btv = (struct bttv*)data;
	int state;

	state = btread(BT848_I2C) & 0x01;
	return state;
}

static struct i2c_algo_bit_data __devinitdata bttv_i2c_algo_bit_template = {
	.setsda  = bttv_bit_setsda,
	.setscl  = bttv_bit_setscl,
	.getsda  = bttv_bit_getsda,
	.getscl  = bttv_bit_getscl,
	.udelay  = 16,
	.timeout = 200,
};

/* ----------------------------------------------------------------------- */
/* I2C functions - hardware i2c                                            */

static u32 functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_SMBUS_EMUL;
}

static int
bttv_i2c_wait_done(struct bttv *btv)
{
	int rc = 0;

	/* timeout */
	if (wait_event_interruptible_timeout(btv->i2c_queue,
		btv->i2c_done, msecs_to_jiffies(85)) == -ERESTARTSYS)

	rc = -EIO;

	if (btv->i2c_done & BT848_INT_RACK)
		rc = 1;
	btv->i2c_done = 0;
	return rc;
}

#define I2C_HW (BT878_I2C_MODE  | BT848_I2C_SYNC |\
		BT848_I2C_SCL | BT848_I2C_SDA)

static int
bttv_i2c_sendbytes(struct bttv *btv, const struct i2c_msg *msg, int last)
{
	u32 xmit;
	int retval,cnt;

	/* sanity checks */
	if (0 == msg->len)
		return -EINVAL;

	/* start, address + first byte */
	xmit = (msg->addr << 25) | (msg->buf[0] << 16) | I2C_HW;
	if (msg->len > 1 || !last)
		xmit |= BT878_I2C_NOSTOP;
	btwrite(xmit, BT848_I2C);
	retval = bttv_i2c_wait_done(btv);
	if (retval < 0)
		goto err;
	if (retval == 0)
		goto eio;
	if (i2c_debug) {
		printk(" <W %02x %02x", msg->addr << 1, msg->buf[0]);
		if (!(xmit & BT878_I2C_NOSTOP))
			printk(" >\n");
	}

	for (cnt = 1; cnt < msg->len; cnt++ ) {
		/* following bytes */
		xmit = (msg->buf[cnt] << 24) | I2C_HW | BT878_I2C_NOSTART;
		if (cnt < msg->len-1 || !last)
			xmit |= BT878_I2C_NOSTOP;
		btwrite(xmit, BT848_I2C);
		retval = bttv_i2c_wait_done(btv);
		if (retval < 0)
			goto err;
		if (retval == 0)
			goto eio;
		if (i2c_debug) {
			printk(" %02x", msg->buf[cnt]);
			if (!(xmit & BT878_I2C_NOSTOP))
				printk(" >\n");
		}
	}
	return msg->len;

 eio:
	retval = -EIO;
 err:
	if (i2c_debug)
		printk(" ERR: %d\n",retval);
	return retval;
}

static int
bttv_i2c_readbytes(struct bttv *btv, const struct i2c_msg *msg, int last)
{
	u32 xmit;
	u32 cnt;
	int retval;

	for(cnt = 0; cnt < msg->len; cnt++) {
		xmit = (msg->addr << 25) | (1 << 24) | I2C_HW;
		if (cnt < msg->len-1)
			xmit |= BT848_I2C_W3B;
		if (cnt < msg->len-1 || !last)
			xmit |= BT878_I2C_NOSTOP;
		if (cnt)
			xmit |= BT878_I2C_NOSTART;
		btwrite(xmit, BT848_I2C);
		retval = bttv_i2c_wait_done(btv);
		if (retval < 0)
			goto err;
		if (retval == 0)
			goto eio;
		msg->buf[cnt] = ((u32)btread(BT848_I2C) >> 8) & 0xff;
		if (i2c_debug) {
			if (!(xmit & BT878_I2C_NOSTART))
				printk(" <R %02x", (msg->addr << 1) +1);
			printk(" =%02x", msg->buf[cnt]);
			if (!(xmit & BT878_I2C_NOSTOP))
				printk(" >\n");
		}
	}
	return msg->len;

 eio:
	retval = -EIO;
 err:
	if (i2c_debug)
		printk(" ERR: %d\n",retval);
	return retval;
}

static int bttv_i2c_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg *msgs, int num)
{
	struct v4l2_device *v4l2_dev = i2c_get_adapdata(i2c_adap);
	struct bttv *btv = to_bttv(v4l2_dev);
	int retval = 0;
	int i;

	if (i2c_debug)
		printk("bt-i2c:");
	btwrite(BT848_INT_I2CDONE|BT848_INT_RACK, BT848_INT_STAT);
	for (i = 0 ; i < num; i++) {
		if (msgs[i].flags & I2C_M_RD) {
			/* read */
			retval = bttv_i2c_readbytes(btv, &msgs[i], i+1 == num);
			if (retval < 0)
				goto err;
		} else {
			/* write */
			retval = bttv_i2c_sendbytes(btv, &msgs[i], i+1 == num);
			if (retval < 0)
				goto err;
		}
	}
	return num;

 err:
	return retval;
}

static const struct i2c_algorithm bttv_algo = {
	.master_xfer   = bttv_i2c_xfer,
	.functionality = functionality,
};

/* ----------------------------------------------------------------------- */
/* I2C functions - common stuff                                            */

/* read I2C */
int bttv_I2CRead(struct bttv *btv, unsigned char addr, char *probe_for)
{
	unsigned char buffer = 0;

	if (0 != btv->i2c_rc)
		return -1;
	if (bttv_verbose && NULL != probe_for)
		printk(KERN_INFO "bttv%d: i2c: checking for %s @ 0x%02x... ",
		       btv->c.nr,probe_for,addr);
	btv->i2c_client.addr = addr >> 1;
	if (1 != i2c_master_recv(&btv->i2c_client, &buffer, 1)) {
		if (NULL != probe_for) {
			if (bttv_verbose)
				printk("not found\n");
		} else
			printk(KERN_WARNING "bttv%d: i2c read 0x%x: error\n",
			       btv->c.nr,addr);
		return -1;
	}
	if (bttv_verbose && NULL != probe_for)
		printk("found\n");
	return buffer;
}

/* write I2C */
int bttv_I2CWrite(struct bttv *btv, unsigned char addr, unsigned char b1,
		    unsigned char b2, int both)
{
	unsigned char buffer[2];
	int bytes = both ? 2 : 1;

	if (0 != btv->i2c_rc)
		return -1;
	btv->i2c_client.addr = addr >> 1;
	buffer[0] = b1;
	buffer[1] = b2;
	if (bytes != i2c_master_send(&btv->i2c_client, buffer, bytes))
		return -1;
	return 0;
}

/* read EEPROM content */
void __devinit bttv_readee(struct bttv *btv, unsigned char *eedata, int addr)
{
	memset(eedata, 0, 256);
	if (0 != btv->i2c_rc)
		return;
	btv->i2c_client.addr = addr >> 1;
	tveeprom_read(&btv->i2c_client, eedata, 256);
}

static char *i2c_devs[128] = {
	[ 0x1c >> 1 ] = "lgdt330x",
	[ 0x30 >> 1 ] = "IR (hauppauge)",
	[ 0x80 >> 1 ] = "msp34xx",
	[ 0x86 >> 1 ] = "tda9887",
	[ 0xa0 >> 1 ] = "eeprom",
	[ 0xc0 >> 1 ] = "tuner (analog)",
	[ 0xc2 >> 1 ] = "tuner (analog)",
};

static void do_i2c_scan(char *name, struct i2c_client *c)
{
	unsigned char buf;
	int i,rc;

	for (i = 0; i < ARRAY_SIZE(i2c_devs); i++) {
		c->addr = i;
		rc = i2c_master_recv(c,&buf,0);
		if (rc < 0)
			continue;
		printk("%s: i2c scan: found device @ 0x%x  [%s]\n",
		       name, i << 1, i2c_devs[i] ? i2c_devs[i] : "???");
	}
}

/* init + register i2c algo-bit adapter */
int __devinit init_bttv_i2c(struct bttv *btv)
{
	strlcpy(btv->i2c_client.name, "bttv internal", I2C_NAME_SIZE);

	if (i2c_hw)
		btv->use_i2c_hw = 1;
	if (btv->use_i2c_hw) {
		/* bt878 */
		strlcpy(btv->c.i2c_adap.name, "bt878",
			sizeof(btv->c.i2c_adap.name));
		btv->c.i2c_adap.id = I2C_HW_B_BT848;	/* FIXME */
		btv->c.i2c_adap.algo = &bttv_algo;
	} else {
		/* bt848 */
	/* Prevents usage of invalid delay values */
		if (i2c_udelay<5)
			i2c_udelay=5;

		strlcpy(btv->c.i2c_adap.name, "bttv",
			sizeof(btv->c.i2c_adap.name));
		btv->c.i2c_adap.id = I2C_HW_B_BT848;
		memcpy(&btv->i2c_algo, &bttv_i2c_algo_bit_template,
		       sizeof(bttv_i2c_algo_bit_template));
		btv->i2c_algo.udelay = i2c_udelay;
		btv->i2c_algo.data = btv;
		btv->c.i2c_adap.algo_data = &btv->i2c_algo;
	}
	btv->c.i2c_adap.owner = THIS_MODULE;

	btv->c.i2c_adap.dev.parent = &btv->c.pci->dev;
	snprintf(btv->c.i2c_adap.name, sizeof(btv->c.i2c_adap.name),
		 "bt%d #%d [%s]", btv->id, btv->c.nr,
		 btv->use_i2c_hw ? "hw" : "sw");

	i2c_set_adapdata(&btv->c.i2c_adap, &btv->c.v4l2_dev);
	btv->i2c_client.adapter = &btv->c.i2c_adap;


	if (btv->use_i2c_hw) {
		btv->i2c_rc = i2c_add_adapter(&btv->c.i2c_adap);
	} else {
		bttv_bit_setscl(btv,1);
		bttv_bit_setsda(btv,1);
		btv->i2c_rc = i2c_bit_add_bus(&btv->c.i2c_adap);
	}
	if (0 == btv->i2c_rc && i2c_scan)
		do_i2c_scan(btv->c.v4l2_dev.name, &btv->i2c_client);

	/* Instantiate the IR receiver device, if present */
	if (0 == btv->i2c_rc) {
		struct i2c_board_info info;
		/* The external IR receiver is at i2c address 0x34 (0x35 for
		   reads).  Future Hauppauge cards will have an internal
		   receiver at 0x30 (0x31 for reads).  In theory, both can be
		   fitted, and Hauppauge suggest an external overrides an
		   internal.

		   That's why we probe 0x1a (~0x34) first. CB
		*/
		const unsigned short addr_list[] = {
			0x1a, 0x18, 0x4b, 0x64, 0x30,
			I2C_CLIENT_END
		};

		memset(&info, 0, sizeof(struct i2c_board_info));
		strlcpy(info.type, "ir_video", I2C_NAME_SIZE);
		i2c_new_probed_device(&btv->c.i2c_adap, &info, addr_list);
	}
	return btv->i2c_rc;
}

int __devexit fini_bttv_i2c(struct bttv *btv)
{
	if (0 != btv->i2c_rc)
		return 0;

	return i2c_del_adapter(&btv->c.i2c_adap);
}

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           	CS4281_FIFO_SIZE,
};

static struct snd_pcm_hardware snd_cs4281_capture =
{
	.info =			SNDRV_PCM_INFO_MMAP |
				SNDRV_PCM_INFO_INTERLEAVED |
				SNDRV_PCM_INFO_MMAP_VALID |
				SNDRV_PCM_INFO_PAUSE |
				SNDRV_PCM_INFO_RESUME,
	.formats =		SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S8 |
				SNDRV_PCM_FMTBIT_U16_LE | SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_U16_BE | SNDRV_PCM_FMTBIT_S16_BE |
				SNDRV_PCM_FMTBIT_U32_LE | SNDRV_PCM_FMTBIT_S32_LE |
				SNDRV_PCM_FMTBIT_U32_BE | SNDRV_PCM_FMTBIT_S32_BE,
	.rates =		SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_48000,
	.rate_min =		4000,
	.rate_max =		48000,
	.channels_min =		1,
	.channels_max =		2,
	.buffer_bytes_max =	(512*1024),
	.period_bytes_min =	64,
	.period_bytes_max =	(512*1024),
	.periods_min =		1,
	.periods_max =		2,
	.fifo_size =		CS4281_FIFO_SIZE,
};

static int snd_cs4281_playback_open(struct snd_pcm_substream *substream)
{
	struct cs4281 *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct cs4281_dma *dma;

	dma = &chip->dma[0];
	dma->substream = substream;
	dma->left_slot = 0;
	dma->right_slot = 1;
	runtime->private_data = dma;
	runtime->hw = snd_cs4281_playback;
	/* should be detected from the AC'97 layer, but it seems
	   that although CS4297A rev B reports 18-bit ADC resolution,
	   samples are 20-bit */
	snd_pcm_hw_constraint_msbits(runtime, 0, 32, 20);
	return 0;
}

static int snd_cs4281_capture_open(struct snd_pcm_substream *substream)
{
	struct cs4281 *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct cs4281_dma *dma;

	dma = &chip->dma[1];
	dma->substream = substream;
	dma->left_slot = 10;
	dma->right_slot = 11;
	runtime->private_data = dma;
	runtime->hw = snd_cs4281_capture;
	/* should be detected from the AC'97 layer, but it seems
	   that although CS4297A rev B reports 18-bit ADC resolution,
	   samples are 20-bit */
	snd_pcm_hw_constraint_msbits(runtime, 0, 32, 20);
	return 0;
}

static int snd_cs4281_playback_close(struct snd_pcm_substream *substream)
{
	struct cs4281_dma *dma = substream->runtime->private_data;

	dma->substream = NULL;
	return 0;
}

static int snd_cs4281_capture_close(struct snd_pcm_substream *substream)
{
	struct cs4281_dma *dma = substream->runtime->private_data;

	dma->substream = NULL;
	return 0;
}

static struct snd_pcm_ops snd_cs4281_playback_ops = {
	.open =		snd_cs4281_playback_open,
	.close =	snd_cs4281_playback_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	snd_cs4281_hw_params,
	.hw_free =	snd_cs4281_hw_free,
	.prepare =	snd_cs4281_playback_prepare,
	.trigger =	snd_cs4281_trigger,
	.pointer =	snd_cs4281_pointer,
};

static struct snd_pcm_ops snd_cs4281_capture_ops = {
	.open =		snd_cs4281_capture_open,
	.close =	snd_cs4281_capture_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	snd_cs4281_hw_params,
	.hw_free =	snd_cs4281_hw_free,
	.prepare =	snd_cs4281_capture_prepare,
	.trigger =	snd_cs4281_trigger,
	.pointer =	snd_cs4281_pointer,
};

static int __devinit snd_cs4281_pcm(struct cs4281 * chip, int device,
				    struct snd_pcm ** rpcm)
{
	struct snd_pcm *pcm;
	int err;

	if (rpcm)
		*rpcm = NULL;
	err = snd_pcm_new(chip->card, "CS4281", device, 1, 1, &pcm);
	if (err < 0)
		return err;

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &snd_cs4281_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &snd_cs4281_capture_ops);

	pcm->private_data = chip;
	pcm->info_flags = 0;
	strcpy(pcm->name, "CS4281");
	chip->pcm = pcm;

	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV,
					      snd_dma_pci_data(chip->pci), 64*1024, 512*1024);

	if (rpcm)
		*rpcm = pcm;
	return 0;
}

/*
 *  Mixer section
 */

#define CS_VOL_MASK	0x1f

static int snd_cs4281_info_volume(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_info *uinfo)
{
	uinfo->type              = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count             = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = CS_VOL_MASK;
	return 0;
}
 
static int snd_cs4281_get_volume(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct cs4281 *chip = snd_kcontrol_chip(kcontrol);
	int regL = (kcontrol->private_value >> 16) & 0xffff;
	int regR = kcontrol->private_value & 0xffff;
	int volL, volR;

	volL = CS_VOL_MASK - (snd_cs4281_peekBA0(chip, regL) & CS_VOL_MASK);
	volR = CS_VOL_MASK - (snd_cs4281_peekBA0(chip, regR) & CS_VOL_MASK);

	ucontrol->value.integer.value[0] = volL;
	ucontrol->value.integer.value[1] = volR;
	return 0;
}

static int snd_cs4281_put_volume(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct cs4281 *chip = snd_kcontrol_chip(kcontrol);
	int change = 0;
	int regL = (kcontrol->private_value >> 16) & 0xffff;
	int regR = kcontrol->private_value & 0xffff;
	int volL, volR;

	volL = CS_VOL_MASK - (snd_cs4281_peekBA0(chip, regL) & CS_VOL_MASK);
	volR = CS_VOL_MASK - (snd_cs4281_peekBA0(chip, regR) & CS_VOL_MASK);

	if (ucontrol->value.integer.value[0] != volL) {
		volL = CS_VOL_MASK - (ucontrol->value.integer.value[0] & CS_VOL_MASK);
		snd_cs4281_pokeBA0(chip, regL, volL);
		change = 1;
	}
	if (ucontrol->value.integer.value[1] != volR) {
		volR = CS_VOL_MASK - (ucontrol->value.integer.value[1] & CS_VOL_MASK);
		snd_cs4281_pokeBA0(chip, regR, volR);
		change = 1;
	}
	return change;
}

static const DECLARE_TLV_DB_SCALE(db_scale_dsp, -4650, 150, 0);

static struct snd_kcontrol_new snd_cs4281_fm_vol = 
{
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Synth Playback Volume",
	.info = snd_cs4281_info_volume, 
	.get = snd_cs4281_get_volume,
	.put = snd_cs4281_put_volume, 
	.private_value = ((BA0_FMLVC << 16) | BA0_FMRVC),
	.tlv = { .p = db_scale_dsp },
};

static struct snd_kcontrol_new snd_cs4281_pcm_vol = 
{
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "PCM Stream Playback Volume",
	.info = snd_cs4281_info_volume, 
	.get = snd_cs4281_get_volume,
	.put = snd_cs4281_put_volume, 
	.private_value = ((BA0_PPLVC << 16) | BA0_PPRVC),
	.tlv = { .p = db_scale_dsp },
};

static void snd_cs4281_mixer_free_ac97_bus(struct snd_ac97_bus *bus)
{
	struct cs4281 *chip = bus->private_data;
	chip->ac97_bus = NULL;
}

static void snd_cs4281_mixer_free_ac97(struct snd_ac97 *ac97)
{
	struct cs4281 *chip = ac97->private_data;
	if (ac97->num)
		chip->ac97_secondary = NULL;
	else
		chip->ac97 = NULL;
}

static int __devinit snd_cs4281_mixer(struct cs4281 * chip)
{
	struct snd_card *card = chip->card;
	struct snd_ac97_template ac97;
	int err;
	static struct snd_ac97_bus_ops ops = {
		.write = snd_cs4281_ac97_write,
		.read = snd_cs4281_ac97_read,
	};

	if ((err = snd_ac97_bus(card, 0, &ops, chip, &chip->ac97_bus)) < 0)
		return err;
	chip->ac97_bus->private_free = snd_cs4281_mixer_free_ac97_bus;

	memset(&ac97, 0, sizeof(ac97));
	ac97.private_data = chip;
	ac97.private_free = snd_cs4281_mixer_free_ac97;
	if ((err = snd_ac97_mixer(chip->ac97_bus, &ac97, &chip->ac97)) < 0)
		return err;
	if (chip->dual_codec) {
		ac97.num = 1;
		if ((err = snd_ac97_mixer(chip->ac97_bus, &ac97, &chip->ac97_secondary)) < 0)
			return err;
	}
	if ((err = snd_ctl_add(card, snd_ctl_new1(&snd_cs4281_fm_vol, chip))) < 0)
		return err;
	if ((err = snd_ctl_add(card, snd_ctl_new1(&snd_cs4281_pcm_vol, chip))) < 0)
		return err;
	return 0;
}


/*
 * proc interface
 */

static void snd_cs4281_proc_read(struct snd_info_entry *entry, 
				  struct snd_info_buffer *buffer)
{
	struct cs4281 *chip = entry->private_data;

	snd_iprintf(buffer, "Cirrus Logic CS4281\n\n");
	snd_iprintf(buffer, "Spurious half IRQs   : %u\n", chip->spurious_dhtc_irq);
	snd_iprintf(buffer, "Spurious end IRQs    : %u\n", chip->spurious_dtc_irq);
}

static long snd_cs4281_BA0_read(struct snd_info_entry *entry,
				void *file_private_data,
				struct file *file, char __user *buf,
				unsigned long count, unsigned long pos)
{
	long size;
	struct cs4281 *chip = entry->private_data;
	
	size = count;
	if (pos + size > CS4281_BA0_SIZE)
		size = (long)CS4281_BA0_SIZE - pos;
	if (size > 0) {
		if (copy_to_user_fromio(buf, chip->ba0 + pos, size))
			return -EFAULT;
	}
	return size;
}

static long snd_cs4281_BA1_read(struct snd_info_entry *entry,
				void *file_private_data,
				struct file *file, char __user *buf,
				unsigned long count, unsigned long pos)
{
	long size;
	struct cs4281 *chip = entry->private_data;
	
	size = count;
	if (pos + size > CS4281_BA1_SIZE)
		size = (long)CS4281_BA1_SIZE - pos;
	if (size > 0) {
		if (copy_to_user_fromio(buf, chip->ba1 + pos, size))
			return -EFAULT;
	}
	return size;
}

static struct snd_info_entry_ops snd_cs4281_proc_ops_BA0 = {
	.read = snd_cs4281_BA0_read,
};

static struct snd_info_entry_ops snd_cs4281_proc_ops_BA1 = {
	.read = snd_cs4281_BA1_read,
};

static void __devinit snd_cs4281_proc_init(struct cs4281 * chip)
{
	struct snd_info_entry *entry;

	if (! snd_card_proc_new(chip->card, "cs4281", &entry))
		snd_info_set_text_ops(entry, chip, snd_cs4281_proc_read);
	if (! snd_card_proc_new(chip->card, "cs4281_BA0", &entry)) {
		entry->content = SNDRV_INFO_CONTENT_DATA;
		entry->private_data = chip;
		entry->c.ops = &snd_cs4281_proc_ops_BA0;
		entry->size = CS4281_BA0_SIZE;
	}
	if (! snd_card_proc_new(chip->card, "cs4281_BA1", &entry)) {
		entry->content = SNDRV_INFO_CONTENT_DATA;
		entry->private_data = chip;
		entry->c.ops = &snd_cs4281_proc_ops_BA1;
		entry->size = CS4281_BA1_SIZE;
	}
}

/*
 * joystick support
 */

#if defined(CONFIG_GAMEPORT) || (defined(MODULE) && defined(CONFIG_GAMEPORT_MODULE))

static void snd_cs4281_gameport_trigger(struct gameport *gameport)
{
	struct cs4281 *chip = gameport_get_port_data(gameport);

	if (snd_BUG_ON(!chip))
		return;
	snd_cs4281_pokeBA0(chip, BA0_JSPT, 0xff);
}

static unsigned char snd_cs4281_gameport_read(struct gameport *gameport)
{
	struct cs4281 *chip = gameport_get_port_data(gameport);

	if (snd_BUG_ON(!chip))
		return 0;
	return snd_cs4281_peekBA0(chip, BA0_JSPT);
}

#ifdef COOKED_MODE
static int snd_cs4281_gameport_cooked_read(struct gameport *gameport,
					   int *axes, int *buttons)
{
	struct cs4281 *chip = gameport_get_port_data(gameport);
	unsigned js1, js2, jst;
	
	if (snd_BUG_ON(!chip))
		return 0;

	js1 = snd_cs4281_peekBA0(chip, BA0_JSC1);
	js2 = snd_cs4281_peekBA0(chip, BA0_JSC2);
	jst = snd_cs4281_peekBA0(chip, BA0_JSPT);
	
	*buttons = (~jst >> 4) & 0x0F; 
	
	axes[0] = ((js1 & JSC1_Y1V_MASK) >> JSC1_Y1V_SHIFT) & 0xFFFF;
	axes[1] = ((js1 & JSC1_X1V_MASK) >> JSC1_X1V_SHIFT) & 0xFFFF;
	axes[2] = ((js2 & JSC2_Y2V_MASK) >> JSC2_Y2V_SHIFT) & 0xFFFF;
	axes[3] = ((js2 & JSC2_X2V_MASK) >> JSC2_X2V_SHIFT) & 0xFFFF;

	for (jst = 0; jst < 4; ++jst)
		if (axes[jst] == 0xFFFF) axes[jst] = -1;
	return 0;
}
#else
#define snd_cs4281_gameport_cooked_read	NULL
#endif

static int snd_cs4281_gameport_open(struct gameport *gameport, int mode)
{
	switch (mode) {
#ifdef COOKED_MODE
	case GAMEPORT_MODE_COOKED:
		return 0;
#endif
	case GAMEPORT_MODE_RAW:
		return 0;
	default:
		return -1;
	}
	return 0;
}

static int __devinit snd_cs4281_create_gameport(struct cs4281 *chip)
{
	struct gameport *gp;

	chip->gameport = gp = gameport_allocate_port();
	if (!gp) {
		printk(KERN_ERR "cs4281: cannot allocate memory for gameport\n");
		return -ENOMEM;
	}

	gameport_set_name(gp, "CS4281 Gameport");
	gameport_set_phys(gp, "pci%s/gameport0", pci_name(chip->pci));
	gameport_set_dev_parent(gp, &chip->pci->dev);
	gp->open = snd_cs4281_gameport_open;
	gp->read = snd_cs4281_gameport_read;
	gp->trigger = snd_cs4281_gameport_trigger;
	gp->cooked_read = snd_cs4281_gameport_cooked_read;
	gameport_set_port_data(gp, chip);

	snd_cs4281_pokeBA0(chip, BA0_JSIO, 0xFF); // ?
	snd_cs4281_pokeBA0(chip, BA0_JSCTL, JSCTL_SP_MEDIUM_SLOW);

	gameport_register_port(gp);

	return 0;
}

static void snd_cs4281_free_gameport(struct cs4281 *chip)
{
	if (chip->gameport) {
		gameport_unregister_port(chip->gameport);
		chip->gameport = NULL;
	}
}
#else
static inline int snd_cs4281_create_gameport(struct cs4281 *chip) { return -ENOSYS; }
static inline void snd_cs4281_free_gameport(struct cs4281 *chip) { }
#endif /* CONFIG_GAMEPORT || (MODULE && CONFIG_GAMEPORT_MODULE) */

static int snd_cs4281_free(struct cs4281 *chip)
{
	snd_cs4281_free_gameport(chip);

	if (chip->irq >= 0)
		synchronize_irq(chip->irq);

	/* Mask interrupts */
	snd_cs4281_pokeBA0(chip, BA0_ram might be trying access hardware directly.\n",
				       data == ATKBD_RET_ACK ? "ACK" : "NAK", serio->phys);
			goto out;
		case ATKBD_RET_ERR:
			atkbd->err_count++;
#ifdef ATKBD_DEBUG
			printk(KERN_DEBUG "atkbd.c: Keyboard on %s reports too many keys pressed.\n", serio->phys);
#endif
			goto out;
	}

	code = atkbd_compat_scancode(atkbd, code);

	if (atkbd->emul && --atkbd->emul)
		goto out;

	keycode = atkbd->keycode[code];

	if (keycode != ATKBD_KEY_NULL)
		input_event(dev, EV_MSC, MSC_SCAN, code);

	switch (keycode) {
		case ATKBD_KEY_NULL:
			break;
		case ATKBD_KEY_UNKNOWN:
			printk(KERN_WARNING
			       "atkbd.c: Unknown key %s (%s set %d, code %#x on %s).\n",
			       atkbd->release ? "released" : "pressed",
			       atkbd->translated ? "translated" : "raw",
			       atkbd->set, code, serio->phys);
			printk(KERN_WARNING
			       "atkbd.c: Use 'setkeycodes %s%02x <keycode>' to make it known.\n",
			       code & 0x80 ? "e0" : "", code & 0x7f);
			input_sync(dev);
			break;
		case ATKBD_SCR_1:
			scroll = 1 - atkbd->release * 2;
			break;
		case ATKBD_SCR_2:
			scroll = 2 - atkbd->release * 4;
			break;
		case ATKBD_SCR_4:
			scroll = 4 - atkbd->release * 8;
			break;
		case ATKBD_SCR_8:
			scroll = 8 - atkbd->release * 16;
			break;
		case ATKBD_SCR_CLICK:
			click = !atkbd->release;
			break;
		case ATKBD_SCR_LEFT:
			hscroll = -1;
			break;
		case ATKBD_SCR_RIGHT:
			hscroll = 1;
			break;
		default:
			if (atkbd->release) {
				value = 0;
				atkbd->last = 0;
			} else if (!atkbd->softrepeat && test_bit(keycode, dev->key)) {
				/* Workaround Toshiba laptop multiple keypress */
				value = time_before(jiffies, atkbd->time) && atkbd->last == code ? 1 : 2;
			} else {
				value = 1;
				atkbd->last = code;
				atkbd->time = jiffies + msecs_to_jiffies(dev->rep[REP_DELAY]) / 2;
			}

			input_event(dev, EV_KEY, keycode, value);
			input_sync(dev);

			if (value && test_bit(code, atkbd->force_release_mask)) {
				input_report_key(dev, keycode, 0);
				input_sync(dev);
			}
	}

	if (atkbd->scroll) {
		if (click != -1)
			input_report_key(dev, BTN_MIDDLE, click);
		input_report_rel(dev, REL_WHEEL, scroll);
		input_report_rel(dev, REL_HWHEEL, hscroll);
		input_sync(dev);
	}

	atkbd->release = 0;
out:
	return IRQ_HANDLED;
}

static int atkbd_set_repeat_rate(struct atkbd *atkbd)
{
	const short period[32] =
		{ 33,  37,  42,  46,  50,  54,  58,  63,  67,  75,  83,  92, 100, 109, 116, 125,
		 133, 149, 167, 182, 200, 217, 232, 250, 270, 303, 333, 370, 400, 435, 470, 500 };
	const short delay[4] =
		{ 250, 500, 750, 1000 };

	struct input_dev *dev = atkbd->dev;
	unsigned char param;
	int i = 0, j = 0;

	while (i < ARRAY_SIZE(period) - 1 && period[i] < dev->rep[REP_PERIOD])
		i++;
	dev->rep[REP_PERIOD] = period[i];

	while (j < ARRAY_SIZE(delay) - 1 && delay[j] < dev->rep[REP_DELAY])
		j++;
	dev->rep[REP_DELAY] = delay[j];

	param = i | (j << 5);
	return ps2_command(&atkbd->ps2dev, &param, ATKBD_CMD_SETREP);
}

static int atkbd_set_leds(struct atkbd *atkbd)
{
	struct input_dev *dev = atkbd->dev;
	unsigned char param[2];

	param[0] = (test_bit(LED_SCROLLL, dev->led) ? 1 : 0)
		 | (test_bit(LED_NUML,    dev->led) ? 2 : 0)
		 | (test_bit(LED_CAPSL,   dev->led) ? 4 : 0);
	if (ps2_command(&atkbd->ps2dev, param, ATKBD_CMD_SETLEDS))
		return -1;

	if (atkbd->extra) {
		param[0] = 0;
		param[1] = (test_bit(LED_COMPOSE, dev->led) ? 0x01 : 0)
			 | (test_bit(LED_SLEEP,   dev->led) ? 0x02 : 0)
			 | (test_bit(LED_SUSPEND, dev->led) ? 0x04 : 0)
			 | (test_bit(LED_MISC,    dev->led) ? 0x10 : 0)
			 | (test_bit(LED_MUTE,    dev->led) ? 0x20 : 0);
		if (ps2_command(&atkbd->ps2dev, param, ATKBD_CMD_EX_SETLEDS))
			return -1;
	}

	return 0;
}

/*
 * atkbd_event_work() is used to complete processing of events that
 * can not be processed by input_event() which is often called from
 * interrupt context.
 */

static void atkbd_event_work(struct work_struct *work)
{
	struct atkbd *atkbd = container_of(work, struct atkbd, event_work.work);

	mutex_lock(&atkbd->event_mutex);

	if (test_and_clear_bit(ATKBD_LED_EVENT_BIT, &atkbd->event_mask))
		atkbd_set_leds(atkbd);

	if (test_and_clear_bit(ATKBD_REP_EVENT_BIT, &atkbd->event_mask))
		atkbd_set_repeat_rate(atkbd);

	mutex_unlock(&atkbd->event_mutex);
}

/*
 * Schedule switch for execution. We need to throttle requests,
 * otherwise keyboard may become unresponsive.
 */
static void atkbd_schedule_event_work(struct atkbd *atkbd, int event_bit)
{
	unsigned long delay = msecs_to_jiffies(50);

	if (time_after(jiffies, atkbd->event_jiffies + delay))
		delay = 0;

	atkbd->event_jiffies = jiffies;
	set_bit(event_bit, &atkbd->event_mask);
	wmb();
	schedule_delayed_work(&atkbd->event_work, delay);
}

/*
 * Event callback from the input module. Events that change the state of
 * the hardware are processed here. If action can not be performed in
 * interrupt context it is offloaded to atkbd_event_work.
 */

static int atkbd_event(struct input_dev *dev,
			unsigned int type, unsigned int code, int value)
{
	struct atkbd *atkbd = input_get_drvdata(dev);

	if (!atkbd->write)
		return -1;

	switch (type) {

		case EV_LED:
			atkbd_schedule_event_work(atkbd, ATKBD_LED_EVENT_BIT);
			return 0;

		case EV_REP:
			if (!atkbd->softrepeat)
				atkbd_schedule_event_work(atkbd, ATKBD_REP_EVENT_BIT);
			return 0;
	}

	return -1;
}

/*
 * atkbd_enable() signals that interrupt handler is allowed to
 * generate input events.
 */

static inline void atkbd_enable(struct atkbd *atkbd)
{
	serio_pause_rx(atkbd->ps2dev.serio);
	atkbd->enabled = 1;
	serio_continue_rx(atkbd->ps2dev.serio);
}

/*
 * atkbd_disable() tells input handler that all incoming data except
 * for ACKs and command response should be dropped.
 */

static inline void atkbd_disable(struct atkbd *atkbd)
{
	serio_pause_rx(atkbd->ps2dev.serio);
	atkbd->enabled = 0;
	serio_continue_rx(atkbd->ps2dev.serio);
}

/*
 * atkbd_probe() probes for an AT keyboard on a serio port.
 */

static int atkbd_probe(struct atkbd *atkbd)
{
	struct ps2dev *ps2dev = &atkbd->ps2dev;
	unsigned char param[2];

/*
 * Some systems, where the bit-twiddling when testing the io-lines of the
 * controller may confuse the keyboard need a full reset of the keyboard. On
 * these systems the BIOS also usually doesn't do it for us.
 */

	if (atkbd_reset)
		if (ps2_command(ps2dev, NULL, ATKBD_CMD_RESET_BAT))
			printk(KERN_WARNING "atkbd.c: keyboard reset failed on %s\n", ps2dev->serio->phys);

/*
 * Then we check the keyboard ID. We should get 0xab83 under normal conditions.
 * Some keyboards report different values, but the first byte is always 0xab or
 * 0xac. Some old AT keyboards don't report anything. If a mouse is connected, this
 * should make sure we don't try to set the LEDs on it.
 */

	param[0] = param[1] = 0xa5;	/* initialize with invalid values */
	if (ps2_command(ps2dev, param, ATKBD_CMD_GETID)) {

/*
 * If the get ID command failed, we check if we can at least set the LEDs on
 * the keyboard. This should work on every keyboard out there. It also turns
 * the LEDs off, which we want anyway.
 */
		param[0] = 0;
		if (ps2_command(ps2dev, param, ATKBD_CMD_SETLEDS))
			return -1;
		atkbd->id = 0xabba;
		return 0;
	}

	if (!ps2_is_keyboard_id(param[0]))
		return -1;

	atkbd->id = (param[0] << 8) | param[1];

	if (atkbd->id == 0xaca1 && atkbd->translated) {
		printk(KERN_ERR "atkbd.c: NCD terminal keyboards are only supported on non-translating\n");
		printk(KERN_ERR "atkbd.c: controllers. Use i8042.direct=1 to disable translation.\n");
		return -1;
	}

	return 0;
}

/*
 * atkbd_select_set checks if a keyboard has a working Set 3 support, and
 * sets it into that. Unfortunately there are keyboards that can be switched
 * to Set 3, but don't work well in that (BTC Multimedia ...)
 */

static int atkbd_select_set(struct atkbd *atkbd, int target_set, int allow_extra)
{
	struct ps2dev *ps2dev = &atkbd->ps2dev;
	unsigned char param[2];

	atkbd->extra = 0;
/*
 * For known special keyboards we can go ahead and set the correct set.
 * We check for NCD PS/2 Sun, NorthGate OmniKey 101 and
 * IBM RapidAccess / IBM EzButton / Chicony KBP-8993 keyboards.
 */

	if (atkbd->translated)
		return 2;

	if (atkbd->id	snd_cs4281_pokeBA0(chip, dma->regFCR,
				   BA0_FCR_LS(31) |
				   BA0_FCR_RS(31) |
				   BA0_FCR_SZ(CS4281_FIFO_SIZE) |
				   BA0_FCR_OF(dma->fifo_offset));
	}

	chip->src_left_play_slot = 0;	/* AC'97 left PCM playback (3) */
	chip->src_right_play_slot = 1;	/* AC'97 right PCM playback (4) */
	chip->src_left_rec_slot = 10;	/* AC'97 left PCM record (3) */
	chip->src_right_rec_slot = 11;	/* AC'97 right PCM record (4) */

	/* Activate wave playback FIFO for FM playback */
	chip->dma[0].valFCR = BA0_FCR_FEN | BA0_FCR_LS(0) |
		              BA0_FCR_RS(1) |
 	  	              BA0_FCR_SZ(CS4281_FIFO_SIZE) |
		              BA0_FCR_OF(chip->dma[0].fifo_offset);
	snd_cs4281_pokeBA0(chip, chip->dma[0].regFCR, chip->dma[0].valFCR);
	snd_cs4281_pokeBA0(chip, BA0_SRCSA, (chip->src_left_play_slot << 0) |
					    (chip->src_right_play_slot << 8) |
					    (chip->src_left_rec_slot << 16) |
					    (chip->src_right_rec_slot << 24));

	/* Initialize digital volume */
	snd_cs4281_pokeBA0(chip, BA0_PPLVC, 0);
	snd_cs4281_pokeBA0(chip, BA0_PPRVC, 0);

	/* Enable IRQs */
	snd_cs4281_pokeBA0(chip, BA0_HICR, BA0_HICR_EOI);
	/* Unmask interrupts */
	snd_cs4281_pokeBA0(chip, BA0_HIMR, 0x7fffffff & ~(
					BA0_HISR_MIDI |
					BA0_HISR_DMAI |
					BA0_HISR_DMA(0) |
					BA0_HISR_DMA(1) |
					BA0_HISR_DMA(2) |
					BA0_HISR_DMA(3)));
	synchronize_irq(chip->irq);

	return 0;
}

/*
 *  MIDI section
 */

static void snd_cs4281_midi_reset(struct cs4281 *chip)
{
	snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr | BA0_MIDCR_MRST);
	udelay(100);
	snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
}

static int snd_cs4281_midi_input_open(struct snd_rawmidi_substream *substream)
{
	struct cs4281 *chip = substream->rmidi->private_data;

	spin_lock_irq(&chip->reg_lock);
 	chip->midcr |= BA0_MIDCR_RXE;
	chip->midi_input = substream;
	if (!(chip->uartm & CS4281_MODE_OUTPUT)) {
		snd_cs4281_midi_reset(chip);
	} else {
		snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
	}
	spin_unlock_irq(&chip->reg_lock);
	return 0;
}

static int snd_cs4281_midi_input_close(struct snd_rawmidi_substream *substream)
{
	struct cs4281 *chip = substream->rmidi->private_data;

	spin_lock_irq(&chip->reg_lock);
	chip->midcr &= ~(BA0_MIDCR_RXE | BA0_MIDCR_RIE);
	chip->midi_input = NULL;
	if (!(chip->uartm & CS4281_MODE_OUTPUT)) {
		snd_cs4281_midi_reset(chip);
	} else {
		snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
	}
	chip->uartm &= ~CS4281_MODE_INPUT;
	spin_unlock_irq(&chip->reg_lock);
	return 0;
}

static int snd_cs4281_midi_output_open(struct snd_rawmidi_substream *substream)
{
	struct cs4281 *chip = substream->rmidi->private_data;

	spin_lock_irq(&chip->reg_lock);
	chip->uartm |= CS4281_MODE_OUTPUT;
	chip->midcr |= BA0_MIDCR_TXE;
	chip->midi_output = substream;
	if (!(chip->uartm & CS4281_MODE_INPUT)) {
		snd_cs4281_midi_reset(chip);
	} else {
		snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
	}
	spin_unlock_irq(&chip->reg_lock);
	return 0;
}

static int snd_cs4281_midi_output_close(struct snd_rawmidi_substream *substream)
{
	struct cs4281 *chip = substream->rmidi->private_data;

	spin_lock_irq(&chip->reg_lock);
	chip->midcr &= ~(BA0_MIDCR_TXE | BA0_MIDCR_TIE);
	chip->midi_output = NULL;
	if (!(chip->uartm & CS4281_MODE_INPUT)) {
		snd_cs4281_midi_reset(chip);
	} else {
		snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
	}
	chip->uartm &= ~CS4281_MODE_OUTPUT;
	spin_unlock_irq(&chip->reg_lock);
	return 0;
}

static void snd_cs4281_midi_input_trigger(struct snd_rawmidi_substream *substream, int up)
{
	unsigned long flags;
	struct cs4281 *chip = substream->rmidi->private_data;

	spin_lock_irqsave(&chip->reg_lock, flags);
	if (up) {
		if ((chip->midcr & BA0_MIDCR_RIE) == 0) {
			chip->midcr |= BA0_MIDCR_RIE;
			snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
		}
	} else {
		if (chip->midcr & BA0_MIDCR_RIE) {
			chip->midcr &= ~BA0_MIDCR_RIE;
			snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
		}
	}
	spin_unlock_irqrestore(&chip->reg_lock, flags);
}

static void snd_cs4281_midi_output_trigger(struct snd_rawmidi_substream *substream, int up)
{
	unsigned long flags;
	struct cs4281 *chip = substream->rmidi->private_data;
	unsigned char byte;

	spin_lock_irqsave(&chip->reg_lock, flags);
	if (up) {
		if ((chip->midcr & BA0_MIDCR_TIE) == 0) {
			chip->midcr |= BA0_MIDCR_TIE;
			/* fill UART FIFO buffer at first, and turn Tx interrupts only if necessary */
			while ((chip->midcr & BA0_MIDCR_TIE) &&
			       (snd_cs4281_peekBA0(chip, BA0_MIDSR) & BA0_MIDSR_TBF) == 0) {
				if (snd_rawmidi_transmit(substream, &byte, 1) != 1) {
					chip->midcr &= ~BA0_MIDCR_TIE;
				} else {
					snd_cs4281_pokeBA0(chip, BA0_MIDWP, byte);
				}
			}
			snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
		}
	} else {
		if (chip->midcr & BA0_MIDCR_TIE) {
			chip->midcr &= ~BA0_MIDCR_TIE;
			snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
		}
	}
	spin_unlock_irqrestore(&chip->reg_lock, flags);
}

static struct snd_rawmidi_ops snd_cs4281_midi_output =
{
	.open =		snd_cs4281_midi_output_open,
	.close =	snd_cs4281_midi_output_close,
	.trigger =	snd_cs4281_midi_output_trigger,
};

static struct snd_rawmidi_ops snd_cs4281_midi_input =
{
	.open = 	snd_cs4281_midi_input_open,
	.close =	snd_cs4281_midi_input_close,
	.trigger =	snd_cs4281_midi_input_trigger,
};

static int __devinit snd_cs4281_midi(struct cs4281 * chip, int device,
				     struct snd_rawmidi **rrawmidi)
{
	struct snd_rawmidi *rmidi;
	int err;

	if (rrawmidi)
		*rrawmidi = NULL;
	if ((err = snd_rawmidi_new(chip->card, "CS4281", device, 1, 1, &rmidi)) < 0)
		return err;
	strcpy(rmidi->name, "CS4281");
	snd_rawmidi_set_ops(rmidi, SNDRV_RAWMIDI_STREAM_OUTPUT, &snd_cs4281_midi_output);
	snd_rawmidi_set_ops(rmidi, SNDRV_RAWMIDI_STREAM_INPUT, &snd_cs4281_midi_input);
	rmidi->info_flags |= SNDRV_RAWMIDI_INFO_OUTPUT | SNDRV_RAWMIDI_INFO_INPUT | SNDRV_RAWMIDI_INFO_DUPLEX;
	rmidi->private_data = chip;
	chip->rmidi = rmidi;
	if (rrawmidi)
		*rrawmidi = rmidi;
	return 0;
}

/*
 *  Interrupt handler
 */

static irqreturn_t snd_cs4281_interrupt(int irq, void *dev_id)
{
	struct cs4281 *chip = dev_id;
	unsigned int status, dma, val;
	struct cs4281_dma *cdma;

	if (chip == NULL)
		return IRQ_NONE;
	status = snd_cs4281_peekBA0(chip, BA0_HISR);
	if ((status & 0x7fffffff) == 0) {
		snd_cs4281_pokeBA0(chip, BA0_HICR, BA0_HICR_EOI);
		return IRQ_NONE;
	}

	if (status & (BA0_HISR_DMA(0)|BA0_HISR_DMA(1)|BA0_HISR_DMA(2)|BA0_HISR_DMA(3))) {
		for (dma = 0; dma < 4; dma++)
			if (status & BA0_HISR_DMA(dma)) {
				cdma = &chip->dma[dma];
				spin_lock(&chip->reg_lock);
				/* ack DMA IRQ */
				val = snd_cs4281_peekBA0(chip, cdma->regHDSR);
				/* workaround, sometimes CS4281 acknowledges */
				/* end or middle transfer position twice */
				cdma->frag++;
				if ((val & BA0_HDSR_DHTC) && !(cdma->frag & 1)) {
					cdma->frag--;
					chip->spurious_dhtc_irq++;
					spin_unlock(&chip->reg_lock);
					continue;
				}
				if ((val & BA0_HDSR_DTC) && (cdma->frag & 1)) {
					cdma->frag--;
					chip->spurious_dtc_irq++;
					spin_unlock(&chip->reg_lock);
					continue;
				}
				spin_unlock(&chip->reg_lock);
				snd_pcm_period_elapsed(cdma->substream);
			}
	}

	if ((status & BA0_HISR_MIDI) && chip->rmidi) {
		unsigned char c;
		
		spin_lock(&chip->reg_lock);
		while ((snd_cs4281_peekBA0(chip, BA0_MIDSR) & BA0_MIDSR_RBE) == 0) {
			c = snd_cs4281_peekBA0(chip, BA0_MIDRP);
			if ((chip->midcr & BA0_MIDCR_RIE) == 0)
				continue;
			snd_rawmidi_receive(chip->midi_input, &c, 1);
		}
		while ((snd_cs4281_peekBA0(chip, BA0_MIDSR) & BA0_MIDSR_TBF) == 0) {
			if ((chip->midcr & BA0_MIDCR_TIE) == 0)
				break;
			if (snd_rawmidi_transmit(chip->midi_output, &c, 1) != 1) {
				chip->midcr &= ~BA0_MIDCR_TIE;
				snd_cs4281_pokeBA0(chip, BA0_MIDCR, chip->midcr);
				break;
			}
			snd_cs4281_pokeBA0(chip, BA0_MIDWP, c);
		}
		spin_unlock(&chip->reg_lock);
	}

	/* EOI to the PCI part... reenables interrupts */
	snd_cs4281_pokeBA0(chip, BA0_HICR, BA0_HICR_EOI);

	return IRQ_HANDLED;
}


/*
 * OPL3 command
 */
static void snd_cs4281_opl3_command(struct snd_opl3 *opl3, unsigned short cmd,
				    unsigned char val)
{
	unsigned long flags;
	struct cs4281 *chip = opl3->private_data;
	void __iomem *port;

	if (cmd & OPL3_RIGHT)
		port = chip->ba0 + BA0_B1AP; /* right port */
	else
		port = chip->ba0 + BA0_B0AP; /* left port */

	spin_lock_irqsave(&opl3->reg_lock, flags);

	writel((unsigned int)cmd, port);
	udelay(10);

	writel((unsigned int)val, port + 4);
	udelay(30);

	spin_unlock_irqrestore(&opl3->reg_lock, flags);
}

static int __devinit snd_cs4281_probe(struct pci_dev *pci,
				      const struct pci_device_id *pci_id)
{
	static int dev;
	struct snd_card *card;
	struct cs4281 *chip;
	struct snd_opl3 *opl3;
	int err;

        if (dev >= SNDRV_CARDS)
                return -ENODEV;
	if (!enable[dev]) {
		dev++;
		return -ENOENT;
	}

	err = snd_card_create(index[dev], id[dev], THIS_MODULE, 0, &card);
	if (err < 0)
		return err;

	if ((err = snd_cs4281_create(card, pci, &chip, dual_codec[dev])) < 0) {
		snd_card_free(card);
		return err;
	}
	card->private_data = chip;

	if ((err = snd_cs4281_mixer(chip)) < 0) {
		snd_card_free(card);
		return err;
	}
	if ((err = snd_cs4281_pcm(chip, 0, NULL)) < 0) {
		snd_card_free(card);
		return err;
	}
	if ((err = snd_cs4281_midi(chip, 0, NULL)) < 0) {
		snd_card_free(card);
		return err;
	}
	if ((err = snd_opl3_new(card, OPL3_HW_OPL3_CS4281, &opl3)) < 0) {
		snd_card_free(card);
		return err;
	}
	opl3->private_data = chip;
	opl3->command = snd_cs4281_opl3_command;
	snd_opl3_init(opl3);
	if ((err = snd_opl3_hwdep_new(opl3, 0, 1, NULL)) < 0) {
		snd_card_free(card);
		return err;
	}
	snd_cs4281_create_gameport(chip);
	strcpy(card->driver, "CS4281");
	strcpy(card->shortname, "Cirrus Logic CS4281");
	sprintf(card->longname, "%s at 0x%lx, irq %d",
		card->shortname,
		chip->ba0_addr,
		chip->irq);

	if ((err = snd_card_register(card)) < 0) {
		snd_card_free(card);
		return err;
	}

	pci_set_drvdata(pci, card);
	dev++;
	return 0;
}

static void __devexit snd_cs4281_remove(struct pci_dev *pci)
{
	snd_card_free(pci_get_drvdata(pci));
	pci_set_drvdata(pci, NULL);
}

/*
 * Power Management
 */
#ifdef CONFIG_PM

static int saved_regs[SUSPEND_REGISTERS] = {
	BA0_JSCTL,
	BA0_GPIOR,
	BA0_SSCR,
	BA0_MIDCR,
	BA0_SRCSA,
	BA0_PASR,
	BA0_CASR,
	BA0_DACSR,
	BA0_ADCSR,
	BA0_FMLVC,
	BA0_FMRVC,
	BA0_PPLVC,
	BA0_PPRVC,
};

#define CLKCR1_CKRA                             0x00010000L

static int cs4281_suspend(struct pci_dev *pci, pm_message_t state)
{
	struct snd_card *card = pci_get_drvdata(pci);
	struct cs4281 *chip = card->private_data;
	u32 ulCLK;
	unsigned int i;

	snd_power_change_state(card, SNDRV_CTL_POWER_D3hot);
	snd_pcm_suspend_all(chip->pcm);

	snd_ac97_suspend(chip->ac97);
	snd_ac97_suspend(chip->ac97_secondary);

	ulCLK = snd_cs4281_peekBA0(chip, BA0_CLKCR1);
	ulCLK |= CLKCR1_CKRA;
	snd_cs4281_pokeBA0(chip, BA0_CLKCR1, ulCLK);

	/* Disable interrupts. */
	snd_cs4281_pokeBA0(chip, BA0_HICR, BA0_HICR_CHGM);

	/* remember the status registers */
	for (i = 0; i < ARRAY_SIZE(saved_regs); i++)
		if (saved_regs[i])
			chip->suspend_regs[i] = snd_cs4281_peekBA0(chip, saved_regs[i]);

	/* Turn off the serial ports. */
	snd_cs4281_pokeBA0(chip, BA0_SERMC, 0);

	/* Power off FM, Joystick, AC link, */
	snd_cs4281_pokeBA0(chip, BA0_SSPM, 0);

	/* DLL off. */
	snd_cs4281_pokeBA0(chip, BA0_CLKCR1, 0);

	/* AC link off. */
	snd_cs4281_pokeBA0(chip, BA0_SPMC, 0);

	ulCLK = snd_cs4281_peekBA0(chip, BA0_CLKCR1);
	ulCLK &= ~CLKCR1_CKRA;
	snd_cs4281_pokeBA0(chip, BA0_CLKCR1, ulCLK);

	pci_disable_device(pci);
	pci_save_state(pci);
	pci_set_power_state(pci, pci_choose_state(pci, state));
	return 0;
}

static int cs4281_resume(struct pci_dev *pci)
{
	struct snd_card *card = pci_get_drvdata(pci);
	struct cs4281 *chip = card->private_data;
	unsigned int i;
	u32 ulCLK;

	pci_set_power_state(pci, PCI_D0);
	pci_restore_state(pci);
	if (pci_enable_device(pci) < 0) {
		printk(KERN_ERR "cs4281: pci_enable_device failed, "
		       "disabling device\n");
		snd_card_disconnect(card);
		return -EIO;
	}
	pci_set_master(pci);

	ulCLK = snd_cs4281_peekBA0(chip, BA0_CLKCR1);
	ulCLK |= CLKCR1_CKRA;
	snd_cs4281_pokeBA0(chip, BA0_CLKCR1, ulCLK);

	snd_cs4281_chip_init(chip);

	/* restore the status registers */
	for (i = 0; i < ARRAY_SIZE(saved_regs); i++)
		if (saved_regs[i])
			snd_cs4281_pokeBA0(chip, saved_regs[i], chip->suspend_regs[i]);

	snd_ac97_resume(chip->ac97);
	snd_ac97_resume(chip->ac97_secondary);

	ulCLK = snd_cs4281_peekBA0(chip, BA0_CLKCR1);
	ulCLK &= ~CLKCR1_CKRA;
	snd_cs4281_pokeBA0(chip, BA0_CLKCR1, ulCLK);

	snd_power_change_state(card, SNDRV_CTL_POWER_D0);
	return 0;
}
#endif /* CONFIG_PM */

static struct pci_driver driver = {
	.name = "CS4281",
	.id_table = snd_cs4281_ids,
	.probe = snd_cs4281_probe,
	.remove = __devexit_p(snd_cs4281_remove),
#ifdef CONFIG_PM
	.suspend = cs4281_suspend,
	.resume = cs4281_resume,
#endif
};
	
static int __init alsa_card_cs4281_init(void)
{
	return pci_register_driver(&driver);
}

static void __exit alsa_card_cs4281_exit(void)
{
	pci_unregister_driver(&driver);
}

module_init(alsa_card_cs4281_init)
module_exit(alsa_card_cs4281_exit)

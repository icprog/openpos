er,
				      sizeof(struct acpi_system_info));
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/*
	 * Populate the return buffer
	 */
	info_ptr = (struct acpi_system_info *)out_buffer->pointer;

	info_ptr->acpi_ca_version = ACPI_CA_VERSION;

	/* System flags (ACPI capabilities) */

	info_ptr->flags = ACPI_SYS_MODE_ACPI;

	/* Timer resolution - 24 or 32 bits  */

	if (acpi_gbl_FADT.flags & ACPI_FADT_32BIT_TIMER) {
		info_ptr->timer_resolution = 24;
	} else {
		info_ptr->timer_resolution = 32;
	}

	/* Clear the reserved fields */

	info_ptr->reserved1 = 0;
	info_ptr->reserved2 = 0;

	/* Current debug levels */

	info_ptr->debug_layer = acpi_dbg_layer;
	info_ptr->debug_level = acpi_dbg_level;

	return_ACPI_STATUS(AE_OK);
}

ACPI_EXPORT_SYMBOL(acpi_get_system_info)

/*****************************************************************************
 *
 * FUNCTION:    acpi_install_initialization_handler
 *
 * PARAMETERS:  Handler             - Callback procedure
 *              Function            - Not (currently) used, see below
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Install an initialization handler
 *
 * TBD: When a second function is added, must save the Function also.
 *
 ****************************************************************************/
acpi_status
acpi_install_initialization_handler(acpi_init_handler handler, u32 function)
{

	if (!handler) {
		return (AE_BAD_PARAMETER);
	}

	if (acpi_gbl_init_handler) {
		return (AE_ALREADY_EXISTS);
	}

	acpi_gbl_init_handler = handler;
	return AE_OK;
}

ACPI_EXPORT_SYMBOL(acpi_install_initialization_handler)
#endif				/*  ACPI_FUTURE_USAGE  */
/*****************************************************************************
 *
 * FUNCTION:    acpi_purge_cached_objects
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Empty all caches (delete the cached objects)
 *
 ****************************************************************************/
acpi_status acpi_purge_cached_objects(void)
{
	ACPI_FUNCTION_TRACE(acpi_purge_cached_objects);

	(void)acpi_os_purge_cache(acpi_gbl_state_cache);
	(void)acpi_os_purge_cache(acpi_gbl_operand_cache);
	(void)acpi_os_purge_cache(acpi_gbl_ps_node_cache);
	(void)acpi_os_purge_cache(acpi_gbl_ps_node_ext_cache);
	return_ACPI_STATUS(AE_OK);
}

ACPI_EXPORT_SYMBOL(acpi_purge_cached_objects)
#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    /*
 * PS3 FLASH ROM Storage Driver
 *
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * Copyright 2007 Sony Corp.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#include <asm/lv1call.h>
#include <asm/ps3stor.h>


#define DEVICE_NAME		"ps3flash"

#define FLASH_BLOCK_SIZE	(256*1024)


struct ps3flash_private {
	struct mutex mutex;	/* Bounce buffer mutex */
	u64 chunk_sectors;
	int tag;		/* Start sector of buffer, -1 if invalid */
	bool dirty;
};

static struct ps3_storage_device *ps3flash_dev;

static int ps3flash_read_write_sectors(struct ps3_storage_device *dev,
				       u64 start_sector, int write)
{
	struct ps3flash_private *priv = ps3_system_bus_get_drvdata(&dev->sbd);
	u64 res = ps3stor_read_write_sectors(dev, dev->bounce_lpar,
					     start_sector, priv->chunk_sectors,
					     write);
	if (res) {
		dev_err(&dev->sbd.core, "%s:%u: %s failed 0x%llx\n", __func__,
			__LINE__, write ? "write" : "read", res);
		return -EIO;
	}
	return 0;
}

static int ps3flash_writeback(struct ps3_storage_device *dev)
{
	struct ps3flash_private *priv = ps3_system_bus_get_drvdata(&dev->sbd);
	int res;

	if (!priv->dirty || priv->tag < 0)
		return 0;

	res = ps3flash_read_write_sectors(dev, priv->tag, 1);
	if (res)
		return res;

	priv->dirty = false;
	return 0;
}

static int ps3flash_fetch(struct ps3_storage_device *dev, u64 start_sector)
{
	struct ps3flash_private *priv = ps3_system_bus_get_drvdata(&dev->sbd);
	int res;

	if (start_sector == priv->tag)
		return 0;

	res = ps3flash_writeback(dev);
	if (res)
		return res;

	priv->tag = -1;

	res = ps3flash_read_write_sectors(dev, start_sector, 0);
	if (res)
		return res;

	priv->tag = start_sector;
	return 0;
}

static loff_t ps3flash_llseek(struct file *file, loff_t offset, int origin)
{
	struct ps3_storage_device *dev = ps3flash_dev;
	loff_t res;

	mutex_lock(&file->f_mapping->host->i_mutex);
	switch (origin) {
	case 1:
		offset += file->f_pos;
		break;
	case 2:
		offset += dev->regions[dev->region_idx].size*dev->blk_size;
		break;
	}
	if (offset < 0) {
		res = -EINVAL;
		goto out;
	}

	file->f_pos = offset;
	res = file->f_pos;

out:
	mutex_unlock(&file->f_mapping->host->i_mutex);
	return res;
}

static ssize_t ps3flash_read(char __user *userbuf, void *kernelbuf,
			     size_t count, loff_t *pos)
{
	struct ps3_storage_device *dev = ps3flash_dev;
	struct ps3flash_private *priv = ps3_system_bus_get_drvdata(&dev->sbd);
	u64 size, sector, offset;
	int res;
	size_t remaining, n;
	const void *src;

	dev_dbg(&dev->sbd.core,
		"%s:%u: Reading %zu bytes at position %lld to U0x%p/K0x%p\n",
		__func__, __LINE__, count, *pos, userbuf, kernelbuf);

	size = dev->regions[dev->region_idx].size*dev->blk_size;
	if (*pos >= size || !count)
		return 0;

	if (*pos + count > size) {
		dev_dbg(&dev->sbd.core,
			"%s:%u Truncating count from %zu to %llu\n", __func__,
			__LINE__, count, size - *pos);
		count = size - *pos;
	}

	sector = *pos / dev->bounce_size * priv->chunk_sectors;
	offset = *pos % dev->bounce_size;

	remaining = count;
	do {
		n = min_t(u64, remaining, dev->bounce_size - offset);
		src = dev->bounce_buf + offset;

		mutex_lock(&priv->mutex);

		res = ps3flash_fetch(dev, sector);
		if (res)
			goto fail;

		dev_dbg(&dev->sbd.core,
			"%s:%u: copy %lu bytes from 0x%p to U0x%p/K0x%p\n",
			__func__, __LINE__, n, src, userbuf, kernelbuf);
		if (userbuf) {
			if (copy_to_user(userbuf, src, n)) {
				res = -EFAULT;
				goto fail;
			}
			userbuf += n;
		}
		if (kernelbuf) {
			memcpy(kernelbuf, src, n);
			kernelbuf += n;
		}

		mutex_unlock(&priv->mutex);

		*pos += n;
		remaining -= n;
		sector += priv->chunk_sectors;
		offset = 0;
	} while (remaining > 0);

	return count;

fail:
	mutex_unlock(&priv->mutex);
	return res;
}

static ssize_t ps3flash_write(const char __user *userbuf,
			      const void *kernelbuf, size_t count, loff_t *pos)
{
	struct ps3_storage_device *dev = ps3flash_dev;
	struct ps3flash_private *priv = ps3_system_bus_get_drvdata(&dev->sbd);
	u64 size, sector, offset;
	int res = 0;
	size_t remaining, n;
	void *dst;

	dev_dbg(&dev->sbd.core,
		"%s:%u: Writing %zu bytes at position %lld from U0x%p/K0x%p\n",
		__func__, __LINE__, count, *pos, userbuf, kernelbuf);

	size = dev->regions[dev->region_idx].size*dev->blk_size;
	if (*pos >= size || !count)
		return 0;

	if (*pos + count > size) {
		dev_dbg(&dev->sbd.core,
			"%s:%u Truncating count from %zu to %llu\n", __func__,
			__LINE__, count, size - *pos);
		count = size - *pos;
	}

	sector = *pos / dev->bounce_size * priv->chunk_sectors;
	offset = *pos % dev->bounce_size;

	remaining = count;
	do {
		n = min_t(u64, remaining, dev->bounce_size - offset);
		dst = dev->bounce_buf + offset;

		mutex_lock(&priv->mutex);

		if (n != dev->bounce_size)
			res = ps3flash_fetch(dev, sector);
		else if (sector != priv->tag)
			res = ps3flash_writeback(dev);
		if (res)
			goto fail;

		dev_dbg(&dev->sbd.core,
			"%s:%u: copy %lu bytes from U0x%p/K0x%p to 0x%p\n",
			__func__, __LINE__, n, userbuf, kernelbuf, dst);
		if (userbuf) {
			if (copy_from_user(dst, userbuf, n)) {
				res = -EFAULT;
				goto fail;
			}
			userbuf += n;
		}
		if (kernelbuf) {
			memcpy(dst, kernelbuf, n);
			kernelbuf += n;
		}

		priv->tag = sector;
		priv->dirty = true;

		mutex_unlock(&priv->mutex);

		*pos += n;
		remaining -= n;
		sector += priv->chunk_sectors;
		offset = 0;
	} while (remaining > 0);

	return count;

fail:
	mutex_unlock(&priv->mutex);
	return res;
}

static ssize_t ps3flash_user_read(struct file *file, char __user *buf,
				  size_t count, loff_t *pos)
{
	return ps3flash_read(buf, NULL, count, pos);
}

static ssize_t ps3flash_user_write(struct file *file, const char __user *buf,
				   size_t count, loff_t *pos)
{
	return ps3flash_write(buf, NULL, count, pos);
}

static ssize_t ps3flash_kernel_read(void *buf, size_t count, loff_t pos)
{
	return ps3flash_read(NULL, buf, count, &pos);
}

static ssize_t ps3flash_kernel_write(const void *buf, size_t count,
				     loff_t pos)
{
	ssize_t res;
	int wb;

	res = ps3flash_write(NULL, buf, count, &pos);
	if (res < 0)
		return res;

	/* Make kernel writes synchronous */
	wb = ps3flash_writeback(ps3flash_dev);
	if (wb)
		return wb;

	return res;
}

static int ps3flash_flush(struct file *file, fl_owner_t id)
{
	return ps3flash_writeback(ps3flash_dev);
}

static int ps3flash_fsync(struct file *file, struct dentry *dentry,
			  int datasync)
{
	return ps3flash_writeback(ps3flash_dev);
}

static irqreturn_t ps3flash_interrupt(int irq, void *data)
{
	struct ps3_storage_device *dev = data;
	int res;
	u64 tag, status;

	res = lv1_storage_get_async_status(dev->sbd.dev_id, &tag, &status);

	if (tag != dev->tag)
		dev_err(&dev->sbd.core,
			"%s:%u: tag mismatch, got %llx, expected %llx\n",
			__func__, __LINE__, tag, dev->tag);

	if (res) {
		dev_err(&dev->sbd.core, "%s:%u: res=%d status=0x%llx\n",
			__func__, __LINE__, res, status);
	} else {
		dev->lv1_status = status;
		complete(&dev->done);
	}
	return IRQ_HANDLED;
}

static const struct file_operations ps3flash_fops = {
	.owner	= THIS_MODULE,
	.llseek	= ps3flash_llseek,
	.read	= ps3flash_user_read,
	.write	= ps3flash_user_write,
	.flush	= ps3flash_flush,
	.fsync	= ps3flash_fsync,
};

static const struct ps3_os_area_flash_ops ps3flash_kernel_ops = {
	.read	= ps3flash_kernel_read,
	.write	= ps3flash_kernel_write,
};

static struct miscdevice ps3flash_misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEVICE_NAME,
	.fops	= &ps3flash_fops,
};

static int __devinit ps3flash_probe(struct ps3_system_bus_device *_dev)
{
	struct ps3_storage_device *dev = to_ps3_storage_device(&_dev->core);
	struct ps3flash_private *priv;
	int error;
	unsigned long tmp;

	tmp = dev->regions[dev->region_idx].start*dev->blk_size;
	if (tmp % FLASH_BLOCK_SIZE) {
		dev_err(&dev->sbd.core,
			"%s:%u region start %lu is not aligned\n", __func__,
			__LINE__, tmp);
		return -EINVAL;
	}
	tmp = dev->regions[dev->region_idx].size*dev->blk_size;
	if (tmp % FLASH_BLOCK_SIZE) {
		dev_err(&dev->sbd.core,
			"%s:%u region size %lu is not aligned\n", __func__,
			__LINE__, tmp);
		return -EINVAL;
	}

	/* use static buffer, kmalloc cannot allocate 256 KiB */
	if (!ps3flash_bounce_buffer.address)
		return -ENODEV;

	if (ps3flash_dev) {
		dev_err(&dev->sbd.core,
			"Only one FLASH device is supported\n");
		return -EBUSY;
	}

	ps3flash_dev = dev;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		error = -ENOMEM;
		goto fail;
	}

	ps3_system_bus_set_drvdata(&dev->sbd, priv);
	mutex_init(&priv->mutex);
	priv->tag = -1;

	dev->bounce_size = ps3flash_bounce_buffer.size;
	dev->bounce_buf = ps3flash_bounce_buffer.address;
	priv->chunk_sectors = dev->bounce_size / dev->blk_size;

	error = ps3stor_setup(dev, ps3flash_interrupt);
	if (error)
		goto fail_free_priv;

	ps3flash_misc.parent = &dev->sbd.core;
	error = misc_register(&ps3flash_misc);
	if (error) {
		dev_err(&dev->sbd.core, "%s:%u: misc_register failed %d\n",
			__func__, __LINE__, error);
		goto fail_teardown;
	}

	dev_info(&dev->sbd.core, "%s:%u: registered misc device %d\n",
		 __func__, __LINE__, ps3flash_misc.minor);

	ps3_os_area_flash_register(&ps3flash_kernel_ops);
	return 0;

fail_teardown:
	ps3stor_teardown(dev);
fail_free_priv:
	kfree(priv);
	ps3_system_bus_set_drvdata(&dev->sbd, NULL);
fail:
	ps3flash_dev = NULL;
	return error;
}

static int ps3flash_remove(struct ps3_system_bus_device *_dev)
{
	struct ps3_storage_device *dev = to_ps3_storage_device(&_dev->core);

	ps3_os_area_flash_register(NULL);
	misc_deregister(&ps3flash_misc);
	ps3stor_teardown(dev);
	kfree(ps3_system_bus_get_drvdata(&dev->sbd));
	ps3_system_bus_set_drvdata(&dev->sbd, NULL);
	ps3flash_dev = NULL;
	return 0;
}


static struct ps3_system_bus_driver ps3flash = {
	.match_id	= PS3_MATCH_ID_STOR_FLASH,
	.core.name	= DEVICE_NAME,
	.core.owner	= THIS_MODULE,
	.probe		= ps3flash_probe,
	.remove		= ps3flash_remove,
	.shutdown	= ps3flash_remove,
};


static int __init ps3flash_init(void)
{
	return ps3_system_bus_driver_register(&ps3flash);
}

static void __exit ps3flash_exit(void)
{
	ps3_system_bus_driver_unregister(&ps3flash);
}

module_init(ps3flash_init);
module_exit(ps3flash_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PS3 FLASH ROM Storage Driver");
MODULE_AUTHOR("Sony Corporation");
MODULE_ALIAS(PS3_MODULE_ALIAS_STOR_FLASH);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          /*
 * Copyright (c) 2006, 2007 QLogic Corporation. All rights reserved.
 * Copyright (c) 2005, 2006 PathScale, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/rculist.h>

#include "ipath_verbs.h"

/*
 * Global table of GID to attached QPs.
 * The table is global to all ipath devices since a send from one QP/device
 * needs to be locally routed to any locally attached QPs on the same
 * or different device.
 */
static struct rb_root mcast_tree;
static DEFINE_SPINLOCK(mcast_lock);

/**
 * ipath_mcast_qp_alloc - alloc a struct to link a QP to mcast GID struct
 * @qp: the QP to link
 */
static struct ipath_mcast_qp *ipath_mcast_qp_alloc(struct ipath_qp *qp)
{
	struct ipath_mcast_qp *mqp;

	mqp = kmalloc(sizeof *mqp, GFP_KERNEL);
	if (!mqp)
		goto bail;

	mqp->qp = qp;
	atomic_inc(&qp->refcount);

bail:
	return mqp;
}

static void ipath_mcast_qp_free(struct ipath_mcast_qp *mqp)
{
	struct ipath_qp *qp = mqp->qp;

	/* Notify ipath_destroy_qp() if it is waiting. */
	if (atomic_dec_and_test(&qp->refcount))
		wake_up(&qp->wait);

	kfree(mqp);
}

/**
 * ipath_mcast_alloc - allocate the multicast GID structure
 * @mgid: the multicast GID
 *
 * A list of QPs will be attached to this structure.
 */
static struct ipath_mcast *ipath_mcast_alloc(union ib_gid *mgid)
{
	struct ipath_mcast *mcast;

	mcast = kmalloc(sizeof *mcast, GFP_KERNEL);
	if (!mcast)
		goto bail;

	mcast->mgid = *mgid;
	INIT_LIST_HEAD(&mcast->qp_list);
	init_waitqueue_head(&mcast->wait);
	atomic_set(&mcast->refcount, 0);
	mcast->n_attached = 0;

bail:
	return mcast;
}

static void ipath_mcast_free(struct ipath_mcast *mcast)
{
	struct ipath_mcast_qp *p, *tmp;

	list_for_each_entry_safe(p, tmp, &mcast->qp_list, list)
		ipath_mcast_qp_free(p);

	kfree(mcast);
}

/**
 * ipath_mcast_find - search the global table for the given multicast GID
 * @mgid: the multicast GID to search for
 *
 * Returns NULL if not found.
 *
 * The caller is responsible for decrementing the reference count if found.
 */
struct ipath_mcast *ipath_mcast_find(union ib_gid *mgid)
{
	struct rb_node *n;
	unsigned long flags;
	struct ipath_mcast *mcast;

	spin_lock_irqsave(&mcast_lock, flags);
	n = mcast_tree.rb_node;
	while (n) {
		int ret;

		mcast = rb_entry(n, struct ipath_mcast, rb_node);

		ret = memcmp(mgid->raw, mcast->mgid.raw,
			     sizeof(union ib_gid));
		if (ret < 0)
			n = n->rb_left;
		else if (ret > 0)
			n = n->rb_right;
		else {
			atomic_inc(&mcast->refcount);
			spin_unlock_irqrestore(&mcast_lock, flags);
			goto bail;
		}
	}
	spin_unlock_irqrestore(&mcast_lock, flags);

	mcast = NULL;

bail:
	return mcast;
}

/**
 * ipath_mcast_add - insert mcast GID into table and attach QP struct
 * @mcast: the mcast GID table
 * @mqp: the QP to attach
 *
 * Return zero if both were added.  Return EEXIST if the GID was already in
 * the table but the QP was added.  Return ESRCH if the QP was already
 * attached and neither structure was added.
 */
static int ipath_mcast_add(struct ipath_ibdev *dev,
			   struct ipath_mcast *mcast,
			   struct ipath_mcast_qp *mqp)
{
	struct rb_node **n = &mcast_tree.rb_node;
	struct rb_node *pn = NULL;
	int ret;

	spin_lock_irq(&mcast_lock);

	while (*n) {
		struct ipath_mcast *tmcast;
		struct ipath_mcast_qp *p;

		pn = *n;
		tmcast = rb_entry(pn, struct ipath_mcast, rb_node);

		ret = memcmp(mcast->mgid.raw, tmcast->mgid.raw,
			     sizeof(union ib_gid));
		if (ret < 0) {
			n = &pn->rb_left;
			continue;
		}
		if (ret > 0) {
			n = &pn->rb_right;
			continue;
		}

		/* Search the QP list to see if this is already there. */
		list_for_each_entry_rcu(p, &tmcast->qp_list, list) {
			if (p->qp == mqp->qp) {
				ret = ESRCH;
				goto bail;
			}
		}
		if (tmcast->n_attached == ib_ipath_max_mcast_qp_attached) {
			ret = ENOMEM;
			goto bail;
		}

		tmcast->n_attached++;

		list_add_tail_rcu(&mqp->list, &tmcast->qp_list);
		ret = EEXIST;
		goto bail;
	}

	spin_lock(&dev->n_mcast_grps_lock);
	if (dev->n_mcast_grps_allocated == ib_ipath_max_mcast_grps) {
		spin_unlock(&dev->n_mcast_grps_lock);
		ret = ENOMEM;
		goto bail;
	}

	dev->n_mcast_grps_allocated++;
	spin_unlock(&dev->n_mcast_grps_lock);

	mcast->n_attached++;

	list_add_tail_rcu(&mqp->list, &mcast->qp_list);

	atomic_inc(&mcast->refcount);
	rb_link_node(&mcast->rb_node, pn, n);
	rb_insert_color(&mcast->rb_node, &mcast_tree);

	ret = 0;

bail:
	spin_unlock_irq(&mcast_lock);

	return ret;
}

int ipath_multicast_attach(struct ib_qp *ibqp, union ib_gid *gid, u16 lid)
{
	struct ipath_qp *qp = to_iqp(ibqp);
	struct ipath_ibdev *dev = to_idev(ibqp->device);
	struct ipath_mcast *mcast;
	struct ipath_mcast_qp *mqp;
	int ret;

	/*
	 * Allocate data structures since its better to do this outside of
	 * spin locks and it will most likely be needed.
	 */
	mcast = ipath_mcast_alloc(gid);
	if (mcast == NULL) {
		ret = -ENOMEM;
		goto bail;
	}
	mqp = ipath_mcast_qp_alloc(qp);
	if (mqp == NULL) {
		ipath_mcast_free(mcast);
		ret = -ENOMEM;
		goto bail;
	}
	switch (ipath_mcast_add(dev, mcast, mqp)) {
	case ESRCH:
		/* Neither was used: can't attach the same QP twice. */
		ipath_mcast_qp_free(mqp);
		ipath_mcast_free(mcast);
		ret = -EINVAL;
		goto bail;
	case EEXIST:		/* The mcast wasn't used */
		ipath_mcast_free(mcast);
		break;
	case ENOMEM:
		/* Exceeded the maximum number of mcast groups. */
		ipath_mcast_qp_free(mqp);
		ipath_mcast_free(mcast);
		ret = -ENOMEM;
		goto bail;
	default:
		break;
	}

	ret = 0;

bail:
	return ret;
}

int ipath_multicast_detach(struct ib_qp *ibqp, union ib_gid *gid, u16 lid)
{
	struct ipath_qp *qp = to_iqp(ibqp);
	struct ipath_ibdev *dev = to_idev(ibqp->device);
	struct ipath_mcast *mcast = NULL;
	struct ipath_mcast_qp *p, *tmp;
	struct rb_node *n;
	int last = 0;
	int ret;

	spin_lock_irq(&mcast_lock);

	/* Find the GID in the mcast table. */
	n = mcast_tree.rb_node;
	while (1) {
		if (n == NULL) {
			spin_unlock_irq(&mcast_lock);
			ret = -EINVAL;
			goto bail;
		}

		mcast = rb_entry(n, struct ipath_mcast, rb_node);
		ret = memcmp(gid->raw, mcast->mgid.raw,
			     sizeof(union ib_gid));
		if (ret < 0)
			n = n->rb_left;
		else if (ret > 0)
			n = n->rb_right;
		else
			break;
	}

	/* Search the QP list. */
	list_for_each_entry_safe(p, tmp, &mcast->qp_list, list) {
		if (p->qp != qp)
			continue;
		/*
		 * We found it, so remove it, but don't poison the forward
		 * link until we are sure there are no list walkers.
		 */
		list_del_rcu(&p->list);
		mcast->n_attached--;

		/* If this was the last attached QP, remove the GID too. */
		if (list_empty(&mcast->qp_list)) {
			rb_erase(&mcast->rb_node, &mcast_tree);
			last = 1;
		}
		break;
	}

	spin_unlock_irq(&mcast_lock);

	if (p) {
		/*
		 * Wait for any list walkers to finish before freeing the
		 * list element.
		 */
		wait_event(mcast->wait, atomic_read(&mcast->refcount) <= 1);
		ipath_mcast_qp_free(p);
	}
	if (last) {
		atomic_dec(&mcast->refcount);
		wait_event(mcast->wait, !atomic_read(&mcast->refcount));
		ipath_mcast_free(mcast);
		spin_lock_irq(&dev->n_mcast_grps_lock);
		dev->n_mcast_grps_allocated--;
		spin_unlock_irq(&dev->n_mcast_grps_lock);
	}

	ret = 0;

bail:
	return ret;
}

int ipath_mcast_tree_empty(void)
{
	return mcast_tree.rb_node == NULL;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          PSList.entity[i].bUsed )
            {
                wd->sta.staPSList.entity[i].bUsed = TRUE;
                wd->sta.staPSList.entity[i].bDataQueued = FALSE;
                break;
            }
        }

        if ( i == ZM_MAX_PS_STA )
        {
            /* STA list is full */
            return;
        }

        zfCopyFromRxBuffer(dev, buf, wd->sta.staPSList.entity[i].macAddr,
                           ZM_WLAN_HEADER_A2_OFFSET, 6);

        if ( wd->sta.staPSList.count == 0 )
        {
            // enable ATIM window
            //zfEnableAtimWindow(dev);
        }

        wd->sta.staPSList.count++;
    }
    else if ( wd->sta.staPSList.count )
    {
        for(i=1; i<ZM_MAX_PS_STA; i++)
        {
            if ( wd->sta.staPSList.entity[i].bUsed )
            {
                if ( zfRxBufferEqualToStr(dev, buf,
                                          wd->sta.staPSList.entity[i].macAddr,
                                          ZM_WLAN_HEADER_A2_OFFSET, 6) )
                {
                    wd->sta.staPSList.entity[i].bUsed = FALSE;
                    wd->sta.staPSList.count--;

                    if ( wd->sta.staPSList.entity[i].bDataQueued )
                    {
                        /* send queued data */
                    }
                }
            }
        }

        if ( wd->sta.staPSList.count == 0 )
        {
            /* disable ATIM window */
            //zfDisableAtimWindow(dev);
        }

    }
}

/* IBSS power-saving mode */
u8_t zfStaIbssPSQueueData(zdev_t* dev, zbuf_t* buf)
{
    u8_t   i;
    u16_t  da[3];

    zmw_get_wlan_dev(dev);

    if ( !zfStaIsConnected(dev) )
    {
        return 0;
    }

    if ( wd->wlanMode != ZM_MODE_IBSS )
    {
        return 0;
    }

    if ( wd->sta.staPSList.count == 0 && wd->sta.powerSaveMode <= ZM_STA_PS_NONE )
    {
        return 0;
    }

    /* DA */
#ifdef ZM_ENABLE_NATIVE_WIFI
    da[0] = zmw_tx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET);
    da[1] = zmw_tx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET + 2);
    da[2] = zmw_tx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET + 4);
#else
    da[0] = zmw_tx_buf_readh(dev, buf, 0);
    da[1] = zmw_tx_buf_readh(dev, buf, 2);
    da[2] = zmw_tx_buf_readh(dev, buf, 4);
#endif

    if ( ZM_IS_MULTICAST_OR_BROADCAST(da) )
    {
        wd->sta.staPSList.entity[0].bDataQueued = TRUE;
        wd->sta.ibssPSDataQueue[wd->sta.ibssPSDataCount++] = buf;
        return 1;
    }

    // Unicast packet...

    for(i=1; i<ZM_MAX_PS_STA; i++)
    {
        if ( zfMemoryIsEqual(wd->sta.staPSList.entity[i].macAddr,
                             (u8_t*) da, 6) )
        {
            wd->sta.staPSList.entity[i].bDataQueued = TRUE;
            wd->sta.ibssPSDataQueue[wd->sta.ibssPSDataCount++] = buf;

            return 1;
        }
    }

#if 0
    if ( wd->sta.powerSaveMode > ZM_STA_PS_NONE )
    {
        wd->sta.staPSDataQueue[wd->sta.staPSDataCount++] = buf;

        return 1;
    }
#endif

    return 0;
}

/* IBSS power-saving mode */
void zfStaIbssPSSend(zdev_t* dev)
{
    u8_t   i;
    u16_t  bcastAddr[3] = {0xffff, 0xffff, 0xffff};

    zmw_get_wlan_dev(dev);

    if ( !zfStaIsConnected(dev) )
    {
        return ;
    }

    if ( wd->wlanMode != ZM_MODE_IBSS )
    {
        return ;
    }

    for(i=0; i<ZM_MAX_PS_STA; i++)
    {
        if ( wd->sta.staPSList.entity[i].bDataQueued )
        {
            if ( i == 0 )
            {
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_ATIM,
                              bcastAddr,
                              0, 0, 0);
            }
            else if ( wd->sta.staPSList.entity[i].bUsed )
            {
                // Send ATIM to prevent the peer to go to sleep
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_ATIM,
                              (u16_t*) wd->sta.staPSList.entity[i].macAddr,
                              0, 0, 0);
            }

            wd->sta.staPSList.entity[i].bDataQueued = FALSE;
        }
    }

    for(i=0; i<wd->sta.ibssPSDataCount; i++)
    {
        zfTxSendEth(dev, wd->sta.ibssPSDataQueue[68CF_MAX_DEVICES-1] =
					 W9968CF_DECOMPRESSION};
static int upscaling[]= {[0 ... W9968CF_MAX_DEVICES-1] = W9968CF_UPSCALING};
static unsigned short force_palette[] = {[0 ... W9968CF_MAX_DEVICES-1] = 0};
static int force_rgb[] = {[0 ... W9968CF_MAX_DEVICES-1] = W9968CF_FORCE_RGB};
static int autobright[] = {[0 ... W9968CF_MAX_DEVICES-1] = W9968CF_AUTOBRIGHT};
static int autoexp[] = {[0 ... W9968CF_MAX_DEVICES-1] = W9968CF_AUTOEXP};
static unsigned short lightfreq[] = {[0 ... W9968CF_MAX_DEVICES-1] =
				     W9968CF_LIGHTFREQ};
static int bandingfilter[] = {[0 ... W9968CF_MAX_DEVICES-1]=
			      W9968CF_BANDINGFILTER};
static short clockdiv[] = {[0 ... W9968CF_MAX_DEVICES-1] = W9968CF_CLOCKDIV};
static int backlight[] = {[0 ... W9968CF_MAX_DEVICES-1] = W9968CF_BACKLIGHT};
static int mirror[] = {[0 ... W9968CF_MAX_DEVICES-1] = W9968CF_MIRROR};
static int monochrome[] = {[0 ... W9968CF_MAX_DEVICES-1]=W9968CF_MONOCHROME};
static unsigned int brightness[] = {[0 ... W9968CF_MAX_DEVICES-1] =
				    W9968CF_BRIGHTNESS};
static unsigned int hue[] = {[0 ... W9968CF_MAX_DEVICES-1] = W9968CF_HUE};
static unsigned int colour[]={[0 ... W9968CF_MAX_DEVICES-1] = W9968CF_COLOUR};
static unsigned int contrast[] = {[0 ... W9968CF_MAX_DEVICES-1] =
				  W9968CF_CONTRAST};
static unsigned int whiteness[] = {[0 ... W9968CF_MAX_DEVICES-1] =
				   W9968CF_WHITENESS};
#ifdef W9968CF_DEBUG
static unsigned short debug = W9968CF_DEBUG_LEVEL;
static int specific_debug = W9968CF_SPECIFIC_DEBUG;
#endif

static unsigned int param_nv[24]; /* number of values per parameter */

module_param(simcams, ushort, 0644);
module_param_array(video_nr, short, &param_nv[0], 0444);
module_param_array(packet_size, uint, &param_nv[1], 0444);
module_param_array(max_buffers, ushort, &param_nv[2], 0444);
module_param_array(double_buffer, bool, &param_nv[3], 0444);
module_param_array(clamping, bool, &param_nv[4], 0444);
module_param_array(filter_type, ushort, &param_nv[5], 0444);
module_param_array(largeview, bool, &param_nv[6], 0444);
module_param_array(decompression, ushort, &param_nv[7], 0444);
module_param_array(upscaling, bool, &param_nv[8], 0444);
module_param_array(force_palette, ushort, &param_nv[9], 0444);
module_param_array(force_rgb, ushort, &param_nv[10], 0444);
module_param_array(autobright, bool, &param_nv[11], 0444);
module_param_array(autoexp, bool, &param_nv[12], 0444);
module_param_array(lightfreq, ushort, &param_nv[13], 0444);
module_param_array(bandingfilter, bool, &param_nv[14], 0444);
module_param_array(clockdiv, short, &param_nv[15], 0444);
module_param_array(backlight, bool, &param_nv[16], 0444);
module_param_array(mirror, bool, &param_nv[17], 0444);
module_param_array(monochrome, bool, &param_nv[18], 0444);
module_param_array(brightness, uint, &param_nv[19], 0444);
module_param_array(hue, uint, &param_nv[20], 0444);
module_param_array(colour, uint, &param_nv[21], 0444);
module_param_array(contrast, uint, &param_nv[22], 0444);
module_param_array(whiteness, uint, &param_nv[23], 0444);
#ifdef W9968CF_DEBUG
module_param(debug, ushort, 0644);
module_param(specific_debug, bool, 0644);
#endif

MODULE_PARM_DESC(simcams,
		 "\n<n> Number of cameras allowed to stream simultaneously."
		 "\nn may vary from 0 to "
		 __MODULE_STRING(W9968CF_MAX_DEVICES)"."
		 "\nDefault value is "__MODULE_STRING(W9968CF_SIMCAMS)"."
		 "\n");
MODULE_PARM_DESC(video_nr,
		 "\n<-1|n[,...]> Specify V4L minor mode number."
		 "\n -1 = use next available (default)"
		 "\n  n = use minor number n (integer >= 0)"
		 "\nYou can specify up to "__MODULE_STRING(W9968CF_MAX_DEVICES)
		 " cameras this way."
		 "\nFor example:"
		 "\nvideo_nr=-1,2,-1 would assign minor number 2 to"
		 "\nthe second camera and use auto for the first"
		 "\none and for every other camera."
		 "\n");
MODULE_PARM_DESC(packet_size,
		 "\n<n[,...]> Specify the maximum data payload"
		 "\nsize in bytes for alternate settings, for each device."
		 "\nn is scaled between 63 and 1023 "
		 "(default is "__MODULE_STRING(W9968CF_PACKET_SIZE)")."
		 "\n");
MODULE_PARM_DESC(max_buffers,
		 "\n<n[,...]> For advanced ud->tick & 1 )
        {
            zfTimerCheckAndHandle(dev);
        }

        zfStaCheckRxBeacon(dev);
        zfStaTimer100ms(dev);
        zfStaCheckConnectTimeout(dev);
        zfPowerSavingMgrMain(dev);
    }

#ifdef ZM_ENABLE_AGGREGATION
    /*
     * add by honda
     */
    zfAggScanAndClear(dev, wd->tick);
#endif
}

void zfStaSendBeacon(zdev_t* dev)
{
    zbuf_t* buf;
    u16_t offset, seq;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    //zm_debug_msg0("\n");

    /* TBD : Maximum size of beacon */
    if ((buf = zfwBufAllocate(dev, 1024)) == NULL)
    {
        zm_debug_msg0("Allocate beacon buffer failed");
        return;
    }

    offset = 0;
    /* wlan header */
    /* Frame control */
    zmw_tx_buf_writeh(dev, buf, offset, 0x0080);
    offset+=2;
    /* Duration */
    zmw_tx_buf_writeh(dev, buf, offset, 0x0000);
    offset+=2;
    /* Address 1 */
    zmw_tx_buf_writeh(dev, buf, offset, 0xffff);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, 0xffff);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, 0xffff);
    offset+=2;
    /* Address 2 */
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[0]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[1]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[2]);
    offset+=2;
    /* Address 3 */
    zmw_tx_buf_writeh(dev, buf, offset, wd->sta.bssid[0]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->sta.bssid[1]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->sta.bssid[2]);
    offset+=2;

    /* Sequence number */
    zmw_enter_critical_section(dev);
    seq = ((wd->mmseq++)<<4);
    zmw_leave_critical_section(dev);
    zmw_tx_buf_writeh(dev, buf, offset, seq);
    offset+=2;

    /* 24-31 Time Stamp : hardware will fill this field */
    offset+=8;

    /* Beacon Interval */
    zmw_tx_buf_writeh(dev, buf, offset, wd->beaconInterval);
    offset+=2;

    /* Capability */
    zmw_tx_buf_writeb(dev, buf, offset++, wd->sta.capability[0]);
    zmw_tx_buf_writeb(dev, buf, offset++, wd->sta.capability[1]);

    /* SSID */
    offset = zfStaAddIeSsid(dev, buf, offset);

    if(wd->frequency <= ZM_CH_G_14)  // 2.4 GHz  b+g
    {

    	/* Support Rate */
    	offset = zfMmAddIeSupportRate(dev, buf, offset,
                                  		ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_CCK);

    	/* DS parameter set */
    	offset = zfMmAddIeDs(dev, buf, offset);

    	offset = zfStaAddIeIbss(dev, buf, offset);

        if( wd->wfc.bIbssGMode
            && (wd->supportMode & (ZM_WIRELESS_MODE_24_54|ZM_WIRELESS_MODE_24_N)) )    // Only accompany with enabling a mode .
        {
      	    /* ERP Information */
       	    wd->erpElement = 0;
       	    offset = zfMmAddIeErp(dev, buf, offset);
       	}

       	/* TODO : country information */
        /* RSN */
        if ( wd->sta.authMode == ZM_AUTH_MODE_WPA2PSK )
        {
            offset = zfwStaAddIeWpaRsn(dev, buf, offset, ZM_WLAN_FRAME_TYPE_AUTH);
        }

        if( wd->wfc.bIbssGMode
            && (wd->supportMode & (ZM_WIRELESS_MODE_24_54|ZM_WIRELESS_MODE_24_N)) )    // Only accompany with enabling a mode .
        {
            /* Enable G Mode */
            /* Extended Supported Rates */
       	    offset = zfMmAddIeSupportRate(dev, buf, offset,
                                   		    ZM_WLAN_EID_EXTENDED_RATE, ZM_RATE_SET_OFDM);
	    }
    }
    else    // 5GHz a
    {
        /* Support Rate a Mode */
    	offset = zfMmAddIeSupportRate(dev, buf, offset,
        	                            ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_OFDM);

        /* DS parameter set */
    	offset = zfMmAddIeDs(dev, buf, offset);

    	offset = zfStaAddIeIbss(dev, buf, offset);

        /* TODO : country information */
        /* RSN */
        if ( wd->sta.authMode == ZM_AUTH_MODE_WPA2PSK )
        {
            offset = zfwStaAddIeWpaRsn(dev, buf, offset, ZM_WLAN_FRAME_TYPE_AUTH);
        }
    }

    if ( wd->wlanMode != ZM_MODE_IBSS )
    {
        /* TODO : Need to check if it is ok */
        /* HT Capabilitieordering."
		 "\nThis parameter has effect when using RGBX palettes only."
		 "\nDefault value is "__MODULE_STRING(W9968CF_FORCE_RGB)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(autobright,
		 "\n<0|1[,...]> Image sensor automatically changes brightness:"
		 "\n 0 = no, 1 = yes"
		 "\nDefault value is "__MODULE_STRING(W9968CF_AUTOBRIGHT)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(autoexp,
		 "\n<0|1[,...]> Image sensor automatically changes exposure:"
		 "\n 0 = no, 1 = yes"
		 "\nDefault value is "__MODULE_STRING(W9968CF_AUTOEXP)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(lightfreq,
		 "\n<50|60[,...]> Light frequency in Hz:"
		 "\n 50 for European and Asian lighting,"
		 " 60 for American lighting."
		 "\nDefault value is "__MODULE_STRING(W9968CF_LIGHTFREQ)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(bandingfilter,
		 "\n<0|1[,...]> Banding filter to reduce effects of"
		 " fluorescent lighting:"
		 "\n 0 disabled, 1 enabled."
		 "\nThis filter tries to reduce the pattern of horizontal"
		 "\nlight/dark bands caused by some (usually fluorescent)"
		 " lighting."
		 "\nDefault value is "__MODULE_STRING(W9968CF_BANDINGFILTER)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(clockdiv,
		 "\n<-1|n[,...]> "
		 "Force pixel clock divisor to a specific value (for experts):"
		 "\n  n may vary from 0 to 127."
		 "\n -1 for automatic value."
		 "\nSee also the 'double_buffer' module parameter."
		 "\nDefault value is "__MODULE_STRING(W9968CF_CLOCKDIV)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(backlight,
		 "\n<0|1[,...]> Objects are lit from behind:"
		 "\n 0 = no, 1 = yes"
		 "\nDefault value is "__MODULE_STRING(W9968CF_BACKLIGHT)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(mirror,
		 "\n<0|1[,...]> Reverse image horizontally:"
		 "\n 0 = no, 1 = yes"
		 "\nDefault value is "__MODULE_STRING(W9968CF_MIRROR)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(monochrome,
		 "\n<0|1[,...]> Use image sensor as monochrome sensor:"
		 "\n 0 = no, 1 = yes"
		 "\nNot all the sensors support monochrome color."
		 "\nDefault value is "__MODULE_STRING(W9968CF_MONOCHROME)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(brightness,
		 "\n<n[,...]> Set picture brightness (0-65535)."
		 "\nDefault value is "__MODULE_STRING(W9968CF_BRIGHTNESS)
		 " for every device."
		 "\nThis parameter has no effect if 'autobright' is enabled."
		 "\n");
MODULE_PARM_DESC(hue,
		 "\n<n[,...]> Set picture hue (0-65535)."
		 "\nDefault value is "__MODULE_STRING(W9968CF_HUE)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(colour,
		 "\n<n[,...]> Set picture saturation (0-65535)."
		 "\nDefault value is "__MODULE_STRING(W9968CF_COLOUR)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(contrast,
		 "\n<n[,...]> Set picture contrast (0-65535)."
		 "\nDefault value is "__MODULE_STRING(W9968CF_CONTRAST)
		 " for every device."
		 "\n");
MODULE_PARM_DESC(whiteness,
		 "\n<n[,...]> Set picture whiteness (0-65535)."
		 "\nDefault value is "__MODULE_STRING(W9968CF_WHITENESS)
		 " for every device."
		 "\n");
#ifdef W9968CF_DEBUG
MODULE_PARM_DESC(debug,
		 "\n<n> Debugging information level, from 0 to 6:"
		 "\n0 = none (use carefully)"
		 "\n1 = critical errors"
		 "\n2 = significant informations"
		 "\n3 = configuration or general messages"
		 "\n4 = warnings"
		 "\n5 = called functions"
		 "\n6 = function internals"
		 "\nLevel 5 and 6 are useful for testing only, when only "
		 "one device is used."
		 "\nDefault value is "__MODULE_STRING(W9968CF_DEBUG_LEVEL)"."
		 "\n");
MODULE_PARM_DESC(specific_debug,
		 "\n<0|1> Enable or disable specific debugging messages:"
		 "\n0 = print messages concerning every level"
		 " <= 'debug' level."
		 "\n1 = print messages concerning the level"
		 " indicated by 'debug'."
		 "\nDefault value is "
		 __MODULE_STRING(W9968CF_SPECIFIC_DEBUG)"."
		 "\n");
#endif /* W9968CF_DEBUG */



/****************************************************************************
 * Some prototypes                                                          *
 ****************************************************************************/

/* Video4linux interface */
static const struct v4l2_file_operations w9968cf_fops;
static int w9968cf_open(struct file *);
static int w9968cf_release(struct file *);
static int w9968cf_mmap(struct file *, struct vm_area_struct *);
static long w9968cf_ioctl(struct file *, unsigned, unsigned long);
static ssize_t w9968cf_read(struct file *, char __user *, size_t, loff_t *);
static long w9968cf_v4l_ioctl(struct file *, unsigned int,
			     void __user *);

/* USB-specific */
static int w9968cf_start_transfer(struct w9968cf_device*);
static int w9968cf_stop_transfer(struct w9968cf_device*);
static int w9968cf_write_reg(struct w9968cf_device*, u16 value, u16 index);
static int w9968cf_read_reg(struct w9968cf_device*, u16 index);
static int w9968cf_write_fsb(struct w9968cf_device*, u16* data);
static int w9968cf_write_sb(struct w9968cf_device*, u16 value);
static int w9968cf_read_sb(struct w9968cf_device*);
static int w9968cf_upload_quantizationtables(struct w9968cf_device*);
static void w9968cf_urb_complete(struct urb *urb);

/* Low-level I2C (SMBus) I/O */
static int w9968cf_smbus_start(struct w9968cf_device*);
static int w9968cf_smbus_stop(struct w9968cf_device*);
static int w9968cf_smbus_write_byte(struct w9968cf_device*, u8 v);
static int w9968cf_smbus_read_byte(struct w9968cf_device*, u8* v);
static int w9968cf_smbus_write_ack(struct w9968cf_device*);
static int w9968cf_smbus_read_ack(struct w9968cf_device*);
static int w9968cf_smbus_refresh_bus(struct w9968cf_device*);
static int w9968cf_i2c_adap_read_byte(struct w9968cf_device* cam,
				      u16 address, u8* value);
static int w9968cf_i2c_adap_read_byte_data(struct w9968cf_device*, u16 address,
					   u8 subaddress, u8* value);
static int w9968cf_i2c_adap_write_byte(struct w9968cf_device*,
				       u16 address, u8 subaddress);
static int w9968cf_i2c_adap_fastwrite_byte_data(struct w9968cf_device*,
						u16 address, u8 subaddress,
						u8 value);

/* I2C interface to kernel */
static int w9968cf_i2c_init(struct w9968cf_device*);
static int w9968cf_i2c_smbus_xfer(struct i2c_adapter*, u16 addr,
				  unsigned short flags, char read_write,
				  u8 command, int size, union i2c_smbus_data*);
static u32 w9968cf_i2c_func(struct i2c_adapter*);

/* Memory management */
static void* rvmalloc(unsigned long size);
static void rvfree(void *mem, unsigned long size);
static void w9968cf_deallocate_memory(struct w9968cf_device*);
static int  w9968cf_allocate_memory(struct w9968cf_device*);

/* High-level image sensor control functions */
static int w9968cf_sensor_set_control(struct w9968cf_device*,int cid,int val);
static int w9968cf_sensor_get_control(struct w9968cf_device*,int cid,int *val);
static int w9968cf_sensor_cmd(struct w9968cf_device*,
			      unsigned int cmd, void *arg);
static int w9968cf_sensor_init(struct w9968cf_device*);
static int w9968cf_sensor_update_settings(struct w9968cf_device*);
static int w9968cf_sensor_get_picture(struct w9968cf_device*);
static int w9968cf_sensor_update_picture(struct w9968cf_device*,
					 struct video_picture pict);

/* Other helper functions */
static void w9968cf_configure_camera(struct w9968cf_device*,struct usb_device*,
				     enum w9968cf_model_id,
				     const unsigned short dev_nr);
static void w9968cf_adjust_configuration(struct w9968cf_device*);
static int w9968cf_turn_on_led(struct w9968cf_device*);
static int w9968cf_init_chip(struct w9968cf_device*);
static inline u16 w9968cf_valid_palette(u16 palette);
static inline u16 w9968cf_valid_depth(u16 palette);
static inline u8 w9968cf_need_decompression(u16 palette);
static int w9968cf_set_picture(struct w9968cf_device*, struct video_picture);
static int w9968cf_set_window(struct w9968cf_device*, struct video_window);
static int w9968cf_postprocess_frame(struct w9968cf_device*,
				     struct w9968cf_frame_t*);
static int w9968cf_adjust_window_size(struct w9968cf_device*, u32 *w, u32 *h);
static void w9968cf_init_framelist(struct w9968cf_device*);
static void w9968cf_push_frame(struct w9968cf_device*, u8 f_num);
static void w9968cf_pop_frame(struct w9968cf_device*,struct w9968cf_frame_t**);
static void w9968cf_release_resources(struct w9968cf_device*);



/****************************************************************************
 * Symbolic names                                                           *
 ****************************************************************************/

/* Used to represent a list of values and their respective symbolic names */
struct w9968cf_symbolic_list {
	const int num;
	const char *name;
};

/*--------------------------------------------------------------------------
  Returns the name of the matching element in the symbolic_list array. The
  end of the list must be marked with an element that has a NULL name.
  --------------------------------------------------------------------------*/
static inline const char *
symbolic(struct w9968cf_symbolic_list list[], const int num)
{
	int i;

	for (i = 0; list[i].name != NULL; i++)
		if (list[i].num == num)
			return (list[i].name);

	return "Unknown";
}

static struct w9968cf_symbolic_list camlist[] = {
	{ W9968CF_MOD_GENERIC, "W996[87]CF JPEG USB Dual Mode Camera" },
	{ W9968CF_MOD_CLVBWGP, "Creative Labs Video Blaster WebCam Go Plus" },

	/* Other cameras (having the same descriptors as Generic W996[87]CF) */
	{ W9968CF_MOD_ADPVDMA, "Aroma Digi Pen VGA Dual Mode ADG-5000" },
	{ W9986CF_MOD_AAU, "AVerMedia AVerTV USB" },
	{ W9968CF_MOD_CLVBWG, "Creative Labs Video Blaster WebCam Go" },
	{ W9968CF_MOD_LL, "Lebon LDC-035A" },
	{ W9968CF_MOD_EEEMC, "Ezonics EZ-802 EZMega Cam" },
	{ W9968CF_MOD_OOE, "OmniVision OV8610-EDE" },
	{ W9968CF_MOD_ODPVDMPC, "OPCOM Digi Pen VGA Dual Mode Pen Camera" },
	{ W9968CF_MOD_PDPII, "Pretec Digi Pen-II" },
	{ W9968CF_MOD_PDP480, "Pretec DigiPen-480" },

	{  -1, NULL }
};

static struct w9968cf_symbolic_list senlist[] = {
	{ CC_OV76BE,   "OV76BE" },
	{ CC_OV7610,   "OV7610" },
	{ CC_OV7620,   "OV7620" },
	{ CC_OV7620AE, "OV7620AE" },
	{ CC_OV6620,   "OV6620" },
	{ CC_OV6630,   "OV6630" },
	{ CC_OV6630AE, "OV6630AE" },
	{ CC_OV6630AF, "OV6630AF" },
	{ -1, NULL }
};

/* Video4Linux1 palettes */
static struct w9968cf_symbolic_list v4l1_plist[] = {
	{ VIDEO_PALETTE_GREY,    "GREY" },
	{ VIDEO_PALETTE_HI240,   "HI240" },
	{ VIDEO_PALETTE_RGB565,  "RGB565" },
	{ VIDEO_PALETTE_RGB24,   "RGB24" },
	{ VIDEO_PALETTE_RGB32,   "RGB32" },
	{ VIDEO_PALETTE_RGB555,  "RGB555" },
	{ VIDEO_PALETTE_YUV422,  "YUV422" },
	{ VIDEO_PALETTE_YUYV,    "YUYV" },
	{ VIDEO_PALETTE_UYVY,    "UYVY" },
	{ VIDEO_PALETTE_YUV420,  "YUV420" },
	{ VIDEO_PALETTE_YUV411,  "YUV411" },
	{ VIDEO_PALETTE_RAW,     "RAW" },
	{ VIDEO_PALETTE_YUV422P, "YUV422P" },
	{ VIDEO_PALETTE_YUV411P, "YUV411P" },
	{ VIDEO_PALETTE_YUV420P, "YUV420P" },
	{ VIDEO_PALETTE_YUV410P, "YUV410P" },
	{ -1, NULL }
};

/* Decoder error codes: */
static struct w9968cf_symbolic_list decoder_errlist[] = {
	{ W9968CF_DEC_ERR_CORRUPTED_DATA, "Corrupted data" },
	{ W9968CF_DEC_ERR_BUF_OVERFLOW,   "Buffer overflow" },
	{ W9968CF_DEC_ERR_NO_SOI,         "SOI marker not found" },
	{ W9968CF_DEC_ERR_NO_SOF0,        "SOF0 marker not found" },
	{ W9968CF_DEC_ERR_NO_SOS,         "SOS marker not found" },
	{ W9968CF_DEC_ERR_NO_EOI,         "EOI marker not found" },
	{ -1, NULL }
};

/* URB error codes: */
static struct w9968cf_symbolic_list urb_errlist[] = {
	{ -ENOMEM,    "No memory for allocation of internal structures" },
	{ -ENOSPC,    "The host controller's bandwidth is already consumed" },
	{ -ENOENT,    "URB was canceled by unlink_urb" },
	{ -EXDEV,     "ISO transfer only partially completed" },
	{ -EAGAIN,    "Too match scheduled for the future" },
	{ -ENXIO,     "URB already queued" },
	{ -EFBIG,     "Too much ISO frames requested" },
	{ -ENOSR,     "Buffer error (overrun)" },
	{ -EPIPE,     "Specified endpoint is stalled (device not responding)"},
	{ -EOVERFLOW, "Babble (too much data)" },
	{ -EPROTO,    "Bit-stuff error (bad cable?)" },
	{ -EILSEQ,    "CRC/Timeout" },
	{ -ETIME,     "Device does not respond to token" },
	{ -ETIMEDOUT, "Device does not respond to command" },
	{ -1, NULL }
};

/****************************************************************************
 * Memory management functions                                              *
 ****************************************************************************/
static void* rvmalloc(unsigned long size)
{
	void* mem;
	unsigned long adr;

	size = PAGE_ALIGN(size);
	mem = vmalloc_32(size);
	if (!mem)
		return NULL;

	memset(mem, 0, size); /* Clear the ram out, no junk to the user */
	adr = (unsigned long) mem;
	while (size > 0) {
		SetPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	return mem;
}


static void rvfree(void* mem, unsigned long size)
{
	unsigned long adr;

	if (!mem)
		return;

	adr = (unsigned long) mem;
	while ((long) size > 0) {
		ClearPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
}


/*--------------------------------------------------------------------------
  Deallocate previously allocated memory.
  --------------------------------------------------------------------------*/
static void w9968cf_deallocate_memory(struct w9968cf_device* cam)
{
	u8 i;

	/* Free the isochronous transfer buffers */
	for (i = 0; i < W9968CF_URBS; i++) {
		kfree(cam->transfer_buffer[i]);
		cam->transfer_buffer[i] = NULL;
	}

	/* Free temporary frame buffer */
	if (cam->frame_tmp.buffer) {
		rvfree(cam->frame_tmp.buffer, cam->frame_tmp.size);
		cam->frame_tmp.buffer = NULL;
	}

	/* Free helper buffer */
	if (cam->frame_vpp.buffer) {
		rvfree(cam->frame_vpp.buffer, cam->frame_vpp.size);
		cam->frame_vpp.buffer = NULL;
	}

	/* Free video frame buffers */
	if (cam->frame[0].buffer) {
		rvfree(cam->frame[0].buffer, cam->nbuffers*cam->frame[0].size);
		cam->frame[0].buffer = NULL;
	}

	cam->nbuffers = 0;

	DBG(5, "Memory successfully deallocated")
}


/*--------------------------------------------------------------------------
  Allocate memory buffers for USB transfers and video frames.
  This function is called by open() only.
  Return 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_allocate_memory(struct w9968cf_device* cam)
{
	const u16 p_size = wMaxPacketSize[cam->altsetting-1];
	void* buff = NULL;
	unsigned long hw_bufsize, vpp_bufsize;
	u8 i, bpp;

	/* NOTE: Deallocation is done elsewhere in case of error */

	/* Calculate the max amount of raw data per frame from the device */
	hw_bufsize = cam->maxwidth*cam->maxheight*2;

	/* Calculate the max buf. size needed for post-processing routines */
	bpp = (w9968cf_vpp) ? 4 : 2;
	if (cam->upscaling)
		vpp_bufsize = max(W9968CF_MAX_WIDTH*W9968CF_MAX_HEIGHT*bpp,
				  cam->maxwidth*cam->maxheight*bpp);
	else
		vpp_bufsize = cam->maxwidth*cam->maxheight*bpp;

	/* Allocate memory for the isochronous transfer buffers */
	for (i = 0; i < W9968CF_URBS; i++) {
		if (!(cam->transfer_buffer[i] =
		      kzalloc(W9968CF_ISO_PACKETS*p_size, GFP_KERNEL))) {
			DBG(1, "Couldn't allocate memory for the isochronous "
			       "transfer buffers (%u bytes)",
			    p_size * W9968CF_ISO_PACKETS)
			return -ENOMEM;
		}
	}

	/* Allocate memory for the temporary frame buffer */
	if (!(cam->frame_tmp.buffer = rvmalloc(hw_bufsize))) {
		DBG(1, "Couldn't allocate memory for the temporary "
		       "video frame buffer (%lu bytes)", hw_bufsize)
		return -ENOMEM;
	}
	cam->frame_tmp.size = hw_bufsize;
	cam->frame_tmp.number = -1;

	/* Allocate memory for the helper buffer */
	if (w9968cf_vpp) {
		if (!(cam->frame_vpp.buffer = rvmalloc(vpp_bufsize))) {
			DBG(1, "Couldn't allocate memory for the helper buffer"
			       " (%lu bytes)", vpp_bufsize)
			return -ENOMEM;
		}
		cam->frame_vpp.size = vpp_bufsize;
	} else
		cam->frame_vpp.buffer = NULL;

	/* Allocate memory for video frame buffers */
	cam->nbuffers = cam->max_buffers;
	while (cam->nbuffers >= 2) {
		if ((buff = rvmalloc(cam->nbuffers * vpp_bufsize)))
			break;
		else
			cam->nbuffers--;
	}

	if (!buff) {
		DBG(1, "Couldn't allocate memory for the video frame buffers")
		cam->nbuffers = 0;
		return -ENOMEM;
	}

	if (cam->nbuffers != cam->max_buffers)
		DBG(2, "Couldn't allocate memory for %u video frame buffers. "
		       "Only memory for %u buffers has been allocated",
		    cam->max_buffers, cam->nbuffers)

	for (i = 0; i < cam->nbuffers; i++) {
		cam->frame[i].buffer = buff + i*vpp_bufsize;
		cam->frame[i].size = vpp_bufsize;
		cam->frame[i].number = i;
		/* Circular list */
		if (i != cam->nbuffers-1)
			cam->frame[i].next = &cam->frame[i+1];
		else
			cam->frame[i].next = &cam->frame[0];
		cam->frame[i].status = F_UNUSED;
	}

	DBG(5, "Memory successfully allocated")
	return 0;
}



/****************************************************************************
 * USB-specific functions                                                   *
 ****************************************************************************/

/*--------------------------------------------------------------------------
  This is an handler function which is called after the URBs are completed.
  It collects multiple data packets coming from the camera by putting them
  into frame buffers: one or more zero data length data packets are used to
  mark the end of a video frame; the first non-zero data packet is the start
  of the next video frame; if an error is encountered in a packet, the entire
  video frame is discarded and grabbed again.
  If there are no requested frames in the FIFO list, packets are collected into
  a temporary buffer.
  --------------------------------------------------------------------------*/
static void w9968cf_urb_complete(struct urb *urb)
{
	struct w9968cf_device* cam = (struct w9968cf_device*)urb->context;
	struct w9968cf_frame_t** f;
	unsigned int len, status;
	void* pos;
	u8 i;
	int err = 0;

	if ((!cam->streaming) || cam->disconnected) {
		DBG(4, "Got interrupt, but not streaming")
		return;
	}

	/* "(*f)" will be used instead of "cam->frame_current" */
	f = &cam->frame_current;

	/* If a frame has been requested and we are grabbing into
	   the temporary frame, we'll switch to that requested frame */
	if ((*f) == &cam->frame_tmp && *cam->requested_frame) {
		if (cam->frame_tmp.status == F_GRABBING) {
			w9968cf_pop_frame(cam, &cam->frame_current);
			(*f)->status = F_GRABBING;
			(*f)->length = cam->frame_tmp.length;
			memcpy((*f)->buffer, cam->frame_tmp.buffer,
			       (*f)->length);
			DBG(6, "Switched from temp. frame to frame #%d",
			    (*f)->number)
		}
	}

	for (i = 0; i < urb->number_of_packets; i++) {
		len = urb->iso_frame_desc[i].actual_length;
		status = urb->iso_frame_desc[i].status;
		pos = urb->iso_frame_desc[i].offset + urb->transfer_buffer;

		if (status && len != 0) {
			DBG(4, "URB failed, error in data packet "
			       "(error #%u, %s)",
			    status, symbolic(urb_errlist, status))
			(*f)->status = F_ERROR;
			continue;
		}

		if (len) { /* start of frame */

			if ((*f)->status == F_UNUSED) {
				(*f)->status = F_GRABBING;
				(*f)->length = 0;
			}

			/* Buffer overflows shouldn't happen, however...*/
			if ((*f)->length + len > (*f)->size) {
				DBG(4, "Buffer overflow: bad data packets")
				(*f)->status = F_ERROR;
			}

			if ((*f)->status == F_GRABBING) {
				memcpy((*f)->buffer + (*f)->length, pos, len);
				(*f)->length += len;
			}

		} else if ((*f)->status == F_GRABBING) { /* end of frame */

			DBG(6, "Frame #%d successfully grabbed", (*f)->number)

			if (cam->vpp_flag & VPP_DECOMPRESSION) {
				err = w9968cf_vpp->check_headers((*f)->buffer,
								 (*f)->length);
				if (err) {
					DBG(4, "Skip corrupted frame: %s",
					    symbolic(decoder_errlist, err))
					(*f)->status = F_UNUSED;
					continue; /* grab this frame again */
				}
			}

			(*f)->status = F_READY;
			(*f)->queued = 0;

			/* Take a pointer to the new frame from the FIFO list.
			   If the list is empty,we'll use the temporary frame*/
			if (*cam->requested_frame)
				w9968cf_pop_frame(cam, &cam->frame_current);
			else {
				cam->frame_current = &cam->frame_tmp;
				(*f)->status = F_UNUSED;
			}

		} else if ((*f)->status == F_ERROR)
			(*f)->status = F_UNUSED; /* grab it again */

		PDBGG("Frame length %lu | pack.#%u | pack.len. %u | state %d",
		      (unsigned long)(*f)->length, i, len, (*f)->status)

	} /* end for */

	/* Resubmit this URB */
	urb->dev = cam->usbdev;
	urb->status = 0;
	spin_lock(&cam->urb_lock);
	if (cam->streaming)
		if ((err = usb_submit_urb(urb, GFP_ATOMIC))) {
			cam->misconfigured = 1;
			DBG(1, "Couldn't resubmit the URB: error %d, %s",
			    err, symbolic(urb_errlist, err))
		}
	spin_unlock(&cam->urb_lock);

	/* Wake up the user process */
	wake_up_interruptible(&cam->wait_queue);
}


/*---------------------------------------------------------------------------
  Setup the URB structures for the isochronous transfer.
  Submit the URBs so that the data transfer begins.
  Return 0 on success, a negative number otherwise.
  ---------------------------------------------------------------------------*/
static int w9968cf_start_transfer(struct w9968cf_device* cam)
{
	struct usb_device *udev = cam->usbdev;
	struct urb* urb;
	const u16 p_size = wMaxPacketSize[cam->altsetting-1];
	u16 w, h, d;
	int vidcapt;
	u32 t_size;
	int err = 0;
	s8 i, j;

	for (i = 0; i < W9968CF_URBS; i++) {
		urb = usb_alloc_urb(W9968CF_ISO_PACKETS, GFP_KERNEL);
		if (!urb) {
			for (j = 0; j < i; j++)
				usb_free_urb(cam->urb[j]);
			DBG(1, "Couldn't allocate the URB structures")
			return -ENOMEM;
		}

		cam->urb[i] = urb;
		urb->dev = udev;
		urb->context = (void*)cam;
		urb->pipe = usb_rcvisocpipe(udev, 1);
		urb->transfer_flags = URB_ISO_ASAP;
		urb->number_of_packets = W9968CF_ISO_PACKETS;
		urb->complete = w9968cf_urb_complete;
		urb->transfer_buffer = cam->transfer_buffer[i];
		urb->transfer_buffer_length = p_size*W9968CF_ISO_PACKETS;
		urb->interval = 1;
		for (j = 0; j < W9968CF_ISO_PACKETS; j++) {
			urb->iso_frame_desc[j].offset = p_size*j;
			urb->iso_frame_desc[j].length = p_size;
		}
	}

	/* Transfer size per frame, in WORD ! */
	d = cam->hw_depth;
	w = cam->hw_width;
	h = cam->hw_height;

	t_size = (w*h*d)/16;

	err = w9968cf_write_reg(cam, 0xbf17, 0x00); /* reset everything */
	err += w9968cf_write_reg(cam, 0xbf10, 0x00); /* normal operation */

	/* Transfer size */
	err += w9968cf_write_reg(cam, t_size & 0xffff, 0x3d); /* low bits */
	err += w9968cf_write_reg(cam, t_size >> 16, 0x3e);    /* high bits */

	if (cam->vpp_flag & VPP_DECOMPRESSION)
		err += w9968cf_upload_quantizationtables(cam);

	vidcapt = w9968cf_read_reg(cam, 0x16); /* read picture settings */
	err += w9968cf_write_reg(cam, vidcapt|0x8000, 0x16); /* capt. enable */

	err += usb_set_interface(udev, 0, cam->altsetting);
	err += w9968cf_write_reg(cam, 0x8a05, 0x3c); /* USB FIFO enable */

	if (err || (vidcapt < 0)) {
		for (i = 0; i < W9968CF_URBS; i++)
			usb_free_urb(cam->urb[i]);
		DBG(1, "Couldn't tell the camera to start the data transfer")
		return err;
	}

	w9968cf_init_framelist(cam);

	/* Begin to grab into the temporary buffer */
	cam->frame_tmp.status = F_UNUSED;
	cam->frame_tmp.queued = 0;
	cam->frame_current = &cam->frame_tmp;

	if (!(cam->vpp_flag & VPP_DECOMPRESSION))
		DBG(5, "Isochronous transfer size: %lu bytes/frame",
		    (unsigned long)t_size*2)

	DBG(5, "Starting the isochronous transfer...")

	cam->streaming = 1;

	/* Submit the URBs */
	for (i = 0; i < W9968CF_URBS; i++) {
		err = usb_submit_urb(cam->urb[i], GFP_KERNEL);
		if (err) {
			cam->streaming = 0;
			for (j = i-1; j >= 0; j--) {
				usb_kill_urb(cam->urb[j]);
				usb_free_urb(cam->urb[j]);
			}
			DBG(1, "Couldn't send a transfer request to the "
			       "USB core (error #%d, %s)", err,
			    symbolic(urb_errlist, err))
			return err;
		}
	}

	return 0;
}


/*--------------------------------------------------------------------------
  Stop the isochronous transfer and set alternate setting to 0 (0Mb/s).
  Return 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_stop_transfer(struct w9968cf_device* cam)
{
	struct usb_device *udev = cam->usbdev;
	unsigned long lock_flags;
	int err = 0;
	s8 i;

	if (!cam->streaming)
		return 0;

	/* This avoids race conditions with usb_submit_urb()
	   in the URB completition handler */
	spin_lock_irqsave(&cam->urb_lock, lock_flags);
	cam->streaming = 0;
	spin_unlock_irqrestore(&cam->urb_lock, lock_flags);

	for (i = W9968CF_URBS-1; i >= 0; i--)
		if (cam->urb[i]) {
			usb_kill_urb(cam->urb[i]);
			usb_free_urb(cam->urb[i]);
			cam->urb[i] = NULL;
		}

	if (cam->disconnected)
		goto exit;

	err = w9968cf_write_reg(cam, 0x0a05, 0x3c); /* stop USB transfer */
	err += usb_set_interface(udev, 0, 0); /* 0 Mb/s */
	err += w9968cf_write_reg(cam, 0x0000, 0x39); /* disable JPEG encoder */
	err += w9968cf_write_reg(cam, 0x0000, 0x16); /* stop video capture */

	if (err) {
		DBG(2, "Failed to tell the camera to stop the isochronous "
		       "transfer. However this is not a critical error.")
		return -EIO;
	}

exit:
	DBG(5, "Isochronous transfer stopped")
	return 0;
}


/*--------------------------------------------------------------------------
  Write a W9968CF register.
  Return 0 on success, -1 otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_write_reg(struct w9968cf_device* cam, u16 value, u16 index)
{
	struct usb_device* udev = cam->usbdev;
	int res;

	res = usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0,
			      USB_TYPE_VENDOR | USB_DIR_OUT | USB_RECIP_DEVICE,
			      value, index, NULL, 0, W9968CF_USB_CTRL_TIMEOUT);

	if (res < 0)
		DBG(4, "Failed to write a register "
		       "(value 0x%04X, index 0x%02X, error #%d, %s)",
		    value, index, res, symbolic(urb_errlist, res))

	return (res >= 0) ? 0 : -1;
}


/*--------------------------------------------------------------------------
  Read a W9968CF register.
  Return the register value on success, -1 otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_read_reg(struct w9968cf_device* cam, u16 index)
{
	struct usb_device* udev = cam->usbdev;
	u16* buff = cam->control_buffer;
	int res;

	res = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0), 1,
			      USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      0, index, buff, 2, W9968CF_USB_CTRL_TIMEOUT);

	if (res < 0)
		DBG(4, "Failed to read a register "
		       "(index 0x%02X, error #%d, %s)",
		    index, res, symbolic(urb_errlist, res))

	return (res >= 0) ? (int)(*buff) : -1;
}


/*--------------------------------------------------------------------------
  Write 64-bit data to the fast serial bus registers.
  Return 0 on success, -1 otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_write_fsb(struct w9968cf_device* cam, u16* data)
{
	struct usb_device* udev = cam->usbdev;
	u16 value;
	int res;

	value = *data++;

	res = usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0,
			      USB_TYPE_VENDOR | USB_DIR_OUT | USB_RECIP_DEVICE,
			      value, 0x06, data, 6, W9968CF_USB_CTRL_TIMEOUT);

	if (res < 0)
		DBG(4, "Failed to write the FSB registers "
		       "(error #%d, %s)", res, symbolic(urb_errlist, res))

	return (res >= 0) ? 0 : -1;
}


/*--------------------------------------------------------------------------
  Write data to the serial bus control register.
  Return 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_write_sb(struct w9968cf_device* cam, u16 value)
{
	int err = 0;

	err = w9968cf_write_reg(cam, value, 0x01);
	udelay(W9968CF_I2C_BUS_DELAY);

	return err;
}


/*--------------------------------------------------------------------------
  Read data from the serial bus control register.
  Return 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_read_sb(struct w9968cf_device* cam)
{
	int v = 0;

	v = w9968cf_read_reg(cam, 0x01);
	udelay(W9968CF_I2C_BUS_DELAY);

	return v;
}


/*--------------------------------------------------------------------------
  Upload quantization tables for the JPEG compression.
  This function is called by w9968cf_start_transfer().
  Return 0 on successe.                          */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : auth frame buffer                                         */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      none                                                            */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        ZyDAS Technology Corporation    2005.10     */
/*                                                                      */
/************************************************************************/
/* Note : AP allows one authenticating STA at a time, does not          */
/*        support multiple authentication process. Make sure            */
/*        authentication state machine will not be blocked due          */
/*        to incompleted authentication handshake.                      */
void zfStaProcessAuth(zdev_t* dev, zbuf_t* buf, u16_t* src, u16_t apId)
{
    struct zsWlanAuthFrameHeader* pAuthFrame;
    u8_t  pBuf[sizeof(struct zsWlanAuthFrameHeader)];
    u32_t p1, p2;

    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    if ( !zfStaIsConnecting(dev) )
    {
        return;
    }

    pAuthFrame = (struct zsWlanAuthFrameHeader*) pBuf;
    zfCopyFromRxBuffer(dev, buf, pBuf, 0, sizeof(struct zsWlanAuthFrameHeader));

    if ( wd->sta.connectState == ZM_STA_CONN_STATE_AUTH_OPEN )
    {
        if ( (zmw_le16_to_cpu(pAuthFrame->seq) == 2)&&
             (zmw_le16_to_cpu(pAuthFrame->algo) == 0)&&
             (zmw_le16_to_cpu(pAuthFrame->status) == 0) )
        {

            zmw_enter_critical_section(dev);
            wd->sta.connectTimer = wd->tick;
            zm_debug_msg0("ZM_STA_CONN_STATE_AUTH_COMPLETED");
            wd->sta.connectState = ZM_STA_CONN_STATE_AUTH_COMPLETED;
            zmw_leave_critical_section(dev);

            //Set channel according to AP's configuration
            //Move to here because of Cisco 11n AP feature
            zfCoreSetFrequencyEx(dev, wd->frequency, wd->BandWidth40,
                    wd->ExtOffset, zfAuthFreqCompleteCb);

            /* send association frame */
            if ( wd->sta.connectByReasso )
            {
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_REASOCREQ,
                              wd->sta.bssid, 0, 0, 0);
            }
            else
            {
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_ASOCREQ,
                              wd->sta.bssid, 0, 0, 0);
            }


        }
        else
        {
            zm_debug_msg1("authentication failed, status = ",
                          pAuthFrame->status);

            if (wd->sta.authMode == ZM_AUTH_MODE_AUTO)
            {
                wd->sta.bIsSharedKey = 1;
                zfStaStartConnect(dev, wd->sta.bIsSharedKey);
            }
            else
            {
                zm_debug_msg0("ZM_STA_STATE_DISCONNECT");
                zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_AUTH_FAILED, wd->sta.bssid, 3);
            }
        }
    }
    else if ( wd->sta.connectState == ZM_STA_CONN_STATE_AUTH_SHARE_1 )
    {
        if ( (zmw_le16_to_cpu(pAuthFrame->algo) == 1) &&
             (zmw_le16_to_cpu(pAuthFrame->seq) == 2) &&
             (zmw_le16_to_cpu(pAuthFrame->status) == 0))
              //&& (pAuthFrame->challengeText[1] <= 255) )
        {
            zfMemoryCopy(wd->sta.challengeText, pAuthFrame->challengeText,
                         pAuthFrame->challengeText[1]+2);

            /* send the 3rd authentication frame */
            p1 = 0x30001;
            p2 = 0;
            zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_AUTH,
                          wd->sta.bssid, p1, p2, 0);

            zmw_enter_critical_section(20 | ((address & 0x02) ? 0x0001 : 0x0);
	data[3] |= (address & 0x01) ? 0x0054 : 0x0;

	err += w9968cf_write_fsb(cam, data);

	data[0] = 0x8208 | ((subaddress & 0x80) ? 0x0015 : 0x0);
	data[0] |= (subaddress & 0x40) ? 0x0540 : 0x0;
	data[0] |= (subaddress & 0x20) ? 0x5000 : 0x0;
	data[1] = 0x0820 | ((subaddress & 0x20) ? 0x0001 : 0x0);
	data[1] |= (subaddress & 0x10) ? 0x0054 : 0x0;
	data[1] |= (subaddress & 0x08) ? 0x1500 : 0x0;
	data[1] |= (subaddress & 0x04) ? 0x4000 : 0x0;
	data[2] = 0x2082 | ((subaddress & 0x04) ? 0x0005 : 0x0);
	data[2] |= (subaddress & 0x02) ? 0x0150 : 0x0;
	data[2] |= (subaddress & 0x01) ? 0x5400 : 0x0;
	data[3] = 0x001d;

	err += w9968cf_write_fsb(cam, data);

	data[0] = 0x8208 | ((value & 0x80) ? 0x0015 : 0x0);
	data[0] |= (value & 0x40) ? 0x0540 : 0x0;
	data[0] |= (value & 0x20) ? 0x5000 : 0x0;
	data[1] = 0x0820 | ((value & 0x20) ? 0x0001 : 0x0);
	data[1] |= (value & 0x10) ? 0x0054 : 0x0;
	data[1] |= (value & 0x08) ? 0x1500 : 0x0;
	data[1] |= (value & 0x04) ? 0x4000 : 0x0;
	data[2] = 0x2082 | ((value & 0x04) ? 0x0005 : 0x0);
	data[2] |= (value & 0x02) ? 0x0150 : 0x0;
	data[2] |= (value & 0x01) ? 0x5400 : 0x0;
	data[3] = 0xfe1d;

	err += w9968cf_write_fsb(cam, data);

	/* Disable SBUS outputs */
	err += w9968cf_write_sb(cam, 0x0000);

	if (!err)
		DBG(5, "I2C write byte data done, addr.0x%04X, subaddr.0x%02X "
		       "value 0x%02X", address, subaddress, value)
	else
		DBG(5, "I2C write byte data failed, addr.0x%04X, "
		       "subaddr.0x%02X, value 0x%02X",
		    address, subaddress, value)

	return err;
}


/* SMBus protocol: S Addr Wr [A] Subaddr [A] P S Addr+1 Rd [A] [Value] NA P */
static int
w9968cf_i2c_adap_read_byte_data(struct w9968cf_device* cam,
				u16 address, u8 subaddress,
				u8* value)
{
	int err = 0;

	/* Serial data enable */
	err += w9968cf_write_sb(cam, 0x0013); /* don't change ! */

	err += w9968cf_smbus_start(cam);
	err += w9968cf_smbus_write_byte(cam, address);
	err += w9968cf_smbus_read_ack(cam);
	err += w9968cf_smbus_write_byte(cam, subaddress);
	err += w9968cf_smbus_read_ack(cam);
	err += w9968cf_smbus_stop(cam);
	err += w9968cf_smbus_start(cam);
	err += w9968cf_smbus_write_byte(cam, address + 1);
	err += w9968cf_smbus_read_ack(cam);
	err += w9968cf_smbus_read_byte(cam, value);
	err += w9968cf_smbus_write_ack(cam);
	err += w9968cf_smbus_stop(cam);

	/* Serial data disable */
	err += w9968cf_write_sb(cam, 0x0000);

	if (!err)
		DBG(5, "I2C read byte data done, addr.0x%04X, "
		       "subaddr.0x%02X, value 0x%02X",
		    address, subaddress, *value)
	else
		DBG(5, "I2C read byte data failed, addr.0x%04X, "
		       "subaddr.0x%02X, wrong value 0x%02X",
		    address, subaddress, *value)

	return err;
}


/* SMBus protocol: S Addr+1 Rd [A] [Value] NA P */
static int
w9968cf_i2c_adap_read_byte(struct w9968cf_device* cam,
			   u16 address, u8* value)
{
	int err = 0;

	/* Serial data enable */
	err += w9968cf_write_sb(cam, 0x0013);

	err += w9968cf_smbus_start(cam);
	err += w9968cf_smbus_write_byte(cam, address + 1);
	err += w9968cf_smbus_read_ack(cam);
	err += w9968cf_smbus_read_byte(cam, value);
	err += w9968cf_smbus_write_ack(cam);
	err += w9968cf_smbus_stop(cam);

	/* Serial data disable */
	err += w9968cf_write_sb(cam, 0x0000);

	if (!err)
		DBG(5, "I2C read byte done, addr.0x%04X, "
		       "value 0x%02X", address, *value)
	else
		DBG(5, "I2C read byte failed, addr.0x%04X, "
		       "wrong value 0x%02X", address, *value)

	return err;
}


/* SMBus protocol: S Addr Wr [A] Value [A] P */
static int
w9968cf_i2c_adap_write_byte(struct w9968cf_device* cam,
			    u16 address, u8 value)
{
	DBG(4, "i2c_write_byte() is an unsupported transfer mode")
	return -EINVAL;
}



/****************************************************************************
 * I2C interface to kernel                                                  *
 ****************************************************************************/

static int
w9968cf_i2c_smbus_xfer(struct i2c_adapter *adapter, u16 addr,
		       unsigned short flags, char read_write, u8 command,
		       int size, union i2c_smbus_data *data)
{
	struct v4l2_device *v4l2_dev = i2c_get_adapdata(adapter);
	struct w9968cf_device *cam = to_cam(v4l2_dev);
	u8 i;
	int err = 0;

	if (size == I2C_SMBUS_BYTE) {
		/* Why addr <<= 1? See OVXXX0_SID defines in ovcamchip.h */
		addr <<= 1;

		if (read_write == I2C_SMBUS_WRITE)
			err = w9968cf_i2c_adap_write_byte(cam, addr, command);
		else if (read_write == I2C_SMBUS_READ)
			for (i = 1; i <= W9968CF_I2C_RW_RETRIES; i++) {
				err = w9968cf_i2c_adap_read_byte(cam, addr,
							 &data->byte);
				if (err) {
					if (w9968cf_smbus_refresh_bus(cam)) {
						err = -EIO;
						break;
					}
				} else
					break;
			}
	} else if (size == I2C_SMBUS_BYTE_DATA) {
		addr <<= 1;

		if (read_write == I2C_SMBUS_WRITE)
			err = w9968cf_i2c_adap_fastwrite_byte_data(cam, addr,
							  command, data->byte);
		else if (read_write == I2C_SMBUS_READ) {
			for (i = 1; i <= W9968CF_I2C_RW_RETRIES; i++) {
				err = w9968cf_i2c_adap_read_byte_data(cam,addr,
							 command, &data->byte);
				if (err) {
					if (w9968cf_smbus_refresh_bus(cam)) {
						err = -EIO;
						break;
					}
				} else
					break;
			}

		} else
			return -EINVAL;

	} else {
		DBG(4, "Unsupported I2C transfer mode (%d)", size)
		return -EINVAL;
	}
	return err;
}


static u32 w9968cf_i2c_func(struct i2c_adapter* adap)
{
	return I2C_FUNC_SMBUS_READ_BYTE |
	       I2C_FUNC_SMBUS_READ_BYTE_DATA  |
	       I2C_FUNC_SMBUS_WRITE_BYTE_DATA;
}


static int w9968cf_i2c_init(struct w9968cf_device* cam)
{
	int err = 0;

	static struct i2c_algorithm algo = {
		.smbus_xfer =    w9968cf_i2c_smbus_xfer,
		.functionality = w9968cf_i2c_func,
	};

	static struct i2c_adapter adap = {
		.id =                I2C_HW_SMBUS_W9968CF,
		.owner =             THIS_MODULE,
		.algo =              &algo,
	};

	memcpy(&cam->i2c_adapter, &adap, sizeof(struct i2c_adapter));
	strcpy(cam->i2c_adapter.name, "w9968cf");
	cam->i2c_adapter.dev.parent = &cam->usbdev->dev;
	i2c_set_adapdata(&cam->i2c_adapter, &cam->v4l2_dev);

	DBG(6, "Registering I2C adapter with kernel...")

	err = i2c_add_adapter(&cam->i2c_adapter);
	if (err)
		DBG(1, "Failed to register the I2C adapter")
	else
		DBG(5, "I2C adapter registered")

	return err;
}



/****************************************************************************
 * Helper functions                                                         *
 ****************************************************************************/

/*--------------------------------------------------------------------------
  Turn on the LED on some webcams. A beep should be heard too.
  Return 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_turn_on_led(struct w9968cf_device* cam)
{
	int err = 0;

	err += w9968cf_write_reg(cam, 0xff00, 0x00); /* power-down */
	err += w9968cf_write_reg(cam, 0xbf17, 0x00); /* reset everything */
	err += w9968cf_write_reg(cam, 0xbf10, 0x00); /* normal operation */
	err += w9968cf_write_reg(cam, 0x0010, 0x01); /* serial bus, SDS high */
	err += w9968cf_write_reg(cam, 0x0000, 0x01); /* serial bus, SDS low */
	err += w9968cf_write_reg(cam, 0x0010, 0x01); /* ..high 'beep-beep' */

	if (err)
		DBG(2, "Couldn't turn on the LED")

	DBG(5, "LED turned on")

	return err;
}


/*--------------------------------------------------------------------------
  Write some registers for the device initialization.
  This function is called once on open().
  Return 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_init_chip(struct w9968cf_device* cam)
{
	unsigned long hw_bufsize = cam->maxwidth*cam->maxheight*2,
		      y0 = 0x0000,
		      u0 = y0 + hw_bufsize/2,
		      v0 = u0 + hw_bufsize/4,
		      y1 = v0 + hw_bufsize/4,
		      u1 = y1 + hw_bufsize/2,
		      v1 = u1 + hw_bufsize/4;
	int err = 0;

	err += w9968cf_write_reg(cam, 0xff00, 0x00); /* power off */
	err += w9968cf_write_reg(cam, 0xbf10, 0x00); /* power on */

	err += w9968cf_write_reg(cam, 0x405d, 0x03); /* DRAM timings */
	err +  wd->sta.connectByReasso = FALSE;
            zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_ASOC_FAILED, wd->sta.bssid, 3);
        }
    }

}

void zfStaStoreAsocRspIe(zdev_t* dev, zbuf_t* buf)
{
    u16_t offset;
    u32_t i;
    u16_t length;
    u8_t  *htcap;
    u8_t  asocBw40 = 0;
    u8_t  asocExtOffset = 0;

    zmw_get_wlan_dev(dev);

    for (i=0; i<wd->sta.asocRspFrameBodySize; i++)
    {
        wd->sta.asocRspFrameBody[i] = zmw_rx_buf_readb(dev, buf, i+24);
    }

    /* HT capabilities: 28 octets */
    if (    ((wd->sta.currentFrequency > 3000) && !(wd->supportMode & ZM_WIRELESS_MODE_5_N))
         || ((wd->sta.currentFrequency < 3000) && !(wd->supportMode & ZM_WIRELESS_MODE_24_N)) )
    {
        /* not 11n AP */
        htcap = (u8_t *)&wd->sta.ie.HtCap;
        for (i=0; i<28; i++)
        {
            htcap[i] = 0;
        }
        wd->BandWidth40 = 0;
        wd->ExtOffset = 0;
        return;
    }

    if ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_HT_CAPABILITY)) != 0xffff)
    {
        /* atheros pre n */
        zm_debug_msg0("atheros pre n");
        htcap = (u8_t *)&wd->sta.ie.HtCap;
        htcap[0] = zmw_rx_buf_readb(dev, buf, offset);
        htcap[1] = 26;
        for (i=1; i<=26; i++)
        {
            htcap[i+1] = zmw_rx_buf_readb(dev, buf, offset + i);
            zm_msg2_mm(ZM_LV_1, "ASOC:  HT Capabilities, htcap=", htcap[i+1]);
        }
    }
    else if ((offset = zfFindElement(dev, buf, ZM_WLAN_PREN2_EID_HTCAPABILITY)) != 0xffff)
    {
        /* pre n 2.0 standard */
        zm_debug_msg0("pre n 2.0 standard");
        htcap = (u8_t *)&wd->sta.ie.HtCap;
        for (i=0; i<28; i++)
        {
            htcap[i] = zmw_rx_buf_readb(dev, buf, offset + i);
            zm_msg2_mm(ZM_LV_1, "ASOC:  HT Capabilities, htcap=", htcap[i]);
        }
    }
    else
    {
        /* not 11n AP */
        htcap = (u8_t *)&wd->sta.ie.HtCap;
        for (i=0; i<28; i++)
        {
            htcap[i] = 0;
        }
        wd->BandWidth40 = 0;
        wd->ExtOffset = 0;
        return;
    }

    asocBw40 = (u8_t)((wd->sta.ie.HtCap.HtCapInfo & HTCAP_SupChannelWidthSet) >> 1);

    /* HT information */
    if ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_EXTENDED_HT_CAPABILITY)) != 0xffff)
    {
        /* atheros pre n */
        zm_debug_msg0("atheros pre n HTINFO");
        length = 22;
        htcap = (u8_t *)&wd->sta.ie.HtInfo;
        htcap[0] = zmw_rx_buf_readb(dev, buf, offset);
        htcap[1] = 22;
        for (i=1; i<=22; i++)
        {
            htcap[i+1] = zmw_rx_buf_readb(dev, buf, offset + i);
            zm_msg2_mm(ZM_LV_1, "ASOC:  HT Info, htinfo=", htcap[i+1]);
        }
    }
    else if ((offset = zfFindElement(dev, buf, ZM_WLAN_PREN2_EID_HTINFORMATION)) != 0xffff)
    {
        /* pre n 2.0 standard */
        zm_debug_msg0("pre n 2.0 standard HTINFO");
        length = zmw_rx_buf_readb(dev, buf, offset + 1);
        htcap = (u8_t *)&wd->sta.ie.HtInfo;
        for (i=0; i<24; i++)
        {
            htcap[i] = zmw_rx_buf_readb(dev, buf, offset + i);
            zm_msg2_mm(ZM_LV_1, "ASOC:  HT Info, htinfo=", htcap[i]);
        }
    }
    else
    {
        zm_debug_msg0("no HTINFO");
        htcap = (u8_t *)&wd->sta.ie.HtInfo;
        for (i=0; i<24; i++)
        {
            htcap[i] = 0;
        }
    }
    asocExtOffset = wd->sta.ie.HtInfo.ChannelInfo & ExtHtCap_ExtChannelOffsetBelow;

    if ((wd->sta.EnableHT == 1) && (asocBw40 == 1) && ((asocExtOffset == 1) || (asocExtOffset == 3)))
    {
        wd->BandWidth40 = asocBw40;
        wd->ExtOffset = asocExtOffset;
    }
    else
    {
        wd->BandWidth40 = 0;
        wd->ExtOffset = 0;
    }

    return;
}

void zfStaProcessDeauth(zdev_t* dev, zbuf_t* buf)
{
    u16_t apMacAddr[3];

    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    /* STA : if SA=connected AP then disconnect with AP */
    if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
    {
        apMacAddr[0] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET);
        apMacAddr[1] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HE
		case VIDEO_PALETTE_YUV422:
			reg_v |= 0x0000;
			cam->vpp_flag = VPP_SWAP_YUV_BYTES;
			hw_palette = VIDEO_PALETTE_UYVY;
			break;
		/* Original video is used instead of RGBX palettes.
		   Software conversion later. */
		case VIDEO_PALETTE_GREY:
		case VIDEO_PALETTE_RGB555:
		case VIDEO_PALETTE_RGB565:
		case VIDEO_PALETTE_RGB24:
		case VIDEO_PALETTE_RGB32:
			reg_v |= 0x0000; /* UYVY 16 bit is used */
			hw_depth = 16;
			hw_palette = VIDEO_PALETTE_UYVY;
			cam->vpp_flag = VPP_UYVY_TO_RGBX;
			break;
	}

	/* NOTE: due to memory issues, it is better to disable the hardware
		 double buffering during compression */
	if (cam->double_buffer && !(cam->vpp_flag & VPP_DECOMPRESSION))
		reg_v |= 0x0080;

	if (cam->clamping)
		reg_v |= 0x0020;

	if (cam->filter_type == 1)
		reg_v |= 0x0008;
	else if (cam->filter_type == 2)
		reg_v |= 0x000c;

	if ((err = w9968cf_write_reg(cam, reg_v, 0x16)))
		goto error;

	if ((err = w9968cf_sensor_update_picture(cam, pict)))
		goto error;

	/* If all went well, update the device data structure */
	memcpy(&cam->picture, &pict, sizeof(pict));
	cam->hw_depth = hw_depth;
	cam->hw_palette = hw_palette;

	/* Settings changed, so we clear the frame buffers */
	memset(cam->frame[0].buffer, 0, cam->nbuffers*cam->frame[0].size);

	DBG(4, "Palette is %s, depth is %u bpp",
	    symbolic(v4l1_plist, pict.palette), pict.depth)

	return 0;

error:
	DBG(1, "Failed to change picture settings")
	return err;
}


/*--------------------------------------------------------------------------
  Change the capture area size of the camera.
  This function _must_ be called _after_ w9968cf_set_picture().
  Return 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int
w9968cf_set_window(struct w9968cf_device* cam, struct video_window win)
{
	u16 x, y, w, h, scx, scy, cw, ch, ax, ay;
	unsigned long fw, fh;
	struct ovcamchip_window s_win;
	int err = 0;

	/* Work around to avoid FP arithmetics */
	#define SC(x) ((x) << 10)
	#define UNSC(x) ((x) >> 10)

	/* Make sure we are using a supported resolution */
	if ((err = w9968cf_adjust_window_size(cam, &win.width, &win.height)))
		goto error;

	/* Scaling factors */
	fw = SC(win.width) / cam->maxwidth;
	fh = SC(win.height) / cam->maxheight;

	/* Set up the width and height values used by the chip */
	if ((win.width > cam->maxwidth) || (win.height > cam->maxheight)) {
		cam->vpp_flag |= VPP_UPSCALE;
		/* Calculate largest w,h mantaining the same w/h ratio */
		w = (fw >= fh) ? cam->maxwidth : SC(win.width)/fh;
		h = (fw >= fh) ? SC(win.height)/fw : cam->maxheight;
		if (w < cam->minwidth) /* just in case */
			w = cam->minwidth;
		if (h < cam->minheight) /* just in case */
			h = cam->minheight;
	} else {
		cam->vpp_flag &= ~VPP_UPSCALE;
		w = win.width;
		h = win.height;
	}

	/* x,y offsets of the cropped area */
	scx = cam->start_cropx;
	scy = cam->start_cropy;

	/* Calculate cropped area manteining the right w/h ratio */
	if (cam->largeview && !(cam->vpp_flag & VPP_UPSCALE)) {
		cw = (fw >= fh) ? cam->maxwidth : SC(win.width)/fh;
		ch = (fw >= fh) ? SC(win.height)/fw : cam->maxheight;
	} else {
		cw = w;
		ch = h;
	}

	/* Setup the window of the sensor */
	s_win.format = VIDEO_PALETTE_UYVY;
	s_win.width = cam->maxwidth;
	s_win.height = cam->maxheight;
	s_win.quarter = 0; /* full progressive video */

	/* Center it */
	s_win.x = (s_win.width - cw) / 2;
	s_win.y = (s_win.height - ch) / 2;

	/* Clock divisor */
	if (cam->clockdiv >= 0)
		s_win.clockdiv = cam->clockdiv; /* manual override */
	else
		switch (cam->sensor) {
			case CC_OV6620:
				s_win.clockdiv = 0;
				break;
			case CC_OV6630:
				s_win.clockdiv = 0;
				break;
			case CC_OV76BE:
			case CC_OV7610:
			case CC_OV7620:
				s_win.clockdiv = 0;
				break;
			default:
				s_win.clockdiv = W9968CF_DEF_CLOCKDIVISOR;
		}

	/* We have to scale win.x and win.y offsets */
	if ( (cam->largeview && !(cam->vpp_flag & VPP_UPSCALE))
	     || (cam->vpp_flag & VPP_UPSCALE) ) {
		ax = SC(win.x)/fw;
		ay = SC(win.y)/fh;
	} else {
		ax = win.x;
		ay = win.y;
	}

	if ((ax + cw) > cam->maxwidth)
		ax = cam->maxwidth - cw;

	if ((ay + ch) > cam->maxheight)
		ay = cam->maxheight - ch;

	/* Adjust win.x, win.y */
	if ( (cam->largeview && !(cam->vpp_flag & VPP_UPSCALE))
	     || (cam->vpp_flag & VPP_UPSCALE) ) {
		win.x = UNSC(ax*fw);
		win.y = UNSC(ay*fh);
	} else {
		win.x = ax;
		win.y = ay;
	}

	/* Offsets used by the chip */
	x = ax + s_win.x;
	y = ay + s_win.y;

	/* Go ! */
	if ((err = w9968cf_sensor_cmd(cam, OVCAMCHIP_CMD_S_MODE, &s_win)))
		goto error;

	err += w9968cf_write_reg(cam, scx + x, 0x10);
	err += w9968cf_write_reg(cam, scy + y, 0x11);
	err += w9968cf_write_reg(cam, scx + x + cw, 0x12);
	err += w9968cf_write_reg(cam, scy + y + ch, 0x13);
	err += w9968cf_write_reg(cam, w, 0x14);
	err += w9968cf_write_reg(cam, h, 0x15);

	/* JPEG width & height */
	err += w9968cf_write_reg(cam, w, 0x30);
	err += w9968cf_write_reg(cam, h, 0x31);

	/* Y & UV frame buffer strides (in WORD) */
	if (cam->vpp_flag & VPP_DECOMPRESSION) {
		err += w9968cf_write_reg(cam, w/2, 0x2c);
		err += w9968cf_write_reg(cam, w/4, 0x2d);
	} else
		err += w9968cf_write_reg(cam, w, 0x2c);

	if (err)
		goto error;

	/* If all went well, update the device data structure */
	memcpy(&cam->window, &win, sizeof(win));
	cam->hw_width = w;
	cam->hw_height = h;

	/* Settings changed, so we clear the frame buffers */
	memset(cam->frame[0].buffer, 0, cam->nbuffers*cam->frame[0].size);

	DBG(4, "The capture area is %dx%d, Offset (x,y)=(%u,%u)",
	    win.width, win.height, win.x, win.y)

	PDBGG("x=%u ,y=%u, w=%u, h=%u, ax=%u, ay=%u, s_win.x=%u, s_win.y=%u, "
	      "cw=%u, ch=%u, win.x=%u, win.y=%u, win.width=%u, win.height=%u",
	      x, y, w, h, ax, ay, s_win.x, s_win.y, cw, ch, win.x, win.y,
	      win.width, win.height)

	return 0;

error:
	DBG(1, "Failed to change the capture area size")
	return err;
}


/*--------------------------------------------------------------------------
  Adjust the asked values for window width and height.
  Return 0 on success, -1 otherwise.
  --------------------------------------------------------------------------*/
static int
w9968cf_adjust_window_size(struct w9968cf_device *cam, u32 *width, u32 *height)
{
	unsigned int maxw, maxh, align;

	maxw = cam->upscaling && !(cam->vpp_flag & VPP_DECOMPRESSION) &&
	       w9968cf_vpp ? max((u16)W9968CF_MAX_WIDTH, cam->maxwidth)
			   : cam->maxwidth;
	maxh = cam->upscaling && !(cam->vpp_flag & VPP_DECOMPRESSION) &&
	       w9968cf_vpp ? max((u16)W9968CF_MAX_HEIGHT, cam->maxheight)
			   : cam->maxheight;
	align = (cam->vpp_flag & VPP_DECOMPRESSION) ? 4 : 0;

	v4l_bound_align_image(width, cam->minwidth, maxw, align,
			      height, cam->minheight, maxh, align, 0);

	PDBGG("Window size adjusted w=%u, h=%u ", *width, *height)

	return 0;
}


/*--------------------------------------------------------------------------
  Initialize the FIFO list of requested frames.
  --------------------------------------------------------------------------*/
static void w9968cf_init_framelist(struct w9968cf_device* cam)
{
	u8 i;

	for (i = 0; i < cam->nbuffers; i++) {
		cam->requested_frame[i] = NULL;
		cam->frame[i].queued = 0;
		cam->frame[i].status = F_UNUSED;
	}
}


/*--------------------------------------------------------------------------
  Add a frame in the FIFO list of requested frames.
  This function is called in process context.
  --------------------------------------------------------------------------*/
static void w9968cf_push_frame(struct w9968cf_device* cam, u8 f_num)
{
	u8 f;
	unsigned long lock_flags;

	spin_lock_irqsave(&cam->flist_lock, lock_flags);

	for (f=0; cam->requested_frame[f] != NULL; f++);
	cam->requested_frame[f] = &cam->frame[f_num];
	cam->frame[f_num].queued = 1;
	cam->frame[f_num].status = F_UNUSED; /* clear the status */

	spin_unlock_irqrestore(&cam->flist_lock, lock_flags);

	DBG(6, "Frame #%u pushed into the FIFO list. Position %u", f_num, f)
}


/*--------------------------------------------------------------------------
  Read, store and remove the first pointer in the FIFO list of requested
  frames. This function is called in interrupt context.
  --------------------------------------------------------------------------*/
static void
w9968cf_pop_frame(struct w9968cf_device* cam, struct w9968cf_frame_t** framep)
{
	u8 i;

	spin_lock(&cam->flist_lock);

	*framep = cam->requested_frame[0];

	/* Shift the list of pointers */
	for (i = 0; i < cam->nbuffers-1; i++)
		cam->requested_frame[i] = cam->requested_frame[i+1];
	cam->requested_frame[i] = NULL;

	spin_unlock(&cam->flist_lock);

	DBG(6,"Popped frame #%d from the list", (*framep)->number)
}


/*--------------------------------------------------------------------------
  High-level video post-processing routine on grabbed frames.
  Return 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int
w9968cf_postprocess_frame(struct w9968cf_device* cam,
			  struct w9968cf_frame_t* fr)
{
	void *pIn = fr->buffer, *pOut = cam->frame_vpp.buffer, *tmp;
	u16 w = cam->window.width,
	    h = cam->window.height,
	    d = cam->picture.depth,
	    fmt = cam->picture.palette,
	    rgb = cam->force_rgb,
	    hw_w = cam->hw_width,
	    hw_h = cam->hw_height,
	    hw_d = cam->hw_depth;
	int err = 0;

	#define _PSWAP(pIn, pOut) {tmp = (pIn); (pIn) = (pOut); (pOut) = tmp;}

	if (cam->vpp_flag & VPP_DECOMPRESSION) {
		memcpy(pOut, pIn, fr->length);
		_PSWAP(pIn, pOut)
		err = w9968cf_vpp->decode(pIn, fr->length, hw_w, hw_h, pOut);
		PDBGG("Compressed frame length: %lu",(unsigned long)fr->length)
		fr->length = (hw_w*hw_h*hw_d)/8;
		_PSWAP(pIn, pOut)
		if (err) {
			DBG(4, "An error occurred while decoding the frame: "
			       "%s", symbolic(decoder_errlist, err))
			return err;
		} else
			DBG(6, "Frame decoded")
	}

	if (cam->vpp_flag & VPP_SWAP_YUV_BYTES) {
		w9968cf_vpp->swap_yuvbytes(pIn, fr->length);
		DBG(6, "Original UYVY component ordering changed")
	}

	if (cam->vpp_flag & VPP_UPSCALE) {
		w9968cf_vpp->scale_up(pIn, pOut, hw_w, hw_h, hw_d, w, h);
		fr->length = (w*h*hw_d)/8;
		_PSWAP(pIn, pOut)
		DBG(6, "Vertical up-scaling done: %u,%u,%ubpp->%u,%u",
		    hw_w, hw_h, hw_d, w, h)
	}

	if (cam->vpp_flag & VPP_UYVY_TO_RGBX) {
		w9968cf_vpp->uyvy_to_rgbx(pIn, fr->length, pOut, fmt, rgb);
		fr->length = (w*h*d)/8;
		_PSWAP(pIn, pOut)
		DBG(6, "UYVY-16bit to %s conversion done",
		    symbolic(v4l1_plist, fmt))
	}

	if (pOut == fr->buffer)
		memcpy(fr->buffer, cam->frame_vpp.buffer, fr->length);

	return 0;
}



/****************************************************************************
 * Image sensor control routines                                            *
 ****************************************************************************/

static int
w9968cf_sensor_set_control(struct w9968cf_device* cam, int cid, int val)
{
	struct ovcamchip_control ctl;
	int err;

	ctl.id = cid;
	ctl.value = val;

	err = w9968cf_sensor_cmd(cam, OVCAMCHIP_CMD_S_CTRL, &ctl);

	return err;
}


static int
w9968cf_sensor_get_control(struct w9968cf_device* cam, int cid, int* val)
{
	struct ovcamchip_control ctl;
	int err;

	ctl.id = cid;

	err = w9968cf_sensor_cmd(cam, OVCAMCHIP_CMD_G_CTRL, &ctl);
	if (!err)
		*val = ctl.value;

	return err;
}


static int
w9968cf_sensor_cmd(struct w9968cf_device* cam, unsigned int cmd, void* arg)
{
	int rc;

	rc = v4l2_subdev_call(cam->sensor_sd, core, ioctl, cmd, arg);
	/* The I2C driver returns -EPERM on non-supported controls */
	return (rc < 0 && rc != -EPERM) ? rc : 0;
}


/*--------------------------------------------------------------------------
  Update some settings of the image sensor.
  Returns: 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_sensor_update_settings(struct w9968cf_device* cam)
{
	int err = 0;

	/* Auto brightness */
	err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_AUTOBRIGHT,
					 cam->auto_brt);
	if (err)
		return err;

	/* Auto exposure */
	err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_AUTOEXP,
					 cam->auto_exp);
	if (err)
		return err;

	/* Banding filter */
	err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_BANDFILT,
					 cam->bandfilt);
	if (err)
		return err;

	/* Light frequency */
	err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_FREQ,
					 cam->lightfreq);
	if (err)
		return err;

	/* Back light */
	err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_BACKLIGHT,
					 cam->backlight);
	if (err)
		return err;

	/* Mirror */
	err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_MIRROR,
					 cam->mirror);
	if (err)
		return err;

	return 0;
}


/*--------------------------------------------------------------------------
  Get some current picture settings from the image sensor and update the
  internal 'picture' structure of the camera.
  Returns: 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_sensor_get_picture(struct w9968cf_device* cam)
{
	int err, v;

	err = w9968cf_sensor_get_control(cam, OVCAMCHIP_CID_CONT, &v);
	if (err)
		return err;
	cam->picture.contrast = v;

	err = w9968cf_sensor_get_control(cam, OVCAMCHIP_CID_BRIGHT, &v);
	if (err)
		return err;
	cam->picture.brightness = v;

	err = w9968cf_sensor_get_control(cam, OVCAMCHIP_CID_SAT, &v);
	if (err)
		return err;
	cam->picture.colour = v;

	err = w9968cf_sensor_get_control(cam, OVCAMCHIP_CID_HUE, &v);
	if (err)
		return err;
	cam->picture.hue = v;

	DBG(5, "Got picture settings from the image sensor")

	PDBGG("Brightness, contrast, hue, colour, whiteness are "
	      "%u,%u,%u,%u,%u", cam->picture.brightness,cam->picture.contrast,
	      cam->picture.hue, cam->picture.colour, cam->picture.whiteness)

	return 0;
}


/*--------------------------------------------------------------------------
  Update picture settings of the image sensor.
  Returns: 0 on success, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int
w9968cf_sensor_update_picture(struct w9968cf_device* cam,
			      struct video_picture pict)
{
	int err = 0;

	if ((!cam->sensor_initialized)
	    || pict.contrast != cam->picture.contrast) {
		err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_CONT,
						 pict.contrast);
		if (err)
			goto fail;
		DBG(4, "Contrast changed from %u to %u",
		    cam->picture.contrast, pict.contrast)
		cam->picture.contrast = pict.contrast;
	}

	if (((!cam->sensor_initialized) ||
	    pict.brightness != cam->picture.brightness) && (!cam->auto_brt)) {
		err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_BRIGHT,
						 pict.brightness);
		if (err)
			goto fail;
		DBG(4, "Brightness changed from %u to %u",
		    cam->picture.brightness, pict.brightness)
		cam->picture.brightness = pict.brightness;
	}

	if ((!cam->sensor_initialized) || pict.colour != cam->picture.colour) {
		err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_SAT,
						 pict.colour);
		if (err)
			goto fail;
		DBG(4, "Colour changed from %u to %u",
		    cam->picture.colour, pict.colour)
		cam->picture.colour = pict.colour;
	}

	if ((!cam->sensor_initialized) || pict.hue != cam->picture.hue) {
		err = w9968cf_sensor_set_control(cam, OVCAMCHIP_CID_HUE,
						 pict.hue);
		if (err)
			goto fail;
		DBG(4, "Hue changed from %u to %u",
		    cam->picture.hue, pict.hue)
		cam->picture.hue = pict.hue;
	}

	return 0;

fail:
	DBG(4, "Failed to change sensor picture setting")
	return err;
}



/****************************************************************************
 * Camera configuration                                                     *
 ****************************************************************************/

/*--------------------------------------------------------------------------
  This function is called when a supported image sensor is detected.
  Return 0 if the initialization succeeds, a negative number otherwise.
  --------------------------------------------------------------------------*/
static int w9968cf_sensor_init(struct w9968cf_device* cam)
{
	int err = 0;

	if ((err = w9968cf_sensor_cmd(cam, OVCAMCHIP_CMD_INITIALIZE,
				      &cam->monochrome)))
		goto error;

	if ((err = w9968cf_sensor_cmd(cam, OVCAMCHIP_CMD_Q_SUBTYPE,
				      &cam->sensor)))
		goto error;

	/* NOTE: Make sure width and height are a multiple of 16 */
	switch (v4l2_i2c_subdev_addr(cam->sensor_sd)) {
		case OV6xx0_SID:
			cam->maxwidth = 352;
			cam->maxheight = 288;
			cam->minwidth = 64;
			cam->minheight = 48;
			break;
		case OV7xx0_SID:
			cam->maxwidth = 640;
			cam->maxheight = 480;
			cam->minwidth = 64;
			cam->minheight = 48;
			break;
		default:
			DBG(1, "Not supported image sensor detected for %s",
			    symbolic(camlist, cam->id))
			return -EINVAL;
	}

	/* These values depend on the ones in the ovxxx0.c sources */
	switch (cam->sensor) {
		case CC_OV7620:
			cam->start_cropx = 287;
			cam->start_cropy = 35;
			/* Seems to work around a bug in the image sensor */
			cam->vs_polarity = 1;
			cam->hs_polarity = 1;
			break;
		default:
			cam->start_cropx = 320;
			cam->start_cropy = 35;
			cam->vs_polarity = 1;
			cam->hs_polarity = 0;
	}

	if ((err = w9968cf_sensor_update_settings(cam)))
		goto error;

	if ((err = w9968cf_sensor_update_picture(cam, cam->picture)))
		goto error;

	cam->sensor_initialized = 1;

	DBG(2, "%s image sensor initialized", symbolic(senlist, cam->sensor))
	return 0;

error:
	cam->sensor_initialized = 0;
	cam->sensor = CC_UNKNOWN;
	DBG(1, "Image sensor initialization failed for %s (/dev/video%d). "
	       "Try to detach and attach this device again",
	    symbolic(camlist, cam->id), cam->v4ldev->num)
	return err;
}


/*--------------------------------------------------------------------------
  Fill some basic fields in the main device data structure.
  This function is called once on w9968cf_usb_probe() for each recognized
  camera.
  --------------------------------------------------------------------------*/
static void
w9968cf_configure_camera(struct w9968cf_device* cam,
			 struct usb_device* udev,
			 enum w9968cf_model_id mod_id,
			 const unsigned short dev_nr)
{
	mutex_init(&cam->fileop_mutex);
	init_waitqueue_head(&cam->open);
	spin_lock_init(&cam->urb_lock);
	spin_lock_init(&cam->flist_lock);

	cam->users = 0;
	cam->disconnected = 0;
	cam->id = mod_id;
	cam->sensor = CC_UNKNOWN;
	cam->sensor_initialized = 0;

	/* Calculate the alternate setting number (from 1 to 16)
	   according to the 'packet_size' module parameter */
	if (packet_size[dev_nr] < W9968CF_MIN_PACKET_SIZE)
		packet_size[dev_nr] = W9968CF_MIN_PACKET_SIZE;
	for (cam->altsetting = 1;
	     packet_size[dev_nr] < wMaxPacketSize[cam->altsetting-1];
	     cam->altsetting++);

	cam->max_buffers = (max_buffers[dev_nr] < 2 ||
			    max_buffers[dev_nr] > W9968CF_MAX_BUFFERS)
			   ? W9968CF_BUFFERS : (u8)max_buffers[dev_nr];

	cam->double_buffer = (double_buffer[dev_nr] == 0 ||
			      double_buffer[dev_nr] == 1)
			     ? (u8)double_buffer[dev_nr]:W9968CF_DOUBLE_BUFFER;

	cam->clamping = (clamping[dev_nr] == 0 || clamping[dev_nr] == 1)
			? (u8)clamping[dev_nr] : W9968CF_CLAMPING;

	cam->filter_type = (filter_type[dev_nr] == 0 ||
			    filter_type[dev_nr] == 1 ||
			    filter_type[dev_nr] == 2)
			   ? (u8)filter_type[dev_nr] : W9968CF_FILTER_TYPE;

	cam->capture = 1;

	cam->largeview = (largeview[dev_nr] == 0 || largeview[dev_nr] == 1)
			 ? (u8)largeview[dev_nr] : W9968CF_LARGEVIEW;

	cam->decompression = (decompression[dev_nr] == 0 ||
			      decompression[dev_nr] == 1 ||
			      decompression[dev_nr] == 2)
			     ? (u8)decompression[dev_nr]:W9968CF_DECOMPRESSION;

	cam->upscaling = (upscaling[dev_nr] == 0 ||
			  upscaling[dev_nr] == 1)
			 ? (u8)upscaling[dev_nr] : W9968CF_UPSCALING;

	cam->auto_brt = (autobright[dev_nr] == 0 || autobright[dev_nr] == 1)
			? (u8)autobright[dev_nr] : W9968CF_AUTOBRIGHT;

	cam->auto_exp = (autoexp[dev_nr] == 0 || autoexp[dev_nr] == 1)
			? (u8)autoexp[dev_nr] : W9968CF_AUTOEXP;

	cam->lightfreq = (lightfreq[dev_nr] == 50 || lightfreq[dev_nr] == 60)
			 ? (u8)lightfreq[dev_nr] : W9968CF_LIGHTFREQ;

	cam->bandfilt = (bandingfilter[dev_nr] == 0 ||
			 bandingfilter[dev_nr] == 1)
			? (u8)bandingfilter[dev_nr] : W9968CF_BANDINGFILTER;

	cam->backlight = (backlight[dev_nr] == 0 || backlight[dev_nr] == 1)
			 ? (u8)backlight[dev_nr] : W9968CF_BACKLIGHT;

	cam->clockdiv = (clockdiv[dev_nr] == -1 || clockdiv[dev_nr] >= 0)
			? (s8)clockdiv[dev_nr] : W9968CF_CLOCKDIV;

	cam->mirror = (mirror[dev_nr] == 0 || mirror[dev_nr] == 1)
		      ? (u8)mirror[dev_nr] : W9968CF_MIRROR;

	cam->monochrome = (monochrome[dev_nr] == 0 || monochrome[dev_nr] == 1)
			  ? monochrome[dev_nr] : W9968CF_MONOCHROME;

	cam->picture.brightness = (u16)brightness[dev_nr];
	cam->picture.hue = (u16)hue[dev_nr];
	cam->picture.colour = (u16)colour[dev_nr];
	cam->picture.contrast = (u16)contrast[dev_nr];
	cam->picture.whiteness = (u16)whiteness[dev_nr];
	if (w9968cf_valid_palette((u16)force_palette[dev_nr])) {
		cam->picture.palette = (u16)force_palette[dev_nr];
		cam->force_palette = 1;
	} else {
		cam->force_palette = 0;
		if (cam->decompression == 0)
			cam->picture.palette = W9968CF_PALETTE_DECOMP_OFF;
		else if (cam->decompression == 1)
			cam->picture.palette = W9968CF_PALETTE_DECOMP_FORCE;
		else
			cam->picture.palette = W9968CF_PALETTE_DECOMP_ON;
	}
	cam->picture.depth = w9968cf_valid_depth(cam->picture.palette);

	cam->force_rgb = (force_rgb[dev_nr] == 0 || force_rgb[dev_nr] == 1)
			 ? (u8)force_rgb[dev_nr] : W9968CF_FORCE_RGB;

	cam->window.x = 0;
	cam->window.y = 0;
	cam->window.width = W9968CF_WIDTH;
	cam->window.height = W9968CF_HEIGHT;
	cam->window.chromakey = 0;
	cam->window.clipcount = 0;
	cam->window.flags = 0;

	DBG(3, "%s configured with settings #%u:",
	    symbolic(camlist, cam->id), dev_nr)

	DBG(3, "- Data packet size for USB isochrnous transfer: %u bytes",
	    wMaxPacketSize[cam->altsetting-1])

	DBG(3, "- Number of requested video frame buffers: %u",
	    cam->max_buffers)

	if (cam->double_buffer)
		DBG(3, "- Hardware double buffering enabled")
	else
		DBG(3, "- Hardware double buffering disabled")

	if (cam->filter_type == 0)
		DBG(3, "- Video filtering disabled")
	else if (cam->filter_type == 1)
		DBG(3, "- Video filtering enabled: type 1-2-1")
	else if (cam->filter_type == 2)
		DBG(3, "- Video filtering enabled: type 2-3-6-3-2")

	if (cam->clamping)
		DBG(3, "- Video data clamping (CCIR-601 format) enabled")
	else
		DBG(3, "- Video data clamping (CCIR-601 format) disabled")

	if (cam->largeview)
		DBG(3, "- Large view enabled")
	else
		DBG(3, "- Large view disabled")

	if ((cam->decompression) == 0 && (!cam->force_palette))
		DBG(3, "- Decompression disabled")
	else if ((cam->decompression) == 1 && (!cam->force_palette))
		DBG(3, "- Decompression forced")
	else if ((cam->decompression) == 2 && (!cam->force_palette))
		DBG(3, "- Decompression allowed")

	if (cam->upscaling)
		DBG(3, "- Software image scaling enabled")
	else
		DBG(3, "- Software image scaling disabled")

	if (cam->force_palette)
		DBG(3, "- Image palette forced to %s",
		    symbolic(v4l1_plist, cam->picture.palette))

	if (cam->force_rgb)
		DBG(3, "- RGB component ordering will be used instead of BGR")

	if (cam->auto_brt)
		DBG(3, "- Auto brightness enabled")
	else
		DBG(3, "- Auto brightness disabled")

	if (cam->auto_exp)
		DBG(3, "- Auto exposure enabled")
	else
		DBG(3, "- Auto exposure disabled")

	if (cam->backlight)
		DBG(3, "- Backlight exposure algorithm enabled")
	else
		DBG(3, "- Backlight exposure algorithm disabled")

	if (cam->mirror)
		DBG(3, "- Mirror enabled")
	else
		DBG(3, "- Mirror disabled")

	if (cam->bandfilt)
		DBG(3, "- Banding filter enabled")
	else
		DBG(3, "- Banding filter disabled")

	DBG(3, "- Power lighting frequency: %u", cam->lightfreq)

	if (cam->clockdiv == -1)
		DBG(3, "- Automatic clock divisor enabled")
	else
		DBG(3, "- Clock divisor: %d", cam->clockdiv)

	if (cam->monochrome)
		DBG(3, "- Image sensor used as monochrome")
	else
		DBG(3, "- Image sensor not used as monochrome")
}


/*--------------------------------------------------------------------------
  If the video post-processing module is not loaded, some parameters
  must be overridden.
  --------------------------------------------------------------------------*/
static void w9968cf_adjust_configuration(struct w9968cf_device* cam)
{
	if (!w9968cf_vpp) {
		if (cam->decompression == 1) {
			cam->decompression = 2;
			DBG(2, "Video post-processing module not found: "
			       "'decompression' parameter forced to 2")
		}
		if (cam->upscaling) {
			cam->upscaling = 0;
			DBG(2, "Video post-processing module not found: "
			       "'upscaling' parameter forced to 0")
		}
		if (cam->picture.palette != VIDEO_PALETTE_UYVY) {
			cam->force_palette = 0;
			DBG(2, "Video post-processing module not found: "
			       "'force_palette' parameter forced to 0")
		}
		cam->picture.palette = VIDEO_PALETTE_UYVY;
		cam->picture.depth = w9968cf_valid_depth(cam->picture.palette);
	}
}


/*--------------------------------------------------------------------------
  Release the resources used by the driver.
  This function is called on disconnect
  (or on close if deallocation has been deferred)
  --------------------------------------------------------------------------*/
static void w9968cf_release_resources(struct w9968cf_device* cam)
{
	mutex_lock(&w9968cf_devlist_mutex);

	DBG(2, "V4L device deregistered: /dev/video%d", cam->v4ldev->num)

	video_unregister_device(cam->v4ldev);
	list_del(&cam->v4llist);
	i2c_del_adapter(&cam->i2c_adapter);
	w9968cf_deallocate_memory(cam);
	kfree(cam->control_buffer);
	kfree(cam->data_buffer);
	v4l2_device_unregister(&cam->v4l2_dev);

	mutex_unlock(&w9968cf_devlist_mutex);
}



/****************************************************************************
 * Video4Linux interface                                                    *
 ****************************************************************************/

static int w9968cf_open(struct file *filp)
{
	struct w9968cf_device* cam;
	int err;

	/* This the only safe way to prevent race conditions with disconnect */
	if (!down_read_trylock(&w9968cf_disconnect))
		return -EAGAIN;

	cam = (struct w9968cf_device*)video_get_drvdata(video_devdata(filp));

	mutex_lock(&cam->dev_mutex);

	if (cam->sensor == CC_UNKNOWN) {
		DBG(2, "No supported image sensor has been detected by the "
		       "'ovcamchip' module for the %s (/dev/video%d). Make "
		       "sure it is loaded *before* (re)connecting the camera.",
		    symbolic(camlist, cam->id), cam->v4ldev->num)
		mutex_unlock(&cam->dev_mutex);
		up_read(&w9968cf_disconnect);
		return -ENODEV;
	}

	if (cam->users) {
		DBG(2, "%s (/dev/video%d) has been already occupied by '%s'",
		    symbolic(camlist, cam->id), cam->v4ldev->num, cam->command)
		if ((filp->f_flags & O_NONBLOCK)||(filp->f_flags & O_NDELAY)) {
			mutex_unlock(&cam->dev_mutex);
			up_read(&w9968cf_disconnect);
			return -EWOULDBLOCK;
		}
		mutex_unlock(&cam->dev_mutex);
		err = wait_event_interruptible_exclusive(cam->open,
							 cam->disconnected ||
							 !cam->users);
		if (err) {
			up_read(&w9968cf_disconnect);
			return err;
		}
		if (cam->disconnected) {
			up_read(&w9968cf_disconnect);
			return -ENODEV;
		}
		mutex_lock(&cam->dev_mutex);
	}

	DBG(5, "Opening '%s', /dev/video%d ...",
	    symbolic(camlist, cam->id), cam->v4ldev->num)

	cam->streaming = 0;
	cam->misconfigured = 0;

	w9968cf_adjust_configuration(cam);

	if ((err = w9968cf_allocate_memory(cam)))
		goto deallocate_memory;

	if ((err = w9968cf_init_chip(cam)))
		goto deallocate_memory;

	if ((err = w9968cf_start_transfer(cam)))
		goto deallocate_memory;

	filp->private_data = cam;

	cam->users++;
	strcpy(cam->command, current->comm);

	init_waitqueue_head(&cam->wait_queue);

	DBG(5, "Video device is open")

	mutex_unlock(&cam->dev_mutex);
	up_read(&w9968cf_disconnect);

	return 0;

deallocate_memory:
	w9968cf_deallocate_memory(cam);
	DBG(2, "Failed to open the video device")
	mutex_unlock(&cam->dev_mutex);
	up_read(&w9968cf_disconnect);
	return err;
}


static int w9968cf_release(struct file *filp)
{
	struct w9968cf_device* cam;

	cam = (struct w9968cf_device*)video_get_drvdata(video_devdata(filp));

	mutex_lock(&cam->dev_mutex); /* prevent disconnect() to be called */

	w9968cf_stop_transfer(cam);

	if (cam->disconnected) {
		w9968cf_release_resources(cam);
		mutex_unlock(&cam->dev_mutex);
		kfree(cam);
		return 0;
	}

	cam->users--;
	w9968cf_deallocate_memory(cam);
	wake_up_interruptible_nr(&cam->open, 1);

	DBG(5, "Video device closed")
	mutex_unlock(&cam->dev_mutex);
	return 0;
}


static ssize_t
w9968cf_read(struct file* filp, char __user * buf, size_t count, loff_t* f_pos)
{
	struct w9968cf_device* cam;
	struct w9968cf_frame_t* fr;
	int err = 0;

	cam = (struct w9968cf_device*)video_get_drvdata(video_devdata(filp));

	if (filp->f_flags & O_NONBLOCK)
		return -EWOULDBLOCK;

	if (mutex_lock_interruptible(&cam->fileop_mutex))
		return -ERESTARTSYS;

	if (cam->disconnected) {
		DBG(2, "Device not present")
		mutex_unlock(&cam->fileop_mutex);
		return -ENODEV;
	}

	if (cam->misconfigured) {
		DBG(2, "The camera is misconfigured. Close and open it again.")
		mutex_unlock(&cam->fileop_mutex);
		return -EIO;
	}

	if (!cam->frame[0].queued)
		w9968cf_push_frame(cam, 0);

	if (!cam->frame[1].queued)
		w9968cf_push_frame(cam, 1);

	err = wait_event_interruptible(cam->wait_queue,
				       cam->frame[0].status == F_READY ||
				       cam->frame[1].status == F_READY ||
				       cam->disconnected);
	if (err) {
		mutex_unlock(&cam->fileop_mutex);
		return err;
	}
	if (cam->disconnected) {
		mutex_unlock(&cam->fileop_mutex);
		return -ENODEV;
	}

	fr = (cam->frame[0].status == F_READY) ? &cam->frame[0]:&cam->frame[1];

	if (w9968cf_vpp)
		w9968cf_postprocess_frame(cam, fr);

	if (count > fr->length)
		count = fr->length;

	if (copy_to_user(buf, fr->buffer, count)) {
		fr->status = F_UNUSED;
		mutex_unlock(&cam->fileop_mutex);
		return -EFAULT;
	}
	*f_pos += count;

	fr->status = F_UNUSED;

	DBG(5, "%zu bytes read", count)

	mutex_unlock(&cam->fileop_mutex);
	return count;
}


static int w9968cf_mmap(struct file* filp, struct vm_area_struct *vma)
{
	struct w9968cf_device* cam = (struct w9968cf_device*)
				     video_get_drvdata(video_devdata(filp));
	unsigned long vsize = vma->vm_end - vma->vm_start,
		      psize = cam->nbuffers * cam->frame[0].size,
		      start = vma->vm_start,
		      pos = (unsigned long)cam->frame[0].buffer,
		      page;

	if (cam->disconnected) {
		DBG(2, "Device not present")
		return -ENODEV;
	}

	if (cam->misconfigured) {
		DBG(2, "The camera is misconfigured. Close and open it again")
		return -EIO;
	}

	PDBGG("mmapping %lu bytes...", vsize)

	if (vsize > psize - (vma->vm_pgoff << PAGE_SHIFT))
		return -EINVAL;

	while (vsize > 0) {
		page = vmalloc_to_pfn((void *)pos);
		if (remap_pfn_range(vma, start, page + vma->vm_pgoff,
						PAGE_SIZE, vma->vm_page_prot))
			return -EAGAIN;
		start += PAGE_SIZE;
		pos += PAGE_SIZE;
		vsize -= PAGE_SIZE;
	}

	DBG(5, "mmap method successfully called")
	return 0;
}


static long
w9968cf_ioctl(struct file *filp,
	      unsigned int cmd, unsigned long arg)
{
	struct w9968cf_device* cam;
	long err;

	cam = (struct w9968cf_device*)video_get_drvdata(video_devdata(filp));

	if (mutex_lock_interruptible(&cam->fileop_mutex))
		return -ERESTARTSYS;

	if (cam->disconnected) {
		DBG(2, "Device not present")
		mutex_unlock(&cam->fileop_mutex);
		return -ENODEV;
	}

	if (cam->misconfigured) {
		DBG(2, "The camera is misconfigured. Close and open it again.")
		mutex_unlock(&cam->fileop_mutex);
		return -EIO;
	}

	err = w9968cf_v4l_ioctl(filp, cmd, (void __user *)arg);

	mutex_unlock(&cam->fileop_mutex);
	return err;
}


static long w9968cf_v4l_ioctl(struct file *filp,
			     unsigned int cmd, void __user *arg)
{
	struct w9968cf_device* cam;
	const char* v4l1_ioctls[] = {
		"?", "CGAP", "GCHAN", "SCHAN", "GTUNER", "STUNER",
		"GPICT", "SPICT", "CCAPTURE", "GWIN", "SWIN", "GFBUF",
		"SFBUF", "KEY", "GFREQ", "SFREQ", "GAUDIO", "SAUDIO",
		"SYNC", "MCAPTURE", "GMBUF", "GUNIT", "GCAPTURE", "SCAPTURE",
		"SPLAYMODE", "SWRITEMODE", "GPLAYINFO", "SMICROCODE",
		"GVBIFMT", "SVBIFMT"
	};

	#define V4L1_IOCTL(cmd) \
		((_IOC_NR((cmd)) < ARRAY_SIZE(v4l1_ioctls)) ? \
		v4l1_ioctls[_IOC_NR((cmd))] : "?")

	cam = (struct w9968cf_device*)video_get_drvdata(video_devdata(filp));

	switch (cmd) {

	case VIDIOCGCAP: /* get video capability */
	{
		struct video_capability cap = {
			.type = VID_TYPE_CAPTURE | VID_TYPE_SCALES,
			.channels = 1,
			.audios = 0,
			.minwidth = cam->minwidth,
			.minheight = cam->minheight,
		};
		sprintf(cap.name, "W996[87]CF USB Camera #%d",
			cam->v4ldev->num);
		cap.maxwidth = (cam->upscaling && w9968cf_vpp)
			       ? max((u16)W9968CF_MAX_WIDTH, cam->maxwidth)
				 : cam->maxwidth;
		cap.maxheight = (cam->upscaling && w9968cf_vpp)
				? max((u16)W9968CF_MAX_HEIGHT, cam->maxheight)
				  : cam->maxheight;

		if (copy_to_user(arg, &cap, sizeof(cap)))
			return -EFAULT;

		DBG(5, "VIDIOCGCAP successfully called")
		return 0;
	}

	case VIDIOCGCHAN: /* get video channel informations */
	{
		struct video_channel chan;
		if (copy_from_user(&chan, arg, sizeof(chan)))
			return -EFAULT;

		if (chan.channel != 0)
			return -EINVAL;

		strcpy(chan.name, "Camera");
		chan.tuners = 0;
		chan.flags = 0;
		chan.type = VIDEO_TYPE_CAMERA;
		chan.norm = VIDEO_MODE_AUTO;

		if (copy_to_user(arg, &chan, sizeof(chan)))
			return -EFAULT;

		DBG(5, "VIDIOCGCHAN successfully called")
		return 0;
	}

	case VIDIOCSCHAN: /* set active channel */
	{
		struct video_channel chan;

		if (copy_from_user(&chan, arg, sizeof(chan)))
			return -EFAULT;

		if (chan.channel != 0)
			return -EINVAL;

		DBG(5, "VIDIOCSCHAN successfully called")
		return 0;
	}

	case VIDIOCGPICT: /* get image properties of the picture */
	{
		if (w9968cf_sensor_get_picture(cam))
			return -EIO;

		if (copy_to_user(arg, &cam->picture, sizeof(cam->picture)))
			return -EFAULT;

		DBG(5, "VIDIOCGPICT successfully called")
		return 0;
	}

	case VIDIOCSPICT: /* change picture settings */
	{
		struct video_picture pict;
		int err = 0;

		if (copy_from_user(&pict, arg, sizeof(pict)))
			return -EFAULT;

		if ( (cam->force_palette || !w9968cf_vpp)
		     && pict.palette != cam->picture.palette ) {
			DBG(4, "Palette %s rejected: only %s is allowed",
			    symbolic(v4l1_plist, pict.palette),
			    symbolic(v4l1_plist, cam->picture.palette))
			return -EINVAL;
		}

		if (!w9968cf_valid_palette(pict.palette)) {
			DBG(4, "Palette %s not supported. VIDIOCSPICT failed",
			    symbolic(v4l1_plist, pict.palette))
			return -EINVAL;
		}

		if (!cam->force_palette) {
		   if (cam->decompression == 0) {
		      if (w9968cf_need_decompression(pict.palette)) {
			 DBG(4, "Decompression disabled: palette %s is not "
				"allowed. VIDIOCSPICT failed",
			     symbolic(v4l1_plist, pict.palette))
			 return -EINVAL;
		      }
		   } else if (cam->decompression == 1) {
		      if (!w9968cf_need_decompression(pict.palette)) {
			 DBG(4, "Decompression forced: palette %s is not "
				"allowed. VIDIOCSPICT failed",
			     symbolic(v4l1_plist, pict.palette))
			 return -EINVAL;
		      }
		   }
		}

		if (pict.depth != w9968cf_valid_depth(pict.palette)) {
			DBG(4, "Requested depth %u bpp is not valid for %s "
			       "palette: ignored and changed to %u bpp",
			    pict.depth, symbolic(v4l1_plist, pict.palette),
			    w9968cf_valid_depth(pict.palette))
			pict.depth = w9968cf_valid_depth(pict.palette);
		}

		if (pict.palette != cam->picture.palette) {
			if(*cam->requested_frame
			   || cam->frame_current->queued) {
				err = wait_event_interruptible
				      ( cam->wait_queue,
					cam->disconnected ||
					(!*cam->requested_frame &&
					 !cam->frame_current->queued) );
				if (err)
					return err;
				if (cam->disconnected)
					return -ENODEV;
			}

			if (w9968cf_stop_transfer(cam))
				goto ioctl_fail;

			if (w9968cf_set_picture(cam, pict))
				goto ioctl_fail;

			if (w9968cf_start_transfer(cam))
				goto ioctl_fail;

		} else if (w9968cf_sensor_update_picture(cam, pict))
			return -EIO;


		DBG(5, "VIDIOCSPICT successfully called")
		return 0;
	}

	case VIDIOCSWIN: /* set capture area */
	{
		struct video_window win;
		int err = 0;

		if (copy_from_user(&win, arg, sizeof(win)))
			return -EFAULT;

		DBG(6, "VIDIOCSWIN called: clipcount=%d, flags=%u, "
		       "x=%u, y=%u, %ux%u", win.clipcount, win.flags,
		    win.x, win.y, win.width, win.height)

		if (win.clipcount != 0 || win.flags != 0)
			return -EINVAL;

		if ((err = w9968cf_adjust_window_size(cam, &win.width,
						      &win.height))) {
			DBG(4, "Resolution not supported (%ux%u). "
			       "VIDIOCSWIN failed", win.width, win.height)
			return err;
		}

		if (win.x != cam->window.x ||
		    win.y != cam->window.y ||
		    win.width != cam->window.width ||
		    win.height != cam->window.height) {
			if(*cam->requested_frame
			   || cam->frame_current->queued) {
				err = wait_event_interruptible
				      ( cam->wait_queue,
					cam->disconnected ||
					(!*cam->requested_frame &&
					 !cam->frame_current->queued) );
				if (err)
					return err;
				if (cam->disconnected)
					return -ENODEV;
			}

			if (w9968cf_stop_transfer(cam))
				goto ioctl_fail;

			/* This _must_ be called before set_window() */
			if (w9968cf_set_picture(cam, cam->picture))
				goto ioctl_fail;

			if (w9968cf_set_window(cam, win))
				goto ioctl_fail;

			if (w9968cf_start_transfer(cam))
				goto ioctl_fail;
		}

		DBG(5, "VIDIOCSWIN successfully called. ")
		return 0;
	}

	case VIDIOCGWIN: /* get current window properties */
	{
		if (copy_to_user(arg,&cam->window,sizeof(struct video_window)))
			return -EFAULT;

		DBG(5, "VIDIOCGWIN successfully called")
		return 0;
	}

	case VIDIOCGMBUF: /* request for memory (mapped) buffer */
	{
		struct video_mbuf mbuf;
		u8 i;

		mbuf.size = cam->nbuffers * cam->frame[0].size;
		mbuf.frames = cam->nbuffers;
		for (i = 0; i < cam->nbuffers; i++)
			mbuf.offsets[i] = (unsigned long)cam->frame[i].buffer -
					  (unsigned long)cam->frame[0].buffer;

		if (copy_to_user(arg, &mbuf, sizeof(mbuf)))
			return -EFAULT;

		DBG(5, "VIDIOCGMBUF successfully called")
		return 0;
	}

	case VIDIOCMCAPTURE: /* start the capture to a frame */
	{
		struct video_mmap mmap;
		struct w9968cf_frame_t* fr;
		u32 w, h;
		int err = 0;

		if (copy_from_user(&mmap, arg, sizeof(mmap)))
			return -EFAULT;

		DBG(6, "VIDIOCMCAPTURE called: frame #%u, format=%s, %dx%d",
		    mmap.frame, symbolic(v4l1_plist, mmap.format),
		    mmap.width, mmap.height)

		if (mmap.frame >= cam->nbuffers) {
			DBG(4, "Invalid frame number (%u). "
			       "VIDIOCMCAPTURE failed", mmap.frame)
			return -EINVAL;
		}

		if (mmap.format!=cam->picture.palette &&
		    (cam->force_palette || !w9968cf_vpp)) {
			DBG(4, "Palette %s rejected: only %s is allowed",
			    symbolic(v4l1_plist, mmap.format),
			    symbolic(v4l1_plist, cam->picture.palette))
			return -EINVAL;
		}

		if (!w9968cf_valid_palette(mmap.format)) {
			DBG(4, "Palette %s not supported. "
			       "VIDIOCMCAPTURE failed",
			    symbolic(v4l1_plist, mmap.format))
			return -EINVAL;
		}

		if (!cam->force_palette) {
		   if (cam->decompression == 0) {
		      if (w9968cf_need_decompression(mmap.format)) {
			 DBG(4, "Decompression disabled: palette %s is not "
				"allowed. VIDIOCSPICT failed",
			     symbolic(v4l1_plist, mmap.format))
			 return -EINVAL;
		      }
		   } else if (cam->decompression == 1) {
		      if (!w9968cf_need_decompression(mmap.format)) {
			 DBG(4, "Decompression forced: palette %s is not "
				"allowed. VIDIOCSPICT failed",
			     symbolic(v4l1_plist, mmap.format))
			 return -EINVAL;
		      }
		   }
		}

		w = mmap.width; h = mmap.height;
		err = w9968cf_adjust_window_size(cam, &w, &h);
		mmap.width = w; mmap.height = h;
		if (err) {
			DBG(4, "Resolution not supported (%dx%d). "
			       "VIDIOCMCAPTURE failed",
			    mmap.width, mmap.height)
			return err;
		}

		fr = &cam->frame[mmap.frame];

		if (mmap.width  != cam->window.width ||
		    mmap.height != cam->window.height ||
		    mmap.format != cam->picture.palette) {

			struct video_window win;
			struct video_picture pict;

			if(*cam->requested_frame
			   || cam->frame_current->queued) {
				DBG(6, "VIDIOCMCAPTURE. Change settings for "
				       "frame #%u: %dx%d, format %s. Wait...",
				    mmap.frame, mmap.width, mmap.height,
				    symbolic(v4l1_plist, mmap.format))
				err = wait_event_interruptible
				      ( cam->wait_queue,
					cam->disconnected ||
					(!*cam->requested_frame &&
					 !cam->frame_current->queued) );
				if (err)
					return err;
				if (cam->disconnected)
					return -ENODEV;
			}

			memcpy(&win, &cam->window, sizeof(win));
			memcpy(&pict, &cam->picture, sizeof(pict));
			win.width = mmap.width;
			win.height = mmap.height;
			pict.palette = mmap.format;

			if (w9968cf_stop_transfer(cam))
				goto ioctl_fail;

			/* This before set_window */
			if (w9968cf_set_picture(cam, pict))
				goto ioctl_fail;

			if (w9968cf_set_window(cam, win))
				goto ioctl_fail;

			if (w9968cf_start_transfer(cam))
				goto ioctl_fail;

		} else 	if (fr->queued) {

			DBG(6, "Wait until frame #%u is free", mmap.frame)

			err = wait_event_interruptible(cam->wait_queue,
						       cam->disconnected ||
						       (!fr->queued));
			if (err)
				return err;
			if (cam->disconnected)
				return -ENODEV;
		}

		w9968cf_push_frame(cam, mmap.frame);
		DBG(5, "VIDIOCMCAPTURE(%u): successfully called", mmap.frame)
		return 0;
	}

	case VIDIOCSYNC: /* wait until the capture of a frame is finished */
	{
		unsigned int f_num;
		struct w9968cf_frame_t* fr;
		int err = 0;

		if (copy_from_user(&f_num, arg, sizeof(f_num)))
			return -EFAULT;

		if (f_num >= cam->nbuffers) {
			DBG(4, "Invalid frame number (%u). "
			       "VIDIOCMCAPTURE failed", f_num)
			return -EINVAL;
		}

		DBG(6, "VIDIOCSYNC called for frame #%u", f_num)

		fr = &cam->frame[f_num];

		switch (fr->status) {
		case F_UNUSED:
			if (!fr->queued) {
				DBG(4, "VIDIOSYNC: Frame #%u not requested!",
				    f_num)
				return -EFAULT;
			}
		case F_ERROR:
		case F_GRABBING:
			err = wait_event_interruptible(cam->wait_queue,
						       (fr->status == F_READY)
						       || cam->disconnected);
			if (err)
				return err;
			if (cam->disconnected)
				return -ENODEV;
			break;
		case F_READY:
			break;
		}

		if (w9968cf_vpp)
			w9968cf_postprocess_frame(cam, fr);

		fr->status = F_UNUSED;

		DBG(5, "VIDIOCSYNC(%u) successfully called", f_num)
		return 0;
	}

	case VIDIOCGUNIT:/* report the unit numbers of the associated devices*/
	{
		struct video_unit unit = {
			.video = cam->v4ldev->minor,
			.vbi = VIDEO_NO_UNIT,
			.radio = VIDEO_NO_UNIT,
			.audio = VIDEO_NO_UNIT,
			.teletext = VIDEO_NO_UNIT,
		};

		if (copy_to_user(arg, &unit, sizeof(unit)))
			return -EFAULT;

		DBG(5, "VIDIOCGUNIT successfully called")
		return 0;
	}

	case VIDIOCKEY:
		return 0;

	case VIDIOCGFBUF:
	{
		if (clear_user(arg, sizeof(struct video_buffer)))
			return -EFAULT;

		DBG(5, "VIDIOCGFBUF successfully called")
		return 0;
	}

	case VIDIOCGTUNER:
	{
		struct video_tuner tuner;
		if (copy_from_user(&tuner, arg, sizeof(tuner)))
			return -EFAULT;

		if (tuner.tuner != 0)
			return -EINVAL;

		strcpy(tuner.name, "no_tuner");
		tuner.rangelow = 0;
		tuner.rangehigh = 0;
		tuner.flags = VIDEO_TUNER_NORM;
		tuner.mode = VIDEO_MODE_AUTO;
		tuner.signal = 0xffff;

		if (copy_to_user(arg, &tuner, sizeof(tuner)))
			return -EFAULT;

		DBG(5, "VIDIOCGTUNER successfully called")
		return 0;
	}

	case VIDIOCSTUNER:
	{
		struct video_tuner tuner;
		if (copy_from_user(&tuner, arg, sizeof(tuner)))
			return -EFAULT;

		if (tuner.tuner != 0)
			return -EINVAL;

		if (tuner.mode != VIDEO_MODE_AUTO)
			return -EINVAL;

		DBG(5, "VIDIOCSTUNER successfully called")
		return 0;
	}

	case VIDIOCSFBUF:
	case VIDIOCCAPTURE:
	case VIDIOCGFREQ:
	case VIDIOCSFREQ:
	case VIDIOCGAUDIO:
	case VIDIOCSAUDIO:
	case VIDIOCSPLAYMODE:
	case VIDIOCSWRITEMODE:
	case VIDIOCGPLAYINFO:
	case VIDIOCSMICROCODE:
	case VIDIOCGVBIFMT:
	case VIDIOCSVBIFMT:
		DBG(4, "Unsupported V4L1 IOCtl: VIDIOC%s "
		       "(type 0x%01X, "
		       "n. 0x%01X, "
		       "dir. 0x%01X, "
		       "size 0x%02X)",
		    V4L1_IOCTL(cmd),
		    _IOC_TYPE(cmd),_IOC_NR(cmd),_IOC_DIR(cmd),_IOC_SIZE(cmd))

		return -EINVAL;

	default:
		DBG(4, "Invalid V4L1 IOCtl: VIDIOC%s "
		       "type 0x%01X, "
		       "n. 0x%01X, "
		       "dir. 0x%01X, "
		       "size 0x%02X",
		    V4L1_IOCTL(cmd),
		    _IOC_TYPE(cmd),_IOC_NR(cmd),_IOC_DIR(cmd),_IOC_SIZE(cmd))

		return -ENOIOCTLCMD;

	} /* end of switch */

ioctl_fail:
	cam->misconfigured = 1;
	DBG(1, "VIDIOC%s failed because of hardware problems. "
	       "To use the camera, close and open it again.", V4L1_IOCTL(cmd))
	return -EFAULT;
}


static const struct v4l2_file_operations w9968cf_fops = {
	.owner =   THIS_MODULE,
	.open =    w9968cf_open,
	.release = w9968cf_release,
	.read =    w9968cf_read,
	.ioctl =   w9968cf_ioctl,
	.mmap =    w9968cf_mmap,
};



/****************************************************************************
 * USB probe and V4L registration, disconnect and id_table[] definition     *
 ****************************************************************************/

static int
w9968cf_usb_probe(struct usb_interface* intf, const struct usb_device_id* id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct w9968cf_device* cam;
	int err = 0;
	enum w9968cf_model_id mod_id;
	struct list_head* ptr;
	u8 sc = 0; /* number of simultaneous cameras */
	static unsigned short dev_nr; /* 0 - we are handling device number n */
	static unsigned short addrs[] = {
		OV7xx0_SID,
		OV6xx0_SID,
		I2C_CLIENT_END
	};

	if (le16_to_cpu(udev->descriptor.idVendor)  == winbond_id_table[0].idVendor &&
	    le16_to_cpu(udev->descriptor.idProduct) == winbond_id_table[0].idProduct)
		mod_id = W9968CF_MOD_CLVBWGP; /* see camlist[] table */
	else if (le16_to_cpu(udev->descriptor.idVendor)  == winbond_id_table[1].idVendor &&
		 le16_to_cpu(udev->descriptor.idProduct) == winbond_id_table[1].idProduct)
		mod_id = W9968CF_MOD_GENERIC; /* see camlist[] table */
	else
		return -ENODEV;

	cam = (struct w9968cf_device*)
		  kzalloc(sizeof(struct w9968cf_device), GFP_KERNEL);
	if (!cam)
		return -ENOMEM;

	err = v4l2_device_register(&intf->dev, &cam->v4l2_dev);
	if (err)
		goto fail0;

	mutex_init(&cam->dev_mutex);
	mutex_lock(&cam->dev_mutex);

	cam->usbdev = udev;

	DBG(2, "%s detected", symbolic(camlist, mod_id))

	if (simcams > W9968CF_MAX_DEVICES)
		simcams = W9968CF_SIMCAMS;

	/* How many cameras are connected ? */
	mutex_lock(&w9968cf_devlist_mutex);
	list_for_each(ptr, &w9968cf_dev_list)
		sc++;
	mutex_unlock(&w9968cf_devlist_mutex);

	if (sc >= simcams) {
		DBG(2, "Device rejected: too many connected cameras "
		       "(max. %u)", simcams)
		err = -EPERM;
		goto fail;
	}


	/* Allocate 2 bytes of memory for camera control USB transfers */
	if (!(cam->control_buffer = kzalloc(2, GFP_KERNEL))) {
		DBG(1,"Couldn't allocate memory for camera control transfers")
		err = -ENOMEM;
		goto fail;
	}

	/* Allocate 8 bytes of memory for USB data transfers to the FSB */
	if (!(cam->data_buffer = kzalloc(8, GFP_KERNEL))) {
		DBG(1, "Couldn't allocate memory for data "
		       "transfers to the FSB")
		err = -ENOMEM;
		goto fail;
	}

	/* Register the V4L device */
	cam->v4ldev = video_device_alloc();
	if (!cam->v4ldev) {
		DBG(1, "Could not allocate memory for a V4L structure")
		err = -ENOMEM;
		goto fail;
	}

	strcpy(cam->v4ldev->name, symbolic(camlist, mod_id));
	cam->v4ldev->fops = &w9968cf_fops;
	cam->v4ldev->minor = video_nr[dev_nr];
	cam->v4ldev->release = video_device_release;
	video_set_drvdata(cam->v4ldev, cam);
	cam->v4ldev->v4l2_dev = &cam->v4l2_dev;

	err = video_register_device(cam->v4ldev, VFL_TYPE_GRABBER,
				    video_nr[dev_nr]);
	if (err) {
		DBG(1, "V4L device registration failed")
		if (err == -ENFILE && video_nr[dev_nr] == -1)
			DBG(2, "Couldn't find a free /dev/videoX node")
		video_nr[dev_nr] = -1;
		dev_nr = (dev_nr < W9968CF_MAX_DEVICES-1) ? dev_nr+1 : 0;
		goto fail;
	}

	DBG(2, "V4L device registered as /dev/video%d", cam->v4ldev->num)

	/* Set some basic constants */
	w9968cf_configure_camera(cam, udev, mod_id, dev_nr);

	/* Add a new entry into the list of V4L registered devices */
	mutex_lock(&w9968cf_devlist_mutex);
	list_add(&cam->v4llist, &w9968cf_dev_list);
	mutex_unlock(&w9968cf_devlist_mutex);
	dev_nr = (dev_nr < W9968CF_MAX_DEVICES-1) ? dev_nr+1 : 0;

	w9968cf_turn_on_led(cam);

	w9968cf_i2c_init(cam);
	cam->sensor_sd = v4l2_i2c_new_probed_subdev(&cam->v4l2_dev,
			&cam->i2c_adapter,
			"ovcamchip", "ovcamchip", addrs);

	usb_set_intfdata(intf, cam);
	mutex_unlock(&cam->dev_mutex);

	err = w9968cf_sensor_init(cam);
	return 0;

fail: /* Free uncase ZM_AUTH_MODE_WPA_AUTO:
        case ZM_AUTH_MODE_WPAPSK_AUTO:
            encAlgoType = 0;
            if(pBssInfo->rsnIe[1] != 0)
            {
                UnicastCipherNum = (pBssInfo->rsnIe[8]) +
                                   (pBssInfo->rsnIe[9] << 8);

                /* If there is only one unicast cipher */
                if (UnicastCipherNum == 1)
                {
                    encAlgoType = pBssInfo->rsnIe[13];
                    //encAlgoType = pBssInfo->rsnIe[7];
                }
                else
                {
                    u16_t ii;
                    u16_t desiredCipher = 0;
                    u16_t IEOffSet = 13;

                    /* Enumerate all the supported unicast cipher */
                    for (ii = 0; ii < UnicastCipherNum; ii++)
                    {
                        if (pBssInfo->rsnIe[IEOffSet+ii*4] > desiredCipher)
                        {
                            desiredCipher = pBssInfo->rsnIe[IEOffSet+ii*4];
                        }
                    }

                    encAlgoType = desiredCipher;
                }

                if ( encAlgoType == 0x02 )
                {
    			    wd->sta.wepStatus = ZM_ENCRYPTION_TKIP;

    			    if ( wd->sta.authMode == ZM_AUTH_MODE_WPA_AUTO )
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA2;
                    }
                    else //ZM_AUTH_MODE_WPAPSK_AUTO
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA2PSK;
                    }
                }
                else if ( encAlgoType == 0x04 )
                {
                    wd->sta.wepStatus = ZM_ENCRYPTION_AES;

                    if ( wd->sta.authMode == ZM_AUTH_MODE_WPA_AUTO )
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA2;
                    }
                    else //ZM_AUTH_MODE_WPAPSK_AUTO
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA2PSK;
                    }
                }
                else
                {
                    ret = FALSE;
                }
            }
            else if(pBssInfo->wpaIe[1] != 0)
            {
                UnicastCipherNum = (pBssInfo->wpaIe[12]) +
                                   (pBssInfo->wpaIe[13] << 8);

                /* If there is only one unicast cipher */
                if (UnicastCipherNum == 1)
                {
                    encAlgoType = pBssInfo->wpaIe[17];
                    //encAlgoType = pBssInfo->wpaIe[11];
                }
                else
                {
                    u16_t ii;
                    u16_t desiredCipher = 0;
                    u16_t IEOffSet = 17;

                    /* Enumerate all the supported unicast cipher */
                    for (ii = 0; ii < UnicastCipherNum; ii++)
                    {
                        if (pBssInfo->wpaIe[IEOffSet+ii*4] > desiredCipher)
                        {
                            desiredCipher = pBssInfo->wpaIe[IEOffSet+ii*4];
                        }
                    }

                    encAlgoType = desiredCipher;
                }

                if ( encAlgoType == 0x02 )
                {
    			    wd->sta.wepStatus = ZM_ENCRYPTION_TKIP;

    			    if ( wd->sta.authMode == ZM_AUTH_MODE_WPA_AUTO )
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA;
                    }
                    else //ZM_AUTH_MODE_WPAPSK_AUTO
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPAPSK;
                    }
                }
                else if ( encAlgoType == 0x04 )
                {
                    wd->sta.wepStatus = ZM_ENCRYPTION_AES;

                    if ( wd->sta.authMode == ZM_AUTH_MODE_WPA_AUTO )
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA;
                    }
                    else //ZM_AUTH_MODE_WPAPSK_AUTO
                    {
                 /*
 * Copyright (C) 2003-2008 Takahiro Hirofuchi
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */


#include "usbip_common.h"
#include "vhci.h"

#define DRIVER_VERSION "1.0"
#define DRIVER_AUTHOR "Takahiro Hirofuchi"
#define DRIVER_DESC "Virtual Host Controller Interface Driver for USB/IP"
#define DRIVER_LICENCE "GPL"
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENCE);



/*
 * TODO
 *	- update root hub emulation
 *	- move the emulation code to userland ?
 *		porting to other operating systems
 *		minimize kernel code
 *	- add suspend/resume code
 *	- clean up everything
 */


/* See usb gadget dummy hcd */


static int vhci_hub_status(struct usb_hcd *hcd, char *buff);
static int vhci_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue,
		u16 wIndex, char *buff, u16 wLength);
static int vhci_urb_enqueue(struct usb_hcd *hcd, struct urb *urb,
							gfp_t mem_flags);
static int vhci_urb_dequeue(struct usb_hcd *hcd, struct urb *urb, int status);
static int vhci_start(struct usb_hcd *vhci_hcd);
static void vhci_stop(struct usb_hcd *hcd);
static int vhci_get_frame_number(struct usb_hcd *hcd);

static const char driver_name[] = "vhci_hcd";
static const char driver_desc[] = "USB/IP Virtual Host Contoroller";

struct vhci_hcd *the_controller;

static const char *bit_desc[] = {
	"CONNECTION",		/*0*/
	"ENABLE",		/*1*/
	"SUSPEND",		/*2*/
	"OVER_CURRENT",		/*3*/
	"RESET",		/*4*/
	"R5",		/*5*/
	"R6",		/*6*/
	"R7",		/*7*/
	"POWER",		/*8*/
	"LOWSPEED",		/*9*/
	"HIGHSPEED",		/*10*/
	"PORT_TEST",		/*11*/
	"INDICATOR",		/*12*/
	"R13",		/*13*/
	"R14",		/*14*/
	"R15",		/*15*/
	"C_CONNECTION",		/*16*/
	"C_ENABLE",		/*17*/
	"C_SUSPEND",		/*18*/
	"C_OVER_CURRENT",	/*19*/
	"C_RESET",		/*20*/
	"R21",		/*21*/
	"R22",		/*22*/
	"R23",		/*23*/
	"R24",		/*24*/
	"R25",		/*25*/
	"R26",		/*26*/
	"R27",		/*27*/
	"R28",		/*28*/
	"R29",		/*29*/
	"R30",		/*30*/
	"R31",		/*31*/
};


static void dump_port_status(u32 status)
{
	int i = 0;

	printk(KERN_DEBUG "status %08x:", status);
	for (i = 0; i < 32; i++) {
		if (status & (1 << i))
			printk(" %s", bit_desc[i]);
	}

	printk("\n");
}



void rh_port_connect(int rhport, enum usb_device_speed speed)
{
	unsigned long	flags;

	dbg_vhci_rh("rh_port_connect %d\n", rhport);

	spin_lock_irqsave(&the_controller->lock, flags);

	the_controller->port_status[rhport] |= USB_PORT_STAT_CONNECTION
		| (1 << USB_PORT_FEAT_C_CONNECTION);

	switch (speed) {
	case USB_SPEED_HIGH:
		the_controller->port_status[rhport] |= USB_PORT_STAT_HIGH_SPEED;
		break;
	case USB_SPEED_LOW:
		the_controller->port_status[rhport] |= USB_PORT_STAT_LOW_SPEED;
		break;
	default:
		break;
	}

	/* spin_lock(&the_controller->vdev[rhport].ud.lock);
	 * the_controller->vdev[rhport].ud.status = VDEV_CONNECT;
	 * spin_unlock(&the_controller->vdev[rhport].ud.lock); */

	the_controller->pending_port = rhport;

	spin_unlock_irqrestore(&the_controller->lock, flags);

	usb_hcd_poll_rh_status(vhci_to_hcd(the_controller));
}

void rh_port_disconnect(int rhport)
{
	unsigned long flags;

	dbg_vhci_rh("rh_port_disconnect %d\n", rhport);

	spin_lock_irqsave(&the_controller->lock, flags);
	/* stop_activity(dum, driver); */
	the_controller->port_status[rhport] &= ~USB_PORT_STAT_CONNECTION;
	the_controller->port_status[rhport] |=
					(1 << USB_PORT_FEAT_C_CONNECTION);


	/* not yet complete the disconnection
	 * spin_lock(&vdev->ud.lock);
	 * vdev->ud.status = VHC_ST_DISCONNECT;
	 * spin_unlock(&vdev->ud.lock); */

	spin_unlock_irqrestore(&the_controller->lock, flags);
}



/*----------------------------------------------------------------------*/

#define PORT_C_MASK \
	((USB_PORT_STAT_C_CONNECTION \
	  | USB_PORT_STAT_C_ENABLE \
	  | USB_PORT_STAT_C_SUSPEND \
	  | USB_PORT_STAT_C_OVERCURRENT \
	  | USB_PORT_STAT_C_RESET) << 16)

/*
 * This function is almostly the same as dummy_hcd.c:dummy_hub_status() without
 * suspend/resume support. But, it is modified to provide multiple ports.
 *
 * @buf: a bitmap to show which port status has been changed.
 *  bit  0: reserved or used for another purpose?
 *  bit  1: the status of port 0 has been changed.
 *  bit  2: the status of port 1 has been changed.
 *  ...
 *  bit  7: the status of port 6 has been changed.
 *  bit  8: the status of port 7 has been changed.
 *  ...
 *  bit 15: the status of port 14 has been changed.
 *
 * So, the maximum number of ports is 31 ( port 0 to port 30) ?
 *
 * The return value is the actual transfered length in byte. If nothing has
 * been changed, return 0. In the case that the number of ports is less than or
 * equal to 6 (VHCI_NPORTS==7), return 1.
 *
 */
static int vhci_hub_status(struct usb_hcd *hcd, char *buf)
{
	struct vhci_hcd	*vhci;
	unsigned long	flags;
	int		retval = 0;

	/* the enough buffer is allocated according to USB_MAXCHILDREN */
	unsigned long	*event_bits = (unsigned long *) buf;
	int		rhport;
	int		changed = 0;


	*event_bits = 0;

	vhci = hcd_to_vhci(hcd);

	spin_lock_irqsave(&vhci->lock, flags);
	if (!test_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags)) {
		dbg_vhci_rh("hw accessible flag in on?\n");
		goto done;
	}

	/* check pseudo status register for each port */
	for (rhport = 0; rhport < VHCI_NPORTS; rhport++) {
		if ((vhci->port_status[rhport] & PORT_C_MASK)) {
			/* The status of a port has been changed, */
			dbg_vhci_rh("port %d is changed\n", rhport);

			*event_bits |= 1 << (rhport + 1);
			changed = 1;
		}
	}

	uinfo("changed %d\n", changed);

	if (hcd->state == HC_STATE_SUSPENDED)
		usb_hcd_resume_root_hub(hcd);

	if (changed)
		retval = 1 + (VHCI_NPORTS / 8);
	else
		retval = 0;

done:
	spin_unlock_irqrestore(&vhci->lock, flags);
	return retval;
}

/* See hub_configure in hub.c */
static inline void hub_descriptor(struct usb_hub_descriptor *desc)
{
	memset(desc, 0, sizeof(*desc));
	desc->bDescriptorType = 0x29;
	desc->bDescLength = 9;
	desc->wHubCharacteristics = (__force __u16)
		(__constant_cpu_to_le16(0x0001));
	desc->bNbrPorts = VHCI_NPORTS;
	desc->bitmap[0] = 0xff;
	desc->bitmap[1] = 0xff;
}

static int vhci_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue,
			    u16 wIndex, char *buf, u16 wLength)
{
	struct vhci_hcd	*dum;
	int             retval = 0;
	unsigned long   flags;
	int		rhport;

	u32 prev_port_status[VHCI_NPORTS];

	if (!test_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags))
		return -ETIMEDOUT;

	/*
	 * NOTE:
	 * wIndex shows the port number and begins from 1.
	 */
	dbg_vhci_rh("typeReq %x wValue %x wIndex %x\n", typeReq, wValue,
								wIndex);
	if (wIndex > VHCI_NPORTS)
		printk(KERN_ERR "%s: invalid port number %d\n", __func__, wIndex);
	rhport = ((__u8)(wIndex & 0x00ff)) - 1;

	dum = hcd_to_vhci(hcd);

	spin_lock_irqsave(&dum->lock, flags);

	/* store old status and compare now and old later */
	if (dbg_flag_vhci_rh) {
		int i = 0;
		for (i = 0; i < VHCI_NPORTS; i++)
			prev_port_status[i] = dum->port_status[i];
	}

	switch (typeReq) {
	case ClearHubFeature:
		dbg_vhci_rh(" ClearHubFeature\n");
		break;
	case ClearPortFeature:
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			if (dum->port_status[rhport] & USB_PORT_STAT_SUSPEND) {
				/* 20msec signaling */
				dum->resuming = 1;
				dum->re_timeout =
					jiffies + msecs_to_jiffies(20);
			}
			break;
		case USB_PORT_FEAT_POWER:
			dbg_vhci_rh(" ClearPortFeature: USB_PORT_FEAT_POWER\n");
			dum->port_status[rhport] = 0;
			/* dum->address = 0; */
			/* dum->hdev = 0; */
			dum->resuming = 0;
			break;
		case USB_PORT_FEAT_C_RESET:
			dbg_vhci_rh(" ClearPortFeature: "
					"USB_PORT_FEAT_C_RESET\n");
			switch (dum->vdev[rhport].speed) {
			case USB_SPEED_HIGH:
				dum->port_status[rhport] |=
						USB_PORT_STAT_HIGH_SPEED;
				break;
			case USB_SPEED_LOW:
				dum->port_status[rhport] |=
						USB_PORT_STAT_LOW_SPEED;
				break;
			default:
				break;
			}
		default:
			dbg_vhci_rh(" ClearPortFeature: default %x\n", wValue);
			dum->port_status[rhport] &= ~(1 << wValue);
		}
		break;
	case GetHubDescriptor:
		dbg_vhci_rh(" GetHubDescriptor\n");
		hub_descriptor((struct usb_hub_descriptor *) buf);
		break;
	case GetHubStatus:
		dbg_vhci_rh(" GetHubStatus\n");
		*(__le32 *) buf = __constant_cpu_to_le32(0);
		break;
	case GetPortStatus:
		dbg_vhci_rh(" GetPortStatus port %x\n", wIndex);
		if (wIndex > VHCI_NPORTS || wIndex < 1) {
			printk(KERN_ERR "%s: invalid port number %d\n",
			       __func__, wIndex);
			retval = -EPIPE;
		}

		/* we do no care of resume. */

		/* whoever resets or resumes must GetPortStatus to
		 * complete it!!
		 *                                   */
		if (dum->resuming && time_after(jiffies, dum->re_timeout)) {
			printk(KERN_ERR "%s: not yet\n", __func__);
			dum->port_status[rhport] |=
					(1 << USB_PORT_FEAT_C_SUSPEND);
			dum->port_status[rhport] &=
					~(1 << USB_PORT_FEAT_SUSPEND);
			dum->resuming = 0;
			dum->re_timeout = 0;
			/* if (dum->driver && dum->driver->resume) {
			 *	spin_unlock (&dum->lock);
			 *	dum->driver->resume (&dum->gadget);
			 *	spin_lock (&dum->lock);
			 * } */
		}

		if ((dum->port_status[rhport] & (1 << USB_PORT_FEAT_RESET)) !=
				0 && time_after(jiffies, dum->re_timeout)) {
			dum->port_status[rhport] |=
						(1 << USB_PORT_FEAT_C_RESET);
			dum->port_status[rhport] &=
						~(1 << USB_PORT_FEAT_RESET);
			dum->re_timeout = 0;

			if (dum->vdev[rhport].ud.status ==
							VDEV_ST_NOTASSIGNED) {
				dbg_vhci_rh(" enable rhport %d (status %u)\n",
						rhport,
						dum->vdev[rhport].ud.status);
				dum->port_status[rhport] |=
							USB_PORT_STAT_ENABLE;
			}
#if 0
			if (dum->driver) {

				dum->port_status[rhport] |=
							USB_PORT_STAT_ENABLE;
				/* give it the best speed we agree on */
				dum->gadget.speed = dum->driver->speed;
				dum->gadget.ep0->maxpacket = 64;
				switch (dum->gadget.speed) {
				case USB_SPEED_HIGH:
					dum->port_status[rhport] |=
					USB_PORT_STAT_HIGH_SPEED;
					break;
				case USB_SPEED_LOW:
					dum->gadget.ep0->maxpacket = 8;
					dum->port_status[rhport] |=
					USB_PORT_STAT_LOW_SPEED;
					break;
				default:
					dum->gadget.speed = USB_SPEED_FULL;
					break;
				}
			}
#endif

		}
		((u16 *) buf)[0] = cpu_to_le16(dum->port_status[rhport]);
		((u16 *) buf)[1] =
				cpu_to_le16(dum->port_status[rhport] >> 16);

		dbg_vhci_rh(" GetPortStatus bye %x %x\n", ((u16 *)buf)[0],
							((u16 *)buf)[1]);
		break;
	case SetHubFeature:
		dbg_vhci_rh(" SetHubFeature\n");
		retval = -EPIPE;
		break;
	case SetPortFeature:
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			dbg_vhci_rh(" SetPortFeature: "
					"USB_PORT_FEAT_SUSPEND\n");
			printk(KERN_ERR "%s: not yet\n", __func__);
#if 0
			dum->port_status[rhport] |=
						(1 << USB_PORT_FEAT_SUSPEND);
			if (dum->driver->suspend) {
				spin_unlock(&dum->lock);
				dum->driver->suspend(&dum->gadget);
				spin_lock(&dum->lock);
			}
#endif
			break;
		case USB_PORT_FEAT_RESET:
			dbg_vhci_rh(" SetPortFeature: USB_PORT_FEAT_RESET\n");
			/* if it's already running, disconnect first */
			if (dum->port_status[rhport] & USB_PORT_STAT_ENABLE) {
				dum->port_status[rhport] &=
						~(USB_PORT_STAT_ENABLE |
						  USB_PORT_STAT_LOW_SPEED |
						  USB_PORT_STAT_HIGH_SPEED);
#if 0
				if (dum->driver) {
					dev_dbg(hardware, "disconnect\n");
					stop_activity(dum, dum->driver);
				}
#endif

				/* FIXME test that code path! */
			}
			/* 50msec reset signaling */
			dum->re_timeout = jiffies + msecs_to_jiffies(50);

			/* FALLTHROUGH */
		default:
			dbg_vhci_rh(" SetPortFeature: default %d\n", wValue);
			dum->port_status[rhport] |= (1 << wValue);
		}
		break;

	default:
		printk(KERN_ERR "%s: default: no such request\n", __func__);
		/* dev_dbg (hardware,
		 *		"hub control req%04x v%04x i%04x l%d\n",
		 *		typeReq, wValue, wIndex, wLength); */

		/* "protocol stall" on error */
		retval = -EPIPE;
	}

	if (dbg_flag_vhci_rh) {
		printk(KERN_DEBUG "port %d\n", rhport);
		dump_port_status(prev_port_status[rhport]);
		dump_port_status(dum->port_status[rhport]);
	}
	dbg_vhci_rh(" bye\n");

	spin_unlock_irqrestore(&dum->lock, flags);

	return retval;
}



/*----------------------------------------------------------------------*/

static struct vhci_device *get_vdev(struct usb_device *udev)
{
	int i;

	if (!udev)
		return NULL;

	for (i = 0; i < VHCI_NPORTS; i++)
		if (the_controller->vdev[i].udev == udev)
			return port_to_vdev(i);

	return NULL;
}

static void vhci_tx_urb(struct urb *urb)
{
	struct vhci_device *vdev = get_vdev(urb->dev);
	struct vhci_priv *priv;
	unsigned long flag;

	if (!vdev) {
		err("could not get virtual device");
		/* BUG(); */
		return;
	}

	spin_lock_irqsave(&vdev->priv_lock, flag);

	priv = kzalloc(sizeof(struct vhci_priv), GFP_ATOMIC);
	if (!priv) {
		dev_err(&urb->dev->dev, "malloc vhci_priv\n");
		spin_unlock_irqrestore(&vdev->priv_lock, flag);
		usbip_event_add(&vdev->ud, VDEV_EVENT_ERROR_MALLOC);
		return;
	}

	priv->seqnum = atomic_inc_return(&the_controller->seqnum);
	if (priv->seqnum == 0xffff)
		uinfo("seqnum max\n");

	priv->vdev = vdev;
	priv->urb = urb;

	urb->hcpriv = (void *) priv;


	list_add_tail(&priv->list, &vdev->priv_tx);

	wake_up(&vdev->waitq_tx);
	spin_unlock_irqrestore(&vdev->priv_lock, flag);
}

static int vhci_urb_enqueue(struct usb_hcd *hcd, struct urb *urb,
			    gfp_t mem_flags)
{
	struct device *dev = &urb->dev->dev;
	int ret = 0;
	unsigned long flags;

	dbg_vhci_hc("enter, usb_hcd %p urb %p mem_flags %d\n",
		    hcd, urb, mem_flags);

	/* patch to usb_sg_init() is in 2.5.60 */
	BUG_ON(!urb->transfer_buffer && urb->transfer_buffer_length);

	spin_lock_irqsave(&the_controller->lock, flags);

	/* check HC is active or not */
	if (!HC_IS_RUNNING(hcd->state)) {
		dev_err(dev, "HC is not running\n");
		spin_unlock_irqrestore(&the_controller->lock, flags);
		return -ENODEV;
	}

	if (urb->status != -EINPROGRESS) {
		dev_err(dev, "URB already unlinked!, status %d\n", urb->status);
		spin_unlock_irqrestore(&the_controller->lock, flags);
		return urb->status;
	}

	ret = usb_hcd_link_urb_to_ep(hcd, urb);
	if (ret)
		goto no_need_unlink;

	/*
	 * The enumelation process is as follows;
	 *
	 *  1. Get_Descriptor request to DevAddrs(0) EndPoint(0)
	 *     to get max packet length of default pipe
	 *
	 *  2. Set_Address request to DevAddr(0) EndPoint(0)
	 *
	 */

	if (usb_pipedevice(urb->pipe) == 0) {
		__u8 type = usb_pipetype(urb->pipe);
		struct usb_ctrlrequest *ctrlreq =
				(struct usb_ctrlrequest *) urb->setup_packet;
		struct vhci_device *vdev =
				port_to_vdev(the_controller->pending_port);

		if (type != PIPE_CONTROL || !ctrlreq) {
			dev_err(dev, "invalid request to devnum 0\n");
			ret = EINVAL;
			goto no_need_xmit;
		}

		switch (ctrlreq->bRequest) {
		case USB_REQ_SET_ADDRESS:
			/* set_address may come when a device is reset */
			dev_info(dev, "SetAddress Request (%d) to port %d\n",
				 ctrlreq->wValue, vdev->rhport);

			vdev->udev = urb->dev;

			spin_lock(&vdev->ud.lock);
			vdev->ud.status = VDEV_ST_USED;
			spin_unlock(&vdev->ud.lock);

			if (urb->status == -EINPROGRESS) {
				/* This request is successfully completed. */
				/* If not -EINPROGRESS, possibly unlinked. */
				urb->status = 0;
			}

			goto no_need_xmit;

		case USB_REQ_GET_DESCRIPTOR:
			if (ctrlreq->wValue == (USB_DT_DEVICE << 8))
				dbg_vhci_hc("Not yet?: "
						"Get_Descriptor to device 0 "
						"(get max pipe size)\n");

			/* FIXME: reference count? (usb_get_dev()) */
			vdev->udev = urb->dev;
			goto out;

		default:
			/* NOT REACHED */
			dev_err(dev, "invalid request to devnum 0 bRequest %u, "
				"wValue %u\n", ctrlreq->bRequest,
				ctrlreq->wValue);
			ret =  -EINVAL;
			goto no_need_xmit;
		}

	}

out:
	vhci_tx_urb(urb);

	spin_unlock_irqrestore(&the_controller->lock, flags);

	return 0;

no_need_xmit:
	usb_hcd_unlink_urb_from_ep(hcd, urb);
no_need_unlink:
	spin_unlock_irqrestore(&the_controller->lock, flags);

	usb_hcd_giveback_urb(vhci_to_hcd(the_controller), urb, urb->status);

	return 0;
}

/*
 * vhci_rx gives back the urb after receiving the reply of the urb.  If an
 * unlink pdu is sent or not, vhci_rx receives a normal return pdu and gives
 * back its urb. For the driver unlinking the urb, the content of the urb is
 * not important, but the calling to its completion handler is important; the
 * completion of unlinking is notified by the completion handler.
 *
 *
 * CLIENT SIDE
 *
 * - When vhci_hcd receives RET_SUBMIT,
 *
 *	- case 1a). the urb of the pdu is not unlinking.
 *		- normal case
 *		=> just give back the urb
 *
 *	- case 1b). the urb of the pdu is unlinking.
 *		- usbip.ko will return a reply of the unlinking request.
 *		=> give back the urb now and go to case 2b).
 *
 * - When vhci_hcd receives RET_UNLINK,
 *
 *	- case 2a). a submit request is still pending in vhci_hcd.
 *		- urb was really pending in usbip.ko and urb_unlink_urb() was
 *		  completed there.
 *		=> free a pending submit request
 *		=> notify unlink completeness by giving back the urb
 *
 *	- case 2b). a submit request is *not* pending in vhci_hcd.
 *		- urb was already given back to the core driver.
 *		=> do not give back the urb
 *
 *
 * SERVER SIDE
 *
 * - When usbip receives CMD_UNLINK,
 *
 *	- case 3a). the urb of the unlink request is now in submission.
 *		=> do usb_unlink_urb().
 *		=> after the unlink is completed, send RET_UNLINK.
 *
 *	- case 3b). the urb of the unlink request is not in submission.
 *		- may be already completed or never be received
 *		=> send RET_UNLINK
 *
 */
static int vhci_urb_dequeue(struct usb_hcd *hcd, struct urb *urb, int status)
{
	unsigned long flags;
	struct vhci_priv *priv;
	struct vhci_device *vdev;

	uinfo("vhci_hcd: dequeue a urb %p\n", urb);


	spin_lock_irqsave(&the_controller->lock, flags);

	priv = urb->hcpriv;
	if (!priv) {
		/* URB was never linked! or will be soon given back by
		 * vhci_rx. */
		spin_unlock_irqrestore(&the_controller->lock, flags);
		return 0;
	}

	{
		int ret = 0;
		ret = usb_hcd_check_unlink_urb(hcd, urb, status);
		if (ret) {
			spin_unlock_irqrestore(&the_controller->lock, flags);
			return 0;
		}
	}

	 /* send unlink request here? */
	vdev = priv->vdev;

	if (!vdev->ud.tcp_socket) {
		/* tcp connection is closed */
		unsigned long flags2;

		spin_lock_irqsave(&vdev->priv_lock, flags2);

		uinfo("vhci_hcd: device %p seems to be disconnected\n", vdev);
		list_del(&priv->list);
		kfree(priv);
		urb->hcpriv = NULL;

		spin_unlock_irqrestore(&vdev->priv_lock, flags2);

	} else {
		/* tcp connection is alive */
		unsigned long flags2;
		struct vhci_unlink *unlink;

		spin_lock_irqsave(&vdev->priv_lock, flags2);

		/* setup CMD_UNLINK pdu */
		unlink = kzalloc(sizeof(struct vhci_unlink), GFP_ATOMIC);
		if (!unlink) {
			uerr("malloc vhci_unlink\n");
			spin_unlock_irqrestore(&vdev->priv_lock, flags2);
			spin_unlock_irqrestore(&the_controller->lock, flags);
			usbip_event_add(&vdev->ud, VDEV_EVENT_ERROR_MALLOC);
			return -ENOMEM;
		}

		unlink->seqnum = atomic_inc_return(&the_controller->seqnum);
		if (unlink->seqnum == 0xffff)
			uinfo("seqnum max\n");

		unlink->unlink_seqnum = priv->seqnum;

		uinfo("vhci_hcd: device %p seems to be still connected\n",
									vdev);

		/* send cmd_unlink and try to cancel the pending URB in the
		 * peer */
		list_add_tail(&unlink->list, &vdev->unlink_tx);
		wake_up(&vdev->waitq_tx);

		spin_unlock_irqrestore(&vdev->priv_lock, flags2);
	}


	/*
	 * If tcp connection is alive, we have sent CMD_UNLINK.
	 * vhci_rx will receive RET_UNLINK and give back the URB.
	 * Otherwise, we give back it here.
	 */
	if (!vdev->ud.tcp_socket) {
		/* tcp connection is closed */
		uinfo("vhci_hcd: vhci_urb_dequeue() gives back urb %p\n", urb);

		usb_hcd_unlink_urb_from_ep(hcd, urb);

		spin_unlock_irqrestore(&the_controller->lock, flags);
		usb_hcd_giveback_urb(vhci_to_hcd(the_controller), urb,
								urb->status);
		spin_lock_irqsave(&the_controller->lock, flags);
	}

	spin_unlock_irqrestore(&the_controller->lock, flags);

	dbg_vhci_hc("leave\n");
	return 0;
}


static void vhci_device_unlink_cleanup(struct vhci_device *vdev)
{
	struct vhci_unlink *unlink, *tmp;

	spin_lock(&vdev->priv_lock);

	list_for_each_entry_safe(unlink, tmp, &vdev->unlink_tx, list) {
		list_del(&unlink->list);
		kfree(unlink);
	}

	list_for_each_entry_safe(unlink, tmp, &vdev->unlink_rx, list) {
		list_del(&unlink->list);
		kfree(unlink);
	}

	spin_unlock(&vdev->priv_lock);
}

/*
 * The important thing is that only one context begins cleanup.
 * This is why error handling and cleanup become simple.
 * We do not want to consider race condition as possible.
 */
static void vhci_shutdown_connection(struct usbip_device *ud)
{
	struct vhci_device *vdev = container_of(ud, struct vhci_device, ud);

	/* need this? see stub_dev.c */
	if (ud->tcp_socket) {
		udbg("shutdown tcp_socket %p\n", ud->tcp_socket);
		kernel_sock_shutdown(ud->tcp_socket, SHUT_RDWR);
	}

	usbip_stop_threads(&vdev->ud);
	uinfo("stop threads\n");

	/* active connection is closed */
	if (vdev->ud.tcp_socket != NULL) {
		sock_release(vdev->ud.tcp_socket);
		vdev->ud.tcp_socket = NULL;
	}
	uinfo("release socket\n");

	vhci_device_unlink_cleanup(vdev);

	/*
	 * rh_port_disconnect() is a trigger of ...
	 *   usb_disable_device():
	 *	disable all the endpoints for a USB device.
	 *   usb_disable_endpoint():
	 *	disable endpoints. pending urbs are unlinked(dequeued).
	 *
	 * NOTE: After calling rh_port_disconnect(), the USB device drivers of a
	 * deteched device should release used urbs in a cleanup function(i.e.
	 * xxx_disconnect()). Therefore, vhci_hcd does not need to release
	 * pushed urbs and their private data in this function.
	 *
	 * NOTE: vhci_dequeue() must be considered carefully. When shutdowning
	 * a connection, vhci_shutdown_connection() expects vhci_dequeue()
	 * gives back pushed urbs and frees their private data by request of
	 * the cleanup function of a USB driver. When unlinking a urb with an
	 * active connection, vhci_dequeue() does not give back the urb which
	 * is actually given back by vhci_rx after receiving its return pdu.
	 *
	 */
	rh_port_disconnect(vdev->rhport);

	uinfo("disconnect device\n");
}


static void vhci_device_reset(struct usbip_device *ud)
{
	struct vhci_device *vdev = container_of(ud, struct vhci_device, ud);

	spin_lock(&ud->lock);

	vdev->speed  = 0;
	vdev->devid  = 0;

	ud->tcp_socket = NULL;

	ud->status = VDEV_ST_NULL;

	spin_unlock(&ud->lock);
}

static void vhci_device_unusable(struct usbip_device *ud)
{
	spin_lock(&ud->lock);

	ud->status = VDEV_ST_ERROR;

	spin_unlock(&ud->lock);
}

static void vhci_device_init(struct vhci_device *vdev)
{
	memset(vdev, 0, sizeof(*vdev));

	usbip_task_init(&vdev->ud.tcp_rx, "vhci_rx", vhci_rx_loop);
	usbip_task_init(&vdev->ud.tcp_tx, "vhci_tx", vhci_tx_loop);

	vdev->ud.side   = USBIP_VHCI;
	vdev->ud.status = VDEV_ST_NULL;
	/* vdev->ud.lock   = SPIN_LOCK_UNLOCKED; */
	spin_lock_init(&vdev->ud.lock);

	INIT_LIST_HEAD(&vdev->priv_rx);
	INIT_LIST_HEAD(&vdev->priv_tx);
	INIT_LIST_HEAD(&vdev->unlink_tx);
	INIT_LIST_HEAD(&vdev->unlink_rx);
	/* vdev->priv_lock = SPIN_LOCK_UNLOCKED; */
	spin_lock_init(&vdev->priv_lock);

	init_waitqueue_head(&vdev->waitq_tx);

	vdev->ud.eh_ops.shutdown = vhci_shutdown_connection;
	vdev->ud.eh_ops.reset = vhci_device_reset;
	vdev->ud.eh_ops.unusable = vhci_device_unusable;

	usbip_start_eh(&vdev->ud);
}


/*----------------------------------------------------------------------*/

static int vhci_start(struct usb_hcd *hcd)
{
	struct vhci_hcd *vhci = hcd_to_vhci(hcd);
	int rhport;
	int err = 0;

	dbg_vhci_hc("enter vhci_start\n");


	/* initialize private data of usb_hcd */

	for (rhport = 0; rhport < VHCI_NPORTS; rhport++) {
		struct vhci_device *vdev = &vhci->vdev[rhport];
		vhci_device_init(vdev);
		vdev->rhport = rhport;
	}

	atomic_set(&vhci->seqnum, 0);
	spin_lock_init(&vhci->lock);



	hcd->power_budget = 0; /* no limit */
	hcd->state  = HC_STATE_RUNNING;
	hcd->uses_new_polling = 1;


	/* vhci_hcd is now ready to be controlled through sysfs */
	err = sysfs_create_group(&vhci_dev(vhci)->kobj, &dev_attr_group);
	if (err) {
		uerr("create sysfs files\n");
		return err;
	}

	return 0;
}

static void vhci_stop(struct usb_hcd *hcd)
{
	struct vhci_hcd *vhci = hcd_to_vhci(hcd);
	int rhport = 0;

	dbg_vhci_hc("stop VHCI controller\n");


	/* 1. remove the userland interface of vhci_hcd */
	sysfs_remove_group(&vhci_dev(vhci)->kobj, &dev_attr_group);

	/* 2. shutdown all the ports of vhci_hcd */
	for (rhport = 0 ; rhport < VHCI_NPORTS; rhport++) {
		struct vhci_device *vdev = &vhci->vdev[rhport];

		usbip_event_add(&vdev->ud, VDEV_EVENT_REMOVED);
		usbip_stop_eh(&vdev->ud);
	}


	uinfo("vhci_stop done\n");
}

/*----------------------------------------------------------------------*/

static int vhci_get_frame_number(struct usb_hcd *hcd)
{
	uerr("Not yet implemented\n");
	return 0;
}


#ifdef CONFIG_PM

/* FIXME: suspend/resume */
static int vhci_bus_suspend(struct usb_hcd *hcd)
{
	struct vhci_hcd *vhci = hcd_to_vhci(hcd);

	dev_dbg(&hcd->self.root_hub->dev, "%s\n", __func__);

	spin_lock_irq(&vhci->lock);
	/* vhci->rh_state = DUMMY_RH_SUSPENDED;
	 * set_link_state(vhci); */
	hcd->state = HC_STATE_SUSPENDED;
	spin_unlock_irq(&vhci->lock);

	return 0;
}

static int vhci_bus_resume(struct usb_hcd *hcd)
{
	struct vhci_hcd *vhci = hcd_to_vhci(hcd);
	int rc = 0;

	dev_dbg(&hcd->self.root_hub->dev, "%s\n", __func__);

	spin_lock_irq(&vhci->lock);
	if (!test_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags)) {
		rc = -ESHUTDOWN;
	} else {
		/* vhci->rh_state = DUMMY_RH_RUNNING;
		 * set_link_state(vhci);
		 * if (!list_empty(&vhci->urbp_list))
		 *	mod_timer(&vhci->timer, jiffies); */
		hcd->state = HC_STATE_RUNNING;
	}
	spin_unlock_irq(&vhci->lock);
	return rc;

	return 0;
}

#else

#define vhci_bus_suspend      NULL
#define vhci_bus_resume       NULL
#endif



static struct hc_driver vhci_hc_driver = {
	.description	= driver_name,
	.product_desc	= driver_desc,
	.hcd_priv_size	= sizeof(struct vhci_hcd),

	.flags		= HCD_USB2,

	.start		= vhci_start,
	.stop 		= vhci_stop,

	.urb_enqueue	= vhci_urb_enqueue,
	.urb_dequeue	= vhci_urb_dequeue,

	.get_frame_number = vhci_get_frame_number,

	.hub_status_data = vhci_hub_status,
	.hub_control    = vhci_hub_control,
	.bus_suspend	= vhci_bus_suspend,
	.bus_resume	= vhci_bus_resume,
};

static int vhci_hcd_probe(struct platform_device *pdev)
{
	struct usb_hcd		*hcd;
	int			ret;

	uinfo("proving...\n");

	dbg_vhci_hc("name %s id %d\n", pdev->name, pdev->id);

	/* will be removed */
	if (pdev->dev.dma_mask) {
		dev_info(&pdev->dev, "vhci_hcd DMA not supported\n");
		return -EINVAL;
	}

	/*
	 * Allocate and initialize hcd.
	 * Our private data is also allocated automatically.
	 */
	hcd = usb_create_hcd(&vhci_hc_driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		uerr("create hcd failed\n");
		return -ENOMEM;
	}


	/* this is private data for vhci_hcd */
	the_controller = hcd_to_vhci(hcd);

	/*
	 * Finish generic HCD structure initialization and register.
	 * Call the driver's reset() and start() routines.
	 */
	ret = usb_add_hcd(hcd, 0, 0);
	if (ret != 0) {
		uerr("usb_add_hcd failed %d\n", ret);
		usb_put_hcd(hcd);
		the_controller = NULL;
		return ret;
	}


	dbg_vhci_hc("bye\n");
	return 0;
}


static int vhci_hcd_remove(struct platform_device *pdev)
{
	struct usb_hcd	*hcd;

	hcd = platform_get_drvdata(pdev);
	if (!hcd)
		return 0;

	/*
	 * Disconnects the root hub,
	 * then reverses the effects of usb_add_hcd(),
	 * invoking the HCD's stop() methods.
	 */
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	the_controller = NULL;


	return 0;
}



#ifdef CONFIG_PM

/* what should happen for USB/IP under suspend/resume? */
static int vhci_hcd_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct usb_hcd *hcd;
	int rhport = 0;
	int connected = 0;
	int ret = 0;

	dev_dbg(&pdev->dev, "%s\n", __func__);

	hcd = platform_get_drvdata(pdev);

	spin_lock(&the_controller->lock);

	for (rhport = 0; rhport < VHCI_NPORTS; rhport++)
		if (the_controller->port_status[rhport] &
						USB_PORT_STAT_CONNECTION)
			connected += 1;

	spin_unlock(&the_controller->lock);

	if (connected > 0) {
		uinfo("We have %d active connection%s. Do not suspend.\n",
				connected, (connected == 1 ? "" : "s"));
		ret =  -EBUSY;
	} else {
		uinfo("suspend vhci_hcd");
		clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	}

	return ret;
}

static int vhci_hcd_resume(struct platform_device *pdev)
{
	struct usb_hcd *hcd;

	dev_dbg(&pdev->dev, "%s\n", __func__);

	hcd = platform_get_drvdata(pdev);
	set_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	usb_hcd_poll_rh_status(hcd);

	return 0;
}

#else

#define vhci_hcd_suspend	NULL
#define vhci_hcd_resume		NULL

#endif


static struct platform_driver vhci_driver = {
	.probe	= vhci_hcd_probe,
	.remove	= __devexit_p(vhci_hcd_remove),
	.suspend = vhci_hcd_suspend,
	.resume	= vhci_hcd_resume,
	.driver	= {
		.name = (char *) driver_name,
		.owner = THIS_MODULE,
	},
};

/*----------------------------------------------------------------------*/

/*
 * The VHCI 'device' is 'virtual'; not a real plug&play hardware.
 * We need to add this virtual device as a platform device arbitrarily:
 *	1. platform_device_register()
 */
static void the_pdev_release(struct device *dev)
{
	return;
}

static struct platform_device the_pdev = {
	/* should be the same name as driver_name */
	.name = (char *) driver_name,
	.id = -1,
	.dev = {
		/* .driver = &vhci_driver, */
		.release = the_pdev_release,
	},
};

static int __init vhci_init(void)
{
	int ret;

	dbg_vhci_hc("enter\n");
	if (usb_disabled())
		return -ENODEV;

	printk(KERN_INFO KBUILD_MODNAME ": %s, %s\n", driver_name,
	       DRIVER_VERSION);

	ret = platform_driver_register(&vhci_driver);
	if (ret < 0)
		goto err_driver_register;

	ret = platform_device_register(&the_pdev);
	if (ret < 0)
		goto err_platform_device_register;

	dbg_vhci_hc("bye\n");
	return ret;

	/* error occurred */
err_platform_device_register:
	platform_driver_unregister(&vhci_driver);

err_driver_register:
	dbg_vhci_hc("bye\n");
	return ret;
}
module_init(vhci_init);

static void __exit vhci_cleanup(void)
{
	dbg_vhci_hc("enter\n");

	platform_device_unregister(&the_pdev);
	platform_driver_unregister(&vhci_driver);

	dbg_vhci_hc("bye\n");
}
module_exit(vhci_cleanup);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              zm_debug_msg0("get the first MIC failure");
        //zfTimerSchedule(dev, ZM_EVENT_CM_TIMER, ZM_TICK_CM_TIMEOUT);

        /* Timer Resolution on WinXP is 15/16 ms  */
        /* Decrease Time offset for <XP> Counter Measure */
        zfTimerSchedule(dev, ZM_EVENT_CM_TIMER, ZM_TICK_CM_TIMEOUT - ZM_TICK_CM_TIMEOUT_OFFSET);
    }
    else if ( wd->sta.cmMicFailureCount == 2 )
    {
        zm_debug_msg0("get the second MIC failure");
        /* reserve 2 second for OS to send MIC failure report to AP */
        wd->sta.cmDisallowSsidLength = wd->sta.ssidLen;
        zfMemoryCopy(wd->sta.cmDisallowSsid, wd->sta.ssid, wd->sta.ssidLen);
        //wd->sta.cmMicFailureCount = 0;
        zfTimerCancel(dev, ZM_EVENT_CM_TIMER);
        //zfTimerSchedule(dev, ZM_EVENT_CM_DISCONNECT, ZM_TICK_CM_DISCONNECT);

        /* Timer Resolution on WinXP is 15/16 ms  */
        /* Decrease Time offset for <XP> Counter Measure */
        zfTimerSchedule(dev, ZM_EVENT_CM_DISCONNECT, ZM_TICK_CM_DISCONNECT - ZM_TICK_CM_DISCONNECT_OFFSET);
    }
    else
    {
        micNotify = 0;
    }

    zmw_leave_critical_section(dev);

    if (micNotify == 1)
    {
        da0 = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_A1_OFFSET);
        if ( da0 & 0x01 )
        {
            if (wd->zfcbMicFailureNotify != NULL)
            {
                wd->zfcbMicFailureNotify(dev, wd->sta.bssid, ZM_MIC_GROUP_ERROR);
            }
        }
        else
        {
            if (wd->zfcbMicFailureNotify != NULL)
            {
                wd->zfcbMicFailureNotify(dev, wd->sta.bssid, ZM_MIC_PAIRWISE_ERROR);
            }
        }
    }
}


u8_t zfStaBlockWlanScan(zdev_t* dev)
{
    u8_t   ret=FALSE;

    zmw_get_wlan_dev(dev);

    if ( wd->sta.bChannelScan )
    {
        return TRUE;
    }

    return ret;
}

void zfStaResetStatus(zdev_t* dev, u8_t bInit)
{
    u8_t   i;

    zmw_get_wlan_dev(dev);

    zfHpDisableBeacon(dev);

    wd->dtim = 1;
    wd->sta.capability[0] = 0x01;
    wd->sta.capability[1] = 0x00;
    /* 802.11h */
    if (wd->sta.DFSEnable || wd->sta.TPCEnable)
        wd->sta.capability[1] |= ZM_BIT_0;

    /* release queued packets */
    for(i=0; i<wd->sta.ibssPSDataCount; i++)
    {
        zfwBufFree(dev, wd->sta.ibssPSDataQueue[i], 0);
    }

    for(i=0; i<wd->sta.staPSDataCount; i++)
    {
        zfwBufFree(dev, wd->sta.staPSDataQueue[i], 0);
    }

    wd->sta.ibssPSDataCount = 0;
    wd->sta.staPSDataCount = 0;
    zfZeroMemory((u8_t*) &wd->sta.staPSList, sizeof(struct zsStaPSList));

    wd->sta.wmeConnected = 0;
    wd->sta.psMgr.tempWakeUp = 0;
    wd->sta.qosInfo = 0;
    zfQueueFlush(dev, wd->sta.uapsdQ);

    return;

}

void zfStaIbssMonitoring(zdev_t* dev, u8_t reset)
{
    u16_t i;
    u16_t oppositeCount;
    struct zsPartnerNotifyEvent event;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    //zm_debug_msg1("zfStaIbssMonitoring %d", wd->sta.oppositeCount);

    zmw_enter_critical_section(dev);

    if ( wd->sta.oppositeCount == 0 )
    {
        goto done;
    }

    if ( wd->sta.bChannelScan )
    {
        goto done;
    }

    oppositeCount = wd->sta.oppositeCount;

    for(i=0; i < ZM_MAX_OPPOSITE_COUNT; i++)
    {
        if ( oppositeCount == 0 )
        {
            break;
        }

        if ( reset )
        {
            wd->sta.oppositeInfo[i].valid = 0;
        }

        if ( wd->sta.oppositeInfo[i].valid == 0 )
        {
            continue;
        }

        oppositeCount--;

        if ( wd->sta.oppositeInfo[i].aliveCounter )
        {
            zm_debug_msg1("Setting alive to ", wd->sta.oppositeInfo[i].aliveCounter);

            zmw_leave_critical_section(dev);

            if ( wd->sta.oppositeInfo[i].aliveCounter != ZM_IBSS_PEER_ALIVE_COUNTER )
            {
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_PROBEREQ,
                              (u16_t*)wd->sta.oppositeInfo[i].macAddr, 1, 0, 0);
            }

            zmw_enter_critical_section(dev);
            wd->sta.oppositeInfo[i].aliveCounter--;
        }
        else
        {
            zm_debug_msg                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
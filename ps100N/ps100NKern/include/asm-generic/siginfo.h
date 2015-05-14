
	}

	mutex_unlock(&uwb_rc->ies_mutex);

	return result;
}
EXPORT_SYMBOL_GPL(uwb_rc_ie_add);


/*
 * Remove an IE from internal cache
 *
 * We are dealing with our internal IE cache so no need to verify that the
 * IEs are valid (it has been done already).
 *
 * Should be called with ies_mutex held
 *
 * We do not break out once an IE is found in the cache. It is currently
 * possible to have more than one IE with the same ID included in the
 * beacon. We don't reallocate, we just mark the size smaller.
 */
static
void uwb_rc_ie_cache_rm(struct uwb_rc *uwb_rc, enum uwb_ie to_remove)
{
	struct uwb_ie_hdr *ie;
	size_t len = le16_to_cpu(uwb_rc->ies->wIELength);
	void *ptr;
	size_t size;

	ptr = uwb_rc->ies->IEData;
	size = len;
	for (;;) {
		ie = uwb_ie_next(&ptr, &size);
		if (!ie)
			break;
		if (ie->element_id == to_remove) {
			len -= sizeof(struct uwb_ie_hdr) + ie->length;
			memmove(ie, ptr, size);
			ptr = ie;
		}
	}
	uwb_rc->ies->wIELength = cpu_to_le16(len);
}


/**
 * uwb_rc_ie_rm - remove an IE from the radio controller's beacon
 * @uwb_rc: the radio controller.
 * @element_id: the element ID of the IE to remove.
 *
 * Only IEs previously added with uwb_rc_ie_add() may be removed.
 *
 * Returns 0 on success; or -ve the SET-IE command to the radio
 * controller failed.
 */
int uwb_rc_ie_rm(struct uwb_rc *uwb_rc, enum uwb_ie element_id)
{
	int result = 0;

	mutex_lock(&uwb_rc->ies_mutex);

	uwb_rc_ie_cache_rm(uwb_rc, element_id);

	if (uwb_rc->beaconing != -1)
		result = uwb_rc_set_ie(uwb_rc, uwb_rc->ies);

	mutex_unlock(&uwb_rc->ies_mutex);

	return result;
}
EXPORT_SYMBOL_GPL(uwb_rc_ie_rm);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      /*
 * drivers/usb/driver.c - most of the driver model stuff for usb
 *
 * (C) Copyright 2005 Greg Kroah-Hartman <gregkh@suse.de>
 *
 * based on drivers/usb/usb.c which had the following copyrights:
 *	(C) Copyright Linus Torvalds 1999
 *	(C) Copyright Johannes Erdfelt 1999-2001
 *	(C) Copyright Andreas Gal 1999
 *	(C) Copyright Gregory P. Smith 1999
 *	(C) Copyright Deti Fliegl 1999 (new USB architecture)
 *	(C) Copyright Randy Dunlap 2000
 *	(C) Copyright David Brownell 2000-2004
 *	(C) Copyright Yggdrasil Computing, Inc. 2000
 *		(usb_device_id matching changes by Adam J. Richter)
 *	(C) Copyright Greg Kroah-Hartman 2002-2003
 *
 * NOTE! This is not actually a driver at all, rather this is
 * just a collection of helper routines that implement the
 * matching, probing, releasing, suspending and resuming for
 * real drivers.
 *
 */

#include <linux/device.h>
#include <linux/usb.h>
#include <linux/usb/quirks.h>
#include <linux/workqueue.h>
#include "hcd.h"
#include "usb.h"


#ifdef CONFIG_HOTPLUG

/*
 * Adds a new dynamic USBdevice ID to this driver,
 * and cause the driver to probe for all devices again.
 */
ssize_t usb_store_new_id(struct usb_dynids *dynids,
			 struct device_driver *driver,
			 const char *buf, size_t count)
{
	struct usb_dynid *dynid;
	u32 idVendor = 0;
	u32 idProduct = 0;
	int fields = 0;
	int retval = 0;

	fields = sscanf(buf, "%x %x", &idVendor, &idProduct);
	if (fields < 2)
		return -EINVAL;

	dynid = kzalloc(sizeof(*dynid), GFP_KERNEL);
	if (!dynid)
		return -ENOMEM;

	INIT_LIST_HEAD(&dynid->node);
	dynid->id.idVendor = idVendor;
	dynid->id.idProduct = idProduct;
	dynid->id.match_flags = USB_DEVICE_ID_MATCH_DEVICE;

	spin_lock(&dynids->lock);
	list_add_tail(&dynid->node, &dynids->list);
	spin_unlock(&dynids->lock);

	if (get_driver(driver)) {
		retval = driver_attach(driver);
		put_driver(driver);
	}

	if (retval)
		return retval;
	return count;
}
EXPORT_SYMBOL_GPL(usb_store_new_id);

static ssize_t store_new_id(struct device_driver *driver,
			    const char *buf, size_t count)
{
	struct usb_driver *usb_drv = to_usb_driver(driver);

	return usb_store_new_id(&usb_drv->dynids, driver, buf, count);
}
static DRIVER_ATTR(new_id, S_IWUSR, NULL, store_new_id);

static int usb_create_newid_file(struct usb_driver *usb_drv)
{
	int error = 0;

	if (usb_drv->no_dynamic_id)
		goto exit;

	if (usb_drv->probe != NULL)
		error = driver_create_file(&usb_drv->drvwrap.driver,
					   &driver_attr_new_id);
exit:
	return error;
}

static void usb_remove_newid_file(struct usb_driver *usb_drv)
{
	if (usb_drv->no_dynamic_id)
		return;

	if (usb_drv->probe != NULL)
		driver_remove_file(&usb_drv->drvwrap.driver,
				   &driver_attr_new_id);
}

static void usb_free_dynids(struct usb_driver *usb_drv)
{
	struct usb_dynid *dynid, *n;

	spin_lock(&usb_drv->dynids.lock);
	list_for_each_entry_safe(dynid, n, &usb_drv->dynids.list, node) {
		list_del(&dynid->node);
		kfree(dynid);
	}
	spin_unlock(&usb_drv->dynids.lock);
}
#else
static inline int usb_create_newid_file(struct usb_driver *usb_drv)
{
	return 0;
}

static void usb_remove_newid_file(struct usb_driver *usb_drv)
{
}

static inline void usb_free_dynids(struct usb_driver *usb_drv)
{
}
#endif

static const struct usb_device_id *usb_match_dynamic_id(struct usb_interface *intf,
							struct usb_driver *drv)
{
	struct usb_dynid *dynid;

	spin_lock(&drv->dynids.lock);
	list_for_each_entry(dynid, &drv->dynids.list, node) {
		if (usb_match_one_id(intf, &dynid->id)) {
			spin_unlock(&drv->dynids.lock);
			return &dynid->id;
		}
	}
	spin_unlock(&drv->dynids.lock);
	return NULL;
}


/* called from driver core with dev locked */
static int usb_probe_device(struct device *dev)
{
	struct usb_device_driver *udriver = to_usb_device_driver(dev->driver);
	struct usb_device *udev = to_usb_device(dev);
	int error = -ENODEV;

	dev_dbg(dev, "%s\n", __func__);

	/* TODO: Add real matching code */

	/* The device should always appear to be in use
	 * unless the driver suports autosuspend.
	 */
	udev->pm_usage_cnt = !(udriver->supports_autosuspend);

	error = udriver->probe(udev);
	return error;
}

/* called from driver core with dev locked */
static int usb_unbind_device(struct device *dev)
{
	struct usb_device_driver *udriver = to_usb_device_driver(dev->driver);

	udriver->disconnect(to_usb_device(dev));
	return 0;
}

/*
 * Cancel any pending scheduled resets
 *
 * [see usb_queue_reset_device()]
 *
 * Called after unconfiguring / when releasing interfaces. See
 * comments in __usb_queue_reset_device() 
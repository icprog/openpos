one will
 * be allocated.
 */
int uwb_dev_add(struct uwb_dev *uwb_dev, struct device *parent_dev,
		struct uwb_rc *parent_rc)
{
	int result;
	struct device *dev;

	BUG_ON(uwb_dev == NULL);
	BUG_ON(parent_dev == NULL);
	BUG_ON(parent_rc == NULL);

	mutex_lock(&uwb_dev->mutex);
	dev = &uwb_dev->dev;
	uwb_dev->rc = parent_rc;
	result = __uwb_dev_sys_add(uwb_dev, parent_dev);
	if (result < 0)
		printk(KERN_ERR "UWB: unable to register dev %s with sysfs: %d\n",
		       dev_name(dev), result);
	mutex_unlock(&uwb_dev->mutex);
	return result;
}


void uwb_dev_rm(struct uwb_dev *uwb_dev)
{
	mutex_lock(&uwb_dev->mutex);
	__uwb_dev_sys_rm(uwb_dev);
	mutex_unlock(&uwb_dev->mutex);
}


static
int __uwb_dev_try_get(struct device *dev, void *__target_uwb_dev)
{
	struct uwb_dev *target_uwb_dev = __target_uwb_dev;
	struct uwb_dev *uwb_dev = to_uwb_dev(dev);
	if (uwb_dev == target_uwb_dev) {
		uwb_dev_get(uwb_dev);
		return 1;
	} else
		return 0;
}


/**
 * Given a UWB device descriptor, validate and refcount it
 *
 * @returns NULL if the device does not exist or is quiescing; the ptr to
 *               it otherwise.
 */
struct uwb_dev *uwb_dev_try_get(struct uwb_rc *rc, struct uwb_dev *uwb_dev)
{
	if (uwb_dev_for_each(rc, __uwb_dev_try_get, uwb_dev))
		return uwb_dev;
	else
		return NULL;
}
EXPORT_SYMBOL_GPL(uwb_dev_try_get);


/**
 * Remove a device from the system [grunt for other functions]
 */
int __uwb_dev_offair(struct uwb_dev *uwb_dev, struct uwb_rc *rc)
{
	struct device *dev = &uwb_dev->dev;
	char macbuf[UWB_ADDR_STRSIZE], devbuf[UWB_ADDR_STRSIZE];

	uwb_mac_addr_print(macbuf, sizeof(macbuf), &uwb_dev->mac_addr);
	uwb_dev_addr_print(devbuf, sizeof(devbuf), &uwb_dev->dev_addr);
	dev_info(dev, "uwb device (mac %s dev %s) disconnected from %s %s\n",
		 macbuf, devbuf,
		 rc ? rc->uwb_dev.dev.parent->bus->name : "n/a",
		 rc ? dev_name(rc->uwb_dev.dev.parent) : "");
	uwb_dev_rm(uwb_dev);
	list_del(&uwb_dev->bce->node);
	uwb_bce_put(uwb_dev->bce);
	uwb_dev_put(uwb_dev);	/* for the creation in _onair() */

	return 0;
}


/**
 * A device went off the air, clean up after it!
 *
 * This is called by the UWB Daemon (through the beacon purge function
 * uwb_bcn_cache_purge) when it is detected that a device has been in
 * radio silence for a while.
 *
 * If this device is actually a local radio controller we don't need
 * to go through the offair process, as it is not registered as that.
 *
 * NOTE: uwb_bcn_cache.mutex is held!
 */
void uwbd_dev_offair(struct uwb_beca_e *bce)
{
	struct uwb_dev *uwb_dev;

	uwb_dev = bce->uwb_dev;
	if (uwb_dev) {
		uwb_notify(uwb_dev->rc, uwb_dev, UWB_NOTIF_OFFAIR);
		__uwb_dev_offair(uwb_dev, uwb_dev->rc);
	}
}


/**
 * A device went on the air, start it up!
 *
 * This is called by the UWB Daemon when it is detected that a device
 * has popped up in the radio range of the radio controller.
 *
 * It will just create the freaking device, register the beacon and
 * stuff and yatla, done.
 *
 *
 * NOTE: uwb_beca.mutex is held, bce->mutex is held
 */
void uwbd_dev_onair(struct uwb_rc *rc, struct uwb_beca_e *bce)
{
	int result;
	struct device *dev = &rc->uwb_dev.dev;
	struct uwb_dev *uwb_dev;
	char macbuf[UWB_ADDR_STRSIZE], devbuf[UWB_ADDR_STRSIZE];

	uwb_mac_addr_print(macbuf, sizeof(macbuf), bce->mac_addr);
	uwb_dev_addr_print(devbuf, sizeof(devbuf), &bce->dev_addr);
	uwb_dev = kzalloc(sizeof(struct uwb_dev), GFP_KERNEL);
	if (uwb_dev == NULL) {
		dev_err(dev, "new device %s: Cannot allocate memory\n",
			macbuf);
		return;
	}
	uwb_dev_init(uwb_dev);		/* This sets refcnt to one, we own it */
	uwb_dev->mac_addr = *bce->mac_addr;
	uwb_dev->dev_addr = bce->dev_addr;
	dev_set_name(&uwb_dev->dev, macbuf);
	result = uwb_dev_add(uwb_dev, &rc->uwb_dev.dev, rc);
	if (result < 0) {
		dev_err(dev, "new device %s: cannot instantiate device\n",
			macbuf);
		goto error_dev_add;
	}
	/* plug the beacon cache */
	bce->uwb_dev = uwb_dev;
	uwb_dev->bce = bce;
	uwb_bce_get(bce);		/* released in uwb_dev_sys_release() */
	dev_info(dev, "uwb device (mac %s dev %s) connected to %s %s\n",
		 macbuf, devbuf, rc->uwb_dev.dev.parent->bus->name,
		 dev_name(rc->uwb_dev.dev.parent));
	uwb_notify(rc, uwb_dev, UWB_NOTIF_ONAIR);
	return;

error_dev_add:
	kfree(uwb_dev);
	return;
}

/**
 * Iterate over the list of UWB devices, calling a @function on each
 *
 * See docs for bus_for_each()....
 *
 * @rc:       radio controller for the devices.
 * @function: function to call.
 * @priv:     data to pass to @function.
 * @returns:  0 if no invocation of function() returned a value
 *            different to zero. That value otherwise.
 */
int uwb_dev_for_each(struct uwb_rc *rc, uwb_dev_for_each_f function, void *priv)
{
	return device_for_each_child(&rc->uwb_dev.dev, priv, function);
}
EXPORT_SYMBOL_GPL(uwb_dev_for_each);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
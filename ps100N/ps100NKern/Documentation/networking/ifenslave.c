ort_read(gameport) ^ t) & t & 0xf;
		port->fuzz = (port->speed * ANALOG_FUZZ_MAGIC) / port->loop / 1000 + ANALOG_FUZZ_BITS;

		for (i = 0; i < ANALOG_INIT_RETRIES; i++) {
			if (!analog_cooked_read(port))
				break;
			msleep(ANALOG_MAX_TIME);
		}

		u = v = 0;

		msleep(ANALOG_MAX_TIME);
		t = gameport_time(gameport, ANALOG_MAX_TIME * 1000);
		gameport_trigger(gameport);
		while ((gameport_read(port->gameport) & port->mask) && (u < t))
			u++;
		udelay(ANALOG_SAITEK_DELAY);
		t = gameport_time(gameport, ANALOG_SAITEK_TIME);
		gameport_trigger(gameport);
		while ((gameport_read(port->gameport) & port->mask) && (v < t))
			v++;

		if (v < (u >> 1)) { /* FIXME - more than one port */
			analog_options[0] |= /* FIXME - more than one port */
				ANALOG_SAITEK | ANALOG_BTNS_CHF | ANALOG_HBTN_CHF | ANALOG_HAT1_CHF;
			return 0;
		}

		gameport_close(gameport);
	}

	if (!gameport_open(gameport, drv, GAMEPORT_MODE_COOKED)) {

		for (i = 0; i < ANALOG_INIT_RETRIES; i++)
			if (!gameport_cooked_read(gameport, port->axes, &port->buttons))
				break;
		for (i = 0; i < 4; i++)
			if (port->axes[i] != -1)
				port->mask |= 1 << i;

		port->fuzz = gameport->fuzz;
		port->cooked = 1;
		return 0;
	}

	return gameport_open(gameport, drv, GAMEPORT_MODE_RAW);
}

static int analog_connect(struct gameport *gameport, struct gameport_driver *drv)
{
	struct analog_port *port;
	int i;
	int err;

	if (!(port = kzalloc(sizeof(struct analog_port), GFP_KERNEL)))
		return - ENOMEM;

	err = analog_init_port(gameport, drv, port);
	if (err)
		goto fail1;

	err = analog_init_masks(port);
	if (err)
		goto fail2;

	gameport_set_poll_handler(gameport, analog_poll);
	gameport_set_poll_interval(gameport, 10);

	for (i = 0; i < 2; i++)
		if (port->analog[i].mask) {
			err = analog_init_device(port, port->analog + i, i);
			if (err)
				goto fail3;
		}

	return 0;

 fail3: while (--i >= 0)
		if (port->analog[i].mask)
			input_unregister_device(port->analog[i].dev);
 fail2:	gameport_close(gameport);
 fail1:	gameport_set_drvdata(gameport, NULL);
	kfree(port);
	return err;
}

static void analog_disconnect(struct gameport *gameport)
{
	struct analog_port *port = gameport_get_drvdata(gameport);
	int i;

	for (i = 0; i < 2; i++)
		if (port->analog[i].mask)
			input_unregister_device(port->analog[i].dev);
	gameport_close(gameport);
	gameport_set_drvdata(gameport, NULL);
	printk(KERN_INFO "analog.c: %d out of %d reads (%d%%) on %s failed\n",
		port->bads, port->reads, port->reads ? (port->bads * 100 / port->reads) : 0,
		port->gameport->phys);
	kfree(port);
}

struct analog_types {
	char *name;
	int value;
};

static struct analog_types analog_types[] = {
	{ "none",	0x00000000 },
	{ "auto",	0x000000ff },
	{ "2btn",	0x0000003f },
	{ "y-joy",	0x0cc00033 },
	{ "y-pad",	0x8cc80033 },
	{ "fcs",	0x000008f7 },
	{ "chf",	0x000002ff },
	{ "fullchf",	0x000007ff },
	{ "gamepad",	0x000830f3 },
	{ "gamepad8",	0x0008f0f3 },
	{ NULL, 0 }
};

static void analog_parse_options(void)
{
	int i, j;
	char *end;

	for (i = 0; i < js_nargs; i++) {

		for (j = 0; analog_types[j].name; j++)
			if (!strcmp(analog_types[j].name, js[i])) {
				analog_options[i] = analog_types[j].value;
				break;
			}
		if (analog_types[j].name) continue;

		analog_options[i] = simple_strtoul(js[i], &end, 0);
		if (end != js[i]) continue;

		analog_options[i] = 0xff;
		if (!strlen(js[i])) continue;

		printk(KERN_WARNING "analog.c: Bad config for port %d - \"%s\"\n", i, js[i]);
	}

	for (; i < ANALOG_PORTS; i++)
		analog_options[i] = 0xff;
}

/*
 * The gameport device structure.
 */

static struct gameport_driver analog_drv = {
	.driver		= {
		.name	= "analog",
	},
	.description	= DRIVER_DESC,
	.connect	= analog_connect,
	.disconnect	= analog_disconnect,
};

static int __init analog_init(void)
{
	analog_parse_options();
	return gameport_register_driver(&analog_drv);
}

static void __exit analog_exit(void)
{
	gameport_unregister_driver(&analog_drv);
}

module_init(analog_init);
module_exit(analog_exit);
                                                                                                                  /*
 * Ultra Wide Band
 * Driver initialization, etc
 *
 * Copyright (C) 2005-2006 Intel Corporation
 * Inaky Perez-Gonzalez <inaky.perez-gonzalez@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 *
 * FIXME: docs
 *
 * Life cycle: FIXME: explain
 *
 *  UWB radio controller:
 *
 *    1. alloc a uwb_rc, zero it
 *    2. call uwb_rc_init() on it to set it up + ops (won't do any
 *       kind of allocation)
 *    3. register (now it is owned by the UWB stack--deregister before
 *       freeing/destroying).
 *    4. It lives on it's own now (UWB stack handles)--when it
 *       disconnects, call unregister()
 *    5. free it.
 *
 *    Make sure you have a reference to the uwb_rc before calling
 *    any of the UWB API functions.
 *
 * TODO:
 *
 * 1. Locking and life cycle management is crappy still. All entry
 *    points to the UWB HCD API assume you have a reference on the
 *    uwb_rc structure and that it won't go away. They mutex lock it
 *    before doing anything.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/kdev_t.h>
#include <linux/random.h>

#include "uwb-internal.h"


/* UWB stack attributes (or 'global' constants) */


/**
 * If a beacon dissapears for longer than this, then we consider the
 * device who was represented by that beacon to be gone.
 *
 * ECMA-368[17.2.3, last para] establishes that a device must not
 * consider a device to be its neighbour if he doesn't receive a beacon
 * for more than mMaxLostBeacons. mMaxLostBeacons is defined in
 * ECMA-368[17.16] as 3; because we can get only one beacon per
 * superframe, that'd be 3 * 65ms = 195 ~ 200 ms. Let's give it time
 * for jitter and stuff and make it 500 ms.
 */
unsigned long beacon_timeout_ms = 500;

static
ssize_t beacon_timeout_ms_show(struct class *class, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%lu\n", beacon_timeout_ms);
}

static
ssize_t beacon_timeout_ms_store(struct class *class,
				const char *buf, size_t size)
{
	unsigned long bt;
	ssize_t result;
	result = sscanf(buf, "%lu", &bt);
	if (result != 1)
		return -EINVAL;
	beacon_timeout_ms = bt;
	return size;
}

static struct class_attribute uwb_class_attrs[] = {
	__ATTR(beacon_timeout_ms, S_IWUSR | S_IRUGO,
	       beacon_timeout_ms_show, beacon_timeout_ms_store),
	__ATTR_NULL,
};

/** Device model classes */
struct class uwb_rc_class = {
	.name        = "uwb_rc",
	.class_attrs = uwb_class_attrs,
};


static int __init uwb_subsys_init(void)
{
	int result = 0;

	result = uwb_est_create();
	if (result < 0) {
		printk(KERN_ERR "uwb: Can't initialize EST subsystem\n");
		goto error_est_init;
	}

	result = class_register(&uwb_rc_class);
	if (result < 0)
		goto error_uwb_rc_class_register;
	uwb_dbg_init();
	return 0;

error_uwb_rc_class_register:
	uwb_est_destroy();
error_est_init:
	return result;
}
module_init(uwb_subsys_init);

static void __exit uwb_subsys_exit(void)
{
	uwb_dbg_exit();
	class_unregister(&uwb_rc_class);
	uwb_est_destroy();
	return;
}
module_exit(uwb_subsys_exit);

MODULE_AUTHOR("Inaky Perez-Gonzalez <inaky.perez-gonzalez@intel.com>");
MODULE_DESCRIPTION("Ultra Wide Band core");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                          INDX( 	                 (   �   �      � s .   ��  v         �K     x h     �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c � i g l u e . h . s v n - b a s e               x h     �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               x h     �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h � s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e             � �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               x h     �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               x h     �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=�� ��3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     x h     �K     �/=����3
��1/=����N㒍�       �               s c s i g l u e . h . s v n - b a s e               �K     �/=����3
��1/=�� ��N㒍�       �               s c s i g l u e . h . s v n - b a s e               e       �K     � v     �K     �0(=��7b�3
���S(=���tEג��                      u n u s u a l _ d a t a f a b . h . s v n - b a s e   �K     � p     �K     pF.=��%��3
��,�.=��4�Kג�� �      ��               u n u s u a l _ d e v s . h . s v n - b a s e �K     � v     �K     E$=���Ő3
��&h$=����Cג��       �               u n u s u a l _ f r e e c o m . h . s v n - b a s e   �K     � r   � �K     T�(=��£�3
��f)=����G㒍�       �               u n u s u a l _ k a r m a . h . s v n - b a s e       �K     � x     �K     @'=���L�3
���'=��x�Gג��       '               u n u s u a l _ o n e t o u c h . h . s v n - b a s e �K     � t     �K     XZ=��U!�3
���z=����=㒍�                      u n u s u a l _ s d d r 0 9 . h . s v n - b a s e     �K     � t     �K     �/&=����3
�� S&=����S㒍�       �               u n u s u a l _ s d d r 5 5 . h . s � n - b a s e     �K     � r     �K     l�,=�����3
����,=����@ג��       �               u n u s u a l _ u s b a t . h . s v n - b a s e       �K     p ^     �K     V�)=��ά�3
���F*=���}U㒍� �      �r               u s b . c . s v n - b a s e   �K     � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e                                                                                                                     � INDX( 	                (   �   �      + u ��                �K     � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               �3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               �3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s + a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג�+        R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               �K     .�"=�+ �3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e       +       �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e       +       � p     �K     .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e               .�"=���3
���#=��:�Dג��       R               u s u a l - t a b l e s . c . s v n - b a s e                                                                                                                                                                                                                                                                     +                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               +                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               + /*
 * Copyright 2004-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file pmic_event.c
 * @brief This file manage all event of PMIC component.
 *
 * It contains event subscription, unsubscription and callback
 * launch methods implemeted.
 *
 * @ingroup PMIC_CORE
 */

/*
 * Includes
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>
#include "pmic.h"

/*!
 * This structure is used to keep a list of subscribed
 * callbacks for an event.
 */
typedef struct {
	/*!
	 * Keeps a list of subscribed clients to an event.
	 */
	struct list_head list;

	/*!
	 * Callback function with parameter, called when event occurs
	 */
	pmic_event_callback_t callback;
} pmic_event_callback_list_t;

/* Create a mutex to be used to prevent concurrent access to the event list */
static DECLARE_MUTEX(event_mutex);

/* This is a pointer to the event handler array. It defines the currently
 * active set of events and user-defined callback functions.
 */
static struct list_head pmic_events[PMIC_MAX_EVENTS];

/*!
 * This function initializes event list for PMIC event handling.
 *
 */
void pmic_event_list_init(void)
{
	int i;

	for (i = 0; i < PMIC_MAX_EVENTS; i++) {
		INIT_LIST_HEAD(&pmic_events[i]);
	}

	sema_init(&event_mutex, 1);
	return;
}

/*!
 * This function is used to subscribe on an event.
 *
 * @param	event   the event number to be subscribed
 * @param	callback the callback funtion to be subscribed
 *
 * @return       This function returns 0 on SUCCESS, error on FAILURE.
 */
PMIC_STATUS pmic_event_subscribe(type_event event,
				 pmic_event_callback_t callback)
{
	pmic_event_callback_list_t *new = NULL;

	pr_debug("Event:%d Subscribe\n", event);

	/* Check whether the event & callback are valid? */
	if (event >= PMIC_MAX_EVENTS) {
		pr_debug("Invalid Event:%d\n", event);
		return -EINVAL;
	}
	if (NULL == callback.func) {
		pr_debug("Null or Invalid Callback\n");
		return -EINVAL;
	}

	/* Create a new linked list entry */
	new = kmalloc(sizeof(pmic_event_callback_list_t), GFP_KERNEL);
	if (NULL == new) {
		return -ENOMEM;
	}
	/* Initialize the list node fields */
	new->callback.func = callback.func;
	new->callback.param = callback.param;
	INIT_LIST_HEAD(&new->list);

	/* Obtain the lock to access the list */
	if (down_interruptible(&event_mutex)) {
		kfree(new);
		return PMIC_SYSTEM_ERROR_EINTR;
	}

	/* Unmask the requested event */
	if (list_empty(&pmic_events[event])) {
		if (pmic_event_unmask(event) != PMIC_SUCCESS) {
			kfree(new);
			up(&event_mutex);
			return PMIC_ERROR;
		}
	}

	/* Add this entry to the event list */
	list_add_tail(&new->list, &pmic_events[event]);

	/* Release the lock */
	up(&event_mutex);

	return PMIC_SUCCESS;
}

/*!
 * This function is used to unsubscribe on an event.
 *
 * @param	event   the event number to be unsubscribed
 * @param	callback the callback funtion to be unsubscribed
 *
 * @return       This function returns 0 on SUCCESS, error on FAILURE.
 */
PMIC_STATUS pmic_event_unsubscribe(type_event event,
				   pmic_event_callback_t callback)
{
	struct list_head *p;
	struct list_head *n;
	pmic_event_callback_list_t *temp = NULL;
	int ret = PMIC_EVENT_NOT_SUBSCRIBED;

	pr_debug("Event:%d Unsubscribe\n", event);

	/* Check whether the event & callback are valid? */
	if (event >= PMIC_MAX_EVENTS) {
		pr_debug("Invalid Event:%d\n", event);
		return -EINVAL;
	}

	if (NULL == callback.func) {
		pr_debug("Null or Invalid Callback\n");
		return -EINVAL;
	}

	/* Obtain the lock to access the list */
	if (down_interruptible(&event_mutex)) {
		return PMIC_SYSTEM_ERROR_EINTR;
	}

	/* Find the entry in the list */
	list_for_each_safe(p, n, &pmic_events[event]) {
		temp = list_entry(p, pmic_event_callback_list_t, list);
		if (temp->callback.func == callback.func
		    && temp->callback.param == callback.param) {
			/* Remove the entry from the list */
			list_del(p);
			kfree(temp);
			ret = PMIC_SUCCESS;
			break;
		}
	}

	/* Unmask the requested event */
	if (list_empty(&pmic_events[event])) {
		if (pmic_event_mask(event) != PMIC_SUCCESS) {
			ret = PMIC_UNSUBSCRIBE_ERROR;
		}
	}

	/* Release the lock */
	up(&event_mutex);

	return ret;
}

/*!
 * This function calls all callback of a specific event.
 *
 * @param	event   the active event number
 *
 * @return 	None
 */
void pmic_event_callback(type_event event)
{
	struct list_head *p;
	pmic_event_callback_list_t *temp = NULL;

	/* Obtain the lock to access the list */
	if (down_interruptible(&event_mutex)) {
		return;
	}

	if (list_empty(&pmic_events[event])) {
		pr_debug("PMIC Event:%d detected. No callback subscribed\n",
			 event);
		up(&event_mutex);
		return;
	}

	list_for_each(p, &pmic_events[event]) {
		temp = list_entry(p, pmic_event_callback_list_t, list);
		temp->callback.func(temp->callback.param);
	}

	/* Release the lock */
	up(&event_mutex);

	return;

}

EXPORT_SYMBOL(pmic_event_subscribe);
EXPORT_SYMBOL(pmic_event_unsubscribe);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   D$(�$�����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$�C�����g�2 � ����t�D$(�$�%����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$���������2 � ����t�D$(�$�����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$�c����Ç�2 � ����t�D$(�$�E����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$��������2 � ����t�D$(�$������[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$�����ç�2 � ����t�D$(�$�e����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$������7�2 � ����t�D$(�$������[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$������ǉ2 � ����t�D$(�$�����[ËP��R���   ~��@��@	%�   ��0�D$(t�@H��[Ít& �@D��[�        S���D$$�3�����W�2 � ����t�D$(�$�����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$��������2 � ����t�D$(�$�����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$�S�����w�2 � ����t�D$(�$�5����[ËP��R���   ~��@����t�D$(�@H��[Ë@��@-�   ��w�D$(�@D��[�           S���D$$���������2 � ����t�D$(�$�����[ËP��R���   ~��@��@	%�   ���D$(v�@H��[Ít& �@D��[�        S���D$$�c����Ç�2 � ����t�D$(�$�E����[ËP��R���   ~��@��@	%�   �������D$(v�@H��[Í�    �@D��[�S���D$$��������2 � ����t�D$(�$������[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$�����ç�2 � ����t�D$(�$�e����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$������7�2 � ����t�D$(�$������[ËP��R���   ~��@��@	%�   ��8�D$(t�@H��[Ít& �@D��[�        S���D$$������ǅ2 � ����t�D$(�$�����[ËP��R���   ~��@��@	%�   ���D$(v�@H��[Ít& �@D��[�        S���D$$�3�����W�2 � ����t�D$(�$�����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$��������2 � ����t�D$(�$�����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$�S�����w�2 � ����t�D$(�$�5����[ËP��R���   ~��@����t�D$(�@H��[Ë@��x�u�D$(�@D��[�   S���D$$��������2 � ����t�D$(�$������[ËP��R���   ~��@��@	%�   �������D$(v�@H��[Í�    �@D��[�S���D$$�s����×�2 �T$(� ������t�$�U����[ËH��I���   ~�@��@	%�   ���������t��t�BH��[Ív �BD��[�VS���D$$������&�2 �T$(� ������t�$������[^Í�    �H��I���   ~݋@������t�BD��[^Ív ���p���  ���I9qxu�;�8  tًBH��[^�            U��WV��S1���<�M�k����Ï�2 �9�$  �} �j  �}�?�O�}����ρ��  ���}�t��%�  ����  ��x��D  ;�H  �  �P��D  �M�8�W	�}��M܁��   ��9�t�D$�|$�$�z���E��������1Ҁ9 ��   �}���   �D$�E܉T$�D$   �D$    �$�t$��V���}���D$    �D$    �D$    �|$�4$�{����M�@��P	�E܉L$�|$���   ���$�U��}����E�U�������   ��p��N	�΁��   ��9�t�t& �E��B%����   @�B��e�[^_]Ð�U�/����E؍E�T$�$�D$    �t$�E܉U��Q���U��E؋}������R�R	t��%�   ��������3  �D$    �D$    �D$    �|$�4$�z����@��E�|$�D$�E܉$�����}� P�N�����FP�e�[^_]Ð�t& �A� �u��=�� w����������t& �M�T$�$�D$    �t$�M��P���}���D$    �D$    �D$    �|$�4$� z����M��@��E�|$�$�D$������E�����_����E���������@�����f��}��`����E���u_�E�   �� ����E�M�ɉH�<���f����������u�   ��t& ��t[���   DD$�<$�|��������&    �}���t���   EǉE���E�   뇍�&    �4$����������v �   � U��S����� ��������~2 �u��x`�]��Í�    �@f.��V��rFf(��W��fW�f(���f��fT�fr�f\���tV��f��f.�f��f~T$�D$s����K���D$�������D$��p����$�D$�   �9��1��]���  �D$���J  �zD u�@HÍt& �R|��t�T$��у���u�J��y�u؋R�R�у���t�@DÍt& �R��z�u��     U��VS��0�E��������}2 � ����t�E�$����e�[^]Ít& �P��R��ލU�$�D$�D$    �D$    �D$   �����u���4$�����4$�D$��'���U�E�ҋ@8t��$�E������E�e�[^]�         S���T$$������7}2 �������t�D$(�$������[ËH��I���   ~��R��у���uӋJ��I���   ~ċH��B�9��D$(t�@H��[Í�&    �@D��[�S���T$$�����÷|2 �������t�D$(�$�u����[ËH��y�u�R��у���uًJ��I��͉$�T$�&���D$(�@8��[�      S���'�����K|2 �|$ t�D$(�$�����[Ð�D$$� ����t�D$(�@8��[Ít& �P��z�u�P�BK%�������BK��            S���������{2 �|$ t�D$(�$�����[Ð�D$$� ����t�D$(�@8��[Ít& �P��z�u�P�BK$~��BK��WV1�S�V�����z{2 ��@  �|$�D$�<$�jB����$4  ��u
���t& ���<$�mJ����$4  ��u��@  ��[^_�      VS���D$$�������{2 �0�D$(������Pd��	����D$�D$��6����T$�$�L� ��th��B����$�:� ������4$��T�D$�����������$�� ����[^Í�&    ��>����D$��6����T$�D$P   �$�� �f�������D$    �D$   ��T�D$�D$(�$�wO����?'���$詭 �       VS��������*z2 �t$(��������P`��	����D$�D$��6����T$�$�`� ������4$�D$    �D$   ��T�D$��N����?'���$�-� �F8��[^Ít& ��>����D$��6����T$�D$P   �$��� �          S���D$$�S�����wy2 � ����t�D$(�$�5	����[ËP��R���   ~��P��z�u�@��@����t�@��@	�D$(u+�@H��[Ív �P��z�uߋP����uՋD$(�܍�&    �@D��[�        UW��V��S����������x2 �l$0����tr�l$�<$�~�����@����t�FD��[^_]Ð���D  ;�H  �@��x��   �B��D  �������:tY�F<9��  t>�4$�������[^_]Í�    �@��@-�   ���y����l$�<$觻���z���f��FH��
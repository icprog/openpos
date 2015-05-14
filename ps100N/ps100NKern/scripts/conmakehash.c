de, &i1480u->tx_list);
	spin_unlock_irqrestore(&i1480u->tx_list_lock, flags);
	return wtx;

error_create:
	kfree(wtx->urb);
error_urb_alloc:
	kfree(wtx);
error_wtx_alloc:
	return NULL;
}

/*
 * Actual fragmentation and transmission of frame
 *
 * @wlp:  WLP substack data structure
 * @skb:  To be transmitted
 * @dst:  Device address of destination
 * @returns: 0 on success, <0 on failure
 *
 * This function can also be called directly (not just from
 * hard_start_xmit), so we also check here if the interface is up before
 * taking sending anything.
 */
int i1480u_xmit_frame(struct wlp *wlp, struct sk_buff *skb,
		      struct uwb_dev_addr *dst)
{
	int result = -ENXIO;
	struct i1480u *i1480u = container_of(wlp, struct i1480u, wlp);
	struct device *dev = &i1480u->usb_iface->dev;
	struct net_device *net_dev = i1480u->net_dev;
	struct i1480u_tx *wtx;
	struct wlp_tx_hdr *wlp_tx_hdr;
	static unsigned char dev_bcast[2] = { 0xff, 0xff };

	BUG_ON(i1480u->wlp.rc == NULL);
	if ((net_dev->flags & IFF_UP) == 0)
		goto out;
	result = -EBUSY;
	if (atomic_read(&i1480u->tx_inflight.count) >= i1480u->tx_inflight.max) {
		netif_stop_queue(net_dev);
		goto error_max_inflight;
	}
	result = -ENOMEM;
	wtx = i1480u_tx_create(i1480u, skb, GFP_ATOMIC);
	if (unlikely(wtx == NULL)) {
		if (printk_ratelimit())
			dev_err(dev, "TX: no memory for WLP TX URB,"
				"dropping packet (in flight %d)\n",
				atomic_read(&i1480u->tx_inflight.count));
		netif_stop_queue(net_dev);
		goto error_wtx_alloc;
	}
	wtx->i1480u = i1480u;
	/* Fill out the i1480 header; @i1480u->def_tx_hdr read without
	 * locking. We do so because they are kind of orthogonal to
	 * each other (and thus not changed in an atomic batch).
	 * The ETH header is right after the WLP TX header. */
	wlp_tx_hdr = wtx->wlp_tx_hdr;
	*wlp_tx_hdr = i1480u->options.def_tx_hdr;
	wlp_tx_hdr->dstaddr = *dst;
	if (!memcmp(&wlp_tx_hdr->dstaddr, dev_bcast, sizeof(dev_bcast))
	    && (wlp_tx_hdr_delivery_id_type(wlp_tx_hdr) & WLP_DRP)) {
		/*Broadcast message directed to DRP host. Send as best effort
		 * on PCA. */
		wlp_tx_hdr_set_delivery_id_type(wlp_tx_hdr, i1480u->options.pca_base_priority);
	}

	result = usb_submit_urb(wtx->urb, GFP_ATOMIC);		/* Go baby */
	if (result < 0) {
		dev_err(dev, "TX: cannot submit URB: %d\n", result);
		/* We leave the freeing of skb to calling function */
		wtx->skb = NULL;
		goto error_tx_urb_submit;
	}
	atomic_inc(&i1480u->tx_inflight.count);
	net_dev->trans_start = jiffies;
	return result;

error_tx_urb_submit:
	i1480u_tx_destroy(i1480u, wtx);
error_wtx_alloc:
error_max_inflight:
out:
	return result;
}


/*
 * Transmit an skb  Called when an skbuf has to be transmitted
 *
 * The skb is first passed to WLP substack to ensure this is a valid
 * frame. If valid the device address of destination will be filled and
 * the WLP header prepended to the skb. If this step fails we fake sending
 * the frame, if we return an error the network stack will just keep trying.
 *
 * Broadcast frames inside a WSS needs to be treated special as multicast is
 * not supported. A broadcast frame is sent as unicast to each member of the
 * WSS - this is done by the WLP substack when it finds a broadcast frame.
 * So, we test if the WLP substack took over the skb and only transmit it
 * if it has not (been taken over).
 *
 * @net_dev->xmit_lock is held
 */
int i1480u_hard_start_xmit(struct sk_buff *skb, struct net_device *net_dev)
{
	int result;
	struct i1480u *i1480u = netdev_priv(net_dev);
	struct device *dev = &i1480u->usb_iface->dev;
	struct uwb_dev_addr dst;

	if ((net_dev->flags & IFF_UP) == 0)
		goto error;
	result = wlp_prepare_tx_frame(dev, &i1480u->wlp, skb, &dst);
	if (result < 0) {
		dev_err(dev, "WLP verification of TX frame failed (%d). "
			"Dropping packet.\n", result);
		goto error;
	} else if (result == 1) {
		/* trans_start time will be set when WLP actually transmits
		 * the frame */
		goto out;
	}
	result = i1480u_xmit_frame(&i1480u->wlp, skb, &dst);
	if (result < 0) {
		dev_err(dev, "Frame TX failed (%d).\n", result);
		goto error;
	}
	return NETDEV_TX_OK;
error:
	dev_kfree_skb_any(skb);
	net_dev->stats.tx_dropped++;
out:
	return NETDEV_TX_OK;
}


/*
 * Called when a pkt transmission doesn't complete in a reasonable period
 * Device reset may sleep - do it outside of interrupt context (delayed)
 */
void i1480u_tx_timeout(struct net_device *net_dev)
{
	struct i1480u *i1480u = netdev_priv(net_dev);

	wlp_reset_all(&i1480u->wlp);
}


void i1480u_tx_release(struct i1480u *i1480u)
{
	unsigned long flags;
	struct i1480u_tx *wtx, *next;
	int count = 0, empty;

	spin_lock_irqsave(&i1480u->tx_list_lock, flags);
	list_for_each_entry_safe(wtx, next, &i1480u->tx_list, list_node) {
		count++;
		usb_unlink_urb(wtx->urb);
	}
	spin_unlock_irqrestore(&i1480u->tx_list_lock, flags);
	count = count*10; /* i1480ut 200ms per unlinked urb (intervals of 20ms) */
	/*
	 * We don't like this sollution too much (dirty as it is), but
	 * it is cheaper than putting a refcount on each i1480u_tx and
	 * i1480uting for all of them to go away...
	 *
	 * Called when no more packets can be added to tx_list
	 * so can i1480ut for it to be empty.
	 */
	while (1) {
		spin_lock_irqsave(&i1480u->tx_list_lock, flags);
		empty = list_empty(&i1480u->tx_list);
		spin_unlock_irqrestore(&i1480u->tx_list_lock, flags);
		if (empty)
			break;
		count--;
		BUG_ON(count == 0);
		msleep(20);
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
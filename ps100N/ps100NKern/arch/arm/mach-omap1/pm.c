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


static void setup_cmd_submit_pdu(struct usbip_header *pdup,  struct urb *urb)
{
	struct vhci_priv *priv = ((struct vhci_priv *)urb->hcpriv);
	struct vhci_device *vdev = priv->vdev;

	dbg_vhci_tx("URB, local devnum %u, remote devid %u\n",
				usb_pipedevice(urb->pipe), vdev->devid);

	pdup->base.command = USBIP_CMD_SUBMIT;
	pdup->base.seqnum  = priv->seqnum;
	pdup->base.devid   = vdev->devid;
	if (usb_pipein(urb->pipe))
		pdup->base.direction = USBIP_DIR_IN;
	else
		pdup->base.direction = USBIP_DIR_OUT;
	pdup->base.ep      = usb_pipeendpoint(urb->pipe);

	usbip_pack_pdu(pdup, urb, USBIP_CMD_SUBMIT, 1);

	if (urb->setup_packet)
		memcpy(pdup->u.cmd_submit.setup, urb->setup_packet, 8);
}

static struct vhci_priv *dequeue_from_priv_tx(struct vhci_device *vdev)
{
	unsigned long flags;
	struct vhci_priv *priv, *tmp;

	spin_lock_irqsave(&vdev->priv_lock, flags);

	list_for_each_entry_safe(priv, tmp, &vdev->priv_tx, list) {
		list_move_tail(&priv->list, &vdev->priv_rx);
		spin_unlock_irqrestore(&vdev->priv_lock, flags);
		return priv;
	}

	spin_unlock_irqrestore(&vdev->priv_lock, flags);

	return NULL;
}



static int vhci_send_cmd_submit(struct vhci_device *vdev)
{
	struct vhci_priv *priv = NULL;

	struct msghdr msg;
	struct kvec iov[3];
	size_t txsize;

	size_t total_size = 0;

	while ((priv = dequeue_from_priv_tx(vdev)) != NULL) {
		int ret;
		struct urb *urb = priv->urb;
		struct usbip_header pdu_header;
		void *iso_buffer = NULL;

		txsize = 0;
		memset(&pdu_header, 0, sizeof(pdu_header));
		memset(&msg, 0, sizeof(msg));
		memset(&iov, 0, sizeof(iov));

		dbg_vhci_tx("setup txdata urb %p\n", urb);


		/* 1. setup usbip_header */
		setup_cmd_submit_pdu(&pdu_header, urb);
		usbip_header_correct_endian(&pdu_header, 1);

		iov[0].iov_base = &pdu_header;
		iov[0].iov_len  = sizeof(pdu_header);
		txsize += sizeof(pdu_header);

		/* 2. setup transfer buffer */
		if (!usb_pipein(urb->pipe) && urb->transfer_buffer_length > 0) {
			iov[1].iov_base = urb->transfer_buffer;
			iov[1].iov_len  = urb->transfer_buffer_length;
			txsize += urb->transfer_buffer_length;
		}

		/* 3. setup iso_packet_descriptor */
		if (usb_pipetype(urb->pipe) == PIPE_ISOCHRONOUS) {
			ssize_t len = 0;

			iso_buffer = usbip_alloc_iso_desc_pdu(urb, &len);
			if (!iso_buffer) {
				usbip_event_add(&vdev->ud,
						SDEV_EVENT_ERROR_MALLOC);
				return -1;
			}

			iov[2].iov_base = iso_buffer;
			iov[2].iov_len  = len;
			txsize += len;
		}

		ret = kernel_sendmsg(vdev->ud.tcp_socket, &msg, iov, 3, txsize);
		if (ret != txsize) {
			uerr("sendmsg failed!, retval %d for %zd\n", ret,
								txsize);
			kfree(iso_buffer);
			usbip_event_add(&vdev->ud, VDEV_EVENT_ERROR_TCP);
			return -1;
		}

		kfree(iso_buffer);
		dbg_vhci_tx("send txdata\n");

		total_size += txsize;
	}

	return total_size;
}


/*-------------------------------------------------------------------------*/

static struct vhci_unlink *dequeue_from_unlink_tx(struct vhci_device *vdev)
{
	unsigned long flags;
	struct vhci_unlink *unlink, *tmp;

	spin_lock_irqsave(&vdev->priv_lock, flags);

	list_for_each_entry_safe(unlink, tmp, &vdev->unlink_tx, list) {
		list_move_tail(&unlink->list, &vdev->unlink_rx);
		spin_unlock_irqrestore(&vdev->priv_lock, flags);
		return unlink;
	}

	spin_unlock_irqrestore(&vdev->priv_lock, flags);

	return NULL;
}

static int vhci_send_cmd_unlink(struct vhci_device *vdev)
{
	struct vhci_unlink *unlink = NULL;

	struct msghdr msg;
	struct kvec iov[3];
	size_t txsize;

	size_t total_size = 0;

	while ((unlink = dequeue_from_unlink_tx(vdev)) != NULL) {
		int ret;
		struct usbip_header pdu_header;

		txsize = 0;
		memset(&pdu_header, 0, sizeof(pdu_header));
		memset(&msg, 0, sizeof(msg));
		memset(&iov, 0, sizeof(iov));

		dbg_vhci_tx("setup cmd unlink, %lu \n", unlink->seqnum);


		/* 1. setup usbip_header */
		pdu_header.base.command = USBIP_CMD_UNLINK;
		pdu_header.base.seqnum  = unlink->seqnum;
		pdu_header.base.devid	= vdev->devid;
		pdu_header.base.ep	= 0;
		pdu_header.u.cmd_unlink.seqnum = unlink->unlink_seqnum;

		usbip_header_correct_endian(&pdu_header, 1);

		iov[0].iov_base = &pdu_header;
		iov[0].iov_len  = sizeof(pdu_header);
		txsize += sizeof(pdu_header);

		ret = kernel_sendmsg(vdev->ud.tcp_socket, &msg, iov, 1, txsize);
		if (ret != txsize) {
			uerr("sendmsg failed!, retval %d for %zd\n", ret,
								txsize);
			usbip_event_add(&vdev->ud, VDEV_EVENT_ERROR_TCP);
			return -1;
		}


		dbg_vhci_tx("send txdata\n");

		total_size += txsize;
	}

	return total_size;
}


/*-------------------------------------------------------------------------*/

void vhci_tx_loop(struct usbip_task *ut)
{
	struct usbip_device *ud = container_of(ut, struct usbip_device, tcp_tx);
	struct vhci_device *vdev = container_of(ud, struct vhci_device, ud);

	while (1) {
		if (signal_pending(current)) {
			uinfo("vhci_tx signal catched\n");
			break;
		}

		if (vhci_send_cmd_submit(vdev) < 0)
			break;

		if (vhci_send_cmd_unlink(vdev) < 0)
			break;

		wait_event_interruptible(vdev->waitq_tx,
				(!list_empty(&vdev->priv_tx) ||
				 !list_empty(&vdev->unlink_tx)));

		dbg_vhci_tx("pending urbs ?, now wake up\n");
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     o o t h A v r c p C o n t r o l l e r T r a n s f o r m e r H a n d l e r I m p l # i n t e r n a l E n t i t y D e c l :     m N e t w o r k S e l e c t i o n M o d e A u t o m a t i c R e g i s t r a n t s = h a n d l e I s i m R e f r e s h   w i t h   R E F R E S H _ R E S U L T _ R E S E T s t a r t A f t e r E d g e   c a l l e d   f r o m   a   v a l i d   p o s i t i o n A b o u t   t o   d e l i v e r   t r a n s a c t i o n T e r m i n a t e d E v e n t a n d r o i d . a p p . N o t i f i c a t i o n $ B u i l d e r R e m o t e V i e w s o r g . a p a c h e . h t t p . i m p l . c o n n . A b s t r a c t P o o l E n t r y O n l y   N o P a d d i n g   c a n   b e   u s e d   w i t h   A E A D   m o d e s . T o o   m a n y   v a l u e s   f o r   M e s s a g e D i g e s t   a t t r i b u t e F i l t e r   d i d   n o t   r e t u r n   a n   o u t p u t   f o r m a t   f o r   o r g . a p a c h e . h t t p . p a r a m s . C o r e C o n n e c t i o n P N a m e s a n d r o i d . n e t . I C o n n e c t i v i t y M a n a g e r $ S t u b $ P r o x y P a r c e l a b l e C a l l   % s   h a s   n o n e x i s t e n t   p a r e n t   % s a n d r o i d . a p p . P e n d i n g I n t e n t $ C a n c e l e d E x c e p t i o n t e x I m a g e 2 D   c a n ' t   b e   u s e d   w i t h   a   n u l l   B i t m a p I g n o r i n g   c a l l b a c k   r e q u e s t   f r o m   o l d   p r e b u i l t T h i s   d e v i c e   d o e s   n o t   s u p p o r t   c a r d   e m u l a t i o n I n v a l i d   m e t h o d   s p e c i f i e d   f o r   c r e a t e R e q u e s t : I n p u t   % x   h a d   n o   o u t p u t   f o r m a t   l e n g t h   l i s t e d o r g . a p a c h e . c o m m o n s . l o g g i n g . d i a g n o s t i c s . d e s t T r a n s f o r m e r H a n d l e r I m p l # s e t D o c u m e n t L o c a t o r :   S h o u l d   h a v e   s u b t i t l e   c o n t r o l l e r   a l r e a d y   s e t [ E N V ]   A n c e s t r y   o f   c l a s s l o a d e r   w h i c h   l o a d e d   t h e   v a l u e   i s   n o t   a   v a l i d   e n c o d e d   p r o t o c o l ,   T r a n s f o r m e r H a n d l e r I m p l # s t a r t P r e f i x M a p p i n g :   a n d r o i d . n e t . w i f i . L I N K _ C O N F I G U R A T I O N _ C H A N G E D a n d r o i d . o s . s t o r a g e . I M o u n t S e r v i c e $ S t u b $ P r o x y < o v e r l a y >   d o e s   n o t   s p e c i f y   a   t a r g e t   p a c k a g e D e s t r o y i n g   l e a k e d   p r i n t e r   d i s c o v e r y   s e s s i o n C a l l i n g   t h r e a d   n o t   a s s o c i a t e d   w i t h   a   l o o p e r [ a d d S u b s c r i p t i o n I n f o R e c o r d ] -   i n v a l i d   s l o t I d a n d r o i d . v i e w . K e y C h a r a c t e r M a p $ F a l l b a c k A c t i o n C a n n o t   c r e a t e   v e c t o r   o f   n o n - p r i m i t i v e   t y p e . U n a b l e   t o   w r i t e   p r e m i u m   S M S   p o l i c y   d a t a b a s e o r g . a p a c h e . h t t p . p a r a m s . H t t p C o n n e c t i o n P a r a m s a n d r o i d . a c c o u n t s . O p e r a t i o n C a n c e l e d E x c e p t i o n G i v e n   d r m R i g h t s   o r   c o n t e n t P a t h   i s   n o t   v a l i d { h t t p : / / x m l . a p a c h e . o r g / x a l a n } l i n e - s e p a r a t o r s e t R a d i o C a p a b i l i t y :   n e w   r e q u e s t   s e s s i o n   i d = g e t D e v i c e I d L a b e l :   n o   k n o w n   l a b e l   f o r   p h o n e   C o u l d n ' t   r e t r i e v e   p e r m i s s i o n s   f o r   p a c k a g e :   a n d r o i d . w i d g e t . A b s L i s t V i e w $ O n S c r o l l L i s t e n e r c o m . a n d r o i d . c e r t i n s t a l l e r . C e r t I n s t a l l e r M a i n g o v . n i s t . j a v a x . s i p . R F C _ 2 5 4 3 _ S U P P O R T _ E N A B L E D N o   a n d r o i d . v o i c e _ i n t e r a c t i o n   m e t a - d a t a   f o r     l e a k e d   t r a n s a c t i o n s   d e t e c t e d   a n d   r e m o v e d . 
 m e d i a p l a y e r   w e n t   a w a y   w i t h   u n h a n d l e d   e v e n t s [ G S M P h o n e ]   s e n d B u r s t D t m f ( )   i s   a   C D M A   m e t h o d C a n n o t   c r e a t e   A u d i o R e c o r d   s i n k   f o r   A u d i o M i x a n d r o i d . s e r v i c e . w a l l p a p e r . I W a l l p a p e r S e r v i c e K e y p h r a s e R e c o g n i t i o n E v e n t   [ k e y p h r a s e E x t r a s = U n e x p e c t e d   e r r o r   r e a d i n g   p i n   l i s t ;   f a i l i n g . C o u l d   n o t   r e a d   K e y C h a r a c t e r M a p   f r o m   p a r c e l . I n v a l i d   i m a g e   p i x e l   s t r i d e ,   r o w   b y t e   w i d t h   [ L a n d r o i d . a n i m a t i o n . K e y f r a m e $ F l o a t K e y f r a m e ; :   < r i p p l e >   r e q u i r e s   a   v a l i d   c o l o r   a t t r i b u t e D i a l o g   e x i s t s   w i t h   l o o s e   d i a l o g   v a l i d a t i o n   I n v a l i d   A u d i o M i x :   n o t   d e f i n e d   f o r   l o o p   b a c k a p p l i c a t i o n / c o m . a n d r o i d . m a n a g e d p r o v i s i o n i n g K e y   m u s t   b e   D S A   p u b l i c   o r   p r i v a t e   k e y ;   w a s   g e t C l o s e s t A v a i l a b l e Z o o m C r o p   -   a c t u a l C r o p   =   A u t h S c h e m e   r e g i s t r y   n o t   s e t   i n   H T T P   c o n t e x t w a i t S o c k e t S i g n a l   r e a d   1 6   b y t e s   s i g n a l   r e t :   C o r r u p t i o n   r e p o r t e d   b y   s q l i t e   o n   d a t a b a s e :   g e t S e l e c t e d T e x t   o n   i n a c t i v e   I n p u t C o n n e c t i o n a n d r o i d . a p p . e x t r a . P R O V I S I O N I N G _ W I F I _ P A C _ U R L G o t   R e m o t e E x c e p t i o n   c a l l i n g   s e t S e l e c t e d T e x t [ C S P ]   f o u n d   V a l u e A d d e d S e r v i c e s G r o u p ,   v a l u e   a n d r o i d . c o n t e n t . I C o n t e n t S e r v i c e $ S t u b $ P r o x y D i a l o g   i s   t e r m i n a t e d   - -   d r o p p i n g   r e s p o n s e ! a l g o r i t h m   i d e n t i f i e r   i n   k e y   n o t   r e c o g n i s e d a n d r o i d . v i e w . V i e w R o o t I m p l $ Q u e u e d I n p u t E v e n t a n d r o i d . d a t a b a s e . C o n t e n t O b s e r v e r $ T r a n s p o r t o r g . a p a c h e . h t t p . c o n n . s c h e m e . S c h e m e R e g i s t r y a n d r o i d . a p p . e x t r a . P R O V I S I O N I N G _ W I F I _ H I D D E N F r a g m e n t   k a n   b a r a   s � t t a s   f � r   e n   a l l m � n   U R I F a i l e d   t o   r e w r i t e   r e s o u r c e   r e f e r e n c e s   f o r   u p d a t e C u r r e n t C a r r i e r I n P r o v i d e r   n o t   u p d a t e d D c D i s c o n n e c t i n g S t a t e   n o t   h a n d l e d   m s g . w h a t = p r o v i d e r   n a m e   c o n t a i n s   i l l e g a l   c h a r a c t e r :   a n d r o i d . d a t a b a s e . B u l k C u r s o r T o C u r s o r A d a p t o r S i n g l e   p a t h   s e g m e n t   i s   n o t   a   r e s o u r c e   I D :   g e t S u g g e s t e d R e t r y T i m e :   r e t r y   i s   N O T   n e e d e d s e t I n i t i a l A t t a c h A p n :   u s i n g   f i r s t A p n S e t t i n g N u l l   d e s c r i p t i o n   p a s s e d   t o   s e t D e s c r i p t i o n . f a i l e d   t o   g e t   p e r i p h e r a l   m o d e   c a p a b i l i t y :   s t a r t L e S c a n :   c a n n o t   g e t   B l u e t o o t h L e S c a n n e r 0 3 F 7 0 6 1 7 9 8 E B 9 9 E 2 3 8 F D 6 F 1 B F 9 5 B 4 8 F E E B 4 8 5 4 2 5 2 B 0 7 A 5 2 6 C 6 3 D 3 E 2 5 A 2 5 6 A 0 0 7 6 9 9 F 5 4 4 7 E 3 2 A E 4 5 6 B 5 0 E 0 3 F F F F F F F F F F F F F F F F F F F E 1 A E E 1 4 0 F 1 1 0 A F F 9 6 1 3 0 9 a n d r o i d . s e n s o r . m a g n e t i c _ f i e l d _ u n c a l i b r a t e d b a s e   a l i g n e d   c h i l d   i n d e x   o u t   o f   r a n g e   ( 0 ,   p r o c e s s   m m i   s e r v i c e   c o d e   u s i n g   U i c c A p p   s c = a n d r o i d . d a t a b a s e . s q l i t e . S Q L i t e Q u e r y B u i l d e r b e g i n B a t c h E d i t   o n   i n a c t i v e   I n p u t C o n n e c t i o n g o v . n i s t . j a v a x . s i p . R E C E I V E _ U D P _ B U F F E R _ S I Z E s e t R a d i o C a p a b i l i t y :   m N e w R a d i o A c c e s s F a m i l y [ 0 7 1 3 6 1 2 D C D D C B 4 0 A A B 9 4 6 B D A 2 9 C A 9 1 F 7 3 A F 9 5 8 A F D 9 g o v . n i s t . j a v a x . s i p . A U T O _ G E N E R A T E _ T I M E S T A M P s e t R a d i o C a p a b i l i t y :   m O l d R a d i o A c c e s s F a m i l y [ a n d r o i d . m e d i a . m e t a d a t a . D I S P L A Y _ D E S C R I P T I O N g o v . n i s t . j a v a x . s i p . M A X _ C L I E N T _ T R A N S A C T I O N S S e a r c h   s u g g e s t i o n s   c u r s o r   t h r e w   e x c e p t i o n . c o m . a n d r o i d . o r g . c o n s c r y p t . O p e n S S L K e y H o l d e r g o v . n i s t . j a v a x . s i p . M A X _ S E R V E R _ T R A N S A C T I O N S s h o w S t a t u s I c o n   o n   i n a c t i v e   I n p u t C o n n e c t i o n o r g . a p a c h e . h t t p . i m p l . c l i e n t . R e q u e s t W r a p p e r U n k n o w n   f a i l u r e   i n   o p e n i n g   c a m e r a   d e v i c e :   g e t R e m a i n g P o s t D i a l S t r i n g :   r e t   e m p t y   s t r i n g P o l i c y Q u a l i f i e r I n f o :   [ 
 p o l i c y Q u a l i f i e r I d :   R e m o t e E x c e p t i o n   i n   M e d i a S c a n n e r . s c a n F i l e ( ) P e r s i s t a b l e B u n d l e [ m P a r c e l l e d D a t a . d a t a S i z e = [ g e t A c t i v e S u b s c r i p t i o n I n f o ] -   i n v a l i d   s u b I d i m p l i c i t   t a g g i n g   n o t   i m p l e m e n t e d   f o r   t a g :   a n d r o i d . i n p u t m e t h o d s e r v i c e . E x t r a c t E d i t T e x t < m a n i f e s t >   h a s   m o r e   t h a n   o n e   < a p p l i c a t i o n > o r g . a p a c h e . h a r m o n y . d a l v i k . N a t i v e T e s t T a r g e t N e g a t i v e   o f f s e t   p a s s e d   t o   w r i t e I n p u t S t r e a m W A R N I N G   :   A t t e m p t   t o   c r e t e   A C K   w i t h o u t   O K   A S N . 1   U T C T i m e :   c o n s t r u c t e d   i d e n t i f i e r   a t   [ E r r o r   d e s t r o y i n g   p r i n t e r   d i s c o v e r y   s e s s i o n A d d i t i o n a l   i n f o   m u s t   b e   g r e a t e r   t h a n   z e r o ! D i a l o g   n o t   y e t   e s t a b l i s h e d   - -   n o   r e s p o n s e !     c o l . r g b   * =   t e x t u r e 2 D ( U N I _ T e x 0 ,   t 0 ) . r g b ; 
 K e y   m u s t   b e   E C   p u b l i c   o r   p r i v a t e   k e y ;   w a s   P a r c e l a b l e C a l l   % s   h a s   n o n e x i s t e n t   c h i l d   % s h a s M a t c h e d T e t h e r A p n S e t t i n g   R e m o t e E x c e p t i o n C e r t i f i c a t i o n   p a t h   c o u l d   n o t   b e   v a l i d a t e d . g e t S e r v i c e P r o v i d e r N a m e :   m P a r e n t A p p   i s   n u l l A t t e m p t i n g   t o   c o n n e c t   f i l t e r   n o t   i n   g r a p h ! a n d r o i d . a p p . A c t i v i t y T h r e a d $ D r o p B o x R e p o r t e r K e y   m u s t   b e   D H   p u b l i c   o r   p r i v a t e   k e y ;   w a s   a n d r o i d . s e t t i n g s . N E T W O R K _ O P E R A T O R _ S E T T I N G S 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 5 1 E E 7 8 6 A 8 1 8 F 3 A